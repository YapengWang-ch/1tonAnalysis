#!/bin/sh

# for i in {257..290}
# do
#     bsub -o ../Preoutput/log/data${i}.log ./PreAnalysisData ${i} ../Preoutput/PreAnalysis_${i}.root
# done

# for i in {1..20}
# do
# 	bsub -o ../Preoutput/log/Muon${i}.log ./PreAnalysisData\ \"/home/jinping/WORK/guozy/JinpingPackage/JSAP-install/Simulation/output/MuonE/Muon${i}_E.root\" ../Preoutput/MC/Muon/Muon${i}.root
# done


#!/bin/bash

#!/bin/bash

input_file=/home/wangyp/1ton/GainCali/Water/ReConstruction/preAnalysis/GoodFileList.txt

# 设置LD_LIBRARY_PATH
# export LD_LIBRARY_PATH=/home/wangyp/1ton/GainCali/Water/ReConstruction/preAnalysis/lib:$LD_LIBRARY_PATH

# 检查文件是否存在
if [ ! -f "$input_file" ]; then
    echo "Error: FileList not found!"
    exit 1
fi

# export LD_LIBRARY_PATH=/home/wangyp/1ton/GainCali/Water/ReConstruction/preAnalysis/lib:$LD_LIBRARY_PATH

# 逐行读取文件
while IFS= read -r line; do
    # 跳过空行和注释行（以#开头的行）
    if [[ -z "$line" || "$line" =~ ^# ]]; then
        continue
    fi

    # 提取两列整数
    col1=$(echo "$line" | awk '{print $1}')
    col2=$(echo "$line" | awk '{print $2}')

    # 检查是否成功提取到两列整数
    if [[ -z "$col1" || -z "$col2" ]]; then
        echo "Error: Invalid line format: '$line'"
        continue
    fi

    if ["$col1" < 43668]; then 
        continue
    fi 
    # 组合两列整数为字符串
    if [ "$col2" -eq 0 ]; then
        filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d.root" "$col1" "$col1")
    else
        filename=$(printf "/mnt/neutrino/01_RawData/60PMTWater/Phy/run%08d/Jinping_1ton_Phy_*_%08d_%d.root" "$col1" "$col1" "$col2")
    fi
    outputfile=$(printf "/home/wangyp/1ton/ReConstruction/Output/run%08d_%d.root" "$col1" "$col2")
    
    echo "Processing file: $filename"
    echo "Output file: $outputfile"
    # 调用另一个命令，将组合后的字符串作为参数传递
    ./PreAnalysisData "$filename" "$outputfile" "-b" "${col1}" "${col2}"

done < "$input_file"
