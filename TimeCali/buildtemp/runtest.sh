#!/bin/bash
inputfile="/mnt/stage/wangyp/Simulation/new1ton/water/CaliTest/All/*.root"
peaksfile="/mnt/stage/wangyp/Simulation/new1ton/water/CaliTest/PreAnalysis_All/peaks.root"
mkdir -p "/mnt/stage/wangyp/Simulation/new1ton/water/CaliTest/PreAnalysis_All"
outputpath="../output/Test/All.txt"
histpath="../output/Test/All_PER.root"
# logfile="../log"

./getPeaksMC_ReconR "${inputfile}" "${histpath}" -t 10000 
# ./TimeCali2 "${peaksfile}" "${outputpath}" "${histpath}"
# rm -rf "$logfile"
# mkdir -p "$peaksfile" "$outputpath" "$histpath" "$logfile"

# rundata() {
#     local run=$1
#     local runfile=$(printf "${inputfile}/run%08d" ${run})
#     local runlog=$(printf "${logfile}/run%08d.log" ${run})
#     if [ -d "$runfile" ]; then
#         echo "Processing run: ${run}" >&2
#         echo "Processing run: ${run}" >> "${runlog}"
#         local input=$(printf "${inputfile}/run%08d/*.root" ${run})
#         local peaks=$(printf "${peaksfile}/run%08d_peaks.root" ${run})
#         rm -f "${peaks}"
#         local output=$(printf "${outputpath}/run%08d_TCali.txt" ${run})
#         ./getPeaks "${input}" "${peaks}" -t 10000 >> "${runlog}"
#         if [ -e "${peaks}" ]; then
#             ./TimeCali "${peaks}" "${output}" "${histpath}/run${run}.root"  >> "${runlog}"
#             echo "Run ${run} completed." 
#         else
#             echo "No valid entries in  run ${run}." 
#             return 1
#         fi

#     else
#         echo "Run ${run} does not exist." 
#         return 1
#     fi
# }

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