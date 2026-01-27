#!/bin/bash

directory="./log/"

# 检查路径是否存在
if [ ! -d "$directory" ]; then
    echo "Directory '$directory' does not exist."
    exit 1
fi

# 遍历路径下的所有 .log 文件
find "$directory" -type f -name "*.log" | while read -r file; do
    if [ -f "$file" ]; then
        if grep -q "crash" "$file"; then
            # echo "File '$file' does NOT contain 'Finish Calculation'."
            echo "File '$file' contains 'crash'."
        fi
    else
        echo "File '$file' does not exist."
    fi
done

