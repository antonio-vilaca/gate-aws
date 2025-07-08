# Gate Controller with ESP32

![Last update](https://img.shields.io/badge/last%20update-2025--07--07-brightgreen)

This project implements an automated gate control system using an ESP32, designed to handle opening, closing and safe stopping of a gate.  
The logic is implemented in C using ESP-IDF, FreeRTOS and GPIO drivers.

---

⚠️ **WARNING**  
This project involves AC mains electricity.  
Do **NOT** attempt to build or install it unless you are qualified to work with high-voltage systems.  
Improper handling can result in serious injury or death.

---

## 🚀 Features

- State machine with `STOPPED`, `OPENING`, `CLOSING` states.
- Uses `struct` and `enum` to organize gate state data.
- Handles:
  - **Button presses on falling edge** to start, stop or reverse movement.
  - **End-stop sensors** to stop when gate fully open or closed.
  - **Timeout safety stop** after 15 seconds (150 cycles of 100ms) if end-stop fails.
- LEDs indicate OPEN and CLOSED positions.

---

## 🔌 Hardware pinout

| Function                | GPIO |
|--------------------------|------|
| Lock relay               | 12   |
| Boot relay               | 14   |
| Sensor closed            | 13   |
| Sensor open              | 19   |
| LED green                | 33   |
| LED blue                 | 32   |
| LED open indicator       | 18   |
| LED closed indicator     | 17   |
| Button (external pull-up)| 34   |
| Motor relay M1           | 27   |
| Motor relay M2           | 26   |

---

## ⚙️ Logic overview

- **When STOPPED:**  
  - Pressing the button starts moving:
    - alternates between OPENING and CLOSING each time.

- **When MOVING (OPENING or CLOSING):**  
  - Pressing the button immediately stops the gate.

- **End-stop sensors:**  
  - Automatically stop the gate when fully open or closed.

- **Timeout:**  
  - If the gate is still moving after ~15 seconds, stops for safety.

---

## 🔄 State machine

- **STOPPED**
  - button press → OPENING (if odd press)
  - button press → CLOSING (if even press)
- **OPENING**
  - sensor open → STOPPED
  - timeout → STOPPED
  - button press → STOPPED
- **CLOSING**
  - sensor closed → STOPPED
  - timeout → STOPPED
  - button press → STOPPED

---

## 🧭 Code structure

- Uses:
  - `typedef enum` to define gate states.
  - `typedef struct` to group state, counters and last button state.
- Main logic runs in `task_sensores`, reading inputs and updating outputs every 100ms.
- Outputs controlled with `gpio_set_level`.
- Logging via `ESP_LOGI` for debug and monitoring.

---

## 🚀 Build & flash

Make sure you have ESP-IDF environment set up. Then:

```bash
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
