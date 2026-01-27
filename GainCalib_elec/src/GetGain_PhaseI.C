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

vector<double> GainFit(const char * FilePath, int ch, const char * HistogramName="") 
{
    double popt[7]={1e4,0.5,80,20,4,30,10};

    TF1*func_ser=new TF1("func_ser",SER,20,800,7);
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
    func_ser->SetParName(3,"sigma");
    func_ser->SetParName(4,"lam");
    func_ser->SetParName(5,"mu_ts");
    func_ser->SetParName(6,"sigma_ts");

    func_ser->SetParLimits(0,0,1e12);
    func_ser->SetParLimits(1,0,1);
    func_ser->SetParLimits(2,0,200);
    func_ser->SetParLimits(3,1,30);
    func_ser->SetParLimits(4,2,10);
    // func_ser->SetParLimits(5,10,200);        
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

    func_ser->SetParameter(5,peakX*0.8);
    func_ser->SetParLimits(5,peakX*0.4,peakX);
    func_ser->SetParameter(6,peakX*0.3);
    if (strcmp(HistogramName,"ch00")==0){
        func_ser->SetParameter(5,5);
        func_ser->SetParameter(5,45);
        cout <<"ch00 set."<<endl;
    }
    if (strcmp(HistogramName,"ch02")==0){
        func_ser->SetParameter(5,8);
        func_ser->SetParameter(5,20);
        cout <<"ch00 set."<<endl;
    }
    vector<double> para;
    if (!th1d){
        printf("error: Histogram not found\n");
        para.push_back(0);
        para.push_back(0);
        para.push_back(0);
        return para;
    }
    if (th1d->GetEntries()<=10){
        para.push_back(150);
        para.push_back(0);
        para.push_back(0);
        para.push_back(0);
        para.push_back(0);
        para.push_back(0);
        para.push_back(0);
    return para;
    }
    // func_ser->SetParameter(0,th1d->GetBinContent(30));
    th1d->Draw("");
    th1d->Fit("func_ser","R","",30,7*peakX);
    func_ser->Draw("SAME");
    TF1* func_gamma=new TF1("Gamma",DrawGamma,20,800,7);
    func_gamma->SetLineColor(kGreen);
    func_gamma->SetLineStyle(2);
    TF1* func_tweedie=new TF1("Tweedie",DrawTweedie,20,800,7);
    func_tweedie->SetLineColor(kBlue);
    func_tweedie->SetLineStyle(2);
    for(int i=0; i<7; i++){
        func_gamma->SetParameter(i,func_ser->GetParameter(i));
        func_tweedie->SetParameter(i,func_ser->GetParameter(i));
    }
    func_gamma->Draw("SAME");
    func_tweedie->Draw("SAME");
    // th1d->Sa
    double frac=func_ser->GetParameter(1);
    double sigma_frac=func_ser->GetParError(1);
    double mu=func_ser->GetParameter(2);
    double sigma_mu=func_ser->GetParError(2);
    double lammu=func_ser->GetParameter(4)*func_ser->GetParameter(5);
    double sigma_lammu=std::sqrt(func_ser->GetParameter(4)*func_ser->GetParameter(4)*func_ser->GetParError(5)*func_ser->GetParError(5)+func_ser->GetParameter(5)*func_ser->GetParameter(5)*func_ser->GetParError(4)*func_ser->GetParError(4));

    double GainEff=frac*mu+(1-frac)*lammu;
    double Sigma_GainEff=std::sqrt(sigma_frac*sigma_frac*(mu-lammu)*(mu-lammu)+frac*frac*sigma_mu*sigma_mu+(1-frac)*(1-frac)*sigma_lammu*sigma_lammu);
    double Reso=func_ser->GetParameter(3)/func_ser->GetParameter(2);
    {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.035);
        double chi2 = func_ser->GetChisquare();
        int ndf = (int)func_ser->GetNDF();
        double redchi = (ndf>0) ? chi2/ndf : 0;
        latex.DrawLatex(0.35, 0.78, Form("GainEff = %.5g #pm %.5g", GainEff, Sigma_GainEff));
        latex.DrawLatex(0.35, 0.73, Form("frac = %.4f ", frac));
        latex.DrawLatex(0.35, 0.68, Form("mu = %.3f #pm %.3f", mu, sigma_mu));
        latex.DrawLatex(0.35, 0.63, Form("sigma = %.4f", func_ser->GetParameter(3)));
        latex.DrawLatex(0.35, 0.58, Form("lambda = %.4f", func_ser->GetParameter(4)));
        latex.DrawLatex(0.35, 0.53, Form("mu_lambda = %.4f", func_ser->GetParameter(5)));
        latex.DrawLatex(0.35, 0.48, Form("#chi^{2}/ndf = %.2f / %d (%.2f)", chi2, ndf, redchi));
    }
    if (mu<func_ser->GetParameter(5)){
        cout <<"Warning: mu:"<<mu<<" less than mu_ts:"<<func_ser->GetParameter(5)<<endl;
    }
    if (func_ser->GetParameter(4)<2){
        cout <<"Warning: lambda:"<<func_ser->GetParameter(4)<<" < 2."<<endl;
    }
    // vector<double> para;
    para.push_back(GainEff);
    para.push_back(Sigma_GainEff);
    para.push_back(Reso);
    para.push_back(mu);
    para.push_back(sigma_mu);
    para.push_back(func_ser->GetChisquare());
    para.push_back(func_ser->GetNDF());
    // cout<<"Effective Gain:"<<endl;
    // cout<<GainEff<<endl;
    // printf("1\n");
    // f->Delete();
    // th1d->Delete();
    // func_ser->Delete();
    //   printf("2\n");
    // c1->SetLogy();
    c1->SaveAs(Form("../output/fit_PhaseI/fit%02dset1.pdf",ch));
    return para;
}


vector<double> GaussFit(const char * FilePath, const char * HistogramName="") 
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


    c1->SaveAs(Form("../output/fitGauss/fit%s.pdf",HistogramName));
    return para;
}

std::vector<double> getapprogain(const char* FilePath, int ch, const char* HistogramName){
    TFile*f=new TFile(FilePath,"read");
    TH1D*hist=(TH1D*)f->Get(HistogramName);
    if (!hist){
        printf("error: Histogram not found\n");
            std::vector<double> para;
        para.push_back(0);
        return para;
    }
    double sum = 0.0;
    int count = 0;

    Int_t originalFirstBin = hist->GetXaxis()->GetFirst();
    Int_t originalLastBin = hist->GetXaxis()->GetLast();
    hist->GetXaxis()->SetRangeUser(40.0, 1000.0);
    Int_t maxBin = hist->GetMaximumBin();
    Double_t maxBinCenter = hist->GetXaxis()->GetBinCenter(maxBin);
    hist->GetXaxis()->SetRange(originalFirstBin, originalLastBin);

    for (int i = 0; i <= hist->GetNbinsX(); i++) {
        double binCenter = hist->GetBinCenter(i);
        if (binCenter >= 40 && binCenter <= maxBinCenter*5) {
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

int GetGain(const char* inputpath, const char* outputpath){
    ofstream fout(outputpath);
    if (!fout) {
        std::cerr << "failed opening outputfile." << std::endl;
        return 1;
    }
    fout << "channel gain chi2 NDF" << endl;
    // for (int i=0; i<60; i++){
    //     vector<double> para=GainFit(Form("%s/ch%02d.root",inputpath,i),i,"set1");
    //     fout<<i<<" "<<para[0]<<" "<<para[5] <<" "<<para[6]<<endl;
    // }

    for (int i=0; i<60; i++){
        vector<double> para=getapprogain(Form("%s/ch%02d.root",inputpath,i),i,"set0");
        fout<<i<<" "<<para[0]<<endl;
    }
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
    GetGain(filepath,outputpath);
    return 0;
}