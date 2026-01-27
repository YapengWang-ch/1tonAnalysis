#!/bin/bash
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks"
# DNfile="../output/dn/darknoise.root"
DNfile="../output/dn/darknoise.root"

GainList="../output/gain/gain_ser.txt"
mkdir -p "../output/dn"
./GetDN "${peaksfile}" 47269 47269 "${DNfile}"
# ./GetGain "${DNfile}" "${GainList}"
