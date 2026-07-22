#include "jy61p.h"

// 实例化全局变量
JY61P_Angle_t g_angle_data = {0}; 
uint8_t g_uart_rx_byte = 0;       

/**
  * @brief  JY61P 单字节解析状态机 (仅提取 0x53 角度包)
  * @param  byte 串口收到的 1 个字节
  */
void JY61P_ParseAngle(uint8_t byte) {
    static uint8_t rx_buf[11];
    static uint8_t rx_cnt = 0;
    static uint8_t state = 0; 

    switch (state) {
        case 0:
            if (byte == 0x55) { 
                rx_buf[0] = byte;
                rx_cnt = 1;
                state = 1;
            }
            break;

        case 1:
            if (byte == 0x53) { // 确认是角度包
                rx_buf[1] = byte;
                rx_cnt = 2;
                state = 2;
            } else if (byte == 0x55) {
                rx_buf[0] = byte;
                rx_cnt = 1;
            } else {
                state = 0; 
            }
            break;

        case 2:
            rx_buf[rx_cnt++] = byte;
            if (rx_cnt >= 11) { 
                uint8_t sum = 0;
                for (int i = 0; i < 10; i++) {
                    sum += rx_buf[i];
                }

                // 校验和通过，提取角度
                if (sum == rx_buf[10]) {
                    int16_t raw_roll  = (int16_t)((rx_buf[3] << 8) | rx_buf[2]);
                    int16_t raw_pitch = (int16_t)((rx_buf[5] << 8) | rx_buf[4]);
                    int16_t raw_yaw   = (int16_t)((rx_buf[7] << 8) | rx_buf[6]);

                    g_angle_data.roll  = (float)raw_roll / 32768.0f * 180.0f;
                    g_angle_data.pitch = (float)raw_pitch / 32768.0f * 180.0f;
                    g_angle_data.yaw   = (float)raw_yaw / 32768.0f * 180.0f;
                    
                    g_angle_data.update_flag = 1; 
                }
                
                state = 0; 
                rx_cnt = 0;
            }
            break;
    }
}
