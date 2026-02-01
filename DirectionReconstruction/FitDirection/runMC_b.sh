
#!/bin/bash 

MAX_JOBS=4
mkdir -p ../Output/ReconMC
MCMuons="../Output/MCMuons/MCRun_*.root"
# MCMuons="../OutputNew/MCMuons_Templates/MCRun_*.root"
templates="../../TemplateGen/templates/MuonMap_Water_new.root"
mkdir -p ../Output/FitResultMC
mkdir -p ../Output/FitResultMC/PDE_withRock_WCT

# process_k() {
#     local i=$1
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/ReconMC_k${i}.root"
#     # MCReconDirection="../Output/ReconMC/ReconMC_NoTSigma_k${i}.root"
    
#     echo "processing file ${MCReconDirection}"
#     ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
# }

# export -f process_k
# export MCMuons
# export MCFitFile
# export subdir


# subdir="CT_sigma0.3_b"
# logfile="../runMC_CT.log"
# MCFitFile="../OutputNew/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 1 ns
# echo "MCFitFile: $MCFitFile"
# ./FitMC_CT "$MCMuons" "$MCFitFile" "${templates}" 100 >> $logfile
# mkdir -p ../OutputNew/ReconMC/${subdir}

# for i in {1..100}
# do 
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/ReconMC_k${i}.root"
#     echo "processing file ${MCReconDirection}"
#     ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
# done 

subdir="WCT_T6_sigma0.3_Test"
MCFitFile="../OutputNew/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 1 ns
echo "MCFitFile: $MCFitFile"
./FitMCLarge "$MCMuons" "$MCFitFile" "${templates}" 100 6
mkdir -p ../OutputNew/ReconMC/${subdir}

for i in {1..100}
do 
    MCReconDirection="../OutputNew/ReconMC/${subdir}/ReconMC_k${i}.root"
    echo "processing file ${MCReconDirection}"
    ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
done 


# seq 1 100 | parallel -j $MAX_JOBS --progress --joblog joblog.txt \
#     --eta --noswap "process_k {}"
# echo "all task finished"
