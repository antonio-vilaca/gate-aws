# Gate-AWS: IoT Control System for Tilting Garage Door

Welcome! This is the repository for the **IoT Tilting Gate Control Project**, currently under active development, hardware testing, and debugging.

## 🔧 Project Overview

This system is designed to control the opening and closing of a garage door using the **AWS Cloud platform** and an **ESP32** microcontroller. It operates with three relays:

- Two for opening and closing the gate
- One for a lamp or visual indicator

The system also utilizes **four reed switch sensors**:

- Two to monitor gate open/close status
- One as a safety sensor to detect the presence of a vehicle underneath the door, which triggers an immediate stop
- One reserved for future expansion

Control of the gate can be performed via:

- ☁️ **AWS IoT Cloud** (remote access)
- 📱 **Bluetooth app** (local fallback)
- 🗣️ Planned integration with **Google Home**

### ⚙️ Motor Speed and Ramp Control

The system includes **motor speed adjustment** and **smooth acceleration/deceleration ramps** for opening and closing the gate.  
> ⚠️ Please refer to your motor's technical manual to verify compatibility with speed control and ramp functions.

## 🚀 Status

- ✅ Codebase rewrite in ESP-IDF (pure C/C++)
- 🧪 Hardware bench testing in progress
- 🔐 SSH-enabled GitHub push
- 📡 Cloud and Bluetooth integration in development

---

Stay tuned for updates! Contributions and suggestions are welcome.
