#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <fstream>
#include <vector>
#include <iostream>
#include "ctools.h"

using namespace std;

// vector<int> badchannellist = {2,5,10,11,18,26,29,34,40,42,50,51,53,58};
// // vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
// vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
std::vector<int> badchannellist={26,29,38,54};

std::vector<int> *BadChannelList = &badchannellist;
const char* GaliListFile = "../data/GainList.txt"; // 增益列表文件路径

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}

float Getrmax(vector<PeakInfo> &peaks) {
    float maxcharge = 0;
    float totalcharge = 0;
    for (const auto& peak : peaks) {
        if (peak.charge > maxcharge) {
            maxcharge = peak.charge;
        }
        totalcharge += peak.charge;
    }
    return maxcharge/totalcharge; // 返回最大charge与总charge的比值
}

float deltaTime(vector<PeakInfo> &peaks) {
    if (peaks.size() < 2) {
        return 0; // 如果峰值数量少于2，返回0
    }
    
    float firstTime = 900;
    float secondTime = 900;
    for (const auto& peak : peaks) {
        if (peak.risetime < firstTime) {
            secondTime = firstTime; // 更新第二个时间
            firstTime = peak.risetime; // 更新第一个时间
        } else if (peak.risetime < secondTime) {
            secondTime = peak.risetime; // 更新第二个时间
        }
    }
    return secondTime - firstTime; // 返回最后一个峰与第一个峰的时间差
}

float TotalCharge(vector<PeakInfo> &peaks) {
    float totalCharge = 0;
    for (const auto& peak : peaks) {
        totalCharge += peak.charge;
    }
    return totalCharge; // 返回总电荷
}

void analyze_darknoise(const char* filename, const char* outputpath) {
    // 打开数据文件
    TFile* file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }

    // 获取树
    TTree* tree = (TTree*)file->Get("PeakData");
    if (!tree) {
        cerr << "Error: Cannot find tree 'PeakData'" << endl;
        file->Close();
        return;
    }

    // 设置分支地址
    Int_t Sec, NanoSec;
    Int_t runNo = 0,triggerNo = 0;
    double ReconR;
    vector<PeakInfo>* peaks = nullptr;

    tree->SetBranchAddress("RunNo", &runNo);
    // tree->SetBranchAddress("triggerNo", &triggerNo);
    tree->SetBranchAddress("Sec", &Sec);
    tree->SetBranchAddress("NanoSec", &NanoSec);
    tree->SetBranchAddress("darknoise", &peaks);
    tree->SetBranchAddress("ReconR",&ReconR);

    Int_t timebegin,timeend;
    tree->GetEntry(0);
    timebegin = Sec;
    timeend = Sec;
    Long64_t nEntries = tree->GetEntries();

    vector<float> gain; //read gainlist
    if (Read_GainList(GaliListFile, gain, 60) != 0) {
        cerr << "Error: Cannot read gain list from " << GaliListFile << endl;
        return;
    }

    vector<double> sumTime(60,0.0);
    vector<int> countTime(60,0);
    vector<int> channelcount(60,0);
    for (int refchannel=0; refchannel < 60; refchannel ++){
        if (IsBadChannel(refchannel,BadChannelList)) continue;
        cout <<"processing ch"<<refchannel<<endl;
        vector<TH1D*> histsPerChannel;
        for (int ch = 0; ch < 60; ++ch) {
            histsPerChannel.push_back(new TH1D(Form("hRise_Ch%d", ch), 
                                        Form("Rise Time Channel %d;relative Rise Time (ns);Counts", ch),
                                        400, -50, 50));
        }
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            if (timebegin > Sec) timebegin = Sec; // 更新开始时间
            if (timeend < Sec) timeend = Sec; // 更新结束时间
            if (peaks == nullptr || peaks->empty()) continue; // 跳过空的峰值数据
            // if (ReconR>0.3) continue;
            if (peaks->size() >50 || peaks->size() < 4) continue; // 跳过高能触发事例和低能触发事例
            float totalCharge = TotalCharge(*peaks);
            if (totalCharge > 100000) continue; // 跳过高能事例
            // if (deltaTime(*peaks) > 10) continue; //  
            if (Getrmax(*peaks) > 0.3) continue; // 跳过最大电荷与总电荷比值大于0.3的事

            double risetime_0=-1;
            for (size_t j = 0; j < peaks->size(); j++) {
                int channel = peaks->at(j).channel;
                if ((channel==refchannel)&& (peaks->at(j).charge<2000)&& (peaks->at(j).charge>50)){
                    risetime_0 = peaks->at(j).risetime;
                    break;
                }
            }
            if (risetime_0 < 0) continue;
            channelcount[refchannel]++;

            for (size_t j = 0; j < peaks->size(); j++) {
                int channel = peaks->at(j).channel;
                if (IsBadChannel(channel, BadChannelList)) {
                    continue; // 跳过坏通道
                }
                if ((peaks->at(j).charge<1000)&& (peaks->at(j).charge>50)) continue; 
                double risetime = peaks->at(j).risetime;
                
                if (channel > refchannel && channel < 60) {
                    // hRisetime->Fill(channel, risetime-risetime_0);
                    histsPerChannel[channel]->Fill(risetime-risetime_0);
                }
            }
        }

        for (int ch = refchannel + 1; ch < 60; ++ch) {
            if (IsBadChannel(ch, BadChannelList)) continue;
            TH1D* hist = histsPerChannel[ch];
            
            double Mean = hist->GetMean();
            double lowBound = Mean - 30.0;
            double highBound = Mean + 30.0;
            
            // 计算指定范围内的加权平均值
            double sumX = 0.0;
            double sumW = 0.0;
            int binLow = hist->FindBin(lowBound);
            int binHigh = hist->FindBin(highBound);
            
            for (int bin = binLow; bin <= binHigh; bin++) {
                double binCenter = hist->GetBinCenter(bin);
                double binContent = hist->GetBinContent(bin);
                sumX += binCenter * binContent;
                sumW += binContent;
            }
            
            double mean = (sumW > 0) ? sumX / sumW : Mean; // 防止除零错误
            
            // 更新时间和计数
            sumTime[refchannel] -= mean;
            sumTime[ch] += mean;
            countTime[refchannel]++;
            countTime[ch]++;
        }
        for (int ch = 0; ch < 60; ++ch) {
            delete histsPerChannel[ch];
        }
    }

        // 创建文本文件输出
    ofstream txtFile(outputpath);
    if (!txtFile.is_open()) {
        cerr << "Error: Cannot open text file for writing" << endl;
        return;
    }
    
    // 写入文件头
    txtFile << "# run" <<runNo <<" beginning time:"<< timebegin << " end time:" << timeend << endl;
    txtFile << "# Channel\tMean (ns)\tMean Error (ns)\tSigma (ns)\tSigma Error (ns)\tEntries\n";
    txtFile << "# -----------------------------------------------------------------------\n";    

            // 写入文本文件
    for (int ch=0; ch<60 ;ch++){
            txtFile << ch << "\t"
                    << sumTime[ch]/(countTime[ch]+1) << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << 0 << "\t"
                    << channelcount[ch] << "\n";
    }
    
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input_file.root> <output_file.txt>" << endl;
        return 1;
    }
    
    analyze_darknoise(argv[1], argv[2]);
    return 0;
}