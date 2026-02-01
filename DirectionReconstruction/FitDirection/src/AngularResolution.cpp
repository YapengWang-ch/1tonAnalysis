// calculation the directiuon resolution of the reconstruction for certain knn
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
#include "TF1.h"
using namespace std;

Double_t ManualChiSquarePDF(Double_t *x, Double_t *par) {
        double k = par[0];     // 自由度
        double A = par[1];     // 幅值
        double scale = par[2]; // 缩放因子
    
        double xx = x[0] / scale *k ; // 缩放后的变量
        if (xx <= 0) return 0;
    
        // 卡方分布公式
        double gamma = std::tgamma(0.5 * k);
        double pdf = (1.0 / (std::pow(2.0, 0.5*k) * gamma)) 
                     * std::pow(xx, 0.5*k - 1) 
                     * std::exp(-0.5 * xx);
    
        return A * pdf / scale *k ; // 应用幅值
}

int main(int argc, char **argv){
    // read parameters
    const char* MCResultFile = nullptr;
    const char* OutputFile = nullptr;
    int knn=0;

    if (argc !=4 ){
        cout << "Usage: " << argv[0] << " <MCResultFile> <OutputFile> <knn>" << endl;
        return 1;
    }
    MCResultFile = argv[1];
    OutputFile = argv[2];
    knn = atoi(argv[3]);
    cout << "knn= " << knn << endl;
    if (knn <= 0) {
        cout << "Error: knn must be greater than 0." << endl;
        return 1;
    }

    // read tree
    TChain *tin = new TChain("Direction","Direction");
    tin->Add(MCResultFile);
    int nEntries = tin->GetEntries();
    if (nEntries == 0) {
        cout << "Error: No entries in the input file." << endl;
        return 1;
    }
    cout << "Total entries: " << nEntries << endl;

    // Set up branches
    // double costheta_rec, phi_rec;     // reconstructed angle.
    // double costheta_truth, phi_truth; // truth angle.
    // double cosalpha_rec, beta_rec; // reconsturcted position;
    // double cosalpha_truth, beta_truth; // truth position;
    // double trackL_truth,trackL_rec; // muon track length;
    double DeltaAngle;            
    int nLightedPMT;    
    // the included angle between truth angle and reconstructed angle.
    // double DeltaPosition;
    // double DeltaL;
    // tin->SetBranchAddress("costheta_rec", &costheta_rec);
    // tin->SetBranchAddress("phi_rec", &phi_rec);
    // tin->SetBranchAddress("costheta_truth", &costheta_truth);
    // tin->SetBranchAddress("phi_truth", &phi_truth);
    // tin->SetBranchAddress("cosalpha_rec", &cosalpha_rec);
    // tin->SetBranchAddress("beta_rec", &beta_rec);
    // tin->SetBranchAddress("cosalpha_truth", &cosalpha_truth);
    // tin->SetBranchAddress("beta_truth", &beta_truth);
    // tin->SetBranchAddress("trackL_truth", &trackL_truth);
    // tin->SetBranchAddress("trackL_rec", &trackL_rec);
    tin->SetBranchAddress("DeltaAngle", &DeltaAngle);
    tin->SetBranchAddress("nLightedPMT",&nLightedPMT);
    // tin->SetBranchAddress("DeltaPosition", &DeltaPosition);
    // tin->SetBranchAddress("DeltaL", &DeltaL);

    // create output histograms
    TH1D *hDeltaAngle = new TH1D("DeltaAngle", "DeltaAngle", 200, 0., 180.);

    for (int i = 0; i < nEntries; i++) {
        tin->GetEntry(i);
        if (nLightedPMT<51) continue;
        hDeltaAngle->Fill(DeltaAngle/TMath::Pi()*180.);
    }
    hDeltaAngle->SetTitle("Angular Resolution of Template Reconstruction;#Delta#Theta[degree];Counts");
    hDeltaAngle->SetMinimum(0);
    hDeltaAngle->SetMaximum(1.2 * hDeltaAngle->GetMaximum());

    // TF1 *f_chi2 = new TF1("f_chi2", ManualChiSquarePDF, 0, 180,3);
    // f_chi2->FixParameter(0,knn);
    // f_chi2->FixParameter(1,1000);
    // f_chi2->SetParameter(2,25);
    // f_chi2->SetParLimits(2,10,50);
    // hDeltaAngle->Fit(f_chi2);
    // create output file
    TFile *fout = new TFile(OutputFile, "recreate");
    if (!fout) {
        cout << "Error: failed to create output file." << endl;
        return 1;
    }
    hDeltaAngle->Write();
    // f_chi2->Write();
    fout->Close();
    cout << "Output file: " << OutputFile << endl;
    cout << "Finished." << endl;
    delete tin;
    delete hDeltaAngle;
    return 0;
}