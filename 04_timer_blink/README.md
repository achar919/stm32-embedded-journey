# 04 · Hardware Timer Blink

> A hardware timer (TIM6) blinks the LED with exact timing, completely on its own. The button changes the blink speed while it runs.

**Date:** 15 Oct 2026 · **Board:** NUCLEO-F446RE · **Tools:** STM32CubeMX, STM32CubeIDE, Tera Term

```
Speed 1: 300 ms
Speed 2: 80 ms
Speed 0: 1000 ms
```

> Builds on [Lesson 03 · Button interrupt](../03_button_interrupt/). Next: [Lesson 05 · PWM LED fade](../05_pwm_fade/).

---

## Goal

Move timing out of the code and into hardware.

| Method | How it works | Limitation |
|---|---|---|
| `HAL_Delay()` | CPU waits doing nothing | Blocks everything |
| `HAL_GetTick()` checks | Loop compares timestamps | Timing slips if the loop is busy |
| **Hardware timer** | Timer counts independently and interrupts on time | None for this job: the CPU stays free |

---

## How a timer counts

```
Counter
 ARR ┤      ╱|      ╱|      ╱|
     │    ╱  |    ╱  |    ╱  |
   0 ┼──╱────┴──╱────┴──╱────┴──► time
             ↑       ↑       ↑
        update interrupt ("period elapsed")
```

The counter runs from 0 to **ARR**, resets, and raises an update interrupt.

**Formula:**
```
Interrupt period = (PSC + 1) × (ARR + 1) ÷ Timer clock
```

---

## Finding the timer clock

From the CubeMX **Clock Configuration** tab:

```
HSI 16 MHz → PLL (÷16 ×336 ÷4) → SYSCLK 84 MHz → HCLK 84 MHz
HCLK → APB1 ÷2 → 42 MHz (peripherals) → ×2 → 84 MHz (APB1 timers) ← TIM6
```

When the APB1 bus is divided, STM32 doubles the clock for its timers, so TIM6 runs at **84 MHz**, not 42 MHz.

---

## CubeMX configuration (TIM6)

| Setting | Value | Reason |
|---|---|---|
| Activated | ✅ | |
| Prescaler | `8399` | 84 MHz ÷ 8400 = **10 kHz** tick (0.1 ms) |
| Counter Period | `4999` | 5000 ticks = **500 ms** |
| auto-reload preload | Enable | |
| NVIC | ✅ TIM6 global interrupt | |
| SYS → Timebase Source | SysTick | Keeps TIM6 free for this lesson |

TIM6 is a 16-bit basic timer: PSC and ARR are each limited to 0–65535.

---

## Code (`Core/Src/main.c`)

**Variables** (`USER CODE BEGIN PV`)
```c
uint16_t blinkPeriod[3] = {9999, 2999, 799};   // 1000 ms, 300 ms, 80 ms (ms × 10 − 1)
uint8_t  speedIndex = 0;
```

**Start the timer** (`USER CODE BEGIN 2`)
```c
HAL_TIM_Base_Start_IT(&htim6);
```

**Timer callback** (`USER CODE BEGIN 0`)
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)                  // all timers share this callback
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    }
}
```

**Main loop** (`USER CODE BEGIN 3`): the button from Lesson 03 changes the speed
```c
if (buttonPressed)
{
    buttonPressed = 0;
    speedIndex = (speedIndex + 1) % 3;

    __HAL_TIM_SET_AUTORELOAD(&htim6, blinkPeriod[speedIndex]);   // change ARR while running
    __HAL_TIM_SET_COUNTER(&htim6, 0);                            // restart the count

    printf("Speed %u: %u ms\r\n", speedIndex, (blinkPeriod[speedIndex] + 1) / 10);
}
```

The blinking itself needs **no code in the main loop**: the timer hardware drives it.

---

## Key concepts

| Concept | Meaning |
|---|---|
| **Prescaler (PSC)** | Divides the timer clock down, like gears slowing a motor |
| **Auto-reload (ARR)** | How far the counter goes before it resets and interrupts |
| **`HAL_TIM_Base_Start_IT`** | CubeMX configures the timer but does **not** start it; this call starts it with interrupts |
| **`htim->Instance`** | Identifies which timer fired, since every timer uses the same callback |
| **`__HAL_TIM_SET_AUTORELOAD`** | Changes the period on the fly |

---

## Problems I faced and how I fixed them

| Problem | Root cause | Fix |
|---|---|---|
| LED looked **always on** instead of blinking | Prescaler was still `0`: 84 MHz ÷ 5000 ≈ 16,800 interrupts per second, far too fast for the eye | Set Prescaler to `8399` |
| Unsure which clock value to use | The Clock Configuration tab shows both **APB1 peripheral (42 MHz)** and **APB1 Timer (84 MHz)** | Timers use the **APB1 Timer clock** (84 MHz) |
| Typed value didn't stick | CubeMX only saves a field after **Enter** | Pressed Enter, then regenerated |

---

## Key takeaways

- Hardware timers give **exact, independent timing** without blocking the CPU.
- Always derive PSC and ARR from the **actual timer clock** in the clock tree.
- A configured timer does nothing until it's **started**.
- When something looks "always on", it may be switching **too fast to see**.

---

## Next

➡ [**05 · PWM LED fade**](../05_pwm_fade/): use a timer to control LED brightness with pulse-width modulation.
