//  direction reconstruction
//  templates & analysised data is needed
#include "TStopwatch.h"
#include "TString.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include "TH1D.h"
#include <iostream>
#include <numeric>
#include "TChain.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

const int NChannels = 60;
// const char* TempleteFile = "../../TemplateGen/templates/MuonMap_Water_new.root";
// const char* TSgimaFile="../data/TimeVariance.txt";
const int MaxTemplate = 200000;
// vector<int> badchannellist={2,5,10,11,18,26,29,34,38,40,42,46,50,51,53,54,58,17,41,37,59,6,13,23,24};
// vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
vector<int> badchannellist={26,29,38,54};
std::vector<int> *BadChannelList = &badchannellist;

struct node
{
    double cosTheta;
    double Phi;
    double cosAlpha;
    double Beta;
    double Time2Mean[NChannels];
    // double Time2MeanB[NChannels];
    double Energy[NChannels];
};

struct fitresult
{
    double chi2;
    double costheta;
    double phi;
    double cosalpha;
    double beta;
    bool operator<(const fitresult &v) const
    {
        return chi2 < v.chi2;
    }
};

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}


void ReadMap(const char* TempleteFile, vector<node> &MeanMap)
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
        double sumtime=0, sumcharge=0;
        double sumtime2=0, sumcharge2=0;
        int count=0;
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList)) continue;
            sumtime += Time2Mean[j];
            sumcharge += Energy[j];
            sumtime2 += Time2Mean[j] * Time2Mean[j];
            sumcharge2 += Energy[j] * Energy[j];
            count++;
        }
        double timeMean= sumtime / count;
        double chargeMean = sumcharge / count;
        double timeVariance = (sumtime2 - sumtime * sumtime / count) / count;
        double chargeVariance = (sumcharge2 - sumcharge * sumcharge / count) / count;
        
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) continue;
           d1.Time2Mean[j] = (Time2Mean[j]- timeMean) / sqrt(timeVariance); // shift to mean time and normalize by variance
           d1.Energy[j] = (Energy[j] - chargeMean) / sqrt(chargeVariance); // shift to mean charge and normalize by variance
        }

        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
}

int main(int argc, char **argv)
{

    TString inputFilename, outputfilename, TemplateFile;
    TString TimeCalibYesOrNo;
    if (argc == 5)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
        TemplateFile = argv[3];
        TimeCalibYesOrNo = argv[4];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitDataLarge inputFilename outputFilename TemplatesFile TimeCalibYesOrNo" << endl;
        cout << endl;
        return 1;
    }
    // const double Ts = TString(argv[4]).Atof();
    // // double sigma=sigma_TCali;
    // const double Ts2=Ts*Ts;
    vector<node> MeanMap;
    ReadMap(TemplateFile,MeanMap);

    TChain *InputDataChain = new TChain("MuonAnalysis");
    InputDataChain->Add(inputFilename + "*");
    Int_t nEntries = InputDataChain->GetEntries();
    if (nEntries == 0)
    {
        cout << "No muon in " << inputFilename << endl;
        return -1;
    }
    cout << "nEntries of input file: " << nEntries << endl;

    double TotalPE, PEmax2Sum, TimeRange,ReconE;
    int RunNo, FileNo, TriggerNo,BadChannelNum,nLightedPMT;
    int BadChannelId[NChannels];
    double Time[NChannels], DelayTime[NChannels];
    double PE[NChannels];

    InputDataChain->SetBranchAddress("RunNo", &RunNo);
    InputDataChain->SetBranchAddress("FileNo", &FileNo);
    InputDataChain->SetBranchAddress("TriggerNo", &TriggerNo);
    InputDataChain->SetBranchAddress("ReconE", &TotalPE);
    InputDataChain->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    InputDataChain->SetBranchAddress("nLightedPMT", &nLightedPMT);
    InputDataChain->SetBranchAddress("Time", Time);
    InputDataChain->SetBranchAddress("PE", PE);
    InputDataChain->SetBranchAddress("DelayTime", DelayTime);
    InputDataChain->SetBranchAddress("BadChannelNum", &BadChannelNum);
    InputDataChain->SetBranchAddress("BadChannelId", BadChannelId);

    constexpr int knn = 100; //超参k的取值上限
    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Test", "Test");
    double FitBeta, FitcosAlpha, FitcosTheta, FitPhi;
    double BestBeta[knn], BestcosAlpha[knn], BestcosTheta[knn], BestPhi[knn], Chi2[knn];
    double Time1[NChannels], TimeB[NChannels], PE1[NChannels];
    
    tout->Branch("RunNo", &RunNo);
    tout->Branch("FileNo", &FileNo);
    tout->Branch("TriggerNo", &TriggerNo);
    tout->Branch("nLightedPMT", &nLightedPMT);
    tout->Branch("ReconE", &ReconE);
    tout->Branch("PEmax2Sum", &PEmax2Sum);
    tout->Branch("TimeRange", &TimeRange);
    tout->Branch("BestcosTheta", BestcosTheta, TString::Format("BestcosTheta[%d]/D", knn));
    tout->Branch("BestPhi", BestPhi, TString::Format("BestPhi[%d]/D", knn));
    tout->Branch("BestcosAlpha", BestcosAlpha, TString::Format("BestcosAlpha[%d]/D", knn));
    tout->Branch("BestBeta", BestBeta, TString::Format("BestBeta[%d]/D", knn));
    tout->Branch("Chi2", Chi2, TString::Format("Chi2[%d]/D", knn));
    tout->Branch("Time1", Time1, "Time1[60]/D");
    tout->Branch("PE1", PE1, "PE1[60]/D");



    for (int i = 0; i < nEntries; i++) //遍历缪子data，18个
    {
     //   if (i % 50 == 0)
            cout << i << endl;
        InputDataChain->GetEntry(i);
        
        if (TimeCalibYesOrNo == "TimeCalibYes")
        {
            for (int i = 0; i < NChannels; i++)
            {
                Time[i] -= DelayTime[i];
            }
        }
        
        double sumtime=0, sumcharge=0;
        double sumtime2=0, sumcharge2=0;
        int count=0;
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList))
            {
                Time[j] = 0;
                PE[j] = 0;
            }
            else
            {
                sumtime += Time[j];
                sumcharge += PE[j];
                sumtime2 += Time[j] * Time[j];
                sumcharge2 += PE[j] * PE[j];
                count++;
            }
        }
        double timeMean= sumtime / count;
        double chargeMean = sumcharge / count;
        double timeVariance = (sumtime2 - sumtime * sumtime / count) / count;
        double chargeVariance = (sumcharge2 - sumcharge * sumcharge / count) / count;

        double leastchi2 = 1e10;
        int fitcode = 0;
        double meanTime2  = 0;
        double meanPE2 = 0;
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) continue;
            Time1[j] = (Time[j] - timeMean) / sqrt(timeVariance); // shift to mean time and normalize by variance
            PE[j] = (PE[j] - chargeMean) / sqrt(chargeVariance); // shift to mean charge and normalize by variance
        }

        vector<fitresult> tmp;
        for (auto &&item : MeanMap)
        {
            double chi2 = 0, chi2_time=0,chi2_charge =0;

            for (int j = 0; j < NChannels; j++)
            {   
                if (IsBadChannel(j,BadChannelList)) continue;
                chi2_time += pow(Time1[j] - (item.Time2Mean)[j], 2);
                chi2_charge += pow(PE[j] - (item.Energy)[j], 2);
            }    
            chi2 = chi2_time + chi2_charge;  

            if (tmp.size() < knn || (tmp.size() == knn && chi2 < tmp[knn - 1].chi2))
            {
                fitresult rr;
                rr.chi2 = chi2;
                rr.costheta = item.cosTheta;
                rr.phi = item.Phi;
                rr.cosalpha = item.cosAlpha;
                rr.beta = item.Beta;
                if (tmp.size() < knn)
                    tmp.emplace_back(rr);
                else
                    tmp[knn - 1] = rr;
                sort(tmp.begin(), tmp.end());
            }
        }

        for (int j = 0; j < knn && j < tmp.size(); j++)
        {
            BestcosTheta[j] = tmp[j].costheta;
            BestPhi[j] = tmp[j].phi;
            BestcosAlpha[j] = tmp[j].cosalpha;
            BestBeta[j] = tmp[j].beta;
            Chi2[j] = tmp[j].chi2;
        }
        tout->Fill();
    }
    tout->Write();
    cout << "nEntries of output File: " << tout->GetEntries() << endl;
    fout->Close();

    return 0;
}
