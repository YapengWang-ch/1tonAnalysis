#include "TChain.h"
#include "TFile.h"
#include <vector>
#include "TVector3.h"
#include <istream>
#include <sstream>  
#include <iostream>
#include <fstream>
#include "TTree.h"
#include "ctools.h"
#include <chrono>

using namespace std;
Int_t ChannelN=60;
const char* PositionFile = "../data/PMTPosition.txt"; // PMT位置文件路径

bool GetPeak(class PeakInfo &peakAdd,int ch,vector<unsigned short>* waveform,float baseline, int TriggerTimeBegin, int TriggerTimeEnd){//
    std::vector<class PeakInfo> peaks=WaveForm_GetPeak(ch,waveform,TriggerTimeBegin,TriggerTimeEnd,baseline,5,30,0.1);
    int pc=peaks.size();
    if (pc){
        if ((peaks[0].risetime<TriggerTimeEnd-TriggerTimeBegin)&&((peaks[0].risetime>0))){//第一个峰
            if (1<pc){
                if (peaks[1].risetime < TriggerTimeEnd-TriggerTimeBegin)
                    return false; //同一个波形内出现两个连续的峰
            }
            peakAdd=peaks[0];
            return true;
        }
    }
    return false; //触发区间内未出现峰
}

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
    
    float x = 0, y = 0, z = 0;
    for (const auto& peak : peaks) {
        if (peak.channel < PMTPosition.size()) {
            x += 1.5*PMTPosition[peak.channel].X();
            y += 1.5*PMTPosition[peak.channel].Y();
            z += 1.5*PMTPosition[peak.channel].Z();
        }
    }
    
    int n = peaks.size();
    ReconPosition.SetXYZ(x / n, y / n, z / n);
    return 0; // 成功
}

int GetDN(const char* filepath, const char* outputpath, std::vector<float> *BaseLine=nullptr, std::vector<unsigned short> *badchannellist=nullptr, int maxdn=-1, float timeinterval=-1){
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);
    vector<unsigned short>* waveform=nullptr;
    Int_t second;
    Int_t nanosecond;
    vector<unsigned short>* ChannelId=nullptr;
    Int_t runNo, TriggerNo;
    Redata->SetBranchAddress("RunNo",&runNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    Redata->SetBranchAddress("Waveform",&waveform);
    Redata->SetBranchAddress("ChannelId",&ChannelId);
    Redata->SetBranchAddress("Sec",&second);
    Redata->SetBranchAddress("NanoSec",&nanosecond);

    int EN=Redata->GetEntries();
    printf("Total entries: %d\n", EN);

    // TFile *
    TTree *DNdata = new TTree("PeakData","PeakData");
    float q;
    int ID;
    double ReconX,ReconY,ReconZ,ReconR;
    DNdata->Branch("RunNo",&runNo);
    DNdata->Branch("TriggerNo",&TriggerNo);
    DNdata->Branch("Sec",&second);
    DNdata->Branch("NanoSec",&nanosecond);
    DNdata->Branch("ReconX",&ReconX);
    DNdata->Branch("ReconY",&ReconY);
    DNdata->Branch("ReconZ",&ReconZ);
    DNdata->Branch("ReconR",&ReconR);
    vector<class PeakInfo> peaks;
    DNdata->Branch("darknoise",&peaks);

    std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
    Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
    
    class PeakInfo peak;
    int FormLength;
    double lastsecond=0;
    float baseline;
        for (int i=0;i<EN;i++){
            if (i % 10000 == 0) {
                printf("Processing entry: %lld / %d / %d\n", DNdata->GetEntries(),i, EN);
            }
            Redata->GetEntry(i);
            FormLength=waveform->size()/ChannelId->size();
            if ((second-lastsecond)*1e9+nanosecond>timeinterval || timeinterval<0){
                for (int j = 0; j < ChannelId->size() ; j++){    
                    ID=(*ChannelId)[j];
                    if (badchannellist != nullptr && !ValidCh(ID, *badchannellist)) { // badchannel cut
                        continue;
                    }
                    // cout << "Processing channel: " << ID << endl;
                    if (BaseLine != nullptr && !BaseLine->empty()){  //baseline input
                        baseline=(*BaseLine)[j]; 
                    }else{
                        if(WaveForm_BaseLine(baseline,waveform,j*FormLength+20,(j+1)*FormLength,80,30,4)) break;  //baseline calculation
                    }
                    // cout << "Baseline: " << baseline << endl;
                    // if (baseline<0){ //baseline cut
                    //     continue;
                    // }
                    vector<class PeakInfo> peak=WaveForm_GetPeak(ID,waveform,j*FormLength,j*FormLength+500,baseline,5,30,0.2,false,false);
                    // cout << "Found " << peak.size() << " peaks in channel " << ID << endl;
                    if (peak.size()<=0) continue;
                    if (peak[0].risetime>50 && peak[0].risetime<400){
                        // cout << "Peak risetime: " << peak[0].risetime << ", peak value: " << peak[0].peakvalue << endl;
                        peaks.push_back(peak[0]);
                    }


                }
                // cout << "Found " << peaks.size() << " peaks in this entry." << endl;
                if ((maxdn<0 || peaks.size()<=maxdn) && !peaks.empty()){ // max darknoise cut
                    // cout << "Processing " << peaks.size() << " peaks for reconstruction." << endl;
                    TVector3 ReconPosition;
                    if (PositionRecon(*PMTPosition,peaks,ReconPosition)==0){
                        ReconX= ReconPosition.X();
                        ReconY= ReconPosition.Y();
                        ReconZ= ReconPosition.Z();
                        ReconR= ReconPosition.Mag();
                        DNdata->Fill();
                    }            
                }
                peaks.clear();
            }
            lastsecond=second+(1e-9)*nanosecond;
        }

    if(DNdata->GetEntries()){ 
        TFile *saveFile=new TFile(outputpath,"RECREATE");
        DNdata->Write();
        printf("Data Wrote, path: %s\n", outputpath);
        saveFile->Close();
        delete saveFile;
    }
    delete Redata;
    delete DNdata;
    return 0;
}

int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;
    std::vector<float> *BaseLine=nullptr;
    // std::vector<unsigned short> badchannellist={5,11,18,26,29,40,51,53,58};
        std::vector<unsigned short> badchannellist={26,29,38,54};

    // std::vector<unsigned short> *BadChannelList = &badchannellist;
    std::vector<unsigned short> *BadChannelList = nullptr;;
    int maxdn=-1;
    float timeinterval=-1;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            maxdn = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc){
            timeinterval = atof(argv[++i]);
        } else if (filepath == nullptr) {
            filepath = argv[i];
        } else if (outputpath == nullptr) {
            outputpath = argv[i];
        }
    }

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> <output file> [-c maxdn] [-t timeinterval]\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GetDN(filepath,outputpath,BaseLine,BadChannelList,maxdn,timeinterval);
    return 0;
}
