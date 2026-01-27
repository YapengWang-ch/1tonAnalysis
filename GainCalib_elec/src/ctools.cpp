#include "ctools.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
// #include <string>
#include <unordered_map>

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



std::string ParticleName(int pdgid) {
    static const std::unordered_map<int, std::string> particleMap = {
        // 轻子
        {11, "e-"}, {-11, "e+"},
        {12, "nu_e"}, {-12, "anti_nu_e"},
        {13, "mu-"}, {-13, "mu+"},
        {14, "nu_mu"}, {-14, "anti_nu_mu"},
        {15, "tau-"}, {-15, "tau+"},
        {16, "nu_tau"}, {-16, "anti_nu_tau"},
        
        // 夸克
        {1, "d"}, {-1, "anti_d"},
        {2, "u"}, {-2, "anti_u"},
        {3, "s"}, {-3, "anti_s"},
        {4, "c"}, {-4, "anti_c"},
        {5, "b"}, {-5, "anti_b"},
        {6, "t"}, {-6, "anti_t"},
        
        // 规范玻色子
        {21, "g"},      // 胶子
        {22, "gamma"},  // 光子
        {23, "Z0"},     // Z玻色子
        {24, "W+"},     // W+玻色子
        {-24, "W-"},    // W-玻色子
        
        // 希格斯玻色子
        {25, "H0"},
        
        // 常见介子
        {111, "pi0"},   // π⁰
        {211, "pi+"},   // π⁺
        {-211, "pi-"},  // π⁻
        {113, "rho0"},  // ρ⁰
        {213, "rho+"},  // ρ⁺
        {-213, "rho-"}, // ρ⁻
        {221, "eta"},   // η
        {331, "eta'"},  // η'
        {130, "K_L0"},  // K长寿命
        {310, "K_S0"},  // K短寿命
        {311, "K0"},    // K⁰
        {-311, "anti_K0"}, // 反K⁰
        {321, "K+"},    // K⁺
        {-321, "K-"},   // K⁻
        {411, "D+"},    // D⁺
        {-411, "D-"},   // D⁻
        {421, "D0"},    // D⁰
        {-421, "anti_D0"}, // 反D⁰
        {431, "D_s+"},  // D_s⁺
        {-431, "D_s-"}, // D_s⁻
        {441, "eta_c"}, // η_c
        {443, "J/psi"}, // J/ψ
        
        // 常见重子
        {2112, "n"},    // 中子
        {-2112, "anti_n"}, // 反中子
        {2212, "p"},    // 质子
        {-2212, "anti_p"}, // 反质子
        {2224, "Delta++"}, // Δ⁺⁺
        {-2224, "anti_Delta++"}, // 反Δ⁺⁺
        {3112, "Sigma-"}, // Σ⁻
        {-3112, "anti_Sigma-"}, // 反Σ⁻
        {3222, "Sigma+"}, // Σ⁺
        {-3222, "anti_Sigma+"}, // 反Σ⁺
        {3312, "Xi-"},  // Ξ⁻
        {-3312, "anti_Xi-"}, // 反Ξ⁻
        {3334, "Omega-"}, // Ω⁻
        {-3334, "anti_Omega-"} // 反Ω⁻
    };
    
    auto it = particleMap.find(pdgid);
    if (it != particleMap.end()) {
        return it->second;
    } else {
        return "unknown_" + std::to_string(pdgid);
    }
}

bool ends_with(const char* str, const char* suffix ){
    if (!str || !suffix) return false;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return false;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

char* change_extension(const char* path, const char* new_exten) {
    if (!path) return NULL;
    
    regex_t regex;
    int ret;
    
    // 编译正则表达式：匹配文件扩展名（\.\w*$）
    ret = regcomp(&regex, "\\.\\w*$", REG_EXTENDED);
    if (ret != 0) {
        return strdup(path);
    }
    
    ret = regexec(&regex, path, 0, NULL, 0);
    
    char* result;
    if (ret == 0) {
        regmatch_t match;
        
        regexec(&regex, path, 1, &match, 0);
        
        size_t new_len = match.rm_so; // 匹配开始位置即为扩展名前的位置
        result =  (char*)malloc(new_len + 1); // +1 用于空字符
        
        if (result) {
            strncpy(result, path, new_len);
            result[new_len] = '\0'; // 确保字符串以空字符结尾
        } else {
            result = strdup(path);
        }
    } else if (ret == REG_NOMATCH) {
        result = strdup(path);
    } else {
        char error_msg[100];
        regerror(ret, &regex, error_msg, sizeof(error_msg));
        printf("Error reguler expression matching: %s\n", error_msg);
        result = strdup(path);
    }

    regfree(&regex);
    
    return Form("%s%s",result,new_exten);
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
        if (line[0]=='#') continue;
        std::istringstream iss(line);
        int channel;
        float x, y, z;
        if (!(iss >> channel >> x >> y >> z)) {
            printf("Error: Invalid line format in file %s.\n", filepath);
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

int Read_TimeOffset(const char* filepath, std::vector<double> &TimeOffset, std::vector<double> &TimeUctt, int NChannels){
    std::ifstream file(filepath);
    if (!file.is_open()){
        printf("Error: Cannot open file %s.\n", filepath);
        return 1;
    }
    TimeOffset.clear();
    TimeOffset.resize(NChannels);
    TimeUctt.clear();
    TimeUctt.resize(NChannels);
    string line;
    while (getline(file, line)){
        if (line.empty()) continue; // skip empty lines
        if (line[0]=='#') continue;
        std::istringstream iss(line);
        int channel;
        float time_offset,time_uctt;
        if (!(iss >> channel >> time_offset>>time_uctt)){
            if (!(iss >> channel >> time_offset)){
                printf("Error: Invalid line format in file %s.\n", filepath);
                continue; 
            }// skip invalid lines
            time_uctt=-1;
        }
        if (channel < 0 || channel >= NChannels) {
            printf("Warning: Channel %d out of range [0, %d). Skipping.\n", channel, NChannels);
            continue; // skip out-of-range channels
        }
        TimeOffset[channel]=time_offset;
        TimeUctt[channel]=time_uctt;
    }
    file.close();
    return 0;
}

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
    // cout << "Wave Integral: from "<<risetime<<" to "<<falltime<<endl;
    int i = (int)risetime+1;
    for(i; i< falltime; i++){
        q += baseline - (*waveform)[i];
        // cout <<"iter:"<<i<<" value:"<<baseline - (*waveform)[i]<<" sum:"<<q<<endl;
    }
    if (q<0){
        printf("Warning : Integral less than zero.\n");
        return -3;
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
        if (percentage<0){ // consistant with preAnalysis
            double Ledge=risetime-10;
            if (Ledge<iterbegin) Ledge=iterbegin;
            double Redge=falltime+20;
            if (Redge>iterend) Redge=iterend;
            q = WaveForm_Integral(waveform, peakpositions[i], Ledge, Redge, baseline);
        }else if (Intergral2RFTime){
            q = WaveForm_Integral(waveform, peakpositions[i], risetime, falltime, baseline);
        }else{
            int IntBegin=peakpositions[i]-10;
            if (IntBegin<iterbegin)IntBegin=iterbegin;
            int IntEnd=peakpositions[i]+40;
            if (IntEnd>iterend)IntEnd=iterend;
            q = WaveForm_Integral(waveform, peakpositions[i], IntBegin, IntEnd, baseline);
        }
        // if(q>0)
        peaks.push_back(PeakInfo(ch, peakpositions[i]-iterbegin, baseline-(*waveform)[peakpositions[i]], q, risetime-iterbegin, falltime-iterbegin));
    }
    return peaks;
}

template float Vec_Mean<float>(std::vector<float>&);
template float Vec_Error<float>(std::vector<float>&);