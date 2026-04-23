#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <TChain.h>
#include "ctools.h"

using namespace std;

// vector<int> badchannellist = {2,5,10,11,18,26,29,34,40,42,50,51,53,58};
// // vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
int basechannel = 23;
std::vector<int> *BadChannelList = &badchannellist;
const char* GaliListFile = "../data/GainList.txt"; // 增益列表文件路径

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}

float Getrmax(vector<PeakInfo> &peaks) {
    float maxcharge = 0;
    float totalcharge = 0;
    for (const auto& peak : peaks) {
        if (peak.charge > maxcharge) {
            maxcharge = peak.charge;
        }
        totalcharge += peak.charge;
    }
    return maxcharge/totalcharge; // 返回最大charge与总charge的比值
}

float deltaTime(vector<PeakInfo> &peaks) {
    if (peaks.size() < 2) {
        return 0; // 如果峰值数量少于2，返回0
    }
    
    float firstTime = 900;
    float secondTime = 900;
    for (const auto& peak : peaks) {
        if (peak.risetime < firstTime) {
            secondTime = firstTime; // 更新第二个时间
            firstTime = peak.risetime; // 更新第一个时间
        } else if (peak.risetime < secondTime) {
            secondTime = peak.risetime; // 更新第二个时间
        }
    }
    return secondTime - firstTime; // 返回最后一个峰与第一个峰的时间差
}

float TotalCharge(vector<PeakInfo> &peaks) {
    float totalCharge = 0;
    for (const auto& peak : peaks) {
        totalCharge += peak.charge;
    }
    return totalCharge; // 返回总电荷
}

void analyze_darknoise(const char* filename, const char* outputpath, const char* histfile) {
    TChain * tree = new TChain("PeakData");
    tree->Add(filename);
    if (!tree) {
        cerr << "Error: Cannot find tree 'PeakData'" << endl;
        return;
    }

    // 设置分支地址
    Int_t Sec, NanoSec;
    Int_t runNo = 0,triggerNo = 0;
    double ReconR;
    vector<PeakInfo>* peaks = nullptr;


    tree->SetBranchAddress("RunNo", &runNo);
    tree->SetBranchAddress("TriggerNo", &triggerNo);
    tree->SetBranchAddress("Sec", &Sec);
    tree->SetBranchAddress("NanoSec", &NanoSec);
    tree->SetBranchAddress("darknoise", &peaks);
    tree->SetBranchAddress("ReconR",&ReconR);

    Int_t timebegin,timeend;
    tree->GetEntry(0);
    timebegin = Sec;
    timeend = Sec;
    // // Int_t triggerbegin=10000000,triggerend=0;
    // 创建直方图
    TH2D* hRisetime = new TH2D("hRisetime", "relative Rise Time;Channel;relative Rise Time (ns)",
                               60, 0, 60, 400, -50, 50);

    // 为每个通道创建一维直方图
    vector<TH1D*> histsPerChannel;
    for (int ch = 0; ch < 60; ++ch) {
        histsPerChannel.push_back(new TH1D(Form("hRise_Ch%d", ch), 
                                    Form("Rise Time Channel %d;relative Rise Time (ns);Counts", ch),
                                    400, -50, 50));
    }

    vector<float> gain; //read gainlist
    if (Read_GainList(GaliListFile, gain, 60) != 0) {
        cerr << "Error: Cannot read gain list from " << GaliListFile << endl;
        return;
    }

    // 循环所有条目
    Long64_t nEntries = tree->GetEntries();
    cout << "Processing " << nEntries << " events..." << endl;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        if (timebegin > Sec) {
            timebegin = Sec; // 更新开始时间
        }
        if (timeend < Sec) {
            timeend = Sec; // 更新结束时间
        }
        if (peaks == nullptr || peaks->empty()) {
            continue; // 跳过空的峰值数据
        }
        // if (ReconR>0.25) continue;
        if (peaks->size() >50 || peaks->size() < 3) {
            continue; // 跳过高能触发事例和低能触发事例
        }
        float totalCharge = TotalCharge(*peaks);
        if (totalCharge > 1000000) {
            continue; // 跳过高能事例
        }
        // if (deltaTime(*peaks) > 10) {
        //     continue; // 跳过时间间隔小于0.5ns的事例
        // }
        if (Getrmax(*peaks) > 0.25) {
            continue; // 跳过最大电荷与总电荷比值大于0.3的事例
        }
        // 处理每个峰值
        double risetime_0=-1;
        for (size_t j = 0; j < peaks->size(); j++) {
            int channel = peaks->at(j).channel;
            if ((channel==basechannel)&& (peaks->at(j).charge/gain[channel]>1)){
                risetime_0 = peaks->at(j).risetime;
                break;
            }
        }
        if (risetime_0 < 0) continue;
        for (size_t j = 0; j < peaks->size(); j++) {
            int channel = peaks->at(j).channel;
            if (IsBadChannel(channel, BadChannelList)) {
                continue; // 跳过坏通道
            }
            double risetime = peaks->at(j).risetime;
            
            if (channel >= 0 && channel < 60 && peaks->at(j).charge/gain[channel]>1) {
                hRisetime->Fill(channel, risetime-risetime_0);
                histsPerChannel[channel]->Fill(risetime-risetime_0);
                if (channel==27){
                    cout << "run:"<<runNo<<", TriggerNo:"<<triggerNo<<", rte:"<< risetime - risetime_0<<endl;
                }
            }
        }
        
        // 每10000个事件输出进度
        if (i % 10000 == 0) {
            cout << "Processed " << i << "/" << nEntries << " events" << endl;
        }
    }

                    // 创建文本文件输出
    ofstream txtFile(outputpath);
    if (!txtFile.is_open()) {
        cerr << "Error: Cannot open text file for writing" << endl;
        return;
    }
    
    // 写入文件头
    txtFile << "# run" <<runNo <<" beginning time:"<< timebegin << " end time:" << timeend << endl;
    txtFile << "# Channel\tMean (ns)\tMean Error (ns)\tSigma (ns)\tSigma Error (ns)\tEntries\n";
    txtFile << "# -----------------------------------------------------------------------\n";  

    // 创建ROOT文件中的拟合结果树
    TTree* fitResults = new TTree("FitResults", "Gaussian Fit Results");
    int channel;
    double mean, mean_err, sigma, sigma_err;
    int entries;
    fitResults->Branch("channel", &channel);
    fitResults->Branch("mean", &mean);
    fitResults->Branch("mean_err", &mean_err);
    fitResults->Branch("sigma", &sigma);
    fitResults->Branch("sigma_err", &sigma_err);
    fitResults->Branch("entries", &entries);

    // 对每个通道进行高斯拟合
    TCanvas* cFit = new TCanvas("cFit", "Fitting Results", 1200, 800);
    cFit->Divide(6, 10); // 60个通道分页显示 (6x10)

    for (int ch = 0; ch < 60; ++ch) {
        channel = ch;
        entries = histsPerChannel[ch]->GetEntries();
        mean = 0.0;
        mean_err = 0.0;
        sigma = 0.0;
        sigma_err = 0.0;
        
        cFit->cd(ch + 1);
        TH1D* hist = histsPerChannel[ch];
        
        if (entries > 50) { // 确保有足够的数据点
            double Mean = hist->GetMean();
            TF1* gausFit = new TF1(Form("gausFit_Ch%d", ch), "gaus", -50, 50);
            hist->Fit(gausFit, "LS RQ","",Mean - 20,Mean + 20); // R: 使用直方图范围, Q: 不输出拟合结果 LS:least squares
            
            // 获取拟合参数
            mean = gausFit->GetParameter(1);
            mean_err = gausFit->GetParError(1);
            sigma = gausFit->GetParameter(2);
            sigma_err = gausFit->GetParError(2);
            // mean = hist->GetMean();
            // mean_err = hist->GetStd()/TMath::Sqrt(hist->Integral());
            // sigma = hist->GetStd();
            // sigma_err = 0;
            if (channel== basechannel){
                mean=0;
                mean_err=0;
                sigma=0;
                sigma_err=0;
            }
            // 绘制直方图和拟合曲线
            hist->Draw();
            gausFit->Draw("same");
        } else {
            // 数据不足，不进行拟合
            hist->Draw();
        }
        
  
        // 写入文本文件
        txtFile << channel << "\t"
                << mean << "\t"
                << mean_err << "\t"
                << sigma << "\t"
                << sigma_err << "\t"
                << entries << "\n";
        
        // 填充ROOT树
        fitResults->Fill();
    }
    
    // 关闭文本文件
    txtFile.close();
    cout << "Fit results saved to risetime_fit_results.txt" << endl;

    // 创建并填充趋势图
    TGraphErrors* gMean = new TGraphErrors();
    TGraphErrors* gSigma = new TGraphErrors();
    gMean->SetName("gMeanTrend");
    gMean->SetTitle("Mean Rise Time per Channel;Channel;Mean (ns)");
    gSigma->SetName("gSigmaTrend");
    gSigma->SetTitle("Sigma per Channel;Channel;Sigma (ns)");

    for (int ch = 0; ch < 60; ++ch) {
        if (histsPerChannel[ch]->GetEntries() > 50) {
            gMean->SetPoint(ch, ch, mean);
            gMean->SetPointError(ch, 0.5, mean_err);
            gSigma->SetPoint(ch, ch, sigma);
            gSigma->SetPointError(ch, 0.5, sigma_err);
        }
    }

    // 保存结果到ROOT文件
    TFile* outFile = new TFile(histfile, "RECREATE");
    hRisetime->Write();
    fitResults->Write();
    gMean->Write();
    gSigma->Write();
    
    // 保存每个通道的直方图
    for (auto& hist : histsPerChannel) {
        hist->Write();
    }
    
    // 保存画布
    cFit->Write();
    outFile->Close();

    // 清理
    // file->Close();
    // delete file;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input_file.root> <output_file.txt> <hist_file.root>" << endl;
        return 1;
    }
    
    analyze_darknoise(argv[1], argv[2], argv[3]);
    return 0;
}