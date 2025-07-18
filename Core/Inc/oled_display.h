/**
 * @file oled_display.h
 * @brief OLED显示模块头文件
 * @details 2.42寸 128x64 SSD1309控制器 I2C接口OLED显示模块
 * @author STM32开发团队
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

/* OLED硬件配置 */
#define OLED_I2C_ADDRESS_7BIT    0x3C    // 7位I2C地址
#define OLED_I2C_ADDRESS_8BIT    0x78    // 8位I2C地址（写操作）
#define OLED_WIDTH               128     // OLED宽度像素
#define OLED_HEIGHT              64      // OLED高度像素
#define OLED_PAGES               8       // OLED页数（64/8）

/* 显示模式定义 */
#define OLED_CMD                 0       // 写命令
#define OLED_DATA                1       // 写数据
#define OLED_COLOR_NORMAL        0       // 正常显示
#define OLED_COLOR_REVERSE       1       // 反色显示

/* 字体大小定义 */
#define FONT_SIZE_6X8            8       // 6x8字体
#define FONT_SIZE_8X16           16      // 8x16字体

/* 显示界面枚举 */
typedef enum {
    OLED_INTERFACE_STARTUP = 0,          // 启动LOGO界面
    OLED_INTERFACE_SELFCHECK,            // 系统自检界面
    OLED_INTERFACE_NORMAL,               // 正常待机界面
    OLED_INTERFACE_ALARM                 // 异常显示界面
} OLEDInterface_t;

/* LOGO显示阶段枚举 */
typedef enum {
    LOGO_STAGE_BULHER = 0,               // BULHER LOGO阶段
    LOGO_STAGE_MINGYER,                  // MINGYER LOGO阶段
    LOGO_STAGE_COMPLETE                  // LOGO显示完成
} LogoStage_t;

/* 自检项目枚举 */
typedef enum {
    SELFCHECK_RELAY = 0,                 // 继电器测试
    SELFCHECK_NTC,                       // NTC传感器测试
    SELFCHECK_FLASH,                     // Flash存储器测试
    SELFCHECK_I2C,                       // I2C总线测试
    SELFCHECK_GPIO,                      // GPIO状态测试
    SELFCHECK_COMPLETE                   // 自检完成
} SelfCheckItem_t;

/* 自检状态枚举 */
typedef enum {
    SELFCHECK_STATUS_TESTING = 0,        // 正在测试
    SELFCHECK_STATUS_PASS,               // 测试通过
    SELFCHECK_STATUS_FAIL                // 测试失败
} SelfCheckStatus_t;

/* 显示管理结构体 */
typedef struct {
    OLEDInterface_t current_interface;   // 当前显示界面
    uint32_t interface_start_time;       // 界面开始时间
    LogoStage_t logo_stage;             // LOGO显示阶段
    SelfCheckItem_t selfcheck_item;     // 当前自检项目
    SelfCheckStatus_t selfcheck_status[6]; // 各项自检状态
    uint8_t refresh_flag;               // 刷新标志
} OLEDDisplay_t;

/* 全局变量声明 */
extern OLEDDisplay_t g_oled_display;

/* =================== 基础显示函数声明 =================== */

/**
 * @brief OLED初始化
 * @return HAL_StatusTypeDef 初始化状态
 */
HAL_StatusTypeDef OLED_Init(void);

/**
 * @brief 写命令到OLED
 * @param cmd 命令字节
 * @return HAL_StatusTypeDef 发送状态
 */
HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd);

/**
 * @brief 写数据到OLED
 * @param data 数据字节
 * @return HAL_StatusTypeDef 发送状态
 */
HAL_StatusTypeDef OLED_WriteData(uint8_t data);

/**
 * @brief 清屏
 */
void OLED_Clear(void);

/**
 * @brief 刷新显示缓存到OLED
 */
void OLED_Refresh(void);

/**
 * @brief 画点
 * @param x X坐标 (0-127)
 * @param y Y坐标 (0-63)
 * @param color 颜色 (0-清除, 1-点亮)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief 画线
 * @param x1 起始X坐标
 * @param y1 起始Y坐标
 * @param x2 结束X坐标
 * @param y2 结束Y坐标
 * @param color 颜色
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

/**
 * @brief 显示字符
 * @param x X坐标
 * @param y Y坐标
 * @param chr 字符
 * @param size 字体大小
 * @param mode 显示模式
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**
 * @brief 显示字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 字符串
 * @param size 字体大小
 * @param mode 显示模式
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode);

/**
 * @brief 显示数字
 * @param x X坐标
 * @param y Y坐标
 * @param num 数字
 * @param len 显示长度
 * @param size 字体大小
 * @param mode 显示模式
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);

/**
 * @brief 显示图片
 * @param x X坐标
 * @param y Y坐标
 * @param width 图片宽度
 * @param height 图片高度
 * @param bmp 图片数据
 * @param mode 显示模式
 */
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode);

/* =================== 界面管理函数声明 =================== */

/**
 * @brief 显示模块初始化
 */
void OLEDDisplay_Init(void);

/**
 * @brief 显示任务处理
 * @details 在主循环中调用，处理界面切换和显示更新
 */
void OLEDDisplay_Task(void);

/**
 * @brief 设置显示界面
 * @param interface 目标界面
 */
void OLEDDisplay_SetInterface(OLEDInterface_t interface);

/**
 * @brief 强制刷新显示
 */
void OLEDDisplay_ForceRefresh(void);

/* =================== 具体界面显示函数声明 =================== */

/**
 * @brief 显示启动LOGO界面
 */
void OLEDDisplay_ShowStartupLogo(void);

/**
 * @brief 显示系统自检界面
 */
void OLEDDisplay_ShowSelfCheck(void);

/**
 * @brief 显示正常待机界面
 */
void OLEDDisplay_ShowNormalInterface(void);

/**
 * @brief 显示异常报警界面
 */
void OLEDDisplay_ShowAlarmInterface(void);

/**
 * @brief 更新自检状态
 * @param item 自检项目
 * @param status 自检状态
 */
void OLEDDisplay_UpdateSelfCheckStatus(SelfCheckItem_t item, SelfCheckStatus_t status);

#endif /* __OLED_DISPLAY_H */ 


