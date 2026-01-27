#include "PeakInfo.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <regex.h>
// #include <string>
// #include <unordered_map>

int WaveForm_BaseLine(float &baseline,std::vector<unsigned short>* waveform, int iterbegin, int iterend, int QueueLength, int path, int maxAcceptableElectronicRaising){
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
    int intlength=100;
    if (intlength>(iterend-iterbegin)) intlength=iterend-iterbegin;
    float badbaseline = std::accumulate(waveform->begin() + iterbegin, waveform->begin() + iterbegin+intlength, 0.0) / intlength;
    std::queue<int> cform = form;
    do {
        // 波形平移
        for (i = 0; i < minp + path; i++){
            if (tailp + i + iterbegin >= iterend){
                // printf("Warning : Stable Baseline not found, 1 returned.\n");
                baseline=badbaseline;
                return 1;
            }
            form.pop();
            form.push((*waveform)[iterbegin + tailp + i]); 
        }
        tailp += i;
        // 计算局部波形的最小值与均值
        cform = form;
        min = 1000;
        queuep = 0;
        minp = 0;
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
        mean = (float)sum / QueueLength;
    } while (mean - min > maxAcceptableElectronicRaising);
    baseline=mean;
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
                // cout <<"higher peak "<<baseline - (*waveform)[k]<<" instead the last."<<endl;
                peaks.pop_back();
                peaks.push_back(k);
                spacecount = spacing;
            } else {
                spacecount--;
            }
        } else {
            // cout << "searching peak at "<<k<<" value "<<baseline - (*waveform)[k]<<" for threhold "<<threhold<<endl;
            if (baseline - (*waveform)[k] > threhold){ // waveform over threshold
                if (iterbegin < k - spacing){
                    sbegin = k - spacing;
                } else sbegin = iterbegin;
                vmax = (*waveform)[sbegin];
                vmin = (*waveform)[sbegin];
                // cout << "searching in last "<<spacing<<" ns"<<endl;
                for (int i = sbegin + 1; i < k; i++){
                    if (vmax < (*waveform)[i]){
                        vmax = (*waveform)[i];
                    }
                    if (vmin > (*waveform)[i]){
                        vmin = (*waveform)[i];
                    }
                }
                if (vmax - threhold*0.8 > (*waveform)[k] && vmin > (*waveform)[k]){ // waveform decline at least 80% of threshold in the spacing, no highest subpeak in the spacing
                    // cout <<"peak found at "<<k<<endl;
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

float WaveForm_Integral(std::vector<unsigned short>* waveform, int peakposition, float risetime, float falltime, float baseline){
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
        q = WaveForm_Integral(waveform, peakpositions[i], risetime, falltime, baseline);
        else{
            int IntBegin=peakpositions[i]-10;
            if (IntBegin<iterbegin)IntBegin=iterbegin;
            int IntEnd=peakpositions[i]+40;
            if (IntEnd>iterend)IntEnd=iterend;
            q = WaveForm_Integral(waveform, peakpositions[i], IntBegin, IntEnd, baseline);
        }
        // if(q>0)
        peaks.push_back(PeakInfo(ch, baseline,peakpositions[i]-iterbegin, baseline-(*waveform)[peakpositions[i]], q, risetime-iterbegin, falltime-iterbegin));
    }
    return peaks;
}
