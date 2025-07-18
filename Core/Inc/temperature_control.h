/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature_control.h
  * @brief          : 温度控制头文件
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TEMPERATURE_CONTROL_H
#define __TEMPERATURE_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"
#include "adc.h"
#include "tim.h"

/* 函数声明 -------------------------------------------------------------------*/

// 初始化
void TemperatureControl_Init(void);

// ADC数据采集
uint16_t TemperatureControl_GetAdcValue(uint8_t channel);  // 获取指定通道ADC值

// 温度计算
float TemperatureControl_GetTemperature(uint8_t ntc_channel);  // 获取指定NTC温度

// 风扇控制
void TemperatureControl_SetFanDuty(uint8_t duty_percent);      // 设置PWM占空比
uint16_t TemperatureControl_GetFanRpm(void);                   // 获取风扇转速

// 温度监控
void TemperatureControl_Process(void);                         // 温度控制主处理
AlarmType_t TemperatureControl_CheckTemperatureAlarm(void);    // 检查温度异常

#ifdef __cplusplus
}
#endif

#endif /* __TEMPERATURE_CONTROL_H */ 

