/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_main.h
  * @brief          : Ӧ��������ģ��ͷ�ļ�
  ******************************************************************************
  * @attention
  *
  * Ӧ��������ģ�鸺��
  * 1. ϵͳ״̬�������������Լ������������
  * 2. ��ѭ�������߼���������Ⱥ�ʱ����ƣ�
  * 3. ������ģ�����ϣ�ͳһ���ýӿڣ�
  * 4. ϵͳ��������ʵ�֣�����ҵ���߼���
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"
#include "oled_display.h"
#include "system_monitor.h"

/* ϵͳ��״̬ö�� ------------------------------------------------------------*/
typedef enum {
    SYSTEM_STARTUP = 0,      // ����״̬����ʾLOGO�ͳ�ʼ����
    SYSTEM_SELFCHECK,        // �Լ�״̬���Ĳ��Լ����̣�
    SYSTEM_WORKING           // ����״̬���������м�أ�
} SystemMainState_t;

/* ������״̬ö�� ------------------------------------------------------------*/
typedef enum {
    STARTUP_INIT = 0,        // ������ʼ��
    STARTUP_BUHLER_LOGO,     // ��ʾBuhler��˾LOGO
    STARTUP_MINYER_LOGO,     // ��ʾMinyer LOGO + ������
    STARTUP_COMPLETE         // �������
} StartupSubState_t;

/* ������״̬ö�� ------------------------------------------------------------*/
typedef enum {
    WORKING_MONITORING = 0,  // �������״̬
    WORKING_PROCESSING,      // ��������״̬
    WORKING_DISPLAY_UPDATE   // ��ʾ����״̬
} WorkingSubState_t;

/* ϵͳ����״̬�ṹ�� --------------------------------------------------------*/
typedef struct {
    SystemMainState_t main_state;        // ��״̬
    StartupSubState_t startup_substate;  // ������״̬
    WorkingSubState_t working_substate;  // ������״̬
    uint32_t state_start_time;          // ״̬��ʼʱ��
    uint8_t state_changed;              // ״̬�ı��־
    uint8_t system_ready;               // ϵͳ������־
} SystemRunState_t;

/* �������״̬�ṹ�� --------------------------------------------------------*/
typedef struct {
    uint32_t last_relay_task_time;      // �ϴμ̵�������ʱ��
    uint32_t last_temp_task_time;       // �ϴ��¶�����ʱ��
    uint32_t last_alarm_task_time;      // �ϴα�������ʱ��
    uint32_t last_display_task_time;    // �ϴ���ʾ����ʱ��
    uint32_t last_monitor_task_time;    // �ϴμ������ʱ��
} TaskScheduleState_t;

/* �궨�� -------------------------------------------------------------------*/
#define MAIN_LOOP_DELAY_MS          10   // ��ѭ����ʱ�����룩
#define BUHLER_LOGO_DISPLAY_MS      2000 // Buhler LOGO��ʾʱ�䣨���룩
#define MINYER_LOGO_DISPLAY_MS      2000 // Minyer LOGO��ʾʱ�䣨���룩

// ����������ڶ��壨���룩
#define RELAY_TASK_PERIOD_MS        50   // �̵�����������
#define TEMP_TASK_PERIOD_MS         100  // �¶���������
#define ALARM_TASK_PERIOD_MS        20   // ������������
#define DISPLAY_TASK_PERIOD_MS      100  // ��ʾ��������
#define MONITOR_TASK_PERIOD_MS      100  // �����������

/* �������� -----------------------------------------------------------------*/

// �����ƺ���
void AppMain_Init(void);                       // Ӧ�������Ƴ�ʼ��
void AppMain_Run(void);                        // Ӧ�����������У���ѭ����
void AppMain_Reset(void);                      // Ӧ�������Ƹ�λ

// ״̬������
void AppMain_StateMachine(void);               // ��״̬������
void AppMain_StartupStateMachine(void);        // ����״̬������
void AppMain_WorkingStateMachine(void);        // ����״̬������
void AppMain_ErrorStateMachine(void);          // ����״̬������

// ������Ⱥ���
void AppMain_TaskScheduler(void);              // ���������
void AppMain_ScheduleRelayTask(void);          // ���ȼ̵�������
void AppMain_ScheduleTemperatureTask(void);    // �����¶�����
void AppMain_ScheduleAlarmTask(void);          // ���ȱ�������
void AppMain_ScheduleDisplayTask(void);        // ������ʾ����
void AppMain_ScheduleMonitorTask(void);        // ���ȼ������

// ״̬������
void AppMain_ChangeState(SystemState_t new_state); // �ı���״̬
SystemMainState_t AppMain_GetCurrentState(void);       // ��ȡ��ǰ��״̬
SystemRunState_t AppMain_GetRunState(void);           // ��ȡ����״̬
uint8_t AppMain_IsSystemReady(void);                  // ���ϵͳ�Ƿ����

// �������̺���
void AppMain_HandleStartupInit(void);          // ����������ʼ��
void AppMain_HandleBuhlerLogo(void);           // ����Buhler LOGO��ʾ
void AppMain_HandleMinyerLogo(void);           // ����Minyer LOGO��ʾ
void AppMain_CompleteStartup(void);            // �����������

// �������̺���
void AppMain_HandleWorking(void);              // ������״̬
void AppMain_ProcessChannelControl(void);      // ����ͨ������
void AppMain_ProcessTemperatureControl(void);  // �����¶ȿ���
void AppMain_ProcessAlarmHandling(void);       // ����������
void AppMain_ProcessDisplayUpdate(void);       // ������ʾ����

// ��������
void AppMain_HandleError(void);                // �������״̬
void AppMain_RecoverFromError(void);           // �Ӵ���״̬�ָ�
uint8_t AppMain_CheckSystemHealth(void);       // ���ϵͳ����״̬

// ���ߺ���
uint32_t AppMain_GetStateElapsedTime(void);    // ��ȡ��ǰ״̬����ʱ��
void AppMain_UpdateStateTime(void);            // ����״̬ʱ��
uint8_t AppMain_IsTaskTimeReached(uint32_t last_time, uint32_t period); // �������ʱ���Ƿ񵽴�

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H */ 


