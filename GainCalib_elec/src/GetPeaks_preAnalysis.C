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
        if (peak.channel < PMTPosition.size() && peak.channel>=0) {
            x += 1.5*peak.charge*PMTPosition[peak.channel].X();
            y += 1.5*peak.charge*PMTPosition[peak.channel].Y();
            z += 1.5*peak.charge*PMTPosition[peak.channel].Z();
            totalcharge+=peak.charge;
        }
    }
    
    int n = peaks.size();
    if (totalcharge<100) totalcharge=100;
    ReconPosition.SetXYZ(x / totalcharge, y / totalcharge, z / totalcharge);
    return 0; // 成功
}


int GetPeaks(const char* filepath, const char* outputpath){
    TChain *Redata = new TChain("SimpleAnalysis","SimpleAnalysis");
    Redata->Add(filepath);
    vector<ChannelInfo_t>* ChannelInfo = nullptr;
    double TotalPE;
    int TriggerNo,RunNo;
    int Sec;
    int NanoSec;
    // data->SetBranchAddress("BadChannelList", &BadChannelList);
    Redata->SetBranchAddress("ChannelInfo", &ChannelInfo);
    // Redata->SetBranchAddress("TotalPE", &TotalPE);
    Redata->SetBranchAddress("TriggerNo", &TriggerNo);
    Redata->SetBranchAddress("RunNo", &RunNo);
    Redata->SetBranchAddress("Sec", &Sec);
    Redata->SetBranchAddress("NanoSec", &NanoSec);

    int EN=Redata->GetEntries();
    printf("Total entries: %d\n", EN);

    TTree *DNdata = new TTree("PeakData","PeakData");
    float q;
    int ID;
    double ReconX,ReconY,ReconZ,ReconR;
    double timeinterval;
    double secondpeaktime,secondpeakamp;

    DNdata->Branch("RunNo",&RunNo);
    DNdata->Branch("TriggerNo",&TriggerNo);
    DNdata->Branch("Sec",&Sec);
    DNdata->Branch("NanoSec",&NanoSec);
    DNdata->Branch("TimeInterval",&timeinterval);
    vector<class ChannelData> peaks;
    DNdata->Branch("Peaks","vector<ChannelData>",&peaks);
    vector<int> peakcount;
    DNdata->Branch("PeakCount",&peakcount);

    std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
    Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
    cout << "PMT Position Read."<<endl;

    class PeakInfo peak;
    int FormLength;
    double lastsecond=0;
    float baseline;
    TFile *saveFile=new TFile(outputpath,"RECREATE");
    if (EN>0){  
        cout << "looping ..."<<endl;
        for (int i=0;i<EN;i++){
            Redata->GetEntry(i);            
            if (i%(EN/100)==0){
                cout << "Processing "<<i<<" / "<<EN<<" ("<<(float)i/EN*100<<" %)"<<endl;
            }
            if (ChannelInfo == nullptr || ChannelInfo->empty() || ChannelInfo->size()>60) {
                std::cerr << "Entry " << i << " ChannelInfo == nullptr, skip\n";
                continue;
            }
            if (i==0){ 
                timeinterval=1;
            }else{
                timeinterval=Sec+(1e-9)*NanoSec-lastsecond;
            }
            for (int j = 0; j < ChannelInfo->size() ; j++){    
                ChannelInfo_t chinfo=(*ChannelInfo)[j];
                ID=chinfo.ChannelId;
                if (chinfo.nPeaks<=0) continue;
                if (chinfo.PeakLoc.size()<=0) continue;
                if (chinfo.ChannelId==3 && chinfo.Charge>6000){
                    cout << "Run "<<RunNo<<" Trigger "<<TriggerNo<<" ch "<<chinfo.ChannelId<<" charge "<<chinfo.Charge<<" baseline "<<chinfo.Pedestal<<" front "<<chinfo.FrontBslnMean<<" back "<<chinfo.BackBslnMean<<endl;
                }
                peakcount.push_back(chinfo.PeakLoc.size());
                peaks.emplace_back(ID,(double)(chinfo.PeakLoc[0]),chinfo.PeakAmp[0],chinfo.RiseTime,chinfo.Charge,chinfo.nPeaks,0,0);
                if (chinfo.PeakLoc.size()>=2){
                    peaks.back().secondpeaktime=chinfo.PeakLoc[1];
                    peaks.back().secondpeakamp=chinfo.PeakAmp[1];
                }
            }
            // }
            lastsecond=Sec+(1e-9)*NanoSec;
            DNdata->Fill();
            peaks.clear();
            peakcount.clear();
        }
    }
    cout << "loop finished."<<endl;
    if(DNdata->GetEntries()){ 
        
        DNdata->Write();
        printf("Data Wrote, path: %s\n", outputpath);
    }
    saveFile->Close();
    delete saveFile;
    delete Redata;
    delete DNdata;
    return 0;
}

// int test(){
//     TChain *Redata = new TChain("Readout","Readout");
//     Redata->Add("/mnt/neutrino/01_RawData/60PMTWater/Phy/run00043688/Jinping_1ton_Phy_*_00043688_9.root");
//     vector<unsigned short>* waveform=nullptr;
//     Int_t second;
//     Int_t nanosecond;
//     vector<unsigned short>* ChannelId=nullptr;
//     Int_t runNo, TriggerNo;
//     Redata->SetBranchAddress("RunNo",&runNo);
//     Redata->SetBranchAddress("TriggerNo",&TriggerNo);
//     Redata->SetBranchAddress("Waveform",&waveform);
//     Redata->SetBranchAddress("ChannelId",&ChannelId);
//     Redata->SetBranchAddress("Sec",&second);
//     Redata->SetBranchAddress("NanoSec",&nanosecond);

//     int EN=Redata->GetEntries();
//     printf("Total entries: %d\n", EN);

//     TTree *DNdata = new TTree("PeakData","PeakData");
//     float q;
//     int ID;
//     double ReconX,ReconY,ReconZ,ReconR;
//     double timeinterval;

//     DNdata->Branch("RunNo",&runNo);
//     DNdata->Branch("TriggerNo",&TriggerNo);
//     DNdata->Branch("Sec",&second);
//     DNdata->Branch("NanoSec",&nanosecond);
//     DNdata->Branch("TimeInterval",&timeinterval);
//     DNdata->Branch("ReconX",&ReconX);
//     DNdata->Branch("ReconY",&ReconY);
//     DNdata->Branch("ReconZ",&ReconZ);
//     DNdata->Branch("ReconR",&ReconR);
//     vector<class PeakInfo> peaks;
//     DNdata->Branch("Peaks","vector<PeakInfo>",&peaks);

//     std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
//     Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
//     cout << "PMT Position Read."<<endl;

//     class PeakInfo peak;
//     int FormLength;
//     double lastsecond=0;
//     float baseline;
//     if (EN>0){  
//         cout << "looping ..."<<endl;
//         // for (int i=0;i<EN;i++){
//             Redata->GetEntry(584308-559945);
//             // if (ChannelId->size()<8) continue;
//             FormLength=waveform->size()/ChannelId->size();
//             // if (i==0){ 
//             //     timeinterval=1e5;
//             // }else{
//             //     timeinterval=second+(1e-9)*nanosecond-lastsecond;
//             // }
//             // if ((second-lastsecond)*1e9+nanosecond>timeinterval || timeinterval<0){
//                 for (int j = 0; j < ChannelId->size() ; j++){    

//                     ID=(*ChannelId)[j];
//                     if (ID!=4) continue;

//                     if(WaveForm_BaseLine(baseline,waveform,j*FormLength+20,(j+1)*FormLength,80,30,3)) {
//                         cout << "baseline unstable for run "<<runNo<<" trigger "<<TriggerNo<<" ch "<<ID<<endl;
//                         break;  //baseline calculation
//                     }
//                     // }
//                         // if (baseline<0){ //baseline cut
//                         //     continue;
//                         // }
//                     vector<class PeakInfo> peak=WaveForm_GetPeak(ID,waveform,j*FormLength,(j+1)*FormLength,baseline,5,30,-1,true,false); // percentage -1 in consistant with preAnalysis.
//                     // cout << "channel "<<ID<<" baseline: "<<baseline<<" " <<peak.size()<<" peaks "<<endl;
//                     if (peak.size()<=0) continue;
//                     peak[0].print();
//                     while(peak.size()>0){
//                         peaks.push_back(peak[0]);
//                         peak.erase(peak.begin());
//                     }
//                 }
//                 // cout << "peaks count: "<<peaks.size() << endl;
//                 // if (peaks.size()<=0) continue; // no peaks found
//                     TVector3 ReconPosition;
//                 // if (PositionRecon(*PMTPosition,peaks,ReconPosition)==0){
//                 //     ReconX= ReconPosition.X();
//                 //     ReconY= ReconPosition.Y();
//                 //     ReconZ= ReconPosition.Z();
//                 //     ReconR= ReconPosition.Mag();
//                 //     DNdata->Fill();
//                 //     // cout << "Peaks fill"<<endl;
//                 // } 
                
//                 peaks.clear();
//             // }
//             // lastsecond=second+(1e-9)*nanosecond;
//         // }
//     }

//     delete Redata;
//     delete DNdata;
//     return 0;
// }

int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;
    // std::vector<float> *BaseLine=nullptr;
    // std::vector<unsigned short> badchannellist={5,11,18,26,29,40,51,53,58};
    // std::vector<unsigned short> badchannellist={26,29,38,54};
    // std::vector<unsigned short> *BadChannelList = &badchannellist;
    // std::vector<unsigned short> *BadChannelList = nullptr;
    // int maxdn=-1;
    // float timeinterval=-1;
    
    for (int i = 1; i < argc; i++) {
        // if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
        //     maxdn = atoi(argv[++i]);
        // } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc){
        //     timeinterval = atof(argv[++i]);
        // } else 
        if (filepath == nullptr) {
            filepath = argv[i];
        } else if (outputpath == nullptr) {
            outputpath = argv[i];
        }
    }

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GetPeaks(filepath,outputpath);
    return 0;
}
