#!/bin/bash
# events count for low 

inputfile="/mnt/neutrino/01_RawData/60PMTWater/Phy/"
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks2"
# peaksfile="../output/peaks3"
outputpath="../output/WaterCali2w_high"
histpath="../output_phaseII/peaks2/run48248_ch0.root"
logfile="../log"

# rm -rf "$logfile"
mkdir -p "$peaksfile" "$outputpath" "$histpath" "$logfile"


peaks="${peaksfile}/run00048248_peaks.root"

output="${outputpath}/WaterCali_temp.txt"

./TimeCali2 "${peaks}" "${output}" "${histpath}">>"${logfile}/run48248_0.log"


# # 导出函数以便在子shell中使用
# export -f rundata
# export inputfile peaksfile outputpath histpath logfile

# # 创建必要的目录
# mkdir -p "$peaksfile" "$outputpath" "$histpath"

# # 生成run列表
# run_list=()
# for run in {43636..43847}; do
#     run_list+=($run)
# done

# # 使用xargs并行处理（4线程）
# printf "%s\n" "${run_list[@]}" | xargs -P4 -I{} bash -c 'rundata "$@"' _ {}