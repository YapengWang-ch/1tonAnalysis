#!/bin/bash
#!/bin/bash
inputfile="/mnt/neutrino/01_RawData/60PMTWater/Phy/"
peaksfile="/mnt/stage/wangyp/PreAnalysis/new1ton/water/peaks2"
# peaksfile="../output/peaks3"
outputpath="../output/WaterTQmapW4_10"
logfile="../log"

rm -rf "$logfile"
mkdir -p  "$outputpath"  "$logfile"

./TQmapW "${peaksfile}/*.root" "${outputpath}/TQmapAll.root" 0 4 -r 0.10 0.15 >> "${logfile}/TQmap.log"

./TWfit "${outputpath}/TQmapAll.root" "${outputpath}/TWfit.txt" >> "${logfile}/TWfit.log"
