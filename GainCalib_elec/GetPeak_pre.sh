#!/bin/bash
inputfile="/mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/firstpeak_thd100_fix"
fullfile="/mnt/stage/wangyp/PreAnalysis/new1ton/LS2025/Elec/firstpeak_thd30_fullInte"

# inputfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis"

peaksfile="../output/peaks"
DNDir="../output/dn"
GainDir="../output/gain"
FitDir="../output/fitFull"
mkdir -p "$GainDir" "${FitDir}"

# peaksfile="../output/peaks3"
# outputpath="../output_phaseII/WaterCali2w"
# histpath="../output_phaseII/peaks2"
logfile="../log"


./GetPeaks "${fullfile}/run*.root" "${peaksfile}/Elec_peaks_thd30_fullInte_test.root"
# ./GetDN_FullInt "${peaksfile}/Elec_peaks_thd30_fullInte.root" "${DNDir}/darknoise_FullInt_test.root"
# ./GetGain "${DNDir}/darknoise_thd100_reselect.root" "${GainDir}/gain_elec_thd100_reselect.txt" ${FitDir}
# ./GetFullGain "${DNDir}/darknoise_FullInt.root" "${DNDir}/darknoise_thd100_reselect.root" "${GainDir}/gain_elec_FullInt.txt" ${FitDir}

