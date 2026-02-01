
#!/bin/bash 

MAX_JOBS=4
mkdir -p ../Output/ReconMC
# MCMuons="../Output/MCMuons_Templates/MCRun_*.root"
MCMuons="../Output/MC_test/MCRun_*.root"
templates="../../TemplateGen/templates/MapMuon.root"

mkdir -p ../Output/FitResultMC
mkdir -p ../Output/FitResultMC/MC_test_d645

# process_k() {
#     local i=$1
#     MCReconDirection="../Output/ReconMC/${subdir}/ReconMC_k${i}.root"
#     # MCReconDirection="../Output/ReconMC/ReconMC_NoTSigma_k${i}.root"
    
#     echo "processing file ${MCReconDirection}"
#     ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
# }

# export -f process_k
# export MCMuons
# export MCFitFile
# export subdir


# subdir="Test_CT"
# MCFitFile="../Output/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 1 ns
# echo "MCFitFile: $MCFitFile"
# # ./FitMC_AA "$MCMuons" "$MCFitFile" "${templates}" 100 
# mkdir -p ../Output/ReconMC/${subdir}

# for i in {1..100}
# do 
#     MCReconDirection="../Output/ReconMC/${subdir}/ReconMC_k${i}.root"
#     echo "processing file ${MCReconDirection}"
#     ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
# done 
# mkdir -p ../Output/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../Output/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../Output/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done
# ./plot_k_Resolution "../Output/AngularResolution/${subdir}" "../Output/AngularResolution/k_Resolution_${subdir}.root"

subdir="D645_WCT_t4.5"
MCFitFile="../Output/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 1 ns
echo "MCFitFile: $MCFitFile"
./FitMCLarge "$MCMuons" "$MCFitFile" "${templates}" 100 4
mkdir -p ../Output/ReconMC/${subdir}

for i in {1..100}
do 
    MCReconDirection="../Output/ReconMC/${subdir}/ReconMC_k${i}.root"
    echo "processing file ${MCReconDirection}"
    ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
done 
mkdir -p ../Output/AngularResolution/${subdir}
for i in {1..100}
do
    MCReconDirection="../Output/ReconMC/${subdir}/*_k${i}.root"
    AngularResolutionFile="../Output/AngularResolution/${subdir}/AngularResolution_k${i}.root"
    ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
done
./plot_k_Resolution "../Output/AngularResolution/${subdir}" "../Output/AngularResolution/k_Resolution_${subdir}.root"


# subdir="Test_WCT_t6"
# MCFitFile="../Output/FitResultMC/FitResultMC_${subdir}_k100.root"  # the uncertainty of the TCali is 1 ns
# echo "MCFitFile: $MCFitFile"
# # ./FitMCLarge "$MCMuons" "$MCFitFile" "${templates}" 100 4.5
# mkdir -p ../Output/ReconMC/${subdir}

# for i in {1..100}
# do 
#     MCReconDirection="../Output/ReconMC/${subdir}/ReconMC_k${i}.root"
#     echo "processing file ${MCReconDirection}"
#     ./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
# done 
# mkdir -p ../Output/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../Output/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../Output/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done
# ./plot_k_Resolution "../Output/AngularResolution/${subdir}" "../Output/AngularResolution/k_Resolution_${subdir}.root"


# seq 1 100 | parallel -j $MAX_JOBS --progress --joblog joblog.txt \
#     --eta --noswap "process_k {}"
# echo "all task finished"
