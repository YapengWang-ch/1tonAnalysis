#!/bin/bash

MAX_THREADS=10
input_file=/home/wangyp/1ton/ReConstruction/preAnalysis/GoodFileList.txt

# 检查输入文件
if [ ! -f "$input_file" ]; then
    echo "Error: FileList not found!"
    exit 1
fi

# 创建日志目录
log_dir="../Preoutput/log"
mkdir -p "$log_dir"

# 信号处理：Ctrl+C时终止所有子进程
trap 'kill -INT $(jobs -p) 2>/dev/null; echo -e "\nInterrupted! Killing all jobs..."; exit 1' INT

# 任务执行函数
execute_task() {
    local col1=$1
    local col2=$2
    local log_file="$log_dir/data_${col1}_${col2}.log"
    

    
    # 生成文件名
    if [ "$col2" -eq 0 ]; then
        filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d.root" "$col1" "$col1")
    else
        filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d_%d.root" "$col1" "$col1" "$col2")
    fi
    # 检查文件是否存在
    # if [ ! -f "$filename" ]; then
    # # echo "Error: FileList not found!"
    #     return 0
    # fi

    # 检查log文件是否存在
    if [ ! -f "$log_file" ]; then
        # 创建log文件
        touch "$log_file"
        echo "Created log file: $log_file"
    else
        # 检查log文件中是否存在"There was a crash."
        if ! grep -q "Finish Calculation" "$log_file"; then
            # 清空log文件
            > "$log_file"
            echo "Cleared log file: $log_file"
        else
            # log文件存在且正常，跳过运行
            > "$log_file"
            # echo "Log file exists and contains no crash: $log_file"
            # return 0
        fi
    fi

    # outputfile=$(printf "/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis/run%08d_%d.root" "$col1" "$col2")
    outputfile=$(printf "/mnt/stage/wangyp/PreAnalysis/new1ton/water/debug_run%08d_%d.root" "$col1" "$col2")

    rm $outputfile
    
    echo "Processing: run${col1}_${col2}" | tee -a "$log_file"
    
    # 执行核心命令
    ./PreAnalysisData "$filename" "$outputfile" >> "$log_file" 2>&1
    
    return $?
}

# 启动任务函数
start_task() {
    local col1=$1
    local col2=$2
    # 等待空闲线程
    while [ $(jobs -r | wc -l) -ge $MAX_THREADS ]; do
        sleep 1
    done
    # 启动任务
    execute_task "$col1" "$col2" &
    pids+=($!)
    echo "Started job: run${col1}_${col2} (PID: $!)"
}

# 主循环
tasks=()
# while IFS= read -r line || [[ -n "$line" ]]; do
#     # 跳过空行和注释
#     [[ -z "$line" || "$line" =~ ^# ]] && continue

#     # 解析数据列
#     cols=($line)
#     if [[ ${#cols[@]} -lt 2 ]]; then
#         echo "Invalid line: $line" >> "$log_dir/error.log"
#         continue
#     fi

#     col1=${cols[0]}
#     col2=${cols[1]}

#     # 跳过 run43668 以前数据
#     if [ "$col1" -lt 43666 ]; then
#         continue
#     fi

#     # 启动任务
#     start_task "$col1" "$col2"
#     tasks+=("$col1 $col2")
# done < "$input_file"
for col1 in {47269..47269}; do
    # for col2 in {11..300};do
    for col2 in {0..0};do
    # 启动任务
        if [ "$col2" -eq 0 ]; then
            filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d.root" "$col1" "$col1")
        else
            filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d_%d.root" "$col1" "$col1" "$col2")
        fi
        # 检查文件是否存在
         if ! compgen -G "$filename" > /dev/null; then
             echo "${filename} not found!"
            continue;
            # break;
         fi
        start_task "$col1" "$col2"
        tasks+=("$col1 $col2")
    done
done < "$input_file"
# 等待剩余任务完成
wait

# 检查任务状态
for pid in "${pids[@]}"; do
    if wait $pid; then
        echo "Job $pid completed successfully"
    else
        failed_jobs+=("$pid")
        echo "Job $pid failed!"
    fi
done

# 检查日志文件，准备重试任务
retry_tasks=()
for task in "${tasks[@]}"; do
    col1=$(echo "$task" | cut -d' ' -f1)
    col2=$(echo "$task" | cut -d' ' -f2)
    log_file="$log_dir/data_${col1}_${col2}.log"
    if ! grep -q "Finish Calculation" "$log_file"; then
        echo "Found crash in $log_file, adding to retry queue."
        retry_tasks+=("$task")
        # 清空日志文件
        > "$log_file"
    fi
done

# 重试失败任务
if [ ${#retry_tasks[@]} -gt 0 ]; then
    echo -e "\nRetrying failed tasks..."
    pids=()  # 重置pids数组
    for task in "${retry_tasks[@]}"; do
        col1=$(echo "$task" | cut -d' ' -f1)
        col2=$(echo "$task" | cut -d' ' -f2)
        start_task "$col1" "$col2"
    done
    # 等待重试任务完成
    wait
fi

# 输出汇总报告
echo -e "\n======= Job Summary ======="
echo "Total jobs: ${#tasks[@]}"
echo "Failed jobs: ${#failed_jobs[@]}"
[ ${#failed_jobs[@]} -gt 0 ] && echo "Failed PIDs: ${failed_jobs[*]}"

exit ${#failed_jobs[@]}