#ifndef MY_TOOLS_H
#define MY_TOOLS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <regex.h>
#include <vector>
#include "TDatime.h"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <numeric>
#include <queue>
#include <TVector3.h>
#include "TSystem.h" 
#include <unordered_map>

using namespace std;

bool FileExists(char* FilePath){
    std::ifstream file(FilePath);
    return file.good();
}

template <typename T>
float Vec_Mean(std::vector<T> &vec , int iterbegin=0, int iterend=0){
    if (iterend == 0){
        iterend = vec.size();
    }
    if ((vec.size() < iterend) || (iterend < iterbegin)){
        printf("Warning : Vec_Mean : no element in vector.\n");
        return 0;
    }
    float sum = 0;
    for (size_t i = iterbegin; i < iterend; i++){
        sum += vec[i];
    }
    return sum / (iterend - iterbegin);
}

template <typename T>
int Vec_MaxIter(std::vector<T> &vec, int iterbegin=0, int iterend=0){
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
int Vec_MinIter(std::vector<T> &vec, int iterbegin=0, int iterend=0){
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

string TimePrint(unsigned int time, int UTC=8){
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

int Read_PMTPosition(const char* filepath, std::vector<TVector3> &PMTPosition, int NChannels=60){
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

int Read_TimeOffset(const char* filepath, std::vector<double> &TimeOffset, std::vector<double> &TimeUctt, int NChannels=60){
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

// #include "ctools.tpp"

#endif //MY_TOOLS_H
