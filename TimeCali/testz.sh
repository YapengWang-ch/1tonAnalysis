#!/bin/bash
inputfile="/mnt/neutrino/01_RawData/60PMTWater/Phy/"
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks"
# peaksfile="../output/peaks3"
outputpath="../output/WaterCali_iter_test"
histpath="/mnt/stage/wangyp/PreAnalysis/new1ton/water/TCaliTree"
logfile="../log"

rm -rf "$logfile"
mkdir -p "$peaksfile" "$outputpath" "$histpath" "$logfile"

rundata() {
    local run=$1
    local runfile=$(printf "${inputfile}/run%08d" ${run})
    local runlog=$(printf "${logfile}/run%08d.log" ${run})
    if [ -d "$runfile" ]; then
        echo "Processing run: ${run}" >&2
        echo "Processing run: ${run}" >> "${runlog}"
        local input=$(printf "${inputfile}/run%08d/*.root" ${run})
        local peaks=$(printf "${peaksfile}/run%08d_peaks.root" ${run})
        # rm -f "${peaks}"
        local output=$(printf "${outputpath}/run%08d_TCali.txt" ${run})
        # ./getPeaks_z "${input}" "${peaks}" -t 10000 
        if [ -e "${peaks}" ]; then
            ./TimeCali_z "${peaks}" "${output}" "${histpath}/run${run}.root"  >> "${runlog}"
            echo "Run ${run} completed." 
        else
            echo "No valid entries in run ${run}." 
            return 1
        fi

    else
        echo "Run ${run} does not exist." 
        return 1
    fi
}

# 导出函数以便在子shell中使用
export -f rundata
export inputfile peaksfile outputpath histpath logfile

# 创建必要的目录
mkdir -p "$peaksfile" "$outputpath" "$histpath"

# 生成run列表
# run_list=()
# for run in {43636..43847}; do
#     run_list+=($run)
# done

# # 使用xargs并行处理（4线程）
# printf "%s\n" "${run_list[@]}" | xargs -P4 -I{} bash -c 'rundata "$@"' _ {}

rundata 43845