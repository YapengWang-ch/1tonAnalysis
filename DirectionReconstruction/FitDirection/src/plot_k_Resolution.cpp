// calculation the directiuon resolution of the reconstruction for certain knn
#include "TTree.h"
#include "TFile.h"
#include <iostream>
#include "TMath.h"
#include "TChain.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TLine.h"
#include "Math/DistFunc.h"
using namespace std;

const double mean= 35.667222;
int main(int argc, char **argv) {
    // 读取参数
    const char* inputDirectory = nullptr;
    const char* OutputFile = nullptr;

    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <MCResultFile> <OutputFile>" << endl;
        return 1;
    }
    inputDirectory = argv[1];
    OutputFile = argv[2];

    // 创建输出文件
    TH1D *Resolution = new TH1D("Resolution", "Resolution", 100, 0, 100);

    for (int i = 0; i < 100; i++) {
        // 构造输入文件名
        const char* inputFilename = Form("%s/AngularResolution_k%d.root", inputDirectory, i+1);
        
        // 打开输入文件
        TFile *inputFile = TFile::Open(inputFilename);
        if (!inputFile || inputFile->IsZombie()) {
            cerr << "Error opening file: " << inputFilename << endl;
            continue;
        }

        // 获取直方图
        TH1D *h = dynamic_cast<TH1D*>(inputFile->Get("DeltaAngle"));
        if (!h) {
            cerr << "DeltaAngle histogram not found in " << inputFilename << endl;
            inputFile->Close();
            continue;
        }

        //     // 计算68.3%截断位置
        // double total = h->Integral(0, h->GetNbinsX() + 1);
        // double sum = 0;
        // double angle68 = 0;
        // int bin68 = 1;
        // for (int j = 1; j <= h->GetNbinsX(); ++j) {
        //     sum += h->GetBinContent(j);
        //     if (sum / total >= 0.683) {
        //         bin68 = j;
        //         // 线性插值
        //         double sum_prev = sum - h->GetBinContent(j);
        //         double frac = (0.683 * total - sum_prev) / (sum - sum_prev);
        //         double x1 = h->GetBinCenter(j - 1);
        //         double x2 = h->GetBinCenter(j);
        //         angle68 = x1 + frac * (x2 - x1);
        //         break;
        //     }
        // }
        // Resolution->SetBinContent(i + 1, angle68);

        // double k = chi2fit->GetParameter(1);
        // double scale = chi2fit->GetParameter(2);

        // 设置分辨率直方图内容
        // double resolution = TMath::Sqrt(k * scale);
        Resolution->SetBinContent(i + 1, h->GetMean());
        inputFile->Close();
    }

    // 设置直方图属性并保存
    Resolution->SetTitle("Angular Resolution");
    Resolution->SetXTitle("k");
    Resolution->SetYTitle("Angular Resolution (degree)");
    TLine *line = new TLine(0, mean, 100, mean);
    line->SetLineColor(kRed);
    line->SetLineStyle(2);
    line->SetLineWidth(2);
    TCanvas *c1 = new TCanvas("c1", "Angular Resolution", 800, 600);
    Resolution->Draw();
    line->Draw("same");    
    TFile *fout = new TFile(OutputFile, "recreate");
    Resolution->Write();
    c1->Write();
    fout->Close();

    return 0;
}