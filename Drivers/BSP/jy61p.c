#include "jy61p.h"

// 实例化全局变量
JY61P_Angle_t g_angle_data = {0}; 
uint8_t g_uart_rx_byte = 0;       


// 引入 main.c 里的串口1句柄 (用于给陀螺仪发指令)
extern UART_HandleTypeDef huart1;

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
            if (byte == 0x55) 
            { 
                rx_buf[0] = byte;
                rx_cnt = 1;
                state = 1;
            }
            break;

        case 1:
            if (byte == 0x53)           // 确认是角度包
           { 
                rx_buf[1] = byte;
                rx_cnt = 2;
                state = 2;
            } 
            else if (byte == 0x55) 
            {
                rx_buf[0] = byte;
                rx_cnt = 1;
            } 
            else 
            {
                state = 0; 
            }
            break;

        case 2:
            rx_buf[rx_cnt++] = byte;
            if (rx_cnt >= 11) 
            { 
                uint8_t sum = 0;
                for (int i = 0; i < 10; i++) 
                {
                    sum += rx_buf[i];
                }

                // 校验和通过，提取角度
                if (sum == rx_buf[10]) 
                {
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

void JY61P_ZeroYaw(void) {
    // 维特官方指定的十六进制指令集
    uint8_t cmd_unlock[5] = {0xFF, 0xAA, 0x69, 0x88, 0xB5}; // 解锁
    uint8_t cmd_zero[5]   = {0xFF, 0xAA, 0x01, 0x04, 0x00}; // Z轴归零 (寄存器0x01, 值0x04)
    uint8_t cmd_save[5]   = {0xFF, 0xAA, 0x00, 0x00, 0x00}; // 保存配置

    // 1. 发送解锁指令，并等待 200ms 让陀螺仪内部响应
    HAL_UART_Transmit(&huart1, cmd_unlock, 5, HAL_MAX_DELAY);
    HAL_Delay(200);

    // 2. 发送 Z 轴置零指令，手册强烈要求置零后需静置 3 秒钟
    HAL_UART_Transmit(&huart1, cmd_zero, 5, HAL_MAX_DELAY);
    HAL_Delay(3000); 

    // 3. 发送保存配置指令，确保掉电不丢失 (延时 100ms 确保写完 Flash)
    HAL_UART_Transmit(&huart1, cmd_save, 5, HAL_MAX_DELAY);
    HAL_Delay(100);
}
