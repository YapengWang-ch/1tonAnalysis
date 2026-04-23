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
#include <sstream>
// #include <istream>
// #include <ostream>
#include "ctools.h"

using namespace std;
int ChannelN=60;
int maxiter = 100; // 最大迭代次数
int miniter = 10; //最小迭代次数
const char* timeOffsetFile = "../data/TimeOffset_Water.txt";

// vector<int> badchannellist = {2,5,10,11,18,26,29,34,40,42,50,51,53,58};
// // vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
// vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
    std::vector<int> badchannellist={26,29,38,54};

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

int ReadTimeOffset(double *timeOffset1) {
    std::ifstream fin(timeOffsetFile);
    if (!fin.is_open()) {
        std::cerr << "Error opening TimeOffsetFile " << timeOffsetFile << std::endl;
        return 1;
    }
    for(int i=0; i<ChannelN; i++){
        timeOffset1[i]=0;
    }
    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        int index;
        double col2, col3, target_col;
        if (iss >> index >> col2 >> col3 >> target_col && std::abs(target_col)>0){
            timeOffset1[index]=col2;
        }
    }
    return 0;
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

int tfilter(double time){
    float tstep[21]={-50,-40,-30,-20,-15,-10,-8,-6,-4,-2,0,2,4,6,8,10,15,20,30,40,50};
    int tcount=0;
    for (int i=0; i<21; i++)
        if (time>tstep[i]) tcount++;
    return tcount;
}

void analyze_darknoise(const char* filename, const char* outputpath, const char* histfile) {
    // 打开数据文件
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
    double ReconR;
    vector<PeakInfo>* peaks = nullptr;

    tree->SetBranchAddress("RunNo", &runNo);
    // tree->SetBranchAddress("triggerNo", &triggerNo);
    tree->SetBranchAddress("Sec", &Sec);
    tree->SetBranchAddress("NanoSec", &NanoSec);
    tree->SetBranchAddress("ReconR", &ReconR);
    tree->SetBranchAddress("darknoise", &peaks);

    TFile *histFile = TFile::Open(histfile, "RECREATE");
    if (!histFile || histFile->IsZombie()) {
        cerr << "Error: Cannot open histogram file " << histfile << endl;
        file->Close();
        return;
    }
    TTree* TreeIteration= new TTree("TreeIteration", "Tree Iteration");
    vector<int> channels;
    vector<double> risetimes; 
    TreeIteration->Branch("runNo", &runNo);
    TreeIteration->Branch("triggerNo", &triggerNo);
    TreeIteration->Branch("channels", &channels);
    TreeIteration->Branch("risetimes", &risetimes);

    Int_t timebegin,timeend;
    tree->GetEntry(0);
    timebegin = Sec;
    timeend = Sec;

    // 循环所有条目
    Long64_t nEntries = tree->GetEntries();
    cout << "Processing " << nEntries << " events..." << endl;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        channels.clear();
        risetimes.clear();
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
        if (ReconR > 0.3) {
            continue; // 跳过非中心事例
        }
        if (peaks->size() >30 || peaks->size() < 2) {
            continue; // 跳过高能触发事例和低能触发事例
        }
        float totalCharge = TotalCharge(*peaks);
        if (totalCharge > 100000 ) {
            continue; // 跳过高能事例
        }
        // if (deltaTime(*peaks) > 10) {
        //     continue; // 排除pulsetrigger
        // }
        if (Getrmax(*peaks) > 0.6) {
            continue; // 排除flasher
        }
        // 处理每个峰值
        for (size_t j = 0; j < peaks->size(); j++) {
            int channel = peaks->at(j).channel;
            if (IsBadChannel(channel, BadChannelList)) {
                continue; // 跳过坏通道
            }
            double risetime = peaks->at(j).risetime;
            if (risetime < 140 || risetime > 210) {
                continue; // 跳过不合理的rise time
            }
            if (peaks->at(j).charge < 200|| peaks->at(j).charge > 50000) {
                continue; // 跳过低能事例
            }
            channels.push_back(channel);
            risetimes.push_back(risetime);
        }
        if (channels.size() < 2) {
            continue; // 如果没有有效的通道，跳过
        }
        TreeIteration->Fill();  
        // 每100000个事件输出进度
        if (i % 100000 == 0) {
            cout << "Processed " << i << "/" << nEntries << " events" << endl;
        }
    }
    
    long totalEntries = TreeIteration->GetEntries();
    cout << "Total valid entries: " << totalEntries << endl;
    std::vector<double> TimeShift;
    TimeShift.resize(60, 0); // 初始化时间偏移数组
    double TimeOffset0[60];
    ReadTimeOffset(TimeOffset0);
    for (int i=0; i<60; i++){
        TimeShift[i]=TimeOffset0[i];
    }

    std::vector<double> Timetag;
    Timetag.resize(totalEntries, 0); // 初始化时间标签数组

    // 计算每个通道的统计量
    std::vector<int> channelCounts(60, 0);
        for(int ientry=0; ientry < totalEntries; ientry++){
            TreeIteration->GetEntry(ientry);
            for (size_t j = 0; j < channels.size(); j++) {
                int channel = channels[j];
                channelCounts[channel]++;
            }
        }
    
    //开始迭代
    for (int iter = 0; iter < maxiter; iter++){
        cout << "----- Iteration I" << iter + 1 <<" beginning -----" <<endl;

        for(int ientry=0; ientry < totalEntries; ientry++){
            risetimes.clear();
            channels.clear();
            TreeIteration->GetEntry(ientry);
            double sumtime = 0;
            for (int j = 0; j < channels.size(); j++) {
                int channel = channels[j];
                sumtime += risetimes[j] - TimeShift[channel]; 
            }
            Timetag[ientry] = sumtime / risetimes.size(); // 计算平均rise time
        }
        std::vector<double> TimeShiftNew(60, 0); // 新的时间偏移数组
        int countmatrix[60][22];
        for (int i=0; i<60; i++){
            for (int j=0; j<22; j++){
                countmatrix[i][j]=0;
            }
        }

        int channelcount1[60];
        for (int i=0; i<60; i++){
            channelcount1[i]=0;
        }
        for(int ientry=0; ientry < totalEntries; ientry++){
            TreeIteration->GetEntry(ientry);
            if (channels.size()<2) continue;
            for (size_t j = 0; j < channels.size(); j++) {
                int channel = channels[j];
                double deltat = risetimes[j] - Timetag[ientry]; 
                // if (deltat-TimeShift[channel]>15 || deltat-TimeShift[channel]<-15) {
                //     // continue;
                //     channels.erase(channels.begin()+j);
                //     risetimes.erase(risetimes.begin()+j);
                //     j--;
                //     continue;
                // }
                    TimeShiftNew[channel] += (risetimes[j] - Timetag[ientry]);
                    countmatrix[channel][tfilter(risetimes[j]-Timetag[ientry])]++;
                    channelcount1[channel]++;

            }
        }
        for (int ch = 0; ch < 60; ++ch) {
            if (channelCounts[ch] > 0) {
                TimeShiftNew[ch] /= channelcount1[ch]; // 平均化时间偏移

                cout << "Channel " << ch << " new TimeShift: "<< TimeShiftNew[ch] << " ns, count: " << channelCounts[ch]<< ". ";
                for (int k=0; k<22; k++){
                    cout << countmatrix[ch][k] << ",";
                }
                cout << endl;
            }
        }
        if (iter<miniter) continue;
        double sumuncertainty = 0;
        for (int ch =0; ch<60; ch++){
            sumuncertainty += (TimeShiftNew[ch]-TimeShift[ch]) * (TimeShiftNew[ch] - TimeShift[ch]);
            TimeShift[ch] = TimeShiftNew[ch]; // 更新时间偏移

        }
        cout << "Iteration " << iter + 1 << " completed. Sum uncertainty: " << sumuncertainty << endl;
        if (sumuncertainty*totalEntries < 1e-5) { // 如果时间偏移变化小于0.01，认为收敛
            cout << "Converged after " << iter + 1 << " iterations." << endl;
            break;
        }
    }
    // TTree *TreeIterationNew = TreeIteration->CloneTree(0);
    // TreeIterationNew->Branch("runNo", &runNo);
    // TreeIterationNew->Branch("triggerNo", &triggerNo);  
    // TreeIterationNew->Branch("channels", &channels);
    // TreeIterationNew->Branch("risetimes", &risetimes);
    // // TreeIterationNew->Branch("Timetag", &Timetag);

    //     for(int ientry=0; ientry < TreeIteration->GetEntries(); ientry++){
    //         TreeIteration->GetEntry(ientry);
    //         if (channels.size()<2) continue;
    //         for (size_t j = 0; j < channels.size(); j++) {
    //             int channel = channels[j];
    //             double deltat = risetimes[j] - Timetag[ientry]; 
    //             if (deltat-TimeShift[channel]>10 || deltat-TimeShift[channel]<-10) {
    //                 // continue;
    //                 channels.erase(channels.begin()+j);
    //                 risetimes.erase(risetimes.begin()+j);
    //                 j--;
    //                 continue;
    //             }
    //                 // TimeShiftNew[channel] += (risetimes[j] - Timetag[ientry]);
    //                 // countmatrix[channel][tfilter(risetimes[j]-Timetag[ientry])]++;
    //                 // channelcount1[channel]++;

    //         }
    //         if (channels.size()<2) continue;
    //         TreeIterationNew->Fill();
    //     }
    // for (int iter = 0; iter < maxiter; iter++){
    //     cout << "----- Iteration II" << iter + 1 <<" beginning -----" <<endl;

    //     for(int ientry=0; ientry < TreeIterationNew->GetEntries(); ientry++){
    //         risetimes.clear();
    //         channels.clear();
    //         TreeIterationNew->GetEntry(ientry);
    //         double sumtime = 0;
    //         for (int j = 0; j < channels.size(); j++) {
    //             int channel = channels[j];
    //             sumtime += risetimes[j] - TimeShift[channel]; 
    //         }
    //         Timetag[ientry] = sumtime / risetimes.size(); // 计算平均rise time
    //     }
    //     std::vector<double> TimeShiftNew(60, 0); // 新的时间偏移数组
    //     int countmatrix[60][22];
    //     for (int i=0; i<60; i++){
    //         for (int j=0; j<22; j++){
    //             countmatrix[i][j]=0;
    //         }
    //     }

    //     int channelcount1[60];
    //     for (int i=0; i<60; i++){
    //         channelcount1[i]=0;
    //     }
    //     for(int ientry=0; ientry < TreeIterationNew->GetEntries(); ientry++){
    //         TreeIteration->GetEntry(ientry);
    //         if (channels.size()<2) continue;
    //         for (size_t j = 0; j < channels.size(); j++) {
    //             int channel = channels[j];
    //             double deltat = risetimes[j] - Timetag[ientry]; 
    //             // if (deltat-TimeShift[channel]>15 || deltat-TimeShift[channel]<-15) {
    //             //     // continue;
    //             //     channels.erase(channels.begin()+j);
    //             //     risetimes.erase(risetimes.begin()+j);
    //             //     j--;
    //             //     continue;
    //             // }
    //                 TimeShiftNew[channel] += (risetimes[j] - Timetag[ientry]);
    //                 countmatrix[channel][tfilter(risetimes[j]-Timetag[ientry])]++;
    //                 channelcount1[channel]++;

    //         }
    //     }
    //     double meantime=0;
    //     double countch=0;
    //     for (int ch = 0; ch < 60; ++ch) {
    //         if (channelCounts[ch] > 0) {
    //             TimeShiftNew[ch] /= channelcount1[ch]; // 平均化时间偏移
    //             meantime += TimeShiftNew[ch];
    //             countch++;
    //             cout << "Channel " << ch << " new TimeShift: "<< TimeShiftNew[ch] << " ns, count: " << channelcount1[ch]<< ". ";
    //             for (int k=0; k<22; k++){
    //                 cout << countmatrix[ch][k] << ",";
    //             }
    //             cout << endl;
    //         }
    //     }
    //     meantime /= countch;
    //     double sumuncertainty = 0;
    //     for (int ch =0; ch<60; ch++){
    //         if (channelCounts[ch] > 0)
    //         TimeShiftNew[ch] -= meantime;
    //         sumuncertainty += (TimeShiftNew[ch]-TimeShift[ch]) * (TimeShiftNew[ch] - TimeShift[ch]);
    //         TimeShift[ch] = TimeShiftNew[ch]; // 更新时间偏移

    //     }
    //     cout << "Iteration " << iter + 1 << " completed. Sum uncertainty: " << sumuncertainty << endl;
    //     if (sumuncertainty*totalEntries < 1e-5) { // 如果时间偏移变化小于0.01，认为收敛
    //         cout << "Converged after " << iter + 1 << " iterations." << endl;
    //         break;
    //     }
    // }
    // 创建文本文件输出
    ofstream txtFile(outputpath);
    if (!txtFile.is_open()) {
        cerr << "Error: Cannot open text file for writing" << endl;
        return;
    }
    
    // 写入文件头
    txtFile << "# run" <<runNo <<" beginning time:"<< timebegin << " end time:" << timeend << endl;
    txtFile << "# Channel\t TimeShift (ns)\tEntries\n";
    txtFile << "# -----------------------------------------------------------------------\n";

    for (int ch = 0; ch < 60; ++ch) {
        txtFile << ch << "\t"
                << TimeShift[ch] << "\t"
                << channelCounts[ch] << endl;
    }
    cout << "Time shift results saved to " << outputpath << endl;

    TreeIteration->Write();
    histFile->Close();
    delete file;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input_file.root> <output_file.txt> <hist_file.root>" << endl;
        return 1;
    }
    
    analyze_darknoise(argv[1], argv[2], argv[3]);
    return 0;
}