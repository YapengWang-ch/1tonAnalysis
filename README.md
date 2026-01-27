## 1tonAnalysis --wangyp
The analysis program used for JNE 1ton prototype. See more in JSAP.

### WavePlot
A simple waveform plot and event display program, the input document format should be standard Readout format for JNE. 

    ./plot inputfile outputfile --triggerNo tn --ch channel 
    # plot waveform for certain channel and triggerNo
    # if channel not assigned, plot waveform for certain triggerNo with all channels baseline aligned

    ./plotMC inputfile outputfile --triggerNo tn --ch ch
    # same as ./plot, but for JPSim output format. 
    # because the data type of branch "ChannelId" & "WaveForm" in standard Readout file is vector<unsigned short>, while the format of same branches in JPSim output dacument is vector<unsigned int>

### GainCalib
...
### preAnalysis
...