/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature_control.h
  * @brief          : �¶ȿ���ͷ�ļ�
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TEMPERATURE_CONTROL_H
#define __TEMPERATURE_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "system_config.h"
#include "adc.h"
#include "tim.h"

/* �������� -------------------------------------------------------------------*/

// ��ʼ��
void TemperatureControl_Init(void);

// ADC���ݲɼ�
uint16_t TemperatureControl_GetAdcValue(uint8_t channel);  // ��ȡָ��ͨ��ADCֵ

// �¶ȼ���
float TemperatureControl_GetTemperature(uint8_t ntc_channel);  // ��ȡָ��NTC�¶�

// ���ȿ���
void TemperatureControl_SetFanDuty(uint8_t duty_percent);      // ����PWMռ�ձ�
uint16_t TemperatureControl_GetFanRpm(void);                   // ��ȡ����ת��

// �¶ȼ��
void TemperatureControl_Process(void);                         // �¶ȿ���������
AlarmType_t TemperatureControl_CheckTemperatureAlarm(void);    // ����¶��쳣

#ifdef __cplusplus
}
#endif

#endif /* __TEMPERATURE_CONTROL_H */ 

