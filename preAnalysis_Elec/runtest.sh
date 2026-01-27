#!/bin/bash

workdir="/mnt/stage/wangyp/Simulation/new1ton/water/MuonTest"
inputfile1="${workdir}/cone0_300GeV"
inputfile2="${workdir}/cone75_300GeV"
outputfile1="${workdir}/cone0_300GeV_PreAnalysis"
outputfile2="${workdir}/cone75_300GeV_PreAnalysis"

# mkdir -p ${outputfile1}
# mkdir -p ${outputfile2}

# ./PreAnalysisMC "${inputfile1}/muon.root" "${outputfile1}/muon_0.root" &
# ./PreAnalysisMC "${inputfile2}/muon.root" "${outputfile2}/muon_0.root" 

# for i in {1..3}
# do 
#     ./PreAnalysisMC "${inputfile1}/muon_${i}.root" "${outputfile1}/muon_${i}.root" &
#     ./PreAnalysisMC "${inputfile2}/muon_${i}.root" "${outputfile2}/muon_${i}.root" 
# done
i=3
./PreAnalysisMC "${inputfile1}/muon_${i}.root" "${outputfile1}/muon_${i}.root" &
./PreAnalysisMC "${inputfile2}/muon_${i}.root" "${outputfile2}/muon_${i}.root" 
wait
echo "all task done"