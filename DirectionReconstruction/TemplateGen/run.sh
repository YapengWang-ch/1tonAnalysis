#!/bin/sh

# PreAnalysis script for MC data 
logfile="./mapGenerator.log"
if [ -e $logfile ]; then
    rm $logfile
fi
inputpath="/mnt/stage/wangyp/Simulation/new1ton/waterII/templates/PreAnalysis_MapMuon"
outputpath="MCdata/MapAnalysis_MapMuon"
mkdir -p $outputpath

for i in `seq 114 149`
do  
    for j in `seq 0 10`
    do 
        # fillename="MCdata/RawData/map${i}_muon*.root"
        # echo "processing file: $fillename"
        prepath="${inputpath}/run${i}_${j}.root"
        outputfile="${outputpath}/map${i}_${j}.root"
        # ./../../preAnalysis/build/PreAnalysisMC "$fillename" $prepath
        if [ ! -e "$prepath" ]; then
            echo "file not found: $prepath"
            continue
        fi
        echo "Processing file ${prepath}"
        ./build/mapAnalysis "$prepath" $outputfile
    done
done >> $logfile

./build/MapGenLarge_60Ch  >> $logfile

# logfile="./mapGenerator.log"

# for i in `seq 100 149`
# do  
#     for j in `seq 0 30`
#     do 
#         # fillename="MCdata/RawData/map${i}_muon*.root"
#         # echo "processing file: $fillename"
#         prepath="/mnt/stage/wangyp/Simulation/new1ton/water/templates/PreAnalysis_TruQE_r1400_ABS_sub20/run${i}_${j}.root"
#         outputpath="MCdata/MapAnalysis_TruQE_ABS_sub20/map${i}_${j}.root"
#         # ./../../preAnalysis/build/PreAnalysisMC "$fillename" $prepath
#         if [ ! -e "$prepath" ]; then
#             echo "file not found: $prepath"
#             continue
#         fi
#         echo "Processing file ${prepath}"
#         ./build/mapAnalysis "$prepath" $outputpath
#     done
# done >> $logfile

# ./build/MapGenLarge_60Ch  >> $logfile


