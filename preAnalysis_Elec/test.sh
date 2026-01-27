#!/bin/bash

# 设置最大并行线程数
MAX_JOBS=10

# 创建日志目录
mkdir -p ../Preoutput/log

# 函数定义
test_elec(){
    local col1=$1
    local filename=$(printf "/home/changxu/Electronics/pinehand/20251125_Cs137/run1/%d.root" "$col1")
    mkdir -p /mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/firstpeak_thd100_full
    local outputfile=$(printf "/mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/firstpeak_thd100_full/run%d.root" "$col1")
    
    if [ ! -f "$filename" ]; then
        echo "Error 404: File not found: $filename" >&2
        return 1
    fi

    echo "处理文件: $filename"
    echo "输出到: $outputfile"
    
    ./PreAnalysisData "$filename" "$outputfile"
}

# 导出函数
export -f test_elec

echo "开始并行处理 0-821 的任务，最大线程数: $MAX_JOBS"
echo "=========================================="

# 使用seq生成任务列表并通过xargs并行执行
seq 0 821 | xargs -P $MAX_JOBS -I {} bash -c 'test_elec "$@"' _ {}

echo "=========================================="
echo "所有任务执行完毕！"

# test_elec 0