/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "system_config.h"
#include "relay_control.h"
#include "temperature_control.h"
#include "alarm_manager.h"
#include "oled_display.h"
#include "system_monitor.h"        // 新增：系统监控模块
#include "app_main.h"              // 新增：主控制模块
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 系统运行状态标志
static uint8_t system_initialized = 0;     // 系统初始化完成标志
static uint32_t main_loop_counter = 0;     // 主循环计数器（用于调试）
static uint32_t last_error_time = 0;       // 上次错误发生时间

// 系统性能监控
static uint32_t loop_start_time = 0;       // 循环开始时间
static uint32_t max_loop_time = 0;         // 最大循环时间
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void System_InitializeAllModules(void);   // 系统模块初始化
static void System_RunMainLoop(void);            // 主循环处理
static void System_ErrorHandler(void);           // 系统错误处理
static void System_PerformanceMonitor(void);     // 性能监控
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  // 系统完整初始化
  System_InitializeAllModules();

  // 设置系统初始化完成标志
  system_initialized = 1;

  // 启动主控制模块
  AppMain_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 记录循环开始时间（性能监控）
    loop_start_time = HAL_GetTick();
    
    // 执行主循环处理
    System_RunMainLoop();
    
    // 性能监控
    System_PerformanceMonitor();
    
    // 主循环计数
    main_loop_counter++;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief 系统模块初始化
 * @param None
 * @retval None
 */
static void System_InitializeAllModules(void)
{
    // 按照依赖关系顺序初始化各模块
    RelayControl_Init();           // 继电器控制模块
    TemperatureControl_Init();     // 温度控制模块
    AlarmManager_Init();           // 报警管理模块
    SystemMonitor_Init();          // 系统监控模块
    OLEDDisplay_Init();            // OLED显示模块
    
    // 短暂延时确保所有模块稳定
    HAL_Delay(100);
}

/**
 * @brief 主循环处理
 * @param None
 * @retval None
 */
static void System_RunMainLoop(void)
{
    // 检查系统初始化状态
    if (!system_initialized)
    {
        System_ErrorHandler();
        return;
    }
    
    // 主控制模块运行（包含状态机）
    AppMain_Run();
    
    // 各功能模块任务处理（按优先级排序）
    SystemMonitor_Task();          // 最高优先级：系统监控
    AlarmManager_Task();           // 高优先级：报警处理
    RelayControl_Task();           // 中优先级：继电器控制
    TemperatureControl_Task();     // 中优先级：温度控制
    OLEDDisplay_Task();            // 低优先级：显示更新
}

/**
 * @brief 系统错误处理
 * @param None
 * @retval None
 */
static void System_ErrorHandler(void)
{
    // 记录错误时间
    last_error_time = HAL_GetTick();
    
    // 标记变量已使用，避免编译警告
    (void)last_error_time;
    
    // 激活系统错误报警
    AlarmManager_SetAlarm(ALARM_N);  // N类异常：自检异常
    
    // 紧急安全处理：关闭所有通道
    RelayControl_TurnOffChannel1();
    RelayControl_TurnOffChannel2();
    RelayControl_TurnOffChannel3();
    
    // 激活报警输出
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_RESET);
    
    // 显示错误状态
    OLEDDisplay_ShowAlarmInterface();
}

/**
 * @brief 性能监控
 * @param None
 * @retval None
 */
static void System_PerformanceMonitor(void)
{
    // 计算循环执行时间
    uint32_t loop_time = HAL_GetTick() - loop_start_time;
    
    // 记录最大循环时间
    if (loop_time > max_loop_time)
    {
        max_loop_time = loop_time;
    }
    
    // 如果循环时间过长，进行警告
    if (loop_time > MAIN_LOOP_DELAY_MS * 5) // 超过50ms
    {
        // 可以在这里添加性能警告处理
    }
    
    // 控制主循环频率
    HAL_Delay(MAIN_LOOP_DELAY_MS);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  
  // 系统级错误处理
  System_ErrorHandler();
  
  // 禁用所有中断
  __disable_irq();
  
  // 死循环等待看门狗复位或外部复位
  while (1)
  {
    // 保持报警状态
    HAL_GPIO_WritePin(ALARM_GPIO_Port, ALARM_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
  }
  
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
