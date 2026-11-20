# STM32 Alarm System & ARM Assembly Calculator

CEG3136 (Computer Architecture II) — Lab 2, University of Ottawa.

Bare-metal firmware for the **STM32L552ZE (ARM Cortex-M33)** that runs two cooperative tasks from a single super-loop:

1. **Alarm system** — a GPIO/interrupt-driven state machine with LEDs, a push button and a motion sensor.
2. **Calculator** — a touchpad + display menu app whose math routines are written in **ARM Thumb-2 assembly** and called from C.

No HAL — all peripherals (GPIO, SysTick, I²C, display, touchpad) go through small register-level drivers.

## Program structure

```
main()
 ├─ ConfigureSysClk()
 ├─ Init_Alarm(), Init_Calc()
 ├─ StartSysTick()
 └─ loop forever:
      Task_Alarm()        // alarm state machine
      Task_Calc()         // calculator state machine
      UpdateDisplay()     // housekeeping
      ScanTouchpad()
      ServiceI2CRequests()
      WaitForSysTick()
```

## Alarm system (`Src/alarm.c`)

| State | LEDs | Display | Leaves on |
|---|---|---|---|
| `DISARMED` | all off | `DISARMED` (white) | short press → `ARMED` |
| `ARMED` | blue/green alternate every 1 s | `ARMED` (yellow) | motion → `TRIGGERED`; long press (≥3 s) → `DISARMED` |
| `TRIGGERED` | red on | `TRIGGERED` (red) | short press → `ARMED`; long press → `DISARMED` |

- Short press = released in under 2 s; long press = held 3 s or more.
- Button and motion sensor use edge-triggered GPIO interrupt callbacks that only set flags; all logic runs in the task.
- Every transition is logged over the console with a timestamp and the reason.

| Signal | Pin |
|---|---|
| Blue LED | PB7 |
| Red LED | PA9 |
| Green LED | PC7 |
| Motion sensor | PB9 |
| Push button | PB2 |

## Calculator (`Drivers/Src/calc.c` + `Src/math.s`)

Two menu pages (press **SHIFT** to switch):

| Key | Operation | Assembly routine |
|---|---|---|
| 1 | Increment | `Increment` |
| 2 | Decrement | `Decrement` |
| 3 | Fibonacci (recursive) | `Fibonacci` |
| 4 | Arithmetic: `+ - × ÷` (sub-menu, divide-by-zero guarded) | `Arith` |
| 5 | Factorial (iterative) | `Factorial` |
| 6 | Greatest common divisor (subtraction method) | `GCD` |
| 7 | Average of 1–10 numbers (shown to one decimal) | `Average` |
| 8 | Sort 1–10 numbers, stepped through one at a time | `Sort` |

All routines follow the AAPCS calling convention (arguments in `r0`–`r3`, result in `r0`, callee-saved registers pushed when used).

`Src/sort_original.c` is the C bubble sort that `Sort` in `math.s` was hand-translated from. It is kept for reference and **excluded from the build**.

## Repository layout

```
├── Src/                 main.c, alarm.c, math.s, sort_original.c (reference)
├── Inc/                 alarm.h
├── Drivers/
│   ├── Src/             gpio, systick, i2c, display, touchpad, console, calc, syscalls, sysmem
│   ├── Inc/             driver headers
│   └── CMSIS/           ARM / ST device headers
├── Startup/             startup_stm32l552zetxq.s
├── STM32L552ZETXQ_FLASH.ld
├── STM32L552ZETXQ_RAM.ld
├── .project / .cproject / .settings/   STM32CubeIDE project files
```

## Building & flashing

1. Install **STM32CubeIDE**.
2. Clone this repo, then **File → Import → General → Existing Projects into Workspace** and select the cloned folder.
3. Build with the **Debug** configuration (output lands in `Debug/`, which is git-ignored).
4. Connect the board over ST-LINK and **Run → Debug**.
5. Open a serial terminal on the ST-LINK virtual COM port to see the `printf` console log.

## Credits

- Driver framework and CMSIS headers supplied by the CEG3136 course; CMSIS is © ARM / STMicroelectronics under their included licenses.
