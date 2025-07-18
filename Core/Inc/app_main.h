/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_main.h
  * @brief          : 应用主控制模块头文件
  ******************************************************************************
  * @attention
  *
  * 应用主控制模块负责：
  * 1. 系统状态机管理（启动→自检→工作→错误）
  * 2. 主循环控制逻辑（任务调度和时序控制）
  * 3. 各功能模块整合（统一调用接口）
  * 4. 系统工作流程实现（完整业务逻辑）
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"
#include "oled_display.h"
#include "system_monitor.h"

/* 系统主状态枚举 ------------------------------------------------------------*/
typedef enum {
    SYSTEM_STARTUP = 0,      // 启动状态（显示LOGO和初始化）
    SYSTEM_SELFCHECK,        // 自检状态（四步自检流程）
    SYSTEM_WORKING           // 工作状态（正常运行监控）
} SystemMainState_t;

/* 启动子状态枚举 ------------------------------------------------------------*/
typedef enum {
    STARTUP_INIT = 0,        // 启动初始化
    STARTUP_BUHLER_LOGO,     // 显示Buhler公司LOGO
    STARTUP_MINYER_LOGO,     // 显示Minyer LOGO + 进度条
    STARTUP_COMPLETE         // 启动完成
} StartupSubState_t;

/* 工作子状态枚举 ------------------------------------------------------------*/
typedef enum {
    WORKING_MONITORING = 0,  // 工作监控状态
    WORKING_PROCESSING,      // 工作处理状态
    WORKING_DISPLAY_UPDATE   // 显示更新状态
} WorkingSubState_t;

/* 系统运行状态结构体 --------------------------------------------------------*/
typedef struct {
    SystemMainState_t main_state;        // 主状态
    StartupSubState_t startup_substate;  // 启动子状态
    WorkingSubState_t working_substate;  // 工作子状态
    uint32_t state_start_time;          // 状态开始时间
    uint8_t state_changed;              // 状态改变标志
    uint8_t system_ready;               // 系统就绪标志
} SystemRunState_t;

/* 任务调度状态结构体 --------------------------------------------------------*/
typedef struct {
    uint32_t last_relay_task_time;      // 上次继电器任务时间
    uint32_t last_temp_task_time;       // 上次温度任务时间
    uint32_t last_alarm_task_time;      // 上次报警任务时间
    uint32_t last_display_task_time;    // 上次显示任务时间
    uint32_t last_monitor_task_time;    // 上次监控任务时间
} TaskScheduleState_t;

/* 宏定义 -------------------------------------------------------------------*/
#define MAIN_LOOP_DELAY_MS          10   // 主循环延时（毫秒）
#define BUHLER_LOGO_DISPLAY_MS      2000 // Buhler LOGO显示时间（毫秒）
#define MINYER_LOGO_DISPLAY_MS      2000 // Minyer LOGO显示时间（毫秒）

// 任务调度周期定义（毫秒）
#define RELAY_TASK_PERIOD_MS        50   // 继电器任务周期
#define TEMP_TASK_PERIOD_MS         100  // 温度任务周期
#define ALARM_TASK_PERIOD_MS        20   // 报警任务周期
#define DISPLAY_TASK_PERIOD_MS      100  // 显示任务周期
#define MONITOR_TASK_PERIOD_MS      100  // 监控任务周期

/* 函数声明 -----------------------------------------------------------------*/

// 主控制函数
void AppMain_Init(void);                       // 应用主控制初始化
void AppMain_Run(void);                        // 应用主控制运行（主循环）
void AppMain_Reset(void);                      // 应用主控制复位

// 状态机函数
void AppMain_StateMachine(void);               // 主状态机处理
void AppMain_StartupStateMachine(void);        // 启动状态机处理
void AppMain_WorkingStateMachine(void);        // 工作状态机处理
void AppMain_ErrorStateMachine(void);          // 错误状态机处理

// 任务调度函数
void AppMain_TaskScheduler(void);              // 任务调度器
void AppMain_ScheduleRelayTask(void);          // 调度继电器任务
void AppMain_ScheduleTemperatureTask(void);    // 调度温度任务
void AppMain_ScheduleAlarmTask(void);          // 调度报警任务
void AppMain_ScheduleDisplayTask(void);        // 调度显示任务
void AppMain_ScheduleMonitorTask(void);        // 调度监控任务

// 状态管理函数
void AppMain_ChangeState(SystemState_t new_state); // 改变主状态
SystemMainState_t AppMain_GetCurrentState(void);       // 获取当前主状态
SystemRunState_t AppMain_GetRunState(void);           // 获取运行状态
uint8_t AppMain_IsSystemReady(void);                  // 检查系统是否就绪

// 启动流程函数
void AppMain_HandleStartupInit(void);          // 处理启动初始化
void AppMain_HandleBuhlerLogo(void);           // 处理Buhler LOGO显示
void AppMain_HandleMinyerLogo(void);           // 处理Minyer LOGO显示
void AppMain_CompleteStartup(void);            // 完成启动流程

// 工作流程函数
void AppMain_HandleWorking(void);              // 处理工作状态
void AppMain_ProcessChannelControl(void);      // 处理通道控制
void AppMain_ProcessTemperatureControl(void);  // 处理温度控制
void AppMain_ProcessAlarmHandling(void);       // 处理报警管理
void AppMain_ProcessDisplayUpdate(void);       // 处理显示更新

// 错误处理函数
void AppMain_HandleError(void);                // 处理错误状态
void AppMain_RecoverFromError(void);           // 从错误状态恢复
uint8_t AppMain_CheckSystemHealth(void);       // 检查系统健康状态

// 工具函数
uint32_t AppMain_GetStateElapsedTime(void);    // 获取当前状态经过时间
void AppMain_UpdateStateTime(void);            // 更新状态时间
uint8_t AppMain_IsTaskTimeReached(uint32_t last_time, uint32_t period); // 检查任务时间是否到达

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H */ 


