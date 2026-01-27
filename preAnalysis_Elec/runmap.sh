#!/bin/sh

MAX_PROCESSES=4

# 生成所有待执行的命令

logfile="../MapPreAnalysis.log"
if [ -e $logfile ]; then
    rm $logfile
fi
# rm $logfile
# rm -rf /mnt/stage/wangyp/Simulation/new1ton/water/templates/PreAnalysis_TruQE_r1400_ABS_sub20
inputpath="/mnt/stage/wangyp/Simulation/new1ton/waterII/templates/MapMuon"
outputpath="/mnt/stage/wangyp/Simulation/new1ton/waterII/templates/PreAnalysis_MapMuon"
mkdir -p $outputpath


generate_commands() {
    for i in $(seq 100 149); do
        for j in $(seq 0 10); do
            if [ "$j" -eq 0 ]; then
                filename="${inputpath}/map${i}_muon.root"
            else
                filename="${inputpath}/map${i}_muon_${j}.root"
            fi

            prepath="${outputpath}/run${i}_${j}.root"
            if [ -e "${prepath}" ];then
                echo "file ${prepath} already exists";
            else
                if [ -e "${filename}" ]; then 
                    echo "处理文件: $filename"; 
                    ./PreAnalysisMC "$filename" "$prepath"; 
                else 
                    echo "文件未找到: $filename"; 
                fi
            fi
        done
    done
} > $logfile 

# 使用 xargs 并行执行命令
generate_commands | xargs -P $MAX_PROCESSES -I {} sh -c "{}"
