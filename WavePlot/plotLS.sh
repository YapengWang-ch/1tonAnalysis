#!/bin/sh
outputfile="/home/wangyp/1ton/ReConstruction/WavePlot/output/LS"
inputfile="/mnt/neutrino/01_RawData/60PMTLS/Phy"

# process_run() {
#     i=$1
#     echo "Processing run: $i"
#     ./plot "${inputfile}/run000${i}/*.root" "${outputfile}/run000${i}ch5.pdf" --channel 5
# }

# export -f process_run
# export inputfile
# export outputfile

# seq 43910 45825 | parallel process_run {}
for i in {43910..45825}
do
    echo "Processing run: $i"
    ./plot "${inputfile}/run000${i}/*.root" "${outputfile}/run000${i}ch10.pdf" --channel 10
done
# i=43912
# echo "Processing run: $i"
# ./plot "${inputfile}/run000${i}/*.root" "${outputfile}/run000${i}ch5.pdf" --channel 5

# filename="/mnt/neutrino/01_RawData/60PMTWater/Phy/run00043636/Jinping_1ton_Phy_*_00043636_1.root"
# outputfile="/home/wangyp/1ton/ReConstruction/SimDN/output/darknoise/test.root"
# echo "Processing file: $filename"
# echo "Output file: $outputfile"
# ./getDN "$filename" "$outputfile"
