#!/bin/bash
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis"
DNfile="../output/dn/darknoise_47270_49200.root"
# GainList="../output/gain/gain_Mean.txt"
scanfile="../output/scan/ch00.root"
mkdir -p "../output/scan"
# ./GetDN_pre "${peaksfile}" 47270 49200 "${DNfile}"
./SERScan "${DNfile}" "${scanfile}"

#LargeCharge found in run 47303,triggerNo175200. ch:13 charge:54.8462
#LargeCharge found in run 47317,triggerNo288900. ch:13 charge:52.7309
#LargeCharge found in run 48494,triggerNo1072686. ch:13 charge:48.7755
#LargeCharge found in run 48484,triggerNo674215. ch:13 charge:56.2308
#LargeCharge found in run 47787,triggerNo66338. ch:13 charge:95.5385