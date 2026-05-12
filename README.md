# 🤖 Smart Cleaning Robot — Arduino UNO

A smart floor cleaning robot built with Arduino UNO featuring **Manual Bluetooth control** and **Auto Zigzag cleaning mode** with ultrasonic sensors and a water pump.

---

## 📸 Project Overview

This robot can autonomously clean a room using a zigzag pattern while maintaining wall distance using ultrasonic sensors. It can also be controlled manually via a Bluetooth mobile app built with MIT App Inventor.

---

## ✨ Features

- **Manual Mode** — Control via Bluetooth mobile app (Forward, Backward, Left, Right, Stop)
- **Auto Mode** — Autonomous zigzag cleaning pattern (20→40→60→80→60→40→20 cm)
- **Water Pump** — Manual: 2 sec burst on button press / Auto: 2 sec every 20 seconds
- **Wall Following** — Right/Left ultrasonic sensors keep robot on track
- **Obstacle Detection** — Front ultrasonic sensor triggers 180° turn at 30cm
- **Auto Stop** — Robot stops completely when zigzag sequence is finished
- **Non-blocking turns** — Robot stays responsive during all turns

---

## 🛠️ Hardware

| Component | Quantity |
|---|---|
| Arduino UNO | 1 |
| DC Motor (12V) | 2 |
| L298N Motor Driver | 2 |
| HC-SR04 Ultrasonic Sensor | 3 |
| HC-05 Bluetooth Module | 1 |
| Water Pump | 1 |
| 18650 Battery Pack (12V) | 2 |
| Wheels (rubber) | 4 |

---

## 📌 Pin Connections

### L298N #1 — Left Motor (Battery 1)

| Arduino Pin | L298N #1 | Note |
|---|---|---|
| Pin 8 | IN1 | Direction |
| Pin 9 | IN2 | Direction |
| Pin ~5 | ENA | Speed PWM (remove jumper) |
| — | OUT1/OUT2 | Left motor wires |

### L298N #2 — Right Motor + Pump (Battery 2)

| Arduino Pin | L298N #2 | Note |
|---|---|---|
| Pin 10 | IN1 | Right motor direction |
| Pin 11 | IN2 | Right motor direction |
| Pin ~6 | ENA | Right motor speed PWM (remove jumper) |
| — | OUT1/OUT2 | Right motor wires |
| Pin 4 | IN3 | Pump direction |
| Pin 2 | IN4 | Pump direction |
| Pin ~3 | ENB | Pump speed PWM (remove jumper) |
| — | OUT3/OUT4 | Pump wires |

### HC-05 Bluetooth

| HC-05 | Arduino |
|---|---|
| TX | Pin 0 (RX) |
| RX | Pin 1 (TX) — use voltage divider 5V→3.3V |
| VCC | 5V |
| GND | GND |

### Ultrasonic Sensors (HC-SR04)

| Sensor | TRIG | ECHO |
|---|---|---|
| Front | A0 | A1 |
| Left | A2 | A3 |
| Right | A4 | A5 |

All sensors: VCC → 5V, GND → GND

### Power Wiring

```
Battery 1 (+) → L298N #1 VS
Battery 2 (+) → L298N #2 VS

Battery 1 (-) ──┐
Battery 2 (-) ──┼── All GNDs tied together → Arduino GND
Arduino GND  ──┘

L298N #1 5V out → Arduino 5V pin
```

> ⚠️ **CRITICAL:** All GNDs must be connected together or nothing works.

---

## 🔄 Auto Zigzag Pattern

```
Wall ──────────────────────────────────────
→→→→→↓  20cm from right wall
       ↓ 180° turn
←←←←←←←←  40cm from left wall
↓ 180° turn
→→→→→→→→→→→→  60cm from right wall
              ↓ 180° turn
←←←←←←←←←←←←←←←←  80cm from left wall
                     ↓ 180° turn (reverse)
→→→→→→→→→→→→  60cm from right wall
              ↓ 180° turn
←←←←←←←←  40cm from left wall
↓ 180° turn
→→→→→↓  20cm → DONE → everything stops
Wall ──────────────────────────────────────
```

---

## 📱 MIT App Inventor — Bluetooth Commands

| Button | Command Sent | Action |
|---|---|---|
| Forward | `F` | Move forward (hold) |
| Backward | `B` | Move backward (hold) |
| Left | `L` | Turn left 90° |
| Right | `R` | Turn right 90° |
| Stop | `S` | Stop all motors |
| Pump | `P` | Pump on for 2 seconds |
| Pump OFF | `O` | Force pump off |
| Auto Mode | `A` | Start zigzag cleaning |
| Manual Mode | `M` | Return to manual control |

### App Inventor Components Needed
- `BluetoothClient1` (non-visible)
- `ListPicker1` — Bluetooth device selection
- Buttons for each command above
- Label for status display

### Key Blocks Logic
- Forward/Backward: `SendText("F")` on **TouchDown**, `SendText("S")` on **TouchUp**
- Left/Right: `SendText("L")` or `SendText("R")` on **Click** (Arduino handles timing)
- Pump: `SendText("P")` on **Click** — Arduino auto stops after 2 sec

---

## 💻 Software

### Requirements
- Arduino IDE
- **NewPing library** by Tim Eckel
  - Install: Sketch → Include Library → Manage Libraries → search "NewPing" → Install

### Upload Steps
1. Install Arduino IDE
2. Install NewPing library
3. Open `smart_robot_pumpfixed.ino`
4. Select board: **Arduino UNO**
5. Select correct COM port
6. Click Upload

---

## ⚙️ Tuning Constants

| Constant | Default | How to tune |
|---|---|---|
| `TURN_90_MS` | 4000ms | Increase if robot doesn't reach 90°, decrease if overshoots |
| `TURN_180_MS` | 5500ms | Same — should be roughly double TURN_90_MS |
| `FRONT_OBSTACLE` | 30cm | Increase if robot hits wall before turning |
| `CURVE_SLOW` | 180 | Lower = sharper curve during wall follow |
| `PUMP_INTERVAL` | 20000ms | How often pump fires in auto mode |

---

## 🐛 Common Issues

| Problem | Cause | Fix |
|---|---|---|
| Robot not moving on ground | Battery low or thin wires | Charge battery, use 20AWG+ wire |
| Motors heating L298N | Too much current draw | Add heatsink to L298N |
| Pump not working | Wrong OUT pins | Check pump on OUT3/OUT4, ENB jumper removed |
| Left drifts faster | Pump weight on right side | Already compensated in code |
| COM port error | Wrong port or Serial Monitor open | Close Serial Monitor, check Tools → Port |
| NewPing not found | Library not installed | Sketch → Manage Libraries → install NewPing |
| Robot not going straight | Motor speed difference | Tune LEFT_SPEED constant |

---

## 📁 File Structure

```
smart-cleaning-robot/
├── smart_robot_pumpfixed.ino   ← Main Arduino code
├── README.md                   ← This file
```

---

## 🧠 How It Works

### Manual Mode
Arduino listens on Serial (Bluetooth) for single character commands. Each command triggers a motor function immediately. Left/Right turns use non-blocking `millis()` timer so robot stays responsive.

### Auto Mode
1. Robot starts moving forward
2. Reads right ultrasonic sensor every 60ms
3. Curves left/right to maintain target wall distance
4. When front sensor reads < 30cm → stops → turns 180° → advances zigzag index
5. Alternates between right wall and left wall following
6. After completing 20→40→60→80→60→40→20 sequence → stops everything

### Pump Logic
- Manual: single `startPump()` call → `updatePump()` runs every loop → stops after 2000ms
- Auto: `updateAutoPump()` runs every loop → fires every 20000ms → same 2 sec burst

---

## 👤 Author

Built as a school/personal project using Arduino UNO.

---

## 📄 License

MIT License — free to use and modify.