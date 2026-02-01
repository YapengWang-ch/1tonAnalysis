#include "Utils/JPUtils.h"
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
#include "TVector3.h"
#include "TCanvas.h"
using namespace std;
const int NChannels = 60;

int main(int argc, char **argv)
{
    TString inputFilename;
    TString outputfilename;

    if (argc == 4)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./GetDataDirection inputFilename outputFilename [Int_t knn]" << endl;
        cout << endl;
        return 1;
    }

    TChain *tData = new TChain("Test");
    tData->Add(inputFilename + "*");
    Int_t nEntries = tData->GetEntries();
    if (nEntries == 0)
    {
        cout << "No muon in " << inputFilename << endl;
        return -1;
    }
    cout << "Total entries: " << nEntries << endl;
    const int knn = TString(argv[3]).Atoi();
    cout << "knn= " << knn << endl;

    const int knn_ = 200;

    int RunNo, FileNo, TriggerNo;
    double PEmax2Sum, ReconE, TimeRange;
    double BestBeta[knn_], BestcosAlpha[knn_], BestcosTheta[knn_], BestPhi[knn_], Chi2[knn_];
    double Time1[NChannels];
    double cosTheta, Phi;
    int nLightedPMT;

    tData->SetBranchAddress("RunNo", &RunNo);
    tData->SetBranchAddress("FileNo", &FileNo);
    tData->SetBranchAddress("TriggerNo", &TriggerNo);
    tData->SetBranchAddress("ReconE", &ReconE);
    tData->SetBranchAddress("nLightedPMT", &nLightedPMT);
    tData->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    tData->SetBranchAddress("TimeRange", &TimeRange);
    tData->GetEntry(0);

    if(true)
    {
        tData->SetBranchAddress("BestcosTheta", BestcosTheta);
        tData->SetBranchAddress("BestPhi", BestPhi);
        tData->SetBranchAddress("BestcosAlpha", BestcosAlpha);
        tData->SetBranchAddress("BestBeta", BestBeta);
        tData->SetBranchAddress("Chi2", Chi2);
        tData->SetBranchAddress("Time1", Time1);
    }
    // output:
    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Direction", "Direction");
    double costheta_rec, phi_rec; // reconstructed angle.
    double cosalpha_rec, beta_rec;
    double trackL_rec;
    double MinChi2;
    tout->Branch("RunNo", &RunNo);
    tout->Branch("FileNo", &FileNo);
    tout->Branch("TriggerNo", &TriggerNo);
    tout->Branch("ReconE", &ReconE);
    tout->Branch("PEmax2Sum", &PEmax2Sum);
    tout->Branch("costheta_rec", &costheta_rec);
    tout->Branch("phi_rec", &phi_rec);
    tout->Branch("cosalpha_rec", &cosalpha_rec);
    tout->Branch("beta_rec", &beta_rec);
    tout->Branch("trackL_rec", &trackL_rec);
    tout->Branch("TimeRange", &TimeRange);
    tout->Branch("MinChi2", &MinChi2);

    // Int_t nFlagDownwardMuon = 0;
    for (int i = 0; i < nEntries; i++)
    {
        tData->GetEntry(i);
        if (nLightedPMT < 54 || nLightedPMT > 56)
            continue;
        // if (RunNo > 1706)
        //     continue;
        //cout << "TimeRange = " << TimeRange << endl;
        TVector3 sum(0, 0, 0);
        TVector3 total(0, 0, 0);
        for (int j = 0; j < knn ; j++)
        {
            if (Chi2[j] <=0 || Chi2[j]/Chi2[0]>100) break; // avoid divde by zero and bad fits.
            MinChi2 = Chi2[0];
            double costheta = BestcosTheta[j];
            double phi = BestPhi[j];
            double cosalpha = BestcosAlpha[j];
            double beta = BestBeta[j];

            TVector3 dir(1, 0, 0);
            TVector3 position(1, 0, 0);
            dir.SetTheta(TMath::ACos(costheta));
            dir.SetPhi(phi); // which is the direction that the muon comes from, namely inverse of the direction of momentum.
            position.SetTheta(TMath::ACos(cosalpha));
            position.SetPhi(beta);
            sum += 1. / sqrt(Chi2[j]) * dir;
            total += 1. / sqrt(Chi2[j]) * position;
            // cout<<"sqrt(Chi2[j]): "<<sqrt(Chi2[j])<<endl;
        }
        costheta_rec = sum.CosTheta();
        phi_rec = sum.Phi();
        cosalpha_rec = total.CosTheta();
        beta_rec = total.Phi();
        total.SetMag(645.);
        sum.SetMag(1.);
        trackL_rec = 2*abs(total.Dot(sum));
        
        tout->Fill();
    }
    // cout << "Upward Muons/Total Muons: " << nFlagDownwardMuon << "/" << tout->GetEntries() << endl;

    tout->Write();
    fout->Close();

    return 0;
}
