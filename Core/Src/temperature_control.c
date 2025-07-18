/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : temperature_control.c
  * @brief          : 温度控制源文件
  ******************************************************************************
  */
/* USER CODE END Header */

#include "temperature_control.h"
#include "alarm_manager.h"

/* NTC查表数组 - 每一度一个点，提高精度 ----------------------------------------*/
typedef struct {
    int8_t temperature;   // 温度值（℃）
    float resistance;     // 电阻值（KΩ）
} NTC_TableEntry_t;

// NTC 10K B3435 查表数组（-40℃到125℃，每一度一个点）
static const NTC_TableEntry_t ntc_table[] = {
    {-40, 197.39}, {-39, 186.54}, {-38, 176.35}, {-37, 166.8}, {-36, 157.82},
    {-35, 149.39}, {-34, 141.51}, {-33, 134.09}, {-32, 127.11}, {-31, 120.53},
    {-30, 114.34}, {-29, 108.53}, {-28, 103.04}, {-27, 97.87}, {-26, 92.989},
    {-25, 88.381}, {-24, 84.036}, {-23, 79.931}, {-22, 76.052}, {-21, 72.384},
    {-20, 68.915}, {-19, 65.634}, {-18, 62.529}, {-17, 59.589}, {-16, 56.804},
    {-15, 54.166}, {-14, 51.665}, {-13, 49.294}, {-12, 47.046}, {-11, 44.913},
    {-10, 42.889}, {-9, 40.967}, {-8, 39.142}, {-7, 37.408}, {-6, 35.761},
    {-5, 34.196}, {-4, 32.707}, {-3, 31.291}, {-2, 29.945}, {-1, 28.664},
    {0, 27.445}, {1, 26.283}, {2, 25.177}, {3, 24.124}, {4, 23.121},
    {5, 22.165}, {6, 21.253}, {7, 20.384}, {8, 19.555}, {9, 18.764},
    {10, 18.01}, {11, 17.29}, {12, 16.602}, {13, 15.946}, {14, 15.319},
    {15, 14.72}, {16, 14.148}, {17, 13.601}, {18, 13.078}, {19, 12.578},
    {20, 12.099}, {21, 11.642}, {22, 11.204}, {23, 10.785}, {24, 10.384},
    {25, 10.0}, {26, 9.632}, {27, 9.28}, {28, 8.943}, {29, 8.619},
    {30, 8.309}, {31, 8.012}, {32, 7.727}, {33, 7.453}, {34, 7.191},
    {35, 6.939}, {36, 6.698}, {37, 6.466}, {38, 6.243}, {39, 6.029},
    {40, 5.824}, {41, 5.627}, {42, 5.437}, {43, 5.255}, {44, 5.08},
    {45, 4.911}, {46, 4.749}, {47, 4.593}, {48, 4.443}, {49, 4.299},
    {50, 4.16}, {51, 4.027}, {52, 3.898}, {53, 3.774}, {54, 3.654},
    {55, 3.539}, {56, 3.429}, {57, 3.322}, {58, 3.219}, {59, 3.119},
    {60, 3.024}, {61, 2.931}, {62, 2.842}, {63, 2.756}, {64, 2.673},
    {65, 2.593}, {66, 2.516}, {67, 2.441}, {68, 2.369}, {69, 2.3},
    {70, 2.233}, {71, 2.168}, {72, 2.105}, {73, 2.044}, {74, 1.986},
    {75, 1.929}, {76, 1.874}, {77, 1.821}, {78, 1.77}, {79, 1.72},
    {80, 1.673}, {81, 1.626}, {82, 1.581}, {83, 1.538}, {84, 1.496},
    {85, 1.455}, {86, 1.416}, {87, 1.377}, {88, 1.34}, {89, 1.304},
    {90, 1.27}, {91, 1.236}, {92, 1.204}, {93, 1.172}, {94, 1.141},
    {95, 1.112}, {96, 1.083}, {97, 1.055}, {98, 1.028}, {99, 1.002},
    {100, 0.976}, {101, 0.951}, {102, 0.927}, {103, 0.904}, {104, 0.882},
    {105, 0.86}, {106, 0.838}, {107, 0.818}, {108, 0.798}, {109, 0.778},
    {110, 0.759}, {111, 0.741}, {112, 0.723}, {113, 0.706}, {114, 0.689},
    {115, 0.673}, {116, 0.657}, {117, 0.641}, {118, 0.626}, {119, 0.612},
    {120, 0.598}, {121, 0.584}, {122, 0.57}, {123, 0.557}, {124, 0.545},
    {125, 0.532}
};

#define NTC_TABLE_SIZE (sizeof(ntc_table) / sizeof(ntc_table[0]))

/* 私有变量 -------------------------------------------------------------------*/
static uint16_t adc_values[3];        // ADC采集缓存
static uint32_t fan_pulse_count = 0;  // 风扇脉冲计数
static uint32_t last_rpm_time = 0;    // 上次转速计算时间

/**
  * @brief  温度控制初始化
  * @param  None
  * @retval None
  */
void TemperatureControl_Init(void)
{
    // 启动ADC和DMA
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values, 3);
    
    // 启动TIM3 PWM输出（风扇控制）
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    
    // 设置初始风扇占空比为50%
    TemperatureControl_SetFanDuty(FAN_PWM_NORMAL);
}

/**
  * @brief  获取指定通道ADC值
  * @param  channel: ADC通道号（0-2对应NTC_1到NTC_3）
  * @retval ADC采集值
  */
uint16_t TemperatureControl_GetAdcValue(uint8_t channel)
{
    if (channel < 3)
    {
        return adc_values[channel];
    }
    return 0;
}

/**
  * @brief  ADC值转换为电压
  * @param  adc_value: ADC采集值
  * @retval 对应的电压值（V）
  */
static float TemperatureControl_AdcToVoltage(uint16_t adc_value)
{
    // ADC_REFERENCE_VOLTAGE = 3.3V, ADC_RESOLUTION = 4096
    return ((float)adc_value * 3.3f) / 4096.0f;
}

/**
  * @brief  电压转换为NTC电阻值
  * @param  voltage: 分压点电压（V）
  * @retval NTC电阻值（KΩ）
  */
static float TemperatureControl_VoltageToResistance(float voltage)
{
    // 分压电路：R_NTC = (V_ADC × 10KΩ) / (3.3V - V_ADC)
    if (voltage >= 3.3f) voltage = 3.29f;  // 防止除零
    return (voltage * 10.0f) / (3.3f - voltage);
}

/**
  * @brief  电阻值查表转换为温度
  * @param  resistance: NTC电阻值（KΩ）
  * @retval 对应的温度值（℃）
  */
static float TemperatureControl_ResistanceToTemperature(float resistance)
{
    uint16_t i;
    
    // 边界检查
    if (resistance >= ntc_table[0].resistance)
    {
        return (float)ntc_table[0].temperature;  // 低于-40℃
    }
    if (resistance <= ntc_table[NTC_TABLE_SIZE-1].resistance)
    {
        return (float)ntc_table[NTC_TABLE_SIZE-1].temperature;  // 高于125℃
    }
    
    // 查表找到对应的温度区间
    for (i = 0; i < NTC_TABLE_SIZE - 1; i++)
    {
        if (resistance <= ntc_table[i].resistance && resistance >= ntc_table[i+1].resistance)
        {
            // 线性插值计算精确温度
            float temp1 = (float)ntc_table[i].temperature;
            float temp2 = (float)ntc_table[i+1].temperature;
            float res1 = ntc_table[i].resistance;
            float res2 = ntc_table[i+1].resistance;
            
            // 线性插值公式
            float temperature = temp1 + (temp2 - temp1) * (resistance - res1) / (res2 - res1);
            return temperature;
        }
    }
    
    return 25.0f;  // 默认返回25℃
}

/**
  * @brief  获取指定NTC温度
  * @param  ntc_channel: NTC通道号（0-2）
  * @retval 温度值（℃）
  */
float TemperatureControl_GetTemperature(uint8_t ntc_channel)
{
    uint16_t adc_value = TemperatureControl_GetAdcValue(ntc_channel);
    float voltage = TemperatureControl_AdcToVoltage(adc_value);
    float resistance = TemperatureControl_VoltageToResistance(voltage);
    return TemperatureControl_ResistanceToTemperature(resistance);
}

/**
  * @brief  设置风扇PWM占空比
  * @param  duty_percent: 占空比百分比（0-100）
  * @retval None
  */
void TemperatureControl_SetFanDuty(uint8_t duty_percent)
{
    if (duty_percent > 100) duty_percent = 100;
    
    // TIM3 ARR=99, 所以PWM值范围是0-99
    uint32_t pwm_value = (duty_percent * 99) / 100;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value);
}

/**
  * @brief  获取风扇转速（RPM）
  * @param  None
  * @retval 转速值（RPM）
  */
uint16_t TemperatureControl_GetFanRpm(void)
{
    // 读取风扇传感器引脚状态进行脉冲计数
    static uint8_t last_state = 1;
    uint8_t current_state = HAL_GPIO_ReadPin(FAN_SEN_GPIO_Port, FAN_SEN_Pin);
    
    // 检测下降沿（脉冲）
    if (last_state == 1 && current_state == 0)
    {
        fan_pulse_count++;
    }
    last_state = current_state;
    
    // 每秒计算一次转速
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_rpm_time >= 1000)  // 1秒更新一次
    {
        // 风扇转速计算：RPM = (脉冲频率 × 60秒) ÷ 每转脉冲数
        // 假设每转2个脉冲
        uint16_t rpm = (fan_pulse_count * 60) / 2;
        
        // 重置计数器
        fan_pulse_count = 0;
        last_rpm_time = current_time;
        
        return rpm;
    }
    
    return 0;  // 未到更新时间返回0
}

/**
  * @brief  根据温度更新风扇控制
  * @param  max_temp: 当前最高温度
  * @retval None
  */
static void TemperatureControl_UpdateFanByTemperature(float max_temp)
{
    static FanMode_t current_fan_mode = FAN_MODE_NORMAL;
    FanMode_t new_fan_mode = current_fan_mode;
    
    // 根据温度确定风扇模式（考虑回差）
    if (max_temp < TEMP_NORMAL - TEMP_HYSTERESIS)
    {
        new_fan_mode = FAN_MODE_NORMAL;      // 正常模式
    }
    else if (max_temp >= TEMP_NORMAL && max_temp < TEMP_HIGH - TEMP_HYSTERESIS)
    {
        new_fan_mode = FAN_MODE_HIGH_TEMP;   // 高温模式
    }
    else if (max_temp >= TEMP_HIGH)
    {
        new_fan_mode = FAN_MODE_OVER_TEMP;   // 超温模式
    }
    
    // 更新风扇占空比
    if (new_fan_mode != current_fan_mode)
    {
        current_fan_mode = new_fan_mode;
        g_fan_mode = current_fan_mode;  // 更新全局状态
        
        switch (current_fan_mode)
        {
            case FAN_MODE_NORMAL:
                TemperatureControl_SetFanDuty(FAN_PWM_NORMAL);     // 50%
                break;
            case FAN_MODE_HIGH_TEMP:
            case FAN_MODE_OVER_TEMP:
                TemperatureControl_SetFanDuty(FAN_PWM_HIGH);       // 95%
                break;
        }
    }
}

/**
  * @brief  检查温度异常
  * @param  None
  * @retval 对应的异常类型
  */
AlarmType_t TemperatureControl_CheckTemperatureAlarm(void)
{
    float temp1 = TemperatureControl_GetTemperature(0);  // NTC_1
    float temp2 = TemperatureControl_GetTemperature(1);  // NTC_2
    float temp3 = TemperatureControl_GetTemperature(2);  // NTC_3
    
    // 检查各通道温度是否超过60℃
    if (temp1 >= TEMP_HIGH)
    {
        return ALARM_K;  // NTC_1温度异常
    }
    if (temp2 >= TEMP_HIGH)
    {
        return ALARM_L;  // NTC_2温度异常
    }
    if (temp3 >= TEMP_HIGH)
    {
        return ALARM_M;  // NTC_3温度异常
    }
    
    return ALARM_NONE;
}

/**
  * @brief  温度控制主处理函数
  * @param  None
  * @retval None
  */
void TemperatureControl_Process(void)
{
    // 获取三路温度
    float temp1 = TemperatureControl_GetTemperature(0);
    float temp2 = TemperatureControl_GetTemperature(1);
    float temp3 = TemperatureControl_GetTemperature(2);
    
    // 找出最高温度
    float max_temp = temp1;
    if (temp2 > max_temp) max_temp = temp2;
    if (temp3 > max_temp) max_temp = temp3;
    
    // 根据最高温度更新风扇控制
    TemperatureControl_UpdateFanByTemperature(max_temp);
    
    // 更新风扇转速
    TemperatureControl_GetFanRpm();
}





/**
 * @brief 温度控制任务处理
 * @details 在主循环中调用，处理温度控制相关任务
 */
void TemperatureControl_Task(void)
{
    // 执行温度控制主处理
    TemperatureControl_Process();
    
    // 检查温度异常
    AlarmType_t temp_alarm = TemperatureControl_CheckTemperatureAlarm();
    if (temp_alarm != ALARM_NONE)
    {
        AlarmManager_SetAlarm(temp_alarm);
    }
}

/**
 * @brief 获取风扇占空比
 * @return uint8_t 占空比(0-100)
 */
uint8_t TemperatureControl_GetFanDuty(void)
{
    // 从TIM3的PWM占空比计算百分比
    uint32_t ccr_value = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1);
    uint32_t arr_value = __HAL_TIM_GET_AUTORELOAD(&htim3);
    return (uint8_t)((ccr_value * 100) / arr_value);
}

 

 


