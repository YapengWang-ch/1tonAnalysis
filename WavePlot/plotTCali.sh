#!/bin/sh
outputfile="/home/wangyp/1ton/ReConstruction/TimeCali/data/time_calibration.root"
inputfile="/home/wangyp/1ton/ReConstruction/TimeCali/output"

triggerno="638"

./plot "${outputfile}" "${inputfile}/event${triggerno}.pdf" --triggerNo ${triggerno} 



# plot_data 43740 2 152347
# plot_data 43749 20 1255275
# plot_data 43753 7 490847
# plot_data 43777 104 6576219
# plot_data 43777 45 2845827
# plot_data 43785 2 128713
# plot_data 43793 53 3390070
# plot_data 43795 11 735948
# plot_data 43809 74 4725949
# plot_data 43823 30 1892242
# plot_data 43843 60 3869478 51
# plot_data 43845 153 9828466
# plot_data 43845 290 18624729
# plot_data 43845 66 4241919

