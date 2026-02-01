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
const char * badchannelfile = "../data/BadChannels.txt";
const char* PMT_PositionFile = "../data/PMT_Position.txt";


bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++){
        if (badchannellist->at(i)==PMTId){
            return true;
        }
    }
    return false;
}

double GetTimesError(std::vector<ChannelInfo_t> &ChannelInfo){
    double FirstRisetime=900;
    double SecondRisetime=900;
    for (const auto& channel : ChannelInfo) {
        int npeak=channel.PeakLoc.size();
        for(int i=0; i<npeak; i++){
            // if (channel.PeakAmp[i]<50) continue;
            if (channel.PeakLoc[i]  < FirstRisetime) {
                SecondRisetime = FirstRisetime;
                FirstRisetime = channel.PeakLoc[i]  ;
            }else if (channel.PeakLoc[i]   < SecondRisetime){
                SecondRisetime = channel.PeakLoc[i]  ;
            }
            break;
        }
    }
    return SecondRisetime-FirstRisetime; 
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

float getDistance(Double_t x, Double_t y, Double_t z, Double_t px, Double_t py, Double_t pz, Double_t d) {
    double lsquarecos=(x*px+y*py+z*pz)*(x*px+y*py+z*pz)/(px*px+py*py+pz*pz);
    double lsquare=(x*x+y*y+z*z);
    return sqrt(lsquare - lsquarecos); 
}

int main(int argc, char **argv)
{
    gSystem->Load(gSystem->ExpandPathName("$JSAPSYS/Simulation/DataType/lib/libJPSIMOUTPUT.so"));

    
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
    Int_t nFlag = jputils::ReadRawDataRootFiles(inputFilename, tc);
    Int_t nFlag1 = jputils::ReadRawDataRootFiles(inputFilename, tmc);

    if (nFlag == -1 || nFlag1 == -1)
    {
        cout << "PreAnalysis terminated." << endl;
        return 1;
    }

    std::vector<JPSimTruthTree_t> *truthList = new std::vector<JPSimTruthTree_t>;
    std::vector<JPSimPE_t> *PEList = new std::vector<JPSimPE_t>;
    Int_t RunNo, TriggerNo, TriggerNo1;
    tmc->SetBranchAddress("PEList", &PEList);
    tmc->SetBranchAddress("truthList", &truthList);
    tmc->SetBranchAddress("TriggerNo", &TriggerNo1);

    TFile *file = new TFile(outputfilename, "recreate");

    TTree *ma = new TTree("ma", "ma");
    int FileNo, nChannels, nGlobal;
    double TotalPE, PEmax2Sum, TimeRange;
    int nBeta, ncosAlpha, ncosTheta, nPhi;
    double cosAlpha, Beta, cosTheta, Phi;
    int ChannelIdList[60];
    double Time[60], FrontBslnStdDev[60];
    double Energy[60];
    double tError;
    double ReconR;
    int nLightedPMT;
    double distance;
    vector<int> ChannelId;
    // double EffPE;



    ma->Branch("RunNo", &RunNo);
    ma->Branch("FileNo", &FileNo);
    ma->Branch("TriggerNo", &TriggerNo);
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
    ma->Branch("nLightedPMT",&nLightedPMT);
    ma->Branch("ChannelId",&ChannelId);

    // For Makefile
    double evtPE;
    Int_t Sec;
    Int_t NanoSec;
    std::vector<ChannelInfo_t> *ChannelInfo = nullptr;

    tc->SetBranchAddress("TotalPE", &evtPE);
    tc->SetBranchAddress("ChannelInfo", &ChannelInfo);
    tc->SetBranchAddress("Sec", &Sec);
    tc->SetBranchAddress("NanoSec", &NanoSec);
    tc->SetBranchAddress("RunNo", &RunNo);
    tc->SetBranchAddress("TriggerNo", &TriggerNo);


    long totalEntries = tc->GetEntries();

    cout << "Total readout entries: " << totalEntries << endl;

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
    if (ReadPosition(PMT_PositionFile, PMT_Position_ptr) == 0) {
        std::cerr << "Error: failed to read PMT position!" << std::endl;
        return 1;
    }
    // read bad channel
    vector<int> badchannellist={26,29,38,54};
    std::vector<int> *BadChannelList = &badchannellist;

    double x,y,z;
    int itruth=0, iread=0;
    while (iread<totalEntries && itruth<tmc->GetEntries())
    {
        if (iread % nProcessCheck == 0)
            cout << "Processing entry " << iread << " (" << iread / (double)totalEntries * 100 << "%)" << endl;
        
        tmc->GetEntry(itruth);
        tc->GetEntry(iread);
        iread++;
        itruth++;

        if (TriggerNo != TriggerNo1)
        {
            cout << "The trigger number is not matched!" << endl;
            // cout << "MC entry: " << iread << endl;
            // cout << "Waveform entry: " <<  << endl;
            cout << "MC truth TriggerNo: " << TriggerNo1 << endl;
            cout << "Waveform TriggerNo: " << TriggerNo << endl;
            cout << endl;
            if (TriggerNo > TriggerNo1){
                iread--;
            }else {
                itruth--;
            }
            continue;
        }
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
        // BadChannelList->clear();
        // ReadBadChannel(RunNo, FileNo, BadChannelList);

        for (int j=0; j<ChannelInfo->size(); j++){
            if (IsBadChannel((*ChannelInfo)[j].ChannelId,BadChannelList)){
                ChannelInfo->erase(ChannelInfo->begin()+j);
                j--;
            }
        }

        nChannels = ChannelInfo->size();
        nLightedPMT=nChannels;
        if (nChannels < 56){
            continue;
        }
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


        TVector3 p(PrimaryParticleList[0].px, PrimaryParticleList[0].py, PrimaryParticleList[0].pz);

        double d=1290;  
        distance=getDistance(x,y,z,p.X(),p.Y(),p.Z(),d);
        if (distance > d/2){
            continue;
        }
        // if (distance>1290*0.48){
        //     continue;
        // }

        double theta = p.Theta();
        double phi = p.Phi();
        Double_t t1 = -z * cos(theta) - sin(theta) * (x * cos(phi) + y * sin(phi)) - sqrt(1500 * 1500 - x * x - y * y - z * z + pow(z * cos(theta) + sin(theta) * (x * cos(phi) + y * sin(phi)), 2.));
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
        // for (Int_t j = 0; j < nChannels; j++)
        // {   
        //     UInt_t PMTId = ChannelInfo->at(j).ChannelId;
        //     ChannelIdList[PMTId] = 1;
        //     Energy[PMTId]= ChannelInfo->at(j).PE;
        //     Time[PMTId] = ChannelInfo->at(j).RiseTime;
        // }
        ChannelId.clear();
        for (int j = 0; j < ChannelInfo->size(); j++) {
            ChannelId.push_back((*ChannelInfo)[j].ChannelId);
            ChannelIdList[(*ChannelInfo)[j].ChannelId] = 1;
            Energy[(*ChannelInfo)[j].ChannelId] = (*ChannelInfo)[j].PE;
            Time[(*ChannelInfo)[j].ChannelId] = (*ChannelInfo)[j].RiseTime;
        }
        std::sort(ChannelId.begin(), ChannelId.end());

        TimeRange = 0;
        PEmax2Sum = getrmax(*ChannelInfo);
        tError = GetTimesError(*ChannelInfo);
        if (PositionReCon(*ChannelInfo, PMT_Position_ptr, &x, &y, &z) == 0) {
            std::cerr << "Warning: failed to reconstruct position!" << std::endl;
            continue;
        }
        ReconR = sqrt(x * x + y * y + z * z);
        
        if(PEmax2Sum>0.3 || cosTheta <0 || tError > 10) continue;
        ma->Fill();
    }

    ma->Write();
    file->Close();

    delete tmc;
    delete truthList;
    delete PEList;
    cout << "Finish" << endl;
}