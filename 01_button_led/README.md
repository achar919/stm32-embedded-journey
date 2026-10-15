# 01 · Button → LED

> Press the blue user button and the green LED turns on. Release it and the LED turns off.

<p align="center">
https://github.com/user-attachments/assets/ffaec951-93c4-4a4f-add5-d47df7bab43e
</p>

## Goal

Learn the complete STM32 workflow end to end: configure pins in **CubeMX** → generate code → write logic in **CubeIDE** → build → flash → test on hardware.

## Hardware

- NUCLEO-F446RE (no external wiring needed; uses on-board parts)

## Pin configuration

| Pin | Mode | User label | Board part |
|---|---|---|---|
| PA5 | GPIO_Output | `LED_GREEN` | LD2, green user LED |
| PC13 | GPIO_EXTI13 | `BUTTON` | B1, blue user button |
| PA2 / PA3 | USART2 (async, 115200 8N1) | — | ST-LINK virtual COM port |

![CubeMX pinout](../docs/media/01-cubemx-pinout.png)

## Code

Inside the main loop (`Core/Src/main.c`, `USER CODE BEGIN 3`):

```c
if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET)
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);    // LED on
}
else
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);  // LED off
}
```

## How it works

The button is **active-low**: pressing it connects PC13 to ground, so the pin reads `0` when pressed and `1` when released (held high by a pull-up resistor).

```
Released:  3.3V ──[pull-up]──┬── PC13  → reads 1
                             │
Pressed:                     └──[button]── GND → reads 0
```

The loop polls the pin continuously and mirrors its state onto PA5.

## Problems I faced and how I fixed them

| Problem | Root cause | Fix |
|---|---|---|
| PA5 and PC13 missing from the GPIO table | The newer CubeMX reserves the LED and button for the board support package (BSP) | Disabled BSP *Human Machine Interface* and configured the pins manually as GPIO |
| SPI1 shown as unavailable (⊘) | Its clock pin options are PA5 (my LED) and PB3 (SWO debug); both taken | Learned to check pin conflicts; SPI2/SPI3 remain free if needed |
| "Unable to Launch" on first Run | No launch configuration existed yet | *Run As → STM32 C/C++ Application* to create one |
| CubeMX said **LED1**, the board says **LD2** | Software counts only user LEDs; the board label counts all LEDs | Identify hardware by pin name (PA5), not by label |
| Risk of build errors from the project path | Space in the folder name and a cloud-synced Documents folder | Moved projects to a short local path with no spaces |

## What I learned

- The CubeMX → CubeIDE workflow, and why code must stay inside `USER CODE` blocks
- GPIO input and output with HAL
- Active-low logic and pull-up resistors
- Reading the build output: this program uses ~8.4 KB Flash and ~1.6 KB RAM

## Next

➡ **02 · Button interrupt**: replace polling with an EXTI interrupt so the CPU reacts only when the button is pressed.
