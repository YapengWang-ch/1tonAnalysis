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
    Float_t sigma; // ns

    if (argc == 4)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./GetMCDirection inputFilename outputFilename [Int_t knn]" << endl;
        cout << endl;
        return 1;
    }
    
    const int maxChi2 = TString(argv[3]).Atoi();
    cout << "maxChi2= " << maxChi2 <<endl;

      
    TChain *tData = new TChain("Test");
   
    tData->Add(inputFilename);
    cout << "Total entries: " << tData->GetEntries() << endl;

    const int knn_=100;

    int RunNo, FileNo, TriggerNo;
    double PEmax2Sum, TotalPE, TimeRange;
    double BestBeta[knn_], BestcosAlpha[knn_], BestcosTheta[knn_], BestPhi[knn_], Chi2[knn_];
    double Time1[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;

    tData->SetBranchAddress("TotalPE", &TotalPE);
    tData->SetBranchAddress("PEmax2Sum", &PEmax2Sum);
    tData->SetBranchAddress("TimeRange", &TimeRange);
    tData->SetBranchAddress("BestcosTheta", BestcosTheta);
    tData->SetBranchAddress("BestPhi", BestPhi);
    tData->SetBranchAddress("BestcosAlpha", BestcosAlpha);
    tData->SetBranchAddress("BestBeta", BestBeta);
    tData->SetBranchAddress("Chi2", Chi2);
    tData->SetBranchAddress("Time1", Time1);
    // truth angle:
    tData->SetBranchAddress("cosTheta", &cosTheta);
    tData->SetBranchAddress("Phi", &Phi);
    tData->SetBranchAddress("cosAlpha", &cosAlpha);
    tData->SetBranchAddress("Beta", &Beta);

    
    //output:
    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Direction", "Direction");
    double costheta_rec, phi_rec;     // reconstructed angle.
    double costheta_truth, phi_truth; // truth angle.
    double cosalpha_rec, beta_rec; // reconsturcted position;
    double cosalpha_truth, beta_truth; // truth position;
    double trackL_truth,trackL_rec; // muon track length;
    double DeltaAngle;                // the included angle between truth angle and reconstructed angle.
    double DeltaPosition;
    double DeltaL;
    tout->Branch("TotalPE", &TotalPE);
    tout->Branch("PEmax2Sum", &PEmax2Sum);
    tout->Branch("costheta_truth", &costheta_truth);
    tout->Branch("phi_truth", &phi_truth);
    tout->Branch("costheta_rec", &costheta_rec);
    tout->Branch("phi_rec", &phi_rec);
    tout->Branch("DeltaAngle", &DeltaAngle);
    tout->Branch("cosalpha_truth", &cosalpha_truth);
    tout->Branch("beta_truth", &beta_truth);
    tout->Branch("cosalpha_rec", &cosalpha_rec);
    tout->Branch("beta_rec", &beta_rec);
    tout->Branch("trackL_truth",&trackL_truth);
    tout->Branch("trackL_rec",&trackL_rec);
    tout->Branch("DeltaL",&DeltaL);
    tout->Branch("DeltaPosition", &DeltaPosition);

    TH1D *hDeltaAngle = new TH1D("hDeltaAngle", "hDeltaAngle", 100, 0., 180.);
    //hDeltaAngle->SetTitle(";#Delta#Theta[degree];Counts");
    // TH2D *hChi2 = new TH2D("hChi2", "hChi2", 100, 0., 50, 100, 0., 300.); // delta angle & chi2

    int nCalc = 15;
    for (int i = 0; i < tData->GetEntries(); i++)
    {
        tData->GetEntry(i);
        costheta_truth = cosTheta;
        phi_truth = Phi;
        cosalpha_truth = cosAlpha;
        beta_truth = Beta;
        TVector3 dir_truth(1, 0, 0);
        dir_truth.SetTheta(TMath::ACos(cosTheta));
        dir_truth.SetPhi(Phi);
        TVector3 position_truth(1, 0, 0);
        position_truth.SetTheta(TMath::ACos(cosAlpha));
        position_truth.SetPhi(Beta);
        position_truth.SetMag(645.);
        trackL_truth = 2*abs(position_truth.Dot(dir_truth));
        //dir_truth.SetMag(1.);

        TVector3 sum(0, 0, 0);
        TVector3 total(0, 0, 0);
        int j=0;
        while (j<nCalc || Chi2[j] < maxChi2)
        {   
            double costheta = BestcosTheta[j];
            double phi = BestPhi[j];
            double cosalpha = BestcosAlpha[j];
            double beta = BestBeta[j];

            TVector3 dir(1, 0, 0);
            TVector3 position(1, 0, 0);
            dir.SetTheta(TMath::ACos(costheta));
            dir.SetPhi(phi);
            position.SetTheta(TMath::ACos(cosalpha));
            position.SetPhi(beta);
            if (Chi2[j]<=0.01){
                sum +=  10*dir;
                total +=  10*position; 
                continue;
            }
            sum += 1. / sqrt(Chi2[j]) * dir;
            total += 1. / sqrt(Chi2[j]) * position;
            j++;
            if (j>=knn_)
                break;
        }

        costheta_rec = sum.CosTheta();
        phi_rec = sum.Phi();
        cosalpha_rec = total.CosTheta();
        beta_rec = total.Phi();
       
        total.SetMag(645.);
        sum.SetMag(1.);
        trackL_rec = 2*abs(total.Dot(sum));
        DeltaPosition = (total - position_truth).Mag();
        DeltaAngle = dir_truth.Angle(sum);
        DeltaL = trackL_rec - trackL_truth;
        hDeltaAngle->Fill(DeltaAngle / TMath::Pi() * 180.);
        // for (int j = 0; j < nCalc; j++)
        // {
        //     hChi2->Fill(DeltaAngle / TMath::Pi() * 180., Chi2[j]);
        // }
        tout->Fill();
    }

    cout <<"Mean Include Angle: "<< hDeltaAngle->GetMean() << endl;

    tout->Write();
    // hChi2->Write();
    fout->Close();

    return 0;
}
