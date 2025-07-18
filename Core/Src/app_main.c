/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_main.c
  * @brief          : Ӧ��������ģ��Դ�ļ�
  ******************************************************************************
  * @attention
  *
  * Ӧ��������ģ��ʵ�֣�
  * 1. ϵͳ״̬�����������Լ������������״̬����
  * 2. �������̣�Buhler LOGO��Minyer LOGO���Լ�����
  * 3. �������̣�ͨ�����ơ��¶ȹ���������������ʾ����
  * 4. ������ȣ���ģ�������ʱ��Э���͵���
  * 5. ������ϵͳ���ϼ��ͻָ�����
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_main.h"

/* �ڲ�״̬���� ---------------------------------------------------------------*/
static SystemRunState_t system_run_state = {
    .main_state = SYSTEM_STARTUP,
    .startup_substate = STARTUP_INIT,
    .working_substate = WORKING_MONITORING,
    .state_start_time = 0,
    .state_changed = 0,
    .system_ready = 0
};                                                      // ϵͳ����״̬

static TaskScheduleState_t task_schedule_state = {
    .last_relay_task_time = 0,
    .last_temp_task_time = 0,
    .last_alarm_task_time = 0,
    .last_display_task_time = 0,
    .last_monitor_task_time = 0
};                                                      // �������״̬

/* �ڲ��������� ---------------------------------------------------------------*/
static void InitializeAllModules(void);                // ��ʼ������ģ��
static void UpdateSystemState(void);                   // ����ϵͳ״̬
static void HandleChannelEnableSignals(void);          // ����ͨ��ʹ���ź�
static void HandleSystemMonitoring(void);              // ����ϵͳ���

/**
 * @brief Ӧ�������Ƴ�ʼ��
 * @param None
 * @retval None
 */
void AppMain_Init(void)
{
    // ��ʼ��ϵͳ����״̬
    system_run_state.main_state = SYSTEM_STARTUP;
    system_run_state.startup_substate = STARTUP_INIT;
    system_run_state.working_substate = WORKING_MONITORING;
    system_run_state.state_start_time = HAL_GetTick();
    system_run_state.state_changed = 1;
    system_run_state.system_ready = 0;
    
    // ��ʼ���������״̬
    uint32_t current_time = HAL_GetTick();
    task_schedule_state.last_relay_task_time = current_time;
    task_schedule_state.last_temp_task_time = current_time;
    task_schedule_state.last_alarm_task_time = current_time;
    task_schedule_state.last_display_task_time = current_time;
    task_schedule_state.last_monitor_task_time = current_time;
    
    // ��ʼ�����й���ģ��
    InitializeAllModules();
}

/**
 * @brief Ӧ�����������У���ѭ����
 * @param None
 * @retval None
 */
void AppMain_Run(void)
{
    // ��״̬������
    AppMain_StateMachine();
    
    // ���������
    AppMain_TaskScheduler();
    
    // ��ѭ����ʱ
    HAL_Delay(MAIN_LOOP_DELAY_MS);
}

/**
 * @brief Ӧ�������Ƹ�λ
 * @param None
 * @retval None
 */
void AppMain_Reset(void)
{
    // ��λϵͳ״̬
    system_run_state.main_state = SYSTEM_STARTUP;
    system_run_state.startup_substate = STARTUP_INIT;
    system_run_state.working_substate = WORKING_MONITORING;
    system_run_state.state_start_time = HAL_GetTick();
    system_run_state.state_changed = 1;
    system_run_state.system_ready = 0;
    
    // ���³�ʼ������ģ��
    InitializeAllModules();
}

/**
 * @brief ��״̬������
 * @param None
 * @retval None
 */
void AppMain_StateMachine(void)
{
    // ����ϵͳ״̬
    UpdateSystemState();
    
    // ������״ִ̬�ж�Ӧ����
    switch (system_run_state.main_state)
    {
        case SYSTEM_STARTUP:
            AppMain_StartupStateMachine();
            break;
            
        case SYSTEM_SELFCHECK:
            // ����ϵͳ�Լ�
            if (SystemMonitor_RunSelfCheck() == SELFCHECK_COMPLETE)
            {
                // �Լ���ɣ����빤��״̬
                AppMain_ChangeState(STATE_STANDBY);
            }
            else if (SystemMonitor_RunSelfCheck() == SELFCHECK_FAILED)
            {
                // �Լ�ʧ�ܣ��������״̬
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
 * @brief ����״̬������
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
 * @brief ����״̬������
 * @param None
 * @retval None
 */
void AppMain_WorkingStateMachine(void)
{
    // ���ϵͳ����״̬
    if (!AppMain_CheckSystemHealth())
    {
        AppMain_ChangeState(STATE_ERROR);
        return;
    }
    
    switch (system_run_state.working_substate)
    {
        case WORKING_MONITORING:
            // ϵͳ��ش���
            HandleSystemMonitoring();
            system_run_state.working_substate = WORKING_PROCESSING;
            break;
            
        case WORKING_PROCESSING:
            // ҵ���߼�����
            AppMain_ProcessChannelControl();
            AppMain_ProcessTemperatureControl();
            AppMain_ProcessAlarmHandling();
            system_run_state.working_substate = WORKING_DISPLAY_UPDATE;
            break;
            
        case WORKING_DISPLAY_UPDATE:
            // ��ʾ���´���
            AppMain_ProcessDisplayUpdate();
            system_run_state.working_substate = WORKING_MONITORING;
            break;
    }
}

/**
 * @brief ����״̬������
 * @param None
 * @retval None
 */
void AppMain_ErrorStateMachine(void)
{
    // �������״̬
    AppMain_HandleError();
    
    // ����ϵͳ�ָ�
    AppMain_RecoverFromError();
}

/**
 * @brief ���������
 * @param None
 * @retval None
 */
void AppMain_TaskScheduler(void)
{
    // ���ȼ̵�������
    AppMain_ScheduleRelayTask();
    
    // �����¶�����
    AppMain_ScheduleTemperatureTask();
    
    // ���ȱ�������
    AppMain_ScheduleAlarmTask();
    
    // ������ʾ����
    AppMain_ScheduleDisplayTask();
    
    // ���ȼ������
    AppMain_ScheduleMonitorTask();
}

/**
 * @brief ���ȼ̵�������
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
 * @brief �����¶�����
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
 * @brief ���ȱ�������
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
 * @brief ������ʾ����
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
 * @brief ���ȼ������
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
 * @brief �ı���״̬
 * @param new_state �µ�״̬
 * @retval None
 */
void AppMain_ChangeState(SystemState_t new_state)
{
    // ��SystemState_tת��ΪSystemMainState_t
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
            // ����״̬���⴦��
            system_run_state.main_state = SYSTEM_WORKING; // ��ʱ��Ϊ����״̬
            system_run_state.state_start_time = HAL_GetTick();
            system_run_state.state_changed = 1;
            // ����������߼�
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
 * @brief ��ȡ��ǰ��״̬
 * @param None
 * @retval SystemMainState_t ��ǰ��״̬
 */
SystemMainState_t AppMain_GetCurrentState(void)
{
    return system_run_state.main_state;
}

/**
 * @brief ��ȡ����״̬
 * @param None
 * @retval SystemRunState_t ϵͳ����״̬�ṹ��
 */
SystemRunState_t AppMain_GetRunState(void)
{
    return system_run_state;
}

/**
 * @brief ���ϵͳ�Ƿ����
 * @param None
 * @retval uint8_t 1-ϵͳ������0-ϵͳδ����
 */
uint8_t AppMain_IsSystemReady(void)
{
    return system_run_state.system_ready;
}

/**
 * @brief ����������ʼ��
 * @param None
 * @retval None
 */
void AppMain_HandleStartupInit(void)
{
    // ����OLED��ʾ��ʼ��
    OLEDDisplay_Init();
    
    // �л�����ʾBuhler LOGO
    system_run_state.startup_substate = STARTUP_BUHLER_LOGO;
    system_run_state.state_start_time = HAL_GetTick();
}

/**
 * @brief ����Buhler LOGO��ʾ
 * @param None
 * @retval None
 */
void AppMain_HandleBuhlerLogo(void)
{
    // ��ʾBuhler��˾LOGO
    if (system_run_state.state_changed)
    {
        OLEDDisplay_ShowStartupLogo();
        system_run_state.state_changed = 0;
    }
    
    // �����ʾʱ���Ƿ�ﵽ2��
    if (AppMain_GetStateElapsedTime() >= BUHLER_LOGO_DISPLAY_MS)
    {
        // �л�����ʾMinyer LOGO
        system_run_state.startup_substate = STARTUP_MINYER_LOGO;
        system_run_state.state_start_time = HAL_GetTick();
        system_run_state.state_changed = 1;
    }
}

/**
 * @brief ����Minyer LOGO��ʾ
 * @param None
 * @retval None
 */
void AppMain_HandleMinyerLogo(void)
{
    // ��ʾMinyer LOGO�ͽ�����
    if (system_run_state.state_changed)
    {
        OLEDDisplay_ShowStartupLogo();
        system_run_state.state_changed = 0;
    }
    
    // ����LOGO��ʾ�Զ���������������
    OLEDDisplay_ShowStartupLogo();
    
    // �����ʾʱ���Ƿ�ﵽ2��
    uint32_t elapsed_time = AppMain_GetStateElapsedTime();
    if (elapsed_time >= MINYER_LOGO_DISPLAY_MS)
    {
        // �����������
        system_run_state.startup_substate = STARTUP_COMPLETE;
        system_run_state.state_changed = 1;
    }
}

/**
 * @brief �����������
 * @param None
 * @retval None
 */
void AppMain_CompleteStartup(void)
{
    // ����������ɣ������Լ�״̬
    AppMain_ChangeState(STATE_SELFCHECK);
}

/**
 * @brief ������״̬
 * @param None
 * @retval None
 */
void AppMain_HandleWorking(void)
{
    // ����״̬�µĳ��洦��
    // ��������ڹ���״̬���б��ֽ�Ϊ��С�Ĵ�����
}

/**
 * @brief ����ͨ������
 * @param None
 * @retval None
 */
void AppMain_ProcessChannelControl(void)
{
    // ����ͨ��ʹ���ź�
    HandleChannelEnableSignals();
}

/**
 * @brief �����¶ȿ���
 * @param None
 * @retval None
 */
void AppMain_ProcessTemperatureControl(void)
{
    // �¶ȿ���������������д������������������߼�
    // ��������¶�״̬����ϵͳ��Ϊ
}

/**
 * @brief ����������
 * @param None
 * @retval None
 */
void AppMain_ProcessAlarmHandling(void)
{
    // ��������������������д������������������߼�
    // ������ݱ���״̬����ϵͳ��Ϊ
    
    // ����Ƿ������ر�����Ҫ�������״̬
    if (AlarmManager_HasAnyAlarm())
    {
        // ����Ƿ�Ϊϵͳ�����ع���
        if (AlarmManager_IsAlarmActive(ALARM_N) || // �Լ��쳣
            AlarmManager_IsAlarmActive(ALARM_O))   // ��Դ�쳣
        {
            // �������״̬
            AppMain_ChangeState(STATE_ERROR);
        }
    }
}

/**
 * @brief ������ʾ����
 * @param None
 * @retval None
 */
void AppMain_ProcessDisplayUpdate(void)
{
    // ��ʾ����������������д������������������߼�
    // �������ϵͳ״̬������ʾ����
}

/**
 * @brief �������״̬
 * @param None
 * @retval None
 */
void AppMain_HandleError(void)
{
    // �ڴ���״̬�£�ֹͣ���зǱ�Ҫ����
    // ֻ���ֱ�������ʾ����
    
    // ȷ�������������
    AlarmManager_Process();
    
    // ���´�����ʾ
    OLEDDisplay_ShowAlarmInterface();
}

/**
 * @brief �Ӵ���״̬�ָ�
 * @param None
 * @retval None
 */
void AppMain_RecoverFromError(void)
{
    // ����Ƿ���ԴӴ���״̬�ָ�
    if (AppMain_CheckSystemHealth())
    {
        // ������п�����ı���
        AlarmManager_ClearAllAlarms();
        
        // ���½��빤��״̬
        AppMain_ChangeState(STATE_STANDBY);
    }
}

/**
 * @brief ���ϵͳ����״̬
 * @param None
 * @retval uint8_t 1-ϵͳ������0-ϵͳ�쳣
 */
uint8_t AppMain_CheckSystemHealth(void)
{
    // ����Դ״̬
    if (!SystemMonitor_CheckPowerStatus())
    {
        return 0; // ��Դ�쳣
    }
    
    // ���ϵͳ���״̬
    if (!SystemMonitor_IsSystemNormal())
    {
        return 0; // ����쳣
    }
    
    // ����Ƿ������ر���
    if (AlarmManager_IsAlarmActive(ALARM_N) || // �Լ��쳣
        AlarmManager_IsAlarmActive(ALARM_O))   // ��Դ�쳣
    {
        return 0; // ���ر���
    }
    
    return 1; // ϵͳ����
}

/**
 * @brief ��ȡ��ǰ״̬����ʱ��
 * @param None
 * @retval uint32_t ״̬����ʱ�䣨���룩
 */
uint32_t AppMain_GetStateElapsedTime(void)
{
    return HAL_GetTick() - system_run_state.state_start_time;
}

/**
 * @brief ����״̬ʱ��
 * @param None
 * @retval None
 */
void AppMain_UpdateStateTime(void)
{
    system_run_state.state_start_time = HAL_GetTick();
}

/**
 * @brief �������ʱ���Ƿ񵽴�
 * @param last_time �ϴ�����ʱ��
 * @param period ��������
 * @retval uint8_t 1-ʱ�䵽�0-ʱ��δ��
 */
uint8_t AppMain_IsTaskTimeReached(uint32_t last_time, uint32_t period)
{
    uint32_t current_time = HAL_GetTick();
    return (current_time - last_time) >= period;
}

/* �ڲ�����ʵ�� ---------------------------------------------------------------*/

/**
 * @brief ��ʼ������ģ��
 * @param None
 * @retval None
 */
static void InitializeAllModules(void)
{
    // ��������˳���ʼ����ģ��
    
    // 1. �������ó�ʼ����CubeMX�����Ӳ����ʼ����
    
    // 2. ϵͳ���ó�ʼ��
    // SystemConfig_Init(); // �����Ҫ�Ļ�
    
    // 3. �̵������Ƴ�ʼ��
    RelayControl_Init();
    
    // 4. �¶ȿ��Ƴ�ʼ��
    TemperatureControl_Init();
    
    // 5. ���������ʼ��
    AlarmManager_Init();
    
    // 6. OLED��ʾ��ʼ������������������ɣ�
    // OLEDDisplay_Init();
    
    // 7. ϵͳ��س�ʼ��
    SystemMonitor_Init();
}

/**
 * @brief ����ϵͳ״̬
 * @param None
 * @retval None
 */
static void UpdateSystemState(void)
{
    // ���״̬�ı��־����״̬������ɺ�
    if (system_run_state.state_changed)
    {
        // ״̬�ı��־���ھ����״̬�����������
    }
}

/**
 * @brief ����ͨ��ʹ���ź�
 * @param None
 * @retval None
 */
static void HandleChannelEnableSignals(void)
{
    // ��ȡʹ���ź�״̬
    uint8_t k1_en = (HAL_GPIO_ReadPin(K1_EN_GPIO_Port, K1_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k2_en = (HAL_GPIO_ReadPin(K2_EN_GPIO_Port, K2_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t k3_en = (HAL_GPIO_ReadPin(K3_EN_GPIO_Port, K3_EN_Pin) == GPIO_PIN_RESET) ? 1 : 0;
    
    // ���ʹ���źų�ͻ
    uint8_t active_count = k1_en + k2_en + k3_en;
    if (active_count > 1)
    {
        // ��·ͬʱ�������A���쳣
        AlarmManager_SetAlarm(ALARM_A);
        return;
    }
    
    // ����ʹ���źſ��ƶ�Ӧͨ��
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
        // ����ͨ��������
        RelayControl_TurnOffChannel1();
        RelayControl_TurnOffChannel2();
        RelayControl_TurnOffChannel3();
    }
}

/**
 * @brief ����ϵͳ���
 * @param None
 * @retval None
 */
static void HandleSystemMonitoring(void)
{
    // ϵͳ�����Ҫ��SystemMonitor_Task�д���
    // ���������һЩӦ�ò�ļ���߼�
    
    // ����ϵͳ���
    SystemMonitor_UpdateMonitoring();
} 

