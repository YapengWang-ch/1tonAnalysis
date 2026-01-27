#!/bin/bash
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis"
DNfile="../output/GainEvo47500_48000.root"
GainList="../output/gain/gain_pre2.txt"
mkdir -p "../output/dn"
./GetDNEvo "${peaksfile}" 47500 48000 "${DNfile}"
# ./GetGain "${DNfile}" "${GainList}"
