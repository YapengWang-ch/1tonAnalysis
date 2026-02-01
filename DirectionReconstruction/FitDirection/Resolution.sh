#!/bin/bash
mkdir -p ../OutputNew/Resolution


# subdir="WCT_T6_sigma0.5"

# mkdir -p ../OutputNew/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../OutputNew/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done

# ./plot_k_Resolution "../OutputNew/AngularResolution/${subdir}" "../OutputNew/AngularResolution/k_Resolution_${subdir}.root"

# subdir="CT_sigma0.3_b"

# mkdir -p ../OutputNew/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../OutputNew/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done

# ./plot_k_Resolution "../OutputNew/AngularResolution/${subdir}" "../OutputNew/AngularResolution/k_Resolution_${subdir}.root"
# subdir="ABS_sub20_WCT_T6"

# mkdir -p ../OutputNew/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../OutputNew/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done

# ./plot_k_Resolution "../OutputNew/AngularResolution/${subdir}" "../OutputNew/AngularResolution/k_Resolution_${subdir}.root"

# subdir="ABS_sub20_CT"

# mkdir -p ../OutputNew/AngularResolution/${subdir}
# for i in {1..100}
# do
#     MCReconDirection="../OutputNew/ReconMC/${subdir}/*_k${i}.root"
#     AngularResolutionFile="../OutputNew/AngularResolution/${subdir}/AngularResolution_k${i}.root"
#     ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
# done

# ./plot_k_Resolution "../OutputNew/AngularResolution/${subdir}" "../OutputNew/AngularResolution/k_Resolution_${subdir}.root"

subdir="ABS_sub20_AA"

mkdir -p ../OutputNew/AngularResolution/${subdir}
for i in {1..100}
do
    MCReconDirection="../OutputNew/ReconMC/${subdir}/*_k${i}.root"
    AngularResolutionFile="../OutputNew/AngularResolution/${subdir}/AngularResolution_k${i}.root"
    ./AngularResolution $MCReconDirection $AngularResolutionFile ${i}
done

./plot_k_Resolution "../OutputNew/AngularResolution/${subdir}" "../OutputNew/AngularResolution/k_Resolution_${subdir}.root"
