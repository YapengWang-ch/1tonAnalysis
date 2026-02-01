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

int main()
{
    TChain *t = new TChain("ma");
    for (Int_t i = 1; i <= 400; i++)
    {
        t->Add(TString::Format("../MuonMapMCData/Water_30ch/Muon-PreAnalysis/Run%d.root", i));
    }

    Int_t nEntries = t->GetEntries();
    cout << "input: nEntries= " << nEntries << endl;

    int RunNo, FileNo, TriggerNo, nChannels;
    double TotalPE, PEmax2Sum, TimeRange;
    double cosTheta, Phi, cosAlpha, Beta;
    int ChannelIdList[30];
    double Time[30], Energy[30];

    t->SetBranchAddress("RunNo", &RunNo);
    t->SetBranchAddress("FileNo", &FileNo);l
    t->SetBranchAddress("TriggerNo", &TriggerNo);
    t->SetBranchAddress("TotalPE", &TotalPE);
    t->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    t->SetBranchAddress("TimeRange", &TimeRange);
    t->SetBranchAddress("cosTheta", &cosTheta);
    t->SetBranchAddress("Phi", &Phi);
    t->SetBranchAddress("cosAlpha", &cosAlpha);
    t->SetBranchAddress("Beta", &Beta);
    t->SetBranchAddress("nChannels",&nChannels);
    t->SetBranchAddress("ChannelIdList",ChannelIdList);
    t->SetBranchAddress("Time", Time);
    t->SetBranchAddress("Energy", Energy);

    TFile *fout = new TFile("../template/MuonMap30Ch_Water.root", "recreate");
    TTree *tout = new TTree("MuonMap", "MuonMap");
    double Time2Mean[30],Time2MeanB[30];
    tout->Branch("cosTheta", &cosTheta);
    tout->Branch("Phi", &Phi);
    tout->Branch("cosAlpha", &cosAlpha);
    tout->Branch("Beta", &Beta);
    tout->Branch("nChannels",&nChannels);
    tout->Branch("ChannelIdList",ChannelIdList,"ChannelIdList[60]/I");
    tout->Branch("Time2Mean", Time2Mean, "Time2Mean[30]/D");
    tout->Branch("Time2MeanB", Time2MeanB, "Time2MeanB[30]/D");
    tout->Branch("Energy", Energy, "Energy[30]/D");

    Int_t nFlag = 0;
    for (int i = 0; i < nEntries; i++)
    {
        t->GetEntry(i);
        if(nChannels<30)
            continue;
        if (TotalPE < 800 || PEmax2Sum > 0.3|| cosTheta<0)
            continue;

        double meanTime1 = accumulate(Time, Time + 30, 0.0) / 30;
        double meanPE = accumulate(Energy,Energy+30,0.0) / 30;
        double meanTimeB[4];
        meanTimeB[0] = accumulate(Time, Time+8, 0.0)/8;
        meanTimeB[1] = accumulate(Time+8, Time+16, 0.0)/8;
        meanTimeB[2] = accumulate(Time+16, Time+24, 0.0)/8;
        meanTimeB[3] = accumulate(Time+24, Time+30, 0.0)/6;

        for (int j = 0; j < 30; j++)
        {
            Time2Mean[j] = Time[j] - meanTime1;
            Energy[j] = Energy[j] - meanPE;
            int nb = j / 8;
            Time2MeanB[j] = Time[j]-meanTimeB[nb];
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
