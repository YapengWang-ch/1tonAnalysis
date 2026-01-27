#include "TChain.h"
#include "TFile.h"
// #include "TString.h"
// #include "TMath.h"
#include "TH1F.h"
// #include "TH2F.h"
#include "TH1D.h"
// #include "TSystem.h" 
#include "TClass.h"
// #include "TVector3.h"
#include "TCanvas.h"
// #include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include "SER.h"
#include "TLatex.h"
#include "MultiGauss.h"
#include "TF1.h"
// #include "ctools.h"
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include "SER.h"
using namespace std;
Int_t NChannels=60;


int getdate(int second){
    TDatime datime(second);
    return datime.GetDate();
}
int getsecond(int date,int n){
    TDatime datime;
    datime.Set(date,12*3600*n);
    return datime.Convert();
}

std::pair<double,double> GetFWHM(TH1D* h){
    // 返回 {FWHM, peakX}，如果失败返回 { -1, -1 }
    if (!h) return {-1,-1};
    int nb = h->GetNbinsX();
    if (nb<=0) return {-1,-1};

    int maxBin = h->GetMaximumBin();
    double peakY = h->GetBinContent(maxBin);
    if (peakY <= 0) return {-1,-1};
    double half = peakY/2.0;

    // helper lambda 做线性插值：已知 (x1,y1) 和 (x2,y2)，求 x 当 y==half
    auto interp = [&](double x1,double y1,double x2,double y2,double y)->double{
        if (y2==y1) return (x1+x2)/2.0;
        return x1 + (y - y1) * (x2 - x1) / (y2 - y1);
    };

    double xleft = NAN, xright = NAN;

    // 向左找半高点（从峰箱向下）
    for (int b = maxBin; b >= 1; --b){
        double y = h->GetBinContent(b);
        if (y < half){
            // 插值点在 bin b 和 b+1 之间
            double x1 = h->GetXaxis()->GetBinCenter(b);
            double y1 = y;
            double x2 = h->GetXaxis()->GetBinCenter(b+1);
            double y2 = h->GetBinContent(b+1);
            xleft = interp(x1,y1,x2,y2,half);
            break;
        }
    }
    if (!std::isfinite(xleft)){
        // 如果未找到（例如半高在最左侧），用最左边界
        xleft = h->GetXaxis()->GetXmin();
    }

    // 向右找半高点（从峰箱向上）
    for (int b = maxBin; b <= nb; ++b){
        double y = h->GetBinContent(b);
        if (y < half){
            // 插值点在 bin b-1 和 b 之间
            double x1 = h->GetXaxis()->GetBinCenter(b-1);
            double y1 = h->GetBinContent(b-1);
            double x2 = h->GetXaxis()->GetBinCenter(b);
            double y2 = y;
            xright = interp(x1,y1,x2,y2,half);
            break;
        }
    }
    if (!std::isfinite(xright)){
        // 如果未找到（例如半高在最右侧），用最右边界
        xright = h->GetXaxis()->GetXmax();
    }

    double fwhm = xright - xleft;
    double peakX = h->GetXaxis()->GetBinCenter(maxBin);
    return {fwhm, peakX};
}

// int GetGainFormHist(TH1F* hCharge, double &gain, double &sigma){
    
// }

// void GetHist(int ch){
//     TTree *date=new TTree();
//     date->ReadFile("shift.csv","date/I",' ');
//     // date->Print("v");
//     int shiftdate; 
//     date->SetBranchAddress("date",&shiftdate);
//     int datesets=date->GetEntries();

//     TChain *data=new TChain("DNdata","DNdata");
//     data->Add(Form("/home/wangyp/DarkNoise/Water/Phy/output/darknoise/run*_*.root"));
//     TFile *file = new TFile(Form("./output/charge/ch%02d.root",ch), "RECREATE");

//     int EN=data->GetEntries();
//     printf("dataread entries: %d\n",EN);
//     Int_t second;
//     Int_t Channel;
//     float q;
//     data->SetBranchAddress("ChannelID",&Channel);
//     data->SetBranchAddress("Q",&q);
//     data->SetBranchAddress("Sec",&second);
    
//     // TChain *RunHour = new TChain("RunHour","RunHour");
//     // RunHour->Add("output/darknoise/RunHour.root");
//     // int runsecond;
//     // int encount;
//     // RunHour->SetBranchAddress("second",&runsecond);
//     // RunHour->SetBranchAddress("entries",&encount);
//     // printf("RunHour Read, %lld entries.\n",RunHour->GetEntries());
//     gStyle->SetTimeOffset(0);

//     // RunHour->GetEntry(0);
//     // int datebegin=getdate(runsecond);
//     TH1D *hc[datesets];
//     for (int i=0 ;i<datesets; i++ ){
//         hc[i]=new TH1D(Form("set%d",i),Form("ch%02d",ch),300,0,1500);
//     }
//     int dd=0;
//     int i=0;
//     date->GetEntry(dd);
//     while(i<EN){
//       data->GetEntry(i);
//     //   if ((i%1000==0))
//         // printf("process: %d/%d\n",i,EN);
//       if(ch!=Channel){
//         i++;
//         continue;
//       }
        
//       while(shiftdate<getdate(second+8*3600)){
//         hc[dd]->Write();
//         dd+=1;
//         date->GetEntry(dd);
//       }
//       hc[dd]->Fill(q);
//       i++;
//     }
//     hc[dd]->Write();
//     delete data;
//     delete file;
//     printf("Hist of ch %02d plotted\n",ch);
// }
vector<double> GainFit(const char * FilePath, const char * HistogramName="", const char * OutputDir=""){
    //////////////////////////////////////////////
    // SER函数拟合

    double popt[7]={1e4,0.5,1600,0.3,4,0.7,0.3};

    TF1*func_ser=new TF1("func_ser",SER,600,10000,7);
    func_ser->SetParameter(0,popt[0]);
    func_ser->SetParameter(1,popt[1]);
    func_ser->SetParameter(2,popt[2]);
    func_ser->SetParameter(3,popt[3]);
    func_ser->SetParameter(4,popt[4]);
    func_ser->SetParameter(5,popt[5]);
    func_ser->SetParameter(6,popt[6]);
    
    func_ser->SetParName(0,"Scale");
    func_ser->SetParName(1,"Frac");
    func_ser->SetParName(2,"mu");
    func_ser->SetParName(3,"sigma/mu");
    func_ser->SetParName(4,"lam");
    func_ser->SetParName(5,"mu_ts/mu");
    func_ser->SetParName(6,"sigma_ts/mu_ts");

    // 存储边界信息用于后续检查
    double parLimits[7][2] = {{0}};
    
    func_ser->SetParLimits(0,0,1e12);
    parLimits[0][0] = 0; parLimits[0][1] = 1e12;
    
    func_ser->SetParLimits(1,0.2,0.8);
    parLimits[1][0] = 0.2; parLimits[1][1] = 0.8;
    
    func_ser->SetParLimits(2,800,5000);
    parLimits[2][0] = 800; parLimits[2][1] = 5000;
    
    func_ser->SetParLimits(3,0.1,0.4);
    parLimits[3][0] = 0.1; parLimits[3][1] = 0.4;
    
    func_ser->SetParLimits(4,2,10);
    parLimits[4][0] = 2; parLimits[4][1] = 10;
    
    func_ser->SetParLimits(5,0.2,1);
    parLimits[5][0] = 0.2; parLimits[5][1] = 1;
    // 注意：参数5的边界是在后面设置的，这里先不存储
    
    func_ser->SetParLimits(6,0.1,0.8);
    parLimits[6][0] = 0.1; parLimits[6][1] = 0.8;

    func_ser->SetLineColor(2);
    func_ser->SetLineWidth(2);
    //////////////////////////////////////////////
    TCanvas*c1=new TCanvas();
    TFile*f=new TFile(FilePath,"read");
    TH1D*th1d=(TH1D*)f->Get(HistogramName);
    
    vector<double> para;
    if (!th1d){
        printf("error: Histogram not found\n");
        for(int i=0;i<7;i++) para.push_back(0);
        return para;
    }else if (th1d->GetEntries()<100){
        printf("error: Histogram entries less than 100\n");
        for(int i=0;i<7;i++) para.push_back(0);
        return para;
    }
    
    auto res = GetFWHM(th1d);
    double fwhm = res.first;
    double peakX = res.second;
    double sigma_equiv = fwhm / 2.355;
    printf("FWHM=%.3f  peak=%.3f  sigma_eq=%.3f  res(FWHM/peak)=%.4f\n", fwhm, peakX, sigma_equiv, fwhm/peakX);

    double leftEdge=1000;
    double rightEdge=7*peakX;
    while (th1d->GetBinContent(th1d->FindBin(leftEdge+1))>th1d->GetBinContent(1+th1d->FindBin(leftEdge+1)) && th1d->GetBinContent(th1d->FindBin(leftEdge+1))< th1d->GetBinContent(th1d->FindBin(peakX))){
        leftEdge+=th1d->GetBinWidth(1);
    }
    // 设置参数5的边界（根据之前的注释，这里会设置）
    // func_ser->SetParLimits(5,peakX*0.4,peakX);
    // parLimits[5][0] = peakX*0.4; parLimits[5][1] = peakX;
    
    func_ser->SetParameter(0,1e5);
    func_ser->SetParameter(2,peakX);
    func_ser->SetParameter(3,sigma_equiv);
    // func_ser->SetParameter(5,peakX*0.8);
    // func_ser->SetParameter(6,peakX*0.3);

    th1d->Draw("");
    th1d->Fit("func_ser","R","",leftEdge,rightEdge);
    func_ser->Draw("SAME");
    
    TF1* func_gamma=new TF1("Gamma",DrawGamma,300,10000,7);
    func_gamma->SetLineColor(kGreen);
    func_gamma->SetLineStyle(2);
    TF1* func_tweedie=new TF1("Tweedie",DrawTweedie,300,10000,7);
    func_tweedie->SetLineColor(kBlue);
    func_tweedie->SetLineStyle(2);
    
    for(int i=0; i<7; i++){
        func_gamma->SetParameter(i,func_ser->GetParameter(i));
        func_tweedie->SetParameter(i,func_ser->GetParameter(i));
    }
    func_gamma->Draw("SAME");
    func_tweedie->Draw("SAME");
    
    double frac=func_ser->GetParameter(1);
    double sigma_frac=func_ser->GetParError(1);
    double mu=func_ser->GetParameter(2);
    double sigma_mu=func_ser->GetParError(2);
    double lammu=func_ser->GetParameter(4)*func_ser->GetParameter(5)*mu;
    double sigma_lammu=std::sqrt(func_ser->GetParameter(4)*func_ser->GetParameter(4)*func_ser->GetParError(5)*func_ser->GetParError(5)+func_ser->GetParameter(5)*func_ser->GetParameter(5)*func_ser->GetParError(4)*func_ser->GetParError(4));

    double GainEff=frac*mu+(1-frac)*lammu;
    double Sigma_GainEff=std::sqrt(sigma_frac*sigma_frac*(mu-lammu)*(mu-lammu)+frac*frac*sigma_mu*sigma_mu+(1-frac)*(1-frac)*sigma_lammu*sigma_lammu);
    double Reso=func_ser->GetParameter(3);
    
    // 边界检查函数
    auto CheckBoundary = [&](int parIndex, double tolerance = 1e-5) -> bool {
        double value = func_ser->GetParameter(parIndex);
        double lower = parLimits[parIndex][0];
        double upper = parLimits[parIndex][1];
        
        // 检查是否接近或达到边界
        bool atLowerBound = fabs(value - lower) < tolerance;
        bool atUpperBound = fabs(value - upper) < tolerance;
        
        return (atLowerBound || atUpperBound);
    };
    
    // 检查所有参数是否达到边界
    vector<bool> atBoundary(7, false);
    vector<string> boundaryWarnings;
    
    for(int i=0; i<7; i++) {
        atBoundary[i] = CheckBoundary(i);
        if(atBoundary[i]) {
            double value = func_ser->GetParameter(i);
            double lower = parLimits[i][0];
            double upper = parLimits[i][1];
            
            if(fabs(value - lower) < 1e-5) {
                boundaryWarnings.push_back(Form("Parameter %d (%s) at LOWER bound: %.4f", 
                    i, func_ser->GetParName(i), lower));
            } else if(fabs(value - upper) < 1e-5) {
                boundaryWarnings.push_back(Form("Parameter %d (%s) at UPPER bound: %.4f", 
                    i, func_ser->GetParName(i), upper));
            }
        }
    }
    
    // 绘制文本信息
    {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.035);
        double chi2 = func_ser->GetChisquare();
        int ndf = (int)func_ser->GetNDF();
        double redchi = (ndf>0) ? chi2/ndf : 0;
        
        latex.DrawLatex(0.35, 0.78, Form("GainEff = %.5g", GainEff));
        latex.DrawLatex(0.35, 0.73, Form("frac = %.4f", frac));
        latex.DrawLatex(0.35, 0.68, Form("mu = %.3f #pm %.3f", mu, sigma_mu));
        latex.DrawLatex(0.35, 0.63, Form("sigma/mu = %.4f", func_ser->GetParameter(3)));
        latex.DrawLatex(0.35, 0.58, Form("lambda = %.4f", func_ser->GetParameter(4)));
        latex.DrawLatex(0.35, 0.53, Form("mu_lambda/mu = %.4f", func_ser->GetParameter(5)));
        latex.DrawLatex(0.35, 0.48, Form("#chi^{2}/ndf = %.2f / %d (%.2f)", chi2, ndf, redchi));
        
        // 添加边界警告信息（如果有的话）
        if(!boundaryWarnings.empty()) {
            latex.SetTextColor(kRed);
            latex.SetTextSize(0.03);
            
            latex.DrawLatex(0.35, 0.40, "WARNING: Parameters at boundaries!");
            
            double ypos = 0.35;
            for(size_t i=0; i<boundaryWarnings.size() && i<3; i++) {
                latex.DrawLatex(0.35, ypos, boundaryWarnings[i].c_str());
                ypos -= 0.05;
            }
            
            if(boundaryWarnings.size() > 3) {
                latex.DrawLatex(0.35, ypos, Form("... and %d more", (int)boundaryWarnings.size()-3));
            }
            
            // 在终端也输出警告
            cout << "WARNING: The following parameters are at boundaries:" << endl;
            for(const auto& warning : boundaryWarnings) {
                cout << "  " << warning << endl;
            }
        }
    }
    
    // 原有的检查
    if (mu<func_ser->GetParameter(5)){
        cout <<"Warning: mu:"<<mu<<" less than mu_ts:"<<func_ser->GetParameter(5)<<endl;
    }
    if (func_ser->GetParameter(4)<2){
        cout <<"Warning: lambda:"<<func_ser->GetParameter(4)<<" < 2."<<endl;
    }
    
    // 将边界检查结果也添加到返回向量中
    para.push_back(GainEff);
    para.push_back(Sigma_GainEff);
    para.push_back(Reso);
    para.push_back(mu);
    para.push_back(sigma_mu);
    para.push_back(func_ser->GetChisquare());
    para.push_back(func_ser->GetNDF());
    
    // 添加一个标志，表示是否有参数达到边界
    para.push_back(boundaryWarnings.empty() ? 0.0 : 1.0);
    
    // 添加每个参数是否达到边界的信息
    for(int i=0; i<7; i++) {
        para.push_back(atBoundary[i] ? 1.0 : 0.0);
    }

    c1->SaveAs(Form("%s/fit%s.pdf",OutputDir,HistogramName));
    return para;
}

vector<double> FullGainFit(const char * FilePath, const char * HistogramName="", const char * OutputDir="") 
{
    double popt[12]={5e6,0.99,0.009,0,50,20,0.5,1600,400,4,600,100};

    TF1*func_ser=new TF1("func_ser",CombinedSER,0,10000,12);
    func_ser->SetParameter(0,popt[0]);
    func_ser->SetParameter(1,popt[1]);
    func_ser->SetParameter(2,popt[2]);
    func_ser->SetParameter(3,popt[3]);
    func_ser->SetParameter(4,popt[4]);
    func_ser->SetParameter(5,popt[5]);
    func_ser->SetParameter(6,popt[6]);
    func_ser->SetParameter(7,popt[7]);
    func_ser->SetParameter(8,popt[8]);
    func_ser->SetParameter(9,popt[9]);
    func_ser->SetParameter(10,popt[10]);    
    func_ser->SetParameter(11,popt[11]);

    func_ser->SetParName(0,"Scale");
    func_ser->SetParName(1,"Gauss_frac");
    func_ser->SetParName(2,"Exp_frac");
    func_ser->SetParName(3,"Gauss_mu");
    func_ser->SetParName(4,"Gauss_sigma");
    func_ser->SetParName(5,"Exp_lambda");
    func_ser->SetParName(6,"Gamma_frac_ser");
    func_ser->SetParName(7,"Gamma_mu");
    func_ser->SetParName(8,"Gamma_sigma");
    func_ser->SetParName(9,"Tweedie_lambda");
    func_ser->SetParName(10,"Tweedie_mu_ts");
    func_ser->SetParName(11,"Tweedie_sigma_ts");

    // 存储边界信息用于后续检查
    double parLimits[12][2] = {0, 1e12,
                              0, 1.0,
                              0, 1.0,
                              -10, 10,
                              10, 200,
                              5, 500,
                              0.35, 0.7,
                              500, 3000,
                              10, 600,
                              2, 10,
                              250, 2000,
                              10, 1000};
    for (int i=0; i<12; i++) {
        func_ser->SetParLimits(i, parLimits[i][0], parLimits[i][1]);
    }
    
    func_ser->SetLineColor(2);
    func_ser->SetLineWidth(2);
    //////////////////////////////////////////////
    TCanvas*c1=new TCanvas();
    TFile*f=new TFile(FilePath,"read");
    TH1D*th1d=(TH1D*)f->Get(HistogramName);
    
    vector<double> para;
    if (!th1d){
        printf("error: Histogram not found\n");
        for(int i=0;i<7;i++) para.push_back(0);
        return para;
    }else if (th1d->GetEntries()<100){
        printf("error: Histogram entries less than 100\n");
        for(int i=0;i<7;i++) para.push_back(0);
        return para;
    }

    th1d->Draw("");
    th1d->Fit("func_ser","R","",0,7*1000);
    func_ser->Draw("SAME");
    
    TF1* func_gamma=new TF1("Gamma",DrawGamma,0,10000,12);
    func_gamma->SetLineColor(kGreen);
    func_gamma->SetLineStyle(2);
    TF1* func_tweedie=new TF1("Tweedie",DrawTweedie,0,10000,12);
    func_tweedie->SetLineColor(kBlue);
    func_tweedie->SetLineStyle(2);
    TF1* func_exp=new TF1("Exp",DrawExp,0,10000,12);
    func_exp->SetLineColor(kMagenta);
    func_exp->SetLineStyle(2);
    TF1* func_gauss=new TF1("Gauss",DrawGauss,0,10000,12);
    func_gauss->SetLineColor(kCyan);
    func_gauss->SetLineStyle(2);

    
    for(int i=0; i<12; i++){
        func_gamma->SetParameter(i,func_ser->GetParameter(i));
        func_tweedie->SetParameter(i,func_ser->GetParameter(i));
        func_exp->SetParameter(i,func_ser->GetParameter(i));
        func_gauss->SetParameter(i,func_ser->GetParameter(i));
    }
    func_gamma->Draw("SAME");
    func_tweedie->Draw("SAME");
    func_exp->Draw("SAME");
    func_gauss->Draw("SAME");

    double frac=func_ser->GetParameter(6);
    double sigma_frac=func_ser->GetParError(6);
    double mu=func_ser->GetParameter(7);
    double sigma_mu=func_ser->GetParError(7);
    double lammu=func_ser->GetParameter(9)*func_ser->GetParameter(10);
    double sigma_lammu=std::sqrt(func_ser->GetParameter(9)*func_ser->GetParameter(9)*func_ser->GetParError(10)*func_ser->GetParError(10)+func_ser->GetParameter(10)*func_ser->GetParameter(10)*func_ser->GetParError(9)*func_ser->GetParError(9));

    double GainEff=frac*mu+(1-frac)*lammu;
    double Sigma_GainEff=std::sqrt(sigma_frac*sigma_frac*(mu-lammu)*(mu-lammu)+frac*frac*sigma_mu*sigma_mu+(1-frac)*(1-frac)*sigma_lammu*sigma_lammu);
    double Reso=func_ser->GetParameter(8)/func_ser->GetParameter(7);
    
    // 边界检查函数
    auto CheckBoundary = [&](int parIndex, double tolerance = 1e-5) -> bool {
        double value = func_ser->GetParameter(parIndex);
        double lower = parLimits[parIndex][0];
        double upper = parLimits[parIndex][1];
        
        // 检查是否接近或达到边界
        bool atLowerBound = fabs(value - lower) < tolerance;
        bool atUpperBound = fabs(value - upper) < tolerance;
        
        return (atLowerBound || atUpperBound);
    };
    
    // 检查所有参数是否达到边界
    vector<bool> atBoundary(12, false);
    vector<string> boundaryWarnings;
    
    for(int i=0; i<12; i++) {
        atBoundary[i] = CheckBoundary(i);
        if(atBoundary[i]) {
            double value = func_ser->GetParameter(i);
            double lower = parLimits[i][0];
            double upper = parLimits[i][1];
            
            if(fabs(value - lower) < 1e-5) {
                boundaryWarnings.push_back(Form("Parameter %d (%s) at LOWER bound: %.4f", 
                    i, func_ser->GetParName(i), lower));
            } else if(fabs(value - upper) < 1e-5) {
                boundaryWarnings.push_back(Form("Parameter %d (%s) at UPPER bound: %.4f", 
                    i, func_ser->GetParName(i), upper));
            }
        }
    }
    
    // 绘制文本信息
    {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.035);
        double chi2 = func_ser->GetChisquare();
        int ndf = (int)func_ser->GetNDF();
        double redchi = (ndf>0) ? chi2/ndf : 0;
        
        latex.DrawLatex(0.35, 0.78, Form("GainEff = %.5g", GainEff));
        latex.DrawLatex(0.35, 0.73, Form("frac = %.4f", frac));
        latex.DrawLatex(0.35, 0.68, Form("mu = %.3f #pm %.3f", mu, sigma_mu));
        latex.DrawLatex(0.35, 0.63, Form("sigma = %.4f", func_ser->GetParameter(3)));
        latex.DrawLatex(0.35, 0.58, Form("lambda = %.4f", func_ser->GetParameter(4)));
        latex.DrawLatex(0.35, 0.53, Form("mu_lambda = %.4f", func_ser->GetParameter(5)));
        latex.DrawLatex(0.35, 0.48, Form("#chi^{2}/ndf = %.2f / %d (%.2f)", chi2, ndf, redchi));
        
        // 添加边界警告信息（如果有的话）
        if(!boundaryWarnings.empty()) {
            latex.SetTextColor(kRed);
            latex.SetTextSize(0.03);
            
            latex.DrawLatex(0.35, 0.40, "WARNING: Parameters at boundaries!");
            
            double ypos = 0.35;
            for(size_t i=0; i<boundaryWarnings.size() && i<3; i++) {
                latex.DrawLatex(0.35, ypos, boundaryWarnings[i].c_str());
                ypos -= 0.05;
            }
            
            if(boundaryWarnings.size() > 3) {
                latex.DrawLatex(0.35, ypos, Form("... and %d more", (int)boundaryWarnings.size()-3));
            }
            
            // 在终端也输出警告
            cout << "WARNING: The following parameters are at boundaries:" << endl;
            for(const auto& warning : boundaryWarnings) {
                cout << "  " << warning << endl;
            }
        }
    }
    
    // 将边界检查结果也添加到返回向量中
    para.push_back(GainEff);
    para.push_back(Sigma_GainEff);
    para.push_back(Reso);
    para.push_back(mu);
    para.push_back(sigma_mu);
    para.push_back(func_ser->GetChisquare());
    para.push_back(func_ser->GetNDF());
    
    // 添加一个标志，表示是否有参数达到边界
    para.push_back(boundaryWarnings.empty() ? 0.0 : 1.0);
    
    // 添加每个参数是否达到边界的信息
    for(int i=0; i<12; i++) {
        para.push_back(atBoundary[i] ? 1.0 : 0.0);
    }
    c1->SetLogy();
    c1->SaveAs(Form("%s/fit%s.pdf",OutputDir,HistogramName));
    return para;
}

vector<double> GaussFit(const char * FilePath, const char * HistogramName="", const char * OutputDir="") 
{
    double popt[15]={1e9, 0.6, 80, 10,
                    0.3, 100, 20,
                    0.03, 140, 30,
                    0.03, 200, 40,
                    300, 80};

    TF1*func_ser=new TF1("func_ser",MultiGauss,20,800,15);
    for (int i=0; i<15; i++){
        func_ser->SetParameter(i,popt[i]);
    }

    func_ser->SetParName(0,"Scale");
    func_ser->SetParName(1,"lambda0");
    func_ser->SetParName(2,"mu0");
    func_ser->SetParName(3,"sigma0");
    func_ser->SetParName(4,"lambda1");
    func_ser->SetParName(5,"mu1");
    func_ser->SetParName(6,"sigma1");
    func_ser->SetParName(7,"lambda2");
    func_ser->SetParName(8,"mu2");
    func_ser->SetParName(9,"sigma2");
    func_ser->SetParName(10,"lambda3");
    func_ser->SetParName(11,"mu3");
    func_ser->SetParName(12,"sigma3");
    func_ser->SetParName(13,"mu4");
    func_ser->SetParName(14,"sigma4");

    func_ser->SetLineColor(2);
    func_ser->SetLineWidth(2);
    //////////////////////////////////////////////
    TCanvas*c1=new TCanvas();
    TFile*f=new TFile(FilePath,"read");
    TH1D*th1d=(TH1D*)f->Get(HistogramName);
    Int_t maxBin = th1d->GetMaximumBin();
    Double_t maxBinCenter = th1d->GetXaxis()->GetBinCenter(maxBin);
    
    func_ser->SetParameter(0,th1d->Integral(30,600)); //scale
    func_ser->SetParLimits(0,0,1e9); 

    func_ser->SetParameter(2,maxBinCenter); //mu
    func_ser->SetParLimits(2,maxBinCenter-15,maxBinCenter+15);
    func_ser->SetParameter(5,maxBinCenter*1.5);
    func_ser->SetParLimits(5,0,maxBinCenter*5);
    func_ser->SetParameter(8,maxBinCenter*2);
    func_ser->SetParLimits(8,0,800);
    func_ser->SetParameter(11,maxBinCenter*3);
    func_ser->SetParLimits(11,0,800);
    func_ser->SetParameter(13,maxBinCenter*4);
    func_ser->SetParLimits(13,0,800);
    
    func_ser->SetParLimits(1,0.1,1);//ratio 
    func_ser->SetParLimits(4,0.0,0.5);
    func_ser->SetParLimits(7,0.0,0.2);
    func_ser->SetParLimits(10,0.0,0.1);

    func_ser->SetParLimits(3,1,30); //sigma
    func_ser->SetParLimits(6,1,60);
    func_ser->SetParLimits(9,1,100);
    func_ser->SetParLimits(12,1,200);
    func_ser->SetParLimits(14,1,200);


    vector<double> para;
    if (!th1d){
        printf("error: Histogram not found\n");
        para.push_back(0);
        return para;
    }
    // func_ser->SetParameter(0,th1d->GetBinContent(30));
    th1d->Draw("");
    th1d->Fit("func_ser","R","",20,800);

    double mean=0, sigma2=0;
    double ratio=0;
    for(int i=0; i<4; i++){
        ratio+=func_ser->GetParameter(i*3+1);
        mean+=func_ser->GetParameter(i*3+1)*func_ser->GetParameter(i*3+2);
        sigma2+=func_ser->GetParameter(i*3+1)*func_ser->GetParameter(i*3+1)*func_ser->GetParameter(i*3+3)*func_ser->GetParameter(i*3+3);
    }
    mean+=func_ser->GetParameter(13)*(1-ratio);
    sigma2+=func_ser->GetParameter(14)*func_ser->GetParameter(14)*(1-ratio)*(1-ratio);

    para.push_back(mean);
    para.push_back(TMath::Sqrt(sigma2));

    // vector<double> para;
    for(int i=0; i<15; i++){
        para.push_back(func_ser->GetParameter(i));
    }
    para.push_back(func_ser->GetChisquare());
    para.push_back(func_ser->GetNDF());


    c1->SaveAs(Form("%s/fit%s.pdf",OutputDir,HistogramName));
    return para;
}

std::vector<double> getapprogain(const char* FilePath, const char* HistogramName){
    TFile*f=new TFile(FilePath,"read");
    TH1D*hist=(TH1D*)f->Get(HistogramName);

    double sum = 0.0;
    int count = 0;

    Int_t originalFirstBin = hist->GetXaxis()->GetFirst();
    Int_t originalLastBin = hist->GetXaxis()->GetLast();
    hist->GetXaxis()->SetRangeUser(30.0, 700.0);
    Int_t maxBin = hist->GetMaximumBin();
    Double_t maxBinCenter = hist->GetXaxis()->GetBinCenter(maxBin);
    hist->GetXaxis()->SetRange(originalFirstBin, originalLastBin);

    for (int i = 0; i <= hist->GetNbinsX(); i++) {
        double binCenter = hist->GetBinCenter(i);
        if (binCenter >= 30 && binCenter <= maxBinCenter*6) {
            sum += hist->GetBinContent(i)*hist->GetBinCenter(i);
            count +=hist->GetBinContent(i);
        }
    }

    // 计算平均值
    double average = 0.0;
    if (count > 0) {
        average = sum / count;
    }
    std::vector<double> para;
    para.push_back(average);
    // std::cout << "x 轴范围在 0 到 600 内的平均值为: " << average << std::endl;

    delete hist;
    return para;
}

int GetGain(const char* inputpath, const char* outputpath,const char* fitdir){
    ofstream fout(outputpath);
    if (!fout) {
        std::cerr << "failed opening outputfile." << std::endl;
        return 1;
    }
    fout << "channel gain chi2 NDF" << endl;
    for (int i=0; i<60; i++){
        vector<double> para=GainFit(inputpath,Form("ch%02d",i),fitdir);
        fout<<i<<" "<<para[0]<<" "<<para[5] <<" "<<para[6];
        if (para.size()>7 && para[7]<0.5){
            fout << " true"<<endl;
        }else{
            fout << " false"<<endl;
        }
    }
    // for (int i=0; i<60; i++){
    //     vector<double> para=getapprogain(inputpath,Form("ch%02d",i));
    //     fout<<i<<" "<<para[0]<<endl;
    // }
    return 0;
}


int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;
    const char* fitdir = nullptr;

    filepath=argv[1];
    outputpath=argv[2];
    fitdir=argv[3];

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> <output file> <fit directory>\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GetGain(filepath,outputpath,fitdir);
    return 0;
}