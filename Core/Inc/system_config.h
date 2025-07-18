/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_config.h
  * @brief          : ϵͳ����ͷ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* �쳣���Ͷ��� ---------------------------------------------------------------*/
typedef enum
{
    ALARM_NONE = 0,    // ���쳣
    ALARM_A,           // A���쳣 - ʹ�ܳ�ͻ
    ALARM_B,           // B���쳣 - K1_1_STA�쳣
    ALARM_C,           // C���쳣 - K2_1_STA�쳣
    ALARM_D,           // D���쳣 - K3_1_STA�쳣
    ALARM_E,           // E���쳣 - K1_2_STA�쳣
    ALARM_F,           // F���쳣 - K2_2_STA�쳣
    ALARM_G,           // G���쳣 - K3_2_STA�쳣
    ALARM_H,           // H���쳣 - SW1_STA�쳣
    ALARM_I,           // I���쳣 - SW2_STA�쳣
    ALARM_J,           // J���쳣 - SW3_STA�쳣
    ALARM_K,           // K���쳣 - NTC_1�¶��쳣
    ALARM_L,           // L���쳣 - NTC_2�¶��쳣
    ALARM_M,           // M���쳣 - NTC_3�¶��쳣
    ALARM_N,           // N���쳣 - �Լ��쳣
    ALARM_O            // O���쳣 - ��Դ����쳣
} AlarmType_t;

/* ϵͳ״̬���� ---------------------------------------------------------------*/
typedef enum
{
    STATE_STARTUP = 0,    // ϵͳ����
    STATE_LOGO,           // LOGO��ʾ
    STATE_SELFCHECK,      // ϵͳ�Լ�
    STATE_STANDBY,        // ϵͳ����
    STATE_CH1_ON,         // ͨ��1����
    STATE_CH2_ON,         // ͨ��2����
    STATE_CH3_ON,         // ͨ��3����
    STATE_ERROR,          // ϵͳ����
    
    // ���system_monitorģ����Ҫ��ö��ֵ
    SYSTEM_ALL_OFF = STATE_STANDBY,     // ȫ���ض�״̬
    SYSTEM_CHANNEL1_ON = STATE_CH1_ON,  // ͨ��1��״̬
    SYSTEM_CHANNEL2_ON = STATE_CH2_ON,  // ͨ��2��״̬
    SYSTEM_CHANNEL3_ON = STATE_CH3_ON,  // ͨ��3��״̬
    SYSTEM_ERROR = STATE_ERROR          // ����״̬
} SystemState_t;

/* ͨ������ -------------------------------------------------------------------*/
typedef enum
{
    CHANNEL_OFF = 0,      // �ر�
    CHANNEL_1,            // ͨ��1
    CHANNEL_2,            // ͨ��2
    CHANNEL_3,            // ͨ��3
    CHANNEL_ON            // ͨ������״̬
} Channel_t;

/* ͨ��״̬���� ---------------------------------------------------------------*/
typedef enum
{
    CHANNEL_OFF_STATE = 0,   // ͨ���ر�״̬
    CHANNEL_ON_STATE         // ͨ������״̬
} ChannelState_t;

/* ͨ�����Ͷ��� ---------------------------------------------------------------*/
typedef enum
{
    CHANNEL_TYPE_1 = 0,   // ͨ������1
    CHANNEL_TYPE_2,       // ͨ������2
    CHANNEL_TYPE_3        // ͨ������3
} ChannelType_t;

/* �¶ȴ��������� -------------------------------------------------------------*/
typedef enum
{
    TEMP_SENSOR_1 = 0,    // �¶ȴ�����1
    TEMP_SENSOR_2,        // �¶ȴ�����2
    TEMP_SENSOR_3,        // �¶ȴ�����3
    TEMP_SENSOR_COUNT     // ����������
} TempSensor_t;

/* ����ģʽ���� ---------------------------------------------------------------*/
typedef enum
{
    FAN_MODE_NORMAL = 0,  // ����ģʽ���¶�<35�棩
    FAN_MODE_HIGH_TEMP,   // ����ģʽ���¶ȡ�35�棩
    FAN_MODE_OVER_TEMP    // ����ģʽ���¶ȡ�60�棩
} FanMode_t;

/* ���ò��� -------------------------------------------------------------------*/
#define TEMP_NORMAL         35.0f     // �����¶���ֵ
#define TEMP_HIGH           60.0f     // ������ֵ
#define TEMP_ALARM_THRESHOLD 60.0f    // �����¶���ֵ����TEMP_HIGH����һ�£�
#define TEMP_HYSTERESIS     2.0f      // �¶Ȼز�

#define FAN_PWM_NORMAL      50        // ��������ռ�ձ�
#define FAN_PWM_HIGH        95        // ���·���ռ�ձ�

#define RELAY_PULSE_TIME    500       // �̵�������ʱ��ms
#define LOGO_DISPLAY_TIME   2000      // LOGO��ʾʱ��ms

/* ������س��� ---------------------------------------------------------------*/
#define ALARM_COUNT         15        // ������������(A-O)
#define MAX_ALARM_COUNT     10        // ���ͬʱ��Ծ��������

/* ȫ�ֱ��� -------------------------------------------------------------------*/
extern volatile AlarmType_t g_alarm;         // ��ǰ�쳣
extern volatile SystemState_t g_state;       // ��ǰ״̬
extern volatile Channel_t g_channel;         // ��ǰͨ��
extern volatile FanMode_t g_fan_mode;        // ��ǰ����ģʽ

/* �������� -------------------------------------------------------------------*/
void SystemConfig_Init(void);                // ��ʼ��

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CONFIG_H */

