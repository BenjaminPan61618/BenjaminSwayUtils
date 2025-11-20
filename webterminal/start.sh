#!/bin/bash

# 启动第一个Python命令并放到后台
python3 ./webterminal/terminal_server.py &
TERM_PID=$!

# 启动第二个Python命令并放到后台
python3 -m http.server 8000 &
HTTP_PID=$!

# 等待一段时间让进程初始化
sleep 2

# 检查两个进程是否都在运行
if ! kill -0 $TERM_PID 2>/dev/null; then
    notify-send "Web Terminal Error" "Failed to start terminal server" --urgency=critical
    kill $HTTP_PID 2>/dev/null
    exit 1
fi

if ! kill -0 $HTTP_PID 2>/dev/null; then
    notify-send "Web Terminal Error" "Failed to start HTTP server" --urgency=critical
    kill $TERM_PID 2>/dev/null
    exit 1
fi

# 如果都正常运行，显示成功消息
notify-send "Web Terminal" "Successfully started both services" --urgency=normal

# 等待任一进程结束
wait -n $TERM_PID $HTTP_PID

# 获取退出的进程状态
EXIT_CODE=$?

# 终止另一个进程
kill $TERM_PID $HTTP_PID 2>/dev/null

# 检查哪个进程失败并通知
if ! kill -0 $TERM_PID 2>/dev/null; then
    notify-send "Web Terminal Error" "Terminal server stopped unexpectedly" --urgency=critical
elif ! kill -0 $HTTP_PID 2>/dev/null; then
    notify-send "Web Terminal Error" "HTTP server stopped unexpectedly" --urgency=critical
fi

exit $EXIT_CODE