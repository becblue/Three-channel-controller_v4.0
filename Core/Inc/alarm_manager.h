/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_manager.h
  * @brief          : 报警管理系统头文件
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ALARM_MANAGER_H
#define __ALARM_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"

/* 蜂鸣器响声模式定义 ---------------------------------------------------------*/
typedef enum
{
    BEEP_MODE_OFF = 0,        // 关闭蜂鸣器
    BEEP_MODE_1S_PULSE,       // 1秒间隔脉冲（A类、N类异常）
    BEEP_MODE_50MS_PULSE,     // 50ms间隔脉冲（B~J类异常）
    BEEP_MODE_CONTINUOUS      // 持续低电平（K~M类异常）
} BeepMode_t;

/* 函数声明 -------------------------------------------------------------------*/

// 初始化
void AlarmManager_Init(void);

// 报警设置
void AlarmManager_SetAlarm(AlarmType_t alarm_type);           // 设置异常报警
void AlarmManager_ClearAlarm(AlarmType_t alarm_type);         // 清除异常报警
void AlarmManager_ClearAllAlarms(void);                       // 清除所有异常

// 报警状态
uint8_t AlarmManager_IsAlarmActive(AlarmType_t alarm_type);   // 检查特定异常是否激活
uint8_t AlarmManager_HasAnyAlarm(void);                       // 检查是否有任何异常

// 报警输出控制
void AlarmManager_SetAlarmOutput(uint8_t enable);             // 控制ALARM输出（PB4）
void AlarmManager_SetBeepMode(BeepMode_t mode);               // 设置蜂鸣器模式

// 主处理函数
void AlarmManager_Process(void);                              // 报警管理主处理

// 报警解除条件检查
uint8_t AlarmManager_CheckAlarmClearCondition(AlarmType_t alarm_type);  // 检查报警解除条件

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_MANAGER_H */ 

