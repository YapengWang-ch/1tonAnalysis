#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <fstream>
#include "TChain.h"
#include <vector>
#include <iostream>
#include "ctools.h"
#include "TPaveText.h"
using namespace std;

double TW_function(double *x, double *par) {
    double x_0 = par[0];
    double y_0 = par[1];
    double x_1 = par[2];
    double y_1 = par[3];
    double val = x[0];


    if (val > 0.01 && val < x_0)
        return y_0;
    if (val >= x_0 && val <= x_1)
        return y_0 + (y_1 - y_0) * (val - x_0) / (x_1 - x_0);
    if (val > x_1)
        return y_1;
    return 0;
}

int TWfit(const char* filename, const char* Graphname, const char* outputpath, vector<double> &par){
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return -1;
    }

    TGraphErrors *graph = (TGraphErrors*)file->Get(Graphname);
    if (!graph) {
        cerr << "Error: Cannot find graph " << Graphname << " in file " << filename << endl;
        file->Close();
        return -1;
    }

    TF1 *fitFunc = new TF1("fitFunc", TW_function, 0, 5, 4);
    double initPar[4] = {0.2, 3, 0.5, -2};
    fitFunc->SetParameters(initPar);
    graph->Fit(fitFunc, "R");

    // 计算残差并绘制残差分布
    int nPoints = graph->GetN();
    std::vector<double> Qs, residuals, Qerrs, yerrs;
    for (int i = 0; i < nPoints; ++i) {
        double x, y;
        graph->GetPoint(i, x, y);
        double y_fit = fitFunc->Eval(x);
        Qs.push_back(x);
        residuals.push_back(y - y_fit);
        Qerrs.push_back(graph->GetErrorX(i));
        yerrs.push_back(graph->GetErrorY(i));
    }

    TCanvas *cFit = new TCanvas("cFit", "TW Fit", 800, 600);
    graph->Draw("AP");
    fitFunc->Draw("SAME");
        TPaveText *pt = new TPaveText(0.55,0.60,0.95,0.92,"NDC");
    pt->SetFillColor(0);
    pt->SetTextAlign(12);
    pt->AddText("Fit Parameters:");
    for (int i = 0; i < 4; ++i) {
        TString line;
        line.Form("p%d = %.3g", i, fitFunc->GetParameter(i));
        pt->AddText(line);
    }
    double chi2 = fitFunc->GetChisquare();
    int ndf = fitFunc->GetNDF();
    TString chi2line;
    chi2line.Form("#chi^{2}/NDF = %.2f / %d = %.2f", chi2, ndf, ndf>0?chi2/ndf:0);
    pt->AddText(chi2line);
    pt->Draw();
    cFit->SaveAs(Form("%s_fit.pdf", outputpath));

    // 绘制残差分布
    TGraphErrors* gRes = new TGraphErrors(Qs.size(), Qs.data(), residuals.data(), Qerrs.data(), yerrs.data());
    gRes->SetTitle("Residual vs Q;Q;Residual");
    gRes->SetMarkerStyle(20);

    TCanvas *cRes = new TCanvas("cRes", "Residuals", 800, 600);
    gRes->Draw("AP");
    cRes->SaveAs(Form("%s_fit_residual.pdf", outputpath));

    par.clear();
    for (int i = 0; i < 4; ++i) {
        par.push_back(fitFunc->GetParameter(i));
    }
    cout << "Fitted parameters: ";
    for (const auto& p : par) {
        cout << p << " ";
    }
    cout << endl;

    file->Close();
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input_file.root> <output_file.txt>" << endl;
        return 1;
    }

    vector<double> parameters;
    std::string outFileName(argv[2]);
    size_t lastdot = outFileName.find_last_of(".");
    std::string outPrefix = (lastdot == std::string::npos) ? outFileName : outFileName.substr(0, lastdot);

    int result = TWfit(argv[1], "gMean", outPrefix.c_str(), parameters);
    if (result != 0) {
        return result;
    }
    ofstream outFile(argv[2]);
    if (!outFile) {
        cerr << "Error: Cannot open output file " << argv[2] << endl;
        return 1;
    }
    for (const auto& p : parameters) {
        outFile << p << " ";
    }
    outFile << endl;
    outFile.close();
    return 0;
}
