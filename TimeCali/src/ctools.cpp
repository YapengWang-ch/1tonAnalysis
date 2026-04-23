#include "ctools.h"
#include <istream>
#include <sstream>  
#include <iostream>
#include <fstream>

bool FileExists(char* FilePath){
    std::ifstream file(FilePath);
    return file.good();
}

template <typename T>
float Vec_Mean(std::vector<T> &vec){
    return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

template <typename T>
int Vec_Maxiter(std::vector<T> &vec, int iterbegin, int iterend){
    if (iterend == 0){
        iterend = vec.size();
    }
    if ((vec.size() < iterend) || (iterend < iterbegin)){
        printf("Warning : Vec_Max : no element in vector.\n");
        return 0;
    }
    T max = vec[iterbegin];
    int iter = 0;
    for (size_t i = iterbegin + 1; i < iterend; i++){
        if (max < vec[i]){
            max = vec[i];
            iter = i;
        }
    }
    return iter;
}

template <typename T>
int Vec_Miniter(std::vector<T> &vec, int iterbegin, int iterend){
    if (iterend == 0){
        iterend = vec.size();
    }
    if ((vec.size() < iterend) || (iterend < iterbegin)){
        printf("Warning : Vec_Min : no element in vector.\n");
        return 0;
    }
    T min = vec[iterbegin];
    int iter = 0;
    for (size_t i = iterbegin + 1; i < iterend; i++){
        if (min > vec[i]){
            min = vec[i];
            iter = i;
        }
    }
    return iter;
}

template <typename T>
float Vec_Sum(std::vector<T> &vec){
    return std::accumulate(vec.begin(), vec.end(), 0.0);
}

template <typename T>
float Vec_Var(std::vector<T> &vec){
    if (vec.size() < 2){
        printf("Error:Vec_Var, vector length less than 2\n");
        return 0;
    }
    float mean = Vec_Mean(vec);
    float sumvar = 0;
    for (int i = 0; i < vec.size(); i++){
        sumvar += (vec[i] - mean) * (vec[i] - mean);
    }
    return sumvar / (vec.size() - 1);
}

template <typename T>
float Vec_Error(std::vector<T> &vec){
    return sqrt(Vec_Var(vec));
}

template <typename T>
float Vec_Mean_Var(std::vector<T> &vec){
    if (vec.size() < 2){
        printf("Error:Vec_Mean_Var, vector length less than 2\n");
        return 0;
    }
    return Vec_Var(vec) / (vec.size());
}

template <typename T>
float Vec_Mean_Error(std::vector<T> &vec){
    return sqrt(Vec_Mean_Var(vec));
}

string TimePrint(unsigned int time, int UTC){
    TDatime Time(time + UTC * 3600);
    return Time.AsString();
}

int Read_PMTPosition(const char* filepath, std::vector<TVector3> &PMTPosition, int NChannels){
    std::ifstream file(filepath);
    if (!file.is_open()){
        printf("Error: Cannot open file %s.\n", filepath);
        return 1;
    }
    PMTPosition.clear();
    PMTPosition.resize(NChannels);
    string line;
    while (getline(file, line)){
        if (line.empty()) continue; // skip empty lines
        std::istringstream iss(line);
        int channel;
        float x, y, z;
        if (!(iss >> channel >> x >> y >> z)) {
            printf("warning: Invalid line format in file %s.\n", filepath);
            continue; // skip invalid lines
        }
        if (channel < 0 || channel >= NChannels) {
            printf("Warning: Channel %d out of range [0, %d). Skipping.\n", channel, NChannels);
            
            continue; // skip out-of-range channels
        }
        PMTPosition[channel] = TVector3(x, y, z);
    }
    file.close();
    return 0;
}

int Read_GainList(const char* filepath, std::vector<float> &GainList, int NChannels){
    std::ifstream file(filepath);
    if (!file.is_open()){
        printf("Error: Cannot open file %s.\n", filepath);
        return 1;
    }
    GainList.clear();
    GainList.resize(NChannels);
    string line;
    while (getline(file, line)){
        if (line.empty()) continue; // skip empty lines
        std::istringstream iss(line);
        int channel;
        float gain;
        if (!(iss >> channel >> gain)) {
            printf("Warning: Invalid line format in file %s.\n", filepath);
            continue; // skip invalid lines
        }
        if (channel < 0 || channel >= NChannels) {
            printf("Warning: Channel %d out of range [0, %d). Skipping.\n", channel, NChannels);
            continue; // skip out-of-range channels
        }
        GainList[channel] = gain;
    }
    file.close();
    return 0;
}

int WaveForm_BaseLine(float &baseline, std::vector<unsigned short>* waveform, int iterbegin, int iterend, int QueueLength, int path, int maxAcceptableElstronicRaising){
    std::queue<int> form;
    for (int i = iterbegin; i < iterbegin + QueueLength; i++){
        form.push((*waveform)[i]);
    }
    int k = 0;
    int sum = 0;
    float mean = 0;
    int min = 1000;
    int minp = -path;
    int tailp = QueueLength;
    int queuep = 0;
    int i = 0;
    float badbaseline = std::accumulate(waveform->begin() + iterbegin, waveform->begin() + iterend, 0.0) / (iterend - iterbegin);
    std::queue<int> cform = form;
    do {
        // 波形平移
        for (i = 0; i < minp + path; i++){
            if (tailp + i + iterbegin >= iterend){
                printf("Warning : Stable Baseline not found, -1 returned.\n");
                baseline = badbaseline;
                return 1;
            }
            form.pop();
            form.push((*waveform)[iterbegin + tailp + i]); 
        }
        tailp += i; // 标记队尾在波形中位置
        // 计算局部波形的最小值与均值
        cform = form;
        min = 1000;
        queuep = 0; // 标记队列当前位置
        minp = 0; //标记峰在队列中的位置
        sum = 0;
        while (!cform.empty()){
            sum += cform.front();
            queuep += 1;
            if (min >= cform.front()){
                min = cform.front();  
                minp = queuep;
            }
            cform.pop();
        }
        mean = (float) sum / QueueLength;
    } while (mean - min > maxAcceptableElstronicRaising);
    baseline = mean;
    return 0;        
}

std::vector<int> WaveForm_PeakFind(std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold, int spacing){
    std::vector<int> peaks;
    int spacecount = 0; // mark whether the position is out of the spacing
    int sbegin = iterbegin; // start position of the spacing
    int vmax = 0; // maximum value in the spacing
    int vmin = 0; // minimum value in the spacing
    for (int k = iterbegin; k < iterend; k++){
        if (spacecount > 0){
            if ((*waveform)[k] < (*waveform)[peaks.back()]){
                peaks.pop_back();
                peaks.push_back(k);
                spacecount = spacing;
            } else {
                spacecount--;
            }
        } else {
            if (baseline - (*waveform)[k] > threhold){ // waveform over threshold
                if (iterbegin < k - spacing){
                    sbegin = k - spacing;
                } else sbegin = iterbegin;
                vmax = (*waveform)[sbegin];
                vmin = (*waveform)[sbegin];
                for (int i = sbegin + 1; i < k; i++){
                    if (vmax < (*waveform)[i]){
                        vmax = (*waveform)[i];
                    }
                    if (vmin > (*waveform)[i]){
                        vmin = (*waveform)[i];
                    }
                }
                if (vmax - threhold*0.8 > (*waveform)[k] && vmin < (*waveform)[k]){ // waveform decline at least 80% of threshold in the spacing, no highest subpeak in the spacing
                    peaks.push_back(k);
                    spacecount = spacing;
                }
            }
        }
    }
    return peaks;
}  

float WaveForm_PeakFindRiseTime(std::vector<unsigned short>* waveform, int peakposition, int wavebeginiter, float baseline, float percentage){
    if(peakposition<wavebeginiter){
        printf("Warning : Peak position less than waveform begin iter.\n");
        return peakposition;
    }
    // printf("peakposition: %d, wavebeginiter: %d, percentage: %.2f\n", peakposition, wavebeginiter,percentage);
    int risetime = peakposition;
    int pvalue = (*waveform)[risetime];    
    if(pvalue>baseline){
        printf("Warning : Peak value higher than baseline.\n");
        return peakposition;
    }
    float rtthreshold = percentage * pvalue + (1 - percentage) * baseline;
    while ((*waveform)[risetime] < rtthreshold && risetime>wavebeginiter){
        risetime--;
    }

    float frisetime = risetime;
    if (risetime == wavebeginiter){
        // printf("Warning : Peak rise time not found, return waveform begin iter.\n");
        return risetime; // no rise time found
    }
    frisetime += ((*waveform)[risetime] - rtthreshold) / ((*waveform)[risetime] - (*waveform)[risetime + 1]);
    return frisetime;
}  

float WaveForm_PeakFindFallTime(std::vector<unsigned short>* waveform, int peakposition, int waveenditer, float baseline, float percentage){
    if(peakposition>waveenditer){
        printf("Warning : Peak position bigger than waveform end iter.\n");
        return peakposition;
    }
    // printf("peakposition: %d, wavebeginiter: %d, percentage: %.2f\n", peakposition, wavebeginiter,percentage);
    int falltime = peakposition;
    int pvalue = (*waveform)[falltime];    
    if(pvalue>baseline){
        printf("Warning : Peak value higher than baseline.\n");
        return peakposition;
    }
    float rtthreshold = percentage * pvalue + (1 - percentage) * baseline;
    while ((*waveform)[falltime] < rtthreshold && falltime<waveenditer){
        falltime++;
    }

    float ffalltime = falltime;
    if (falltime == waveenditer){
        // printf("Warning : Peak rise time not found, return waveform begin iter.\n");
        return falltime; // no rise time found
    }
    ffalltime -= ((*waveform)[falltime] - rtthreshold) / ((*waveform)[falltime] - (*waveform)[falltime - 1]);
    return ffalltime;
} 

float WaveForm_Integral(std::vector<unsigned short>* waveform, int peakposition, float risetime, float falltime, float baseline, float percentage){
    if (peakposition < risetime || peakposition > falltime){
        printf("Warning : Peak not in the integral area.\n");
        return -2; // peak out of limits
    }
    float q = 0;
    int i = (int)risetime+1;
    for(i; i< falltime; i++){
        // if ((*waveform)[i] >  baseline){
        //     cout << "Warning : Waveform value higher than baseline, integral stopped." << endl;
        //     return 1;
        // }
        q += baseline - (*waveform)[i];
    }
    return q;
}

std::vector<class PeakInfo> WaveForm_GetPeak(int ch, std::vector<unsigned short>* waveform, int iterbegin, int iterend, float baseline, float threhold, int spacing, float percentage, bool Intergral2RFTime, bool debug){
    std::vector<class PeakInfo> peaks;
    std::vector<int> peakpositions = WaveForm_PeakFind(waveform, iterbegin, iterend, baseline, threhold, spacing);
    for (int i = 0; i < peakpositions.size(); i++){
        float risetime = WaveForm_PeakFindRiseTime(waveform, peakpositions[i], iterbegin, baseline, percentage);
        float falltime = WaveForm_PeakFindFallTime(waveform, peakpositions[i], iterend, baseline, percentage);
        float q=0;
        if (Intergral2RFTime)
        q = WaveForm_Integral(waveform, peakpositions[i], risetime, falltime, baseline, percentage);
        else
        q = WaveForm_Integral(waveform, peakpositions[i], peakpositions[i]-50, peakpositions[i]+150, baseline, percentage);
        // if(q>0)
        peaks.push_back(PeakInfo(ch, peakpositions[i]-iterbegin, baseline-(*waveform)[peakpositions[i]], q, risetime-iterbegin, falltime-iterbegin));
    }
    return peaks;
}
