/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_config.c
  * @brief          : ϵͳ����Դ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#include "system_config.h"
#include "alarm_manager.h"

/* ȫ�ֱ������� ---------------------------------------------------------------*/
volatile AlarmType_t g_alarm = ALARM_NONE;        // ��ǰ�쳣
volatile SystemState_t g_state = STATE_STARTUP;   // ��ǰ״̬
volatile Channel_t g_channel = CHANNEL_OFF;       // ��ǰͨ��
volatile FanMode_t g_fan_mode = FAN_MODE_NORMAL;  // ��ǰ����ģʽ

/**
  * @brief  ϵͳ���ó�ʼ��
  * @param  None
  * @retval None
  */
void SystemConfig_Init(void)
{
    // ��ʼ��ȫ�ֱ���
    g_alarm = ALARM_NONE;
    g_state = STATE_STARTUP;
    g_channel = CHANNEL_OFF;
} 

 
