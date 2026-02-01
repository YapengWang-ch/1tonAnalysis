// convert data from "SimpleAnalysis" to "MuonAnalysis"
#include "TTree.h"
#include "TFile.h"
#include "TChain.h"
#include "ChannelInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
int NChannels = 60;
const char* timeOffsetFile = "../data/TimeOffset.txt";
const char* MuonListFile = "../data/MuonList.txt";
const char* GainListFile="../data/GainList.txt";
const char* ClockShiftFile="../data/boardfix.txt";
const int BoardId[60]={0,1,2,3,4,6,2,2,1,2,
    0,5,4,5,6,7,0,1,5,3,
    4,5,6,7,0,1,3,3,4,2,
    6,7,0,1,5,3,4,5,6,7,
    3,1,4,3,4,5,6,7,0,1,
    7,5,6,0,4,7,0,1,2,3};
// int runedge=43784;
struct MuonIndex_t{
    int RunNo;
    int FileNo;
    int TriggerNo;
    int nLightedPMT;
    double TotalPE;
    double rmax;
};

// read muon list 
int ReadMuonList(vector<MuonIndex_t> &MuonIndex) {
    ifstream infile(MuonListFile);
    if (!infile.is_open()) {
        cout << "Error opening file: " << MuonListFile << endl;
        return 1;
    }
    string line;
    MuonIndex_t muonIndex;
    getline(infile, line);//跳过第一行
    while (getline(infile, line)) {
        istringstream iss(line);
        MuonIndex_t muonIndex;
        // int nlightedPMT;  // 临时变量读取第五列（跳过）
        int temp;
        if (iss >> muonIndex.RunNo >> muonIndex.FileNo >> muonIndex.TriggerNo >> temp >> temp
                >> muonIndex.TotalPE >> muonIndex.nLightedPMT >> muonIndex.rmax) {
                    if (muonIndex.nLightedPMT>60){
                        cout <<"Warning: nLightedPMT="<< muonIndex.nLightedPMT<<" >60, set to 60."<<endl;
                        muonIndex.nLightedPMT=60;
                    }
                    if (muonIndex.rmax < 0 || muonIndex.rmax > 1){ 
                        cout << "Warning: rmax " << muonIndex.rmax << " out of range [0,1], set to 0." << endl;
                        muonIndex.rmax = 0;
                    }
            MuonIndex.push_back(muonIndex);
        } else {
            cout << "Error parsing line: " << line << endl;
            // return 1;
        }
    }
    infile.close();
    return 0;
}

void PrintVector(vector<int> a){
    for (auto &value : a){
        cout <<value<<",";
    }
    cout<<"\b"<<endl;
}

// read time offset , the third colum of file
int ReadTimeOffset(double *timeOffset1,double *timeOffset2) {
    std::ifstream fin(timeOffsetFile);
    if (!fin.is_open()) {
        std::cerr << "Error opening TimeOffsetFile " << timeOffsetFile << std::endl;
        return 1;
    }
    for(int i=0; i<NChannels; i++){
        timeOffset1[i]=0;
    }
    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        int index;
        double col2, col3, target_col;
        if (iss >> index >> col2 >> col3){
            timeOffset1[index]=col2;
            timeOffset2[index]=col2;
        } 
    }
    return 0;
}

int ClockShiftFix(int runNo, double *timeOffset, double *timeOffsetFixed){
    std::ifstream fin(ClockShiftFile);
    if (!fin.is_open()) {
        std::cerr << "Error opening ClockShiftFile " << ClockShiftFile << std::endl;
        return 1;
    }
    for(int i=0; i<NChannels; i++){
        timeOffsetFixed[i]=timeOffset[i];
    }
    double BoardShift[8]={0,0,0,0,0,0,0,0};
    std::string line;
    bool found=false;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        int run, boardId;
        double shift;
        if (iss >> run >> BoardShift[0] >> BoardShift[1] >> BoardShift[2] >> BoardShift[3]
                >> BoardShift[4] >> BoardShift[5] >> BoardShift[6] >> BoardShift[7]){
            if (run == runNo){
                for (int i=0; i<NChannels; i++){
                    timeOffsetFixed[i]=timeOffset[i]+BoardShift[BoardId[i]];
                }
                found=true;
            }
        } 
    }
    if (!found){
        cout <<"Error: runNo "<<runNo<<" not found in ClockShiftFile "<<ClockShiftFile<<endl;
        return 1;
    }
    return 0;
}

int ReadGainList(const char* GainListFile, vector<double> &GainList1) {
    std::ifstream fin(GainListFile);
    if (!fin.is_open()) {
        std::cerr << "Error opening GainListFile " << GainListFile << std::endl;
        return 1;
    }
    GainList1.resize(NChannels, 1.0);
    std::string line;
    while (std::getline(fin, line)) {
        std::istringstream iss(line);
        int index;
        double gain1, gain2;
        if (iss >> index >> gain1) {
            if (index >= 0 && index < NChannels) {
                GainList1[index] = gain1;
            }
        }
    }
    for (int i=0; i<NChannels; i++){
        if (GainList1[i]<=1) {
            cout <<"Warning: GainList1["<<i<<"]="<<GainList1[i]<<endl;
            GainList1[i]=150.0;
        }
    }
    return 0;
}
// check bad channel
bool IsBadChannel(int channelId, int *BadChannelId, int BadChannelNum) {
    for (int i = 0; i < BadChannelNum; i++) {
        if (channelId == BadChannelId[i]) {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "Usage: ./PreAnalysisData inputFileDirectory outputfilepath" << endl;
        return -1;
    }

    const char* inputFileDirectory = argv[1];
    const char* outputfilepath = argv[2];

    // read muon list
    vector<MuonIndex_t> MuonIndex;
    if (ReadMuonList(MuonIndex) != 0) {
        cout << "Error reading muon list." << endl;
        return -1;
    }
    int nMuons = MuonIndex.size();
    cout << "Muon list read successfully. totally "<< nMuons<< " muons." << endl;

    // read time offset
    double timeOffset[NChannels],timeOffset2[NChannels],timeOffsetFixed[NChannels];
    if (ReadTimeOffset(timeOffset,timeOffset2) != 0) {
        cout << "Error reading time offset." << endl;
        return -1;
    }

    // create output file
    TFile *fout = new TFile(outputfilepath, "recreate");
    TTree *tout = new TTree("MuonAnalysis", "MuonAnalysis");
    double ReconE, PEmax2Sum, TimeRange;
    int RunNo, FileNo, TriggerNo,BadChannelNum=4,nLightedPMT;
    std::vector<int> ChannelId; 
    int BadChannelId[NChannels];
    double Time[NChannels];
    double PE[NChannels];

    // fill BadChannelId
    for (int i = 0; i < NChannels; i++) {
        BadChannelId[i] = -1;
    }
    BadChannelId[0] = 26;
    BadChannelId[1] = 29;
    BadChannelId[2] = 38;
    BadChannelId[3] = 54;

    // vector<int> ClockShiftEvents ={4,8,11,14};
    
    // set branch addresses
    tout->Branch("RunNo", &RunNo);
    tout->Branch("FileNo", &FileNo);
    tout->Branch("TriggerNo", &TriggerNo);
    tout->Branch("nLightedPMT",&nLightedPMT);
    tout->Branch("ReconE", &ReconE);
    tout->Branch("PEmax2Sum", &PEmax2Sum);
    tout->Branch("Time", Time,"Time[60]/D");
    tout->Branch("PE", PE, "PE[60]/D");
    tout->Branch("DelayTime", timeOffsetFixed, "DelayTime[60]/D");
    tout->Branch("BadChannelNum", &BadChannelNum);
    tout->Branch("BadChannelId", BadChannelId, "BadChannelId[60]/I");
    tout->Branch("ChannelId",&ChannelId);

    vector<double> GainList1;
    if (ReadGainList(GainListFile, GainList1) != 0) {
        cout << "Error reading gain list." << endl;
        return -1;
    }

    for (int i = 0; i<nMuons; i++){
        MuonIndex_t Muon = MuonIndex[i];
        if (Muon.nLightedPMT<56) continue; // pre-cut
        const char* inputFileName = Form("%s/run%08d_%d.root", inputFileDirectory, Muon.RunNo, Muon.FileNo);
        cout << "Processing file: " << inputFileName << endl;

        TChain *tin = new TChain("SimpleAnalysis");
        tin->Add(inputFileName);
        int nEntries = tin->GetEntries();
        if (nEntries == 0) {
            cout << "No entries in file: " << inputFileName << endl;
            continue;
        }

        vector<ChannelInfo_t>* ChannelInfo = nullptr;
        // int TriggerNo;
        tin->SetBranchAddress("ChannelInfo", &ChannelInfo);
        tin->SetBranchAddress("TriggerNo", &TriggerNo);
    
        for (int j = 0; j < nEntries; j++) {
            tin->GetEntry(j);
            if (TriggerNo == Muon.TriggerNo) break;
        }

        if (TriggerNo != Muon.TriggerNo){
            cout << "TriggerNo "<<Muon.TriggerNo<<" not found in file: " << inputFileName << endl;
            continue;
        }
        if (ChannelInfo == nullptr || ChannelInfo->empty()) {
            cout << "Error: ChannelInfo is null." << endl;
            continue;
        }
        
        // if (Muon.RunNo < runedge) {
            for (int j = 0; j < ChannelInfo->size(); j++) {
                ChannelInfo->at(j).PE = ChannelInfo->at(j).Charge/GainList1[ChannelInfo->at(j).ChannelId];
            }
        // } else {
        //     for (int j = 0; j < ChannelInfo->size(); j++) {
        //         ChannelInfo->at(j).PE = ChannelInfo->at(j).Charge/GainList2[ChannelInfo->at(j).ChannelId];
        //     }
        // }
        
        for (int j = 0; j < NChannels; j++) {
            PE[j] = 0;
            Time[j] = 0;
        }
        ChannelId.resize(0);
        for (int j = 0; j < ChannelInfo->size(); j++) {
            if (IsBadChannel((*ChannelInfo)[j].ChannelId, BadChannelId, 9) || (*ChannelInfo)[j].PE<0) {
                ChannelInfo->erase(ChannelInfo->begin() + j);
                j--;
            }
        }
        
        for (int j = 0; j < ChannelInfo->size(); j++) {
            ChannelId.push_back((*ChannelInfo)[j].ChannelId);
            PE[(*ChannelInfo)[j].ChannelId] = (*ChannelInfo)[j].PE;
            Time[(*ChannelInfo)[j].ChannelId] = (*ChannelInfo)[j].RiseTime;
        }
        std::sort(ChannelId.begin(), ChannelId.end());

        // for (int k = 0; k < NChannels; k++) {
        //     timeOffset[k] = timeOffset1[k];
        // }

        if (ClockShiftFix(Muon.RunNo, timeOffset, timeOffsetFixed) != 0) {
            cout << "Error in ClockShiftFix for runNo " << Muon.RunNo << endl;
            return -1;
        }

        ReconE = Muon.TotalPE;
        PEmax2Sum = Muon.rmax;
        RunNo = Muon.RunNo;
        FileNo = Muon.FileNo;
        nLightedPMT=Muon.nLightedPMT;
        if (ChannelId.size()!=nLightedPMT){
            cout <<"Error valid channels! ChannelId:"<<ChannelId.size()<<" nLightedPMT:"<<nLightedPMT<<endl;
            cout << ChannelId.size()<<" channels:";
            PrintVector(ChannelId);
        }
        // TriggerNo = Muon.TriggerNo;
        tout->Fill();
    }
    tout->Write();
    fout->Close();
    return 0;
}