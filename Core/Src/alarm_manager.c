/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_manager.c
  * @brief          : ��������ϵͳԴ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#include "alarm_manager.h"
#include "relay_control.h"
#include "temperature_control.h"

/* ˽�б��� -------------------------------------------------------------------*/
static uint16_t active_alarms = 0;            // ��ǰ��Ծ���쳣��־λͼ��λ0-14��ӦA-O���쳣��
static BeepMode_t current_beep_mode = BEEP_MODE_OFF;  // ��ǰ������ģʽ
static uint32_t beep_timer = 0;               // ������ʱ�����
static uint8_t beep_state = 0;                // ��������ǰ״̬��0=�ߵ�ƽ��1=�͵�ƽ��

/* ˽�к������� ---------------------------------------------------------------*/
static void AlarmManager_UpdateBeepMode(void);
static void AlarmManager_ProcessBeep(void);
static uint8_t AlarmManager_CheckTruthTableMatch(void);

/**
  * @brief  ������������ʼ��
  * @param  None
  * @retval None
  */
void AlarmManager_Init(void)
{
    // ��������쳣��־
    active_alarms = 0;
    g_alarm = ALARM_NONE;
    
    // ��ʼ��ALARM���Ϊ�ߵ�ƽ���ޱ�����
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);
    
    // ��ʼ��������Ϊ�ߵ�ƽ��������
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    
    current_beep_mode = BEEP_MODE_OFF;
    beep_timer = 0;
    beep_state = 0;
}

/**
  * @brief  �����쳣����
  * @param  alarm_type: �쳣����
  * @retval None
  */
void AlarmManager_SetAlarm(AlarmType_t alarm_type)
{
    if (alarm_type >= ALARM_A && alarm_type <= ALARM_O)
    {
        // ���ö�Ӧ���쳣��־λ
        uint16_t alarm_bit = 1 << (alarm_type - ALARM_A);
        active_alarms |= alarm_bit;
        
        // ����ȫ���쳣״̬
        g_alarm = alarm_type;
        
        // ����ALARM������͵�ƽ��
        AlarmManager_SetAlarmOutput(1);
        
        // ���·�����ģʽ
        AlarmManager_UpdateBeepMode();
    }
}

/**
  * @brief  ����쳣����
  * @param  alarm_type: �쳣����
  * @retval None
  */
void AlarmManager_ClearAlarm(AlarmType_t alarm_type)
{
    if (alarm_type >= ALARM_A && alarm_type <= ALARM_O)
    {
        // �����Ӧ���쳣��־λ
        uint16_t alarm_bit = 1 << (alarm_type - ALARM_A);
        active_alarms &= ~alarm_bit;
        
        // ���û���κ��쳣������ȫ��״̬
        if (active_alarms == 0)
        {
            g_alarm = ALARM_NONE;
            AlarmManager_SetAlarmOutput(0);  // �ر�ALARM���
        }
        
        // ���·�����ģʽ
        AlarmManager_UpdateBeepMode();
    }
}

/**
  * @brief  ��������쳣
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
  * @brief  ����ض��쳣�Ƿ񼤻�
  * @param  alarm_type: �쳣����
  * @retval 1=���0=δ����
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
  * @brief  ����Ƿ����κ��쳣
  * @param  None
  * @retval 1=���쳣��0=���쳣
  */
uint8_t AlarmManager_HasAnyAlarm(void)
{
    return (active_alarms != 0) ? 1 : 0;
}

/**
  * @brief  ����ALARM���
  * @param  enable: 1=���ñ���������͵�ƽ����0=�رձ���������ߵ�ƽ��
  * @retval None
  */
void AlarmManager_SetAlarmOutput(uint8_t enable)
{
    if (enable)
    {
        HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_RESET);  // �͵�ƽ - ����
    }
    else
    {
        HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_SET);    // �ߵ�ƽ - ����
    }
}

/**
  * @brief  ���÷�����ģʽ
  * @param  mode: ������ģʽ
  * @retval None
  */
void AlarmManager_SetBeepMode(BeepMode_t mode)
{
    current_beep_mode = mode;
    beep_timer = HAL_GetTick();
    
    if (mode == BEEP_MODE_OFF)
    {
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);  // �ߵ�ƽ - ����
        beep_state = 0;
    }
    else if (mode == BEEP_MODE_CONTINUOUS)
    {
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);  // �͵�ƽ - ������
        beep_state = 1;
    }
}

/**
  * @brief  ���ݵ�ǰ�쳣���·�����ģʽ
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
    
    // ���K��L��M���쳣�����ȼ���ߣ�
    if (AlarmManager_IsAlarmActive(ALARM_K) || 
        AlarmManager_IsAlarmActive(ALARM_L) || 
        AlarmManager_IsAlarmActive(ALARM_M))
    {
        AlarmManager_SetBeepMode(BEEP_MODE_CONTINUOUS);
    }
    // ���A��N���쳣
    else if (AlarmManager_IsAlarmActive(ALARM_A) || 
             AlarmManager_IsAlarmActive(ALARM_N))
    {
        AlarmManager_SetBeepMode(BEEP_MODE_1S_PULSE);
    }
    // ���B~J���쳣
    else
    {
        AlarmManager_SetBeepMode(BEEP_MODE_50MS_PULSE);
    }
}

/**
  * @brief  ����������������
  * @param  None
  * @retval None
  */
static void AlarmManager_ProcessBeep(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t interval = 0;
    
    if (current_beep_mode == BEEP_MODE_OFF || current_beep_mode == BEEP_MODE_CONTINUOUS)
    {
        return;  // ����Ҫ��������
    }
    
    // ȷ��������
    if (current_beep_mode == BEEP_MODE_1S_PULSE)
    {
        interval = 1000;  // 1��
    }
    else if (current_beep_mode == BEEP_MODE_50MS_PULSE)
    {
        interval = 50;    // 50ms
    }
    
    // ����Ƿ����л�ʱ��
    if (current_time - beep_timer >= interval)
    {
        beep_timer = current_time;
        beep_state = !beep_state;  // �л�״̬
        
        if (beep_state)
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);  // �͵�ƽ - ��
        }
        else
        {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);    // �ߵ�ƽ - ����
        }
    }
}

/**
  * @brief  ��鵱ǰ״̬�Ƿ������ֵ��
  * @param  None
  * @retval 1=���ϣ�0=������
  */
static uint8_t AlarmManager_CheckTruthTableMatch(void)
{
    // ��ȡʹ���ź�
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // ��ȡ�̵���״̬
    uint8_t k1_1_sta = RelayControl_GetRelayStatus(CHANNEL_1, 1);
    uint8_t k1_2_sta = RelayControl_GetRelayStatus(CHANNEL_1, 2);
    uint8_t k2_1_sta = RelayControl_GetRelayStatus(CHANNEL_2, 1);
    uint8_t k2_2_sta = RelayControl_GetRelayStatus(CHANNEL_2, 2);
    uint8_t k3_1_sta = RelayControl_GetRelayStatus(CHANNEL_3, 1);
    uint8_t k3_2_sta = RelayControl_GetRelayStatus(CHANNEL_3, 2);
    
    // ��ȡ�Ӵ���״̬
    uint8_t sw1_sta = RelayControl_GetContactorStatus(CHANNEL_1);
    uint8_t sw2_sta = RelayControl_GetContactorStatus(CHANNEL_2);
    uint8_t sw3_sta = RelayControl_GetContactorStatus(CHANNEL_3);
    
    // �����ֵ��ƥ��
    // Channel_1��: K1_EN=0,K2_EN=1,K3_EN=1,K1_1_STA=1,K1_2_STA=1,����=0
    if (k1_en == 1 && k2_en == 0 && k3_en == 0)
    {
        return (k1_1_sta == 1 && k1_2_sta == 1 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 1 && sw2_sta == 0 && sw3_sta == 0);
    }
    // Channel_2��: K1_EN=1,K2_EN=0,K3_EN=1,K2_1_STA=1,K2_2_STA=1,����=0
    else if (k1_en == 0 && k2_en == 1 && k3_en == 0)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 1 && k2_2_sta == 1 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 0 && sw2_sta == 1 && sw3_sta == 0);
    }
    // Channel_3��: K1_EN=1,K2_EN=1,K3_EN=0,K3_1_STA=1,K3_2_STA=1,����=0
    else if (k1_en == 0 && k2_en == 0 && k3_en == 1)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 1 && k3_2_sta == 1 && sw1_sta == 0 && sw2_sta == 0 && sw3_sta == 1);
    }
    // ȫ���ر�: K1_EN=1,K2_EN=1,K3_EN=1,����״̬=0
    else if (k1_en == 0 && k2_en == 0 && k3_en == 0)
    {
        return (k1_1_sta == 0 && k1_2_sta == 0 && k2_1_sta == 0 && k2_2_sta == 0 && 
                k3_1_sta == 0 && k3_2_sta == 0 && sw1_sta == 0 && sw2_sta == 0 && sw3_sta == 0);
    }
    
    return 0;  // �������κ���Ч״̬
}

/**
  * @brief  ��鱨���������
  * @param  alarm_type: �쳣����
  * @retval 1=���Խ����0=���ܽ��
  */
uint8_t AlarmManager_CheckAlarmClearCondition(AlarmType_t alarm_type)
{
    switch (alarm_type)
    {
        case ALARM_A:  // ʹ�ܳ�ͻ�쳣
        {
            // ��ȡʹ���ź�
            uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
            
            // ֻ����һ·�͵�ƽ��ȫ���ߵ�ƽ
            uint8_t low_count = k1_en + k2_en + k3_en;
            return (low_count <= 1) ? 1 : 0;
        }
        
        case ALARM_B:  case ALARM_C:  case ALARM_D:  case ALARM_E:  
        case ALARM_F:  case ALARM_G:  case ALARM_H:  case ALARM_I:  
        case ALARM_J:  case ALARM_N:  // ״̬�쳣���Լ��쳣
        {
            // ����Ƿ������ֵ��
            return AlarmManager_CheckTruthTableMatch();
        }
        
        case ALARM_K:  // NTC_1�¶��쳣
        {
            float temp = TemperatureControl_GetTemperature(0);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58������
        }
        
        case ALARM_L:  // NTC_2�¶��쳣
        {
            float temp = TemperatureControl_GetTemperature(1);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58������
        }
        
        case ALARM_M:  // NTC_3�¶��쳣
        {
            float temp = TemperatureControl_GetTemperature(2);
            return (temp < (TEMP_HIGH - TEMP_HYSTERESIS)) ? 1 : 0;  // 58������
        }
        
        case ALARM_O:  // ��Դ����쳣
        {
            // ���DC_CTRL�ź��Ƿ�ָ�����
            return (HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin) == GPIO_PIN_SET) ? 1 : 0;
        }
        
        default:
            return 0;
    }
}

/**
  * @brief  ����������������
  * @param  None
  * @retval None
  */
void AlarmManager_Process(void)
{
    // ����������������
    AlarmManager_ProcessBeep();
    
    // ��鵱ǰ��Ծ�쳣�Ľ������
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
 * @brief ��ȡ��ǰ��Ծ��������
 * @return uint8_t ��Ծ��������
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
 * @brief ��ȡ��Ծ�����б�
 * @param alarms ��������
 * @param max_count �������
 * @return uint8_t ʵ�ʻ�ȡ�ı�������
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
 * @brief ���±������״̬
 */
void AlarmManager_UpdateAlarmOutput(void)
{
    // ������κα���������PB4���͵�ƽ��Ч��
    if (AlarmManager_GetAlarmCount() > 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // ����������͵�ƽ
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);   // ����״̬���ߵ�ƽ
    }
}

/**
 * @brief ���·�����״̬
 */
void AlarmManager_UpdateBeeper(void)
{
    static uint32_t last_beep_time = 0;
    static uint8_t beep_state = 0;
    uint32_t current_time = HAL_GetTick();
    
    // ��鱨�����ȼ���ģʽ
    uint8_t alarm_count = AlarmManager_GetAlarmCount();
    if (alarm_count == 0)
    {
        // �ޱ������رշ�����
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        return;
    }
    
    // ��ȡ������ȼ���������
    AlarmType_t active_alarms[5];
    AlarmManager_GetActiveAlarms(active_alarms, 5);
    AlarmType_t highest_alarm = active_alarms[0];
    
    uint32_t beep_interval;
    if (highest_alarm == ALARM_A || highest_alarm == ALARM_N)
    {
        beep_interval = 1000; // A��N�ࣺ1����
    }
    else if (highest_alarm >= ALARM_B && highest_alarm <= ALARM_J)
    {
        beep_interval = 50;   // B~J�ࣺ50ms���
    }
    else // K~M��
    {
        // K~M�ࣺ�����͵�ƽ
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        return;
    }
    
    // ʵ������������
    if (current_time - last_beep_time >= beep_interval)
    {
        beep_state = !beep_state;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, beep_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
        last_beep_time = current_time;
    }
}

/**
 * @brief �Զ��������
 */
void AlarmManager_AutoClearAlarms(void)
{
    // ����ʵ�֣�������б������������
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
 * @brief ��������������
 * @details ����ѭ���е��ã�������״̬���º��������
 */
void AlarmManager_Task(void)
{
    // ���±������״̬
    AlarmManager_UpdateAlarmOutput();
    
    // ���·��������״̬
    AlarmManager_UpdateBeeper();
    
    // ��鲢�Զ�������������ı���
    AlarmManager_AutoClearAlarms();
}

 
