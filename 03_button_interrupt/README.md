# 03 · Button Interrupt

> The button no longer gets checked millions of times per second. The hardware **interrupts** the CPU only when it's pressed.

**Date:** 14 Oct 2026 · **Board:** NUCLEO-F446RE · **Tools:** STM32CubeMX, STM32CubeIDE, Tera Term

```
[3520 ms] Press 1
[4875 ms] Press 2
[6110 ms] Press 3
```

> Builds on [Lesson 02 · Smart button + UART logging](../02_smart_button_uart/). Next: [Lesson 04 · Hardware timer blink](../04_timer_blink/).

---

## Goal

Replace **polling** (the main loop constantly reading the button) with an **external interrupt (EXTI)**, so the CPU reacts instantly and stays free for other work.

| | Polling (Lesson 02) | Interrupt (this lesson) |
|---|---|---|
| How | Loop reads the pin continuously | Hardware notifies the CPU on a pin change |
| CPU time | Spent checking | Free until something happens |
| Can miss a press? | Yes, if the loop is busy | No, the hardware catches it |

---

## How the interrupt reaches my code

```
Press B1 → PC13 falls HIGH→LOW → EXTI line 13 detects the edge
→ NVIC pauses main()
→ EXTI15_10_IRQHandler()        (generated, stm32f4xx_it.c)
→ HAL_GPIO_EXTI_IRQHandler()    (HAL)
→ HAL_GPIO_EXTI_Callback()      (my code)
→ main() resumes
```

- **EXTI**: hardware that watches pins for edges
- **NVIC**: the interrupt controller that decides what runs and when
- PC13 is on **EXTI line 13**; lines 10–15 share one handler, hence `EXTI15_10`

---

## CubeMX configuration

| Setting | Value |
|---|---|
| PC13 mode | `GPIO_EXTI13` |
| GPIO mode | External Interrupt, **falling edge** (button is active-low) |
| Pull | No pull-up / pull-down (the board has its own resistor) |
| User label | `BUTTON` |
| NVIC | ✅ **EXTI line[15:10] interrupts** |

---

## Code (`Core/Src/main.c`)

**Variables** (`USER CODE BEGIN PV`)
```c
volatile uint8_t buttonPressed = 0;   // set by the interrupt, read by main()
uint32_t pressCount = 0;
```

**Interrupt callback** (`USER CODE BEGIN 0`, as its own function next to `__io_putchar`)
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t lastPressTime = 0;
    uint32_t now = HAL_GetTick();

    if (GPIO_Pin == BUTTON_Pin && (now - lastPressTime) > 200)   // ignore bounce
    {
        buttonPressed = 1;                                         // raise the flag
        lastPressTime = now;
    }
}
```

**Main loop** (`USER CODE BEGIN 3`)
```c
if (buttonPressed)
{
    buttonPressed = 0;                                            // lower the flag
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    pressCount++;
    printf("[%lu ms] Press %lu\r\n", HAL_GetTick(), pressCount);
}
```

---

## Key concepts

### The flag pattern
> **Interrupt: only raise a flag (fast). Main loop: do the real work.**

Like a doorbell: the bell only rings; you walk to the door and talk to the visitor. Interrupts must finish in microseconds, so slow work (`printf`, delays) stays in `main()`.

### `volatile`
The compiler may cache a variable in a register if nothing in the loop appears to change it. It cannot see that an interrupt changes `buttonPressed`. `volatile` forces it to re-read memory every time. **Any variable shared between an interrupt and `main()` must be `volatile`.**

### `static` inside a function
`lastPressTime` keeps its value between calls, like a global, but is only visible inside the callback.

### Debouncing in an interrupt
Contact bounce fires several interrupts per press. Accepting a press only if more than 200 ms have passed since the last one filters them out.

---

## Golden rules for interrupt callbacks

| ❌ Never | Why |
|---|---|
| `HAL_Delay()` | Depends on the SysTick interrupt, which may be blocked, so the program can freeze |
| `printf()` | Too slow (~1.5 ms per line at 115200 baud) |
| Long loops or heavy work | `main()` is paused the whole time |

---

## Problems I faced and how I fixed them

| Problem | Root cause | Fix |
|---|---|---|
| **EXTI line[15:10]** missing from the NVIC list | PC13 was `GPIO_Input`, not interrupt mode | Set PC13 to `GPIO_EXTI13`; the entry then appeared |
| NVIC search didn't filter the table | CubeMX search **highlights** the first match instead of filtering | Used the ▶ arrow to jump between matches |
| `'BUTTON_Pin' undeclared` | Changing PC13's mode removed its user label, so `BUTTON_Pin` was no longer generated | Re-added the `BUTTON` label, regenerated, refreshed (F5) |
| Button did nothing **and** `printf` stopped | The callback had been pasted **inside** `__io_putchar()`, which also deleted its `HAL_UART_Transmit` line | Made them two separate functions. In C, functions are never nested. |
| Unsure whether CubeIDE had the new code | CubeIDE doesn't always notice files changed by CubeMX | Refreshed (F5) and confirmed `EXTI15_10_IRQHandler` in `stm32f4xx_it.c` and `GPIO_MODE_IT_FALLING` in `main.c` |

---

## Key takeaways

- Interrupts let hardware **notify** the CPU instead of being polled.
- Keep callbacks **short**: set a flag and return.
- Shared variables between an interrupt and `main()` must be **`volatile`**.
- After changing a pin's mode in CubeMX, **check its label** is still there.
- A misspelled callback name gives **no error**: HAL silently calls its own empty default.

---

## Next

➡ [**04 · Hardware timer blink**](../04_timer_blink/): let a timer peripheral generate exact timing on its own.
