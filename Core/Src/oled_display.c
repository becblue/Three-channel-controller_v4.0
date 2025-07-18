/**
 * @file oled_display.c
 * @brief OLED��ʾģ��Դ�ļ�
 * @details 2.42�� 128x64 SSD1309������ I2C�ӿ�OLED��ʾģ��
 * @author STM32�����Ŷ�
 * @date 2024
 */

#include "oled_display.h"
#include "oled_fonts.h"
#include <string.h>
#include <stdio.h>

/* �ⲿLOGOͼ���������� */
extern const unsigned char gImage_buhler_logo[438];
extern const unsigned char gImage_minyer_logo[411];

/* ȫ�ֱ������� */
OLEDDisplay_t g_oled_display;                     // OLED��ʾ����ṹ��
static uint8_t g_oled_gram[OLED_WIDTH][OLED_PAGES]; // OLED��ʾ����

/* =================== ����I2Cͨ�ź��� =================== */

/**
 * @brief д���OLED
 * @param cmd �����ֽ�
 * @return HAL_StatusTypeDef ����״̬
 */
HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd}; // ��һ���ֽ�0x00��ʾд����
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, data, 2, 100);
}

/**
 * @brief д���ݵ�OLED
 * @param data �����ֽ�
 * @return HAL_StatusTypeDef ����״̬
 */
HAL_StatusTypeDef OLED_WriteData(uint8_t data)
{
    uint8_t buffer[2] = {0x40, data}; // ��һ���ֽ�0x40��ʾд����
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, buffer, 2, 100);
}

/**
 * @brief ����д���ݵ�OLED
 * @param data ���ݻ�����
 * @param len ���ݳ���
 * @return HAL_StatusTypeDef ����״̬
 */
static HAL_StatusTypeDef OLED_WriteDataBuffer(uint8_t *data, uint16_t len)
{
    uint8_t buffer[129]; // ���128�ֽ����� + 1�ֽڿ����ֽ�
    buffer[0] = 0x40;    // �����ֽڣ���ʾд����
    memcpy(&buffer[1], data, len);
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS_8BIT, buffer, len + 1, 100);
}

/* =================== OLEDӲ�����ƺ��� =================== */

/**
 * @brief OLED��ʼ��
 * @return HAL_StatusTypeDef ��ʼ��״̬
 */
HAL_StatusTypeDef OLED_Init(void)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    // ��ʱ�ȴ�OLED����
    HAL_Delay(100);
    
    // SSD1309��ʼ������
    status |= OLED_WriteCmd(0xFD); // ��������
    status |= OLED_WriteCmd(0x12);
    status |= OLED_WriteCmd(0xAE); // �ر���ʾ
    status |= OLED_WriteCmd(0xD5); // ������ʾʱ�ӷ�Ƶ
    status |= OLED_WriteCmd(0xA0);
    status |= OLED_WriteCmd(0xA8); // ���ö�·���ñ�
    status |= OLED_WriteCmd(0x3F); // 1/64 ռ�ձ�
    status |= OLED_WriteCmd(0xD3); // ������ʾƫ��
    status |= OLED_WriteCmd(0x00); // ��ƫ��
    status |= OLED_WriteCmd(0x40); // ������ʼ�е�ַ
    status |= OLED_WriteCmd(0xA1); // ���ö���ӳ�䣨ˮƽ��ת��
    status |= OLED_WriteCmd(0xC8); // ����ɨ�跽�򣨴�ֱ��ת��
    status |= OLED_WriteCmd(0xDA); // ����COM��������
    status |= OLED_WriteCmd(0x12);
    status |= OLED_WriteCmd(0x81); // ���öԱȶ�
    status |= OLED_WriteCmd(0x7F);
    status |= OLED_WriteCmd(0xD9); // ����Ԥ�������
    status |= OLED_WriteCmd(0x82);
    status |= OLED_WriteCmd(0xDB); // ����VCOM��ѹ
    status |= OLED_WriteCmd(0x34);
    status |= OLED_WriteCmd(0xA4); // �ر�ȫ������
    status |= OLED_WriteCmd(0xA6); // ����������ʾ���Ƿ�ɫ��
    
    // ������������ʾ
    OLED_Clear();
    status |= OLED_WriteCmd(0xAF); // ������ʾ
    
    return status;
}

/**
 * @brief ����
 */
void OLED_Clear(void)
{
    // �����ʾ����
    memset(g_oled_gram, 0, sizeof(g_oled_gram));
    
    // ˢ�µ�OLED
    OLED_Refresh();
}

/**
 * @brief ˢ����ʾ���浽OLED
 */
void OLED_Refresh(void)
{
    uint8_t page;
    
    for (page = 0; page < OLED_PAGES; page++)
    {
        // ����ҳ��ַ
        OLED_WriteCmd(0xB0 + page);
        // ��������ʼ��ַ
        OLED_WriteCmd(0x00); // ��4λ
        OLED_WriteCmd(0x10); // ��4λ
        
        // ����һҳ���ݣ�128�ֽڣ�
        OLED_WriteDataBuffer(g_oled_gram[0] + page * OLED_WIDTH, OLED_WIDTH);
    }
}

/* =================== ������ͼ���� =================== */

/**
 * @brief ����
 * @param x X���� (0-127)
 * @param y Y���� (0-63)
 * @param color ��ɫ (0-���, 1-����)
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t page, bit;
    
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;
    
    page = y / 8;      // ����ҳ��ַ
    bit = y % 8;       // ����λλ��
    
    if (color)
    {
        g_oled_gram[x][page] |= (1 << bit);   // ��������
    }
    else
    {
        g_oled_gram[x][page] &= ~(1 << bit);  // �������
    }
}

/**
 * @brief ����
 * @param x1 ��ʼX����
 * @param y1 ��ʼY����
 * @param x2 ����X����
 * @param y2 ����Y����
 * @param color ��ɫ
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    
    delta_x = x2 - x1; // ������������
    delta_y = y2 - y1;
    row = x1;
    col = y1;
    
    if (delta_x > 0) incx = 1;      // ���õ�������
    else if (delta_x == 0) incx = 0; // ��ֱ��
    else { incx = -1; delta_x = -delta_x; }
    
    if (delta_y > 0) incy = 1;
    else if (delta_y == 0) incy = 0; // ˮƽ��
    else { incy = -1; delta_y = -delta_y; }
    
    if (delta_x > delta_y) distance = delta_x; // ѡȡ��������������
    else distance = delta_y;
    
    for (t = 0; t <= distance + 1; t++) // �������
    {
        OLED_DrawPoint(row, col, color); // ����
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

/* =================== �ַ���ʾ���� =================== */

/**
 * @brief ��ʾ�ַ�
 * @param x X����
 * @param y Y����
 * @param chr �ַ�
 * @param size �����С (8 or 16)
 * @param mode ��ʾģʽ (0-����, 1-��ɫ)
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // �ַ���ռ�ֽ���
    
    chr = chr - ' '; // �õ�ƫ�ƺ��ֵ
    
    for (t = 0; t < csize; t++)
    {
        if (size == 8)
        {
            temp = asc2_0806[chr][t]; // ����0806�ֿ�
        }
        else if (size == 16)
        {
            temp = asc2_1608[chr][t]; // ����1608�ֿ�
        }
        else
        {
            return; // ��֧�ֵ������С
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
            if (y >= OLED_HEIGHT) return; // ������Ļ��Χ
        }
        x++;
        if ((size != 8) && ((x - (x - t * (size / 2))) == size / 2))
        {
            x = x - size / 2;
            y0 = y0 + 8;
        }
        y = y0;
        if (x >= OLED_WIDTH) return; // ������Ļ��Χ
    }
}

/**
 * @brief ��ʾ�Զ����ַ�
 * @param x X����
 * @param y Y����
 * @param char_code �Զ����ַ�����
 * @param mode ��ʾģʽ
 */
void OLED_ShowCustomChar(uint8_t x, uint8_t y, uint8_t char_code, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    
    if (char_code == 0 || char_code > CUSTOM_CHAR_COUNT)
        return;
    
    for (t = 0; t < 16; t++) // �Զ����ַ��̶�Ϊ8x16
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
        if ((x - (x - t * 4)) == 8) // 8x16�ַ�����
        {
            x = x - 8;
            y0 = y0 + 8;
        }
        y = y0;
        if (x >= OLED_WIDTH) return;
    }
}

/**
 * @brief ��ʾ�ַ���
 * @param x X����
 * @param y Y����
 * @param str �ַ���
 * @param size �����С
 * @param mode ��ʾģʽ
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode)
{
    uint8_t x0 = x;
    
    while ((*str <= '~') && (*str >= ' ')) // �ж��Ƿ�Ƿ��ַ�
    {
        if (x > (OLED_WIDTH - size / 2)) // ���д���
        {
            x = x0;
            y += size;
        }
        if (y > (OLED_HEIGHT - size)) // ������Ļ��Χ
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
 * @brief ���������� m^n
 * @param m ����
 * @param n ָ��
 * @return uint32_t ���
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
 * @brief ��ʾ����
 * @param x X����
 * @param y Y����
 * @param num ����
 * @param len ��ʾ����
 * @param size �����С
 * @param mode ��ʾģʽ
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
 * @brief ��ʾͼƬ
 * @param x X����
 * @param y Y����
 * @param width ͼƬ���
 * @param height ͼƬ�߶�
 * @param bmp ͼƬ����
 * @param mode ��ʾģʽ
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

/* =================== ��������� =================== */

/**
 * @brief ��ʾģ���ʼ��
 */
void OLEDDisplay_Init(void)
{
    // ��ʼ��OLEDӲ��
    OLED_Init();
    
    // ��ʼ����ʾ����ṹ��
    memset(&g_oled_display, 0, sizeof(g_oled_display));
    g_oled_display.current_interface = OLED_INTERFACE_STARTUP;
    g_oled_display.interface_start_time = HAL_GetTick();
    g_oled_display.logo_stage = LOGO_STAGE_BULHER;
    g_oled_display.selfcheck_item = SELFCHECK_RELAY;
    g_oled_display.refresh_flag = 1;
    
    // ��ʼ���Լ�״̬Ϊ������
    for (int i = 0; i < 6; i++)
    {
        g_oled_display.selfcheck_status[i] = SELFCHECK_STATUS_TESTING;
    }
}

/**
 * @brief ������ʾ����
 * @param interface Ŀ�����
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
 * @brief ǿ��ˢ����ʾ
 */
void OLEDDisplay_ForceRefresh(void)
{
    g_oled_display.refresh_flag = 1;
}

/**
 * @brief �����Լ�״̬
 * @param item �Լ���Ŀ
 * @param status �Լ�״̬
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
 * @brief ��ʾ������
 * @details ����ѭ���е��ã���������л�����ʾ����
 */
void OLEDDisplay_Task(void)
{
    static uint32_t last_refresh_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // �����Զ��л��߼�
    switch (g_oled_display.current_interface)
    {
        case OLED_INTERFACE_STARTUP:
            // ����������ʾ6����л����Լ����
            if (current_time - g_oled_display.interface_start_time > 6000)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_SELFCHECK);
            }
            break;
            
        case OLED_INTERFACE_SELFCHECK:
            // �Լ���ɺ��л�����������
            if (g_oled_display.selfcheck_item >= SELFCHECK_COMPLETE)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_NORMAL);
            }
            break;
            
        case OLED_INTERFACE_NORMAL:
            // ����Ƿ��б������������л�����������
            if (AlarmManager_GetAlarmCount() > 0)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_ALARM);
            }
            break;
            
        case OLED_INTERFACE_ALARM:
            // ��鱨���Ƿ��������������л�����������
            if (AlarmManager_GetAlarmCount() == 0)
            {
                OLEDDisplay_SetInterface(OLED_INTERFACE_NORMAL);
            }
            break;
    }
    
    // ��ʱˢ�»�ǿ��ˢ��
    if (g_oled_display.refresh_flag || (current_time - last_refresh_time > 1000))
    {
        // ����
        OLED_Clear();
        
        // ���ݵ�ǰ������ʾ����
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
        
        // ˢ�µ�OLED
        OLED_Refresh();
        
        g_oled_display.refresh_flag = 0;
        last_refresh_time = current_time;
    }
}

/* =================== ���������ʾ���� =================== */

/**
 * @brief ��ʾ����LOGO����
 */
void OLEDDisplay_ShowStartupLogo(void)
{
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed_time = current_time - g_oled_display.interface_start_time;
    
    if (elapsed_time < 3000)
    {
        // ��ʾBUHLER LOGO (ǰ3��)
        g_oled_display.logo_stage = LOGO_STAGE_BULHER;
        
        // ������ʾBUHLER LOGOͼ��
        // ����ͼ��ߴ�Ϊ96x27���أ�������ʾ
        OLED_ShowPicture(16, 18, 96, 27, &gImage_buhler_logo[6], OLED_COLOR_NORMAL);
    }
    else if (elapsed_time < 6000)
    {
        // ��ʾmingyer LOGO + ������ (��3��)
        g_oled_display.logo_stage = LOGO_STAGE_MINGYER;
        
        // ������ʾmingyer LOGOͼ��
        // ����ͼ��ߴ�Ϊ115x27���أ�������ʾ
        OLED_ShowPicture(6, 8, 115, 27, &gImage_minyer_logo[6], OLED_COLOR_NORMAL);
        
        // ��ʾ������
        uint8_t progress = ((elapsed_time - 3000) * 100) / 3000; // ������Ȱٷֱ�
        uint8_t bar_width = (progress * 100) / 100; // ���������
        
        // ���ƽ������߿�
        OLED_DrawLine(14, 40, 113, 40, OLED_COLOR_NORMAL); // �ϱ�
        OLED_DrawLine(14, 47, 113, 47, OLED_COLOR_NORMAL); // �±�
        OLED_DrawLine(14, 40, 14, 47, OLED_COLOR_NORMAL);  // ���
        OLED_DrawLine(113, 40, 113, 47, OLED_COLOR_NORMAL); // �ұ�
        
        // ��������
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
 * @brief ��ʾϵͳ�Լ����
 */
void OLEDDisplay_ShowSelfCheck(void)
{
    // ����
    OLED_ShowString(25, 0, "System Self-Test", 8, OLED_COLOR_NORMAL);
    
    // ���Ʒָ���
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // �Լ���Ŀ�б�
    const char* selfcheck_items[] = {
        "Relay Test",
        "NTC Sensor Test", 
        "Flash Memory",
        "I2C Bus",
        "GPIO Status"
    };
    
    // ��ʾ�Լ���Ŀ��״̬
    for (int i = 0; i < 5; i++)
    {
        uint8_t y_pos = 16 + i * 8;
        
        // ��ʾ��Ŀ����
        OLED_ShowString(0, y_pos, selfcheck_items[i], 8, OLED_COLOR_NORMAL);
        
        // ��ʾ״̬����
        if (g_oled_display.selfcheck_status[i] == SELFCHECK_STATUS_PASS)
        {
            // ��ʾ�Թ����� ?
            OLED_ShowCustomChar(110, y_pos - 4, CUSTOM_CHAR_CHECKMARK, OLED_COLOR_NORMAL);
        }
        else if (g_oled_display.selfcheck_status[i] == SELFCHECK_STATUS_FAIL)
        {
            // ��ʾʵ�ķ��� ��
            OLED_ShowCustomChar(110, y_pos - 4, CUSTOM_CHAR_SQUARE, OLED_COLOR_NORMAL);
        }
        else
        {
            // �����У���ʾ�հ�
            OLED_ShowString(110, y_pos, " ", 8, OLED_COLOR_NORMAL);
        }
    }
    
    // ���Ʒָ���
    OLED_DrawLine(0, 56, 127, 56, OLED_COLOR_NORMAL);
    
    // ��ʾ״̬��Ϣ
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
 * @brief ��ʾ������������
 */
void OLEDDisplay_ShowNormalInterface(void)
{
    char buffer[32];
    
    // ����
    OLED_ShowString(25, 0, "System Normal", 8, OLED_COLOR_NORMAL);
    
    // ���Ʒָ���
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // ͨ��״̬��ʾ
    ChannelType_t ch1_state = RelayControl_GetChannelState(CHANNEL_1);
    ChannelType_t ch2_state = RelayControl_GetChannelState(CHANNEL_2);
    ChannelType_t ch3_state = RelayControl_GetChannelState(CHANNEL_3);
    
    sprintf(buffer, "CH1: [%s]", (ch1_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 16, buffer, 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "CH2: [%s]", (ch2_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 26, buffer, 8, OLED_COLOR_NORMAL);
    
    sprintf(buffer, "CH3: [%s]", (ch3_state == CHANNEL_ON) ? "ON " : "OFF");
    OLED_ShowString(0, 36, buffer, 8, OLED_COLOR_NORMAL);
    
    // ���Ʒָ���
    OLED_DrawLine(0, 48, 127, 48, OLED_COLOR_NORMAL);
    
    // �¶Ⱥͷ�����Ϣ
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
 * @brief ��ʾ�쳣��������
 */
void OLEDDisplay_ShowAlarmInterface(void)
{
    char buffer[32];
    AlarmType_t active_alarms[MAX_ALARM_COUNT];
    uint8_t alarm_count = AlarmManager_GetActiveAlarms(active_alarms, MAX_ALARM_COUNT);
    
    // ������źͱ���
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
    
    // ���Ʒָ���
    OLED_DrawLine(0, 12, 127, 12, OLED_COLOR_NORMAL);
    
    // ��ʾǰ3��������Ϣ
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
    
    // ���Ʒָ���
    OLED_DrawLine(0, 48, 127, 48, OLED_COLOR_NORMAL);
    
    // �¶Ⱥͷ�����Ϣ��������������ͬ��
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


