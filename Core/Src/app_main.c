/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_main.c
  * @brief          : 应用主控制模块源文件
  ******************************************************************************
  * @attention
  *
  * 应用主控制模块实现：
  * 1. 系统状态机：启动→自检→工作→错误状态管理
  * 2. 启动流程：Buhler LOGO→Minyer LOGO→自检流程
  * 3. 工作流程：通道控制→温度管理→报警处理→显示更新
  * 4. 任务调度：各模块任务的时序协调和调用
  * 5. 错误处理：系统故障检测和恢复机制
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_main.h"

/* 内部状态变量 ---------------------------------------------------------------*/
static SystemRunState_t system_run_state = {
    .main_state = SYSTEM_STARTUP,
    .startup_substate = STARTUP_INIT,
    .working_substate = WORKING_MONITORING,
    .state_start_time = 0,
    .state_changed = 0,
    .system_ready = 0
};                                                      // 系统运行状态

static TaskScheduleState_t task_schedule_state = {
    .last_relay_task_time = 0,
    .last_temp_task_time = 0,
    .last_alarm_task_time = 0,
    .last_display_task_time = 0,
    .last_monitor_task_time = 0
};                                                      // 任务调度状态

/* 内部函数声明 ---------------------------------------------------------------*/
static void InitializeAllModules(void);                // 初始化所有模块
static void UpdateSystemState(void);                   // 更新系统状态
static void HandleChannelEnableSignals(void);          // 处理通道使能信号
static void HandleSystemMonitoring(void);              // 处理系统监控

/**
 * @brief 应用主控制初始化
 * @param None
 * @retval None
 */
void AppMain_Init(void)
{
    // 初始化系统运行状态
    system_run_state.main_state = SYSTEM_STARTUP;
    system_run_state.startup_substate = STARTUP_INIT;
    system_run_state.working_substate = WORKING_MONITORING;
    system_run_state.state_start_time = HAL_GetTick();
    system_run_state.state_changed = 1;
    system_run_state.system_ready = 0;
    
    // 初始化任务调度状态
    uint32_t current_time = HAL_GetTick();
    task_schedule_state.last_relay_task_time = current_time;
    task_schedule_state.last_temp_task_time = current_time;
    task_schedule_state.last_alarm_task_time = current_time;
    task_schedule_state.last_display_task_time = current_time;
    task_schedule_state.last_monitor_task_time = current_time;
    
    // 初始化所有功能模块
    InitializeAllModules();
}

/**
 * @brief 应用主控制运行（主循环）
 * @param None
 * @retval None
 */
void AppMain_Run(void)
{
    // 主状态机处理
    AppMain_StateMachine();
    
    // 任务调度器
    AppMain_TaskScheduler();
    
    // 主循环延时
    HAL_Delay(MAIN_LOOP_DELAY_MS);
}

/**
 * @brief 应用主控制复位
 * @param None
 * @retval None
 */
void AppMain_Reset(void)
{
    // 复位系统状态
    system_run_state.main_state = SYSTEM_STARTUP;
    system_run_state.startup_substate = STARTUP_INIT;
    system_run_state.working_substate = WORKING_MONITORING;
    system_run_state.state_start_time = HAL_GetTick();
    system_run_state.state_changed = 1;
    system_run_state.system_ready = 0;
    
    // 重新初始化所有模块
    InitializeAllModules();
}

/**
 * @brief 主状态机处理
 * @param None
 * @retval None
 */
void AppMain_StateMachine(void)
{
    // 更新系统状态
    UpdateSystemState();
    
    // 根据主状态执行对应处理
    switch (system_run_state.main_state)
    {
        case SYSTEM_STARTUP:
            AppMain_StartupStateMachine();
            break;
            
        case SYSTEM_SELFCHECK:
            // 运行系统自检
            if (SystemMonitor_RunSelfCheck() == SELFCHECK_COMPLETE)
            {
                // 自检完成，进入工作状态
                AppMain_ChangeState(STATE_STANDBY);
            }
            else if (SystemMonitor_RunSelfCheck() == SELFCHECK_FAILED)
            {
                // 自检失败，进入错误状态
                AppMain_ChangeState(STATE_ERROR);
            }
            break;
            
        case SYSTEM_WORKING:
            AppMain_WorkingStateMachine();
            break;
            
        case SYSTEM_ERROR:
            AppMain_ErrorStateMachine();
            break;
    }
}

/**
 * @brief 启动状态机处理
 * @param None
 * @retval None
 */
void AppMain_StartupStateMachine(void)
{
    switch (system_run_state.startup_substate)
    {
        case STARTUP_INIT:
            AppMain_HandleStartupInit();
            break;
            
        case STARTUP_BUHLER_LOGO:
            AppMain_HandleBuhlerLogo();
            break;
            
        case STARTUP_MINYER_LOGO:
            AppMain_HandleMinyerLogo();
            break;
            
        case STARTUP_COMPLETE:
            AppMain_CompleteStartup();
            break;
    }
}

/**
 * @brief 工作状态机处理
 * @param None
 * @retval None
 */
void AppMain_WorkingStateMachine(void)
{
    // 检查系统健康状态
    if (!AppMain_CheckSystemHealth())
    {
        AppMain_ChangeState(STATE_ERROR);
        return;
    }
    
    switch (system_run_state.working_substate)
    {
        case WORKING_MONITORING:
            // 系统监控处理
            HandleSystemMonitoring();
            system_run_state.working_substate = WORKING_PROCESSING;
            break;
            
        case WORKING_PROCESSING:
            // 业务逻辑处理
            AppMain_ProcessChannelControl();
            AppMain_ProcessTemperatureControl();
            AppMain_ProcessAlarmHandling();
            system_run_state.working_substate = WORKING_DISPLAY_UPDATE;
            break;
            
        case WORKING_DISPLAY_UPDATE:
            // 显示更新处理
            AppMain_ProcessDisplayUpdate();
            system_run_state.working_substate = WORKING_MONITORING;
            break;
    }
}

/**
 * @brief 错误状态机处理
 * @param None
 * @retval None
 */
void AppMain_ErrorStateMachine(void)
{
    // 处理错误状态
    AppMain_HandleError();
    
    // 尝试系统恢复
    AppMain_RecoverFromError();
}

/**
 * @brief 任务调度器
 * @param None
 * @retval None
 */
void AppMain_TaskScheduler(void)
{
    // 调度继电器任务
    AppMain_ScheduleRelayTask();
    
    // 调度温度任务
    AppMain_ScheduleTemperatureTask();
    
    // 调度报警任务
    AppMain_ScheduleAlarmTask();
    
    // 调度显示任务
    AppMain_ScheduleDisplayTask();
    
    // 调度监控任务
    AppMain_ScheduleMonitorTask();
}

/**
 * @brief 调度继电器任务
 * @param None
 * @retval None
 */
void AppMain_ScheduleRelayTask(void)
{
    if (AppMain_IsTaskTimeReached(task_schedule_state.last_relay_task_time, RELAY_TASK_PERIOD_MS))
    {
        RelayControl_Task();
        task_schedule_state.last_relay_task_time = HAL_GetTick();
    }
}

/**
 * @brief 调度温度任务
 * @param None
 * @retval None
 */
void AppMain_ScheduleTemperatureTask(void)
{
    if (AppMain_IsTaskTimeReached(task_schedule_state.last_temp_task_time, TEMP_TASK_PERIOD_MS))
    {
        TemperatureControl_Task();
        task_schedule_state.last_temp_task_time = HAL_GetTick();
    }
}

/**
 * @brief 调度报警任务
 * @param None
 * @retval None
 */
void AppMain_ScheduleAlarmTask(void)
{
    if (AppMain_IsTaskTimeReached(task_schedule_state.last_alarm_task_time, ALARM_TASK_PERIOD_MS))
    {
        AlarmManager_Task();
        task_schedule_state.last_alarm_task_time = HAL_GetTick();
    }
}

/**
 * @brief 调度显示任务
 * @param None
 * @retval None
 */
void AppMain_ScheduleDisplayTask(void)
{
    if (AppMain_IsTaskTimeReached(task_schedule_state.last_display_task_time, DISPLAY_TASK_PERIOD_MS))
    {
        OLEDDisplay_Task();
        task_schedule_state.last_display_task_time = HAL_GetTick();
    }
}

/**
 * @brief 调度监控任务
 * @param None
 * @retval None
 */
void AppMain_ScheduleMonitorTask(void)
{
    if (AppMain_IsTaskTimeReached(task_schedule_state.last_monitor_task_time, MONITOR_TASK_PERIOD_MS))
    {
        SystemMonitor_Task();
        task_schedule_state.last_monitor_task_time = HAL_GetTick();
    }
}

/**
 * @brief 改变主状态
 * @param new_state 新的状态
 * @retval None
 */
void AppMain_ChangeState(SystemState_t new_state)
{
    // 将SystemState_t转换为SystemMainState_t
    SystemMainState_t main_state;
    
    switch (new_state)
    {
        case STATE_STARTUP:
        case STATE_LOGO:
            main_state = SYSTEM_STARTUP;
            break;
        case STATE_SELFCHECK:
            main_state = SYSTEM_SELFCHECK;
            break;
        case STATE_STANDBY:
        case STATE_CH1_ON:
        case STATE_CH2_ON:
        case STATE_CH3_ON:
            main_state = SYSTEM_WORKING;
            break;
        case STATE_ERROR:
        default:
            // 错误状态特殊处理
            system_run_state.main_state = SYSTEM_WORKING; // 临时设为工作状态
            system_run_state.state_start_time = HAL_GetTick();
            system_run_state.state_changed = 1;
            // 激活错误处理逻辑
            return;
    }
    
    if (system_run_state.main_state != main_state)
    {
        system_run_state.main_state = main_state;
        system_run_state.state_start_time = HAL_GetTick();
        system_run_state.state_changed = 1;
    }
}

/**
 * @brief 获取当前主状态
 * @param None
 * @retval SystemMainState_t 当前主状态
 */
SystemMainState_t AppMain_GetCurrentState(void)
{
    return system_run_state.main_state;
}

/**
 * @brief 获取运行状态
 * @param None
 * @retval SystemRunState_t 系统运行状态结构体
 */
SystemRunState_t AppMain_GetRunState(void)
{
    return system_run_state;
}

/**
 * @brief 检查系统是否就绪
 * @param None
 * @retval uint8_t 1-系统就绪，0-系统未就绪
 */
uint8_t AppMain_IsSystemReady(void)
{
    return system_run_state.system_ready;
}

/**
 * @brief 处理启动初始化
 * @param None
 * @retval None
 */
void AppMain_HandleStartupInit(void)
{
    // 启动OLED显示初始化
    OLEDDisplay_Init();
    
    // 切换到显示Buhler LOGO
    system_run_state.startup_substate = STARTUP_BUHLER_LOGO;
    system_run_state.state_start_time = HAL_GetTick();
}

/**
 * @brief 处理Buhler LOGO显示
 * @param None
 * @retval None
 */
void AppMain_HandleBuhlerLogo(void)
{
    // 显示Buhler公司LOGO
    if (system_run_state.state_changed)
    {
        OLEDDisplay_ShowStartupLogo();
        system_run_state.state_changed = 0;
    }
    
    // 检查显示时间是否达到2秒
    if (AppMain_GetStateElapsedTime() >= BUHLER_LOGO_DISPLAY_MS)
    {
        // 切换到显示Minyer LOGO
        system_run_state.startup_substate = STARTUP_MINYER_LOGO;
        system_run_state.state_start_time = HAL_GetTick();
        system_run_state.state_changed = 1;
    }
}

/**
 * @brief 处理Minyer LOGO显示
 * @param None
 * @retval None
 */
void AppMain_HandleMinyerLogo(void)
{
    // 显示Minyer LOGO和进度条
    if (system_run_state.state_changed)
    {
        OLEDDisplay_ShowStartupLogo();
        system_run_state.state_changed = 0;
    }
    
    // 启动LOGO显示自动包含进度条功能
    OLEDDisplay_ShowStartupLogo();
    
    // 检查显示时间是否达到2秒
    uint32_t elapsed_time = AppMain_GetStateElapsedTime();
    if (elapsed_time >= MINYER_LOGO_DISPLAY_MS)
    {
        // 启动流程完成
        system_run_state.startup_substate = STARTUP_COMPLETE;
        system_run_state.state_changed = 1;
    }
}

/**
 * @brief 完成启动流程
 * @param None
 * @retval None
 */
void AppMain_CompleteStartup(void)
{
    // 启动流程完成，进入自检状态
    AppMain_ChangeState(STATE_SELFCHECK);
}

/**
 * @brief 处理工作状态
 * @param None
 * @retval None
 */
void AppMain_HandleWorking(void)
{
    // 工作状态下的常规处理
    // 这个函数在工作状态机中被分解为更小的处理函数
}

/**
 * @brief 处理通道控制
 * @param None
 * @retval None
 */
void AppMain_ProcessChannelControl(void)
{
    // 处理通道使能信号
    HandleChannelEnableSignals();
}

/**
 * @brief 处理温度控制
 * @param None
 * @retval None
 */
void AppMain_ProcessTemperatureControl(void)
{
    // 温度控制已在任务调度中处理，这里可以做额外的逻辑
    // 例如根据温度状态调整系统行为
}

/**
 * @brief 处理报警管理
 * @param None
 * @retval None
 */
void AppMain_ProcessAlarmHandling(void)
{
    // 报警管理已在任务调度中处理，这里可以做额外的逻辑
    // 例如根据报警状态调整系统行为
    
    // 检查是否有严重报警需要进入错误状态
    if (AlarmManager_HasAnyAlarm())
    {
        // 检查是否为系统级严重故障
        if (AlarmManager_IsAlarmActive(ALARM_N) || // 自检异常
            AlarmManager_IsAlarmActive(ALARM_O))   // 电源异常
        {
            // 进入错误状态
            AppMain_ChangeState(STATE_ERROR);
        }
    }
}

/**
 * @brief 处理显示更新
 * @param None
 * @retval None
 */
void AppMain_ProcessDisplayUpdate(void)
{
    // 显示更新已在任务调度中处理，这里可以做额外的逻辑
    // 例如根据系统状态更新显示内容
}

/**
 * @brief 处理错误状态
 * @param None
 * @retval None
 */
void AppMain_HandleError(void)
{
    // 在错误状态下，停止所有非必要操作
    // 只保持报警和显示功能
    
    // 确保报警输出激活
    AlarmManager_Process();
    
    // 更新错误显示
    OLEDDisplay_ShowAlarmInterface();
}

/**
 * @brief 从错误状态恢复
 * @param None
 * @retval None
 */
void AppMain_RecoverFromError(void)
{
    // 检查是否可以从错误状态恢复
    if (AppMain_CheckSystemHealth())
    {
        // 清除所有可清除的报警
        AlarmManager_ClearAllAlarms();
        
        // 重新进入工作状态
        AppMain_ChangeState(STATE_STANDBY);
    }
}

/**
 * @brief 检查系统健康状态
 * @param None
 * @retval uint8_t 1-系统健康，0-系统异常
 */
uint8_t AppMain_CheckSystemHealth(void)
{
    // 检查电源状态
    if (!SystemMonitor_CheckPowerStatus())
    {
        return 0; // 电源异常
    }
    
    // 检查系统监控状态
    if (!SystemMonitor_IsSystemNormal())
    {
        return 0; // 监控异常
    }
    
    // 检查是否有严重报警
    if (AlarmManager_IsAlarmActive(ALARM_N) || // 自检异常
        AlarmManager_IsAlarmActive(ALARM_O))   // 电源异常
    {
        return 0; // 严重报警
    }
    
    return 1; // 系统健康
}

/**
 * @brief 获取当前状态经过时间
 * @param None
 * @retval uint32_t 状态经过时间（毫秒）
 */
uint32_t AppMain_GetStateElapsedTime(void)
{
    return HAL_GetTick() - system_run_state.state_start_time;
}

/**
 * @brief 更新状态时间
 * @param None
 * @retval None
 */
void AppMain_UpdateStateTime(void)
{
    system_run_state.state_start_time = HAL_GetTick();
}

/**
 * @brief 检查任务时间是否到达
 * @param last_time 上次任务时间
 * @param period 任务周期
 * @retval uint8_t 1-时间到达，0-时间未到
 */
uint8_t AppMain_IsTaskTimeReached(uint32_t last_time, uint32_t period)
{
    uint32_t current_time = HAL_GetTick();
    return (current_time - last_time) >= period;
}

/* 内部函数实现 ---------------------------------------------------------------*/

/**
 * @brief 初始化所有模块
 * @param None
 * @retval None
 */
static void InitializeAllModules(void)
{
    // 按照依赖顺序初始化各模块
    
    // 1. 基础配置初始化（CubeMX已完成硬件初始化）
    
    // 2. 系统配置初始化
    // SystemConfig_Init(); // 如果需要的话
    
    // 3. 继电器控制初始化
    RelayControl_Init();
    
    // 4. 温度控制初始化
    TemperatureControl_Init();
    
    // 5. 报警管理初始化
    AlarmManager_Init();
    
    // 6. OLED显示初始化（在启动流程中完成）
    // OLEDDisplay_Init();
    
    // 7. 系统监控初始化
    SystemMonitor_Init();
}

/**
 * @brief 更新系统状态
 * @param None
 * @retval None
 */
static void UpdateSystemState(void)
{
    // 清除状态改变标志（在状态处理完成后）
    if (system_run_state.state_changed)
    {
        // 状态改变标志会在具体的状态处理函数中清除
    }
}

/**
 * @brief 处理通道使能信号
 * @param None
 * @retval None
 */
static void HandleChannelEnableSignals(void)
{
    // 读取使能信号状态
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // 检查使能信号冲突
    uint8_t active_count = k1_en + k2_en + k3_en;
    if (active_count > 1)
    {
        // 多路同时激活，触发A类异常
        AlarmManager_SetAlarm(ALARM_A);
        return;
    }
    
    // 根据使能信号控制对应通道
    if (k1_en)
    {
        RelayControl_TurnOnChannel1();
    }
    else if (k2_en)
    {
        RelayControl_TurnOnChannel2();
    }
    else if (k3_en)
    {
        RelayControl_TurnOnChannel3();
    }
    else
    {
        // 所有通道都禁用
        RelayControl_TurnOffChannel1();
        RelayControl_TurnOffChannel2();
        RelayControl_TurnOffChannel3();
    }
}

/**
 * @brief 处理系统监控
 * @param None
 * @retval None
 */
static void HandleSystemMonitoring(void)
{
    // 系统监控主要在SystemMonitor_Task中处理
    // 这里可以做一些应用层的监控逻辑
    
    // 更新系统监控
    SystemMonitor_UpdateMonitoring();
} 

