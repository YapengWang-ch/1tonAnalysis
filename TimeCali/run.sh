#!/bin/bash
inputfile="../data/time_calibration.root"
outputpath="../output/peaks.root"
logfile="../Peaks.log"


./getPeaks ${inputfile} ${outputpath} >> ${logfile} 
./TimeCali ${outputpath} >>${logfile}