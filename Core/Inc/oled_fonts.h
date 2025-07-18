/**
 * @file oled_fonts.h
 * @brief OLED�����ͷ�ļ�
 * @details ����ASCII�ֿ���Զ����ַ����Թ������顢������ţ�
 * @author STM32�����Ŷ�
 * @date 2024
 */

#ifndef __OLED_FONTS_H
#define __OLED_FONTS_H

#include "main.h"

/* �����С���� */
#define FONT_6X8_WIDTH    6
#define FONT_6X8_HEIGHT   8
#define FONT_8X16_WIDTH   8
#define FONT_8X16_HEIGHT  16

/* �Զ����ַ����� */
#define CUSTOM_CHAR_CHECKMARK    0x01    // �Թ����� ?
#define CUSTOM_CHAR_SQUARE       0x02    // ʵ�ķ��� ��  
#define CUSTOM_CHAR_WARNING      0x03    // ������� ?

/* ASCII�ֿ����� - 6x8���� */
extern const unsigned char asc2_0806[][6];

/* ASCII�ֿ����� - 8x16���� */
extern const unsigned char asc2_1608[][16];

/* �Զ����ַ��ֿ����� - 8x16���� */
extern const unsigned char custom_chars_8x16[][16];

/* �Զ����ַ����� */
#define CUSTOM_CHAR_COUNT   3

#endif /* __OLED_FONTS_H */ 



