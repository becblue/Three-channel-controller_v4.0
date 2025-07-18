/**
 * @file oled_display.h
 * @brief OLED��ʾģ��ͷ�ļ�
 * @details 2.42�� 128x64 SSD1309������ I2C�ӿ�OLED��ʾģ��
 * @author STM32�����Ŷ�
 * @date 2024
 */

#ifndef __OLED_DISPLAY_H
#define __OLED_DISPLAY_H

#include "main.h"
#include "i2c.h"
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"

/* OLEDӲ������ */
#define OLED_I2C_ADDRESS_7BIT    0x3C    // 7λI2C��ַ
#define OLED_I2C_ADDRESS_8BIT    0x78    // 8λI2C��ַ��д������
#define OLED_WIDTH               128     // OLED�������
#define OLED_HEIGHT              64      // OLED�߶�����
#define OLED_PAGES               8       // OLEDҳ����64/8��

/* ��ʾģʽ���� */
#define OLED_CMD                 0       // д����
#define OLED_DATA                1       // д����
#define OLED_COLOR_NORMAL        0       // ������ʾ
#define OLED_COLOR_REVERSE       1       // ��ɫ��ʾ

/* �����С���� */
#define FONT_SIZE_6X8            8       // 6x8����
#define FONT_SIZE_8X16           16      // 8x16����

/* ��ʾ����ö�� */
typedef enum {
    OLED_INTERFACE_STARTUP = 0,          // ����LOGO����
    OLED_INTERFACE_SELFCHECK,            // ϵͳ�Լ����
    OLED_INTERFACE_NORMAL,               // ������������
    OLED_INTERFACE_ALARM                 // �쳣��ʾ����
} OLEDInterface_t;

/* LOGO��ʾ�׶�ö�� */
typedef enum {
    LOGO_STAGE_BULHER = 0,               // BULHER LOGO�׶�
    LOGO_STAGE_MINGYER,                  // MINGYER LOGO�׶�
    LOGO_STAGE_COMPLETE                  // LOGO��ʾ���
} LogoStage_t;

/* �Լ���Ŀö�� */
typedef enum {
    SELFCHECK_RELAY = 0,                 // �̵�������
    SELFCHECK_NTC,                       // NTC����������
    SELFCHECK_FLASH,                     // Flash�洢������
    SELFCHECK_I2C,                       // I2C���߲���
    SELFCHECK_GPIO,                      // GPIO״̬����
    SELFCHECK_COMPLETE                   // �Լ����
} SelfCheckItem_t;

/* �Լ�״̬ö�� */
typedef enum {
    SELFCHECK_STATUS_TESTING = 0,        // ���ڲ���
    SELFCHECK_STATUS_PASS,               // ����ͨ��
    SELFCHECK_STATUS_FAIL                // ����ʧ��
} SelfCheckStatus_t;

/* ��ʾ����ṹ�� */
typedef struct {
    OLEDInterface_t current_interface;   // ��ǰ��ʾ����
    uint32_t interface_start_time;       // ���濪ʼʱ��
    LogoStage_t logo_stage;             // LOGO��ʾ�׶�
    SelfCheckItem_t selfcheck_item;     // ��ǰ�Լ���Ŀ
    SelfCheckStatus_t selfcheck_status[6]; // �����Լ�״̬
    uint8_t refresh_flag;               // ˢ�±�־
} OLEDDisplay_t;

/* ȫ�ֱ������� */
extern OLEDDisplay_t g_oled_display;

/* =================== ������ʾ�������� =================== */

/**
 * @brief OLED��ʼ��
 * @return HAL_StatusTypeDef ��ʼ��״̬
 */
HAL_StatusTypeDef OLED_Init(void);

/**
 * @brief д���OLED
 * @param cmd �����ֽ�
 * @return HAL_StatusTypeDef ����״̬
 */
HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd);

/**
 * @brief д���ݵ�OLED
 * @param data �����ֽ�
 * @return HAL_StatusTypeDef ����״̬
 */
HAL_StatusTypeDef OLED_WriteData(uint8_t data);

/**
 * @brief ����
 */
void OLED_Clear(void);

/**
 * @brief ˢ����ʾ���浽OLED
 */
void OLED_Refresh(void);

/**
 * @brief ����
 * @param x X���� (0-127)
 * @param y Y���� (0-63)
 * @param color ��ɫ (0-���, 1-����)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief ����
 * @param x1 ��ʼX����
 * @param y1 ��ʼY����
 * @param x2 ����X����
 * @param y2 ����Y����
 * @param color ��ɫ
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

/**
 * @brief ��ʾ�ַ�
 * @param x X����
 * @param y Y����
 * @param chr �ַ�
 * @param size �����С
 * @param mode ��ʾģʽ
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**
 * @brief ��ʾ�ַ���
 * @param x X����
 * @param y Y����
 * @param str �ַ���
 * @param size �����С
 * @param mode ��ʾģʽ
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode);

/**
 * @brief ��ʾ����
 * @param x X����
 * @param y Y����
 * @param num ����
 * @param len ��ʾ����
 * @param size �����С
 * @param mode ��ʾģʽ
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);

/**
 * @brief ��ʾͼƬ
 * @param x X����
 * @param y Y����
 * @param width ͼƬ���
 * @param height ͼƬ�߶�
 * @param bmp ͼƬ����
 * @param mode ��ʾģʽ
 */
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode);

/* =================== ������������� =================== */

/**
 * @brief ��ʾģ���ʼ��
 */
void OLEDDisplay_Init(void);

/**
 * @brief ��ʾ������
 * @details ����ѭ���е��ã���������л�����ʾ����
 */
void OLEDDisplay_Task(void);

/**
 * @brief ������ʾ����
 * @param interface Ŀ�����
 */
void OLEDDisplay_SetInterface(OLEDInterface_t interface);

/**
 * @brief ǿ��ˢ����ʾ
 */
void OLEDDisplay_ForceRefresh(void);

/* =================== ���������ʾ�������� =================== */

/**
 * @brief ��ʾ����LOGO����
 */
void OLEDDisplay_ShowStartupLogo(void);

/**
 * @brief ��ʾϵͳ�Լ����
 */
void OLEDDisplay_ShowSelfCheck(void);

/**
 * @brief ��ʾ������������
 */
void OLEDDisplay_ShowNormalInterface(void);

/**
 * @brief ��ʾ�쳣��������
 */
void OLEDDisplay_ShowAlarmInterface(void);

/**
 * @brief �����Լ�״̬
 * @param item �Լ���Ŀ
 * @param status �Լ�״̬
 */
void OLEDDisplay_UpdateSelfCheckStatus(SelfCheckItem_t item, SelfCheckStatus_t status);

#endif /* __OLED_DISPLAY_H */ 


