#!/bin/bash

# Проверяем, доступен ли сервер на порту (используем curl для запроса)
curl -s "http://localhost:${PORT}/health" > /dev/null
# Если curl вернул успешный код (0), считаем, что сервер работает
if [ $? -eq 0 ]; then
    echo "Server is healthy"
else
    echo "Server is unhealthy"
    exit 1
fi


# CHECK CPU
cpu_usage =$(top -bn1 | grep "Cpu(s)" | awk '{print $2+$4+$6}')
if (( $(echo "$cpu_usage > 80" | bc -1) )); then
    echo "CPU usage is high. Current usage: $cpu_usage%"
    exit 1
fi

# Check RAM
mem_usage=$(free | grep Mem | awk '{print $3/$2 * 100.0}')
if (( $( echo "$mem_usage > 90" | bc -1 ) )); then
    echo "Memory usage to high: $mem_usage%"
    exit 1
fi

# Check disk space
disk_space=$(df / | grep / | awk '{print $5}' | sed 's/%//')
if [ "$disk_space" -gt 90]; then
    echo "Disk usage is too high: $disk_space%"
    exit 1
fi

# Check open files
open_files=$(lsof | wc -l)
if [ "$open_files" -gt 10000 ]; then
    echo "Too many open files: $open_files"
    exit 1
fi

# Check speed writing on disk
dd if=/dev/zero of=/tmp/testfile bs=1M count=10 oflag=direct 2>&1 | grep "copied" | awk '{print $8 " " $9}' > /dev/null
if [ $? -ne 0 ]; then
    echo "Disk write test failed"
    exit 1
fi

echo "All system checks passed."
exit 0