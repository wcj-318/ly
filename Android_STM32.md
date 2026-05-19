# Android 控制 STM32 麦轮小车实现说明

## 1. 本版实现范围

本工程已经从原来的 TB6612 双电机固定转速 Demo，改为 Android BLE 单字符遥控小车程序：

- JDY-23 通过 UART 接收 Android 指令
- STM32 解析 `F/B/L/R/G/H/S/A/D` 控制命令
- 四轮方向独立控制
- 左右两侧共 PWM 调速
- 保留原工程编码器采样和电压采样调试输出

说明：本工程是 STM32F10x 标准外设库工程，不是 HAL/CubeMX 工程，所以蓝牙接收使用 `USARTx_IRQHandler`，没有使用 `HAL_UART_Receive_IT()`。

## 2. 关键取舍

`蓝牙开发.md` 原先建议蓝牙模块使用 `USART2 PA2/PA3`，但原工程已经把 `PA2` 用作电池电压 ADC 采样。按照“引脚如果有更新优先保留原工程”的要求，本版使用 JDY-23，并保留 `PA2` 电压采样，把 BLE 串口放到 `USART3 PB10/PB11`。

如果后期必须改回 `USART2 PA2/PA3`：

1. 修改 `Driver/ax_uart.c` 里的 `AX_BLUETOOTH_Init()`：USART3 改 USART2，PB10/PB11 改 PA2/PA3。
2. 修改 `User/stm32f10x_it.c`：`USART3_IRQHandler()` 改为 `USART2_IRQHandler()`。
3. 停用或迁移 `Driver/ax_vin.c` 中的 `PA2 ADC2_IN2` 电压采样。

## 3. 实际引脚表

### PWM

| 功能 | 引脚 | 定时器 | 代码位置 |
|---|---|---|---|
| LEFT_PWM | PA8 | TIM1_CH1 | `Driver/ax_motor.c` |
| RIGHT_PWM | PA11 | TIM1_CH4 | `Driver/ax_motor.c` |

### 方向 IO

| 电机 | IN1 | IN2 | 代码宏 |
|---|---|---|---|
| 左前 LF | PB12 | PB13 | `LF_IN1_PIN/LF_IN2_PIN` |
| 左后 LR | PB14 | PB15 | `LR_IN1_PIN/LR_IN2_PIN` |
| 右前 RF | PA4 | PA5 | `RF_IN1_PIN/RF_IN2_PIN` |
| 右后 RR | PB8 | PB9 | `RR_IN1_PIN/RR_IN2_PIN` |

### 编码器与电压

| 功能 | 引脚 | 代码位置 |
|---|---|---|
| 编码器 A | PA6/PA7, TIM3 | `Driver/ax_encoder.c` |
| 编码器 B | PA0/PA1, TIM2 | `Driver/ax_encoder.c` |
| 电池电压 | PA2, ADC2_IN2 | `Driver/ax_vin.c` |

### BLE

| JDY-23 | STM32 | 说明 |
|---|---|---|
| TXD | PB11 | USART3_RX |
| RXD | PB10 | USART3_TX |
| VCC | 3.3V | JDY-23 支持 1.8V ~ 3.6V，禁止接 5V |
| GND | GND | 共地 |

波特率：`9600`，配置位置：`User/main.c` 中 `AX_BLUETOOTH_Init(9600)`。JDY-23 出厂默认串口波特率为 `9600`，广播名通常为 `JDY-23`。

## 4. Android 指令协议

Android 每次发送 1 个 ASCII 字符。按钮按下发送动作字符，松开发送 `S` 停止。

| 指令 | 动作 | STM32 函数 |
|---|---|---|
| F | 前进 | `Car_Forward(car_speed)` |
| B | 后退 | `Car_Backward(car_speed)` |
| L | 左平移 | `Car_LeftShift(car_speed)` |
| R | 右平移 | `Car_RightShift(car_speed)` |
| G | 左旋转 | `Car_LeftRotate(car_speed)` |
| H | 右旋转 | `Car_RightRotate(car_speed)` |
| S | 停止 | `Car_Stop()` |
| A | 加速 | `car_speed += CAR_SPEED_STEP` |
| D | 差速甩尾 | `Car_Drift(car_speed)` |

命令解析位置：`User/main.c` 的 `Car_HandleCommand()`。

## 5. 速度修改

默认速度在 `User/main.c`：

```c
#define CAR_DEFAULT_SPEED 2000
#define CAR_SPEED_STEP    500
```

PWM 最大值在 `Driver/ax_motor.h`：

```c
#define AX_MOTOR_PWM_MAX 7200
```

原工程 TIM1 周期是 `7200-1`，所以速度范围保持为 `0 ~ 7200`。

## 6. 麦轮动作表

方向定义在 `Driver/ax_motor.c`：

| 动作 | LF | LR | RF | RR |
|---|---:|---:|---:|---:|
| 前进 | 1 | 1 | 1 | 1 |
| 后退 | -1 | -1 | -1 | -1 |
| 左平移 | -1 | 1 | 1 | -1 |
| 右平移 | 1 | -1 | -1 | 1 |
| 左旋转 | -1 | -1 | 1 | 1 |
| 右旋转 | 1 | 1 | -1 | -1 |
| 停止 | 0 | 0 | 0 | 0 |

如果实车某个轮子方向相反，只改对应动作表或调换该电机 IN1/IN2，不要先改 PWM。

## 7. 代码入口

- `User/main.c`
  - 初始化系统、调试串口、电机、编码器、电压采样、BLE
  - 接收蓝牙命令并调用小车动作函数
  - 每 100ms 输出编码器和电压信息

- `Driver/ax_uart.c`
  - `AX_UART_Init()`：USART1 调试串口
  - `AX_BLUETOOTH_Init()`：USART3 BLE 串口
  - `AX_BLUETOOTH_GetCommand()`：主循环读取最新命令
  - `AX_BLUETOOTH_IRQHandler()`：USART3 接收中断

- `Driver/ax_motor.c`
  - `AX_MOTOR_Init()`：PWM 与方向 IO 初始化
  - `Car_Forward()` 等函数：小车动作控制
  - `AX_MOTOR_SetSideSpeed()`：设置左右共 PWM

- `User/stm32f10x_it.c`
  - `USART3_IRQHandler()` 转发 BLE 串口中断

## 8. 调试步骤

1. 烧录后打开 USART1 调试串口，波特率 `115200`。
2. 确认启动输出：

```text
Android BLE Mecanum Car
JDY-23 BLE UART: USART3 PB10(TX) PB11(RX), 9600bps
```

3. Android 连接 JDY-23 后发送 `F`，车应前进；发送 `S`，车应停止。
4. 逐个测试 `B/L/R/G/H/D`。
5. 如果某个动作方向不对，先检查该轮 IN1/IN2 接线，再调整 `Driver/ax_motor.c` 里的动作表。

## 9. PID 后续接入位置

本版先完成 Android 遥控闭环入口和编码器采样保留。后续要加入 PID 时建议只改两处：

1. `User/main.c`：把 `car_speed` 从 PWM 值改成目标编码器计数。
2. 新增或扩展 `Driver/ax_motor.c`：根据 PID 输出调用 `AX_MOTOR_SetSideSpeed(left_pwm, right_pwm)`。

注意：目前只有两路编码器，适合做左右侧速度 PID；如果要四轮独立 PID，需要增加四路编码器或明确每个编码器对应的轮子。
