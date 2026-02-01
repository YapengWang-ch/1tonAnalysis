#!/bin/bash 

MAX_JOBS=4

MCMuons="../Output/MCMuons/MCRun_0*.root"
MCFitFile="../Output/FitResultMC/FitResultMC_k100_proj.root"
LogFile="../ProjectionRecon.log"
rm -f $LogFile
# ./FitMCLargeProj "$MCMuons" "$MCFitFile" 100 >> $LogFile
for i in {20..50}
do
    MCReconDirection="../Output/ReconMC/ProjReconMC_k30_D${i}.root"
    echo "processing k=${i}"
    ./GetMCDirectionProj "$MCFitFile" "$MCReconDirection" "$i" 80
done >> $LogFile
