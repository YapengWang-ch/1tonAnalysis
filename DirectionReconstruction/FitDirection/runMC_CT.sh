
#!/bin/bash 

MAX_JOBS=4

subdir="CT"
MCMuons="../OutputNew/MCMuons/MCRun_*.root"
# MCFitFile="../Output/FitResultMC/FitResultMC_NoTSigma_k100.root"
MCFitFile="../OutputNew/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 0.1 ns
echo "MCFitFile: $MCFitFile"
templates="../../TemplateGen/templates/MuonMap_Water_WCT_TruQE.root"

./FitMC_CT "$MCMuons" "$MCFitFile" "${templates}" 100 1

mkdir -p ../OutputNew/ReconMC/${subdir}

process_k() {
    local i=$1
    MCReconDirection="../OutputNew/ReconMC/${subdir}/ReconMC_k${i}.root"
    # MCReconDirection="../Output/ReconMC/ReconMC_NoTSigma_k${i}.root"
    
    echo "processing file ${MCReconDirection}"
    ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
}

export -f process_k
export MCMuons
export MCFitFile
export subdir

seq 1 100 | parallel -j $MAX_JOBS --progress --joblog joblog.txt \
    --eta --noswap "process_k {}"

echo "all task finished"
