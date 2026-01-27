#include "TChain.h"
#include "TFile.h"
// #include "TString.h"
// #include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
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

double fitChi2(const char * FilePath, const char * HistogramName, double p, double lambda) 
{
    double popt[7]={1e4,0.5,80,20,4,30,10};

    TF1*func_ser=new TF1("func_ser",SER,20,800,7);
    func_ser->SetParameter(0,popt[0]);
    // func_ser->SetParameter(1,p);
    func_ser->FixParameter(1,p);
    func_ser->SetParameter(2,popt[2]);
    func_ser->SetParameter(3,popt[3]);
    // func_ser->SetParameter(4,lambda);
    func_ser->FixParameter(4,lambda);
    func_ser->SetParameter(5,popt[5]);
    func_ser->SetParameter(6,popt[6]);
    
    func_ser->SetParName(0,"Scale");
    func_ser->SetParName(1,"Frac");
    func_ser->SetParName(2,"mu");
    func_ser->SetParName(3,"sigma");
    func_ser->SetParName(4,"lam");
    func_ser->SetParName(5,"mu_ts");
    func_ser->SetParName(6,"sigma_ts");

    func_ser->SetParLimits(0,0,1e12);
    // func_ser->SetParLimits(1,0,1);
    func_ser->SetParLimits(2,0,200);
    func_ser->SetParLimits(3,1,30);
    // func_ser->SetParLimits(4,2,10);
    func_ser->SetParLimits(5,10,200);        
    func_ser->SetParLimits(6,1,100);    


    func_ser->SetLineColor(2);
    func_ser->SetLineWidth(2);
    //////////////////////////////////////////////
    TCanvas*c1=new TCanvas();
    TFile*f=new TFile(FilePath,"read");
    TH1D*th1d=(TH1D*)f->Get(HistogramName);
    auto res = GetFWHM(th1d);
    double fwhm = res.first;
    double peakX = res.second;
    double sigma_equiv = fwhm / 2.355;
    printf("FWHM=%.3f  peak=%.3f  sigma_eq=%.3f  res(FWHM/peak)=%.4f\n", fwhm, peakX, sigma_equiv, fwhm/peakX);

    // Int_t maxBin = th1d->GetMaximumBin();
    // Double_t maxBinCenter = th1d->GetXaxis()->GetBinCenter(maxBin);
    
    func_ser->SetParameter(0,1e5);
    // cout <<"scale set"<<endl;
    // func_ser->SetParLimits(2,peakX*0.5,peakX*1.5);
    func_ser->SetParameter(2,peakX);
    // cout << "peak set"<<endl;
    func_ser->SetParameter(3,sigma_equiv); 
    // cout << "sigma set"<<endl;

    func_ser->SetParameter(5,peakX*0.45);
    // func_ser->SetParLimits(5,peakX*0.2,peakX);
    func_ser->SetParameter(6,peakX*0.3);
    // if (strcmp(HistogramName,"ch00")==0){
    //     func_ser->SetParameter(5,5);
    //     func_ser->SetParameter(5,45);
    //     cout <<"ch00 set."<<endl;
    // }
    // if (strcmp(HistogramName,"ch02")==0){
    //     func_ser->SetParameter(5,8);
    //     func_ser->SetParameter(5,20);
    //     cout <<"ch00 set."<<endl;
    // }
    // vector<double> para;
    // if (!th1d){
    //     printf("error: Histogram not found\n");
    //     para.push_back(0);
    //     para.push_back(0);
    //     para.push_back(0);
    //     return para;
    // }
    // func_ser->SetParameter(0,th1d->GetBinContent(30));
    th1d->Draw("");
    th1d->Fit("func_ser","R","",30,5*peakX);
    func_ser->Draw("SAME");
    return func_ser->GetChisquare();
    // TF1* func_gamma=new TF1("Gamma",DrawGamma,20,800,7);
    // func_gamma->SetLineColor(kGreen);
    // func_gamma->SetLineStyle(2);
    // TF1* func_tweedie=new TF1("Tweedie",DrawTweedie,20,800,7);
    // func_tweedie->SetLineColor(kBlue);
    // func_tweedie->SetLineStyle(2);
    // for(int i=0; i<7; i++){
    //     func_gamma->SetParameter(i,func_ser->GetParameter(i));
    //     func_tweedie->SetParameter(i,func_ser->GetParameter(i));
    // }
    // func_gamma->Draw("SAME");
    // func_tweedie->Draw("SAME");
    // th1d->Sa
    // double frac=func_ser->GetParameter(1);
    // double sigma_frac=func_ser->GetParError(1);
    // double mu=func_ser->GetParameter(2);
    // double sigma_mu=func_ser->GetParError(2);
    // double lammu=func_ser->GetParameter(4)*func_ser->GetParameter(5);
    // double sigma_lammu=std::sqrt(func_ser->GetParameter(4)*func_ser->GetParameter(4)*func_ser->GetParError(5)*func_ser->GetParError(5)+func_ser->GetParameter(5)*func_ser->GetParameter(5)*func_ser->GetParError(4)*func_ser->GetParError(4));

    // double GainEff=frac*mu+(1-frac)*lammu;
    // double Sigma_GainEff=std::sqrt(sigma_frac*sigma_frac*(mu-lammu)*(mu-lammu)+frac*frac*sigma_mu*sigma_mu+(1-frac)*(1-frac)*sigma_lammu*sigma_lammu);
    // double Reso=func_ser->GetParameter(3)/func_ser->GetParameter(2);
    // {
    //     TLatex latex;
    //     latex.SetNDC();
    //     latex.SetTextFont(42);
    //     latex.SetTextSize(0.035);
    //     double chi2 = func_ser->GetChisquare();
    //     int ndf = (int)func_ser->GetNDF();
    //     double redchi = (ndf>0) ? chi2/ndf : 0;
    //     latex.DrawLatex(0.35, 0.78, Form("GainEff = %.5g #pm %.5g", GainEff, Sigma_GainEff));
    //     latex.DrawLatex(0.35, 0.73, Form("frac = %.4f ", frac));
    //     latex.DrawLatex(0.35, 0.68, Form("mu = %.3f #pm %.3f", mu, sigma_mu));
    //     latex.DrawLatex(0.35, 0.63, Form("sigma = %.4f", func_ser->GetParameter(3)));
    //     latex.DrawLatex(0.35, 0.58, Form("lambda = %.4f", func_ser->GetParameter(4)));
    //     latex.DrawLatex(0.35, 0.53, Form("mu_lambda = %.4f", func_ser->GetParameter(5)));
    //     latex.DrawLatex(0.35, 0.48, Form("#chi^{2}/ndf = %.2f / %d (%.2f)", chi2, ndf, redchi));
    // }
    // if (mu<func_ser->GetParameter(5)){
    //     cout <<"Warning: mu:"<<mu<<" less than mu_ts:"<<func_ser->GetParameter(5)<<endl;
    // }
    // if (func_ser->GetParameter(4)<2){
    //     cout <<"Warning: lambda:"<<func_ser->GetParameter(4)<<" < 2."<<endl;
    // }
    // vector<double> para;
    // para.push_back(GainEff);
    // para.push_back(Sigma_GainEff);
    // para.push_back(Reso);
    // para.push_back(mu);
    // para.push_back(sigma_mu);
    // para.push_back(func_ser->GetChisquare());
    // para.push_back(func_ser->GetNDF());
    // cout<<"Effective Gain:"<<endl;
    // cout<<GainEff<<endl;
    // printf("1\n");
    // f->Delete();
    // th1d->Delete();
    // func_ser->Delete();
    //   printf("2\n");
    // c1->SetLogy();
    // c1->SaveAs(Form("../output/fit/fit%s.pdf",HistogramName));
    // return para;
}


int GainScan(const char* inputpath, const char* outputpath){

    double chi2=0;
    double p_Limit[2]={0.3,0.7};
    double lambda_Limit[2]={2,10};
    TH2F * hScan=new TH2F("scan","scan", 20, p_Limit[0],p_Limit[1],20, lambda_Limit[0],lambda_Limit[1]);

    for(int i=0; i<20; i++){
        for(int j=0; j<20; j++){
            double p=p_Limit[0]*(1-i*1.0/20.)+i*1.0/20.*p_Limit[1];
            double lambda=lambda_Limit[0]*(1-j*1.0/20.)+j*1.0/20.*lambda_Limit[1];
            cout << "processing p:"<<p<<" lambda:"<<lambda<<endl;
            chi2=fitChi2(inputpath, "ch00", p, lambda);
            hScan->SetBinContent(i+1, j+1, chi2);
        }
    }
    hScan->SaveAs(outputpath);
    return 0;
}

int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;

    filepath=argv[1];
    outputpath=argv[2];

    if (filepath == nullptr || outputpath == nullptr) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
    // cout<< "Processing file"<<filepath <<endl;
    GainScan(filepath,outputpath);
    return 0;
}