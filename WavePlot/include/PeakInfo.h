#ifndef PEAKINFO_H
#define PEAKINFO_H

// #include <fstream>
#include <iostream>
#include <vector>
// #include "TDatime.h"
#include <cstdlib>
// #include <math.h>
#include <stdio.h>
#include <numeric>
#include <queue>
// #include <TVector3.h>
// #include "TSystem.h" 
using namespace std;

class PeakInfo{
    public:
        PeakInfo() = default; // 添加默认构造函数
        int channel;
        int baseline;
        int peakposition;
        float peakvalue;
        float charge;
        float risetime;
        float falltime;
    PeakInfo(int ch, int bl, int pp, float pv, float q, float rt, float ft){
        channel=ch;
        baseline=bl;
        peakposition=pp;
        peakvalue=pv;
        charge=q;
        risetime=rt;
        falltime=ft;
    }
    void print() const {
        std::cout << "Channel: " << channel << ", Baseline: " << baseline << ", Peak Position: " << peakposition
             << ", Peak Value: " << peakvalue << ", Charge: " << charge
             << ", Rise Time: " << risetime << ", Fall Time: " << falltime << std::endl;
    }
};

int WaveForm_BaseLine(float& baseline, std::vector<unsigned short>* waveform, int iterbegin, int iterend, int QueueLength = 100, int path = 30, int maxAcceptableElstronicRaising = 6);
std::vector<int> WaveForm_PeakFind(std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold, int spacing = 30);
float WaveForm_PeakFindRiseTime(std::vector<unsigned short>* waveform, int peakposition, int wavebeginiter, float baseline, float percentage);
float WaveForm_PeakFindFallTime(std::vector<unsigned short>* waveform, int peakposition, int waveenditer, float baseline, float percentage);
float WaveForm_Integral(std::vector<unsigned short>* waveform, int peakposition, float risetime, float falltime, float baseline);
std::vector<class PeakInfo> WaveForm_GetPeak(int ch, std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold=5, int spacing = 30, float percentage = 0.1, bool Intergral2RFTime = true, bool debug = false);


#endif // PEAKINFO_H
