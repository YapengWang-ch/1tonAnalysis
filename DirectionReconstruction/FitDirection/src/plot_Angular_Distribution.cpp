// plot _Angular_Distribution of muons 
// comparison with the model and the P-Value
#include "TTree.h"
#include "TFile.h"
#include <iostream>
#include "TMath.h"
#include "TChain.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TH1D.h"
using namespace std;

const char* MuonFile="../OutputNew/ReconData/ReconMuonsData_WCT_tc4.root";
const char* FigureFile="../figNew/AngularDistribution_WCT_tc4.pdf";
const char* ModelFile="../OutputNew/ReconMC/WCT_T4/ReconMC_k40.root";
double Pvalue=0; // calculated by  Pvalue.cpp
const int NbinsX=6,NbinsY=6;
const double phi_dispalce = -137.6;
// double Pvalue0=0.0204; // calculated by  Pvalue.cpp
int main() {
    // read MC muons 
    TFile* fModel = TFile::Open(ModelFile);
    TTree* tModel = nullptr;
    fModel->GetObject("Direction", tModel);
    if (!tModel) {
        cerr << "Error: Muon data not found in " << ModelFile << endl;
        fModel->Close();
        return 1;
    }
    TH2D* hModel = new TH2D("hModel", "Muon MC; cos#theta; #phi",
        NbinsX, 0.0, 1.0,
        NbinsY, -180, 180);
    tModel->Project("hModel", "phi_rec*180/TMath::Pi():costheta_rec");

    TH1D *hModel_costheta = (TH1D*)hModel->ProjectionX("hModel_costheta");
    TH1D *hModel_phi = (TH1D*)hModel->ProjectionY("hModel_phi");

    // 正确投影到ZY平面（积分X轴）
    // TH2D* h_model = new TH2D("hModel_Angle", "ZY Projection; Z Axis; Y Axis",hModel->GetNbinsZ(), hModel->GetZaxis()->GetXmin(), hModel->GetZaxis()->GetXmax(),hModel->GetNbinsY(), hModel->GetYaxis()->GetXmin(), hModel->GetYaxis()->GetXmax());
    // for (int y_bin = 1; y_bin <= hModel->GetYaxis()->GetNbins(); ++y_bin) {
    //     for (int z_bin = 1; z_bin <= hModel->GetZaxis()->GetNbins(); ++z_bin) {
    //         double sum = 0.0;
    //         for (int x_bin = 1; x_bin <= hModel->GetXaxis()->GetNbins(); ++x_bin) {
    //             sum += hModel->GetBinContent(x_bin, y_bin, z_bin);
    //         }
    //         h_model->SetBinContent(y_bin, z_bin, sum);
    //     }
    // }`
    // TH1D* hModel_costheta=(TH1D*)hModel->Project3D("y");
    // TH1D* hModel_phi=(TH1D*)hModel->Project3D("z");
    // hModel_costheta->SetLineColor(kRed);
    // hModel_phi->SetLineColor(kRed);
    // read data muons
    TFile* fMuon = TFile::Open(MuonFile);
    TTree* tMuon = nullptr;
    fMuon->GetObject("Direction", tMuon);
    if (!tMuon) {
        cerr << "Error: Muon data not found in " << MuonFile << endl;
        fMuon->Close();
        return 1;
    }
    double costheta,phi;
    tMuon->SetBranchAddress("costheta_rec",&costheta);
    tMuon->SetBranchAddress("phi_rec",&phi);

    TH2D* h_data = new TH2D("hMuon", "Muon Data; cos#theta; #phi",
                            NbinsX, 0.0, 1.0,
                            NbinsY, -180, 180);
    for (int ientry=0; ientry<tMuon->GetEntries(); ientry++){
        tMuon->GetEntry(ientry);
        phi -= TMath::Pi() / 180*phi_dispalce;
        if (phi>TMath::Pi()) phi -= 2*TMath::Pi();
        if (phi<-TMath::Pi()) phi+= 2*TMath::Pi();
        h_data->Fill(costheta,phi*180/TMath::Pi());
    }
    // tMuon->Project("hMuon", "phi_rec*180/TMath::Pi()+180:costheta_rec");
        
    TH1D *hMuon_costheta = (TH1D*)h_data->ProjectionX("hMuon_costheta");
    TH1D *hMuon_phi = (TH1D*)h_data->ProjectionY("hMuon_phi");
    // Model Normalization
    const int N_obs = h_data->Integral();
    const double E_total = hModel->Integral();
    // TH2D* h_model_scaled = dynamic_cast<TH2D*>(h_model->Clone("h_model_scaled"));
    hModel_costheta->Scale((float)N_obs / E_total);
    hModel_phi->Scale((float)N_obs / E_total);


    // Canvas initialization
    TCanvas *canv_contour = new TCanvas("contour","contour",800,600);
    canv_contour->SetTopMargin(0.);
    canv_contour->SetBottomMargin(0.);
    canv_contour->SetLeftMargin(0.);
    canv_contour->SetRightMargin(0.);
    gStyle->SetOptStat(0);
    // main plot 
    canv_contour->cd();
    TPad *canv_main = new TPad("canv_main", "canv_main", 0., 0., 0.7, 0.7, 0, 0, 0);
    canv_main->SetTopMargin(0.);
    canv_main->SetRightMargin(0.005);
    canv_main->Draw();
    canv_main->cd();
    h_data->SetTitle("");
    h_data->GetXaxis()->SetTitle("cos#theta");
    h_data->GetXaxis()->CenterTitle();
    h_data->GetYaxis()->SetTitle("#varphi[degree]");
    h_data->GetYaxis()->CenterTitle();
    h_data->Draw("col");
    // Add Pvalue
    TText *pvalue_text = new TText(0.2, 0.9, Form("P-value: %.4f", Pvalue));
    pvalue_text->SetNDC();
    pvalue_text->SetTextColor(kBlack);
    // pvalue_text->Draw("same");
    
    // top plot
    canv_contour->cd();
    TPad *canv_x_top = new TPad("canv_x_top", "canv_x_top", 0., 0.7, 0.7, 0.99, 0, 0, 0);
    canv_x_top->SetRightMargin(0.005);
    canv_x_top->SetBottomMargin(0.);
    canv_x_top->Draw();
    canv_x_top->cd();
    hMuon_costheta->GetXaxis()->SetTitle("");
    hMuon_costheta->GetYaxis()->SetNdivisions(505);
    hMuon_costheta->GetYaxis()->SetRangeUser(0.01,hMuon_costheta->GetMaximum()*2);
    hMuon_costheta->GetYaxis()->SetTitle("Entries");
    hMuon_costheta->GetYaxis()->SetLabelSize(0.11);
    hMuon_costheta->GetYaxis()->SetTitleOffset(0.45);
    hMuon_costheta->GetYaxis()->SetTitleSize(0.13);
    hMuon_costheta->GetYaxis()->CenterTitle();
    hMuon_costheta->Draw("E1");
    hModel_costheta->Draw("hist,same");
    hModel_costheta->SetLineColor(kRed);
    TLegend *lg1 = new TLegend(0.2, 0.4, 0.7, 0.85);
    lg1->SetBorderSize(0);
    lg1->SetFillStyle(0);
    lg1->AddEntry(hMuon_costheta,"Data","lpe");
    lg1->AddEntry(hModel_costheta,"MC","l");
    lg1->Draw("same");

    // right plot 
    canv_contour->cd();
    TPad *canv_y_bottom = new TPad("canv_y_bottom", "canv_y_bottom", 0.7, 0., 0.99, 0.7, 0, 0, 0);
    canv_y_bottom->SetLeftMargin(0.);
    canv_y_bottom->SetTopMargin(0.);
    canv_y_bottom->Draw();
    canv_y_bottom->cd();
    // convert TH to TGraph
    const int n_bins = hMuon_phi->GetNbinsX();
    double x[n_bins],y[n_bins],xer[n_bins],yer[n_bins];
    double xm[2*n_bins],ym[2*n_bins];
    // int j = 0;
    for(int i=1;i<=n_bins;i++)
    {   // the first bin of TH is 1
        // the first point of TGraph is 0
        x[i-1] = hMuon_phi->GetBinContent(i);
        xer[i-1] = hMuon_phi->GetBinError(i);
        y[i-1] = hMuon_phi->GetBinCenter(i);
        yer[i-1] = hMuon_phi->GetBinWidth(i)/2.;

        xm[2*i-2] = hModel_phi->GetBinContent(i);
        ym[2*i-2] = hModel_phi->GetBinCenter(i)-hModel_phi->GetBinWidth(i)/2.;
        xm[2*i-1] = hModel_phi->GetBinContent(i);
        ym[2*i-1] = hModel_phi->GetBinCenter(i)+hModel_phi->GetBinWidth(i)/2.;
        
    }

    TGraphErrors *gdata_phi = new TGraphErrors(n_bins,x,y,xer,yer);
    gdata_phi->GetXaxis()->SetNdivisions(505);
    // gdata_phi->GetXaxis()->SetRangeUser(1,40);
    gdata_phi->GetXaxis()->SetTitle("Entries");
    gdata_phi->GetXaxis()->SetLabelSize(0.08);
    gdata_phi->GetXaxis()->SetTitleOffset(0.45);
    gdata_phi->GetXaxis()->SetLabelOffset(-0.02);
    gdata_phi->GetXaxis()->SetTitleSize(0.11);
    gdata_phi->GetXaxis()->CenterTitle();
    gdata_phi->Draw("AP");
    gdata_phi->GetYaxis()->SetRangeUser(-180,180);
    TGraph *gmodel_phi = new TGraph(2*n_bins,xm,ym);
    gmodel_phi->Draw("L,same");
    gmodel_phi->SetLineColor(kRed);
    // Save the canvas
    canv_contour->SaveAs(FigureFile);
    return 0;
}