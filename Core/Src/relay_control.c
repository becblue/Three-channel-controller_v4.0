/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.c
  * @brief          : 继电器控制源文件
  ******************************************************************************
  */
/* USER CODE END Header */

#include "relay_control.h"

/**
  * @brief  继电器控制初始化
  * @param  None
  * @retval None
  */
void RelayControl_Init(void)
{
    // CubeMX已经完成GPIO初始化，这里无需额外操作
}

/**
  * @brief  读取K1_EN状态（带消抖）
  * @param  None
  * @retval 1: 低电平(使能), 0: 高电平(禁用)
  */
uint8_t RelayControl_ReadK1_EN(void)
{
    // 三次采样消抖：间隔20ms，三次结果一致才返回
    uint8_t read1 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);  // 低电平为使能
    HAL_Delay(20);  // 20ms间隔
    uint8_t read2 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);
    HAL_Delay(20);  // 20ms间隔
    uint8_t read3 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);
    
    // 三次采样结果一致才返回有效值
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // 不一致则返回0（安全状态）
    }
}

/**
  * @brief  读取K2_EN状态（带消抖）
  * @param  None
  * @retval 1: 低电平(使能), 0: 高电平(禁用)
  */
uint8_t RelayControl_ReadK2_EN(void)
{
    // 三次采样消抖：间隔20ms，三次结果一致才返回
    uint8_t read1 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    HAL_Delay(20);  // 20ms间隔
    uint8_t read2 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    HAL_Delay(20);  // 20ms间隔
    uint8_t read3 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    
    // 三次采样结果一致才返回有效值
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // 不一致则返回0（安全状态）
    }
}

/**
  * @brief  读取K3_EN状态（带消抖）
  * @param  None
  * @retval 1: 低电平(使能), 0: 高电平(禁用)
  */
uint8_t RelayControl_ReadK3_EN(void)
{
    // 三次采样消抖：间隔20ms，三次结果一致才返回
    uint8_t read1 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    HAL_Delay(20);  // 20ms间隔
    uint8_t read2 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    HAL_Delay(20);  // 20ms间隔
    uint8_t read3 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    
    // 三次采样结果一致才返回有效值
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // 不一致则返回0（安全状态）
    }
}

/**
  * @brief  开启通道1（K1_1_ON + K1_2_ON同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel1(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K1_1_ON和K1_2_ON在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K1_1_ON_Pin << 16U;  // 拉低K1_1_ON (GPIOC PIN0)
    GPIOA->BSRR = (uint32_t)K1_2_ON_Pin << 16U;  // 拉低K1_2_ON (GPIOA PIN12)
    
    HAL_Delay(RELAY_PULSE_TIME);  // 500ms脉冲
    
    GPIOC->BSRR = K1_1_ON_Pin;  // 拉高K1_1_ON (GPIOC PIN0)
    GPIOA->BSRR = K1_2_ON_Pin;  // 拉高K1_2_ON (GPIOA PIN12)
}

/**
  * @brief  开启通道2（K2_1_ON + K2_2_ON同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel2(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K2_1_ON和K2_2_ON在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K2_1_ON_Pin << 16U;  // 拉低K2_1_ON (GPIOC PIN2)
    GPIOA->BSRR = (uint32_t)K2_2_ON_Pin << 16U;  // 拉低K2_2_ON (GPIOA PIN4)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K2_1_ON_Pin;  // 拉高K2_1_ON (GPIOC PIN2)
    GPIOA->BSRR = K2_2_ON_Pin;  // 拉高K2_2_ON (GPIOA PIN4)
}

/**
  * @brief  开启通道3（K3_1_ON + K3_2_ON同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel3(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K3_1_ON和K3_2_ON在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K3_1_ON_Pin << 16U;  // 拉低K3_1_ON (GPIOC PIN7)
    GPIOD->BSRR = (uint32_t)K3_2_ON_Pin << 16U;  // 拉低K3_2_ON (GPIOD PIN2)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K3_1_ON_Pin;  // 拉高K3_1_ON (GPIOC PIN7)
    GPIOD->BSRR = K3_2_ON_Pin;  // 拉高K3_2_ON (GPIOD PIN2)
}

/**
  * @brief  关闭通道1（K1_1_OFF + K1_2_OFF同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel1(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K1_1_OFF和K1_2_OFF在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K1_1_OFF_Pin << 16U;  // 拉低K1_1_OFF (GPIOC PIN1)
    GPIOA->BSRR = (uint32_t)K1_2_OFF_Pin << 16U;  // 拉低K1_2_OFF (GPIOA PIN3)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K1_1_OFF_Pin;  // 拉高K1_1_OFF (GPIOC PIN1)
    GPIOA->BSRR = K1_2_OFF_Pin;  // 拉高K1_2_OFF (GPIOA PIN3)
}

/**
  * @brief  关闭通道2（K2_1_OFF + K2_2_OFF同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel2(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K2_1_OFF和K2_2_OFF在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K2_1_OFF_Pin << 16U;  // 拉低K2_1_OFF (GPIOC PIN3)
    GPIOA->BSRR = (uint32_t)K2_2_OFF_Pin << 16U;  // 拉低K2_2_OFF (GPIOA PIN5)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K2_1_OFF_Pin;  // 拉高K2_1_OFF (GPIOC PIN3)
    GPIOA->BSRR = K2_2_OFF_Pin;  // 拉高K2_2_OFF (GPIOA PIN5)
}

/**
  * @brief  关闭通道3（K3_1_OFF + K3_2_OFF同时触发500ms脉冲）
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel3(void)
{
    // 绝对同时输出500ms低电平脉冲到两个继电器
    // K3_1_OFF和K3_2_OFF在不同端口，连续写入BSRR实现最大程度同时
    GPIOC->BSRR = (uint32_t)K3_1_OFF_Pin << 16U;  // 拉低K3_1_OFF (GPIOC PIN6)
    GPIOA->BSRR = (uint32_t)K3_2_OFF_Pin << 16U;  // 拉低K3_2_OFF (GPIOA PIN7)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K3_1_OFF_Pin;  // 拉高K3_1_OFF (GPIOC PIN6)
    GPIOA->BSRR = K3_2_OFF_Pin;  // 拉高K3_2_OFF (GPIOA PIN7)
}

/**
  * @brief  读取K1_1_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK1_1_STA(void)
{
    return HAL_GPIO_ReadPin(K1_1_STA_GPIO_Port, K1_1_STA_Pin);
}

/**
  * @brief  读取K1_2_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK1_2_STA(void)
{
    return HAL_GPIO_ReadPin(K1_2_STA_GPIO_Port, K1_2_STA_Pin);
}

/**
  * @brief  读取K2_1_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK2_1_STA(void)
{
    return HAL_GPIO_ReadPin(K2_1_STA_GPIO_Port, K2_1_STA_Pin);
}

/**
  * @brief  读取K2_2_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK2_2_STA(void)
{
    return HAL_GPIO_ReadPin(K2_2_STA_GPIO_Port, K2_2_STA_Pin);
}

/**
  * @brief  读取K3_1_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK3_1_STA(void)
{
    return HAL_GPIO_ReadPin(K3_1_STA_GPIO_Port, K3_1_STA_Pin);
}

/**
  * @brief  读取K3_2_STA状态
  * @param  None
  * @retval 1: 高电平(继电器吸合), 0: 低电平(继电器断开)
  */
uint8_t RelayControl_ReadK3_2_STA(void)
{
    return HAL_GPIO_ReadPin(K3_2_STA_GPIO_Port, K3_2_STA_Pin);
}

/**
  * @brief  读取SW1_STA状态
  * @param  None
  * @retval 1: 高电平(接触器吸合), 0: 低电平(接触器断开)
  */
uint8_t RelayControl_ReadSW1_STA(void)
{
    return HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin);
}

/**
  * @brief  读取SW2_STA状态
  * @param  None
  * @retval 1: 高电平(接触器吸合), 0: 低电平(接触器断开)
  */
uint8_t RelayControl_ReadSW2_STA(void)
{
    return HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin);
}

/**
  * @brief  读取SW3_STA状态
  * @param  None
  * @retval 1: 高电平(接触器吸合), 0: 低电平(接触器断开)
  */
uint8_t RelayControl_ReadSW3_STA(void)
{
    return HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin);
}

/**
  * @brief  检测使能冲突异常（A类异常）
  * @param  None
  * @retval ALARM_A: 多路使能冲突, ALARM_NONE: 无冲突
  */
AlarmType_t RelayControl_CheckEnableConflict(void)
{
    uint8_t k1_en = RelayControl_ReadK1_EN();
    uint8_t k2_en = RelayControl_ReadK2_EN();
    uint8_t k3_en = RelayControl_ReadK3_EN();
    
    // 检查是否有多路同时使能（A类异常）
    uint8_t enable_count = k1_en + k2_en + k3_en;
    
    if (enable_count > 1)  // 多路同时低电平
    {
        return ALARM_A;
    }
    
    return ALARM_NONE;
}

/**
  * @brief  检测继电器状态异常
  * @param  ch: 要检测的通道
  * @retval 对应的异常类型或ALARM_NONE
  */
AlarmType_t RelayControl_CheckRelayStatus(Channel_t ch)
{
    switch (ch)
    {
        case CHANNEL_1:
            // 检查通道1的继电器和接触器状态
            if (!RelayControl_ReadK1_1_STA()) return ALARM_B;  // K1_1_STA异常
            if (!RelayControl_ReadK1_2_STA()) return ALARM_E;  // K1_2_STA异常
            if (!RelayControl_ReadSW1_STA())  return ALARM_H;  // SW1_STA异常
            break;
            
        case CHANNEL_2:
            // 检查通道2的继电器和接触器状态
            if (!RelayControl_ReadK2_1_STA()) return ALARM_C;  // K2_1_STA异常
            if (!RelayControl_ReadK2_2_STA()) return ALARM_F;  // K2_2_STA异常
            if (!RelayControl_ReadSW2_STA())  return ALARM_I;  // SW2_STA异常
            break;
            
        case CHANNEL_3:
            // 检查通道3的继电器和接触器状态
            if (!RelayControl_ReadK3_1_STA()) return ALARM_D;  // K3_1_STA异常
            if (!RelayControl_ReadK3_2_STA()) return ALARM_G;  // K3_2_STA异常
            if (!RelayControl_ReadSW3_STA())  return ALARM_J;  // SW3_STA异常
            break;
            
        default:
            break;
    }
    
    return ALARM_NONE;
}

/**
  * @brief  获取继电器状态（封装函数）
  * @param  channel: 通道号
  * @param  relay_num: 继电器编号（1或2）
  * @retval 继电器状态（1=高电平，0=低电平）
  */
uint8_t RelayControl_GetRelayStatus(Channel_t channel, uint8_t relay_num)
{
    switch (channel)
    {
        case CHANNEL_1:
            return (relay_num == 1) ? RelayControl_ReadK1_1_STA() : RelayControl_ReadK1_2_STA();
            
        case CHANNEL_2:
            return (relay_num == 1) ? RelayControl_ReadK2_1_STA() : RelayControl_ReadK2_2_STA();
            
        case CHANNEL_3:
            return (relay_num == 1) ? RelayControl_ReadK3_1_STA() : RelayControl_ReadK3_2_STA();
            
        default:
            return 0;
    }
}

/**
  * @brief  获取接触器状态（封装函数）
  * @param  channel: 通道号
  * @retval 接触器状态（1=高电平，0=低电平）
  */
uint8_t RelayControl_GetContactorStatus(Channel_t channel)
{
    switch (channel)
    {
        case CHANNEL_1:
            return RelayControl_ReadSW1_STA();
            
        case CHANNEL_2:
            return RelayControl_ReadSW2_STA();
            
        case CHANNEL_3:
            return RelayControl_ReadSW3_STA();
            
        default:
            return 0;
    }
}

