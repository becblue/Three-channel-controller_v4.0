/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.c
  * @brief          : �̵�������Դ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#include "relay_control.h"

/**
  * @brief  �̵������Ƴ�ʼ��
  * @param  None
  * @retval None
  */
void RelayControl_Init(void)
{
    // CubeMX�Ѿ����GPIO��ʼ������������������
}

/**
  * @brief  ��ȡK1_EN״̬����������
  * @param  None
  * @retval 1: �͵�ƽ(ʹ��), 0: �ߵ�ƽ(����)
  */
uint8_t RelayControl_ReadK1_EN(void)
{
    // ���β������������20ms�����ν��һ�²ŷ���
    uint8_t read1 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);  // �͵�ƽΪʹ��
    HAL_Delay(20);  // 20ms���
    uint8_t read2 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);
    HAL_Delay(20);  // 20ms���
    uint8_t read3 = !HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin);
    
    // ���β������һ�²ŷ�����Чֵ
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // ��һ���򷵻�0����ȫ״̬��
    }
}

/**
  * @brief  ��ȡK2_EN״̬����������
  * @param  None
  * @retval 1: �͵�ƽ(ʹ��), 0: �ߵ�ƽ(����)
  */
uint8_t RelayControl_ReadK2_EN(void)
{
    // ���β������������20ms�����ν��һ�²ŷ���
    uint8_t read1 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    HAL_Delay(20);  // 20ms���
    uint8_t read2 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    HAL_Delay(20);  // 20ms���
    uint8_t read3 = !HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin);
    
    // ���β������һ�²ŷ�����Чֵ
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // ��һ���򷵻�0����ȫ״̬��
    }
}

/**
  * @brief  ��ȡK3_EN״̬����������
  * @param  None
  * @retval 1: �͵�ƽ(ʹ��), 0: �ߵ�ƽ(����)
  */
uint8_t RelayControl_ReadK3_EN(void)
{
    // ���β������������20ms�����ν��һ�²ŷ���
    uint8_t read1 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    HAL_Delay(20);  // 20ms���
    uint8_t read2 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    HAL_Delay(20);  // 20ms���
    uint8_t read3 = !HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin);
    
    // ���β������һ�²ŷ�����Чֵ
    if (read1 == read2 && read2 == read3)
    {
        return read1;
    }
    else
    {
        return 0;  // ��һ���򷵻�0����ȫ״̬��
    }
}

/**
  * @brief  ����ͨ��1��K1_1_ON + K1_2_ONͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel1(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K1_1_ON��K1_2_ON�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K1_1_ON_Pin << 16U;  // ����K1_1_ON (GPIOC PIN0)
    GPIOA->BSRR = (uint32_t)K1_2_ON_Pin << 16U;  // ����K1_2_ON (GPIOA PIN12)
    
    HAL_Delay(RELAY_PULSE_TIME);  // 500ms����
    
    GPIOC->BSRR = K1_1_ON_Pin;  // ����K1_1_ON (GPIOC PIN0)
    GPIOA->BSRR = K1_2_ON_Pin;  // ����K1_2_ON (GPIOA PIN12)
}

/**
  * @brief  ����ͨ��2��K2_1_ON + K2_2_ONͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel2(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K2_1_ON��K2_2_ON�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K2_1_ON_Pin << 16U;  // ����K2_1_ON (GPIOC PIN2)
    GPIOA->BSRR = (uint32_t)K2_2_ON_Pin << 16U;  // ����K2_2_ON (GPIOA PIN4)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K2_1_ON_Pin;  // ����K2_1_ON (GPIOC PIN2)
    GPIOA->BSRR = K2_2_ON_Pin;  // ����K2_2_ON (GPIOA PIN4)
}

/**
  * @brief  ����ͨ��3��K3_1_ON + K3_2_ONͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOnChannel3(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K3_1_ON��K3_2_ON�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K3_1_ON_Pin << 16U;  // ����K3_1_ON (GPIOC PIN7)
    GPIOD->BSRR = (uint32_t)K3_2_ON_Pin << 16U;  // ����K3_2_ON (GPIOD PIN2)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K3_1_ON_Pin;  // ����K3_1_ON (GPIOC PIN7)
    GPIOD->BSRR = K3_2_ON_Pin;  // ����K3_2_ON (GPIOD PIN2)
}

/**
  * @brief  �ر�ͨ��1��K1_1_OFF + K1_2_OFFͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel1(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K1_1_OFF��K1_2_OFF�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K1_1_OFF_Pin << 16U;  // ����K1_1_OFF (GPIOC PIN1)
    GPIOA->BSRR = (uint32_t)K1_2_OFF_Pin << 16U;  // ����K1_2_OFF (GPIOA PIN3)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K1_1_OFF_Pin;  // ����K1_1_OFF (GPIOC PIN1)
    GPIOA->BSRR = K1_2_OFF_Pin;  // ����K1_2_OFF (GPIOA PIN3)
}

/**
  * @brief  �ر�ͨ��2��K2_1_OFF + K2_2_OFFͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel2(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K2_1_OFF��K2_2_OFF�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K2_1_OFF_Pin << 16U;  // ����K2_1_OFF (GPIOC PIN3)
    GPIOA->BSRR = (uint32_t)K2_2_OFF_Pin << 16U;  // ����K2_2_OFF (GPIOA PIN5)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K2_1_OFF_Pin;  // ����K2_1_OFF (GPIOC PIN3)
    GPIOA->BSRR = K2_2_OFF_Pin;  // ����K2_2_OFF (GPIOA PIN5)
}

/**
  * @brief  �ر�ͨ��3��K3_1_OFF + K3_2_OFFͬʱ����500ms���壩
  * @param  None
  * @retval None
  */
void RelayControl_TurnOffChannel3(void)
{
    // ����ͬʱ���500ms�͵�ƽ���嵽�����̵���
    // K3_1_OFF��K3_2_OFF�ڲ�ͬ�˿ڣ�����д��BSRRʵ�����̶�ͬʱ
    GPIOC->BSRR = (uint32_t)K3_1_OFF_Pin << 16U;  // ����K3_1_OFF (GPIOC PIN6)
    GPIOA->BSRR = (uint32_t)K3_2_OFF_Pin << 16U;  // ����K3_2_OFF (GPIOA PIN7)
    
    HAL_Delay(RELAY_PULSE_TIME);
    
    GPIOC->BSRR = K3_1_OFF_Pin;  // ����K3_1_OFF (GPIOC PIN6)
    GPIOA->BSRR = K3_2_OFF_Pin;  // ����K3_2_OFF (GPIOA PIN7)
}

/**
  * @brief  ��ȡK1_1_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK1_1_STA(void)
{
    return HAL_GPIO_ReadPin(K1_1_STA_GPIO_Port, K1_1_STA_Pin);
}

/**
  * @brief  ��ȡK1_2_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK1_2_STA(void)
{
    return HAL_GPIO_ReadPin(K1_2_STA_GPIO_Port, K1_2_STA_Pin);
}

/**
  * @brief  ��ȡK2_1_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK2_1_STA(void)
{
    return HAL_GPIO_ReadPin(K2_1_STA_GPIO_Port, K2_1_STA_Pin);
}

/**
  * @brief  ��ȡK2_2_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK2_2_STA(void)
{
    return HAL_GPIO_ReadPin(K2_2_STA_GPIO_Port, K2_2_STA_Pin);
}

/**
  * @brief  ��ȡK3_1_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK3_1_STA(void)
{
    return HAL_GPIO_ReadPin(K3_1_STA_GPIO_Port, K3_1_STA_Pin);
}

/**
  * @brief  ��ȡK3_2_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�̵�������), 0: �͵�ƽ(�̵����Ͽ�)
  */
uint8_t RelayControl_ReadK3_2_STA(void)
{
    return HAL_GPIO_ReadPin(K3_2_STA_GPIO_Port, K3_2_STA_Pin);
}

/**
  * @brief  ��ȡSW1_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�Ӵ�������), 0: �͵�ƽ(�Ӵ����Ͽ�)
  */
uint8_t RelayControl_ReadSW1_STA(void)
{
    return HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin);
}

/**
  * @brief  ��ȡSW2_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�Ӵ�������), 0: �͵�ƽ(�Ӵ����Ͽ�)
  */
uint8_t RelayControl_ReadSW2_STA(void)
{
    return HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin);
}

/**
  * @brief  ��ȡSW3_STA״̬
  * @param  None
  * @retval 1: �ߵ�ƽ(�Ӵ�������), 0: �͵�ƽ(�Ӵ����Ͽ�)
  */
uint8_t RelayControl_ReadSW3_STA(void)
{
    return HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin);
}

/**
  * @brief  ���ʹ�ܳ�ͻ�쳣��A���쳣��
  * @param  None
  * @retval ALARM_A: ��·ʹ�ܳ�ͻ, ALARM_NONE: �޳�ͻ
  */
AlarmType_t RelayControl_CheckEnableConflict(void)
{
    uint8_t k1_en = RelayControl_ReadK1_EN();
    uint8_t k2_en = RelayControl_ReadK2_EN();
    uint8_t k3_en = RelayControl_ReadK3_EN();
    
    // ����Ƿ��ж�·ͬʱʹ�ܣ�A���쳣��
    uint8_t enable_count = k1_en + k2_en + k3_en;
    
    if (enable_count > 1)  // ��·ͬʱ�͵�ƽ
    {
        return ALARM_A;
    }
    
    return ALARM_NONE;
}

/**
  * @brief  ���̵���״̬�쳣
  * @param  ch: Ҫ����ͨ��
  * @retval ��Ӧ���쳣���ͻ�ALARM_NONE
  */
AlarmType_t RelayControl_CheckRelayStatus(Channel_t ch)
{
    switch (ch)
    {
        case CHANNEL_1:
            // ���ͨ��1�ļ̵����ͽӴ���״̬
            if (!RelayControl_ReadK1_1_STA()) return ALARM_B;  // K1_1_STA�쳣
            if (!RelayControl_ReadK1_2_STA()) return ALARM_E;  // K1_2_STA�쳣
            if (!RelayControl_ReadSW1_STA())  return ALARM_H;  // SW1_STA�쳣
            break;
            
        case CHANNEL_2:
            // ���ͨ��2�ļ̵����ͽӴ���״̬
            if (!RelayControl_ReadK2_1_STA()) return ALARM_C;  // K2_1_STA�쳣
            if (!RelayControl_ReadK2_2_STA()) return ALARM_F;  // K2_2_STA�쳣
            if (!RelayControl_ReadSW2_STA())  return ALARM_I;  // SW2_STA�쳣
            break;
            
        case CHANNEL_3:
            // ���ͨ��3�ļ̵����ͽӴ���״̬
            if (!RelayControl_ReadK3_1_STA()) return ALARM_D;  // K3_1_STA�쳣
            if (!RelayControl_ReadK3_2_STA()) return ALARM_G;  // K3_2_STA�쳣
            if (!RelayControl_ReadSW3_STA())  return ALARM_J;  // SW3_STA�쳣
            break;
            
        default:
            break;
    }
    
    return ALARM_NONE;
}

/**
  * @brief  ��ȡ�̵���״̬����װ������
  * @param  channel: ͨ����
  * @param  relay_num: �̵�����ţ�1��2��
  * @retval �̵���״̬��1=�ߵ�ƽ��0=�͵�ƽ��
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
  * @brief  ��ȡ�Ӵ���״̬����װ������
  * @param  channel: ͨ����
  * @retval �Ӵ���״̬��1=�ߵ�ƽ��0=�͵�ƽ��
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

