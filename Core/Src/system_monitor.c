/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_monitor.c
  * @brief          : 系统监控模块源文件
  ******************************************************************************
  * @attention
  *
  * 系统监控模块实现：
  * 1. 四步自检流程：识别期望状态→继电器检查纠错→接触器检查纠错→温度检测
  * 2. 实时状态监控和主动纠错机制
  * 3. 电源监控和异常处理
  * 4. 系统状态验证和管理
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "system_monitor.h"

/* 内部状态变量 ---------------------------------------------------------------*/
static SelfCheckState_t selfcheck_state = SELFCHECK_IDLE;      // 自检状态
static SelfCheckResult_t selfcheck_result = {0};               // 自检结果
static SystemStatus_t system_status = {0};                     // 系统状态
static uint8_t correction_retry_count = 0;                     // 纠错重试次数
static uint32_t last_monitor_time = 0;                         // 上次监控时间

/* 内部函数声明 ---------------------------------------------------------------*/
static uint8_t CheckEnableSignalsState(void);                  // 检查使能信号状态
static uint8_t CheckRelayStatusSignals(void);                  // 检查继电器状态信号
static uint8_t CheckContactorStatusSignals(void);              // 检查接触器状态信号
static uint8_t CheckTemperatureSafety(void);                   // 检查温度安全
static uint8_t CorrectSingleRelay(uint8_t channel, uint8_t target_state); // 纠正单个继电器
static void UpdateSelfCheckProgress(uint8_t step);             // 更新自检进度
static SystemState_t DetermineExpectedState(void);             // 确定期望状态
static uint8_t ValidateStateConsistency(void);                 // 验证状态一致性

/**
 * @brief 系统监控初始化
 * @param None
 * @retval None
 */
void SystemMonitor_Init(void)
{
    // 初始化自检状态
    selfcheck_state = SELFCHECK_IDLE;
    
    // 清空自检结果
    selfcheck_result.step1_result = 0;
    selfcheck_result.step2_result = 0;
    selfcheck_result.step3_result = 0;
    selfcheck_result.step4_result = 0;
    selfcheck_result.overall_result = 0;
    selfcheck_result.progress = 0;
    
    // 初始化系统状态
    system_status.expected_state = SYSTEM_ALL_OFF;
    system_status.current_state = SYSTEM_ALL_OFF;
    system_status.power_status = 1; // 假设电源正常
    system_status.correction_count = 0;
    system_status.monitor_state = MONITOR_NORMAL;
    
    // 重置重试计数
    correction_retry_count = 0;
    
    // 记录初始化时间
    last_monitor_time = HAL_GetTick();
}

/**
 * @brief 运行系统自检流程
 * @param None
 * @retval SelfCheckState_t 当前自检状态
 */
SelfCheckState_t SystemMonitor_RunSelfCheck(void)
{
    static uint32_t step_start_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    switch (selfcheck_state)
    {
        case SELFCHECK_IDLE:
            // 开始自检流程
            selfcheck_state = SELFCHECK_STEP1;
            step_start_time = current_time;
            UpdateSelfCheckProgress(1);
            break;
            
        case SELFCHECK_STEP1:
            // 第一步：识别期望状态
            system_status.expected_state = DetermineExpectedState();
            
            // 检查使能信号是否有冲突
            if (CheckEnableSignalsState())
            {
                selfcheck_result.step1_result = 1;
                selfcheck_state = SELFCHECK_STEP2;
                UpdateSelfCheckProgress(25);
            }
            else
            {
                // A类异常：使能信号冲突
                AlarmManager_SetAlarm(ALARM_A);
                selfcheck_result.step1_result = 0;
                selfcheck_state = SELFCHECK_FAILED;
            }
            break;
            
        case SELFCHECK_STEP2:
            // 第二步：继电器状态检查与纠错
            if (CheckRelayStatusSignals())
            {
                selfcheck_result.step2_result = 1;
                selfcheck_state = SELFCHECK_STEP3;
                UpdateSelfCheckProgress(50);
            }
            else
            {
                // 尝试纠错
                if (correction_retry_count < SELFCHECK_MAX_RETRY)
                {
                    if (SystemMonitor_CorrectRelayStates())
                    {
                        correction_retry_count++;
                        HAL_Delay(CORRECTION_DELAY_MS); // 等待纠错完成
                    }
                    else
                    {
                        selfcheck_result.step2_result = 0;
                        selfcheck_state = SELFCHECK_FAILED;
                    }
                }
                else
                {
                    selfcheck_result.step2_result = 0;
                    selfcheck_state = SELFCHECK_FAILED;
                }
            }
            break;
            
        case SELFCHECK_STEP3:
            // 第三步：接触器状态检查与纠错
            if (CheckContactorStatusSignals())
            {
                selfcheck_result.step3_result = 1;
                selfcheck_state = SELFCHECK_STEP4;
                UpdateSelfCheckProgress(75);
            }
            else
            {
                // 尝试纠错
                if (correction_retry_count < SELFCHECK_MAX_RETRY)
                {
                    if (SystemMonitor_CorrectContactorStates())
                    {
                        correction_retry_count++;
                        HAL_Delay(CORRECTION_DELAY_MS); // 等待纠错完成
                    }
                    else
                    {
                        selfcheck_result.step3_result = 0;
                        selfcheck_state = SELFCHECK_FAILED;
                    }
                }
                else
                {
                    selfcheck_result.step3_result = 0;
                    selfcheck_state = SELFCHECK_FAILED;
                }
            }
            break;
            
        case SELFCHECK_STEP4:
            // 第四步：温度安全检测
            if (CheckTemperatureSafety())
            {
                selfcheck_result.step4_result = 1;
                selfcheck_result.overall_result = 1;
                selfcheck_state = SELFCHECK_COMPLETE;
                UpdateSelfCheckProgress(100);
            }
            else
            {
                selfcheck_result.step4_result = 0;
                selfcheck_state = SELFCHECK_FAILED;
            }
            break;
            
        case SELFCHECK_COMPLETE:
            // 自检完成，保持状态
            break;
            
        case SELFCHECK_FAILED:
            // 自检失败，触发N类异常
            AlarmManager_SetAlarm(ALARM_N);
            selfcheck_result.overall_result = 0;
            break;
    }
    
    return selfcheck_state;
}

/**
 * @brief 更新状态监控
 * @param None
 * @retval None
 */
void SystemMonitor_UpdateMonitoring(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // 检查监控更新周期
    if (current_time - last_monitor_time < MONITOR_UPDATE_MS)
    {
        return;
    }
    
    last_monitor_time = current_time;
    
    // 检查电源状态
    system_status.power_status = SystemMonitor_CheckPowerStatus();
    if (!system_status.power_status)
    {
        SystemMonitor_HandlePowerError();
        return;
    }
    
    // 更新期望状态
    system_status.expected_state = DetermineExpectedState();
    
    // 检查所有状态是否一致
    if (!ValidateStateConsistency())
    {
        // 状态不一致，尝试纠错
        system_status.monitor_state = MONITOR_CORRECTING;
        
        if (SystemMonitor_CorrectSystem())
        {
            system_status.monitor_state = MONITOR_NORMAL;
            system_status.correction_count++;
        }
        else
        {
            system_status.monitor_state = MONITOR_ERROR;
        }
    }
    else
    {
        system_status.monitor_state = MONITOR_NORMAL;
    }
}

/**
 * @brief 检查电源状态
 * @param None
 * @retval uint8_t 1-电源正常，0-电源异常
 */
uint8_t SystemMonitor_CheckPowerStatus(void)
{
    // 读取DC_CTRL引脚状态（PB5，低电平为true）
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin);
    
    // 低电平表示电源正常
    return (pin_state == GPIO_PIN_RESET) ? 1 : 0;
}

/**
 * @brief 处理电源异常
 * @param None
 * @retval None
 */
void SystemMonitor_HandlePowerError(void)
{
    // 触发O类异常
    AlarmManager_SetAlarm(ALARM_O);
    
    // 设置系统状态为错误
    system_status.monitor_state = MONITOR_ERROR;
    
    // 停止所有继电器操作（通过不执行任何控制命令实现）
}

/**
 * @brief 主动纠错系统
 * @param None
 * @retval uint8_t 1-纠错成功，0-纠错失败
 */
uint8_t SystemMonitor_CorrectSystem(void)
{
    uint8_t result = 1;
    
    // 纠正继电器状态
    if (!SystemMonitor_CorrectRelayStates())
    {
        result = 0;
    }
    
    // 延时等待继电器状态稳定
    HAL_Delay(CORRECTION_DELAY_MS);
    
    // 纠正接触器状态
    if (!SystemMonitor_CorrectContactorStates())
    {
        result = 0;
    }
    
    return result;
}

/**
 * @brief 纠正继电器状态
 * @param None
 * @retval uint8_t 1-纠错成功，0-纠错失败
 */
uint8_t SystemMonitor_CorrectRelayStates(void)
{
    uint8_t success = 1;
    SystemState_t expected = system_status.expected_state;
    
    // 根据期望状态纠正各通道继电器
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            // 通道1应该开启，其他关闭
            if (!CorrectSingleRelay(1, 1)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
            
        case SYSTEM_CHANNEL2_ON:
            // 通道2应该开启，其他关闭
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 1)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
            
        case SYSTEM_CHANNEL3_ON:
            // 通道3应该开启，其他关闭
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 1)) success = 0;
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            // 所有通道都应该关闭
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
    }
    
    return success;
}

/**
 * @brief 纠正接触器状态
 * @param None
 * @retval uint8_t 1-纠错成功，0-纠错失败
 */
uint8_t SystemMonitor_CorrectContactorStates(void)
{
    // 接触器状态通过继电器控制，所以这里主要是验证
    // 如果接触器状态异常，通过控制对应的继电器来纠正
    uint8_t success = 1;
    SystemState_t expected = system_status.expected_state;
    
    // 检查并纠正接触器状态
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            // 检查SW1_STA是否为高电平
            if (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) != GPIO_PIN_SET)
            {
                // 尝试重新激活通道1
                RelayControl_OpenChannel(CHANNEL_1);
            }
            break;
            
        case SYSTEM_CHANNEL2_ON:
            // 检查SW2_STA是否为高电平
            if (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) != GPIO_PIN_SET)
            {
                // 尝试重新激活通道2
                RelayControl_OpenChannel(CHANNEL_2);
            }
            break;
            
        case SYSTEM_CHANNEL3_ON:
            // 检查SW3_STA是否为高电平
            if (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) != GPIO_PIN_SET)
            {
                // 尝试重新激活通道3
                RelayControl_OpenChannel(CHANNEL_3);
            }
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            // 所有接触器都应该关闭，如果有开启的则关闭对应通道
            if (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) == GPIO_PIN_SET)
            {
                RelayControl_CloseChannel(CHANNEL_1);
            }
            if (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) == GPIO_PIN_SET)
            {
                RelayControl_CloseChannel(CHANNEL_2);
            }
            if (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) == GPIO_PIN_SET)
            {
                RelayControl_CloseChannel(CHANNEL_3);
            }
            break;
    }
    
    return success;
}

/**
 * @brief 检查期望状态
 * @param None
 * @retval uint8_t 1-状态正确，0-状态错误
 */
uint8_t SystemMonitor_CheckExpectedState(void)
{
    SystemState_t expected = DetermineExpectedState();
    return (expected != SYSTEM_ERROR) ? 1 : 0;
}

/**
 * @brief 检查所有状态
 * @param None
 * @retval uint8_t 1-所有状态正确，0-状态异常
 */
uint8_t SystemMonitor_CheckAllStates(void)
{
    // 检查使能信号
    if (!CheckEnableSignalsState()) return 0;
    
    // 检查继电器状态
    if (!CheckRelayStatusSignals()) return 0;
    
    // 检查接触器状态
    if (!CheckContactorStatusSignals()) return 0;
    
    // 检查温度安全
    if (!CheckTemperatureSafety()) return 0;
    
    return 1;
}

/**
 * @brief 验证系统状态
 * @param None
 * @retval uint8_t 1-状态有效，0-状态无效
 */
uint8_t SystemMonitor_ValidateSystemState(void)
{
    return ValidateStateConsistency();
}

/**
 * @brief 获取自检进度
 * @param None
 * @retval uint8_t 自检进度（0-100）
 */
uint8_t SystemMonitor_GetSelfCheckProgress(void)
{
    return selfcheck_result.progress;
}

/**
 * @brief 获取自检结果
 * @param None
 * @retval SelfCheckResult_t 自检结果结构体
 */
SelfCheckResult_t SystemMonitor_GetSelfCheckResult(void)
{
    return selfcheck_result;
}

/**
 * @brief 获取系统状态
 * @param None
 * @retval SystemStatus_t 系统状态结构体
 */
SystemStatus_t SystemMonitor_GetSystemStatus(void)
{
    return system_status;
}

/**
 * @brief 检查系统是否正常
 * @param None
 * @retval uint8_t 1-系统正常，0-系统异常
 */
uint8_t SystemMonitor_IsSystemNormal(void)
{
    return (system_status.monitor_state == MONITOR_NORMAL) ? 1 : 0;
}

/**
 * @brief 系统监控任务处理
 * @param None
 * @retval None
 */
void SystemMonitor_Task(void)
{
    // 更新状态监控
    SystemMonitor_UpdateMonitoring();
}

/* 内部函数实现 ---------------------------------------------------------------*/

/**
 * @brief 检查使能信号状态
 * @param None
 * @retval uint8_t 1-状态正确，0-状态错误（冲突）
 */
static uint8_t CheckEnableSignalsState(void)
{
    // 读取三个使能信号（低电平有效）
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    uint8_t active_count = k1_en + k2_en + k3_en;
    
    // 检查是否有冲突（多路同时激活）
    if (active_count > 1)
    {
        return 0; // 冲突
    }
    
    return 1; // 正常
}

/**
 * @brief 检查继电器状态信号
 * @param None
 * @retval uint8_t 1-状态正确，0-状态异常
 */
static uint8_t CheckRelayStatusSignals(void)
{
    SystemState_t expected = system_status.expected_state;
    
    // 读取所有继电器状态（高电平有效）
    uint8_t k1_1_sta = (HAL_GPIO_ReadPin(K1_1_STA_GPIO_Port, K1_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k1_2_sta = (HAL_GPIO_ReadPin(K1_2_STA_GPIO_Port, K1_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k2_1_sta = (HAL_GPIO_ReadPin(K2_1_STA_GPIO_Port, K2_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k2_2_sta = (HAL_GPIO_ReadPin(K2_2_STA_GPIO_Port, K2_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k3_1_sta = (HAL_GPIO_ReadPin(K3_1_STA_GPIO_Port, K3_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k3_2_sta = (HAL_GPIO_ReadPin(K3_2_STA_GPIO_Port, K3_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    
    // 根据期望状态检查继电器状态
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            if (k1_1_sta && k1_2_sta && !k2_1_sta && !k2_2_sta && !k3_1_sta && !k3_2_sta)
                return 1;
            break;
            
        case SYSTEM_CHANNEL2_ON:
            if (!k1_1_sta && !k1_2_sta && k2_1_sta && k2_2_sta && !k3_1_sta && !k3_2_sta)
                return 1;
            break;
            
        case SYSTEM_CHANNEL3_ON:
            if (!k1_1_sta && !k1_2_sta && !k2_1_sta && !k2_2_sta && k3_1_sta && k3_2_sta)
                return 1;
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            if (!k1_1_sta && !k1_2_sta && !k2_1_sta && !k2_2_sta && !k3_1_sta && !k3_2_sta)
                return 1;
            break;
    }
    
    // 如果状态不匹配，触发对应的异常
    if (k1_1_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_B);
    if (k2_1_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_C);
    if (k3_1_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_D);
    if (k1_2_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_E);
    if (k2_2_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_F);
    if (k3_2_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_G);
    
    return 0;
}

/**
 * @brief 检查接触器状态信号
 * @param None
 * @retval uint8_t 1-状态正确，0-状态异常
 */
static uint8_t CheckContactorStatusSignals(void)
{
    SystemState_t expected = system_status.expected_state;
    
    // 读取所有接触器状态（高电平有效）
    uint8_t sw1_sta = (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t sw2_sta = (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t sw3_sta = (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    
    // 根据期望状态检查接触器状态
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            if (sw1_sta && !sw2_sta && !sw3_sta)
                return 1;
            break;
            
        case SYSTEM_CHANNEL2_ON:
            if (!sw1_sta && sw2_sta && !sw3_sta)
                return 1;
            break;
            
        case SYSTEM_CHANNEL3_ON:
            if (!sw1_sta && !sw2_sta && sw3_sta)
                return 1;
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            if (!sw1_sta && !sw2_sta && !sw3_sta)
                return 1;
            break;
    }
    
    // 如果状态不匹配，触发对应的异常
    if (sw1_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_H);
    if (sw2_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_I);
    if (sw3_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_J);
    
    return 0;
}

/**
 * @brief 检查温度安全
 * @param None
 * @retval uint8_t 1-温度安全，0-温度异常
 */
static uint8_t CheckTemperatureSafety(void)
{
    float temp1, temp2, temp3;
    
    // 获取三路温度
    TemperatureControl_GetTemperatures(&temp1, &temp2, &temp3);
    
    // 检查是否超过60℃
    if (temp1 >= TEMP_ALARM_THRESHOLD)
    {
        AlarmManager_SetAlarm(ALARM_K);
        return 0;
    }
    
    if (temp2 >= TEMP_ALARM_THRESHOLD)
    {
        AlarmManager_SetAlarm(ALARM_L);
        return 0;
    }
    
    if (temp3 >= TEMP_ALARM_THRESHOLD)
    {
        AlarmManager_SetAlarm(ALARM_M);
        return 0;
    }
    
    return 1; // 所有温度都安全
}

/**
 * @brief 纠正单个继电器状态
 * @param channel 通道号（1-3）
 * @param target_state 目标状态（1-开启，0-关闭）
 * @retval uint8_t 1-纠错成功，0-纠错失败
 */
static uint8_t CorrectSingleRelay(uint8_t channel, uint8_t target_state)
{
    ChannelType_t ch = (ChannelType_t)(channel - 1); // 转换为枚举类型
    
    if (target_state)
    {
        // 开启通道
        return RelayControl_OpenChannel(ch);
    }
    else
    {
        // 关闭通道
        return RelayControl_CloseChannel(ch);
    }
}

/**
 * @brief 更新自检进度
 * @param step 当前步骤进度值
 * @retval None
 */
static void UpdateSelfCheckProgress(uint8_t step)
{
    selfcheck_result.progress = step;
}

/**
 * @brief 确定期望状态
 * @param None
 * @retval SystemState_t 期望的系统状态
 */
static SystemState_t DetermineExpectedState(void)
{
    // 读取使能信号（低电平有效）
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // 根据使能信号组合确定期望状态
    if (!k1_en && !k2_en && !k3_en)
    {
        return SYSTEM_ALL_OFF; // 全部关断
    }
    else if (k1_en && !k2_en && !k3_en)
    {
        return SYSTEM_CHANNEL1_ON; // 通道1开启
    }
    else if (!k1_en && k2_en && !k3_en)
    {
        return SYSTEM_CHANNEL2_ON; // 通道2开启
    }
    else if (!k1_en && !k2_en && k3_en)
    {
        return SYSTEM_CHANNEL3_ON; // 通道3开启
    }
    else
    {
        return SYSTEM_ERROR; // 错误状态（多路同时激活）
    }
}

/**
 * @brief 验证状态一致性
 * @param None
 * @retval uint8_t 1-状态一致，0-状态不一致
 */
static uint8_t ValidateStateConsistency(void)
{
    // 检查期望状态是否有效
    if (system_status.expected_state == SYSTEM_ERROR)
    {
        return 0;
    }
    
    // 检查继电器状态
    if (!CheckRelayStatusSignals())
    {
        return 0;
    }
    
    // 检查接触器状态
    if (!CheckContactorStatusSignals())
    {
        return 0;
    }
    
    return 1; // 状态一致
} 


