#include "TStopwatch.h"
#include <algorithm>
#include "TString.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include <iostream>
#include <numeric>
#include "TMath.h"
#include "TChain.h"
#include "MyStructs.h"
using namespace std;

const int NChannels = 60;
const char* TempleteFile = "../../TemplateGen/templates/MuonMap_Water.root";
const int MaxTemplate = 100000;
const int maxMCMuons = 1000;
// struct node
// {
//     double cosTheta;
//     double Phi;
//     double cosAlpha;
//     double Beta;
//     double Time2Mean[NChannels];
//     double Energy[NChannels];
// };

// struct FitResult{
//     double chi2;
//     struct node FitNode;
//     bool operator<(const FitResult &v) const
//     {
//         return chi2 < v.chi2;
//     }
// };

// struct Readin{
//     double TotalPE, PEmax2Sum, TimeRange;
//     int RunNo, FileNo, TriggerNo;
//     struct node MCNode; 
// };

void ReadMap(vector<node> &MeanMap)
{
    TFile *fmap = new TFile(TempleteFile);
    TTree *tmap = (TTree *)fmap->Get("MuonMap");
    Int_t nEntries = tmap->GetEntries();
    Int_t readTemplates = std::min(nEntries, MaxTemplate);
    double Time2Mean[NChannels], Energy[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;
    tmap->SetBranchAddress("cosTheta", &cosTheta);
    tmap->SetBranchAddress("Phi", &Phi);
    tmap->SetBranchAddress("cosAlpha", &cosAlpha);
    tmap->SetBranchAddress("Beta", &Beta);
    tmap->SetBranchAddress("Time2Mean", Time2Mean);
    tmap->SetBranchAddress("Energy", Energy);

    for (int i = 0; i < readTemplates; i++)
    {
        tmap->GetEntry(i);
        node d1;
        d1.cosTheta = cosTheta;
        d1.Phi = Phi;
        d1.cosAlpha = cosAlpha;
        d1.Beta = Beta;

        for (int j = 0; j < NChannels; j++)
        {
           d1.Time2Mean[j] = Time2Mean[j];
           d1.Energy[j] = Energy[j];
        }
        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
}

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    vector<node> MeanMap;
    ReadMap(MeanMap);
    TString inputFilename, outputfilename;
    if (argc == 4)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
        //sigma = TString::Format(argv[4]).Atof();
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitMCLarge inputFilename outputFilename [Int_t knn]" << endl;
        cout << endl;
        return 1;
    }

    const int knn = TString(argv[3]).Atoi();
    cout << "knn= " << knn << ";";

    TChain *t = new TChain("ma");
    t->Add(inputFilename);
    int totalentries=t->GetEntries();
    if (totalentries <=0 ){
        cout << "Error: no entry found in file: "<< inputFilename<< endl;
    }
    cout << "Total entries: " << totalentries << endl;

    struct Readin MCMuon;
    t->SetBranchAddress("RunNo", &MCMuon.RunNo);
    t->SetBranchAddress("FileNo", &MCMuon.FileNo);
    t->SetBranchAddress("TriggerNo", &MCMuon.TriggerNo);
    t->SetBranchAddress("TotalPE", &MCMuon.TotalPE);
    t->SetBranchAddress("PEmax2Sum", &MCMuon.PEmax2Sum);
    t->SetBranchAddress("TimeRange", &MCMuon.TimeRange);
    // t->SetBranchAddress("nChannels", &nChannels);
    t->SetBranchAddress("cosTheta", &MCMuon.MCNode.cosTheta);
    t->SetBranchAddress("Phi", &MCMuon.MCNode.Phi);
    t->SetBranchAddress("cosAlpha", &MCMuon.MCNode.cosAlpha);
    t->SetBranchAddress("Beta", &MCMuon.MCNode.Beta);
    t->SetBranchAddress("Time", MCMuon.MCNode.Time2Mean);
    t->SetBranchAddress("Energy", MCMuon.MCNode.Energy);

    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Test", "Test");
    struct FitResult Result;
    std::vector<struct FitResult> fittmps;
    tout->Branch("MCMuon", &MCMuon);
    tout->Branch("FitResult", &fittmps);

    gRandom->SetSeed(105);
    vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
    std::vector<int> *BadChannelList = &badchannellist;

    TStopwatch w;
    w.Start();

    for (int i = 0; i < totalentries; i++)
    {
        if (i % 100 == 0)
            cout << i << "-th entry." << endl;
        t->GetEntry(i);

        if (MCMuon.TotalPE < 800 || MCMuon.PEmax2Sum > 0.2 || MCMuon.MCNode.cosTheta<0)
            continue;

        double meanTime1 = 0;
        double meanPE1 = 0;
        for (int j = 0; j < NChannels; j++)
        {
            if (!IsBadChannel(j, BadChannelList))
            {
                meanTime1 += MCMuon.MCNode.Time2Mean[j];
                meanPE1 += MCMuon.MCNode.Energy[j];
            }
        }
        meanTime1 /= (NChannels-BadChannelList->size());
        meanPE1 /= (NChannels-BadChannelList->size());
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList))
            {
                MCMuon.MCNode.Time2Mean[j] = 0;
                MCMuon.MCNode.Energy[j] = 0;
            }
            else
            {
                MCMuon.MCNode.Time2Mean[j] -= meanTime1;
                MCMuon.MCNode.Energy[j] -= meanPE1;
            }
        }
    
        for (auto &&item : MeanMap)
        {   
            double chi2 = 0;
            for (int j = 0; j < NChannels; j++)
            {
                if (IsBadChannel(j, BadChannelList))
                    continue;
                chi2 += pow(MCMuon.MCNode.Time2Mean[j] - (item.Time2Mean)[j], 2);
            }

            if (fittmps.size() < knn || (fittmps.size() == knn && chi2 < fittmps[knn - 1].chi2))
            {   
                Result.chi2=chi2;
                Result.FitNode=item;
                for (int k = 0; k < NChannels; k++){
                    if (IsBadChannel(k, BadChannelList)){
                        Result.FitNode.Time2Mean[k]=0;
                    }
                }
                if (fittmps.size() < knn)
                    fittmps.emplace_back(Result);
                else
                    fittmps[knn - 1] = Result;
                sort(fittmps.begin(), fittmps.end());
            }
        }

        tout->Fill();
        if (tout->GetEntries()>=maxMCMuons)
            break;
    }
    w.Stop();
    cout << "\nTime: \t" << w.RealTime() << " , " << w.CpuTime() << endl;
    cout << "output: nEntries=" << tout->GetEntries() << endl;
    cout << "Finished." << endl;

    tout->Write();
    fout->Close();
    return 0;
}