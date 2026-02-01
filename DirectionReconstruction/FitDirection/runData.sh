#!/bin/bash 
PreAnalysisDataDirectory="/mnt/stage/wangyp/PreAnalysis/new1ton/water/PreAnalysis" 
# mkdir -p ../Output
# mkdir -p ../Output/Muons
mkdir -p ../Output/FitResultData
mkdir -p ../Output/ReconData
templates="../../TemplateGen/templates/MapMuon.root"
Muons="../Output/Muons/Muons_PMT56.root"

./PreAnalysisData $PreAnalysisDataDirectory $Muons

subdir="WCT_t6"
FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
./FitDataLarge $Muons $FitMuons $templates "TimeCalibYes" 6
./GetDataDirection $FitMuons $ReconMuons 20

subdir="WCT_t5"
FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
./FitDataLarge $Muons $FitMuons $templates "TimeCalibYes" 5
./GetDataDirection $FitMuons $ReconMuons 20

subdir="WCT_t4.5"
FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
./FitDataLarge $Muons $FitMuons $templates "TimeCalibYes" 4.5
./GetDataDirection $FitMuons $ReconMuons 20

subdir="WCT_t4"
FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
./FitDataLarge $Muons $FitMuons $templates "TimeCalibYes" 4
./GetDataDirection $FitMuons $ReconMuons 20

# subdir="AA"
# FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
# ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
# ./FitData_AA $Muons $FitMuons $templates "TimeCalibYes"
# ./GetDataDirection $FitMuons $ReconMuons 20

# subdir="CT"
# FitMuons="../Output/FitResultData/FitMuonsData_${subdir}.root"
# ReconMuons="../Output/ReconData/ReconMuonsData_${subdir}.root"
# ./FitData_CT $Muons $FitMuons $templates "TimeCalibYes"
# ./GetDataDirection $FitMuons $ReconMuons 20

# # # ./FitDataLarge 
# ./FitMCLarge
# ./GetDataDirection
# ./GetMCDirection``
