# Benjamin Sway Utils
<a href="README_zh.md">阅读中文README</a><br><br>
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

A comprehensive collection of handy utilities for Sway Desktop on Linux, designed to enhance your desktop experience.

## ✨ Overview

My daily driver runs Ubuntu Sway 25.04, but I soon discovered several missing functionalities that impacted productivity. This project aims to fill those gaps with practical, user-friendly tools.

## 🛠️ Components

### 🔍 Anasrava (History Cleaner)
A powerful toolkit for managing and tidying up your system history on Linux PCs.

**Current Limitations:**
- Firefox history cleaning temporarily unavailable
- Python history cleaning temporarily unavailable

### 💡 Brightness Control
A system tray indicator for brightness control based on libayatana.

**Note:** Automatic brightness feature is currently not working properly.

### 📝 Make Maker
*(Untested)* An automated Makefile generator to streamline your development workflow.

### 🎛️ oh-my-waybar (Waybar Manager)
A comprehensive toolkit for managing Waybar with instant start, kill, and restart capabilities. Includes component inspection features.

**Features:**
- Quick start/stop/restart Waybar
- Visual configuration inspection
- Component status monitoring

**Note:** The toolkit window may automatically hide under specific conditions.

### 🚀 Unified Launcher
Centralized access point for all BSU components, making them easily accessible from one interface.

### 🌐 Web Terminal
Browser-based terminal communication between PCs without requiring SSH.

**Tested Environments:**
- **Server:**
  -  Ubuntu Sway (Python 3.13.3)
  -  Fedora 42 (WSL2)
- **Clients:** 
  - Firefox
  - Chromium
  - WebPositive (Haiku in QEMU)
  - LineageOS Browser (Waydroid)

#### 🚀 Getting Started with Web Terminal

1. **Install Dependencies**
   ```bash
   pip install websockets
   ```

   Important Note: To avoid dependency conflicts, some Linux distributions recommend installing Python packages through their package manager instead of pip:

   ```bash
   # Ubuntu/Debian
   sudo apt install python3-websockets
   
   # Fedora
   sudo dnf install python3-websockets
   
   # Arch Linux
   sudo pacman -S python-websockets
   ```

1. Start the Services
   ```bash
   # Start terminal server
   python terminal_server.py
   
   # Start HTTP server (default port: 8000)
   python -m http.server 8000
   ```
   Note: Some distributions may require using python3 instead of python.

💾 Disk Space Monitor

A lightweight script that alerts you when disk space is running low.

👥 Development Credits

Component Contributors
Anasrava: Dean Hunter<br>
Brightness Control: Dean Hunter, Ma Ling<br>
Desktop Classifier: Chet Turner, Dean Hunter, Curtis Pointer, Ma Ling<br>
Make Maker: Dean Hunter<br>
Waybar Manager: Dean Hunter<br>
Web Terminal: Dean Hunter

🤖 Contributors

· Dean Hunter = DeepSeek<br>
· Ma Ling = QWen<br>
· Curtis Pointer = Cursor<br>
· Chet Turner = ChatGPT

📄 License

This project is licensed under the GPL v3 License - see the LICENSE file for details.
