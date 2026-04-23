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
#include "../include/ctools.h"

using namespace std;
const char* PositionFile = "../data/PMTPosition.txt"; // PMT位置文件路径

// vector<int> badchannellist = {2,5,10,11,18,26,29,34,40,42,50,51,53,58};
// // vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
// int basechannel = 0;
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


void plotDirection() {
    // 打开数据文件
    const char* filename = "../output/Test/All_PER.root";
    TFile* file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }

    // 获取树
    TTree* tree = (TTree*)file->Get("PeakData");
    if (!tree) {
        cerr << "Error: Cannot find tree 'PeakData'" << endl;
        file->Close();
        return;
    }

    // 设置分支地址
    Int_t Sec, NanoSec;
    Int_t runNo = 0,triggerNo = 0;
    double ReconX,ReconY,ReconZ,ReconR;
    vector<PeakInfo>* peaks = nullptr;


    tree->SetBranchAddress("RunNo", &runNo);
    tree->SetBranchAddress("TriggerNo", &triggerNo);
    tree->SetBranchAddress("Sec", &Sec);
    tree->SetBranchAddress("NanoSec", &NanoSec);
    tree->SetBranchAddress("darknoise", &peaks);
    tree->SetBranchAddress("ReconR",&ReconR);
    tree->SetBranchAddress("ReconX",&ReconX);
    tree->SetBranchAddress("ReconY",&ReconY);
    tree->SetBranchAddress("ReconZ",&ReconZ);


    TH2F* directionAll=new TH2F("all","all",200,-180,180,200,-1,1);
    TH2F* directionReconR=new TH2F("ReconR","ReconR",200,-180,180,200,-1,1);
    directionAll->SetXTitle("#phi");
    directionAll->SetYTitle("#cos#theta");
    directionReconR->SetXTitle("#phi");
    directionReconR->SetYTitle("#cos#theta");

    Int_t timebegin,timeend;
    tree->GetEntry(0);
    timebegin = Sec;
    timeend = Sec;
    Long64_t nEntries = tree->GetEntries();


        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            if (timebegin > Sec) timebegin = Sec; // 更新开始时间
            if (timeend < Sec) timeend = Sec; // 更新结束时间
            if (peaks == nullptr || peaks->empty()) continue; // 跳过空的峰值数据
            if (peaks->size() >50 || peaks->size() < 5) continue; // 跳过高能触发事例和低能触发事例
            float totalCharge = TotalCharge(*peaks);
            if (totalCharge > 100000) continue; // 跳过高能事例
            if (deltaTime(*peaks) > 10) continue; // 跳过时间间隔小于0.5ns的事
            if (Getrmax(*peaks) > 0.3) continue; // 跳过最大电荷与总电荷比值大于0.3的事
            double phi= TMath::ATan2(ReconY,ReconX);
            while (phi>TMath::Pi()) phi-=2*TMath::Pi();
            while (phi<-1*TMath::Pi()) phi+=2*TMath::Pi();
            double costheta = ReconZ/ReconR;
            // if (costheta>0 && costheta<0.01 && phi>0.8 && phi<1.7){
            //     cout <<"run:"<<runNo<< " triggerNo:"<<triggerNo << " "<<ReconX<<" "<<ReconY<<" "<<ReconZ<<endl;
            //     for(PeakInfo peak: *peaks){
            //         cout << " "<<peak.channel<<"/"<<peak.charge;
            //     }
            //     cout <<endl;
            // }
            directionAll->Fill(phi*180/TMath::Pi(),costheta);

            if (ReconR>0.3) continue;
            directionReconR->Fill(phi*180/TMath::Pi(),costheta);
        }
    
        directionAll->SaveAs("../output/Test/directionall_MC5.root");
        
        directionReconR->SaveAs("../output/Test/directionReconR_MC5.root");
}

int main(){
    std::vector<TVector3> *PMTPosition = new std::vector<TVector3>;
    Read_PMTPosition(PositionFile,*PMTPosition,60); // 读取PMT位置
    for (int i=0;i<60;i++){
        cout <<"channel:" <<i <<" "<<PMTPosition->at(i).X()<<" "<<PMTPosition->at(i).Y()<<" "<<PMTPosition->at(i).Z()<<endl;
    }
    plotDirection();
    return 0;
}