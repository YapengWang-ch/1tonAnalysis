#!/bin/bash

# 获取工具安装目录
TOOL_DIR="/home/wangyp/1ton/ReConstruction/WavePlot/build"

# 运行C程序
"$TOOL_DIR/EventMC" "$@"
# echo $path
# 运行Python程序
# echo $@
python3 "$TOOL_DIR/plot2D.py" "$@"