# STM32F103C8T6 OLED显示屏驱动项目

## 项目简介

本项目是基于STM32F103C8T6微控制器的2.42寸OLED显示屏驱动程序，使用I2C通信协议。项目展示了OLED屏幕的各种显示功能，包括中文字符、英文字符、数字、图片和滚动显示等。

## 硬件连接

### OLED显示屏连接定义
- **GND** - 电源地
- **VCC** - 3.3V电源
- **D0(SCL)** - PA0 (I2C时钟线)
- **D1(SDA)** - PA1 (I2C数据线)  
- **RES** - PA2 (复位引脚，SPI模块改成IIC模块需要接此引脚，IIC模块用户请忽略)

### 主控芯片
- **STM32F103C8T6** - ARM Cortex-M3内核，64KB Flash，20KB SRAM

## 项目目录结构

```
├── CORE/                    # ARM Cortex-M3内核相关文件
│   ├── core_cm3.c          # ARM Cortex-M3内核驱动实现
│   ├── core_cm3.h          # ARM Cortex-M3内核驱动头文件
│   ├── startup_stm32f10x_hd.s  # 大容量产品启动文件
│   └── startup_stm32f10x_md.s  # 中容量产品启动文件
│
├── HARDWARE/               # 硬件驱动模块
│   └── OLED/              # OLED显示屏驱动
│       ├── oled.c         # OLED驱动实现文件
│       ├── oled.h         # OLED驱动头文件
│       ├── bmp.h          # 位图数据文件
│       └── oledfont.h     # OLED字体数据文件
│
├── SYSTEM/                # 系统功能模块
│   ├── delay/             # 延时功能模块
│   │   ├── delay.c        # 延时函数实现
│   │   └── delay.h        # 延时函数头文件
│   ├── sys/               # 系统配置模块
│   │   ├── sys.c          # 系统配置实现
│   │   └── sys.h          # 系统配置头文件
│   └── usart/             # 串口通信模块
│       ├── usart.c        # 串口驱动实现
│       └── usart.h        # 串口驱动头文件
│
├── USER/                  # 用户应用程序
│   ├── main.c             # 主程序文件
│   ├── stm32f10x.h        # STM32F10x系列芯片头文件
│   ├── stm32f10x_conf.h   # STM32标准库配置文件
│   ├── stm32f10x_it.c     # 中断服务程序实现
│   ├── stm32f10x_it.h     # 中断服务程序头文件
│   ├── system_stm32f10x.c # STM32F10x系统时钟配置
│   ├── system_stm32f10x.h # STM32F10x系统时钟配置头文件
│   └── *.uvprojx          # Keil工程文件
│
├── STM32F10x_FWLib/       # STM32标准外设库
├── OBJ/                   # 编译输出目录
└── keilkilll.bat          # Keil编译清理脚本
```

## 功能特性

### OLED显示功能
1. **基础显示控制**
   - 屏幕初始化和配置
   - 显示开关控制
   - 屏幕清除功能
   - 正常/反色显示切换
   - 屏幕翻转显示

2. **图形绘制功能**
   - 像素点绘制
   - 直线绘制
   - 圆形绘制
   - 位图图片显示

3. **文字显示功能**
   - 英文字符显示 (6x8, 6x12, 8x16, 12x24字体)
   - 中文字符显示 (16x16, 24x24, 32x32, 64x64字体)
   - 字符串显示
   - 数字显示

4. **高级功能**
   - 滚动显示
   - 屏幕刷新控制

### I2C通信协议
- 软件模拟I2C通信
- 支持标准I2C时序
- 可靠的数据传输

## 主要API接口

### 初始化函数
```c
void OLED_Init(void);                    // OLED初始化
```

### 显示控制函数
```c
void OLED_Clear(void);                   // 清屏
void OLED_Refresh(void);                 // 刷新显示
void OLED_ColorTurn(u8 i);              // 反色显示设置
void OLED_DisplayTurn(u8 i);            // 屏幕翻转设置
```

### 图形绘制函数
```c
void OLED_DrawPoint(u8 x,u8 y,u8 t);    // 画点
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);  // 画线
void OLED_DrawCircle(u8 x,u8 y,u8 r);   // 画圆
```

### 文字显示函数
```c
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);        // 显示字符
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);     // 显示字符串
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode); // 显示数字
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);     // 显示中文
```

### 高级功能函数
```c
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode); // 显示图片
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);                     // 滚动显示
```

## 程序运行流程

1. **系统初始化**
   - 延时系统初始化
   - OLED显示屏初始化
   - 设置显示模式

2. **循环显示演示**
   - 显示位图图片
   - 显示中文字符和英文字符串
   - 显示ASCII字符和对应编码
   - 演示不同字体大小的中文显示
   - 演示不同字体大小的英文显示
   - 滚动显示演示

## 开发环境

- **IDE**: Keil MDK-ARM
- **编译器**: ARM Compiler
- **调试器**: J-Link
- **芯片**: STM32F103C8T6
- **标准库**: STM32F10x Standard Peripheral Library

## 编译和下载

1. 使用Keil MDK打开工程文件 `OLED.uvprojx`
2. 选择正确的目标芯片型号
3. 编译工程 (F7)
4. 连接J-Link调试器
5. 下载程序到芯片 (F8)

## 注意事项

1. **电源连接**: 确保OLED模块使用3.3V供电，避免使用5V损坏模块
2. **引脚连接**: 严格按照引脚定义连接，避免接错导致通信失败
3. **I2C通信**: 本项目使用软件模拟I2C，如需修改引脚请同时修改头文件中的宏定义
4. **字体数据**: 中文字体数据存储在 `oledfont.h` 中，如需添加新字符请按格式添加
5. **图片数据**: 位图数据存储在 `bmp.h` 中，使用取模软件生成

## 扩展功能建议

1. **添加更多字体**: 可以添加更多尺寸的中英文字体
2. **图形界面**: 可以开发简单的菜单界面系统
3. **动画效果**: 可以添加更多动画和过渡效果
4. **传感器数据显示**: 结合传感器显示实时数据
5. **用户交互**: 添加按键输入实现用户交互功能

## 版权信息

- **作者**: HuangKai (中景园电子)
- **版本**: v2.0
- **日期**: 2018-10-31
- **店铺**: http://shop73023976.taobao.com
- **说明**: 本程序仅供学习使用

## 技术支持

如有技术问题，请参考：
1. STM32F103C8T6数据手册
2. OLED显示屏驱动芯片手册
3. STM32标准外设库文档
4. 中景园电子技术支持

---
*本README文档详细描述了STM32 OLED显示屏项目的结构和使用方法，帮助开发者快速理解和使用本项目。* 