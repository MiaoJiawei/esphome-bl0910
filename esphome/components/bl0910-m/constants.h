#pragma once
#include <cstdint>

namespace esphome
{
    namespace bl0910
    {
        // Conversion
        static const float BL0910_UREF = 109700.0 / (1316200000); // Voltage
        static const float BL0910_IREF = 1.097 / (12875 * 5.1); // Current
        static const float BL0910_PREF = 120340.9 / (4041259 * 5.1); // Power
        static const float BL0910_WATT = 16 * BL0910_PREF; // Total power
        static const float BL0910_EREF = 4194304 * 0.032768 * 16 / (3600000 * 16 * (404125 * 51 / 120340.9)); // Energy
        static const float BL0910_CF = 16 * BL0910_EREF; // Total Energy
        static const float BL0910_FREF = 10000000; // Frequency
        static const float BL0910_KI = 12875 * 5.1 / 1.097; // Current coefficient
        static const float BL0910_KP = 40.4125 * 5.1 / 1.097 / 1.097; // Power coefficient
        static const float BL0910_TREF = 12.5 / 59 - 40; // Temperature

        // Register address
        // Voltage
        static const uint8_t BL0910_V_RMS = 0x16;

        // Current
        static const uint8_t BL0910_I_1_RMS = 0x0C;
        static const uint8_t BL0910_I_2_RMS = 0x0D;
        static const uint8_t BL0910_I_3_RMS = 0x0E;
        static const uint8_t BL0910_I_4_RMS = 0x0F;
        static const uint8_t BL0910_I_5_RMS = 0x10;
        static const uint8_t BL0910_I_6_RMS = 0x11;
        static const uint8_t BL0910_I_7_RMS = 0x12;
        static const uint8_t BL0910_I_8_RMS = 0x13;
        static const uint8_t BL0910_I_9_RMS = 0x14;
        static const uint8_t BL0910_I_10_RMS = 0x15;

        // Power
        static const uint8_t BL0910_WATT_1 = 0X22;
        static const uint8_t BL0910_WATT_2 = 0X23;
        static const uint8_t BL0910_WATT_3 = 0X24;
        static const uint8_t BL0910_WATT_4 = 0X25;
        static const uint8_t BL0910_WATT_5 = 0X26;
        static const uint8_t BL0910_WATT_6 = 0X27;
        static const uint8_t BL0910_WATT_7 = 0X28;
        static const uint8_t BL0910_WATT_8 = 0X29;
        static const uint8_t BL0910_WATT_9 = 0X2A;
        static const uint8_t BL0910_WATT_10 = 0X2B;
        static const uint8_t BL0910_WATT_SUM = 0X2C;

        // Active pulse count, unsigned
        static const uint8_t BL0910_CF_1_CNT = 0X2F;
        static const uint8_t BL0910_CF_2_CNT = 0X30;
        static const uint8_t BL0910_CF_3_CNT = 0X31;
        static const uint8_t BL0910_CF_4_CNT = 0X32;
        static const uint8_t BL0910_CF_5_CNT = 0X33;
        static const uint8_t BL0910_CF_6_CNT = 0X34;
        static const uint8_t BL0910_CF_7_CNT = 0X35;
        static const uint8_t BL0910_CF_8_CNT = 0X36;
        static const uint8_t BL0910_CF_9_CNT = 0X37;
        static const uint8_t BL0910_CF_10_CNT = 0X38;
        static const uint8_t BL0910_CF_SUM_CNT = 0X39;

        // Voltage frequency
        static const uint8_t BL0910_FREQUENCY = 0X4E;

        // Angle registers (16-bit unsigned, 0x3C~0x45)
        static const uint8_t BL0910_ANGLE_1 = 0x3C;
        static const uint8_t BL0910_ANGLE_2 = 0x3D;
        static const uint8_t BL0910_ANGLE_3 = 0x3E;
        static const uint8_t BL0910_ANGLE_4 = 0x3F;
        static const uint8_t BL0910_ANGLE_5 = 0x40;
        static const uint8_t BL0910_ANGLE_6 = 0x41;
        static const uint8_t BL0910_ANGLE_7 = 0x42;
        static const uint8_t BL0910_ANGLE_8 = 0x43;
        static const uint8_t BL0910_ANGLE_9 = 0x44;
        static const uint8_t BL0910_ANGLE_10 = 0x45;

        // Internal temperature
        static const uint8_t BL0910_TEMPERATURE = 0X5E;

        // Calibration register
        // RMS gain adjustment register
        static const uint8_t BL0910_RMSGN_1 = 0x6C;
        static const uint8_t BL0910_RMSGN_2 = 0x6D;
        static const uint8_t BL0910_RMSGN_3 = 0x6E;
        static const uint8_t BL0910_RMSGN_4 = 0x6F;
        static const uint8_t BL0910_RMSGN_5 = 0x70;
        static const uint8_t BL0910_RMSGN_6 = 0x71;
        static const uint8_t BL0910_RMSGN_7 = 0x72;
        static const uint8_t BL0910_RMSGN_8 = 0x73;
        static const uint8_t BL0910_RMSGN_9 = 0x74;
        static const uint8_t BL0910_RMSGN_10 = 0x75;

        // RMS offset correction register
        static const uint8_t BL0910_RMSOS_1 = 0x77;
        static const uint8_t BL0910_RMSOS_2 = 0x78;
        static const uint8_t BL0910_RMSOS_3 = 0x79;
        static const uint8_t BL0910_RMSOS_4 = 0x7A;
        static const uint8_t BL0910_RMSOS_5 = 0x7B;
        static const uint8_t BL0910_RMSOS_6 = 0x7C;
        static const uint8_t BL0910_RMSOS_7 = 0x7D;
        static const uint8_t BL0910_RMSOS_8 = 0x7E;
        static const uint8_t BL0910_RMSOS_9 = 0x7F;
        static const uint8_t BL0910_RMSOS_10 = 0x80;

        // Active power gain adjustment register
        static const uint8_t BL0910_WATTGN_1 = 0xB6;
        static const uint8_t BL0910_WATTGN_2 = 0xB7;
        static const uint8_t BL0910_WATTGN_3 = 0xB8;
        static const uint8_t BL0910_WATTGN_4 = 0xB9;
        static const uint8_t BL0910_WATTGN_5 = 0xBA;
        static const uint8_t BL0910_WATTGN_6 = 0xBB;
        static const uint8_t BL0910_WATTGN_7 = 0xBC;
        static const uint8_t BL0910_WATTGN_8 = 0xBD;
        static const uint8_t BL0910_WATTGN_9 = 0xBE;
        static const uint8_t BL0910_WATTGN_10 = 0xBF;

        // User write protection setting register
        static const uint8_t BL0910_USR_WRPROT = 0x9E;
        // Reset Register
        static const uint8_t BL0910_SOFT_RESET = 0x9F;
        // You must first write 0x5555 to the write protection setting register before writing to other registers.
        static const uint8_t BL0910_READ_COMMAND = 0x35;  // 读操作命令
        static const uint8_t BL0910_WRITE_COMMAND = 0xCA; // 写操作命令
        
        const uint8_t BL0910_INIT[2][6] = {
            // Reset to default
            {BL0910_WRITE_COMMAND, BL0910_SOFT_RESET, 0x5A, 0x5A, 0x5A, 0x52},
            // Enable User Operation Write
            {BL0910_WRITE_COMMAND, BL0910_USR_WRPROT, 0x55, 0x55, 0x00, 0xB7}};
        static const uint8_t USR_WRPROT_WITABLE[6] = {0xCA, 0x9E, 0x55, 0x55, 0x00, 0xB7};  // 用户寄存器可操作指令
        static const uint8_t USR_SOFT_RESET[6] = {0xCA, 0x9F, 0x5A, 0x5A, 0x5A, 0x52};  // 用户寄存器可操作指令
        static const uint8_t USR_WRPROT_ONLYREAD[6] = {0xCA, 0x9E, 0x00, 0x00, 0x00, 0x61}; // 用户寄存器只读指令


    } // namespace bl0910
} // namespace esphome
