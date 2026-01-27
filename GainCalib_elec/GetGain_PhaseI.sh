#!/bin/bash
# peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks"
# DNfile="../output/dn/darknoise.root"
DNfile="/home/wangyp/DarkNoise/Water/Phy/output/charge"

GainList="../output/gain_PhaseI/gain_set0_Mean.txt"
mkdir -p "../output/gain_PhaseI"
# mkdir -p "../output/gain_PhaseI"

# ./GetDN "${peaksfile}" 47269 49691 "${DNfile}"
./GetGain_PhaseI "${DNfile}" "${GainList}"
