#!/bin/bash

# netfileorstring-server/src/netfile_monitor.sh
# 监控 netfileorstring-server 程序的运行状态,如果程序异常退出,则自动重启程序

while true; do
    # 检查 netfileorstring-server 是否在运行
    if ! pgrep -f "netfileorstring-server" > /dev/null; then
        echo "$(date): netfileorstring-server 未运行，正在重启..."
        # 启动 netfileorstring-server
        ./netfileorstring-server/src/server &
    fi
    # 每隔 10 秒检查一次
    sleep 5
done