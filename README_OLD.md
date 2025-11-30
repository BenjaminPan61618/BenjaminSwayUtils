# Benjamin Sway Utils
A bunch of handy utils for Sway Desktop on Linux. <br> <br>
在 Sway 桌面 (Ubuntu) 上有助提升体验的工具集  <br> <br>
## Beginning 开发缘由
My PC runs Ubuntu Sway 25.04 for my daily use, but soon I found some functions I needed were missing.<br><br>
我日常使用 Ubuntu Sway 25.04 作为主力日用系统，但很快我发现它缺失了不少便利功能。
## Components 组件介绍
### Anasrava 无漏
A powerful kit for tidying up your history on Linux PCs. <br>(Unavailable for Firefox and Python)<br><br>
历史查看及清理工具 <br>（当前版本 Firefox 和Python 历史记录暂时不可用）

### Brightness Control 亮度调节
A simple brightness controlling indicator based on libayatana<br><br>
常驻在状态栏的亮度控制器，基于 ayatana

### Make Maker
(Not tested) Makefile generator<br><br>
（未测试）Makefile 生成器

### oh-my-waybar Waybar Manager
A toolkit for instantly starting, killing or restarting Waybar, and you can check up all the components on Waybar.<br>
  Note: This toolkit window might automatically hide.<br><br>
可以快速重启、重启、启动 Waybar以及检查其上配置组件（目前仅提供配置可视化功能）的工具<br>
  注意：此程序窗口在特定条件下会隐藏

### unified-launcher 统一启动器
Make all the BSU components easier to access.<br><br>
一站式管理所有BSU组件

### Web Terminal 网页终端
Instantly communicate on tty with other PCs through the Browsers without SSH.<br><br>
Server tested: Ubuntu Sway(Python 3.13.3)<br>
Client tested: Firefox, Chromium, WebPositive (Haiku in QEMU), Browser on LineageOS (Waydroid)<br><br>
无需SSH，在浏览器里即可通过终端与其他计算机交互！<br>
服务端测试: Ubuntu Sway<br>
客户端测试: Firefox, Chromium, WebPositive (Haiku), LineageOS 内建浏览器 (Waydroid)

#### 启动 WebTerminal
1. Installing Requirements 安装依赖<br>
pip install websockets<br>
** Note: In order to avoid breaking dependencies, some Linux distros might suggest installing Python packages through Package Manager such as apt, dnf, zypper, pacman ,etc. rather than pip.<br> e.g. sudo apt install python3-websockets ** <br>
** 注意: 在 pip 无法直接下载安装包的情况下，可以优先考虑使用软件包管理器搜索安装缺失的组件，例如： 在 Ubuntu 中，可以通过 apt 安装 python3-websockets  ** <br>
非必要情况下，不建议无视并破坏依赖关系，强行通过 pip 安装.
2. Starting Up 启动<br>
python terminal_server.py<br>
python -m http.server [端口号/Port] <br>
部分发行版可能要求用python3替代python.<br>
You might need to use python3 instead of python on some specific distros like Ubuntu.
#### disk-space-monitor
A small scipt for noticing when the disk space is about to be void. <br><br>
可以在磁盘空间超过阈值时发出提醒的脚本

## 开发鸣谢列表
Anasrava: Dean Hunter<br>
Brightness Control: Dean Hunter, Ma Ling<br>
Desktop Classifier: Chet Turner, Dean Hunter, Curtis Pointer, Ma Ling<br>
Make Maker: Dean Hunter<br>
Waybar Manager: Dean Hunter<br>
Web Terminal: Dean Hunter<br><br><br>

Dean Hunter = DeepSeek<br>
Ma Ling = QWen<br>
Curtis Pointer = Cursor<br>
Chet Turner = ChatGPT
