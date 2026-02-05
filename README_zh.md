# Benjamin Sway Utils

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

专为 Linux Sway 桌面环境设计的实用工具集，旨在提升桌面使用体验。

## ✨ 项目概述

我的主力系统是 Ubuntu Sway 25.04，但在使用过程中发现缺少一些便利功能。本项目旨在通过实用、易用的工具来填补这些空白。

## 🛠️ 组件介绍

### 🔍 Anasrava (历史清理工具) 无漏
强大的系统历史记录管理和清理工具包。

**当前限制:**
- Firefox 历史记录清理暂时不可用
- Python 历史记录清理暂时不可用

### 💡 亮度调节
基于 libayatana 的系统托盘亮度控制指示器组件。

**注意:** 自动亮度功能目前无法正常工作。

### 📝 Make Maker
*(未测试)* 自动化的 Makefile 生成器，简化开发工作流程。

### 🎛️ oh-my-waybar (Waybar 管理器)
功能完整的 Waybar 管理工具包，提供快速启动、关闭和重启功能，包含组件检查功能。

**功能特性:**
- 快速启动/停止/重启 Waybar
- 可视化配置检查
- 组件状态监控

**注意:** 此工具窗口在特定条件下可能会自动隐藏。

### 🚀 统一启动器
所有 BSU 组件的启动器，让您可以从一个界面轻松访问所有工具。

### 🌐 网页终端
无需 SSH，通过浏览器在计算机之间进行终端通信。

**测试环境:**
- **服务端:**
  - Ubuntu Sway (Python 3.13.3)
  - Fedora 42 (WSL2)
- **客户端:**
  - Firefox
  - Chromium
  - WebPositive (QEMU 中的 Haiku)
  - LineageOS 浏览器 (Waydroid)

#### 🚀 启动网页终端

1. **安装依赖**
   ```bash
   pip install websockets
   ```

   重要提示: 为避免依赖冲突，某些 Linux 发行版建议使用软件包管理器而非 pip 安装 Python 包：

   ```bash
   # Ubuntu/Debian
   sudo apt install python3-websockets
   
   # Fedora
   sudo dnf install python3-websockets
   
   # Arch Linux
   sudo pacman -S python-websockets
   ```

1. 启动服务
   ```bash
   # 启动终端服务器
   python terminal_server.py
   
   # 启动 HTTP 服务器 (默认端口: 8000)
   python -m http.server 8000
   ```
   注意: 某些发行版可能需要使用 python3 代替 python。

### 💾 磁盘空间监控

轻量级脚本，在磁盘空间不足时发出提醒。

## 👥 开发鸣谢

贡献者<br>
Anasrava: Dean Hunter<br>
Brightness Control: Dean Hunter, Ma Ling<br>
Desktop Classifier: Chet Turner, Dean Hunter, Curtis Pointer, Ma Ling<br>
Make Maker: Dean Hunter<br>
Waybar Manager: Dean Hunter<br>
Web Terminal: Dean Hunter

## 🤖 镜之彼端

· Dean Hunter = DeepSeek<br>
· Ma Ling = QWen<br>
· Curtis Pointer = Cursor<br>
· Chet Turner = ChatGPT

## 📄 许可证

本项目采用 GPL v3 许可证 - 详情请参阅 LICENSE 文件。
