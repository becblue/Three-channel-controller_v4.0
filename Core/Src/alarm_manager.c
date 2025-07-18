/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_manager.c
  * @brief          : 报警管理系统源文件
  ******************************************************************************
  */
/* USER CODE END Header */

#include "alarm_manager.h"
#include "relay_control.h"
#include "temperature_control.h"

/* 私有变量 -------------------------------------------------------------------*/
static uint16_t active_alarms = 0;            // 当前活跃的异常标志位图（位0-14对应A-O类异常）
static BeepMode_t current_beep_mode = BEEP_MODE_OFF;  // 当前蜂鸣器模式
static uint32_t beep_timer = 0;               // 蜂鸣器时间计数
static uint8_t beep_state = 0;                // 蜂鸣器当前状态（0=高电平，1=低电平）

/* 私有函数声明 ---------------------------------------------------------------*/
static void AlarmManager_UpdateBeepMode(void);
static void AlarmManager_ProcessBeep(void);
static uint8_t AlarmManager_CheckTruthTableMatch(void);

/**
  * @brief  报警管理器初始化
  * @param  None
  * @retval None
  */
void AlarmManager_Init(void)
{
    // 清除所有异常标志
    active_alarms = 0;
    g_alarm = ALARM_NONE;
    
    // 初始化ALARM输出为高电平（无报警）
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);
    
    // 初始化蜂鸣器为高电平（静音）
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    
    current_beep_mode = BEEP_MODE_OFF;
    beep_timer = 0;
    beep_state = 0;
}

/**
  * @brief  设置异常报警
  * @param  alarm_type: 异常类型
  * @retval None
  */
void AlarmManager_SetAlarm(AlarmType_t alarm_type)
{
    if (alarm_type >= ALARM_A && alarm_type <= ALARM_O)
    {
        // 设置对应的异常标志位
        uint16_t alarm_bit = 1 << (alarm_type - ALARM_A);
        active_alarms |= alarm_bit;
        
        // 更新全局异常状态
        g_alarm = alarm_type;
        
        // 启用ALARM输出（低电平）
        AlarmManager_SetAlarmOutput(1);
        
        // 更新蜂鸣器模式
        AlarmManager_UpdateBeepMode();
    }
}

/**
  * @brief  清除异常报警
  * @param  alarm_type: 异常类型
  * @retval None
  */
void AlarmManager_ClearAlarm(AlarmType_t alarm_type)
{
    if (alarm_type >= ALARM_A && alarm_type <= ALARM_O)
    {
        // 清除对应的异常标志位
        uint16_t alarm_bit = 1 << (alarm_type - ALARM_A);
        active_alarms &= ~alarm_bit;
        
        // 如果没有任何异常，更新全局状态
        if (active_alarms == 0)
        {
            g_alarm = ALARM_NONE;
            AlarmManager_SetAlarmOutput(0);  // 关闭ALARM输出
        }
        
        // 更新蜂鸣器模式
        AlarmManager_UpdateBeepMode();
    }
}

/**
  * @brief  清除所有异常
  * @param  None
  * @retval None
  */
void AlarmManager_ClearAllAlarms(void)
{
    active_alarms = 0;
    g_alarm = ALARM_NONE;
    AlarmManager_SetAlarmOutput(0);
    AlarmManager_SetBeepMode(BEEP_MODE_OFF);
}

/**
  * @brief  检查特定异常是否激活
  * @param  alarm_type: 异常类型
  * @retval 1=激活，0=未激活
  */
uint8_t AlarmManager_IsAlarmActive(AlarmType_t alarm_type)
{
    if (alarm_type >= ALARM_A && alarm_type <= ALARM_O)
    {
        uint16_t alarm_bit = 1 << (alarm_type - ALARM_A);
        return (active_alarms & alarm_bit) ? 1 : 0;
    }
    return 0;
}

/**
  * @brief  检查是否有任何异常
  * @param  None
  * @retval 1=有异常，0=无异常
  */
uint8_t AlarmManager_HasAnyAlarm(void)
{
    return (active_alarms != 0) ? 1 : 0;
}

/**
  * @brief  控制ALARM输出
  * @param  enable: 1=启用报警输出（低电平），0=关闭报警输出（高电平）
  * @retval None
  */
void AlarmManager_SetAlarmOutput(uint8_t enable)
{
    if (enable)
    {
        HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_RESET);  // 低电平 - 报警
    }
    else
    {
        HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);    // 高电平 - 正常
    }
}

/**
  * @brief  设置蜂鸣器模式
  * @param  mode: 蜂鸣器模式
  * @retval None
  */
void AlarmManager_SetBeepMode(BeepMode_t mode)
{
    current_beep_mode = mode;
    beep_timer = HAL_GetTick();
    
    if (mode == BEEP_MODE_OFF)
    {
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);  // 高电平 - 静音
        beep_state = 0;
    }
    else if (mode == BEEP_MODE_CONTINUOUS)
    {
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);  // 低电平 - 持续响
        beep_state = 1;
    }
}

/**
  * @brief  根据当前异常更新蜂鸣器模式
  * @param  None
  * @retval None
  */
static void AlarmManager_UpdateBeepMode(void)
{
    if (active_alarms == 0)
    {
        AlarmManager_SetBeepMode(BEEP_MODE_OFF);
        return;
    }
    
    // 检查K、L、M类异常（优先级最高）
    if (AlarmManager_IsAlarmActive(ALARM_K) || 
        AlarmManager_IsAlarmActive(ALARM_L) || 
        AlarmManager_IsAlarmActive(ALARM_M))
    {
        AlarmManager_SetBeepMode(BEEP_MODE_CONTINUOUS);
    }
    // 检查A、N类异常
    else if (AlarmManager_IsAlarmActive(ALARM_A) || 
             AlarmManager_IsAlarmActive(ALARM_N))
    {
        AlarmManager_SetBeepMode(BEEP_MODE_1S_PULSE);
    }
    // 检查B~J类异常
    else
    {
        AlarmManager_SetBeepMode(BEEP_MODE_50MS_PULSE);
    }
}

/**
  * @brief  处理蜂鸣器脉冲输出
  * @param  None
  * @retval None
  */
static void AlarmManager_ProcessBeep(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t interval = 0;
    
    if (current_beep_mode == BEEP_MODE_OFF || current_beep_mode == BEEP_MODE_CONTINUOUS)
    {
        return;  // 不需要处理脉冲
    }
    
    // 确定脉冲间隔
    if (current_beep_mode == BEEP_MODE_1S_PULSE)
    {
        interval = 1000;  // 1秒
    }
    else if (current_beep_mode == BEEP_MODE_50MS_PULSE)
    {
        interval = 50;    // 50ms
    }
    
    // 检查是否到了切换时间
    if (current_time - beep_timer >= interval)
    {
        beep_timer = current_time;
        beep_state = !beep_state;  // 切换状态
        
        if (beep_state)
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);  // 低电平 - 响
        }
        else
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);    // 高电平 - 静音
        }
    }
}

/**
  * @brief  检查当前状态是否符合真值表
  * @param  None
  * @retval 1=符合，0=不符合
  */
static uint8_t AlarmManager_CheckTruthTableMatch(void)
{
    // 读取使能信号
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // 读取继电器状态
    uint8_t k1_1_sta = RelayControl_GetRelayStatus(CHANNEL_1, 1);
    uint8_t k1_2_sta = RelayControl_GetRelayStatus(CHANNEL_1, 2);
    uint8_t k2_1_sta = RelayControl_GetRelayStatus(CHANNEL_2, 1);
    uint8_t k2_2_sta = RelayControl_GetRelayStatus(CHANNEL_2, 2);
    uint8_t k3_1_sta = RelayControl_GetRelayStatus(CHANNEL_3, 1);
    uint8_t k3_2_sta = RelayControl_GetRelayStatus(CHANNEL_3, 2);
    
    // 读取接触器状态
    uint8_t sw1_sta = RelayControl_GetContactorStatus(CHANNEL_1);
    uint8_t sw2_sta = RelayControl_GetContactorStatus(CHANNEL_2);
    uint8_t sw3_sta = RelayControl_GetContactorStatus(CHANNEL_3);
    
    // 检查真值表匹配
    // Channel_1打开: K1_EN=0,K2_EN=1,K3_EN=1,K1_1_STA=1,K1_2_STA=1,其他=0
    if (k1_en == 1 && k2_en == 0 && k3_en == 0)
    {
        return (k1_1_sta == 1 && k1_2_sta == 1 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 1 && sw2_sta == 0 && sw3_sta == 0);
    }
    // Channel_2打开: K1_EN=1,K2_EN=0,K3_EN=1,K2_1_STA=1,K2_2_STA=1,其他=0
    else if (k1_en == 0 && k2_en == 1 && k3_en == 0)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 1 && k2_2_sta == 1 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 0 && sw2_sta == 1 && sw3_sta == 0);
    }
    // Channel_3打开: K1_EN=1,K2_EN=1,K3_EN=0,K3_1_STA=1,K3_2_STA=1,其他=0
    else if (k1_en == 0 && k2_en == 0 && k3_en == 1)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 1 && k3_2_sta == 1 && sw1_sta == 0 && sw2_sta == 0 && sw3_sta == 1);
    }
    // 全部关闭: K1_EN=1,K2_EN=1,K3_EN=1,所有状态=0
    else if (k1_en == 0 && k2_en == 0 && k3_en == 0)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 0 && sw2_sta == 0 && sw3_sta == 0);
    }
    
    return 0;  // 不符合任何有效状态
}

/**
  * @brief  检查报警解除条件
  * @param  alarm_type: 异常类型
  * @retval 1=可以解除，0=不能解除
  */
uint8_t AlarmManager_CheckAlarmClearCondition(AlarmType_t alarm_type)
{
    switch (alarm_type)
    {
        case ALARM_A:  // 使能冲突异常
        {
            // 读取使能信号
            uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            
            // 只能有一路低电平或全部高电平
            uint8_t low_count = k1_en + k2_en + k3_en;
            return (low_count <= 1) ? 1 : 0;
        }
        
        case ALARM_B:  case ALARM_C:  case ALARM_D:  case ALARM_E:  
        case ALARM_F:  case ALARM_G:  case ALARM_H:  case ALARM_I:  
        case ALARM_J:  case ALARM_N:  // 状态异常和自检异常
        {
            // 检查是否符合真值表
            return AlarmManager_CheckTruthTableMatch();
        }
        
        case ALARM_K:  // NTC_1温度异常
        {
            float temp = TemperatureControl_GetTemperature(0);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58℃以下
        }
        
        case ALARM_L:  // NTC_2温度异常
        {
            float temp = TemperatureControl_GetTemperature(1);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58℃以下
        }
        
        case ALARM_M:  // NTC_3温度异常
        {
            float temp = TemperatureControl_GetTemperature(2);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58℃以下
        }
        
        case ALARM_O:  // 电源监控异常
        {
            // 检查DC_CTRL信号是否恢复正常
            return (HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin) == GPIO_PIN_SET) ? 1 : 0;
        }
        
        default:
            return 0;
    }
}

/**
  * @brief  报警管理主处理函数
  * @param  None
  * @retval None
  */
void AlarmManager_Process(void)
{
    // 处理蜂鸣器脉冲输出
    AlarmManager_ProcessBeep();
    
    // 检查当前活跃异常的解除条件
    for (AlarmType_t alarm = ALARM_A; alarm <= ALARM_O; alarm++)
    {
        if (AlarmManager_IsAlarmActive(alarm))
        {
            if (AlarmManager_CheckAlarmClearCondition(alarm))
            {
                AlarmManager_ClearAlarm(alarm);
            }
        }
    }
}

/**
 * @brief 获取当前活跃报警数量
 * @return uint8_t 活跃报警数量
 */
uint8_t AlarmManager_GetAlarmCount(void)
{
    uint8_t count = 0;
    for (AlarmType_t alarm = ALARM_A; alarm <= ALARM_O; alarm++)
    {
        if (AlarmManager_IsAlarmActive(alarm))
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief 获取活跃报警列表
 * @param alarms 报警数组
 * @param max_count 最大数量
 * @return uint8_t 实际获取的报警数量
 */
uint8_t AlarmManager_GetActiveAlarms(AlarmType_t *alarms, uint8_t max_count)
{
    uint8_t count = 0;
    for (AlarmType_t alarm = ALARM_A; alarm <= ALARM_O && count < max_count; alarm++)
    {
        if (AlarmManager_IsAlarmActive(alarm))
        {
            alarms[count] = alarm;
            count++;
        }
    }
    return count;
}

/**
 * @brief 更新报警输出状态
 */
void AlarmManager_UpdateAlarmOutput(void)
{
    // 如果有任何报警，拉低PB4（低电平有效）
    if (AlarmManager_GetAlarmCount() > 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // 报警输出：低电平
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);   // 正常状态：高电平
    }
}

/**
 * @brief 更新蜂鸣器状态
 */
void AlarmManager_UpdateBeeper(void)
{
    static uint32_t last_beep_time = 0;
    static uint8_t beep_state = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 检查报警优先级和模式
    uint8_t alarm_count = AlarmManager_GetAlarmCount();
    if (alarm_count == 0)
    {
        // 无报警，关闭蜂鸣器
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        return;
    }
    
    // 获取最高优先级报警类型
    AlarmType_t active_alarms[5];
    AlarmManager_GetActiveAlarms(active_alarms, 5);
    AlarmType_t highest_alarm = active_alarms[0];
    
    uint32_t beep_interval;
    if (highest_alarm == ALARM_A || highest_alarm == ALARM_N)
    {
        beep_interval = 1000; // A、N类：1秒间隔
    }
    else if (highest_alarm >= ALARM_B && highest_alarm <= ALARM_J)
    {
        beep_interval = 50;   // B~J类：50ms间隔
    }
    else // K~M类
    {
        // K~M类：连续低电平
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        return;
    }
    
    // 实现周期性脉冲
    if (current_time - last_beep_time >= beep_interval)
    {
        beep_state = !beep_state;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, beep_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        last_beep_time = current_time;
    }
}

/**
 * @brief 自动清除报警
 */
void AlarmManager_AutoClearAlarms(void)
{
    // 基本实现：检查所有报警的清除条件
    for (AlarmType_t alarm = ALARM_A; alarm <= ALARM_O; alarm++)
    {
        if (AlarmManager_IsAlarmActive(alarm))
        {
            if (AlarmManager_CheckAlarmClearCondition(alarm))
            {
                AlarmManager_ClearAlarm(alarm);
            }
        }
    }
}

/**
 * @brief 报警管理任务处理
 * @details 在主循环中调用，处理报警状态更新和输出控制
 */
void AlarmManager_Task(void)
{
    // 更新报警输出状态
    AlarmManager_UpdateAlarmOutput();
    
    // 更新蜂鸣器输出状态
    AlarmManager_UpdateBeeper();
    
    // 检查并自动清除满足条件的报警
    AlarmManager_AutoClearAlarms();
}

 
