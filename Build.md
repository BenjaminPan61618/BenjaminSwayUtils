# 构建 Build
Benjamin Sway Utils is a bunch of utils, so the components are individual during building.

Benjamin Sway Utils 是一套工具集，因此各组建独立构筑。

## Anasrava
### 依赖
#### Ubuntu/Debian
sudo apt-get install build-essential libgtk-3-dev libsqlite3-dev libxml2-dev libxml2-dev

#### Fedora/RHEL
sudo dnf install gcc gtk3-devel sqlite-devel libxml2-devel libxml2-devel
### 编译命令
gcc -o anasrava anasrava.c `pkg-config --cflags --libs gtk+-3.0` `pkg-config --cflags --libs libxml-2.0` -lsqlite3
## Brightness Control
### 依赖
#### 系统工具依赖:
brightnessctl, swaymsg
#### 编译工具安装
sudo apt-get install libappindicator3-dev libjson-glib-dev libgtk-3-dev build-essential
### 编译命令
gcc -o brightness-control brightness_control.c `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1 json-glib-1.0`
## Desktop Classfier
### 依赖
#### Ubuntu/Debian
sudo apt update
sudo apt install build-essential pkg-config
sudo apt install libgtk-3-dev libglib2.0-dev libjson-glib-dev libayatana-appindicator3-dev
#### Fedora/RHEL
sudo dnf install gcc gtk3-devel glib2-devel json-glib-devel libappindicator-gtk3-devel
#### Arch
sudo pacman -S base-devel gtk3 glib2 json-glib libappindicator-gtk3
### 编译
在目录下make即可
## Oh-My-Waybar (Waybar Manager)
### 依赖
#### Ubuntu/Debian
sudo apt update
sudo apt install build-essential pkg-config
sudo apt install libgtk-3-dev libjson-glib-dev
### 编译
gcc -o waybar-manager main.c `pkg-config --cflags --libs gtk+-3.0 json-glib-1.0`
## Unified Launcher
### 依赖
#### Ubuntu/Debian
sudo apt update
sudo apt install build-essential pkg-config
sudo apt install libgtk-3-dev libjson-glib-dev
### 编译
在目录下make即可
## 启动 Web Terminal
python terminal_server.py
python -m http.server [端口号]
