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
    // if (chifo.PeakLoc.size()!=1)return 0; 
    for (int i=0; i<chifo.PeakLoc.size(); i++){
        if (chifo.PeakLoc[i]<10 || (chifo.PeakLoc[i]<150 && chifo.PeakAmp[i]>110)) {
            return 0;
        }
    }
    // if (chifo.PeakLoc[0]>100)return 0;
    // if (chifo.RiseTime<20||chifo.RiseTime>80) return 0;
    charge= chifo.Charge;
    return 1; 
}

int GetDN(const char* inputdir,  const char* outputpath){
    TChain *DNdata = new TChain("SimpleAnalysis","SimpleAnalysis");
    // for (int irun=runbegin; irun<=runend; irun++){
    //     DNdata->Add(Form("%s/run%08d*.root",inputdir,irun));
    // }
    DNdata->Add(inputdir);
    long EN = DNdata->GetEntries();
        // if (EN <= 0) {
        //     cout << "Error: No entries found. Check input files and tree name 'SimpleAnalysis'." << endl;
        //     // 列出已经被添加到链中的文件，便于排查哪个文件为空
        //     TObjArray *files = DNdata->GetListOfFiles();
        //     if (files) {
        //         TIter next(files);
        //         TChainElement *el;
        //         while ((el = (TChainElement*)next())) {
        //             cout << "  added file: " << el->GetTitle() << endl;
        //         }
        //     }
        //     return 0;
        // }
    // float q;
    // int ID;
    // double timeinterval;
    // Int_t runNo,TriggerNo;

    // DNdata->SetBranchAddress("RunNo",&runNo);
    // DNdata->SetBranchAddress("TriggerNo",&TriggerNo);
    // // DNdata->Branch("Sec",&second);
    // // DNdata->Branch("NanoSec",&nanosecond);
    // DNdata->SetBranchAddress("TimeInterval",&timeinterval);
    // // DNdata->Branch("ReconX",&ReconX);
    // // DNdata->Branch("ReconY",&ReconY);
    // // DNdata->Branch("ReconZ",&ReconZ);
    // // DNdata->Branch("ReconR",&ReconR);
    // vector<class PeakInfo> *peaks=nullptr;
    // DNdata->SetBranchAddress("Peaks",&peaks);
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

    TH1F * h[60];
    for(int k=0; k<60; k++){
        h[k]=new TH1F(Form("ch%02d",k),Form("ch%02d",k),250,0,1e4);
    }
    // class PeakInfo peak;
    // int FormLength;
    double lastsecond=0;
    double timeinterval;
    double charge;
    // float baseline;
    // long EN=DNdata->GetEntries();
    if (EN<=0){
        cout << "Error: No entries found"<<endl;
        return 0;
    }
    cout <<"Total "<<EN<<" entries. ";
    for(long i=0; i<EN; i++){
        DNdata->GetEntry(i);
        if (i%(EN/100)==0){
            cout << "processing "<<i<<"/"<<EN<<endl;
        }
        timeinterval=Sec-lastsecond+NanoSec*1e-9;
        lastsecond=Sec+NanoSec*1e-9;

        if (timeinterval<2e-5) continue;// 30us
        if (ChannelInfo==nullptr) {
            cout <<"Warning: nullptr of peaks!"<<endl;
            continue;
        }
        if (ChannelInfo->empty()) continue;

        darknoise.clear();
        for (auto &chifo:*ChannelInfo){
            if (GetCharge(charge,chifo)){
                // darknoise.emplace_back(chifo.ChannelId,charge);
                        h[chifo.ChannelId]->Fill(charge);

            }
        }
        // if (darknoise.size()>2 || darknoise.empty()) continue;  
        // if (darknoise[0].second>90 && darknoise[0].second<110 && darknoise[0].first == 13){
        //     cout << "LargeCharge found in run "<<RunNo<<",triggerNo"<<TriggerNo<<". ch:"<<darknoise[0].first<<" charge:"<<darknoise[0].second<<endl;
        // }
        // h[darknoise[0].first]->Fill(darknoise[0].second);
    }
    cout << "All dn read."<<endl;
    TFile* fout=new TFile(outputpath,"RECREATE");
    fout->cd();
    for (int k=0; k<60 ; k++){
        h[k]->Write();
    }
    fout->Write();
    fout->Close();
    delete fout;
    // delete histograms to avoid ROOT double-free at exit
    for (int k=0; k<60 ; k++){
        delete h[k];
    }
    cout <<"Histograms wrote.";
    delete DNdata;
    return 0;
}

int main(int argc, char** argv){
    const char* filepath = nullptr;
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
    outputpath=argv[2];

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GetDN(filepath, outputpath);
    return 0;
}
