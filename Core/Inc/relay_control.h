/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : relay_control.h
  * @brief          : �̵�������ͷ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __RELAY_CONTROL_H
#define __RELAY_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"

/* �������� -------------------------------------------------------------------*/

// ��ʼ��
void RelayControl_Init(void);

// ʹ���źż�⣨��������
uint8_t RelayControl_ReadK1_EN(void);     // ��ȡK1_EN״̬
uint8_t RelayControl_ReadK2_EN(void);     // ��ȡK2_EN״̬  
uint8_t RelayControl_ReadK3_EN(void);     // ��ȡK3_EN״̬

// �̵���������ƣ�˫·ͬʱ����500ms���壩
void RelayControl_TurnOnChannel1(void);   // ����ͨ��1��K1_1_ON + K1_2_ONͬʱ���壩
void RelayControl_TurnOnChannel2(void);   // ����ͨ��2��K2_1_ON + K2_2_ONͬʱ���壩
void RelayControl_TurnOnChannel3(void);   // ����ͨ��3��K3_1_ON + K3_2_ONͬʱ���壩

void RelayControl_TurnOffChannel1(void);  // �ر�ͨ��1��K1_1_OFF + K1_2_OFFͬʱ���壩
void RelayControl_TurnOffChannel2(void);  // �ر�ͨ��2��K2_1_OFF + K2_2_OFFͬʱ���壩
void RelayControl_TurnOffChannel3(void);  // �ر�ͨ��3��K3_1_OFF + K3_2_OFFͬʱ���壩

// ״̬�������
uint8_t RelayControl_ReadK1_1_STA(void);  // ��ȡK1_1_STA״̬
uint8_t RelayControl_ReadK1_2_STA(void);  // ��ȡK1_2_STA״̬
uint8_t RelayControl_ReadK2_1_STA(void);  // ��ȡK2_1_STA״̬
uint8_t RelayControl_ReadK2_2_STA(void);  // ��ȡK2_2_STA״̬
uint8_t RelayControl_ReadK3_1_STA(void);  // ��ȡK3_1_STA״̬
uint8_t RelayControl_ReadK3_2_STA(void);  // ��ȡK3_2_STA״̬

// �Ӵ���״̬���
uint8_t RelayControl_ReadSW1_STA(void);   // ��ȡSW1_STA״̬
uint8_t RelayControl_ReadSW2_STA(void);   // ��ȡSW2_STA״̬
uint8_t RelayControl_ReadSW3_STA(void);   // ��ȡSW3_STA״̬

// ״̬��ȡ��װ����
uint8_t RelayControl_GetRelayStatus(Channel_t channel, uint8_t relay_num);  // ��ȡ�̵���״̬��1��2��
uint8_t RelayControl_GetContactorStatus(Channel_t channel);                 // ��ȡ�Ӵ���״̬
ChannelState_t RelayControl_GetChannelState(Channel_t channel);             // ��ȡͨ��״̬

// �����쳣���
AlarmType_t RelayControl_CheckEnableConflict(void);     // ���A���쳣��ʹ�ܳ�ͻ��
AlarmType_t RelayControl_CheckRelayStatus(Channel_t ch); // ���̵���״̬�쳣

// ��������
void RelayControl_Task(void);                           // �̵�������������

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_CONTROL_H */ 

 
