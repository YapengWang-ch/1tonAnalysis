#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TMath.h>
#include <TLatex.h>
#include <cmath>

const char* filename = "../Output/ReconMC/WAC2_0_0/ReconMC_k20.root";

void analyze_direction() {
    // 打开ROOT文件
    TFile *file = TFile::Open(filename, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file!" << std::endl;
        return;
    }

    // 获取TTree
    TTree *tree = (TTree*)file->Get("Direction");
    if (!tree) {
        std::cerr << "Error: Cannot find tree 'Direction'!" << std::endl;
        file->Close();
        return;
    }

    // 声明分支变量
    Double_t costheta_truth, phi_truth, costheta_rec, phi_rec;
    Double_t cosalpha_truth, beta_truth, cosalpha_rec, beta_rec;

    // 设置分支地址
    tree->SetBranchAddress("costheta_truth", &costheta_truth);
    tree->SetBranchAddress("phi_truth", &phi_truth);
    tree->SetBranchAddress("cosalpha_truth", &cosalpha_truth);
    tree->SetBranchAddress("beta_truth", &beta_truth);
    tree->SetBranchAddress("costheta_rec", &costheta_rec);
    tree->SetBranchAddress("phi_rec", &phi_rec);
    tree->SetBranchAddress("cosalpha_rec", &cosalpha_rec);
    tree->SetBranchAddress("beta_rec", &beta_rec);

    // 创建二维直方图
    TH2F *hAngleDiff = new TH2F("hAngleDiff", 
        "Angle Difference between Position and Direction;Truth Angle Difference [degree];Reconstructed Angle Difference [degree]",
        90, 0, 90,  // X轴: 真实角度差 (0~π)
        90, 0, 90); // Y轴: 重建角度差 (0~π)

    // 创建带状图（残差图）
    TProfile *hResidual = new TProfile("hResidual", 
        "Reconstruction Bias;Truth Angle Difference [rad];Reconstructed - Truth Angle [rad]",
        90, 0, 90); // 与hAngleDiff相同的X轴分箱

    // 计算每个事件的夹角差
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);

        // 计算真实值的夹角差
        Double_t sin_alpha_truth = std::sqrt(1 - cosalpha_truth * cosalpha_truth);
        Double_t sin_theta_truth = std::sqrt(1 - costheta_truth * costheta_truth);
        Double_t posX_truth = sin_alpha_truth * std::cos(beta_truth);
        Double_t posY_truth = sin_alpha_truth * std::sin(beta_truth);
        Double_t posZ_truth = cosalpha_truth;
        Double_t dirX_truth = sin_theta_truth * std::cos(phi_truth);
        Double_t dirY_truth = sin_theta_truth * std::sin(phi_truth);
        Double_t dirZ_truth = costheta_truth;
        Double_t dot_truth = posX_truth * dirX_truth + posY_truth * dirY_truth + posZ_truth * dirZ_truth;
        dot_truth = TMath::Min(TMath::Max(dot_truth, -1.0), 1.0);
        Double_t angleDiff_truth = std::acos(dot_truth);

        // 计算重建值的夹角差
        Double_t sin_alpha_rec = std::sqrt(1 - cosalpha_rec * cosalpha_rec);
        Double_t sin_theta_rec = std::sqrt(1 - costheta_rec * costheta_rec);
        Double_t posX_rec = sin_alpha_rec * std::cos(beta_rec);
        Double_t posY_rec = sin_alpha_rec * std::sin(beta_rec);
        Double_t posZ_rec = cosalpha_rec;
        Double_t dirX_rec = sin_theta_rec * std::cos(phi_rec);
        Double_t dirY_rec = sin_theta_rec * std::sin(phi_rec);
        Double_t dirZ_rec = costheta_rec;
        Double_t dot_rec = posX_rec * dirX_rec + posY_rec * dirY_rec + posZ_rec * dirZ_rec;
        dot_rec = TMath::Min(TMath::Max(dot_rec, -1.0), 1.0);
        Double_t angleDiff_rec = std::acos(dot_rec);

        // 填充二维直方图和带状图
        hAngleDiff->Fill(angleDiff_truth*180/TMath::Pi(), angleDiff_rec*180/TMath::Pi());
        hResidual->Fill(angleDiff_truth*180/TMath::Pi(), angleDiff_rec*180/TMath::Pi() - angleDiff_truth*180/TMath::Pi());
    }

    // 创建画布并分割为两个区域
    TCanvas *c1 = new TCanvas("c1", "Angle Difference Analysis", 800, 900);
    c1->Divide(1, 2); // 垂直分割为两部分
    
    // 上部：二维直方图
    c1->cd(1);
    gPad->SetMargin(0.12, 0.14, 0.1, 0.12); // 调整边距
    // gPad->SetLogz(); // 对数色标
    hAngleDiff->Draw("COLZ");
    
    // 添加对角线参考线
    TLine *diag = new TLine(0, 0, 90, 90);
    diag->SetLineColor(kRed);
    diag->SetLineStyle(2);
    diag->Draw("SAME");
    
    // 添加说明文本
    TLatex *tex = new TLatex();
    tex->SetTextSize(0.04);
    tex->SetTextColor(kRed);
    tex->DrawLatex(0.2, 2.8, "Perfect Reconstruction Line");
    
    // 下部：带状图
    c1->cd(2);
    gPad->SetMargin(0.12, 0.14, 0.2, 0.1); // 调整边距
    
    // 配置带状图
    hResidual->SetLineColor(kBlue);
    hResidual->SetLineWidth(2);
    hResidual->SetMarkerStyle(20);
    hResidual->SetMarkerSize(0.8);
    hResidual->SetMarkerColor(kBlue);
    
    // 添加零线参考
    hResidual->GetYaxis()->SetRangeUser(-10, 10); // 设定Y轴范围
    hResidual->Draw("HIST PE"); // 绘制误差条和点
    
    TLine *zeroLine = new TLine(0, 0, 90, 0);
    zeroLine->SetLineColor(kRed);
    zeroLine->SetLineStyle(2);
    zeroLine->Draw("SAME");
    
    // 添加统计信息
    TLatex *stats = new TLatex();
    stats->SetTextSize(0.04);
    stats->SetNDC();
    stats->DrawLatex(0.15, 0.85, Form("Mean Bias: %.4f rad", hResidual->GetMean(2)));
    stats->DrawLatex(0.15, 0.75, Form("RMS: %.4f rad", hResidual->GetRMS(2)));
    
    // 保存结果
    c1->SaveAs("../AngleAnalysis_.pdf");

    // 清理资源
    delete diag;
    delete zeroLine;
    delete tex;
    delete stats;
    delete c1;
    delete hAngleDiff;
    delete hResidual;
    file->Close();
}