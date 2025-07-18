/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_config.c
  * @brief          : 系统配置源文件
  ******************************************************************************
  */
/* USER CODE END Header */

#include "system_config.h"
#include "alarm_manager.h"

/* 全局变量定义 ---------------------------------------------------------------*/
volatile AlarmType_t g_alarm = ALARM_NONE;        // 当前异常
volatile SystemState_t g_state = STATE_STARTUP;   // 当前状态
volatile Channel_t g_channel = CHANNEL_OFF;       // 当前通道
volatile FanMode_t g_fan_mode = FAN_MODE_NORMAL;  // 当前风扇模式

/**
  * @brief  系统配置初始化
  * @param  None
  * @retval None
  */
void SystemConfig_Init(void)
{
    // 初始化全局变量
    g_alarm = ALARM_NONE;
    g_state = STATE_STARTUP;
    g_channel = CHANNEL_OFF;
} 

 
