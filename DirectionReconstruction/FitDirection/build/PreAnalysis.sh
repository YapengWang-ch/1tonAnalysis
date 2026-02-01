#!/bin/bash 

# input_dir="/home/wangyp/1ton/ReConstruction/Output"
# outputfile="../Output/Muons/Muons.root"

# ./PreAnalysisData $input_dir $outputfile

max_threads=4  # 设置最大线程数
current_jobs=0 # 当前运行的作业数
mkdir -p ../Output/MC_test_d645
# 合并处理i=0到10的情况
for i in {10..19}; do
    # 根据i的值设置输入路径
    # MCinput_dir="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/PreAnalysis_75/run${i}_*.root"
    # MCinput_dir="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/PreAnalysis_PDE_withRock/run${i}_*.root"
    # MCinput_dir="/mnt/stage/wangyp/Simulation/new1ton/water/templates/PreAnalysis_TruQE_r1400/run${i}_*.root"
    MCinput_dir="/mnt/stage/wangyp/Simulation/new1ton/waterII/Muon_CJPL/GeoTest_PreAnalysis/run${i}_*.root"
    MCoutputfile="../Output/MC_test_d645/MCRun_${i}.root"

    # 控制并发：如果达到最大线程数，等待任意一个任务完成
    while (( current_jobs >= max_threads )); do
        wait -n # 等待任意后台任务完成（bash 4.3+）
        ((current_jobs--))
    done

    # 启动任务（后台运行）
    ./PreAnalysisMC "$MCinput_dir" "$MCoutputfile" &
    ((current_jobs++))
done

# 等待所有剩余任务完成
wait

echo "All tasks completed"