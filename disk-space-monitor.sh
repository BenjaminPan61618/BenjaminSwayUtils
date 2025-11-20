#!/bin/bash

# 磁盘监控脚本
# 设置阈值（百分比）
ROOT_THRESHOLD=85
HOME_THRESHOLD=90
CHECK_INTERVAL=300  # 检查间隔（秒）

# 检查是否支持桌面通知
if command -v notify-send &> /dev/null; then
    NOTIFY_AVAILABLE=true
else
    echo "警告: notify-send 不可用，将使用终端提示"
    NOTIFY_AVAILABLE=false
fi

send_alert() {
    local partition=$1
    local usage=$2
    local message="分区 $partition 使用率已达到 ${usage}%，请及时清理！"
    
    if [ "$NOTIFY_AVAILABLE" = true ]; then
        notify-send -u critical "磁盘空间警告" "$message"
    else
        echo "警告: $message"
    fi
    
    # 额外在终端显示清理建议
    echo "清理建议:"
    echo "1. 运行 'sudo apt autoremove' 删除无用包 (Ubuntu/Debian)"
    echo "2. 运行 'sudo journalctl --vacuum-time=7d' 清理日志"
    echo "3. 检查 ~/.cache, ~/.local/share/Trash 目录"
    echo "4. 使用 'du -sh ~/.* | sort -hr' 查看大文件"
}

while true; do
    # 获取根分区使用率
    root_usage=$(df / | awk 'NR==2 {print $5}' | sed 's/%//')
    
    # 获取home分区使用率
    home_usage=$(df /home | awk 'NR==2 {print $5}' | sed 's/%//' 2>/dev/null || echo "0")
    
    # 检查根分区
    if [ "$root_usage" -ge "$ROOT_THRESHOLD" ]; then
        send_alert "/" "$root_usage"
    fi
    
    # 检查home分区
    if [ "$home_usage" -ge "$HOME_THRESHOLD" ]; then
        send_alert "/home" "$home_usage"
    fi
    
    sleep $CHECK_INTERVAL
done