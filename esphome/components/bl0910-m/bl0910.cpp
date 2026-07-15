#include "bl0910.h"
#include "constants.h"
#include <queue>
#include <cmath>
#include "esphome/core/log.h"

namespace esphome
{
  namespace bl0910
  {
    // 定义日志标签为 "bl0910"
    static const char *const TAG = "bl0910";
    // 将 ube24_t 类型的三个字节数据转换为 uint32_t 类型
    constexpr uint32_t to_uint32_t(ube24_t input) { return input.h << 16 | input.m << 8 | input.l; }
    // 将 sbe24_t 类型的三个字节数据转换为 int32_t 类型
    constexpr int32_t to_int32_t(sbe24_t input) { return input.h << 16 | input.m << 8 | input.l; }
    // 校验和计算函数，计算地址和数据字节的校验和
    constexpr uint8_t bl0910_checksum(const uint8_t address, const DataPacket *data)
    {
      return (address + data->l + data->m + data->h) ^ 0xFF;
    }

    // 主循环，读取数据并处理不同通道的数据
    void BL0910::loop()
    {
      // 如果 current_channel_ 为 UINT8_MAX，则直接返回
      if (this->current_channel_ == UINT8_MAX)
      {
        return;
      }

      this->flush(); // 清除串口缓冲区，避免残留数据干扰

      // 根据当前通道读取不同的传感器数据
      switch (this->current_channel_)
      {
      case 0:
        this->read_data_(BL0910_TEMPERATURE, BL0910_TREF, this->temperature_sensor_); // Temperature
        break;
      case 1:
        this->read_data_(BL0910_I_1_RMS, BL0910_IREF, this->current_1_sensor_);
        this->read_data_(BL0910_WATT_1, BL0910_PREF, this->power_1_sensor_);
        this->read_data_(BL0910_CF_1_CNT, BL0910_EREF, this->energy_1_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_1), this->power_factor_1_sensor_);
        break;
      case 2:
        this->read_data_(BL0910_I_2_RMS, BL0910_IREF, this->current_2_sensor_);
        this->read_data_(BL0910_WATT_2, BL0910_PREF, this->power_2_sensor_);
        this->read_data_(BL0910_CF_2_CNT, BL0910_EREF, this->energy_2_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_2), this->power_factor_2_sensor_);
        break;
      case 3:
        this->read_data_(BL0910_I_3_RMS, BL0910_IREF, this->current_3_sensor_);
        this->read_data_(BL0910_WATT_3, BL0910_PREF, this->power_3_sensor_);
        this->read_data_(BL0910_CF_3_CNT, BL0910_EREF, this->energy_3_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_3), this->power_factor_3_sensor_);
        break;
      case 4:
        this->read_data_(BL0910_I_4_RMS, BL0910_IREF, this->current_4_sensor_);
        this->read_data_(BL0910_WATT_4, BL0910_PREF, this->power_4_sensor_);
        this->read_data_(BL0910_CF_4_CNT, BL0910_EREF, this->energy_4_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_4), this->power_factor_4_sensor_);
        break;
      case 5:
        this->read_data_(BL0910_I_5_RMS, BL0910_IREF, this->current_5_sensor_);
        this->read_data_(BL0910_WATT_5, BL0910_PREF, this->power_5_sensor_);
        this->read_data_(BL0910_CF_5_CNT, BL0910_EREF, this->energy_5_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_5), this->power_factor_5_sensor_);
        break;
      case 6:
        this->read_data_(BL0910_I_6_RMS, BL0910_IREF, this->current_6_sensor_);
        this->read_data_(BL0910_WATT_6, BL0910_PREF, this->power_6_sensor_);
        this->read_data_(BL0910_CF_6_CNT, BL0910_EREF, this->energy_6_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_6), this->power_factor_6_sensor_);
        break;
      case 7:
        this->read_data_(BL0910_I_7_RMS, BL0910_IREF, this->current_7_sensor_);
        this->read_data_(BL0910_WATT_7, BL0910_PREF, this->power_7_sensor_);
        this->read_data_(BL0910_CF_7_CNT, BL0910_EREF, this->energy_7_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_7), this->power_factor_7_sensor_);
        break;
      case 8:
        this->read_data_(BL0910_I_8_RMS, BL0910_IREF, this->current_8_sensor_);
        this->read_data_(BL0910_WATT_8, BL0910_PREF, this->power_8_sensor_);
        this->read_data_(BL0910_CF_8_CNT, BL0910_EREF, this->energy_8_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_8), this->power_factor_8_sensor_);
        break;
      case 9:
        this->read_data_(BL0910_I_9_RMS, BL0910_IREF, this->current_9_sensor_);
        this->read_data_(BL0910_WATT_9, BL0910_PREF, this->power_9_sensor_);
        this->read_data_(BL0910_CF_9_CNT, BL0910_EREF, this->energy_9_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_9), this->power_factor_9_sensor_);
        break;
      case 10:
        this->read_data_(BL0910_I_10_RMS, BL0910_IREF, this->current_10_sensor_);
        this->read_data_(BL0910_WATT_10, BL0910_PREF, this->power_10_sensor_);
        this->read_data_(BL0910_CF_10_CNT, BL0910_EREF, this->energy_10_sensor_);
        this->calculate_power_factor_(this->read_angle_raw_(BL0910_ANGLE_10), this->power_factor_10_sensor_);
        break;
      case (UINT8_MAX - 2):
        this->read_data_(BL0910_FREQUENCY, BL0910_FREF, this->frequency_sensor_); // Frequency
        // 保存频率值供功率因数计算使用
        if (this->frequency_sensor_ != nullptr && this->frequency_sensor_->has_state())
        {
          this->frequency_hz_ = this->frequency_sensor_->state;
        }
        this->read_data_(BL0910_V_RMS, BL0910_UREF, this->voltage_sensor_);       // Voltage
        break;
      case (UINT8_MAX - 1):
        this->read_data_(BL0910_WATT_SUM, BL0910_WATT, this->total_power_sensor_);  // Total power
        this->read_data_(BL0910_CF_SUM_CNT, BL0910_CF, this->total_energy_sensor_); // Total Energy
        break;
      default:
        this->current_channel_ = UINT8_MAX - 2; // Go to frequency and voltage
        return;
      }
      // 递增通道并处理后续操作
      this->current_channel_++;
      this->handle_actions_();
    }

    // 初始化设置函数
    void BL0910::setup()
    {
      this->flush();                                                     // 清空串口缓存
      this->write_array(USR_SOFT_RESET, sizeof(USR_SOFT_RESET));         // 恢复初始化
      this->write_array(USR_WRPROT_WITABLE, sizeof(USR_WRPROT_WITABLE)); // 解除写保护
      // 校准 (1: 对应寄存器; 2: 校准前值; 3: 校准后值)
      this->bias_correction_(BL0910_RMSOS_1, 0, 0);
      this->bias_correction_(BL0910_RMSOS_2, 0, 0);
      this->bias_correction_(BL0910_RMSOS_3, 0, 0);
      this->bias_correction_(BL0910_RMSOS_4, 0, 0);
      this->bias_correction_(BL0910_RMSOS_5, 0, 0);
      this->bias_correction_(BL0910_RMSOS_6, 0, 0);
      this->bias_correction_(BL0910_RMSOS_7, 0, 0);
      this->bias_correction_(BL0910_RMSOS_8, 0, 0);
      this->bias_correction_(BL0910_RMSOS_9, 0, 0);
      this->bias_correction_(BL0910_RMSOS_10, 0, 0);
      this->gain_correction_(BL0910_RMSGN_1, 1, 1);
      this->gain_correction_(BL0910_RMSGN_2, 1, 1);
      this->gain_correction_(BL0910_RMSGN_3, 1, 1);
      this->gain_correction_(BL0910_RMSGN_4, 1, 1);
      this->gain_correction_(BL0910_RMSGN_5, 1, 1);
      this->gain_correction_(BL0910_RMSGN_6, 1, 1);
      this->gain_correction_(BL0910_RMSGN_7, 1, 1);
      this->gain_correction_(BL0910_RMSGN_8, 1, 1);
      this->gain_correction_(BL0910_RMSGN_9, 1, 1);
      this->gain_correction_(BL0910_RMSGN_10, 1, 1);
    }

    // 重置当前通道计数，触发下一次读取数据循环
    void BL0910::update()
    {
      this->current_channel_ = 0;
    }

    std::queue<ActionCallbackFuncPtr> enqueue_action_;
    // 将动作加入队列
    size_t BL0910::enqueue_action_(ActionCallbackFuncPtr function)
    {
      this->action_queue_.push_back(function);
      return this->action_queue_.size();
    }

    // 处理动作队列中的所有操作
    void BL0910::handle_actions_()
    {
      if (this->action_queue_.empty())
      {
        return;
      }
      ActionCallbackFuncPtr ptr_func = nullptr;
      // 遍历所有动作并执行
      for (int i = 0; i < this->action_queue_.size(); i++)
      {
        ptr_func = this->action_queue_[i];
        if (ptr_func)
        {
          ESP_LOGI(TAG, "HandleActionCallback[%d]...", i);
          (this->*ptr_func)();
        }
      }
      // 读取剩余数据并清空队列
      while (this->available())
      {
        this->read();
      }

      this->action_queue_.clear();

      // 处理完动作后清空缓冲区
      this->flush();
    }

    // Reset energy
    void BL0910::reset_energy_()
    {
      // 写入初始化命令并清空缓冲区
      this->write_array(BL0910_INIT[0], 6);
      delay(1);
      this->flush();
      ESP_LOGW(TAG, "Device reset with init command.");
    }

    // Read data
    void BL0910::read_data_(const uint8_t address, const float reference, sensor::Sensor *sensor)
    {
      if (sensor == nullptr)
      {
        return;
      }

      DataPacket buffer;
      ube24_t data_u24;
      sbe24_t data_s24;
      float value = 0;

      // 判断数据类型是否为有符号
      bool signed_result = reference == BL0910_TREF || reference == BL0910_WATT || reference == BL0910_PREF;
      this->flush();
      this->write_byte(BL0910_READ_COMMAND);
      this->write_byte(address);

      // 读取数据
      if (this->read_array((uint8_t *)&buffer, sizeof(buffer) - 1))
      {
        if (bl0910_checksum(address, &buffer) == buffer.checksum)
        {
          // 根据是否有符号处理不同数据格式
          if (signed_result)
          {
            data_s24.l = buffer.l;
            data_s24.m = buffer.m;
            data_s24.h = buffer.h;
          }
          else
          {
            data_u24.l = buffer.l;
            data_u24.m = buffer.m;
            data_u24.h = buffer.h;
          }
        }
        else
        {
          ESP_LOGW(TAG, "Checksum failed. Discarding message."); // 如果校验和错误，丢弃数据
          return;
        }
      }
      else
      {
        ESP_LOGW(TAG, "Read failed for address 0x%02X. Discarding message.", address);
        return;
      }
      // 根据不同的参考值处理数据
      if (reference == BL0910_PREF || reference == BL0910_WATT)
      {
        value = (float)to_int32_t(data_s24) * reference;
      }
      if (reference == BL0910_UREF || reference == BL0910_IREF || reference == BL0910_EREF || reference == BL0910_CF)
      {
        value = (float)to_uint32_t(data_u24) * reference;
      }
      if (reference == BL0910_FREF)
      {
        value = reference / (float)to_uint32_t(data_u24);
      }
      if (reference == BL0910_TREF)
      {
        value = (float)to_int32_t(data_s24);
        value = (value - 64) * 12.5 / 59 - 40;
      }
      sensor->publish_state(value);
    }

    // 读取16位相角寄存器（用于功率因数计算）
    uint16_t BL0910::read_angle_raw_(uint8_t address)
    {
      this->flush();
      this->write_byte(BL0910_READ_COMMAND);
      this->write_byte(address);

      uint8_t data[4];
      if (this->read_array(data, 4))
      {
        uint8_t l = data[0], m = data[1], h = data[2], chksum = data[3];
        uint8_t expected = (address + l + m + h) ^ 0xFF;
        if (chksum == expected)
        {
          // 相角为16位无符号值：高字节为 data_m，低字节为 data_l
          return (uint16_t)((m << 8) | l);
        }
        else
        {
          ESP_LOGW(TAG, "Checksum failed for angle register 0x%02X.", address);
        }
      }
      else
      {
        ESP_LOGW(TAG, "Read failed for angle register 0x%02X.", address);
      }
      return 0;
    }

    // 计算功率因数（通过相角寄存器值，与参考实现 bl0910_ser_reader.py 一致）
    void BL0910::calculate_power_factor_(uint16_t angle_reg, sensor::Sensor *power_factor_sensor)
    {
      if (power_factor_sensor == nullptr)
      {
        return;
      }
      // 相角(°) = 360 * angle_reg * freq / 500000
      float angle_deg = 360.0f * angle_reg * this->frequency_hz_ / 500000.0f;
      // PF = cos(angle_deg)
      float power_factor = cosf(angle_deg * M_PI / 180.0f);
      power_factor_sensor->publish_state(power_factor);
    }

    // 偏移校准函数
    void BL0910::bias_correction_(uint8_t address, float measurements, float correction)
    {
      DataPacket data;
      float i_rms0 = measurements * BL0910_KI;
      float i_rms = correction * BL0910_KI;
      int32_t value = (i_rms * i_rms - i_rms0 * i_rms0) / 256; // 计算校准值
      // 将计算值写入数据包
      data.l = (value >> 0) & 0xFF;
      data.m = (value >> 8) & 0xFF;
      data.h = (value >> 16) & 0xFF;
      data.checksum = bl0910_checksum(address, &data);
      this->flush();
      this->write_byte(BL0910_WRITE_COMMAND);
      this->write_byte(address);
      this->write_array((uint8_t *)&data, sizeof(data));
    }

    // 增益校准函数
    void BL0910::gain_correction_(uint8_t address, float measurements, float correction)
    {
      DataPacket data;
      float i_rms0 = measurements * BL0910_KI;
      float i_rms = correction * BL0910_KI;
      int32_t value = int((i_rms / i_rms0 - 1) * 65536); // 计算校准值
      // 将计算值写入数据包
      data.l = (value >> 0) & 0xFF;
      data.m = (value >> 8) & 0xFF;
      data.h = (value >> 16) & 0xFF;
      data.checksum = bl0910_checksum(address, &data);
      this->flush();
      this->write_byte(BL0910_WRITE_COMMAND);
      this->write_byte(address);
      this->write_array((uint8_t *)&data, sizeof(data));
    }

    void BL0910::dump_config()
    {
      ESP_LOGCONFIG(TAG, "BL0910:");
      LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);

      LOG_SENSOR("  ", "Current1", this->current_1_sensor_);
      LOG_SENSOR("  ", "Current2", this->current_2_sensor_);
      LOG_SENSOR("  ", "Current3", this->current_3_sensor_);
      LOG_SENSOR("  ", "Current4", this->current_4_sensor_);
      LOG_SENSOR("  ", "Current5", this->current_5_sensor_);
      LOG_SENSOR("  ", "Current6", this->current_6_sensor_);
      LOG_SENSOR("  ", "Current7", this->current_7_sensor_);
      LOG_SENSOR("  ", "Current8", this->current_8_sensor_);
      LOG_SENSOR("  ", "Current9", this->current_9_sensor_);
      LOG_SENSOR("  ", "Current10", this->current_10_sensor_);

      LOG_SENSOR("  ", "Power1", this->power_1_sensor_);
      LOG_SENSOR("  ", "Power2", this->power_2_sensor_);
      LOG_SENSOR("  ", "Power3", this->power_3_sensor_);
      LOG_SENSOR("  ", "Power4", this->power_4_sensor_);
      LOG_SENSOR("  ", "Power5", this->power_5_sensor_);
      LOG_SENSOR("  ", "Power6", this->power_6_sensor_);
      LOG_SENSOR("  ", "Power7", this->power_7_sensor_);
      LOG_SENSOR("  ", "Power8", this->power_8_sensor_);
      LOG_SENSOR("  ", "Power9", this->power_9_sensor_);
      LOG_SENSOR("  ", "Power10", this->power_10_sensor_);

      LOG_SENSOR("  ", "Power factor 1", this->power_factor_1_sensor_);
      LOG_SENSOR("  ", "Power factor 2", this->power_factor_2_sensor_);
      LOG_SENSOR("  ", "Power factor 3", this->power_factor_3_sensor_);
      LOG_SENSOR("  ", "Power factor 4", this->power_factor_4_sensor_);
      LOG_SENSOR("  ", "Power factor 5", this->power_factor_5_sensor_);
      LOG_SENSOR("  ", "Power factor 6", this->power_factor_6_sensor_);
      LOG_SENSOR("  ", "Power factor 7", this->power_factor_7_sensor_);
      LOG_SENSOR("  ", "Power factor 8", this->power_factor_8_sensor_);
      LOG_SENSOR("  ", "Power factor 9", this->power_factor_9_sensor_);
      LOG_SENSOR("  ", "Power factor 10", this->power_factor_10_sensor_);

      LOG_SENSOR("  ", "Energy1", this->energy_1_sensor_);
      LOG_SENSOR("  ", "Energy2", this->energy_2_sensor_);
      LOG_SENSOR("  ", "Energy3", this->energy_3_sensor_);
      LOG_SENSOR("  ", "Energy4", this->energy_4_sensor_);
      LOG_SENSOR("  ", "Energy5", this->energy_5_sensor_);
      LOG_SENSOR("  ", "Energy6", this->energy_6_sensor_);
      LOG_SENSOR("  ", "Energy7", this->energy_7_sensor_);
      LOG_SENSOR("  ", "Energy8", this->energy_8_sensor_);
      LOG_SENSOR("  ", "Energy9", this->energy_9_sensor_);
      LOG_SENSOR("  ", "Energy10", this->energy_10_sensor_);

      LOG_SENSOR("  ", "Total Power", this->total_power_sensor_);
      LOG_SENSOR("  ", "Total Energy", this->total_energy_sensor_);
      LOG_SENSOR("  ", "Frequency", this->frequency_sensor_);
      LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
    }

  } // namespace bl0910
} // namespace esphome
