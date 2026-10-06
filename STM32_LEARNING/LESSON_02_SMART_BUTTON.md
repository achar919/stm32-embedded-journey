# 02 · Smart Button + UART Logging

> Each press of the blue button toggles the LED, counts the press, and prints the result to my PC over UART.

**Date:** 06 Oct 2026 · **Board:** NUCLEO-F446RE · **Tools:** STM32CubeIDE, Tera Term

```
ready press the blue button in the board
Press 1: LED ON
Press 2: LED OFF
Press 3: LED ON
```

> This lesson builds on [Lesson 01](README.md) in the same project. The Lesson 01 version of `main.c` is preserved in the Git history.

---

## Goal

Move from "LED on while the button is held" to real product behaviour:

1. **One press = one action**, no matter how long the button is held
2. **Ignore switch bounce** so a single press is never counted twice
3. **Print debug messages to the PC** using `printf` over UART

---

## How the data reaches my PC

```
printf() → __io_putchar() → USART2 (PA2) → ST-LINK USB virtual COM port → Tera Term
```

`printf` builds the text, but on a microcontroller it has no screen to write to. The `__io_putchar` function is the bridge that sends each character out through USART2.

**Serial settings:** 115200 baud, 8 data bits, no parity, 1 stop bit (115200 8N1).

---

## Code (`Core/Src/main.c`)

**Includes** (`USER CODE BEGIN Includes`)
```c
#include <stdio.h>
```

**Variables** (`USER CODE BEGIN PV`)
```c
GPIO_PinState lastbutton = GPIO_PIN_SET;   // button state in the previous loop (SET = released)
uint32_t pressCount = 0;                   // total number of real presses
```

**Redirect printf to UART** (`USER CODE BEGIN 0`)
```c
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

**Startup** (`USER CODE BEGIN 2`, runs once after the UART is initialised)
```c
setvbuf(stdout, NULL, _IONBF, 0);          // send every character immediately (no buffering)
printf("ready press the blue button in the board \r\n");
```

**Main loop** (`USER CODE BEGIN 3`)
```c
GPIO_PinState nowButton = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);

if (lastbutton == GPIO_PIN_SET && nowButton == GPIO_PIN_RESET)   // falling edge = new press
{
    HAL_Delay(20);                                                // let contact bounce settle

    if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET)   // still pressed = real press
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        pressCount++;

        if (HAL_GPIO_ReadPin(LED_GREEN_GPIO_Port, LED_GREEN_Pin) == GPIO_PIN_SET)
            printf("Press %lu: LED ON\r\n", pressCount);
        else
            printf("Press %lu: LED OFF\r\n", pressCount);
    }
}

lastbutton = nowButton;                                           // remember state for next loop
```

---

## How it works

### 1. Edge detection

The main loop runs millions of times per second, so one human press (~200 ms) is seen as "pressed" thousands of times. Toggling on every "pressed" reading makes the LED flicker and end in a random state.

The fix is to act only on the **change** from released to pressed (a *falling edge*, since the button is active-low):

| Loop | Last | Now | Action |
|---|---|---|---|
| 1 | Released | Released | — |
| 2 | Released | **Pressed** | ✅ Toggle, count, print |
| 3 | Pressed | Pressed | — (still held) |
| 4 | Pressed | Released | — (finger lifted) |

### 2. Debouncing

Mechanical contacts bounce for a few milliseconds when pressed, producing several fake edges:

```
Expected:  ────┐________________
Real:      ────┐_┌┐_┌┐__________   ← bounce (~5–20 ms)
```

Waiting 20 ms and confirming the button is still pressed filters these out.

### 3. Global vs local variables

- `lastbutton` and `pressCount` are **global**: they keep a permanent place in RAM, so they remember their values across loops.
- `nowButton` is **local**: it's created fresh in each loop iteration and discarded afterwards.

If `pressCount` were declared inside the loop, it would reset every iteration and never pass 1.

---

## C concepts I learned

| Concept | What I learned |
|---|---|
| Exact-size types | `uint8_t` (1 byte, 0–255), `uint16_t` (2 bytes), `uint32_t` (4 bytes). Embedded code uses these because plain `int` varies between platforms. |
| Overflow | A `uint8_t` holding 255 becomes **0** after `+ 1`. The wrong type size causes silent bugs. |
| Memory addresses | `&variable` gives its address. On the STM32, variables live in RAM starting at `0x20000000`. |
| `=` vs `==` | `=` stores a value; `==` compares. Mixing them compiles but behaves wrongly. |
| `&&` | Logical AND: both conditions must be true |
| `++` | Shortcut for "add 1" |
| `printf` placeholders | `%u` for `uint8_t`/`uint16_t`, `%lu` for `uint32_t` (it's `unsigned long` on ARM GCC), `%p` for addresses |
| `enum` / `typedef` | `GPIO_PinState` is an enum with two named values: `GPIO_PIN_RESET` (0) and `GPIO_PIN_SET` (1) |
| HAL is just C | `HAL_GPIO_ReadPin` reads the GPIO **IDR** register and masks one bit. Ctrl + click in CubeIDE shows the source. |

---

## Problems I faced and how I fixed them

| Problem | Root cause | Fix |
|---|---|---|
| Build failed: *implicit declaration of function `printf`* | `stdio.h` not included | Added `#include <stdio.h>` in the Includes block |
| `'lastButton' undeclared` | C is case-sensitive: declared `lastbutton`, used `lastButton` | Used one spelling everywhere |
| Code built and ran, but nothing appeared in the terminal | `printf` had no output path, and stdout was buffered | Added `__io_putchar()` to send via USART2, and `setvbuf(stdout, NULL, _IONBF, 0)` |
| Startup message never seen | It printed before the terminal was connected | Press **RESET** after opening Tera Term |
| Tera Term installer: *"does not support this version of Windows"* | Downloaded the ARM64 build on an x64 PC | Installed the **x64** build |
| Only `COM1` listed in Tera Term | COM1 is the PC's own port; the board appears as *STLink Virtual COM Port* | Selected **Serial → STLink Virtual COM Port (COM10)** |
| Git Bash stuck on a `>` prompt | Unclosed quote in a command | `Ctrl + C`, then retyped with matching quotes |
| `git push` rejected with **GH007** | Commit would expose a private email address | Set the GitHub **noreply** email in Git and amended the commit |
| Demo video rejected (>10 MB) | Phone video too large for GitHub | Trimmed to ~8 s, removed audio, exported at 720p |

---

## Key takeaways

- **Compare "now" with "last time"** to detect events instead of states.
- **Hardware is never perfectly clean**: always debounce mechanical inputs.
- **`printf` debugging over UART** is the most-used debugging tool in embedded work.
- **Read compiler errors top-down**: the first error often causes the rest, and the `note:` lines frequently suggest the fix.

---

## Next

➡ **03 · Button interrupt**: replace polling with an EXTI interrupt so the CPU reacts only when the button is pressed, and make the blinking **non-blocking** with `HAL_GetTick()`.
