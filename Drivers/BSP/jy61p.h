#ifndef __JY61P_H
#define __JY61P_H

#include "main.h"

// 角度数据结构体
typedef struct {
    float roll;          // 横滚角 (X轴)
    float pitch;         // 俯仰角 (Y轴)
    float yaw;           // 偏航角 (Z轴)
    uint8_t update_flag; // 数据更新标志位 (1表示收到了新数据)
} JY61P_Angle_t;

// 声明全局角度变量，供 main.c 读取
extern JY61P_Angle_t g_angle_data;

// 声明全局单字节接收缓存，供中断回调使用
extern uint8_t g_uart_rx_byte;

// 解析状态机函数声明
void JY61P_ParseAngle(uint8_t byte);

//偏航角归零
void JY61P_ZeroYaw(void);


#endif /* __JY61P_H */
