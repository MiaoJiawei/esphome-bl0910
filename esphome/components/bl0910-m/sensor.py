from esphome import automation
from esphome.automation import maybe_simple_id
import esphome.codegen as cg
from esphome.components import sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_CHANNEL, CONF_CURRENT, CONF_ENERGY, CONF_FREQUENCY, CONF_ID, CONF_NAME, CONF_POWER, CONF_TEMPERATURE, CONF_TOTAL_POWER, CONF_VOLTAGE, CONF_POWER_FACTOR, DEVICE_CLASS_CURRENT, DEVICE_CLASS_ENERGY, DEVICE_CLASS_FREQUENCY, DEVICE_CLASS_POWER, DEVICE_CLASS_TEMPERATURE, DEVICE_CLASS_VOLTAGE, DEVICE_CLASS_POWER_FACTOR, ICON_CURRENT_AC, ICON_THERMOMETER, STATE_CLASS_MEASUREMENT, STATE_CLASS_TOTAL_INCREASING, UNIT_AMPERE, UNIT_CELSIUS, UNIT_HERTZ, UNIT_KILOWATT_HOURS, UNIT_VOLT, UNIT_WATT,
)

# 自定义图标
ICON_POWER = "mdi:gauge"
ICON_ENERGY = "mdi:lightning-bolt"
ICON_FREQUENCY = "mdi:metronome"
ICON_VOLTAGE = "mdi:sine-wave"
ICON_POWER_FACTOR = "mdi:angle-acute"

# 组件依赖于UART模块，AUTO_LOAD将加载BL0910模块
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["bl0910-m"]
CONF_TOTAL_ENERGY = "total_energy"

# 定义命名空间和类
bl0910_ns = cg.esphome_ns.namespace("bl0910")
BL0910 = bl0910_ns.class_("BL0910", cg.PollingComponent, uart.UARTDevice)
ResetEnergyAction = bl0910_ns.class_("ResetEnergyAction", automation.Action)

# 传感器通用配置模式，支持通用配置
def create_sensor_schema(icon, accuracy_decimals, device_class, unit, state_class):
    return sensor.sensor_schema(
        icon=icon,
        accuracy_decimals=accuracy_decimals,
        device_class=device_class,
        unit_of_measurement=unit,
        state_class=state_class,
    )

# 配置模式（CONFIG_SCHEMA），用于定义BL0910传感器的各项参数
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BL0910),
            cv.Optional(CONF_FREQUENCY): create_sensor_schema(ICON_FREQUENCY, 2, DEVICE_CLASS_FREQUENCY, UNIT_HERTZ, STATE_CLASS_MEASUREMENT),
            cv.Optional(CONF_TEMPERATURE): create_sensor_schema(ICON_THERMOMETER, 2, DEVICE_CLASS_TEMPERATURE, UNIT_CELSIUS, STATE_CLASS_MEASUREMENT),
            cv.Optional(CONF_VOLTAGE): create_sensor_schema(ICON_VOLTAGE, 1, DEVICE_CLASS_VOLTAGE, UNIT_VOLT, STATE_CLASS_MEASUREMENT),
            cv.Optional(CONF_TOTAL_POWER): create_sensor_schema(ICON_POWER, 3, DEVICE_CLASS_POWER, UNIT_WATT, STATE_CLASS_MEASUREMENT),
            cv.Optional(CONF_TOTAL_ENERGY): create_sensor_schema(ICON_ENERGY, 3, DEVICE_CLASS_ENERGY, UNIT_KILOWATT_HOURS, STATE_CLASS_TOTAL_INCREASING),
        }
    )
    # 为每个通道创建一个Schema，每个通道可以包含电流、功率和能量传感器
    .extend(
        cv.Schema(
            {
                cv.Optional(f"{CONF_CHANNEL}_{i + 1}"): cv.Schema(
                    {
                        cv.Optional(CONF_CURRENT): cv.maybe_simple_value(
                            create_sensor_schema(ICON_CURRENT_AC, 3, DEVICE_CLASS_CURRENT, UNIT_AMPERE, STATE_CLASS_MEASUREMENT),
                            key=CONF_NAME,
                        ),
                        cv.Optional(CONF_POWER): cv.maybe_simple_value(
                            create_sensor_schema(ICON_POWER, 3, DEVICE_CLASS_POWER, UNIT_WATT, STATE_CLASS_MEASUREMENT),
                            key=CONF_NAME,
                        ),
                        cv.Optional(CONF_ENERGY): cv.maybe_simple_value(
                            create_sensor_schema(ICON_ENERGY, 3, DEVICE_CLASS_ENERGY, UNIT_KILOWATT_HOURS, STATE_CLASS_TOTAL_INCREASING),
                            key=CONF_NAME,
                        ),
                        cv.Optional(CONF_POWER_FACTOR): cv.maybe_simple_value(
                            create_sensor_schema(ICON_POWER_FACTOR, 3, DEVICE_CLASS_POWER_FACTOR, "", STATE_CLASS_MEASUREMENT),
                            key=CONF_NAME,
                        ),
                    }
                )
                for i in range(10) # 创建10个通道配置
            }
        )
    )
    # 将UART设备和轮询组件的配置参数扩展到模式中
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.polling_component_schema("10s"))
)

# 设备验证，指定UART的通信参数（波特率由用户在 uart 配置中自由设置）
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "bl0910", require_tx=True, require_rx=True
)

# 注册“重置电量”动作
@automation.register_action(
    "bl0910.reset_energy",
    ResetEnergyAction,
    maybe_simple_id(
        {
            cv.Required(CONF_ID): cv.use_id(BL0910),
        }
    ),
)
async def reset_energy_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

# 辅助函数，用于创建和注册传感器
async def register_sensor(var, config, sensor_name, sensor_fn):
    if sensor_config := config.get(sensor_name):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(sensor_fn(sens))

# 主函数：根据配置生成代码
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # 注册传感器：频率、温度、电压、总功率、总能量
    await register_sensor(var, config, CONF_FREQUENCY, var.set_frequency_sensor)
    await register_sensor(var, config, CONF_TEMPERATURE, var.set_temperature_sensor)
    await register_sensor(var, config, CONF_VOLTAGE, var.set_voltage_sensor)
    await register_sensor(var, config, CONF_TOTAL_POWER, var.set_total_power_sensor)
    await register_sensor(var, config, CONF_TOTAL_ENERGY, var.set_total_energy_sensor)

    # 遍历10个通道，注册各自的电流、功率、电量及功率因数传感器
    for i in range(10):
        if channel_config := config.get(f"{CONF_CHANNEL}_{i + 1}"):
            await register_sensor(var, channel_config, CONF_CURRENT, getattr(var, f"set_current_{i + 1}_sensor"))
            await register_sensor(var, channel_config, CONF_POWER, getattr(var, f"set_power_{i + 1}_sensor"))
            await register_sensor(var, channel_config, CONF_ENERGY, getattr(var, f"set_energy_{i + 1}_sensor"))
            await register_sensor(var, channel_config, CONF_POWER_FACTOR, getattr(var, f"set_power_factor_{i + 1}_sensor"))
