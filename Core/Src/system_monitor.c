/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_monitor.c
  * @brief          : ϵͳ���ģ��Դ�ļ�
  ******************************************************************************
  * @attention
  *
  * ϵͳ���ģ��ʵ�֣�
  * 1. �Ĳ��Լ����̣�ʶ������״̬���̵�����������Ӵ�����������¶ȼ��
  * 2. ʵʱ״̬��غ������������
  * 3. ��Դ��غ��쳣����
  * 4. ϵͳ״̬��֤�͹���
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "system_monitor.h"

/* �ڲ�״̬���� ---------------------------------------------------------------*/
static SelfCheckState_t selfcheck_state = SELFCHECK_IDLE;      // �Լ�״̬
static SelfCheckResult_t selfcheck_result = {0};               // �Լ���
static SystemStatus_t system_status = {0};                     // ϵͳ״̬
static uint8_t correction_retry_count = 0;                     // �������Դ���
static uint32_t last_monitor_time = 0;                         // �ϴμ��ʱ��

/* �ڲ��������� ---------------------------------------------------------------*/
static uint8_t CheckEnableSignalsState(void);                  // ���ʹ���ź�״̬
static uint8_t CheckRelayStatusSignals(void);                  // ���̵���״̬�ź�
static uint8_t CheckContactorStatusSignals(void);              // ���Ӵ���״̬�ź�
static uint8_t CheckTemperatureSafety(void);                   // ����¶Ȱ�ȫ
static uint8_t CorrectSingleRelay(uint8_t channel, uint8_t target_state); // ���������̵���
static void UpdateSelfCheckProgress(uint8_t step);             // �����Լ����
static SystemState_t DetermineExpectedState(void);             // ȷ������״̬
static uint8_t ValidateStateConsistency(void);                 // ��֤״̬һ����

/**
 * @brief ϵͳ��س�ʼ��
 * @param None
 * @retval None
 */
void SystemMonitor_Init(void)
{
    // ��ʼ���Լ�״̬
    selfcheck_state = SELFCHECK_IDLE;
    
    // ����Լ���
    selfcheck_result.step1_result = 0;
    selfcheck_result.step2_result = 0;
    selfcheck_result.step3_result = 0;
    selfcheck_result.step4_result = 0;
    selfcheck_result.overall_result = 0;
    selfcheck_result.progress = 0;
    
    // ��ʼ��ϵͳ״̬
    system_status.expected_state = SYSTEM_ALL_OFF;
    system_status.current_state = SYSTEM_ALL_OFF;
    system_status.power_status = 1; // �����Դ����
    system_status.correction_count = 0;
    system_status.monitor_state = MONITOR_NORMAL;
    
    // �������Լ���
    correction_retry_count = 0;
    
    // ��¼��ʼ��ʱ��
    last_monitor_time = HAL_GetTick();
}

/**
 * @brief ����ϵͳ�Լ�����
 * @param None
 * @retval SelfCheckState_t ��ǰ�Լ�״̬
 */
SelfCheckState_t SystemMonitor_RunSelfCheck(void)
{
    static uint32_t step_start_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    switch (selfcheck_state)
    {
        case SELFCHECK_IDLE:
            // ��ʼ�Լ�����
            selfcheck_state = SELFCHECK_STEP1;
            step_start_time = current_time;
            UpdateSelfCheckProgress(1);
            break;
            
        case SELFCHECK_STEP1:
            // ��һ����ʶ������״̬
            system_status.expected_state = DetermineExpectedState();
            
            // ���ʹ���ź��Ƿ��г�ͻ
            if (CheckEnableSignalsState())
            {
                selfcheck_result.step1_result = 1;
                selfcheck_state = SELFCHECK_STEP2;
                UpdateSelfCheckProgress(25);
            }
            else
            {
                // A���쳣��ʹ���źų�ͻ
                AlarmManager_SetAlarm(ALARM_A);
                selfcheck_result.step1_result = 0;
                selfcheck_state = SELFCHECK_FAILED;
            }
            break;
            
        case SELFCHECK_STEP2:
            // �ڶ������̵���״̬��������
            if (CheckRelayStatusSignals())
            {
                selfcheck_result.step2_result = 1;
                selfcheck_state = SELFCHECK_STEP3;
                UpdateSelfCheckProgress(50);
            }
            else
            {
                // ���Ծ���
                if (correction_retry_count < SELFCHECK_MAX_RETRY)
                {
                    if (SystemMonitor_CorrectRelayStates())
                    {
                        correction_retry_count++;
                        HAL_Delay(CORRECTION_DELAY_MS); // �ȴ��������
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
            // ���������Ӵ���״̬��������
            if (CheckContactorStatusSignals())
            {
                selfcheck_result.step3_result = 1;
                selfcheck_state = SELFCHECK_STEP4;
                UpdateSelfCheckProgress(75);
            }
            else
            {
                // ���Ծ���
                if (correction_retry_count < SELFCHECK_MAX_RETRY)
                {
                    if (SystemMonitor_CorrectContactorStates())
                    {
                        correction_retry_count++;
                        HAL_Delay(CORRECTION_DELAY_MS); // �ȴ��������
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
            // ���Ĳ����¶Ȱ�ȫ���
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
            // �Լ���ɣ�����״̬
            break;
            
        case SELFCHECK_FAILED:
            // �Լ�ʧ�ܣ�����N���쳣
            AlarmManager_SetAlarm(ALARM_N);
            selfcheck_result.overall_result = 0;
            break;
    }
    
    return selfcheck_state;
}

/**
 * @brief ����״̬���
 * @param None
 * @retval None
 */
void SystemMonitor_UpdateMonitoring(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // ����ظ�������
    if (current_time - last_monitor_time < MONITOR_UPDATE_MS)
    {
        return;
    }
    
    last_monitor_time = current_time;
    
    // ����Դ״̬
    system_status.power_status = SystemMonitor_CheckPowerStatus();
    if (!system_status.power_status)
    {
        SystemMonitor_HandlePowerError();
        return;
    }
    
    // ��������״̬
    system_status.expected_state = DetermineExpectedState();
    
    // �������״̬�Ƿ�һ��
    if (!ValidateStateConsistency())
    {
        // ״̬��һ�£����Ծ���
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
 * @brief ����Դ״̬
 * @param None
 * @retval uint8_t 1-��Դ������0-��Դ�쳣
 */
uint8_t SystemMonitor_CheckPowerStatus(void)
{
    // ��ȡDC_CTRL����״̬��PB5���͵�ƽΪtrue��
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(DC_CTRL_GPIO_Port, DC_CTRL_Pin);
    
    // �͵�ƽ��ʾ��Դ����
    return (pin_state == GPIO_PIN_RESET) ? 1 : 0;
}

/**
 * @brief �����Դ�쳣
 * @param None
 * @retval None
 */
void SystemMonitor_HandlePowerError(void)
{
    // ����O���쳣
    AlarmManager_SetAlarm(ALARM_O);
    
    // ����ϵͳ״̬Ϊ����
    system_status.monitor_state = MONITOR_ERROR;
    
    // ֹͣ���м̵���������ͨ����ִ���κο�������ʵ�֣�
}

/**
 * @brief ��������ϵͳ
 * @param None
 * @retval uint8_t 1-����ɹ���0-����ʧ��
 */
uint8_t SystemMonitor_CorrectSystem(void)
{
    uint8_t result = 1;
    
    // �����̵���״̬
    if (!SystemMonitor_CorrectRelayStates())
    {
        result = 0;
    }
    
    // ��ʱ�ȴ��̵���״̬�ȶ�
    HAL_Delay(CORRECTION_DELAY_MS);
    
    // �����Ӵ���״̬
    if (!SystemMonitor_CorrectContactorStates())
    {
        result = 0;
    }
    
    return result;
}

/**
 * @brief �����̵���״̬
 * @param None
 * @retval uint8_t 1-����ɹ���0-����ʧ��
 */
uint8_t SystemMonitor_CorrectRelayStates(void)
{
    uint8_t success = 1;
    SystemState_t expected = system_status.expected_state;
    
    // ��������״̬������ͨ���̵���
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            // ͨ��1Ӧ�ÿ����������ر�
            if (!CorrectSingleRelay(1, 1)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
            
        case SYSTEM_CHANNEL2_ON:
            // ͨ��2Ӧ�ÿ����������ر�
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 1)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
            
        case SYSTEM_CHANNEL3_ON:
            // ͨ��3Ӧ�ÿ����������ر�
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 1)) success = 0;
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            // ����ͨ����Ӧ�ùر�
            if (!CorrectSingleRelay(1, 0)) success = 0;
            if (!CorrectSingleRelay(2, 0)) success = 0;
            if (!CorrectSingleRelay(3, 0)) success = 0;
            break;
    }
    
    return success;
}

/**
 * @brief �����Ӵ���״̬
 * @param None
 * @retval uint8_t 1-����ɹ���0-����ʧ��
 */
uint8_t SystemMonitor_CorrectContactorStates(void)
{
    // �Ӵ���״̬ͨ���̵������ƣ�����������Ҫ����֤
    // ����Ӵ���״̬�쳣��ͨ�����ƶ�Ӧ�ļ̵���������
    uint8_t success = 1;
    SystemState_t expected = system_status.expected_state;
    
    // ��鲢�����Ӵ���״̬
    switch (expected)
    {
        case SYSTEM_CHANNEL1_ON:
            // ���SW1_STA�Ƿ�Ϊ�ߵ�ƽ
            if (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) != GPIO_PIN_SET)
            {
                // �������¼���ͨ��1
                RelayControl_OpenChannel(CHANNEL_1);
            }
            break;
            
        case SYSTEM_CHANNEL2_ON:
            // ���SW2_STA�Ƿ�Ϊ�ߵ�ƽ
            if (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) != GPIO_PIN_SET)
            {
                // �������¼���ͨ��2
                RelayControl_OpenChannel(CHANNEL_2);
            }
            break;
            
        case SYSTEM_CHANNEL3_ON:
            // ���SW3_STA�Ƿ�Ϊ�ߵ�ƽ
            if (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) != GPIO_PIN_SET)
            {
                // �������¼���ͨ��3
                RelayControl_OpenChannel(CHANNEL_3);
            }
            break;
            
        case SYSTEM_ALL_OFF:
        default:
            // ���нӴ�����Ӧ�ùرգ�����п�������رն�Ӧͨ��
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
 * @brief �������״̬
 * @param None
 * @retval uint8_t 1-״̬��ȷ��0-״̬����
 */
uint8_t SystemMonitor_CheckExpectedState(void)
{
    SystemState_t expected = DetermineExpectedState();
    return (expected != SYSTEM_ERROR) ? 1 : 0;
}

/**
 * @brief �������״̬
 * @param None
 * @retval uint8_t 1-����״̬��ȷ��0-״̬�쳣
 */
uint8_t SystemMonitor_CheckAllStates(void)
{
    // ���ʹ���ź�
    if (!CheckEnableSignalsState()) return 0;
    
    // ���̵���״̬
    if (!CheckRelayStatusSignals()) return 0;
    
    // ���Ӵ���״̬
    if (!CheckContactorStatusSignals()) return 0;
    
    // ����¶Ȱ�ȫ
    if (!CheckTemperatureSafety()) return 0;
    
    return 1;
}

/**
 * @brief ��֤ϵͳ״̬
 * @param None
 * @retval uint8_t 1-״̬��Ч��0-״̬��Ч
 */
uint8_t SystemMonitor_ValidateSystemState(void)
{
    return ValidateStateConsistency();
}

/**
 * @brief ��ȡ�Լ����
 * @param None
 * @retval uint8_t �Լ���ȣ�0-100��
 */
uint8_t SystemMonitor_GetSelfCheckProgress(void)
{
    return selfcheck_result.progress;
}

/**
 * @brief ��ȡ�Լ���
 * @param None
 * @retval SelfCheckResult_t �Լ����ṹ��
 */
SelfCheckResult_t SystemMonitor_GetSelfCheckResult(void)
{
    return selfcheck_result;
}

/**
 * @brief ��ȡϵͳ״̬
 * @param None
 * @retval SystemStatus_t ϵͳ״̬�ṹ��
 */
SystemStatus_t SystemMonitor_GetSystemStatus(void)
{
    return system_status;
}

/**
 * @brief ���ϵͳ�Ƿ�����
 * @param None
 * @retval uint8_t 1-ϵͳ������0-ϵͳ�쳣
 */
uint8_t SystemMonitor_IsSystemNormal(void)
{
    return (system_status.monitor_state == MONITOR_NORMAL) ? 1 : 0;
}

/**
 * @brief ϵͳ���������
 * @param None
 * @retval None
 */
void SystemMonitor_Task(void)
{
    // ����״̬���
    SystemMonitor_UpdateMonitoring();
}

/* �ڲ�����ʵ�� ---------------------------------------------------------------*/

/**
 * @brief ���ʹ���ź�״̬
 * @param None
 * @retval uint8_t 1-״̬��ȷ��0-״̬���󣨳�ͻ��
 */
static uint8_t CheckEnableSignalsState(void)
{
    // ��ȡ����ʹ���źţ��͵�ƽ��Ч��
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    uint8_t active_count = k1_en + k2_en + k3_en;
    
    // ����Ƿ��г�ͻ����·ͬʱ���
    if (active_count > 1)
    {
        return 0; // ��ͻ
    }
    
    return 1; // ����
}

/**
 * @brief ���̵���״̬�ź�
 * @param None
 * @retval uint8_t 1-״̬��ȷ��0-״̬�쳣
 */
static uint8_t CheckRelayStatusSignals(void)
{
    SystemState_t expected = system_status.expected_state;
    
    // ��ȡ���м̵���״̬���ߵ�ƽ��Ч��
    uint8_t k1_1_sta = (HAL_GPIO_ReadPin(K1_1_STA_GPIO_Port, K1_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k1_2_sta = (HAL_GPIO_ReadPin(K1_2_STA_GPIO_Port, K1_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k2_1_sta = (HAL_GPIO_ReadPin(K2_1_STA_GPIO_Port, K2_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k2_2_sta = (HAL_GPIO_ReadPin(K2_2_STA_GPIO_Port, K2_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k3_1_sta = (HAL_GPIO_ReadPin(K3_1_STA_GPIO_Port, K3_1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t k3_2_sta = (HAL_GPIO_ReadPin(K3_2_STA_GPIO_Port, K3_2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    
    // ��������״̬���̵���״̬
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
    
    // ���״̬��ƥ�䣬������Ӧ���쳣
    if (k1_1_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_B);
    if (k2_1_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_C);
    if (k3_1_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_D);
    if (k1_2_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_E);
    if (k2_2_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_F);
    if (k3_2_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_G);
    
    return 0;
}

/**
 * @brief ���Ӵ���״̬�ź�
 * @param None
 * @retval uint8_t 1-״̬��ȷ��0-״̬�쳣
 */
static uint8_t CheckContactorStatusSignals(void)
{
    SystemState_t expected = system_status.expected_state;
    
    // ��ȡ���нӴ���״̬���ߵ�ƽ��Ч��
    uint8_t sw1_sta = (HAL_GPIO_ReadPin(SW1_STA_GPIO_Port, SW1_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t sw2_sta = (HAL_GPIO_ReadPin(SW2_STA_GPIO_Port, SW2_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t sw3_sta = (HAL_GPIO_ReadPin(SW3_STA_GPIO_Port, SW3_STA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    
    // ��������״̬���Ӵ���״̬
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
    
    // ���״̬��ƥ�䣬������Ӧ���쳣
    if (sw1_sta != ((expected == SYSTEM_CHANNEL1_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_H);
    if (sw2_sta != ((expected == SYSTEM_CHANNEL2_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_I);
    if (sw3_sta != ((expected == SYSTEM_CHANNEL3_ON) ? 1 : 0)) AlarmManager_SetAlarm(ALARM_J);
    
    return 0;
}

/**
 * @brief ����¶Ȱ�ȫ
 * @param None
 * @retval uint8_t 1-�¶Ȱ�ȫ��0-�¶��쳣
 */
static uint8_t CheckTemperatureSafety(void)
{
    float temp1, temp2, temp3;
    
    // ��ȡ��·�¶�
    TemperatureControl_GetTemperatures(&temp1, &temp2, &temp3);
    
    // ����Ƿ񳬹�60��
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
    
    return 1; // �����¶ȶ���ȫ
}

/**
 * @brief ���������̵���״̬
 * @param channel ͨ���ţ�1-3��
 * @param target_state Ŀ��״̬��1-������0-�رգ�
 * @retval uint8_t 1-����ɹ���0-����ʧ��
 */
static uint8_t CorrectSingleRelay(uint8_t channel, uint8_t target_state)
{
    ChannelType_t ch = (ChannelType_t)(channel - 1); // ת��Ϊö������
    
    if (target_state)
    {
        // ����ͨ��
        return RelayControl_OpenChannel(ch);
    }
    else
    {
        // �ر�ͨ��
        return RelayControl_CloseChannel(ch);
    }
}

/**
 * @brief �����Լ����
 * @param step ��ǰ�������ֵ
 * @retval None
 */
static void UpdateSelfCheckProgress(uint8_t step)
{
    selfcheck_result.progress = step;
}

/**
 * @brief ȷ������״̬
 * @param None
 * @retval SystemState_t ������ϵͳ״̬
 */
static SystemState_t DetermineExpectedState(void)
{
    // ��ȡʹ���źţ��͵�ƽ��Ч��
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // ����ʹ���ź����ȷ������״̬
    if (!k1_en && !k2_en && !k3_en)
    {
        return SYSTEM_ALL_OFF; // ȫ���ض�
    }
    else if (k1_en && !k2_en && !k3_en)
    {
        return SYSTEM_CHANNEL1_ON; // ͨ��1����
    }
    else if (!k1_en && k2_en && !k3_en)
    {
        return SYSTEM_CHANNEL2_ON; // ͨ��2����
    }
    else if (!k1_en && !k2_en && k3_en)
    {
        return SYSTEM_CHANNEL3_ON; // ͨ��3����
    }
    else
    {
        return SYSTEM_ERROR; // ����״̬����·ͬʱ���
    }
}

/**
 * @brief ��֤״̬һ����
 * @param None
 * @retval uint8_t 1-״̬һ�£�0-״̬��һ��
 */
static uint8_t ValidateStateConsistency(void)
{
    // �������״̬�Ƿ���Ч
    if (system_status.expected_state == SYSTEM_ERROR)
    {
        return 0;
    }
    
    // ���̵���״̬
    if (!CheckRelayStatusSignals())
    {
        return 0;
    }
    
    // ���Ӵ���״̬
    if (!CheckContactorStatusSignals())
    {
        return 0;
    }
    
    return 1; // ״̬һ��
} 


