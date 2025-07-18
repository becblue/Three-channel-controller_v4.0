/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.h
  * @brief          : 继电器控制头文件
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __RELAY_CONTROL_H
#define __RELAY_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"

/* 函数声明 -------------------------------------------------------------------*/

// 初始化
void RelayControl_Init(void);

// 使能信号检测（带消抖）
uint8_t RelayControl_ReadK1_EN(void);     // 读取K1_EN状态
uint8_t RelayControl_ReadK2_EN(void);     // 读取K2_EN状态  
uint8_t RelayControl_ReadK3_EN(void);     // 读取K3_EN状态

// 继电器输出控制（双路同时触发500ms脉冲）
void RelayControl_TurnOnChannel1(void);   // 开启通道1（K1_1_ON + K1_2_ON同时脉冲）
void RelayControl_TurnOnChannel2(void);   // 开启通道2（K2_1_ON + K2_2_ON同时脉冲）
void RelayControl_TurnOnChannel3(void);   // 开启通道3（K3_1_ON + K3_2_ON同时脉冲）

void RelayControl_TurnOffChannel1(void);  // 关闭通道1（K1_1_OFF + K1_2_OFF同时脉冲）
void RelayControl_TurnOffChannel2(void);  // 关闭通道2（K2_1_OFF + K2_2_OFF同时脉冲）
void RelayControl_TurnOffChannel3(void);  // 关闭通道3（K3_1_OFF + K3_2_OFF同时脉冲）

// 状态反馈检测
uint8_t RelayControl_ReadK1_1_STA(void);  // 读取K1_1_STA状态
uint8_t RelayControl_ReadK1_2_STA(void);  // 读取K1_2_STA状态
uint8_t RelayControl_ReadK2_1_STA(void);  // 读取K2_1_STA状态
uint8_t RelayControl_ReadK2_2_STA(void);  // 读取K2_2_STA状态
uint8_t RelayControl_ReadK3_1_STA(void);  // 读取K3_1_STA状态
uint8_t RelayControl_ReadK3_2_STA(void);  // 读取K3_2_STA状态

// 接触器状态检测
uint8_t RelayControl_ReadSW1_STA(void);   // 读取SW1_STA状态
uint8_t RelayControl_ReadSW2_STA(void);   // 读取SW2_STA状态
uint8_t RelayControl_ReadSW3_STA(void);   // 读取SW3_STA状态

// 状态获取封装函数
uint8_t RelayControl_GetRelayStatus(Channel_t channel, uint8_t relay_num);  // 获取继电器状态（1或2）
uint8_t RelayControl_GetContactorStatus(Channel_t channel);                 // 获取接触器状态
ChannelState_t RelayControl_GetChannelState(Channel_t channel);             // 获取通道状态

// 基础异常检测
AlarmType_t RelayControl_CheckEnableConflict(void);     // 检测A类异常（使能冲突）
AlarmType_t RelayControl_CheckRelayStatus(Channel_t ch); // 检测继电器状态异常

// 任务处理函数
void RelayControl_Task(void);                           // 继电器控制任务处理

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CONTROL_H */ 

 
