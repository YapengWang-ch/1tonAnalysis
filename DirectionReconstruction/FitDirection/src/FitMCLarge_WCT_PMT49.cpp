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
const int MaxTemplate = 200000;
vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
std::vector<int> *BadChannelList = &badchannellist;
const double sigma2mu_SPE=0.7; //the ratio of sigma and mean for SPE spectrum.
// const double sigma_TCali=0.0; // the error of Time Calibration
const double eta=0.02; //the error of Gain Calibration
const double zeroappro=0.2;

struct node
{
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

        double minTime = 1e10;
        double maxTime = -1e10;
        double meanTime = 0;
        double sumPE = 0;
        for (int j = 0; j < NChannels; j++)
        {
           if (IsBadChannel(j, BadChannelList)) continue;
           if (Energy[j]<zeroappro) Energy[j]=zeroappro;
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

int main(int argc, char **argv)
{

    vector<double> Toffset, TError;
    if (!ReadTCali(Toffset, TError))
    {
        cout << "Error reading Time Calibration data." << endl;
        return 1;
    }
    // vector<double> TSigma;
    // if(ReadTSigma(TSigma)){
    //     return 1;
    // };

    TString inputFilename, outputfilename;
    //Float_t sigma; // ns
    if (argc == 7)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
        //sigma = TString::Format(argv[4]).Atof();
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./FitMCLarge inputFilename outputFilename templateFile  [Int_t knn] [float sigma_TCali] [float Ts]" << endl;
        cout << endl;
        return 1;
    }
    const char* templatefile = argv[3];
    const int knn = TString(argv[4]).Atoi();
    // const double sigma = TString(argv[4]).Atof();
    const double sigma_TCali = TString(argv[5]).Atof();
    const double Ts = TString(argv[6]).Atof();
    double sigma=sigma_TCali;

    double Ts2=Ts*Ts*(1+sigma2mu_SPE*sigma2mu_SPE)*sqrt(1+eta*eta);
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
    tout->Branch("TQratio",TQratio, TString::Format("TQratio[%d]/D", knn)); // TQ ratio = chi2_time / chi2_charge

    gRandom->SetSeed(105);


    // double sigma = 0.1;  //Time Calibration uncertainty
    for (int j = 0; j < NChannels; j++)
    {
        TimeShift[j] = gRandom->Gaus(0, sigma);
    }
    double error = 0.02; // Gain Calibration uncertainty;
    for (int j = 0; j < NChannels; j++)
    {
        PEShift[j] = gRandom->Gaus(0, error);
    }

    TStopwatch w;
    w.Start();
    int aentry = t->GetEntries();

    
    for (int i = 0; i < aentry; i++)
    {
        if (i % 10 == 0)
            cout << i << "-th entry." << endl;

        t->GetEntry(i);
     //   if(nChannels<NChannels)
         //   continue;
        if (TotalPE < 800 || PEmax2Sum > 0.2 || cosTheta<0)
            continue;
        for (int j = 0; j < NChannels; j++)
        {
            Time[j] += TimeShift[j];
            Energy[j] = Energy[j] * (1 + error);
        }
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
            else
            {
                meanTime1 += Time[j]*Energy[j];
                meanPE1 += Energy[j];
                if (Time[j] < minTime) minTime = Time[j];
                if (Time[j] > maxTime) maxTime = Time[j];
            }
        }
        meanTime1 /= meanPE1;
        double timeRange = maxTime - minTime;

        double leastchi2 = 1e10;
        int fitcode = 0;
        double meanTime2  = 0;
        double meanPE2 = 0;
        for (int j = 0; j < NChannels; j++)
        {   
            if (IsBadChannel(j, BadChannelList)) continue;
            Time1[j] = Time[j] - meanTime1;
            Energy[j] = Energy[j] / meanPE1;
            // tVector.emplace_back(Time1[j],j);
        }

        vector<fitresult> tmp;
        for (auto &&item : MeanMap)
        {
            double chi2 = 0;
            double chi2_time = 0,chi2_charge =0;
            double xx=0.;
            for (int j = 0; j < NChannels; j++)
            {   
                if (IsBadChannel(j,BadChannelList)) continue;
                double time_weight=Ts2/Energy[j]/meanPE1+Ts2/(item.Energy)[j]/meanPE1+sigma_TCali*sigma_TCali;
                double charge_weight=meanPE1*(Energy[j]*(1+eta*eta)+(item.Energy)[j])*(1+sigma2mu_SPE);
                chi2_time += pow(Time1[j] - (item.Time2Mean)[j], 2) / time_weight ;
                chi2_charge += meanPE1*meanPE1*pow(Energy[j] - (item.Energy)[j], 2) / charge_weight;
            }
            chi2 = chi2_time + chi2_charge;

            if (tmp.size() < knn || (tmp.size() == knn && chi2 < tmp[knn - 1].chi2))
            {
                fitresult rr;
                rr.chi2 = chi2;
                rr.tqratio = chi2_time / chi2_charge;
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
        for (int j = 0; j < knn; j++)
        {
            BestcosTheta[j] = tmp[j].costheta;
            BestPhi[j] = tmp[j].phi;
            BestcosAlpha[j] = tmp[j].cosalpha;
            BestBeta[j] = tmp[j].beta;
            Chi2[j] = tmp[j].chi2;
            TQratio[j] = tmp[j].tqratio;
            if (j == 0)
            {
                for (Int_t jj = 0; jj < NChannels; jj++)
                {
                    BestTime2Mean[jj] = tmp[j].Time2Mean[jj];
                }
            }
        }
        tout->Fill();
        if (tout->GetEntries()>=1000)
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