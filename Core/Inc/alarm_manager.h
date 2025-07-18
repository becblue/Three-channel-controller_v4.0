/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : alarm_manager.h
  * @brief          : ��������ϵͳͷ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ALARM_MANAGER_H
#define __ALARM_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"

/* ����������ģʽ���� ---------------------------------------------------------*/
typedef enum
{
    BEEP_MODE_OFF = 0,        // �رշ�����
    BEEP_MODE_1S_PULSE,       // 1�������壨A�ࡢN���쳣��
    BEEP_MODE_50MS_PULSE,     // 50ms������壨B~J���쳣��
    BEEP_MODE_CONTINUOUS      // �����͵�ƽ��K~M���쳣��
} BeepMode_t;

/* �������� -------------------------------------------------------------------*/

// ��ʼ��
void AlarmManager_Init(void);

// ��������
void AlarmManager_SetAlarm(AlarmType_t alarm_type);           // �����쳣����
void AlarmManager_ClearAlarm(AlarmType_t alarm_type);         // ����쳣����
void AlarmManager_ClearAllAlarms(void);                       // ��������쳣

// ����״̬
uint8_t AlarmManager_IsAlarmActive(AlarmType_t alarm_type);   // ����ض��쳣�Ƿ񼤻�
uint8_t AlarmManager_HasAnyAlarm(void);                       // ����Ƿ����κ��쳣

// �����������
void AlarmManager_SetAlarmOutput(uint8_t enable);             // ����ALARM�����PB4��
void AlarmManager_SetBeepMode(BeepMode_t mode);               // ���÷�����ģʽ

// ��������
void AlarmManager_Process(void);                              // ��������������

// ��������������
uint8_t AlarmManager_CheckAlarmClearCondition(AlarmType_t alarm_type);  // ��鱨���������

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_MANAGER_H */ 

