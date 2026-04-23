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

using namespace std;

// vector<int> badchannellist = {2,5,10,11,18,26,29,34,40,42,50,51,53,58};
// // vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
// int basechannel = 0;
const char* GaliListFile = "../data/GainList.txt"; // 增益列表文件路径

std::vector<int> *BadChannelList = &badchannellist;
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

float Qlog(float q){
    if (q<10) return q*0.1;
    return 2*log10(q*0.1)+1;
}

void TQmap(double refcharge[2],int refch, int plotch, const char* filename, const char* outputpath) {
    // // 打开数据文件
    // TFile* file = TFile::Open(filename);
    // if (!file || file->IsZombie()) {
    //     cerr << "Error: Cannot open file " << filename << endl;
    //     return;
    // }

    // // 获取树
    // TTree* tree = (TTree*)file->Get("PeakData");
    // if (!tree) {
    //     cerr << "Error: Cannot find tree 'PeakData'" << endl;
    //     file->Close();
    //     return;
    // }

    TChain * tree = new TChain("PeakData");
    tree->Add(filename);
    if (!tree) {
        cerr << "Error: Cannot find tree 'PeakData'" << endl;
        return;
    }

    if (IsBadChannel(refch,BadChannelList)) {
        cout << "Error: Reference channel " << refch << " is a bad channel." << endl;
        // file->Close();
        return;
    }

    if (IsBadChannel(plotch,BadChannelList)) {
        cout << "Error: Plot channel " << plotch << " is a bad channel." << endl;
        // file->Close();
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
    Long64_t nEntries = tree->GetEntries();

    TH2D* hTQmap = new TH2D("hTQmap", "TQmap", 250, 0, 5, 200, -50, 50);
    hTQmap->SetXTitle("Q");
    hTQmap->SetYTitle("RiseTimeError [ns]");
    TH1D* hleftPeak=new TH1D("hLeftPeak","hLeftPeak",1000,timebegin,timeend);

    TGraphErrors* gMean=new TGraphErrors();
    gMean->SetName("gMean");
    gMean->SetTitle("Mean Rise Time Error per Channel;Q;Mean Rise Time Error (ns)");
    gMean->SetMarkerStyle(20);

    vector<float> gain;
    if (Read_GainList(GaliListFile, gain, 60) != 0) {
        cerr << "Error: Cannot read gain list from " << GaliListFile << endl;
        return;
    }

    for (Long64_t i = 0; i < nEntries; i++) {
        if (i%(nEntries/100)==0) {
            cout << "Processing entry " << i << " / " << nEntries << endl;
        }
            tree->GetEntry(i);
            // if (timebegin > Sec) timebegin = Sec; // 更新开始时间
            // if (timeend < Sec) timeend = Sec; // 更新结束时间
            if (peaks == nullptr || peaks->empty()) continue; // 跳过空的峰值数据
            // if (ReconR>0.3) continue;
            if (peaks->size() >50 || peaks->size() < 5) continue; // 跳过高能触发事例和低能触发事例
            float totalCharge = TotalCharge(*peaks);
            // if (totalCharge > 100000) continue; // 跳过高能事例
            // if (deltaTime(*peaks) > 10) continue; // 跳过时间间隔小于0.5ns的事
            if (Getrmax(*peaks) > 0.3) continue; // 跳过最大电荷与总电荷比值大于0.3的事

            double risetime_ref=-1;
            double charge_ref=0;
            for (size_t j = 0; j < peaks->size(); j++) {
                int channel = peaks->at(j).channel;
                double Q_PE= Qlog(peaks->at(j).charge/gain[channel]);
                if ((channel==refch) && (Q_PE>refcharge[0])&& (Q_PE<refcharge[1])){
                    risetime_ref = peaks->at(j).risetime;
                    charge_ref = Q_PE;
                    break;
                }
            }
            if (risetime_ref < 0) continue;

            for (size_t j = 0; j < peaks->size(); j++) {
                int channel = peaks->at(j).channel;
                if (channel != plotch) {
                    continue; // 跳过坏通道
                }
                // if (peaks->at(j).charge<0.5*charge_ref || peaks->at(j).charge>2*charge_ref)
                //     continue; // 取电荷相近的事例
                double QQ= Qlog(peaks->at(j).charge/gain[channel]);
                hTQmap->Fill(QQ, peaks->at(j).risetime - risetime_ref);  
                hleftPeak->Fill(Sec);      
                // if ((peaks->at(j).risetime - risetime_ref<2.5) && (peaks->at(j).risetime - risetime_ref > -5))   {
                    cout << "run:"<<runNo<<", TriggerNo:"<<triggerNo<<", rte:"<<peaks->at(j).risetime - risetime_ref<<endl;
                // }
            }
        }
    for (int ch = 0; ch < hTQmap->GetNbinsX(); ++ch) {
        TH1D* proj = hTQmap->ProjectionY("mean", ch+1, ch+1);
        int bin_min = proj->FindBin(-10);
        int bin_max = proj->FindBin(10);
        double sum = 0, sumw = 0;
        for (int b = bin_min; b <= bin_max; ++b) {
            double v = proj->GetBinContent(b);
            double x = proj->GetBinCenter(b);
            sum += v * x;
            sumw += v;
        }
        double mean = (sumw > 0) ? sum / sumw : 0;
        // 估算均值误差（标准误差）
        bin_min = proj->FindBin(mean - 5);
        bin_max = proj->FindBin(mean + 5);
        sum = 0;
        sumw = 0;
        for (int b = bin_min; b <= bin_max; ++b) {
            double v = proj->GetBinContent(b);
            double x = proj->GetBinCenter(b);
            sum += v * x;
            sumw += v;
        }
        mean = (sumw > 0) ? sum / sumw : 0;
        // 计算方差
        double var = 0;
        for (int b = bin_min; b <= bin_max; ++b) {
            double v = proj->GetBinContent(b);
            double x = proj->GetBinCenter(b);
            var += v * (x - mean) * (x - mean);
        }
        double mean_err = (sumw > 1) ? sqrt(var) / sumw : 0;

        if (mean_err > 0.0001) {
            gMean->SetPoint(ch, hTQmap->GetXaxis()->GetBinCenter(ch+1), mean);
            gMean->SetPointError(ch, 0.01, mean_err);
        }
    }
    
    TFile *outFile = new TFile(outputpath, "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        cerr << "Error: Cannot create output file " << outputpath << endl;
        // file->Close();
        return;
    }
    TCanvas *cFit = new TCanvas("cFit", "TQmap", 800, 600);
    hTQmap->Draw("COLZ");
    gMean->Draw("PE SAME");
    hleftPeak->Write();

    hTQmap->Write();
    gMean->Write();
    cFit->Write();
    outFile->Close();
    cout << "TQmap saved to " << outputpath << endl;
        // 创建文本文件输出
}

int main(int argc, char** argv) {
    if (argc < 5) {
        cerr << "Usage: " << argv[0] << " <input_file.root> <output_file.root> refch plotch" << endl;
        return 1;
    }
    double refch[2]={0,10};
    for(int i=1;i<argc-2;i++){
        if (strcmp(argv[i], "-r") == 0 && i + 2 < argc) {
            refch[0] = atof(argv[++i]);
            refch[1] = atof(argv[++i]);
        }
    }
    // TQmap(refch, argv[1], argv[2]);

    TQmap(refch,atoi(argv[3]), atoi(argv[4]), argv[1], argv[2]);
    return 0;
}