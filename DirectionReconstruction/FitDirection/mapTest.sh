#!/bin/bash 

MapMuons="../../TemplateGen/MCdata/MapAnalysis/map10*.root"


i="1"
MCFitFile="../Output/FitResultMC/Fittest_k${i}.root"
MCReconDirection="../Output/ReconMC/Recontest_k${i}.root"
    
./FitMCLarge "$MapMuons" "$MCFitFile" "$i"
./GetMCDirection "$MCFitFile" "$MCReconDirection" "$i"
