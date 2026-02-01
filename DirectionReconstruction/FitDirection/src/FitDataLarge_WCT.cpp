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
const int MaxTemplate = 2000000;
// vector<int> shiftchannellist={2,5,10,11,18,26,29,34,38,40,42,46,50,51,53,54,58,17,41,37,59,6,13,23,24};
vector<int> shiftchannellist={24,2,6,7,9,58};
std::vector<int>* ShiftChannelList=&shiftchannellist;
// vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
vector<int> badchannellist={26,29,38,54};
const int PMTLimit[2]={50,56};
std::vector<int> *BadChannelList = &badchannellist;
const double sigma2mu_SPE=0.7; //the ratio of sigma and mean for SPE spectrum.
const double sigma_TCali=0.3; // the error of Time Calibration
const double eta=0.05; //the error of Gain Calibration

struct node
{
    int nChannels;
    std::vector<int> ChannelId;
    double cosTheta;
    double Phi;
    double cosAlpha;
    double Beta;
    double Time2Mean[NChannels];
    double TimeRange;
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

void PrintVector(vector<int> a){
    for (auto &value : a){
        cout <<value<<",";
    }
    cout<<"\b \b"<<endl;
}

void ReadMap(const char* TemplateFile, vector<node> &MeanMap)
{
    TFile *fmap = new TFile(TemplateFile);
    TTree *tmap = (TTree *)fmap->Get("MuonMap");
    Int_t nEntries = tmap->GetEntries();
    Int_t readTemplates = std::min(nEntries, MaxTemplate);
    int nLightedPMT;
    double Time2Mean[NChannels], Energy[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;
    std::vector<int> *ChannelId=nullptr;
    tmap->SetBranchAddress("cosTheta", &cosTheta);
    tmap->SetBranchAddress("ChannelId", &ChannelId);
    tmap->SetBranchAddress("Phi", &Phi);
    tmap->SetBranchAddress("nChannels",&nLightedPMT);
    tmap->SetBranchAddress("cosAlpha", &cosAlpha);
    tmap->SetBranchAddress("Beta", &Beta);
    tmap->SetBranchAddress("Time2Mean", Time2Mean);
    tmap->SetBranchAddress("Energy", Energy);

    // int validtemplates[60];
    // for (int i=0; i<60; i++)  {
    //     validtemplates[i]=0;
    // } 
    for (int i = 0; i < readTemplates; i++)
    {
        tmap->GetEntry(i);
        // if (nLightedPMT==50){
        //     for(int k=0; k<60;k++){
        //         if(!IsBadChannel(k,ChannelId))  validtemplates[k]++;
        //     }
        // }
        node d1;
        d1.nChannels=nLightedPMT;
        d1.cosTheta = cosTheta;
        d1.Phi = Phi;
        d1.cosAlpha = cosAlpha;
        d1.Beta = Beta;
        std::sort(ChannelId->begin(),ChannelId->end());
        d1.ChannelId=*ChannelId;

        double minTime = 1e10;
        double maxTime = -1e10;
        double meanTime = 0;
        double sumPE = 0;
        for (int j = 0; j < NChannels; j++)
        {
           if (IsBadChannel(j, BadChannelList)) continue;
        //    cout << j <<": "<< Time2Mean[j]<<", "<< Energy[j]<<endl;
            meanTime += Time2Mean[j]*Energy[j];
            sumPE += Energy[j];
           if (Time2Mean[j] < minTime) minTime = Time2Mean[j];
           if (Time2Mean[j] > maxTime) maxTime = Time2Mean[j];
        }
        // if (maxTime - minTime > 20) continue; // remove bad templates

        meanTime /= sumPE;
        // cout << "Template " << i << ": meanTime= " << meanTime << ", sunPE:"<<sumPE<<", TimeRange= " << maxTime - minTime << endl;
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) {
                d1.Time2Mean[j] = 0;
                d1.Energy[j] = 0;
                continue;
            };
            d1.Time2Mean[j] = Time2Mean[j] - meanTime;
            d1.Energy[j] = Energy[j] / sumPE;
            if (d1.Energy[j]<1e-8) d1.Energy[j]=0;
            // cout <<"entry:"<<
            // cout << j <<": "<< d1.Time2Mean[j]<<", "<< d1.Energy[j]<<endl;
        }
        d1.TimeRange = maxTime - minTime;
        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
    // cout << "50channel entries list:"<<endl;
    // for(int i=0; i<60; i++){
    //     if(IsBadChannel(i,BadChannelList)) continue;
    //     cout <<"ch:"<<i<<" valid templates:"<<validtemplates[i]<<endl;
    // }
}

int main(int argc, char **argv)
{
    // vector<int> vec1={1,2,3,4};
    // vector<int> vec2={1,2,3,4};
    // if (vec1==vec2) cout<<"vector compared successfully.";

    TString inputFilename, outputfilename, TemplateFile;
    TString TimeCalibYesOrNo;
    if (argc == 6)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
        TemplateFile = argv[3];
        TimeCalibYesOrNo = argv[4];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitDataLarge inputFilename outputFilename TemplatesFile TimeCalibYesOrNo [float Ts]" << endl;
        cout << endl;
        return 1;
    }
    const double Ts = TString(argv[5]).Atof();
    // double sigma=sigma_TCali;
    const double Ts2=Ts*Ts;
    cout << "Ts: "<<Ts<<", Ts2: "<<Ts2<<endl;
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
    std::vector<int> *ChannelId=nullptr;

    InputDataChain->SetBranchAddress("RunNo", &RunNo);
    InputDataChain->SetBranchAddress("FileNo", &FileNo);
    InputDataChain->SetBranchAddress("TriggerNo", &TriggerNo);
    InputDataChain->SetBranchAddress("nLightedPMT",&nLightedPMT);
    InputDataChain->SetBranchAddress("ReconE", &TotalPE);
    InputDataChain->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    InputDataChain->SetBranchAddress("Time", Time);
    InputDataChain->SetBranchAddress("PE", PE);
    InputDataChain->SetBranchAddress("DelayTime", DelayTime);
    InputDataChain->SetBranchAddress("BadChannelNum", &BadChannelNum);
    InputDataChain->SetBranchAddress("BadChannelId", BadChannelId);
    InputDataChain->SetBranchAddress("ChannelId", &ChannelId);

    constexpr int knn = 50; //超参k的取值上限
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



    for (int i = 0; i < nEntries; i++) //遍历缪子data，
    {
     //   if (i % 50 == 0)
        InputDataChain->GetEntry(i);
        cout << "entry:"<<i <<" "<<nLightedPMT<<"channels." <<endl;
        if (nLightedPMT!=ChannelId->size()){
            cout << "Error: valid channels error."<<endl;
        }
        if (nLightedPMT<PMTLimit[0] || nLightedPMT>PMTLimit[1]) continue;
        // if (i!=35) continue;
        std::sort(ChannelId->begin(),ChannelId->end());
        if (TimeCalibYesOrNo == "TimeCalibYes")
        {
            for (int i = 0; i < NChannels; i++)
            {
                Time[i] -= DelayTime[i];
            }
        }
        // TimeRange = *max_element(Time, Time + NChannels) - (*min_element(Time, Time + NChannels));
        double meanTime1 = 0;
        double meanPE1 = 0;
        double maxTime = -1e10;
        double minTime = 1e10;
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList))
            {
                Time1[j] = 0;
                PE1[j] = 0;
                continue;
            }
            meanTime1 += Time[j] * PE[j];
            meanPE1 += PE[j];
            if (Time[j] < minTime) minTime = Time[j];
            if (Time[j] > maxTime) maxTime = Time[j];
        }
       
        meanTime1 /= meanPE1;
        double timeRange = maxTime - minTime;

        // per-event shift list: start from global ShiftChannelList and add outliers if timeRange > 15
        std::vector<int> eventShiftList = *ShiftChannelList;
        if (timeRange > 15.0) {
            for (int j = 0; j < NChannels; j++) {
                if (IsBadChannel(j, BadChannelList)) continue;
                double delta = Time[j] - meanTime1;
                if (fabs(delta) > timeRange / 2.0) {
                    if (!IsBadChannel(j, &eventShiftList)) eventShiftList.push_back(j);
                }
            }
        }

        double leastchi2 = 1e10;
        int fitcode = 0;
        
        int TriggerFlag[60];
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) {
                // TriggerFlag[j]=0;
                continue;
            };
            Time1[j] = Time[j] - meanTime1;
            PE[j] = PE[j] / meanPE1;
            PE1[j]=PE[j];
            // TriggerFlag[j]=1;
            // if (PE[j]<1e-10){
            //     PE[j]=0;
            //     TriggerFlag[j]=0;
            // }
            
            // cout << "entry:"<<i<<" ch:"<<j<<" time:"<<Time1[j]<<" PE:"<<PE[j]<<endl;
        }
        int TemplateCount=0;
        int ChannelIdCount=0;
        // if (i!=9) continue;
        vector<fitresult> tmp;
        for (auto &&item : MeanMap)
        {   
            double chi2 = 0, chi2_time=0,chi2_charge =0;
            if (item.nChannels==nLightedPMT) {TemplateCount++;
            }else continue;
            if ((*ChannelId)!=item.ChannelId)continue;
            ChannelIdCount++;
            // for ()
            for (int j = 0; j < ChannelId->size(); j++)
            {   
                
                if (i==18 && (*ChannelId)[j]==33)continue;
                
                // if (!IsBadChannel(j, ChannelId)) continue;
                // for ()
                int ch=(*ChannelId)[j];
                double time_weight=Ts2/PE[ch]/meanPE1+Ts2/(item.Energy)[ch]/meanPE1+sigma_TCali*sigma_TCali;
                // time_weight=1;
                double charge_weight=meanPE1*(PE[ch]*(1+eta*eta)+(item.Energy)[ch])*(1+sigma2mu_SPE);
                if (!IsBadChannel(ch,ShiftChannelList)) {
                    chi2_time += pow(Time1[ch] - (item.Time2Mean)[ch], 2) / time_weight ;
                }
                // cout << Time1[j] <<", "<< (item.Time2Mean)[j]<<", "<< PE[j]<<", "<< time_weight << endl;
                
                chi2_charge += meanPE1*meanPE1*pow(PE[ch] - (item.Energy)[ch], 2) / charge_weight;
                // cout <<meanPE1<<", "<<time_weight<<", "<<charge_weight<<endl;
                // cout << Time1[j] <<", "<<PE[j]<<", "<< Tc << "."<<endl;
                // cout << chi2_time <<", "<<chi2_charge <<endl;
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
        cout <<"event:"<<i<<" templates:"<<TemplateCount<<" templates with same channels:"<<ChannelIdCount<<endl;
        cout << ChannelId->size()<<" channels:";
        PrintVector(*ChannelId);
        cout << "selected templates:"<<tmp.size()<<endl;

        for (int j = 0; j < knn && j < tmp.size(); j++)
        {   
            // if knn>
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
