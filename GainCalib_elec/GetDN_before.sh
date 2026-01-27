#!/bin/bash
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks_pre"
# mkdir $peaksfile
DNfile="../output/dn/darknoise_before.root"
GainList="../output/gain/gain_before.txt"
mkdir -p "../output/dn"
./GetDN "$peaksfile" 43636 43700 "${DNfile}"
./GetGain "${DNfile}" "${GainList}"