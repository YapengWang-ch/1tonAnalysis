#!/bin/bash

# outputfile="/mnt/stage/wangyp/Simulation/new1ton/water/CaliTest/PreAnalysis_BetaUni"
# inputfile="/mnt/stage/wangyp/Simulation/new1ton/water/CaliTest/All"

# mkdir -p ${outputfile}
# # ./PreAnalysisMC "${inputfile}/newPMT{}muon.root" "${outputfile}/newPMT{}_0.root"

# ./PreAnalysisMC "${inputfile}/Beta_5MeV_All2.root" "${outputfile}/Beta_5MeV_Uni_0.root" 

# for i in {1..100}
# do 
#     ./PreAnalysisMC "${inputfile}/Beta_5MeV_All2_${i}.root" "${outputfile}/Beta_5MeV_Uni_${i}.root" 
# done

MAX_PROCESSES=4

logfile="../preAnalysis1.log"
rm -f "$logfile"  # 安全删除
subdir="TruQE_r1400"
# inputpath="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/PMTGeo/PMT_QE_Updated"
# outputpath="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/PMTGeo/PMT_QE_Updated_PreAnalysis"
inputpath="/mnt/stage/wangyp/Simulation/new1ton/water/templates/${subdir}"
outputpath="/mnt/stage/wangyp/Simulation/new1ton/water/templates/PreAnalysis_${subdir}"
mkdir -p "$outputpath"

# 仅生成命令字符串（不执行）
generate_commands() {
    for i in $(seq 0 49); do
        for j in $(seq 0 5); do
            if (( j == 0 )); then
                filename="${inputpath}/map${i}_muon.root"
            else
                filename="${inputpath}/map${i}_muon_${j}.root"
            fi
            prepath="${outputpath}/run${i}_${j}.root"

            if [[ -e "$filename" ]]; then 
                echo "处理文件: $filename" >> "$logfile"
                # 关键修改：只生成命令字符串，不执行
                echo "./PreAnalysisMC \"$filename\" \"$prepath\""
            else 
                echo "文件未找到: $filename" >> "$logfile"
            fi
        done
    done
}

# 通过管道将命令传递给xargs并行执行
generate_commands | xargs -P $MAX_PROCESSES -I {} sh -c "{} >> $logfile 2>&1"