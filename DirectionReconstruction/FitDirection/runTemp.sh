#!/bin/bash 

MAX_JOBS=4

MCMuons="../../TemplateGen/MCdata/MapAnalysis/newPMT/map10_0.root"
MCFitFile="../Output/FitResultMC/FitResultMC_Temp_k100.root"

./FitMCLarge "$MCMuons" "$MCFitFile" 100