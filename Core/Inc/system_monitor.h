/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : system_monitor.h
  * @brief          : ϵͳ���ģ��ͷ�ļ�
  ******************************************************************************
  * @attention
  *
  * ϵͳ���ģ�鸺��
  * 1. ϵͳ�Լ����̣��Ĳ����ܼ��;���
  * 2. ״̬��ع��ܣ�ʵʱ��ظ�����״̬��
  * 3. ����������ƣ����ִ����Զ�������
  * 4. ��Դ��أ�DC_CTRL��Դ״̬��⣩
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __SYSTEM_MONITOR_H
#define __SYSTEM_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"

/* �Լ�״̬ö�� --------------------------------------------------------------*/
typedef enum {
    SELFCHECK_IDLE = 0,      // ����״̬
    SELFCHECK_STEP1,         // ��һ����ʶ������״̬
    SELFCHECK_STEP2,         // �ڶ������̵���������
    SELFCHECK_STEP3,         // ���������Ӵ���������
    SELFCHECK_STEP4,         // ���Ĳ����¶ȼ��
    SELFCHECK_COMPLETE,      // �Լ����
    SELFCHECK_FAILED         // �Լ�ʧ��
} SelfCheckState_t;

/* ���״̬ö�� --------------------------------------------------------------*/
typedef enum {
    MONITOR_NORMAL = 0,      // �������״̬
    MONITOR_CORRECTING,      // ���ھ���״̬
    MONITOR_ERROR           // ��ش���״̬
} MonitorState_t;

/* �Լ����ṹ�� ------------------------------------------------------------*/
typedef struct {
    uint8_t step1_result;    // ��һ�������0-ʧ�ܣ�1-�ɹ�
    uint8_t step2_result;    // �ڶ��������0-ʧ�ܣ�1-�ɹ�
    uint8_t step3_result;    // �����������0-ʧ�ܣ�1-�ɹ�
    uint8_t step4_result;    // ���Ĳ������0-ʧ�ܣ�1-�ɹ�
    uint8_t overall_result;  // ��������0-ʧ�ܣ�1-�ɹ�
    uint8_t progress;        // �Լ���ȣ�0-100
} SelfCheckResult_t;

/* ϵͳ״̬�ṹ�� ------------------------------------------------------------*/
typedef struct {
    SystemState_t expected_state;    // ����״̬
    SystemState_t current_state;     // ��ǰ״̬
    uint8_t power_status;           // ��Դ״̬��0-�쳣��1-����
    uint8_t correction_count;       // �������
    MonitorState_t monitor_state;   // ���״̬
} SystemStatus_t;

/* �궨�� -------------------------------------------------------------------*/
#define SELFCHECK_MAX_RETRY     3    // ������Դ���
#define CORRECTION_DELAY_MS     500  // ������ʱ�����룩
#define MONITOR_UPDATE_MS       100  // ��ظ������ڣ����룩

/* �������� -----------------------------------------------------------------*/

// ��ʼ������
void SystemMonitor_Init(void);                          // ϵͳ��س�ʼ��

// �Լ칦�ܺ���
SelfCheckState_t SystemMonitor_RunSelfCheck(void);      // �����Լ�����
uint8_t SystemMonitor_GetSelfCheckProgress(void);       // ��ȡ�Լ����
SelfCheckResult_t SystemMonitor_GetSelfCheckResult(void); // ��ȡ�Լ���

// ״̬��غ���  
void SystemMonitor_UpdateMonitoring(void);              // ����״̬���
SystemStatus_t SystemMonitor_GetSystemStatus(void);     // ��ȡϵͳ״̬
uint8_t SystemMonitor_IsSystemNormal(void);             // ���ϵͳ�Ƿ�����

// ��Դ��غ���
uint8_t SystemMonitor_CheckPowerStatus(void);           // ����Դ״̬
void SystemMonitor_HandlePowerError(void);              // �����Դ�쳣

// ����������
uint8_t SystemMonitor_CorrectSystem(void);              // ��������ϵͳ
uint8_t SystemMonitor_CorrectRelayStates(void);         // �����̵���״̬
uint8_t SystemMonitor_CorrectContactorStates(void);     // �����Ӵ���״̬

// ״̬��麯��
uint8_t SystemMonitor_CheckExpectedState(void);         // �������״̬
uint8_t SystemMonitor_CheckAllStates(void);             // �������״̬
uint8_t SystemMonitor_ValidateSystemState(void);        // ��֤ϵͳ״̬

// ��������
void SystemMonitor_Task(void);                          // ϵͳ���������

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_MONITOR_H */ 


