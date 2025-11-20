# Benjamin Sway Utils
A bunch of handy utils for Sway Desktop on Linux. <br> <br>
在 Linux Sway 桌面上有助提升体验的工具集  <br> <br>
## Beginning 开发缘由
My PC runs Ubuntu Sway 25.04 for my daily use, but soon I found some functions I needed were missing.<br>
我日常使用 Ubuntu Sway 作为主力日用系统，但很快我发现它缺失了不少便利功能。
## 组件介绍
### Anasrava 无漏
A powerful kit for tidying up your history on Linux PCs. (Unavailable for Firefox and Python)<br>
历史查看及清理工具 （当前版本Firefox历史记录和Python历史记录暂时不可用）

### Brightness Control
A simple brightness controlling indicator based on libayatana<br>
常驻在状态栏的亮度控制器

### Make Maker
(Not tested) Makefile generator<br>
（未测试）Makefile 生成器

### oh-my-waybar Waybar Manager
A toolkit for instantly starting, killing or restarting Waybar, and you can check up all the components on Waybar.<br>
Note: This toolkit window might automatically hide.<br>
可以快速重启、重启、启动Waybar以及检查其上配置的组件（目前仅提供配置可视化功能）<br>
注意：此程序窗口在特定条件下会隐藏

### unified-launcher 统一启动器
Make all the BSU components easier to access.<br>
一站式管理所有BSU组件

### Web Terminal 网页终端
Instantly communicate on tty with other PCs through the Browsers without SSH.<br><br>
Server tested: Ubuntu Sway(Python 3.13.3)<br>
Client tested: Firefox, Chromium, WebPositive on HAIKU, Browser on LineageOS(Waydroid)<br><br>
无需SSH，在浏览器里即可通过终端与其他计算机交互！<br><br>
服务端测试: Ubuntu Sway<br>
客户端测试: Firefox, Chromium, WebPositive on Haiku, LineageOS 内建浏览器 (Waydroid)

#### 启动 WebTerminal
1. 安装依赖<br>
pip install websockets<br><br>
! 注意: 在 pip 无法直接下载安装包的情况下，可以优先考虑使用软件包管理器搜索安装缺失的组件，例如 python3-websockets (Ubuntu) <br>
非必要不建议无视破坏关系强行安装.
2. 启动
python terminal_server.py<br>
python -m http.server [端口号]

#### disk-space-monitor
磁盘空间超过阈值时发出提醒
