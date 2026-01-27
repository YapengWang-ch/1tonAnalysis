// #include "TChain.h"
// #include "TFile.h"
// #include <vector>
#include "TVector3.h"
// #include "ctools.h"
// // #include <chrono>
#include "TChain.h"
#include "TFile.h"
#include "TMath.h"
#include "TH1F.h"
#include "TCanvas.h"
#include <iostream>
#include <vector>
#include <ctime>
#include "TLegend.h"
#include "TStyle.h"
#include "ctools.h"
#include "TROOT.h"
#include "ChannelInfo.h"
#include "TGraph.h"

using namespace std;
Int_t ChannelN=60;
const char* PositionFile = "../data/PMTPosition.txt"; // PMT位置文件路径

// bool GetPeak(class PeakInfo &peakAdd,int ch,vector<unsigned short>* waveform,float baseline, int TriggerTimeBegin, int TriggerTimeEnd){//
//     std::vector<class PeakInfo> peaks=WaveForm_GetPeak(ch,waveform,TriggerTimeBegin,TriggerTimeEnd,baseline,5,30,0.1);
//     int pc=peaks.size();
//     if (pc){
//         if ((peaks[0].risetime<TriggerTimeEnd-TriggerTimeBegin)&&((peaks[0].risetime>0))){//第一个峰
//             if (1<pc){
//                 if (peaks[1].risetime < TriggerTimeEnd-TriggerTimeBegin)
//                     return false; //同一个波形内出现两个连续的峰
//             }
//             peakAdd=peaks[0];
//             return true;
//         }
//     }
//     return false; //触发区间内未出现峰
// }

bool ValidCh(Int_t ch, std::vector<unsigned short> badchannellist){
    if(badchannellist.empty()){
        return true;
    }
    for(unsigned short i:badchannellist){
        if(ch==i){
            return false;
        }
    }
    return true;
}

int PositionRecon(std::vector<TVector3> &PMTPosition, std::vector<PeakInfo> &peaks, TVector3 &ReconPosition){
    if (peaks.size() < 2) {
        return 1; // 至少需要两个峰值进行位置重建
    }
    
    float x = 0, y = 0, z = 0, totalcharge=0;
    for (const auto& peak : peaks) {
        if (peak.channel < PMTPosition.size()) {
            x += 1.5*peak.charge*PMTPosition[peak.channel].X();
            y += 1.5*peak.charge*PMTPosition[peak.channel].Y();
            z += 1.5*peak.charge*PMTPosition[peak.channel].Z();
            totalcharge+=peak.charge;
        }
    }
    
    int n = peaks.size();
    ReconPosition.SetXYZ(x / totalcharge, y / totalcharge, z / totalcharge);
    return 0; // 成功
}

int GetCharge(double &charge, ChannelInfo_t chifo){
    if (chifo.PeakLoc.size()!=1)return 0; 
    if (chifo.PeakLoc[0]>100)return 0;
    if (chifo.RiseTime<20||chifo.RiseTime>80) return 0;
    charge= chifo.Charge;
    return 1; 
}
int GetMeanGain(double &gain, const char* inputdir, int run, Int_t &Second){
    TChain *DNdata = new TChain("SimpleAnalysis","SimpleAnalysis");
    DNdata->Add(Form("%s/run%08d*.root",inputdir,run));
    if (DNdata->GetEntries()<=0) return 0;
    // float q;
    // int ID;
    double timeinterval=0;
    vector<ChannelInfo_t>* ChannelInfo = nullptr;
    double TotalPE;
    int TriggerNo,RunNo;
    int Sec;
    int NanoSec;
    // data->SetBranchAddress("BadChannelList", &BadChannelList);
    DNdata->SetBranchAddress("ChannelInfo", &ChannelInfo);
    // DNdata->SetBranchAddress("TotalPE", &TotalPE);
    DNdata->SetBranchAddress("TriggerNo", &TriggerNo);
    DNdata->SetBranchAddress("RunNo", &RunNo);
    DNdata->SetBranchAddress("Sec", &Sec);
    DNdata->SetBranchAddress("NanoSec", &NanoSec);

    // std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
    // Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
    // cout << "PMT Position Read."<<endl;

    vector<pair<int,double>> darknoise; 

    TH1F* h=new TH1F("ch13","ch13",500,0,1500);

    // class PeakInfo peak;
    // int FormLength;
    // double lastsecond=0;
    // float baseline;
    double charge,lastsecond;
    long EN=DNdata->GetEntries();
    if (EN<=0){
        cout << "Error: No entries found"<<endl;
        return 0;
    }
    DNdata->GetEntry(0);
    Second=Sec;
    cout <<"Total "<<EN<<" entries. ";
    for(long i=0; i<EN; i++){
        DNdata->GetEntry(i);
        // if (i%(EN/100)==0){
        //     cout << "processing "<<i<<"/"<<EN<<endl;
        // }
        timeinterval=Sec-lastsecond+NanoSec*1e-9;
        lastsecond=Sec+NanoSec*1e-9;

        if (timeinterval<3e-5) continue;// 30us
        if (ChannelInfo==nullptr) {
            cout <<"Warning: nullptr of peaks!"<<endl;
            continue;
        }
        if (ChannelInfo->empty()) continue;

        darknoise.clear();
        for (auto &chifo:*ChannelInfo){
            if (GetCharge(charge,chifo)){
                darknoise.emplace_back(chifo.ChannelId,charge);
            }
        }
        if (darknoise.size()>2 || darknoise.empty()) continue;  
        // if (darknoise[0].second>40 && darknoise[0].second<60 && darknoise[0].first == 13){
        //     cout << "LargeCharge found in run "<<RunNo<<",triggerNo"<<TriggerNo<<". ch:"<<darknoise[0].first<<" charge:"<<darknoise[0].second<<endl;
        // }
        if (darknoise[0].first!=13) continue;
        h->Fill(darknoise[0].second);
    }
    if (h->GetEntries()<=10) return 0;
    gain = h->GetMean();
    delete h;
    // delete DNdata;
    return 1;
}

int GetEvolution(const char* inputdir,int runbegin, int runend,  const char* outputpath){
    TGraph* g=new TGraph();
    g->Set(runend-runbegin);
    Int_t second;
    double gain;
    int ig=0;
    for(int run=runbegin; run<runend; run++){
        cout <<"processing run"<<run<<endl;
        if (GetMeanGain(gain,inputdir, run, second)){
            g->SetPoint(ig,run,gain);
            ig++;
        }
    }
    g->Set(ig);
    g->GetXaxis()->SetTitle("runNo");
    g->GetYaxis()->SetTitle("meanCharge");
    g->SaveAs(outputpath);
    return 0;
}

int main(int argc, char** argv){
    const char* filepath = nullptr;
    int runbegin,runend;
    const char* outputpath = nullptr;


    // for (int i = 1; i < argc; i++) {
    //     // if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
    //     //     maxdn = atoi(argv[++i]);
    //     // } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc){
    //     //     timeinterval = atof(argv[++i]);
    //     // } else 
    //     if (filepath == nullptr) {
    //         filepath = argv[i];
    //     } else if (outputpath == nullptr) {
    //         outputpath = argv[i];
    //     }
    // }
    filepath=argv[1];
    runbegin=atoi(argv[2]);
    runend=atoi(argv[3]);
    outputpath=argv[4];

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> runbegin runend <output file>\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GetEvolution(filepath,runbegin, runend, outputpath);
    return 0;
}
