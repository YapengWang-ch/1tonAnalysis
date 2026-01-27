#!/bin/bash
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis"
DNfile="../output/dn/darknoise_47270_49200.root"
GainList="../output/gain/gain_ser_7peak_0.4peak.txt"
mkdir -p "../output/dn"
# ./GetDN_pre "${peaksfile}" 47570 48000 "${DNfile}"
./GetGain "${DNfile}" "${GainList}"

#LargeCharge found in run 47303,triggerNo175200. ch:13 charge:54.8462
#LargeCharge found in run 47317,triggerNo288900. ch:13 charge:52.7309
#LargeCharge found in run 48494,triggerNo1072686. ch:13 charge:48.7755
#LargeCharge found in run 48484,triggerNo674215. ch:13 charge:56.2308
#LargeCharge found in run 47691,triggerNo128513. ch:13 charge:93.3385
#LargeCharge found in run 47959,triggerNo12408. ch:13 charge:94.6154