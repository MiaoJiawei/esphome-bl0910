import serial
import serial.tools.list_ports
import time
import math
import sys

# ==================== 串口配置 ====================
BAUDRATE = 4800                # 必须与硬件设置的波特率一致（由 CS/SCLK 引脚决定）
TIMEOUT = 0.5                   # 串口读超时（秒）

# ==================== 电路参数（互感器方式）====================
# 电流通道（以应用指南典型值为例）
Rt = 2000                       # 电流互感器变比（2000:1）
R_load = 10.2                   # 电流互感器二次侧负载电阻 (R1+R2)，单位 Ω（两个5.1Ω串联）
Gain_I = 1                      # 电流通道 PGA 增益（默认1倍）

# 电压通道
R_sum = 165.0                  # 电压采样串联总电阻 (R25+R26+R35+R36+R37)，单位 kΩ（五个33k）
R46 = 150.0                     # 电压互感器二次侧负载电阻，单位 Ω
Gain_V = 1.025                      # 电压通道 PGA 增益（默认1倍）

# 公共参数
Vref = 1.097                    # 内部基准电压，单位 V
freq = 50                       # 电网频率（默认50Hz，会在运行时从芯片 FREQ 寄存器读取并覆盖）

# ==================== 寄存器地址 ====================
ADDR_V_RMS          = 0x16      # 电压有效值（24位无符号）
ADDR_I_RMS_BASE     = 0x0C      # 电流有效值通道1（0x0C~0x15）
ADDR_WATT_BASE      = 0x22      # 有功功率通道1（0x22~0x2B，24位有符号）
ADDR_ANGLE_BASE     = 0x3C      # 夹角通道1（0x3C~0x45，16位无符号）
ADDR_FREQ           = 0x4E      # 电网频率寄存器（20位无符号，实际频率 = 10000000/FREQ_reg，读取时 bit_width=20）
ADDR_TPS1           = 0x5E      # 内部温度寄存器（10位，低10位有效）

# ==================== 工具函数 ====================
def calc_checksum(addr, data_l, data_m, data_h):
    """计算校验和： (addr + data_l + data_m + data_h) & 0xFF 取反"""
    s = (addr + data_l + data_m + data_h) & 0xFF
    return (~s) & 0xFF

def read_register(ser, addr, bit_width=24, signed=False):
    """
    通过 UART 读取一个寄存器
    :param ser: 已打开的串口对象
    :param addr: 寄存器地址（8位）
    :param bit_width: 数据位宽，支持 16、20、24，决定有效数据掩码
    :param signed: 是否带符号（有功功率为 True，其余一般 False）
    :return: 解析后的整数值，校验失败返回 None
    """
    # 发送读命令：0x35 + addr
    ser.write(bytes([0x35, addr]))
    time.sleep(0.01)  # 等待芯片响应（可根据实际情况调整）

    # 芯片始终返回 4 字节：数据低、中、高 + 校验和
    resp = ser.read(4)
    if len(resp) != 4:
        print(f"读寄存器 0x{addr:02X} 失败：只收到 {len(resp)} 字节")
        return None

    data_l, data_m, data_h, chksum = resp[0], resp[1], resp[2], resp[3]

    # 校验和验证（所有数据字节都参与校验，与芯片协议一致）
    expected = calc_checksum(addr, data_l, data_m, data_h)
    if chksum != expected:
        print(f"校验和错误：addr=0x{addr:02X}, recv chksum=0x{chksum:02X}, expected=0x{expected:02X}")
        return None

    # 合成数据（低字节在前）
    value = (data_h << 16) | (data_m << 8) | data_l
    if bit_width == 16:
        value = (data_m << 8) | data_l       # 16位数据，高字节为 data_m
    elif bit_width == 20:
        value &= 0xFFFFF                      # 仅保留低20位
    # bit_width == 24: 使用完整的24位值

    if signed and (value & (1 << (bit_width - 1))):
        value -= (1 << bit_width)
    return value

def angle_to_pf(angle_reg, freq=50):
    """
    将夹角寄存器值转换为功率因数
    夹角(°) = 360 * angle_reg * freq / 500000
    """
    angle_deg = 360.0 * angle_reg * freq / 500000.0
    return math.cos(math.radians(angle_deg))

def tps1_to_temp(tps_reg):
    """
    内部温度转换公式：温度(°C) = (TPS - 64) * 12.5 / 59 - 40
    TPS 为 10 位值（0~1023）
    """
    tps = tps_reg & 0x3FF  # 取低10位
    return (tps - 64) * 12.5 / 59.0 - 40.0

# ==================== 物理量转换函数 ====================
def convert_current(irms_reg, gain_i, rt, r_load, vref=Vref):
    """
    电流有效值寄存器 -> 实际电流 (A)
    Irms = 12875 * Gain_I * Vi / Vref
    Vi (mV) = I * r_load * 1000 / rt
    所以 I = Irms * Vref * rt / (12875 * Gain_I * r_load * 1000)
    """
    return irms_reg * vref * rt / (12875.0 * gain_i * r_load * 1000.0)

def convert_voltage(vrms_reg, gain_v, r_sum, r46, vref=Vref):
    """
    电压有效值寄存器 -> 实际电压 (V)
    Vrms = 13162 * Gain_V * Vi / Vref
    Vi (mV) = V * r46 / r_sum   (r_sum 单位 kΩ，r46 单位 Ω)
    所以 V = Vrms * Vref * r_sum / (13162 * Gain_V * r46 * 1000)
    """
    return vrms_reg * vref * r_sum / (13162.0 * gain_v * r46)

def convert_power(watt_reg, gain_i, gain_v, rt, r_load, r_sum, r46, vref=Vref):
    """
    有功功率寄存器 -> 实际功率 (W)
    WATT = 40.4125 * Gain_I * Gain_V * Vi * Vi / Vref^2   ??? 实际应为电流电压有效值乘积
    更准确的公式：WATT = 40.4125 * Gain_I * Gain_V * Vi * Vv / Vref^2
    其中 Vi 是电流通道输入电压(mV)，Vv 是电压通道输入电压(mV)
    代入 Vi = I * r_load * 1000 / rt, Vv = V * r46 / r_sum
    得 P = WATT * Vref^2 * r_sum * rt / (40.4125 * Gain_I * Gain_V * r_load * 1000 * r46 * 1000)
    简化：P = WATT * Vref^2 * r_sum * rt / (40.4125 * Gain_I * Gain_V * r_load * r46 * 1e6)
    """
    numerator = watt_reg * vref * vref * r_sum * rt
    denominator = 40.4125 * gain_i * gain_v * r_load * r46 * 1e6
    return numerator / denominator

# ==================== 串口扫描与选择 ====================
def list_serial_ports():
    """扫描系统所有可用串口，返回列表 [(设备名, 描述, 硬件ID), ...]"""
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("未检测到任何串口！")
        return []
    
    print("\n可用的串口列表：")
    print("-" * 70)
    print(f"{'序号':<6} {'设备名':<20} {'描述':<40}")
    print("-" * 70)
    for i, port in enumerate(ports, 1):
        desc = port.description if port.description else "(无描述)"
        print(f"{i:<6} {port.device:<20} {desc:<40}")
    print("-" * 70)
    return [(port.device, port.description) for port in ports]

def select_serial_port():
    """扫描串口并让用户选择"""
    port_list = list_serial_ports()
    if not port_list:
        print("无可用串口，请检查硬件连接。")
        sys.exit(1)

    # 如果只有一个串口，自动选择
    if len(port_list) == 1:
        selected = port_list[0][0]
        print(f"仅检测到一个串口，自动选择: {selected}")
        return selected

    # 多个串口时让用户选择
    while True:
        try:
            choice = input(f"\n请选择串口 (1-{len(port_list)}): ").strip()
            if not choice:
                continue
            idx = int(choice) - 1
            if 0 <= idx < len(port_list):
                selected = port_list[idx][0]
                print(f"已选择串口: {selected}")
                return selected
            else:
                print(f"输入无效，请输入 1-{len(port_list)} 之间的数字。")
        except (ValueError, IndexError):
            print(f"输入无效，请输入 1-{len(port_list)} 之间的数字。")

# ==================== 主程序 ====================
def main():
    # 扫描并选择串口
    SERIAL_PORT = select_serial_port()
    
    try:
        ser = serial.Serial(SERIAL_PORT, baudrate=BAUDRATE, timeout=TIMEOUT)
    except Exception as e:
        print(f"无法打开串口 {SERIAL_PORT}: {e}")
        return

    print("开始读取 BL0910 数据（每10秒一次，互感器采样方式）...")
    print("-" * 80)

    try:
        while True:
            # 从芯片读取电网频率（20位寄存器，实际频率 = 10000000 / FREQ_reg）
            freq_reg = read_register(ser, ADDR_FREQ, bit_width=20, signed=False)
            if freq_reg is not None:
                freq_hz = 10000000.0 / freq_reg
            else:
                print("频率读取失败，使用默认值 50Hz")
                freq_hz = 50.0

            # 读取电压有效值
            vrms_reg = read_register(ser, ADDR_V_RMS, bit_width=24, signed=False)
            if vrms_reg is None:
                print("电压读取失败，跳过本次循环")
                time.sleep(10)
                continue

            # 读取内部温度
            tps_reg = read_register(ser, ADDR_TPS1, bit_width=24, signed=False)
            temperature = tps1_to_temp(tps_reg) if tps_reg is not None else float('nan')

            # 转换电压为实际值
            v_actual = convert_voltage(vrms_reg, Gain_V, R_sum, R46, Vref)

            # 准备存储各通道数据
            channels = []
            for i in range(1, 11):
                addr_i = ADDR_I_RMS_BASE + (i - 1)
                addr_w = ADDR_WATT_BASE + (i - 1)
                addr_a = ADDR_ANGLE_BASE + (i - 1)

                irms_reg = read_register(ser, addr_i, bit_width=24, signed=False)
                watt_reg = read_register(ser, addr_w, bit_width=24, signed=True)
                angle_reg = read_register(ser, addr_a, bit_width=16, signed=False)

                if None in (irms_reg, watt_reg, angle_reg):
                    print(f"通道 {i} 读取失败，数据可能不完整")
                    continue

                # 转换为实际物理量
                i_actual = convert_current(irms_reg, Gain_I, Rt, R_load, Vref)
                p_actual = convert_power(watt_reg, Gain_I, Gain_V, Rt, R_load, R_sum, R46, Vref)
                pf = angle_to_pf(angle_reg, freq_hz)

                channels.append({
                    'channel': i,
                    'i_actual': i_actual,
                    'p_actual': p_actual,
                    'pf': pf,
                    'irms_reg': irms_reg,
                    'watt_reg': watt_reg,
                    'angle_reg': angle_reg
                })

            # 打印结果
            print(f"Time: {time.strftime('%Y-%m-%d %H:%M:%S')}")
            print(f"Temp: {temperature:.2f} °C | Freq: {freq_hz:.2f} Hz")
            print(f"V: {v_actual:.3f} V (Reg: {vrms_reg:8d})")
            for ch in channels:
                print(f"Ch. {ch['channel']:2d} | I: {ch['i_actual']:8.3f} A | "
                      f"P: {ch['p_actual']:8.3f} W | Pf: {ch['pf']:6.4f} | "
                      f"(Reg: I={ch['irms_reg']:6d}, W={ch['watt_reg']:8d}, A={ch['angle_reg']:5d})")
            print("-" * 80)

            # 等待10秒
            time.sleep(10)

    except KeyboardInterrupt:
        print("\n用户中断")
    finally:
        ser.close()

if __name__ == "__main__":
    main()