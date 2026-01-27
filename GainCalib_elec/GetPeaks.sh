#!/bin/bash
inputfile="/mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/Cs127"
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/Cs137_Peaks_pre"

# peaksfile="../output/peaks3"
# outputpath="../output_phaseII/WaterCali2w"
# histpath="../output_phaseII/peaks2"
logfile="../log"

# rm -rf "$logfile"
mkdir -p "$peaksfile" "$logfile"

rundata() {
    local run=$1
    local runfile=$(printf "${inputfile}/run%d.root" ${run})
    local runlog=$(printf "${logfile}/run%d.log" ${run})
    if [ -e "$runfile" ]; then
        echo "Processing run: ${run}" >&2
        echo "Processing run: ${run}" >> "${runlog}"
        local input=$(printf "${inputfile}/run%d.root" ${run})
        local peaks=$(printf "${peaksfile}/run%d_peaks.root" ${run})
        # rm -f "${peaks}"
        # local output=$(printf "${outputpath}/run%08d_TCali.txt" ${run})
        ./GetPeaks "${input}" "${peaks}" >> "${runlog}"
    else
        echo "Run ${run} does not exist: ${runfile}." 
        return 1
    fi
}

# 导出函数以便在子shell中使用
export -f rundata
export inputfile peaksfile logfile

# 创建必要的目录
mkdir -p "$peaksfile"

# 生成run列表
run_list=()
# for run in {47269..49753}; do
for run in {1..70}; do
    run_list+=($run)
    # rundata $run
done

# 使用xargs并行处理（4线程）
printf "%s\n" "${run_list[@]}" | xargs -P4 -I{} bash -c 'rundata "$@"' _ {}