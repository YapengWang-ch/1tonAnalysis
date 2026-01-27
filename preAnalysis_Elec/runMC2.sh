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

# 生成所有待执行的命令

logfile="$HOME/1ton/ReConstruction/MuonFlux/preAnalysis2.log"
rm $logfile
subdir="OutsidePMT"
# mkdir -p /mnt/stage/wangyp/Simulation/new1ton/water/templates/PreAnalysis_plate
inputpath="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/SysUctt/PMTPosition/${subdir}"
outputpath="/mnt/stage/wangyp/Simulation/new1ton/water/Muon_CJPL/SysUctt/PMTPosition/${subdir}_PreAnalysis"
mkdir -p $outputpath

generate_commands() {
    for i in $(seq 0 9); do
        for j in $(seq 0 100); do
            if [ "$j" -eq 0 ]; then
                filename="${inputpath}/run${i}_muon.root"
            else
                filename="${inputpath}/run${i}_muon_${j}.root"
            fi

            prepath="${outputpath}/run${i}_${j}.root"

            if [ -e "${filename}" ]; then 
                echo "处理文件: $filename"; 
                ./PreAnalysisMC "$filename" "$prepath"; 
            else 
                echo "文件未找到: $filename"; 
            fi
        done
    done
} > $logfile 

# 使用 xargs 并行执行命令
generate_commands | xargs -P $MAX_PROCESSES -I {} sh -c "{}"
