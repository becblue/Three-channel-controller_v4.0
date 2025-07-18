/**
 * @file oled_display.c
 * @brief OLED显示模块源文件
 * @details 2.42寸 128x64 SSD1309控制器 I2C接口OLED显示模块
 * @author STM32开发团队
 * @date 2024
 */

#include "oled_display.h"
#include "oled_fonts.h"
#include <string.h>
#include <stdio.h>

/* 外部LOGO图像数据声明 */
extern const unsigned char gImage_buhler_logo[438];
extern const unsigned char gImage_minyer_logo[411];

/* 全局变量定义 */
OLEDDisplay_t g_oled_display;                     // OLED显示管理结构体
static uint8_t g_oled_gram[OLED_WIDTH][OLED_PAGES]; // OLED显示缓存

/* =================== 基础I2C通信函数 =================== */

/**
 * @brief 写命令到OLED
 * @param cmd 命令字节
 * @return HAL_StatusTypeDef 发送状态
 */
HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd}; // 第一个字节0x00表示写命令
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, data, 2, 100);
}

/**
 * @brief 写数据到OLED
 * @param data 数据字节
 * @return HAL_StatusTypeDef 发送状态
 */
HAL_StatusTypeDef OLED_WriteData(uint8_t data)
{
    uint8_t buffer[2] = {0x40, data}; // 第一个字节0x40表示写数据
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, buffer, 2, 100);
}

/**
 * @brief 批量写数据到OLED
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return HAL_StatusTypeDef 发送状态
 */
static HAL_StatusTypeDef OLED_WriteDataBuffer(uint8_t *data, uint16_t len)
{
    uint8_t buffer[129]; // 最大128字节数据 + 1字节控制字节
    buffer[0] = 0x40;    // 控制字节，表示写数据
    memcpy(&buffer[1], data, len);
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, buffer, len + 1, 100);
}

/* =================== OLED硬件控制函数 =================== */

/**
 * @brief OLED初始化
 * @return HAL_StatusTypeDef 初始化状态
 */
HAL_StatusTypeDef OLED_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    // 延时等待OLED启动
    HAL_Delay(100);
    
    // SSD1309初始化序列
    status |= OLED_WriteCmd(0xFD); // 解锁命令
    status |= OLED_WriteCmd(0x12);
    status |= OLED_WriteCmd(0xAE); // 关闭显示
    status |= OLED_WriteCmd(0xD5); // 设置显示时钟分频
    status |= OLED_WriteCmd(0xA0);
    status |= OLED_WriteCmd(0xA8); // 设置多路复用比
    status |= OLED_WriteCmd(0x3F); // 1/64 占空比
    status |= OLED_WriteCmd(0xD3); // 设置显示偏移
    status |= OLED_WriteCmd(0x00); // 无偏移
    status |= OLED_WriteCmd(0x40); // 设置起始行地址
    status |= OLED_WriteCmd(0xA1); // 设置段重映射（水平翻转）
    status |= OLED_WriteCmd(0xC8); // 设置扫描方向（垂直翻转）
    status |= OLED_WriteCmd(0xDA); // 设置COM引脚配置
    status |= OLED_WriteCmd(0x12);
    status |= OLED_WriteCmd(0x81); // 设置对比度
    status |= OLED_WriteCmd(0x7F);
    status |= OLED_WriteCmd(0xD9); // 设置预充电周期
    status |= OLED_WriteCmd(0x82);
    status |= OLED_WriteCmd(0xDB); // 设置VCOM电压
    status |= OLED_WriteCmd(0x34);
    status |= OLED_WriteCmd(0xA4); // 关闭全屏点亮
    status |= OLED_WriteCmd(0xA6); // 设置正常显示（非反色）
    
    // 清屏并开启显示
    OLED_Clear();
    status |= OLED_WriteCmd(0xAF); // 开启显示
    
    return status;
}

/**
 * @brief 清屏
 */
void OLED_Clear(void)
{
    // 清空显示缓存
    memset(g_oled_gram, 0, sizeof(g_oled_gram));
    
    // 刷新到OLED
    OLED_Refresh();
}

/**
 * @brief 刷新显示缓存到OLED
 */
void OLED_Refresh(void)
{
    uint8_t page;
    
    for (page = 0; page < OLED_PAGES; page++)
    {
        // 设置页地址
        OLED_WriteCmd(0xB0 + page);
        // 设置列起始地址
        OLED_WriteCmd(0x00); // 低4位
        OLED_WriteCmd(0x10); // 高4位
        
        // 发送一页数据（128字节）
        OLED_WriteDataBuffer(g_oled_gram[0] + page * OLED_WIDTH, OLED_WIDTH);
    }
}

/* =================== 基础绘图函数 =================== */

/**
 * @brief 画点
 * @param x X坐标 (0-127)
 * @param y Y坐标 (0-63)
 * @param color 颜色 (0-清除, 1-点亮)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t page, bit;
    
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;
    
    page = y / 8;      // 计算页地址
    bit = y % 8;       // 计算位位置
    
    if (color)
    {
        g_oled_gram[x][page] |= (1 << bit);   // 点亮像素
    }
    else
    {
        g_oled_gram[x][page] &= ~(1 << bit);  // 清除像素
    }
}

/**
 * @brief 画线
 * @param x1 起始X坐标
 * @param y1 起始Y坐标
 * @param x2 结束X坐标
 * @param y2 结束Y坐标
 * @param color 颜色
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    
    delta_x = x2 - x1; // 计算坐标增量
    delta_y = y2 - y1;
    row = x1;
    col = y1;
    
    if (delta_x > 0) incx = 1;      // 设置单步方向
    else if (delta_x == 0) incx = 0; // 垂直线
    else { incx = -1; delta_x = -delta_x; }
    
    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0; // 水平线
    else { incy = -1; delta_y = -delta_y; }
    
    if (delta_x > delta_y) distance = delta_x; // 选取基本增量坐标轴
    else distance = delta_y;
    
    for (t = 0; t <= distance + 1; t++) // 画线输出
    {
        OLED_DrawPoint(row, col, color); // 画点
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/* =================== 字符显示函数 =================== */

/**
 * @brief 显示字符
 * @param x X坐标
 * @param y Y坐标
 * @param chr 字符
 * @param size 字体大小 (8 or 16)
 * @param mode 显示模式 (0-正常, 1-反色)
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 字符所占字节数
    
    chr = chr - ' '; // 得到偏移后的值
    
    for (t = 0; t < csize; t++)
    {
        if (size == 8)
        {
            temp = asc2_0806[chr][t]; // 调用0806字库
        }
        else if (size == 16)
        {
            temp = asc2_1608[chr][t]; // 调用1608字库
        }
        else
        {
            return; // 不支持的字体大小
        }
        
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                OLED_DrawPoint(x, y, mode);
            }
            else
            {
                OLED_DrawPoint(x, y, !mode);
            }
            temp <<= 1;
            y++;
            if (y >= OLED_HEIGHT) return; // 超出屏幕范围
        }
        x++;
        if ((size != 8) && ((x - (x - t * (size / 2))) == size / 2))
        {
            x = x - size / 2;
            y0 = y0 + 8;
        }
        y = y0;
        if (x >= OLED_WIDTH) return; // 超出屏幕范围
    }
}

/**
 * @brief 显示自定义字符
 * @param x X坐标
 * @param y Y坐标
 * @param char_code 自定义字符代码
 * @param mode 显示模式
 */
void OLED_ShowCustomChar(uint8_t x, uint8_t y, uint8_t char_code, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    
    if (char_code == 0 || char_code > CUSTOM_CHAR_COUNT)
        return;
    
    for (t = 0; t < 16; t++) // 自定义字符固定为8x16
    {
        temp = custom_chars_8x16[char_code - 1][t];
        
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                OLED_DrawPoint(x, y, mode);
            }
            else
            {
                OLED_DrawPoint(x, y, !mode);
            }
            temp <<= 1;
            y++;
            if (y >= OLED_HEIGHT) return;
        }
        x++;
        if ((x - (x - t * 4)) == 8) // 8x16字符处理
        {
            x = x - 8;
            y0 = y0 + 8;
        }
        y = y0;
        if (x >= OLED_WIDTH) return;
    }
}

/**
 * @brief 显示字符串
 * @param x X坐标
 * @param y Y坐标
 * @param str 字符串
 * @param size 字体大小
 * @param mode 显示模式
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode)
{
    uint8_t x0 = x;
    
    while ((*str <= '~') && (*str >= ' ')) // 判断是否非法字符
    {
        if (x > (OLED_WIDTH - size / 2)) // 换行处理
        {
            x = x0;
            y += size;
        }
        if (y > (OLED_HEIGHT - size)) // 超出屏幕范围
        {
            break;
        }
        
        OLED_ShowChar(x, y, *str, size, mode);
        
        if (size == 8)
        {
            x += FONT_6X8_WIDTH;
        }
        else
        {
            x += FONT_8X16_WIDTH;
        }
        str++;
    }
}

/**
 * @brief 计算幂运算 m^n
 * @param m 底数
 * @param n 指数
 * @return uint32_t 结果
 */
static uint32_t OLED_Pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--)
    {
        result *= m;
    }
    return result;
}

/**
 * @brief 显示数字
 * @param x X坐标
 * @param y Y坐标
 * @param num 数字
 * @param len 显示长度
 * @param size 字体大小
 * @param mode 显示模式
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + (size / 2) * t, y, ' ', size, mode);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }
        OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode);
    }
}

/**
 * @brief 显示图片
 * @param x X坐标
 * @param y Y坐标
 * @param width 图片宽度
 * @param height 图片高度
 * @param bmp 图片数据
 * @param mode 显示模式
 */
void OLED_ShowPicture(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp, uint8_t mode)
{
    uint16_t j = 0;
    uint8_t i, n, temp, m;
    uint8_t x0 = x, y0 = y;
    uint8_t sizey = height / 8 + ((height % 8) ? 1 : 0);
    
    for (n = 0; n < sizey; n++)
    {
        for (i = 0; i < width; i++)
        {
            temp = bmp[j];
            j++;
            for (m = 0; m < 8; m++)
            {
                if (temp & 0x01)
                {
                    OLED_DrawPoint(x, y, mode);
                }
                else
                {
                    OLED_DrawPoint(x, y, !mode);
                }
                temp >>= 1;
                y++;
            }
            x++;
            if ((x - x0) == width)
            {
                x = x0;
                y0 = y0 + 8;
            }
            y = y0;
        }
    }
}

/* =================== 界面管理函数 =================== */

/**
 * @brief 显示模块初始化
 */
void OLEDDisplay_Init(void)
{
    // 初始化OLED硬件
    OLED_Init();
    
    // 初始化显示管理结构体
    memset(&g_oled_display, 0, sizeof(g_oled_display));
    g_oled_display.current_interface = OLED_INTERFACE_STARTUP;
    g_oled_display.interface_start_time = HAL_GetTick();
    g_oled_display.logo_stage = LOGO_STAGE_BULHER;
    g_oled_display.selfcheck_item = SELFCHECK_RELAY;
    g_oled_display.refresh_flag = 1;
    
    // 初始化自检状态为测试中
    for (int i = 0; i < 6; i++)
    {
        g_oled_display.selfcheck_status[i] = SELFCHECK_STATUS_TESTING;
    }
}

/**
 * @brief 设置显示界面
 * @param interface 目标界面
 */
void OLEDDisplay_SetInterface(OLEDInterface_t interface)
{
    if (g_oled_display.current_interface != interface)
    {
        g_oled_display.current_interface = interface;
        g_oled_display.interface_start_time = HAL_GetTick();
        g_oled_display.refresh_flag = 1;
    }
}

/**
 * @brief 强制刷新显示
 */
void OLEDDisplay_ForceRefresh(void)
{
    g_oled_display.refresh_flag = 1;
}

/**
 * @brief 更新自检状态
 * @param item 自检项目
 * @param status 自检状态
 */
void OLEDDisplay_UpdateSelfCheckStatus(SelfCheckItem_t item, SelfCheckStatus_t status)
{
    if (item < SELFCHECK_COMPLETE)
    {
        g_oled_display.selfcheck_status[item] = status;
        g_oled_display.refresh_flag = 1;
    }
}

/**
 * @brief 显示任务处理
 * @details 在主循环中调用，处理界面切换和显示更新
 */
void OLEDDisplay_Task(void)
{
    static uint32_t last_refresh_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // 界面自动切换逻辑
    switch (g_oled_display.current_interface)
    {
        case OLED_INTERFACE_STARTUP:
            // 启动界面显示6秒后切换到自检界面
            if (current_time - g_oled_display.interface_start_time > 6000)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_SELFCHECK);
            }
            break;
            
        case OLED_INTERFACE_SELFCHECK:
            // 自检完成后切换到正常界面
            if (g_oled_display.selfcheck_item >= SELFCHECK_COMPLETE)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_NORMAL);
            }
            break;
            
        case OLED_INTERFACE_NORMAL:
            // 检查是否有报警，如有则切换到报警界面
            if (AlarmManager_GetAlarmCount() > 0)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_ALARM);
            }
            break;
            
        case OLED_INTERFACE_ALARM:
            // 检查报警是否清除，如清除则切换回正常界面
            if (AlarmManager_GetAlarmCount() == 0)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_NORMAL);
            }
            break;
    }
    
    // 定时刷新或强制刷新
    if (g_oled_display.refresh_flag || (current_time - last_refresh_time > 1000))
    {
        // 清屏
        OLED_Clear();
        
        // 根据当前界面显示内容
        switch (g_oled_display.current_interface)
        {
            case OLED_INTERFACE_STARTUP:
                OLEDDisplay_ShowStartupLogo();
                break;
                
            case OLED_INTERFACE_SELFCHECK:
                OLEDDisplay_ShowSelfCheck();
                break;
                
            case OLED_INTERFACE_NORMAL:
                OLEDDisplay_ShowNormalInterface();
                break;
                
            case OLED_INTERFACE_ALARM:
                OLEDDisplay_ShowAlarmInterface();
                break;
        }
        
        // 刷新到OLED
        OLED_Refresh();
        
        g_oled_display.refresh_flag = 0;
        last_refresh_time = current_time;
    }
}

/* =================== 具体界面显示函数 =================== */

/**
 * @brief 显示启动LOGO界面
 */
void OLEDDisplay_ShowStartupLogo(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed_time = current_time - g_oled_display.interface_start_time;
    
    if (elapsed_time < 3000)
    {
        // 显示BUHLER LOGO (前3秒)
        g_oled_display.logo_stage = LOGO_STAGE_BULHER;
        
        // 居中显示BUHLER LOGO图像
        // 假设图像尺寸为96x27像素，居中显示
        OLED_ShowPicture(16, 18, 96, 27, &gImage_buhler_logo[6], OLED_COLOR_NORMAL);
    }
    else if (elapsed_time < 6000)
    {
        // 显示mingyer LOGO + 进度条 (后3秒)
        g_oled_display.logo_stage = LOGO_STAGE_MINGYER;
        
        // 居中显示mingyer LOGO图像
        // 假设图像尺寸为115x27像素，居中显示
        OLED_ShowPicture(6, 8, 115, 27, &gImage_minyer_logo[6], OLED_COLOR_NORMAL);
        
        // 显示进度条
        uint8_t progress = ((elapsed_time - 3000) * 100) / 3000; // 计算进度百分比
        uint8_t bar_width = (progress * 100) / 100; // 进度条宽度
        
        // 绘制进度条边框
        OLED_DrawLine(14, 40, 113, 40, OLED_COLOR_NORMAL); // 上边
        OLED_DrawLine(14, 47, 113, 47, OLED_COLOR_NORMAL); // 下边
        OLED_DrawLine(14, 40, 14, 47, OLED_COLOR_NORMAL);  // 左边
        OLED_DrawLine(113, 40, 113, 47, OLED_COLOR_NORMAL); // 右边
        
        // 填充进度条
        for (uint8_t i = 0; i < bar_width && i < 99; i++)
        {
            OLED_DrawLine(15 + i, 41, 15 + i, 46, OLED_COLOR_NORMAL);
        }
    }
    else
    {
        g_oled_display.logo_stage = LOGO_STAGE_COMPLETE;
    }
}

/**
 * @brief 显示系统自检界面
 */
void OLEDDisplay_ShowSelfCheck(void)
{
    // 标题
    OLED_ShowString(25, 0, "System Self-Test", 8, OLED_COLOR_NORMAL);
    
    // 绘制分隔线
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // 自检项目列表
    const char* selfcheck_items[] = {
        "Relay Test",
        "NTC Sensor Test", 
        "Flash Memory",
        "I2C Bus",
        "GPIO Status"
    };
    
    // 显示自检项目和状态
    for (int i = 0; i < 5; i++)
    {
        uint8_t y_pos = 16 + i * 8;
        
        // 显示项目名称
        OLED_ShowString(0, y_pos, selfcheck_items[i], 8, OLED_COLOR_NORMAL);
        
        // 显示状态符号
        if (g_oled_display.selfcheck_status[i] == SELFCHECK_STATUS_PASS)
        {
            // 显示对勾符号 ?
            OLED_ShowCustomChar(110, y_pos - 4, CUSTOM_CHAR_CHECKMARK, OLED_COLOR_NORMAL);
        }
        else if (g_oled_display.selfcheck_status[i] == SELFCHECK_STATUS_FAIL)
        {
            // 显示实心方块 ■
            OLED_ShowCustomChar(110, y_pos - 4, CUSTOM_CHAR_SQUARE, OLED_COLOR_NORMAL);
        }
        else
        {
            // 测试中，显示空白
            OLED_ShowString(110, y_pos, " ", 8, OLED_COLOR_NORMAL);
        }
    }
    
    // 绘制分隔线
    OLED_DrawLine(0, 56, 127, 56, OLED_COLOR_NORMAL);
    
    // 显示状态信息
    if (g_oled_display.selfcheck_item < SELFCHECK_COMPLETE)
    {
        OLED_ShowString(0, 58, "Self-Test Running...", 8, OLED_COLOR_NORMAL);
    }
    else
    {
        OLED_ShowString(0, 58, "Self-Test Complete", 8, OLED_COLOR_NORMAL);
    }
}

/**
 * @brief 显示正常待机界面
 */
void OLEDDisplay_ShowNormalInterface(void)
{
    char buffer[32];
    
    // 标题
    OLED_ShowString(25, 0, "System Normal", 8, OLED_COLOR_NORMAL);
    
    // 绘制分隔线
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // 通道状态显示
    ChannelType_t ch1_state = RelayControl_GetChannelState(CHANNEL_1);
    ChannelType_t ch2_state = RelayControl_GetChannelState(CHANNEL_2);
    ChannelType_t ch3_state = RelayControl_GetChannelState(CHANNEL_3);
    
    sprintf(buffer, "CH1: [%s]", (ch1_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 16, buffer, 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "CH2: [%s]", (ch2_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 26, buffer, 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "CH3: [%s]", (ch3_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 36, buffer, 8, OLED_COLOR_NORMAL);
    
    // 绘制分隔线
    OLED_DrawLine(0, 48, 127, 48, OLED_COLOR_NORMAL);
    
    // 温度和风扇信息
    float temp1 = TemperatureControl_GetTemperature(0);  // NTC_1
    float temp2 = TemperatureControl_GetTemperature(1);  // NTC_2
    float temp3 = TemperatureControl_GetTemperature(2);  // NTC_3
    uint8_t duty = TemperatureControl_GetFanDuty();
    uint16_t rpm = TemperatureControl_GetFanRPM();
    
    sprintf(buffer, "T1:%d", (int)temp1);
    OLED_ShowString(0, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(18, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "T2:%d", (int)temp2);
    OLED_ShowString(30, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(48, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "T3:%d", (int)temp3);
    OLED_ShowString(60, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(78, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "Duty: %d%%  FAN:%dRPM", duty, rpm);
    OLED_ShowString(0, 58, buffer, 8, OLED_COLOR_NORMAL);
}

/**
 * @brief 显示异常报警界面
 */
void OLEDDisplay_ShowAlarmInterface(void)
{
    char buffer[32];
    AlarmType_t active_alarms[MAX_ALARM_COUNT];
    uint8_t alarm_count = AlarmManager_GetActiveAlarms(active_alarms, MAX_ALARM_COUNT);
    
    // 警告符号和标题
    OLED_ShowCustomChar(0, 0, CUSTOM_CHAR_WARNING, OLED_COLOR_NORMAL);
    
    if (alarm_count > 1)
    {
        sprintf(buffer, "ALARM - Multiple");
        OLED_ShowString(12, 0, buffer, 8, OLED_COLOR_NORMAL);
    }
    else if (alarm_count == 1)
    {
        sprintf(buffer, "ALARM_%c", 'A' + active_alarms[0]);
        OLED_ShowString(12, 0, buffer, 8, OLED_COLOR_NORMAL);
    }
    
    OLED_ShowCustomChar(115, 0, CUSTOM_CHAR_WARNING, OLED_COLOR_NORMAL);
    
    // 绘制分隔线
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // 显示前3个报警信息
    const char* alarm_messages[] = {
        "ERR:A EN conflict",     // ALARM_A
        "ERR:B K1_1 fail",       // ALARM_B
        "ERR:C K2_1 fail",       // ALARM_C
        "ERR:D K3_1 fail",       // ALARM_D
        "ERR:E K1_2 fail",       // ALARM_E
        "ERR:F K2_2 fail",       // ALARM_F
        "ERR:G K3_2 fail",       // ALARM_G
        "ERR:H SW1 fault",       // ALARM_H
        "ERR:I SW2 fault",       // ALARM_I
        "ERR:J SW3 fault",       // ALARM_J
        "ERR:K NTC1>60C",        // ALARM_K
        "ERR:L NTC2>60C",        // ALARM_L
        "ERR:M NTC3>60C",        // ALARM_M
        "ERR:N Self-check",      // ALARM_N
        "ERR:O Power Fault"      // ALARM_O
    };
    
    uint8_t display_count = (alarm_count > 3) ? 3 : alarm_count;
    for (uint8_t i = 0; i < display_count; i++)
    {
        uint8_t y_pos = 16 + i * 8;
        if (active_alarms[i] < ALARM_COUNT)
        {
            OLED_ShowString(0, y_pos, alarm_messages[active_alarms[i]], 8, OLED_COLOR_NORMAL);
        }
    }
    
    // 绘制分隔线
    OLED_DrawLine(0, 48, 127, 48, OLED_COLOR_NORMAL);
    
    // 温度和风扇信息（与正常界面相同）
    float temp1 = TemperatureControl_GetTemperature(0);  // NTC_1
    float temp2 = TemperatureControl_GetTemperature(1);  // NTC_2
    float temp3 = TemperatureControl_GetTemperature(2);  // NTC_3
    uint8_t duty = TemperatureControl_GetFanDuty();
    uint16_t rpm = TemperatureControl_GetFanRPM();
    
    sprintf(buffer, "T1:%d", (int)temp1);
    OLED_ShowString(0, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(18, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "T2:%d", (int)temp2);
    OLED_ShowString(30, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(48, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "T3:%d", (int)temp3);
    OLED_ShowString(60, 52, buffer, 8, OLED_COLOR_NORMAL);
    OLED_ShowString(78, 52, "C", 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "Duty: %d%%  FAN:%dRPM", duty, rpm);
    OLED_ShowString(0, 58, buffer, 8, OLED_COLOR_NORMAL);
} 


