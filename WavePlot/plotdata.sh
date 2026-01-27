#!/bin/sh
outputfile="/home/wangyp/1ton/ReConstruction/Output/waveform"
inputfile="/mnt/neutrino/01_RawData/60PMTWater/Phy"

plot_data() {
    local run=$1
    local file=$2
    local triggerno=$3
    local channel=$4
    if [ $file -eq 0 ];then 
        local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%08d.root" ${run} ${run})
    else
        local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%d.root" ${run} ${file})
    fi 
    local output=$(printf "${outputfile}/run%08d_file%d_Trigger%d_%d.pdf" ${run} ${file} ${triggerno} ${channel}) 
    ./plot "${input}" "${output}" --triggerNo ${triggerno} --channel ${channel}
}


plot_split(){
    local run=$1
    local file=$2
    local triggerno=$3
    if [ $file -eq 0 ];then 
        local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%08d.root" ${run} ${run})
    else
        local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%d.root" ${run} ${file})
    fi 
    ./plot "${input}" $(printf "${outputfile}/run%08d_file%d_Trigger%d.pdf" ${run} ${file} ${triggerno})  --triggerNo ${triggerno}
    for ch in {0..59}
    do
        local output=$(printf "${outputfile}/run%08d_file%d_Trigger%d_%02d.pdf" ${run} ${file} ${triggerno} ${ch}) 
        ./plot "${input}" "${output}" --triggerNo ${triggerno} --channel ${ch}
    done
    gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(printf "${outputfile}/run%08d_file%d_Trigger%d_split.pdf" ${run} ${file} ${triggerno})  $(printf "${outputfile}/run%08d_file%d_Trigger%d*.pdf" ${run} ${file} ${triggerno})
    # rm $(printf "${outputfile}/run%08d_file%d_Trigger%d_*.pdf" ${run} ${file} ${triggerno}) 
}

plot_split_mm(){
    local InputDocu=$1
    # local triggerNo=$2
    local triggerno=$2
    local OutoutDocu=$3
    # if [ $file -eq 0 ];then 
    #     local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%08d.root" ${run} ${run})
    # else
    #     local input=$(printf "${inputfile}/run%08d/Jinping_1ton_Phy_*_%d.root" ${run} ${file})
    # fi 
    ./plot "${InputDocu}" $(printf "${OutoutDocu}_Trigger%d.pdf" ${triggerno})  --triggerNo ${triggerno}
    for ch in {0..65}
    do
        local output=$(printf "${OutoutDocu}_Trigger%d_%02d.pdf" ${triggerno} ${ch}) 
        ./plot "${InputDocu}" "${output}" --triggerNo ${triggerno} --channel ${ch}
    done
    gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile=$(printf "${OutoutDocu}_Trigger%d_split.pdf" ${triggerno})  $(printf "${OutoutDocu}_Trigger%d*.pdf" ${triggerno})
    # rm $(printf "${outputfile}/run%08d_file%d_Trigger%d_*.pdf" ${run} ${file} ${triggerno}) 
}

# plot_data 43811 24 1544490
# plot_split 43811 24 1544490
plot_split_mm "/home/changxu/Electronics/20251125_clksync/run1/0.root" 10 "~/1ton/ReConstruction/TimeCali_Laser/waveform_Elec/run0"
# for i in {0..59}43843 93 5973803
# do
#     echo "Processing run: 43777, ch: $i"
#     plot_data 43795 11 735948 $i
# done

# plot_data 43740 2 152347
# plot_data 43749 20 1255275
# plot_data 43753 7 490847
# plot_data 43777 104 6576219
# plot_data 43777 45 2845827
# plot_data 43785 2 128713
# plot_data 43793 53 3390070
# plot_data 43795 11 735948
# plot_data 43809 74 4725949
# plot_data 43823 30 1892242
# plot_data 43843 60 3869478 51
# plot_data 43845 153 9828466
# plot_data 43845 290 18624729
# plot_data 43845 66 4241919

