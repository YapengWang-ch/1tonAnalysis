#include "TStopwatch.h"
#include <algorithm>
#include "TString.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include <iostream>
#include <numeric>
#include <fstream>
#include <sstream>
#include "TMath.h"
#include "TChain.h"
using namespace std;

const int NChannels = 60;
// const char* TempleteFile = "../../TemplateGen/templates/MuonMap_Water_new.root";
// const char* TSgimaFile="../data/TimeVariance.txt";
const int MaxTemplate = 2000000;
vector<int> badchannellist={26,29,38,54};
vector<int> shiftchannellist={26,29,38,54};

std::vector<int> *BadChannelList = &badchannellist;
std::vector<int> *ShiftChannelList = &shiftchannellist;
const double sigma2mu_SPE=0.7; //the ratio of sigma and mean for SPE spectrum.
const double sigma_TCali=0.3; // the error of Time Calibration
// const double sigma_TCali=0.0; // the error of Time Calibration
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
    //double Time2B[NChannels];
    double Energy[NChannels];
};

struct fitresult
{
    double chi2;
    double tqratio;
    double costheta;
    double phi;
    double cosalpha;
    double beta;
    double Time2Mean[NChannels];
    double Energy[NChannels];
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

void ReadMap(const char* templatefile, vector<node> &MeanMap)
{
    TFile *fmap = new TFile(templatefile);
    TTree *tmap = (TTree *)fmap->Get("MuonMap");
    Int_t nEntries = tmap->GetEntries();
    Int_t readTemplates = std::min(nEntries, MaxTemplate);
    int nLightedPMT;
    std::vector<int> *ChannelId=nullptr;
    double Time2Mean[NChannels], Energy[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;
    tmap->SetBranchAddress("ChannelId", &ChannelId);
    tmap->SetBranchAddress("nChannels",&nLightedPMT);
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
        d1.nChannels=nLightedPMT;
        d1.cosTheta = cosTheta;
        d1.Phi = Phi;
        d1.cosAlpha = cosAlpha;
        d1.Beta = Beta;
        double sumtime=0;
        int count=0;
        if (ChannelId) {
            std::sort(ChannelId->begin(),ChannelId->end());
            d1.ChannelId=*ChannelId;
        }
        if (d1.ChannelId.size() != nLightedPMT) {
            cout << "Warning: Template " << i << " ChannelId size mismatch: " << d1.ChannelId.size() << " vs " << nLightedPMT << endl;
        }
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList)) continue;
            if (!IsBadChannel(j, &d1.ChannelId)) continue;
            sumtime += Time2Mean[j];
           count++;
        }
        double timeMean= sumtime / count;
        
        for (int j = 0; j < NChannels; j++)
        {
           d1.Time2Mean[j] = Time2Mean[j]- timeMean; // shift to mean time
        }
        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
}

bool vectorsEqual(const std::vector<int>& v1, const std::vector<int>& v2) {
    if (v1.size() != v2.size()) return false;
    for (size_t i = 0; i < v1.size(); ++i) {
        if (v1[i] != v2[i]) return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    TString inputFilename, outputfilename;
    //Float_t sigma; // ns
    if (argc == 5)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];

        //sigma = TString::Format(argv[4]).Atof();
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitMCLarge inputFilename outputFilename TemplateFile [Int_t knn]" << endl;
        cout << endl;
        return 1;
    }
    const char* templatefile = argv[3];
    const int knn = TString(argv[4]).Atoi();
    const double sigma = sigma_TCali;
    cout << "knn= " << knn << ";";
    cout << "sigma= " << sigma << endl;

    vector<node> MeanMap;
    ReadMap(templatefile, MeanMap);

    TChain *t = new TChain("ma");
    t->Add(inputFilename);
    cout << "Total entries: " << t->GetEntries() << endl;

    double TotalPE, PEmax2Sum, TimeRange;
    int RunNo, FileNo, TriggerNo;
    double Time[NChannels],Energy[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;
    int nChannels;
        int nLightedPMT;
    vector <int> *ChannelId=nullptr;

    t->SetBranchAddress("RunNo", &RunNo);
    t->SetBranchAddress("FileNo", &FileNo);
    t->SetBranchAddress("TriggerNo", &TriggerNo);
    t->SetBranchAddress("TotalPE", &TotalPE);
    t->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    t->SetBranchAddress("TimeRange", &TimeRange);
    t->SetBranchAddress("nChannels", &nChannels);
    t->SetBranchAddress("cosTheta", &cosTheta);
    t->SetBranchAddress("Phi", &Phi);
    t->SetBranchAddress("cosAlpha", &cosAlpha);
    t->SetBranchAddress("Beta", &Beta);
    t->SetBranchAddress("Time", Time);
    t->SetBranchAddress("Energy", Energy);
    t->SetBranchAddress("nLightedPMT",&nLightedPMT);
    t->SetBranchAddress("ChannelId", &ChannelId);
    
    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Test", "Test");
    double FitBeta, FitcosAlpha, FitcosTheta, FitPhi;
    double BestBeta[knn], BestcosAlpha[knn], BestcosTheta[knn], BestPhi[knn], Chi2[knn], BestTime2Mean[NChannels],TQratio[knn];    
    double Time1[NChannels];
    double TimeShift[NChannels];
    double PEShift[NChannels];

    tout->Branch("RunNo", &RunNo);
    tout->Branch("FileNo", &FileNo);
    tout->Branch("TriggerNo", &TriggerNo);
    tout->Branch("nLightedPMT",&nLightedPMT);
    tout->Branch("ChannelId", &ChannelId);
    tout->Branch("TotalPE", &TotalPE);
    tout->Branch("PEmax2Sum", &PEmax2Sum);
    tout->Branch("TimeRange", &TimeRange);
    tout->Branch("cosTheta", &cosTheta);
    tout->Branch("Phi", &Phi);
    tout->Branch("cosAlpha", &cosAlpha);
    tout->Branch("Beta", &Beta);

    tout->Branch("Time1", Time1, Form("Time1[%d]/D",NChannels));
    //tout->Branch("TimeShift", TimeShift, "TimeShift[NChannels]/D");
    tout->Branch("Energy", Energy, Form("Energy[%d]/D",NChannels));

    tout->Branch("BestTime2Mean", BestTime2Mean, Form("BestTime2Mean[%d]/D",NChannels));
    tout->Branch("BestcosTheta", BestcosTheta, TString::Format("BestcosTheta[%d]/D", knn));
    tout->Branch("BestPhi", BestPhi, TString::Format("BestPhi[%d]/D", knn));
    tout->Branch("BestcosAlpha", BestcosAlpha, TString::Format("BestcosAlpha[%d]/D", knn));
    tout->Branch("BestBeta", BestBeta, TString::Format("BestBeta[%d]/D", knn));
    tout->Branch("Chi2", Chi2, TString::Format("Chi2[%d]/D", knn)); // Chi2 = distance^2
    tout->Branch("TQratio",TQratio, TString::Format("TQratio[%d]/D", knn));

    gRandom->SetSeed(105);


    // double sigma = 0.1;  //Time Calibration uncertainty
    for (int j = 0; j < NChannels; j++)
    {
        TimeShift[j] = gRandom->Gaus(0, sigma);
    }
    // double error = 0.02; // Gain Calibration uncertainty;
    for (int j = 0; j < NChannels; j++)
    {
        PEShift[j] = gRandom->Gaus(0, sigma);
    }

    TStopwatch w;
    w.Start();
    int aentry = t->GetEntries();

    
    for (int i = 0; i < aentry; i++)
    {
        if (i % 100 == 0)
            cout << i << "-th entry." << endl;

        t->GetEntry(i);
        if (TotalPE < 800 || PEmax2Sum > 0.25 || cosTheta<0)
            continue;
        for (int j = 0; j < NChannels; j++)
        {
            Time[j] += TimeShift[j];
        }
        double meanTime1 = 0;
        double meanPE1 = 0;
        int count = 0;
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList)|| !IsBadChannel(j, ChannelId))
            {
                Time[j] = 0;
                Energy[j] = 0;
            }
            else
            {
                meanTime1 += Time[j];
                count +=1;
            }
        }
        meanTime1 /=count;

        double leastchi2 = 1e10;
        int fitcode = 0;
        double meanTime2  = 0;
        double meanPE2 = 0;
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) continue;
            if (!IsBadChannel(j, ChannelId)) continue;
            Time1[j] = Time[j] - meanTime1;
            // Energy[j] = Energy[j] / meanPE1;
        }

        vector<fitresult> tmp;
        // cout << "Event " << i << ": nLightedPMT=" << nLightedPMT << ", ChannelId: ";
        // for (const auto& ch : *ChannelId) {
        //     cout << ch << " ";
        // }
        // cout << endl;

        for (auto &&item : MeanMap)
        {
            if (item.nChannels != nLightedPMT) continue;
            if (!vectorsEqual(*ChannelId, item.ChannelId)) continue;
            double chi2 = 0, chi2_time = 0, chi2_charge = 0;
            double xx=0.;
            for (int j = 0; j < ChannelId->size(); j++)
            {   
                int ch = (*ChannelId)[j];
                if (ch < 0 || ch >= NChannels) {
                    cout << "Error: Channel ID " << ch << " out of bounds!" << endl;
                    continue;
                }
                if (IsBadChannel(ch, BadChannelList)) continue;
                // if (!IsBadChannel(j, ChannelId)) continue;
                // if (IsBadChannel(ch, ShiftChannelList)) continue;
                chi2_time += pow(Time1[ch] - (item.Time2Mean)[ch], 2);
            }
            chi2=chi2_time;

            if (tmp.size() < knn || (tmp.size() == knn && chi2 < tmp[knn - 1].chi2))
            {
                fitresult rr;
                rr.chi2 = chi2;
                rr.tqratio = 0;
                rr.costheta = item.cosTheta;
                rr.phi = item.Phi;
                rr.cosalpha = item.cosAlpha;
                rr.beta = item.Beta;
                for (Int_t j = 0; j < NChannels; j++)
                {
                    (rr.Time2Mean)[j] = (item.Time2Mean)[j];
                    (rr.Energy)[j] = (item.Energy)[j];
                }
                if (tmp.size() < knn)
                    tmp.emplace_back(rr);
                else
                    tmp[knn - 1] = rr;
                sort(tmp.begin(), tmp.end());
            }
        }
                // cout << "Found " << tmp.size() << " matching templates" << endl;

        for (int j = 0; j < knn; j++)
        {
            if (j < tmp.size()) {
                BestcosTheta[j] = tmp[j].costheta;
                BestPhi[j] = tmp[j].phi;
                BestcosAlpha[j] = tmp[j].cosalpha;
                BestBeta[j] = tmp[j].beta;
                Chi2[j] = tmp[j].chi2;
                TQratio[j] = tmp[j].tqratio;
                if (j == 0) {
                    for (int jj = 0; jj < NChannels; jj++) {
                        BestTime2Mean[jj] = tmp[j].Time2Mean[jj];
                    }
                }
            } else {
                // Fill with default values if not enough templates found
                BestcosTheta[j] = -2;
                BestPhi[j] = 0;
                BestcosAlpha[j] = -2;
                BestBeta[j] = 0;
                Chi2[j] = 1e10;
                TQratio[j] = 0;
            }
        }

        tout->Fill();
        if (tout->GetEntries()>=1000) // only run over 1000 events for test
            break;
    }
    
    w.Stop();
    cout << "\nTime: \t" << w.RealTime() << " , " << w.CpuTime() << endl;
    cout << "output: nEntries=" << tout->GetEntries() << endl;
    cout << "Finished." << endl;

    tout->Write();
    fout->Close();
    delete t;
    return 0;
}