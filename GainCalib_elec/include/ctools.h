#ifndef MY_CTOOLS_H
#define MY_CTOOLS_H

#include <fstream>
#include <iostream>
#include <vector>
#include "TDatime.h"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <numeric>
#include <queue>
#include <TVector3.h>
#include "TSystem.h" 
using namespace std;

class PeakInfo{
    public:
        PeakInfo() = default; // 添加默认构造函数
        int channel;
        int peakposition;
        // float baseline;
        float peakvalue;
        float charge;
        float risetime;
        float falltime;
        

    PeakInfo(int ch, int pp,  float pv, float q, float rt, float ft){
        channel=ch;
        peakposition=pp;
        // baseline=bl;
        peakvalue=pv;
        charge=q;
        risetime=rt;
        falltime=ft;
    }
    void print() const {
        cout << "Channel: " << channel << ", Peak Position: " << peakposition
             << ", Peak Value: " << peakvalue << ", Charge: " << charge
             << ", Rise Time: " << risetime << ", Fall Time: " << falltime <<endl;
    }
};

class ChannelData{
    public:
        ChannelData() = default; // 添加默认构造函数
        int channel;
        // double baseline;
        double peakLoc;
        double peakHeight;
        double risetime;
        double charge;
        int npeaks;
        double secondpeaktime;
        double secondpeakamp;

    ChannelData(int ch, double pL,double pH, double rt, double q, int np, double spt, double spa){
        channel=ch;
        // baseline=bl;
        peakLoc=pL;
        peakHeight=pH;
        risetime=rt;
        charge=q;
        npeaks=np;
        secondpeaktime=spt;
        secondpeakamp=spa;
    }
};

template <typename T>
bool FileExists(char* FilePath);
template <typename T>
int Vec_Maxiter(std::vector<T> &vec, int iterbegin = 0, int iterend = 0);
template <typename T>
int Vec_Miniter(std::vector<T> &vec, int iterbegin = 0, int iterend = 0);
template <typename T>
float Vec_Sum(std::vector<T> &vec);
template <typename T>
float Vec_Mean(std::vector<T> &vec);
template <typename T>
float Vec_Var(std::vector<T> &vec);
template <typename T>
float Vec_Error(std::vector<T> &vec);
template <typename T>
float Vec_Mean_Var(std::vector<T> &vec);
template <typename T>
float Vec_Mean_Error(std::vector<T> &vec);

string TimePrint(unsigned int time, int UTC = 8);
string ParticleName(int pdgid);
bool ends_with(const char* str, const char* suffix );
char* change_extension(const char* path, const char* new_exten);
int Read_PMTPosition(const char* filepath, std::vector<TVector3> &PMTPosition, int NChannels = 60);
int Read_TimeOffset(const char* filepath, std::vector<double> &TimeOffset, std::vector<double> &TimeUctt, int NChannels=60);
int WaveForm_BaseLine(float &baseline, std::vector<unsigned short>* waveform, int iterbegin, int iterend, int QueueLength = 50, int path = 30, int maxAcceptableElstronicRaising = 6);
std::vector<int> WaveForm_PeakFind(std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold, int spacing = 30);
float WaveForm_PeakFindRiseTime(std::vector<unsigned short>* waveform, int peakposition, int wavebeginiter, float baseline, float percentage);
float WaveForm_PeakFindFallTime(std::vector<unsigned short>* waveform, int peakposition, int waveenditer, float baseline, float percentage);
float WaveForm_Integral(std::vector<unsigned short>* waveform, int peakposition, float risetime, float falltime, float baseline);
std::vector<class PeakInfo> WaveForm_GetPeak(int ch, std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold=5, int spacing = 30, float percentage = 0.1, bool Intergral2RFTime = true, bool debug = false);

// #include "ctools.tpp"

#endif //MY_CTOOLS_H
