#include <iostream>
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TFile.h"
#include "FindPeaksSG.h"
#include "TTreeReader.h"
#include "JPSimOutput.hh"
#include "TSystem.h"
#include "Utils/JPUtils.h"
#include "TVector3.h"
#include "ChannelInfo.h"
// #include "JPWaveformPreprocess.h"
// #include "JPWaveformAdvprocess.h"

using namespace std;
int NChannels=60;
const char * badchannelfile = "data/BadChannels.txt";

int ReadBadChannel(int runNo, int FileNo, std::vector<int>* badchannellist){  // 读取badchannellist
    badchannellist->push_back(26);
    badchannellist->push_back(29);
    badchannellist->push_back(38);
    badchannellist->push_back(54);
    return 1;
}

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}

// double GetTimesError(std::vector<ChannelInfo_t> &ChannelInfo){
//     double FirstRisetime=900;
//     double SecondRisetime=900;
//     for (const auto& channel : ChannelInfo) {
//         int npeak=channel.PeakLoc.size();
//         for(int i=0; i<npeak; i++){
//             // if (channel.PeakAmp[i]<50) continue;
//             if (channel.PeakLoc[i]  < FirstRisetime) {
//                 SecondRisetime = FirstRisetime;
//                 FirstRisetime = channel.PeakLoc[i]  ;
//             }else if (channel.PeakLoc[i]   < SecondRisetime){
//                 SecondRisetime = channel.PeakLoc[i]  ;
//             }
//             break;
//         }
//     }
//     return SecondRisetime-FirstRisetime; 
// }

double GetTimesError(std::vector<ChannelInfo_t> &ChannelInfo){
    double FirstRisetime=900;
    double SecondRisetime=900;
    vector<double> risetime;
    // double meanrisetime=0;
    int count=0;
    for (const auto& channel : ChannelInfo) {
        int npeak=channel.PeakLoc.size();
        double peakAmp=0,peakLoc=0;
        for (size_t i=0; i<npeak ; i++){
            // cout << "channel: "<<channel.ChannelId<<" RiseTime: "<<channel.RiseTime<<" PeakLoc: "<<channel.PeakLoc[i]<< " PeakAmp: "<<channel.PeakAmp[i]<< endl;
            if (channel.PeakAmp[i]>peakAmp) {
                peakLoc = channel.PeakLoc[i];
                peakAmp = channel.PeakAmp[i];
            }
        }
        risetime.push_back(peakLoc-channel.RiseTime);
        // count++;
    }
    auto max_it = std::max_element(risetime.begin(), risetime.end());
    if (max_it != risetime.end()) {
        risetime.erase(max_it);
    }
    if (risetime.empty()) return -1;
    max_it = std::max_element(risetime.begin(), risetime.end());
    if (max_it != risetime.end()) {
        risetime.erase(max_it);
    }
    if (risetime.empty()) return -1;

    double sum = 0;
    for (auto v : risetime) sum += v;
    return sum / risetime.size();
}

double GetEffPE(std::vector<ChannelInfo_t> &ChannelInfo){
    double EffPE=0;
    for (const auto& channel : ChannelInfo) {
        EffPE+=channel.PE;
    }
    return EffPE;
}

int ReadPosition(const char* filename, double **PMT_Position) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: failed to open file " << filename << std::endl;
        return 0;
    }
    int ChannelId;
    double x, y, z;
    while (file >> ChannelId >> x >> y >> z) {
        PMT_Position[ChannelId][0] = x;
        PMT_Position[ChannelId][1] = y;
        PMT_Position[ChannelId][2] = z;
    }
    file.close();
    return 1;
}

int PositionReCon(std::vector<ChannelInfo_t> ChannelInfo, double **PMT_Position, double *ReConX, double *ReConY, double *ReConZ) {
    if (ChannelInfo.empty()) {
        std::cerr << "Warning: ChannelInfo is empty!" << std::endl;
        return 0;
    }
    double sumPE = 0;
    double sumX = 0;
    double sumY = 0;
    double sumZ = 0;
    for (const auto& channel : ChannelInfo) {
        if (channel.ChannelId < 0 || channel.ChannelId >= NChannels) {
            std::cerr << "Warning: ChannelId " << channel.ChannelId << " is out of bounds!" << std::endl;
            continue;
        }
        sumPE += channel.PE;
        sumX += channel.PE * PMT_Position[channel.ChannelId][0];
        sumY += channel.PE * PMT_Position[channel.ChannelId][1];
        sumZ += channel.PE * PMT_Position[channel.ChannelId][2];
    }
    if (sumPE == 0) {
        std::cerr << "Warning: sumPE is zero!" << std::endl;
        return 0;
    }
    *ReConX = 1.5 * sumX / sumPE;
    *ReConY = 1.5 * sumY / sumPE;
    *ReConZ = 1.5 * sumZ / sumPE;
    return 1;
}

float getrmax(const std::vector<ChannelInfo_t>& ChannelInfo) {
    float maxPE = 0;
    float sumPE = 0;
    for (const auto& channel : ChannelInfo) {
        if (channel.PE > maxPE) {
            maxPE = channel.PE;
        }
        sumPE += channel.PE;
    }
    return sumPE > 0 ? maxPE / sumPE : 0; // 避免除以零
}

float getDistance(Double_t x, Double_t y, Double_t z, Double_t px, Double_t py, Double_t pz) {
    double lsquarecos=(x*px+y*py+z*pz)*(x*px+y*py+z*pz)/(px*px+py*py+pz*pz);
    double lsquare=(x*x+y*y+z*z);
    return sqrt(lsquare - lsquarecos); 
}

int main(int argc, char **argv)
{
    // gSystem->Load(gSystem->ExpandPathName("$JSAPSYS/Simulation/DataType/lib/libJPSIMOUTPUT.so"));

    
    TString inputFilename;
    TString outputfilename;
    if (argc == 3)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./PreAnalysisMC inputFilename outputFilename" << endl;
        cout << endl;
        return 1;
    }

    TChain *tc = new TChain("SimpleAnalysis");
    TChain *tmc = new TChain("MCTruth");

    //cout << "Adding files..." << inputFilename << endl;
    // Int_t nFlag = jputils::ReadRawDataRootFiles(inputFilename, tc);
    // Int_t nFlag1 = jputils::ReadRawDataRootFiles(inputFilename, tmc);
    tc->Add(inputFilename);
    tmc->Add(inputFilename);
    Int_t nFlag = tc->GetEntries();
    Int_t nFlag1 = tmc->GetEntries();

    if (nFlag == -1 || nFlag1 == -1)
    {
        cout << "PreAnalysis terminated." << endl;
        return 1;
    }
    cout << "Total " << nFlag << " waveform files added." << endl;
    cout << "Total " << nFlag1 << " MC truth files added." << endl;
    //cout << "Added " << nFlag << " files." << endl;

    // Int_t bl_begin = 0;
    // Int_t bl_end = 150; // [bl_begin, bl_end)
    // Int_t end_begin = 600;
    // Int_t end_end = 900; // [sideband_begin, sideband_end)
    // Int_t inte_begin = 150;
    // Int_t inte_end = 600; // [inte_begin, inte_end)

    std::vector<JPSimTruthTree_t> *truthList = new std::vector<JPSimTruthTree_t>;
    std::vector<JPSimPE_t> *PEList = new std::vector<JPSimPE_t>;
    Int_t RunNo, TriggerNo, TriggerTruth;
    // tmc->SetBranchAddress("RunNo", &RunNo);
    cout << "set tmc branch address" << endl;
    tmc->SetBranchAddress("TriggerNo", &TriggerTruth);
    tmc->SetBranchAddress("PEList", &PEList);
    tmc->SetBranchAddress("truthList", &truthList);

    TFile *file = new TFile(outputfilename, "recreate");

    TTree *ma = new TTree("ma", "ma");
    int FileNo, nChannels, nGlobal;
    std::vector<int> ChannelId;
    double TotalPE, PEmax2Sum, TimeRange;
    int nBeta, ncosAlpha, ncosTheta, nPhi;
    double cosAlpha, Beta, cosTheta, Phi;
    int ChannelIdList[60];
    double Time[60], FrontBslnStdDev[60];
    double Energy[60];
    double tError;
    double ReconR;
    double distance;
    // double EffPE;

    ma->Branch("RunNo", &RunNo);
    ma->Branch("FileNo", &FileNo);
    ma->Branch("TriggerNo", &TriggerNo);
    ma->Branch("ChannelId",&ChannelId);
    ma->Branch("TotalPE", &TotalPE);
    ma->Branch("PEmax2Sum", &PEmax2Sum);
    ma->Branch("TimeRange", &TimeRange);
    ma->Branch("cosTheta", &cosTheta);
    ma->Branch("Phi", &Phi);
    ma->Branch("cosAlpha", &cosAlpha);
    ma->Branch("Beta", &Beta);
    ma->Branch("nChannels",&nChannels);
    ma->Branch("ChannelIdList",ChannelIdList,"ChannelIdList[60]/I");
    ma->Branch("Time", Time, "Time[60]/D");
    ma->Branch("Energy", Energy, "Energy[60]/D");
    ma->Branch("tError", &tError);
    ma->Branch("ReconR", &ReconR);
    ma->Branch("distance", &distance);
  
    // For Makefile
    double evtPE;
    Int_t Sec;
    Int_t NanoSec;
    std::vector<ChannelInfo_t> *ChannelInfo = nullptr;
    cout << "set tc branch address" << endl;
    tc->SetBranchAddress("TotalPE", &evtPE);
    tc->SetBranchAddress("ChannelInfo", &ChannelInfo);
    tc->SetBranchAddress("Sec", &Sec);
    tc->SetBranchAddress("NanoSec", &NanoSec);
    tc->SetBranchAddress("RunNo", &RunNo);
    tc->SetBranchAddress("TriggerNo", &TriggerNo);

    long totalEntries = tc->GetEntries();
    long totalTruth= tmc->GetEntries();
    cout << "Total readout entries: " << totalEntries << endl;
    cout << "Total truth entries: " << totalTruth << endl;


    Int_t entry = 0;
    Int_t nProcessCheck = TMath::Min(100., totalEntries / 1.);
    if (nProcessCheck < 1)
        nProcessCheck = 1;

    int lastSec = 0;

    Int_t nFlag_ = 0;
    
    // read PMT position
    double PMT_Position[NChannels][3];
    double *PMT_Position_ptr[NChannels];
    for (int i = 0; i < NChannels; ++i) {
        PMT_Position_ptr[i] = PMT_Position[i];
    }
    if (ReadPosition("data/PMT_Position.txt", PMT_Position_ptr) == 0) {
        std::cerr << "Error: failed to read PMT position!" << std::endl;
        return 1;
    }
    // read bad channel
    vector<int> badchannellist={26,29,38,54};
    std::vector<int> *BadChannelList = &badchannellist;

    // badchannel broadcast
    cout << "Bad channels: ";
    for (const auto& ch : *BadChannelList) {
        cout << ch << " ";
    }
    cout << endl;

    double x,y,z;
    int itmc=0;
    int itc=0;
    while (itc<totalEntries && itmc<totalTruth)
    {
        if (itc % nProcessCheck == 0)
            cout << "Processing entry " << itc << " (" << itc / (double)totalEntries * 100 << "%)" << endl;
        
        tmc->GetEntry(itmc);
        tc->GetEntry(itc);
        itmc++;
        itc++;
        // entry++;
        if (ChannelInfo == nullptr || ChannelInfo->size() == 0 || ChannelInfo->size() > 60){
            cout << "ChannelInfo is empty or size is not correct!" << endl;
            continue;
        }
        if (PEList == nullptr || PEList->size() == 0){
            cout << "PEList is empty!" << endl;
            continue;
        }
        if (truthList == nullptr || truthList->size() == 0){
            cout << "truthList is empty!" << endl;
            continue;
        }

        if (TriggerNo != TriggerTruth){
            cout << "The trigger number is not matched!" << endl;
            cout << "MC truth TriggerNo: " << TriggerTruth << endl;
            cout << "Waveform TriggerNo: " << TriggerNo << endl;
            cout << endl;
            if (TriggerNo > TriggerTruth){
                itc--;
                continue;
            }else{
                itmc--;
                continue;
            }
        }
        // BadChannelList->clear();
        // ReadBadChannel(RunNo, FileNo, BadChannelList);
        // cout << "Before removing bad channels, ChannelInfo size: "<< ChannelInfo->size() << endl;
        for (int j=0; j<ChannelInfo->size(); j++){
            if (IsBadChannel((*ChannelInfo)[j].ChannelId,BadChannelList)){
                ChannelInfo->erase(ChannelInfo->begin()+j);
                j--;
            }
        }
        // cout << "After removing bad channels, ChannelInfo size: "<< ChannelInfo->size() << endl;
        ChannelId.resize(0);
        for (int j=0;j<ChannelInfo->size();j++){
            ChannelId.push_back((*ChannelInfo)[j].ChannelId);
        }
        if (ChannelInfo->size()!=ChannelId.size()){
            cout << "Error: ChannelId error!"<<endl;
            // for (int j=0;j<ChannelInfo->size();j++){
            //     cout << j<<": "<<(*ChannelInfo)[j].ChannelId <<endl;
            // }
            // cout << ""
        }
        nChannels = ChannelInfo->size();
        // if (nChannels < 49) continue; // ensure all channels are present
        //if (nChannels != 30)
          //  continue;
        if (Sec == lastSec)
           continue;
        nFlag_++;
        lastSec = Sec;

        // FileNo = jputils::GetFileNumber(tc->GetFile()->GetName());
        FileNo=0;
        vector<JPSimPrimaryParticle_t> PrimaryParticleList = (*truthList)[0].PrimaryParticleList;
        double x = (*truthList)[0].x;
        double y = (*truthList)[0].y;
        double z = (*truthList)[0].z;

        if (x*x+y*y+z*z<1400*1400+1900*1900){
            cout <<"Error: bad muon generate position: x:"<<x<<" y:"<<y<<" z:"<<z<<endl;
            continue;
        }

        TVector3 p(PrimaryParticleList[0].px, PrimaryParticleList[0].py, PrimaryParticleList[0].pz);
        double theta = p.Theta();
        double phi = p.Phi();

        double d=2800;  
        distance=getDistance(x,y,z,p.X(),p.Y(),p.Z());
        // if (distance>d*0.48){
        //     continue;
        // }

        double sq=d * d/4 - x * x - y * y - z * z + pow(z * cos(theta) + sin(theta) * (x * cos(phi) + y * sin(phi)), 2.);
        if (sq < 0) {
            // std::cerr << "Warning: negative square root encountered, skipping entry." << std::endl;
            continue;
        }
        Double_t t1 = -z * cos(theta) - sin(theta) * (x * cos(phi) + y * sin(phi)) - sqrt(sq);
        Double_t x1 = x + sin(theta) * cos(phi) * t1;
        Double_t y1 = y + sin(theta) * sin(phi) * t1;
        Double_t z1 = z + cos(theta) * t1;

        TVector3 incidentPoint(x1, y1, z1);
        cosAlpha = incidentPoint.CosTheta();
        Beta = incidentPoint.Phi();

        p = -p;
        cosTheta = p.CosTheta();
        Phi = p.Phi();
    
        double avg = 0;
        int nc = 0;
        // int windowSize = (*Waveform).size() / (*ChannelId).size();
       
        double peVect[60];
        for(int j = 0; j<60;j++)
        {
            peVect[j] = 0;
            Time[j] = 330;
            ChannelIdList[j] = 0;
        }
        // TotalPE = 0;
        TotalPE = evtPE;

        // vector<vector<decltype(Waveform->begin())>> pp;
        for (Int_t j = 0; j < nChannels; j++)
        {   
            UInt_t PMTId = ChannelInfo->at(j).ChannelId;
            ChannelIdList[PMTId] = 1;
            Energy[PMTId] = ChannelInfo->at(j).PE;
            Time[PMTId] = ChannelInfo->at(j).RiseTime;
        }


        TimeRange = 0;
        PEmax2Sum = getrmax(*ChannelInfo);
        tError = GetTimesError(*ChannelInfo);
        if (PositionReCon(*ChannelInfo, PMT_Position_ptr, &x, &y, &z) == 0) {
            std::cerr << "Warning: failed to reconstruct position!" << std::endl;
            continue;
        }
        ReconR = sqrt(x * x + y * y + z * z);
        // if(TotalPE<5000 || PEmax2Sum>0.125 || TimeRange>20) continue;
        ma->Fill();
        itmc++;
        itc++;
    }

    ma->Write();
    file->Close();

    delete tmc;
    delete truthList;
    delete PEList;
    cout << "Finish" << endl;
}