/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_monitor.h
  * @brief          : 系统监控模块头文件
  ******************************************************************************
  * @attention
  *
  * 系统监控模块负责：
  * 1. 系统自检流程（四步智能检测和纠错）
  * 2. 状态监控功能（实时监控各部件状态）
  * 3. 主动纠错机制（发现错误自动纠正）
  * 4. 电源监控（DC_CTRL电源状态检测）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __SYSTEM_MONITOR_H
#define __SYSTEM_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"

/* 自检状态枚举 --------------------------------------------------------------*/
typedef enum {
    SELFCHECK_IDLE = 0,      // 空闲状态
    SELFCHECK_STEP1,         // 第一步：识别期望状态
    SELFCHECK_STEP2,         // 第二步：继电器检查纠错
    SELFCHECK_STEP3,         // 第三步：接触器检查纠错
    SELFCHECK_STEP4,         // 第四步：温度检测
    SELFCHECK_COMPLETE,      // 自检完成
    SELFCHECK_FAILED         // 自检失败
} SelfCheckState_t;

/* 监控状态枚举 --------------------------------------------------------------*/
typedef enum {
    MONITOR_NORMAL = 0,      // 正常监控状态
    MONITOR_CORRECTING,      // 正在纠错状态
    MONITOR_ERROR           // 监控错误状态
} MonitorState_t;

/* 自检结果结构体 ------------------------------------------------------------*/
typedef struct {
    uint8_t step1_result;    // 第一步结果：0-失败，1-成功
    uint8_t step2_result;    // 第二步结果：0-失败，1-成功
    uint8_t step3_result;    // 第三步结果：0-失败，1-成功
    uint8_t step4_result;    // 第四步结果：0-失败，1-成功
    uint8_t overall_result;  // 总体结果：0-失败，1-成功
    uint8_t progress;        // 自检进度：0-100
} SelfCheckResult_t;

/* 系统状态结构体 ------------------------------------------------------------*/
typedef struct {
    SystemState_t expected_state;    // 期望状态
    SystemState_t current_state;     // 当前状态
    uint8_t power_status;           // 电源状态：0-异常，1-正常
    uint8_t correction_count;       // 纠错次数
    MonitorState_t monitor_state;   // 监控状态
} SystemStatus_t;

/* 宏定义 -------------------------------------------------------------------*/
#define SELFCHECK_MAX_RETRY     3    // 最大重试次数
#define CORRECTION_DELAY_MS     500  // 纠错延时（毫秒）
#define MONITOR_UPDATE_MS       100  // 监控更新周期（毫秒）

/* 函数声明 -----------------------------------------------------------------*/

// 初始化函数
void SystemMonitor_Init(void);                          // 系统监控初始化

// 自检功能函数
SelfCheckState_t SystemMonitor_RunSelfCheck(void);      // 运行自检流程
uint8_t SystemMonitor_GetSelfCheckProgress(void);       // 获取自检进度
SelfCheckResult_t SystemMonitor_GetSelfCheckResult(void); // 获取自检结果

// 状态监控函数  
void SystemMonitor_UpdateMonitoring(void);              // 更新状态监控
SystemStatus_t SystemMonitor_GetSystemStatus(void);     // 获取系统状态
uint8_t SystemMonitor_IsSystemNormal(void);             // 检查系统是否正常

// 电源监控函数
uint8_t SystemMonitor_CheckPowerStatus(void);           // 检查电源状态
void SystemMonitor_HandlePowerError(void);              // 处理电源异常

// 主动纠错函数
uint8_t SystemMonitor_CorrectSystem(void);              // 主动纠错系统
uint8_t SystemMonitor_CorrectRelayStates(void);         // 纠正继电器状态
uint8_t SystemMonitor_CorrectContactorStates(void);     // 纠正接触器状态

// 状态检查函数
uint8_t SystemMonitor_CheckExpectedState(void);         // 检查期望状态
uint8_t SystemMonitor_CheckAllStates(void);             // 检查所有状态
uint8_t SystemMonitor_ValidateSystemState(void);        // 验证系统状态

// 任务处理函数
void SystemMonitor_Task(void);                          // 系统监控任务处理

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_MONITOR_H */ 


