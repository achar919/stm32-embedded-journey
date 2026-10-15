<h1 align="center">STM32 Embedded Journey</h1>

<p align="center">
  Learning embedded systems from zero, one hands-on project at a time, on real STM32 hardware.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/MCU-STM32F446RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white" alt="MCU">
  <img src="https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white" alt="C">
  <img src="https://img.shields.io/badge/IDE-STM32CubeIDE-1F6FEB?style=for-the-badge" alt="IDE">
  <img src="https://img.shields.io/badge/Status-In%20Progress-2EA043?style=for-the-badge" alt="Status">
</p>

<p align="center">
  <img src="docs/media/01-button-led.gif" alt="Button controlling LED on NUCLEO-F446RE" width="420">
</p>

---

## About

I'm documenting my path into embedded systems: configuring microcontrollers, writing firmware in C, and, most importantly, **the real problems I hit and how I fixed them**.

Each folder is a complete, buildable STM32CubeIDE project with its own README covering the goal, setup, code, and debugging notes.

## Hardware

| Board | MCU | Core | Flash / RAM | Used for |
|---|---|---|---|---|
| NUCLEO-F446RE | STM32F446RE | Arm Cortex-M4F @ 180 MHz | 512 KB / 128 KB | Fundamentals (Lessons 01–10) |
| NUCLEO-H563ZI | STM32H563ZI | Arm Cortex-M33 @ 250 MHz | 2 MB / 640 KB | Advanced projects (Ethernet, RTOS) |

## Toolchain

- **STM32CubeMX**: pin, clock, and peripheral configuration
- **STM32CubeIDE**: code, build, flash, debug
- **STM32CubeProgrammer**: ST-LINK firmware and board recovery
- **Tera Term**: UART serial monitor

## Roadmap

| # | Project | Concepts | Status |
|---|---|---|---|
| 01 | [Button → LED](01_button_led/) | GPIO input/output, active-low logic, CubeMX basics | ✅ Done |
| 02 | [Smart button + UART logging](02_smart_button_uart/) | Edge detection, debouncing, `printf` over UART | ✅ Done |
| 03 | [Button interrupt](03_button_interrupt/) | EXTI, NVIC, callbacks, `volatile` | ✅ Done |
| 04 | [Hardware timer blink](04_timer_blink/) | TIM6, prescaler, auto-reload, timer interrupt | ✅ Done |
| 05 | [PWM LED fade](05_pwm_fade/) | PWM, duty cycle, TIM2 CH1 | ✅ Done |
| 06 | [Potentiometer reader](06_adc_pot/) | ADC, averaging, for loop, if/else | ✅ Done |
| 07 | [I2C OLED display](07_i2c_oled/) | I2C, scanning, SH1106 driver, frame buffer | ✅ Done |
| 08 | Register-level blink | RCC, GPIO registers, no HAL | ⏳ Planned |
| 09 | Multi-task system | FreeRTOS | ⏳ Planned |
| 10 | Ethernet sensor dashboard | NUCLEO-H563ZI, networking | ⏳ Planned |
## Repository structure

```
stm32-embedded-journey/
├── README.md
├── 01_button_led/          ← each lesson is its own CubeIDE project
│   ├── Core/               ← main.c lives here
│   ├── Drivers/
│   ├── 01_button_led.ioc
│   └── README.md           ← lesson write-up
├── 02_smart_button_uart/
├── 03_button_interrupt/
├── 04_timer_blink/
└── 05_pwm_fade/
```

## Build any project

1. Clone the repository.
2. In STM32CubeIDE: **File → Import → Existing Projects into Workspace** and select the lesson folder.
3. Build (🔨), connect the Nucleo board over USB, then Run (▶).

## Connect

Following along or have feedback? Reach me on **[LinkedIn](https://www.linkedin.com/in/vishwasachar1128)**.
