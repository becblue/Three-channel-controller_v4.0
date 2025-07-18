/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_config.h
  * @brief          : 系统配置头文件
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 异常类型定义 ---------------------------------------------------------------*/
typedef enum
{
    ALARM_NONE = 0,    // 无异常
    ALARM_A,           // A类异常 - 使能冲突
    ALARM_B,           // B类异常 - K1_1_STA异常
    ALARM_C,           // C类异常 - K2_1_STA异常
    ALARM_D,           // D类异常 - K3_1_STA异常
    ALARM_E,           // E类异常 - K1_2_STA异常
    ALARM_F,           // F类异常 - K2_2_STA异常
    ALARM_G,           // G类异常 - K3_2_STA异常
    ALARM_H,           // H类异常 - SW1_STA异常
    ALARM_I,           // I类异常 - SW2_STA异常
    ALARM_J,           // J类异常 - SW3_STA异常
    ALARM_K,           // K类异常 - NTC_1温度异常
    ALARM_L,           // L类异常 - NTC_2温度异常
    ALARM_M,           // M类异常 - NTC_3温度异常
    ALARM_N,           // N类异常 - 自检异常
    ALARM_O            // O类异常 - 电源监控异常
} AlarmType_t;

/* 系统状态定义 ---------------------------------------------------------------*/
typedef enum
{
    STATE_STARTUP = 0,    // 系统启动
    STATE_LOGO,           // LOGO显示
    STATE_SELFCHECK,      // 系统自检
    STATE_STANDBY,        // 系统待机
    STATE_CH1_ON,         // 通道1开启
    STATE_CH2_ON,         // 通道2开启
    STATE_CH3_ON,         // 通道3开启
    STATE_ERROR           // 系统错误
} SystemState_t;

/* 通道定义 -------------------------------------------------------------------*/
typedef enum
{
    CHANNEL_OFF = 0,      // 关闭
    CHANNEL_1,            // 通道1
    CHANNEL_2,            // 通道2
    CHANNEL_3             // 通道3
} Channel_t;

/* 风扇模式定义 ---------------------------------------------------------------*/
typedef enum
{
    FAN_MODE_NORMAL = 0,  // 正常模式（温度<35℃）
    FAN_MODE_HIGH_TEMP,   // 高温模式（温度≥35℃）
    FAN_MODE_OVER_TEMP    // 超温模式（温度≥60℃）
} FanMode_t;

/* 配置参数 -------------------------------------------------------------------*/
#define TEMP_NORMAL         35.0f     // 正常温度阈值
#define TEMP_HIGH           60.0f     // 高温阈值
#define TEMP_HYSTERESIS     2.0f      // 温度回差

#define FAN_PWM_NORMAL      50        // 正常风扇占空比
#define FAN_PWM_HIGH        95        // 高温风扇占空比

#define RELAY_PULSE_TIME    500       // 继电器脉冲时间ms
#define LOGO_DISPLAY_TIME   2000      // LOGO显示时间ms

/* 全局变量 -------------------------------------------------------------------*/
extern volatile AlarmType_t g_alarm;         // 当前异常
extern volatile SystemState_t g_state;       // 当前状态
extern volatile Channel_t g_channel;         // 当前通道
extern volatile FanMode_t g_fan_mode;        // 当前风扇模式

/* 函数声明 -------------------------------------------------------------------*/
void SystemConfig_Init(void);                // 初始化

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CONFIG_H */ 

