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
    badchannellist->push_back(5);
    badchannellist->push_back(11);
    badchannellist->push_back(18);
    badchannellist->push_back(29);
    badchannellist->push_back(40);
    badchannellist->push_back(51);
    badchannellist->push_back(53);
    badchannellist->push_back(58);
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
        cout << "  ./plotDistance inputFilename outputFilename" << endl;
        cout << endl;
        return 1;
    }

    // TChain *tc = new TChain("SimpleAnalysis");
    TChain *tmc = new TChain("SimTriggerInfo");

    //cout << "Adding files..." << inputFilename << endl;
    // Int_t nFlag = jputils::ReadRawDataRootFiles(inputFilename, tc);
    // Int_t nFlag1 = jputils::ReadRawDataRootFiles(inputFilename, tmc);
    // tc->Add(inputFilename);
    tmc->Add(inputFilename);
    // Int_t nFlag = tc->GetEntries();
    Int_t nFlag1 = tmc->GetEntries();

    if (nFlag1 == -1 || nFlag1 == -1)
    {
        cout << "PreAnalysis terminated." << endl;
        return 1;
    }
    // cout << "Total " << nFlag << " waveform files added." << endl;
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
    // tmc->SetBranchAddress("TriggerNo", &TriggerTruth);
    // tmc->SetBranchAddress("PEList", &PEList);
    tmc->SetBranchAddress("truthList", &truthList);

    TH1F *hd=new TH1F("hd","distance",200,0,2000);
    // TFile *file = new TFile(outputfilename, "recreate");

    // TTree *ma = new TTree("ma", "ma");
    // int FileNo, nChannels, nGlobal;
    // double TotalPE, PEmax2Sum, TimeRange;
    // int nBeta, ncosAlpha, ncosTheta, nPhi;
    // double cosAlpha, Beta, cosTheta, Phi;
    // int ChannelIdList[60];
    // double Time[60], FrontBslnStdDev[60];
    // double Energy[60];
    // double tError;
    // double ReconR;



    Int_t entry = 0;
    Int_t nProcessCheck = TMath::Min(100., nFlag1 / 1.);
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
    vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
    std::vector<int> *BadChannelList = &badchannellist;

    double x,y,z,px,py,pz;
    int itmc=0;
    int itc=0;
    for (int i = 0; i < nFlag1; i++) //遍历缪子data，18个
    {
       if (i % 1000 == 0)
            cout << i <<"/" << nFlag1 << endl;
        tmc->GetEntry(i);
        
        // if (TimeCalibYesOrNo == "TimeCalibYes")
        // {
        //     for (int i = 0; i < NChannels; i++)
        //     {
        //         Time[i] -= DelayTime[i];
        //     }
        // }
        x=truthList->at(0).x;
        y=truthList->at(0).y;
        z=truthList->at(0).z;
        vector<JPSimPrimaryParticle_t> PrimaryParticleList = (*truthList)[0].PrimaryParticleList;
        px=PrimaryParticleList[0].px;
        py=PrimaryParticleList[0].py;
        pz=PrimaryParticleList[0].pz;
        float distance = getDistance(x, y, z, px, py, pz);
        hd->Fill(distance);
    }
    hd->SaveAs(outputfilename);


    // delete tmc;
    // delete truthList;
    // delete PEList;
    cout << "Finish" << endl;
    return 0;
}