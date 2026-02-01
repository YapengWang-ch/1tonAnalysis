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
const int MaxTemplate = 2000000;
vector<int> badchannellist={26,29,38,54};
vector<int> shiftchannellist={26};

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
        if (ChannelId) {
            std::sort(ChannelId->begin(),ChannelId->end());
            d1.ChannelId=*ChannelId;
        }
        if (d1.ChannelId.size() != nLightedPMT) {
            cout << "Warning: Template " << i << " ChannelId size mismatch: " << d1.ChannelId.size() << " vs " << nLightedPMT << endl;
        }
        double minTime = 1e10;
        double maxTime = -1e10;
        double meanTime = 0;
        double sumPE = 0;
        for (int j = 0; j < NChannels; j++)
        {
           if (IsBadChannel(j, BadChannelList)) continue;
            meanTime += Time2Mean[j]* Energy[j];
            sumPE += Energy[j];
           if (Time2Mean[j] < minTime) minTime = Time2Mean[j];
           if (Time2Mean[j] > maxTime) maxTime = Time2Mean[j];
        }
        meanTime /= sumPE;
        for (int j = 0; j < NChannels; j++)
        {   
            d1.Time2Mean[j] = Time2Mean[j] - meanTime;
            d1.Energy[j] = Energy[j] / sumPE;
        }
        d1.TimeRange = maxTime - minTime;
        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
    fmap->Close();
}

bool ReadTCali(vector<double> &Toffset,vector<double> &TError){
    ifstream infile("../data/TimeOffset_LED.txt");
    if (!infile.is_open())
    {
        cout << "Error opening TimeOffset_LED.txt" << endl;
        return false;
    }
    Toffset.resize(NChannels);
    TError.resize(NChannels);
    for (int i = 0; i < NChannels; i++)
    {
        infile >> Toffset[i] >> TError[i];
        if (infile.eof()) break;
    }
    infile.close();
    cout << "Finish reading Time Calibration, " << Toffset.size() << " channels." << endl;
    return true;
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
    if (argc == 6)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitMCLarge inputFilename outputFilename templateFile  [Int_t knn]  [float Ts]" << endl;
        cout << endl;
        return 1;
    }
    const char* templatefile = argv[3];
    const int knn = TString(argv[4]).Atoi();
    const double Ts = TString(argv[5]).Atof();
    double sigma=sigma_TCali;
    double Ts2=Ts*Ts;
    cout << "knn= " << knn << ";";
    cout << "sigma= " << sigma << ";" << "Ts= "<< Ts <<endl;

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
    tout->Branch("Energy", Energy, Form("Energy[%d]/D",NChannels));

    tout->Branch("BestTime2Mean", BestTime2Mean, Form("BestTime2Mean[%d]/D",NChannels));
    tout->Branch("BestcosTheta", BestcosTheta, TString::Format("BestcosTheta[%d]/D", knn));
    tout->Branch("BestPhi", BestPhi, TString::Format("BestPhi[%d]/D", knn));
    tout->Branch("BestcosAlpha", BestcosAlpha, TString::Format("BestcosAlpha[%d]/D", knn));
    tout->Branch("BestBeta", BestBeta, TString::Format("BestBeta[%d]/D", knn));
    tout->Branch("Chi2", Chi2, TString::Format("Chi2[%d]/D", knn));
    tout->Branch("TQratio",TQratio, TString::Format("TQratio[%d]/D", knn));

    gRandom->SetSeed(105);

    // Initialize shifts
    for (int j = 0; j < NChannels; j++)
    {
        TimeShift[j] = gRandom->Gaus(0, sigma);
        PEShift[j] = gRandom->Gaus(0, eta);
    }

    TStopwatch w;
    w.Start();
    int aentry = t->GetEntries();

    for (int i = 0; i < aentry; i++)
    {
        if (i % 100 == 0)
            cout << i << "-th entry." << endl;

        t->GetEntry(i);
        if (nLightedPMT < 56){
            continue;
        }
        // cout << "Event " << i << ": RunNo=" << RunNo << ", FileNo=" << FileNo << ", TriggerNo=" << TriggerNo 
            //  << ", nChannels=" << nChannels << ", nLightedPMT=" << nLightedPMT << endl;
        
        if ( PEmax2Sum > 0.3 || cosTheta<0)
            continue;

        // Apply calibration shifts
        for (int j = 0; j < NChannels; j++)
        {
            Time[j] += TimeShift[j];
            Energy[j] = Energy[j] * (1 + PEShift[j]);
        }

        // Calculate mean time and normalize
        double meanTime1 = 0;
        double meanPE1 = 0;
        double maxTime = -1e10;
        double minTime = 1e10;
        for (int j = 0; j < NChannels; j++)
        {
            if (IsBadChannel(j, BadChannelList))
            {
                Time[j] = 0;
                Energy[j] = 0;
            }
            else if (Energy[j] > 0) // Only use channels with energy
            {
                meanTime1 += Time[j] * Energy[j];
                meanPE1 += Energy[j];
                if (Time[j] < minTime) minTime = Time[j];
                if (Time[j] > maxTime) maxTime = Time[j];
            }
        }
        
        if (meanPE1 <= 0) {
            cout << "Warning: Event " << i << " has zero total PE, skipping." << endl;
            continue;
        }
        
        meanTime1 /= meanPE1;
        double timeRange = maxTime - minTime;

        // Normalize times and energies
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) {
                Time1[j] = 0;
                Energy[j] = 0;
            } else {
                Time1[j] = Time[j] - meanTime1;
                Energy[j] = Energy[j] / meanPE1;
            }
        }

        // cout << "After calibration: meanTime= " << meanTime1 << ", TimeRange= " << timeRange 
            //  << ", meanPE1= " << meanPE1 << endl;

        if (!ChannelId){
            cout << "ChannelId is nullptr!" << endl;
            continue;
        }

        vector<fitresult> tmp;
        int templateCount = 0;
        
        cout << "Event " << i << ": nLightedPMT=" << nLightedPMT << ", ChannelId: ";
        for (const auto& ch : *ChannelId) {
            cout << ch << " ";
        }
        cout << endl;
        for (auto &item : MeanMap)
        {
            templateCount++;
            // if (templateCount % 100000 == 0) {
            //     cout << "Processing template " << templateCount << " of " << MeanMap.size() << endl;
            // }

            // Check if template matches event
            if (item.nChannels != nLightedPMT) continue;
            if (!vectorsEqual(*ChannelId, item.ChannelId)) continue;
            
            // cout << "Matching template found: nChannels=" << item.nChannels << endl;
            double chi2 = 0;
            double chi2_time = 0, chi2_charge = 0;

            for (int j = 0; j < ChannelId->size(); j++)
            {   
                int ch = (*ChannelId)[j];
                
                // Safety checks
                
                if (ch < 0 || ch >= NChannels) {
                    cout << "Error: Channel ID " << ch << " out of bounds!" << endl;
                    continue;
                }
                if (IsBadChannel(ch, BadChannelList)) continue;
                
                // Avoid division by zero
                if (Energy[ch] <= 0 || item.Energy[ch] <= 0) continue;

                double time_weight = Ts2/Energy[ch]/meanPE1 + Ts2/item.Energy[ch]/meanPE1 + sigma_TCali*sigma_TCali;
                double charge_weight = meanPE1*(Energy[ch]*(1+eta*eta) + item.Energy[ch])*(1+sigma2mu_SPE);
                
                // Additional safety check for weights
                if (time_weight <= 0 || charge_weight <= 0) continue;

                // if (!IsBadChannel(ch, ShiftChannelList)) {
                if (true){
                    chi2_time += pow(Time1[ch] - item.Time2Mean[ch], 2) / time_weight;
                }
                chi2_charge += meanPE1*meanPE1 * pow(Energy[ch] - item.Energy[ch], 2) / charge_weight;
            }
            
            chi2 = chi2_time + chi2_charge;

            // Add to results if it's one of the best knn
            if (tmp.size() < knn || chi2 < tmp.back().chi2)
            {
                fitresult rr;
                rr.chi2 = chi2;
                rr.tqratio = (chi2_charge > 0) ? chi2_time / chi2_charge : 0;
                rr.costheta = item.cosTheta;
                rr.phi = item.Phi;
                rr.cosalpha = item.cosAlpha;
                rr.beta = item.Beta;
                for (int j = 0; j < NChannels; j++)
                {
                    rr.Time2Mean[j] = item.Time2Mean[j];
                    rr.Energy[j] = item.Energy[j];
                }
                
                if (tmp.size() < knn) {
                    tmp.push_back(rr);
                } else {
                    tmp[knn-1] = rr;
                }
                sort(tmp.begin(), tmp.end());
            }
        }

        cout << "Found " << tmp.size() << " matching templates" << endl;

        // Fill output tree
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
        if (tout->GetEntries() >= 1000)
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