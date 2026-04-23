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
#include "TH1D.h"
#include "Utils/JPUtils.h"
#include "JPSimOutput.hh"


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


int GetDN(const char* filepath, const char* outputpath, std::vector<float> *BaseLine=nullptr, std::vector<unsigned short> *badchannellist=nullptr, int maxdn=-1, float timeinterval=-1){
    TChain *Redata = new TChain("Readout","Readout");
    TChain *tmc = new TChain("SimTriggerInfo");
    Redata->Add(filepath);
    tmc->Add(filepath);

    vector<unsigned int>* waveform=nullptr;
    Int_t second;
    Int_t nanosecond;
    vector<unsigned int>* ChannelId=nullptr;
    Int_t runNo, TriggerNo;
    Redata->SetBranchAddress("RunNo",&runNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    Redata->SetBranchAddress("Waveform",&waveform);
    Redata->SetBranchAddress("ChannelId",&ChannelId);
    Redata->SetBranchAddress("Sec",&second);
    Redata->SetBranchAddress("NanoSec",&nanosecond);

    std::vector<JPSimTruthTree_t> *truthList = new std::vector<JPSimTruthTree_t>;
    std::vector<JPSimPE_t> *PEList = new std::vector<JPSimPE_t>;
    Int_t TriggerTruth;
    // tmc->SetBranchAddress("RunNo", &RunNo);
    cout << "set tmc branch address" << endl;
    tmc->SetBranchAddress("TriggerNo", &TriggerTruth);
    tmc->SetBranchAddress("PEList", &PEList);
    tmc->SetBranchAddress("truthList", &truthList);

    int EN=Redata->GetEntries();
    printf("Total entries: %d\n", EN);
    int ENMC=tmc->GetEntries();

    TTree *DNdata = new TTree("PeakData","PeakData");
    float q;
    int ID;
    double ReconX,ReconY,ReconZ,ReconR;
    double TruthX,TruthY,TruthZ,TruthR;
    double PEX,PEY,PEZ,PER;

    DNdata->Branch("RunNo",&runNo);
    DNdata->Branch("TriggerNo",&TriggerNo);
    DNdata->Branch("Sec",&second);
    DNdata->Branch("NanoSec",&nanosecond);
    DNdata->Branch("ReconX",&ReconX);
    DNdata->Branch("ReconY",&ReconY);
    DNdata->Branch("ReconZ",&ReconZ);
    DNdata->Branch("ReconR",&ReconR);
    DNdata->Branch("TruthX",&TruthX);
    DNdata->Branch("TruthY",&TruthY);
    DNdata->Branch("TruthZ",&TruthZ);
    DNdata->Branch("TruthR",&TruthR);
    DNdata->Branch("PEX",&PEX);
    DNdata->Branch("PEY",&PEY);
    DNdata->Branch("PEZ",&PEZ);
    DNdata->Branch("PER",&PER);
    vector<class PeakInfo> peaks;
    DNdata->Branch("darknoise",&peaks);

    TH1D* hevent=new TH1D("hevent","hevent",200,0,1.5);
    TH1D* hevent_r = new TH1D("hevent_r","hevent_r",200,0,1.5);

    std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
    Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
    // cout <<"0"<<endl;
    class PeakInfo peak;
    int FormLength=900;
    double lastsecond=0;
    float baseline;
    
    int itc=0,itmc=0;
    if (EN>0){  
        while (itc<EN && itmc<ENMC)
        {   
            if (itc%(EN/1000)==0){
                cout << "processing entry "<<itc <<"/"<<EN<<endl;
            }
            Redata->GetEntry(itc);
            tmc->GetEntry(itmc);
            itc++;
            itmc++;
            if (TriggerNo != TriggerTruth){
                cout << "The trigger number is not matched!" << endl;
                cout << "MC truth TriggerNo: " << TriggerTruth << endl;
                cout << "Waveform TriggerNo: " << TriggerNo << endl;
                cout << endl;
                itc++;
                itmc++;
                if (TriggerNo > TriggerTruth){
                    itc--;
                    continue;
                }else{
                    itmc--;
                    continue;
                }
            }
            if (PEList==nullptr || PEList->size()==0) continue;
            PEX=0;
            PEY=0;
            PEZ=0;
            for(JPSimPE_t SimPE: *PEList){
                PEX+=SimPE.photonX*0.001;
                PEY+=SimPE.photonY*0.001;
                PEZ+=SimPE.photonZ*0.001;
            }
            PEX/=PEList->size();
            PEY/=PEList->size();
            PEZ/=PEList->size();
            PER=TMath::Sqrt(PEX*PEX+PEY*PEY+PEZ*PEZ);

            if (truthList==nullptr || truthList->size()==0) continue;
            TruthX=truthList->at(0).x*0.001;
            TruthY=truthList->at(0).y*0.001;
            TruthZ=truthList->at(0).z*0.001;
            TruthR=TMath::Sqrt(TruthX*TruthX+TruthY*TruthY+TruthZ*TruthZ);
            
            std::vector<unsigned short> *waveform_short = new std::vector<unsigned short>;
            for (unsigned int value:(*waveform)){
                waveform_short->push_back((unsigned short)value);
            }
            std::vector<unsigned short> *ChannelId_short = new std::vector<unsigned short>;
            for (unsigned int value:(*ChannelId)){
                ChannelId_short->push_back((unsigned short)value);
            }
            // cout <<"1"<<endl;
            if (ChannelId_short->size()<8) continue;
            if ((second-lastsecond)*1e9+nanosecond>timeinterval || timeinterval<0){
                for (int j = 0; j < ChannelId_short->size() ; j++){    
                    ID=(*ChannelId_short)[j];
                    if (badchannellist != nullptr && !ValidCh(ID, *badchannellist)) { // badchannel cut
                        // printf("Channel %d is invalid\n", ID);
                        continue;
                    }
                    if (BaseLine != nullptr && !BaseLine->empty()){  //baseline input
                        baseline=(*BaseLine)[j]; 
                    }else{
                        if(WaveForm_BaseLine(baseline,waveform_short,j*FormLength+20,(j+1)*FormLength,80,30,5)) {
                            cout << "baseline unstable for run "<<runNo<<" trigger "<<TriggerNo<<" ch "<<ID<<endl;
                            // for (int kk=(j)*FormLength+20;kk<(j+2)*FormLength;kk++  ){
                            //     cout << " k:" <<kk<<" value:"<<(*waveform)[kk];
                            // }
                            // if (TriggerNo>=4151) return 0;
                        break;  //baseline calculation
                        }
                    }
                    if (baseline<0){ //baseline cut
                        continue;
                    }
                    vector<class PeakInfo> peak=WaveForm_GetPeak(ID,waveform_short,j*FormLength,j*FormLength+500,baseline,5,30,0.2,false,false);
                    if (peak.size()<=0) continue;
                    if (peak[0].risetime>50 && peak[0].risetime<400){
                        peaks.push_back(peak[0]);
                    }

                }
                if ((maxdn<0 || peaks.size()<=maxdn) && !peaks.empty()){ // max darknoise cut
                    TVector3 ReconPosition;
                    if (PositionRecon(*PMTPosition,peaks,ReconPosition)==0){
                        ReconX= ReconPosition.X();
                        ReconY= ReconPosition.Y();
                        ReconZ= ReconPosition.Z();
                        ReconR= ReconPosition.Mag();
                        hevent->Fill(TruthR);
                        if (ReconR<0.3) hevent_r->Fill(TruthR);
                        DNdata->Fill();
                        if (TruthX > 1){
                            for(int kkk=0; kkk<4; kkk++)DNdata->Fill();
                        }
                    } 
                }
                delete ChannelId_short;
                delete waveform_short;
                peaks.clear();
            }
            lastsecond=second+(1e-9)*nanosecond;
        }
    }

    if(DNdata->GetEntries()){ 
        TFile *saveFile=new TFile(outputpath,"RECREATE");
        DNdata->Write();
        hevent->Write();
        hevent_r->Write();
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
    std::vector<unsigned short> badchannellist={5,11,18,26,29,40,51,53,58};
    // std::vector<unsigned short> *BadChannelList = &badchannellist;
    std::vector<unsigned short> *BadChannelList = nullptr;
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
