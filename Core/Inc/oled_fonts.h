/**
 * @file oled_fonts.h
 * @brief OLED字体库头文件
 * @details 包含ASCII字库和自定义字符（对勾、方块、警告符号）
 * @author STM32开发团队
 * @date 2024
 */

#ifndef __OLED_FONTS_H
#define __OLED_FONTS_H

#include "main.h"

/* 字体大小定义 */
#define FONT_6X8_WIDTH    6
#define FONT_6X8_HEIGHT   8
#define FONT_8X16_WIDTH   8
#define FONT_8X16_HEIGHT  16

/* 自定义字符定义 */
#define CUSTOM_CHAR_CHECKMARK    0x01    // 对勾符号 ?
#define CUSTOM_CHAR_SQUARE       0x02    // 实心方块 ■  
#define CUSTOM_CHAR_WARNING      0x03    // 警告符号 ?

/* ASCII字库声明 - 6x8字体 */
extern const unsigned char asc2_0806[][6];

/* ASCII字库声明 - 8x16字体 */
extern const unsigned char asc2_1608[][16];

/* 自定义字符字库声明 - 8x16字体 */
extern const unsigned char custom_chars_8x16[][16];

/* 自定义字符数量 */
#define CUSTOM_CHAR_COUNT   3

#endif /* __OLED_FONTS_H */ 



