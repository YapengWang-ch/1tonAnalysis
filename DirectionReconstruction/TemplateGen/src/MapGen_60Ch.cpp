// calculate timevector & Energy vector from preanalysis data 
// tree "MuonMap" output 
// 60 PMTs
// Input file is fixed as ../MCdata/MapAnalysis/map%d*.root
// output file fixed as ../template/MuonMap_Water.root
#include <iostream>
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TFile.h"
#include "TSystem.h"
#include <vector>
#include <numeric>
#include "TRandom3.h"

using namespace std;

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++) if (badchannellist->at(i)==PMTId) return true;
    return false;
}

int main(int argv, char** ar)
{  
    TChain *t = new TChain("ma");
    for (Int_t i = 100; i <= 149; i++)
    {
        // t->Add(TString::Format("./MCdata/MapAnalysis_TruQE/map%d*.root", i));
        t->Add(TString::Format("./MCdata/MapAnalysis_MapMuon/map%d*.root", i));
    }

    Int_t nEntries = t->GetEntries();
    cout << "input: nEntries= " << nEntries << endl;

    int RunNo, FileNo, TriggerNo, nChannels;
    double TotalPE, PEmax2Sum, TimeRange;
    double cosTheta, Phi, cosAlpha, Beta;
    int ChannelIdList[60];
    double Time[60], Energy[60];
    double tError, ReconR;
    std::vector<int> *ChannelId=nullptr;
    double distance; // the distance between true muon track and detector center.
    t->SetBranchAddress("RunNo", &RunNo);
    t->SetBranchAddress("FileNo", &FileNo);
    t->SetBranchAddress("TriggerNo", &TriggerNo);
    t->SetBranchAddress("TotalPE", &TotalPE);
    t->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    t->SetBranchAddress("TimeRange", &TimeRange);
    t->SetBranchAddress("cosTheta", &cosTheta);
    t->SetBranchAddress("Phi", &Phi);
    t->SetBranchAddress("cosAlpha", &cosAlpha);
    t->SetBranchAddress("nChannels",&nChannels);
    t->SetBranchAddress("ChannelIdList",ChannelIdList);
    t->SetBranchAddress("Beta", &Beta);
    t->SetBranchAddress("Time", Time);
    t->SetBranchAddress("Energy", Energy);
    t->SetBranchAddress("tError", &tError);
    t->SetBranchAddress("ReconR", &ReconR);
    t->SetBranchAddress("ChannelId", &ChannelId);
    t->SetBranchAddress("distance", &distance);

    TFile *fout = new TFile("./templates/MapMuon_d645.root", "recreate");
    TTree *tout = new TTree("MuonMap", "MuonMap");
    double Time2Mean[60],Time2MeanB[60];
    tout->Branch("cosTheta", &cosTheta);
    tout->Branch("Phi", &Phi);
    tout->Branch("cosAlpha", &cosAlpha);
    tout->Branch("Beta", &Beta);
    tout->Branch("nChannels",&nChannels);
    tout->Branch("ChannelIdList",ChannelIdList,"ChannelIdList[60]/I");
    tout->Branch("Time2Mean", Time2Mean, "Time2Mean[60]/D");
    tout->Branch("Energy", Energy, "Energy[60]/D");
    tout->Branch("ChannelId",&ChannelId);

    vector<int> badchannellist={26,29,38,54};
    std::vector<int> *BadChannelList = &badchannellist;
    Int_t nFlag = 0;
    for (int i = 0; i < nEntries; i++)
    {
        t->GetEntry(i);
        if (i%1000 == 0) cout << "Processing entry " << i << endl;
        // cout << "PE" << TotalPE << " PEmax2Sum" << PEmax2Sum << " TimeRange" << TimeRange << " cosTheta" << cosTheta << " nChannels" << nChannels << endl;
        if (distance > 1290*0.5)  // consider only muons passing through the detector
            continue;
        if (PEmax2Sum > 0.3 || cosTheta<0 || TotalPE<300)
            continue;
        // if (cosAlpha<0.888) continue;
        // cout << "tError" << tError << " ReconR" << ReconR << endl;
        if (tError>10) 
            continue;
        // if (ReconR> 0.48 * 1.290)
        //     continue;
        // double meanTime1 = 0;
        // double meanPE = 0;
        // double num = 0.;

        // for(int j = 0; j < 60; j++)
        // {
        //     if(!ChannelIdList[j])
        //         continue;
        //     // num++;
        //     meanPE+=Energy[j];
        //     meanTime1+=Time[j]*Energy[j];
        // }
        
        // meanPE /= num;
        // meanTime1 /= meanPE;

        for (int j = 0; j < 60; j++)
        {   
            // Time2Mean[j] = Time[j] - meanTime1;
            // Energy[j] = Energy[j] / meanPE;
            Time2Mean[j] = Time[j] ; // 在生成模板时不作归一化处理，读取时处理。
            Energy[j] = Energy[j] ;
        }

        tout->Fill();
        nFlag++;
    }
    cout << "nFlag: " << nFlag << endl;
    cout << "output: nEntries= " << tout->GetEntries() << endl;
    cout << "Finished." << endl;

    tout->Write();
    fout->Close();
    return 0;
}

// C(2,51)*4=