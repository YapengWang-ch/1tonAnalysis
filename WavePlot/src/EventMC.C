#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include <iostream>
// #include "ChannelInfo.h"
#include "TTree.h"
#include <vector>
#include "TVector3.h"
#include "TLegend.h"
#include <iomanip>
#include <cstring> 
#include <fstream>
#include <sstream>
#include <string>
#include <istream>
#include <ostream>
#include "TStyle.h" // 添加这一行
#include <sys/stat.h> // 添加这一行
#include "TDatime.h"
#include <sys/types.h> // 添加这一行
#include "PeakInfo.h"
#include "Tools.h"
using namespace std;
Int_t NChannels=60;

struct JPSimPrimaryParticle_t {
	Int_t TrackId;
	Int_t PdgId;
	Double_t px;
	Double_t py;
	Double_t pz;
	Double_t Ek;
    JPSimPrimaryParticle_t()
    {
        TrackId = 0;
        PdgId = 0;
        px = 0;
        py = 0;
        pz = 0;
        Ek = 0;
    }
};

struct JPSimPE_t {
	Int_t PMTId;
	Int_t segmentId;
	Int_t primaryParticleId;
	Double_t photonX;
	Double_t photonY;
	Double_t photonZ;
	Double_t dETime;	 // Zero point is vertex time (0). In ns.
	Double_t photonTime; // Zero point is vertex time (0). In ns.
	Double_t HitTime;	 // Zero point is vertex time (0). In ns. No TT and TTS.
	Double_t PulseTime;  // Zero point is vertex time (0). In ns.
	Int_t PESec;
	Int_t PENanoSec;
    double PESubNanoSec;
	double HitPosInWindow;
	Double_t Wavelength;
	Int_t PEType;
    double Charge;
	JPSimPE_t() {
		PMTId = 0;
		primaryParticleId = 0;
		photonX = 0;
		photonY = 0;
		photonZ = 0;
		dETime = 0;
		photonTime = 0;
		HitTime = 0;
		PulseTime = 0;
		PESec = 0;
		PENanoSec = 0;
		PESubNanoSec = 0;
		HitPosInWindow = 0;
		Wavelength = 0;
		PEType = 0;
        Charge = 0;
	}
};

struct JPSimStepPoint_t
{
	Int_t nProcessType;
	Int_t nProcessSubType;
	Double_t fX;
	Double_t fY;
	Double_t fZ;
	Double_t fPx;
	Double_t fPy;
	Double_t fPz;
	Double_t fEk;
	Double_t fdE;
	Double_t fTime;
	Int_t nTargetZ;
	Int_t nTargetA;
	std::vector<Int_t> nSecondaryPdgId;
	JPSimStepPoint_t() {nTargetZ=-1; nTargetA=-1;}
};


struct JPSimTrack_t
{
	Int_t nSegmentId;
	Int_t nParentTrackId;
	Int_t nTrackId;
	Int_t nPrimaryId;
	Int_t nPdgId;
    Bool_t bDetectedPhoton;
	std::vector<JPSimStepPoint_t> StepPoints;
	JPSimTrack_t() {}
};

struct JPSimTruthTree_t {
	Int_t RunId;		// The ID of Run
	Int_t SegmentId;	// The ID of Segment
	Int_t VertexId;
	Int_t VertexRadZ;
	Int_t VertexRadA;
	Int_t nParticle;
	
	Double_t x;
	Double_t y;
	Double_t z;
	Int_t Sec;
	Int_t NanoSec;
	std::vector<JPSimPrimaryParticle_t> PrimaryParticleList;
	Double_t EkMerged;

	std::vector<Double_t> dEList;	// Energy deposit in tagged volumes

	std::vector<Double_t> userdefinedA;
	std::vector<Double_t> userdefinedB;

	Int_t nFiredPMT;

	Int_t CPh;			// The number of C photons
	Int_t SPh;			// The number of S photons
	Int_t APh;			// The number of all photons
	Int_t CPE;			// The number of C p.e.
	Int_t SPE;
	Int_t APE;

	std::vector<JPSimTrack_t> trackList;

	JPSimTruthTree_t()
	{
		nFiredPMT = 0;
		EkMerged = 0;
		CPE = 0;
		SPE = 0;
		APE = 0;
		APh = 0;
		CPh = 0;
		SPh = 0;
	}

};

int SearchTriggerNo(const char* filepath, int trigger_No, int &entry){
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);
    Int_t TriggerNo;
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    int EN=Redata->GetEntries();

    int k=0;
    for (k=0; k<EN; k++){
        Redata->GetEntry(k);
        if (trigger_No==TriggerNo){
            break;
        }
    }

    if(trigger_No!=TriggerNo){
        cout << "TriggerNo error: the last triggerNo of file "<<filepath<<" is " << TriggerNo << " not "<<  trigger_No<<endl;
        return 1;
    }

    entry = k;
    delete Redata;
    return 0;
}


int GetEventData(const char* filepath, const char* outputpath, const char* title, int entry, const char* PMTPosition_File, const char*  TimeOffset_File){
    // read time offset
    vector<double> Toffset(NChannels,0), TError(NChannels,0);
    bool timeflag=true;
    // cout << "TimeOffset_File: "<<TimeOffset_File<<endl;
    if (TimeOffset_File == nullptr || strcmp(TimeOffset_File, "off") == 0 || strcmp(TimeOffset_File, "no") == 0) {
        timeflag=false;
    }else if (Read_TimeOffset(TimeOffset_File, Toffset, TError))
    {
        cout << "Error reading Time Calibration data, the Time Calibration shut off." << endl;
        timeflag=false;
    }

    //read PMT position
    vector<TVector3> PMTPosition;
    if (Read_PMTPosition(PMTPosition_File,PMTPosition)){
        std::cerr << "Error: failed to read PMT position!" << std::endl;
        return 1;
    }


    //read data 
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);

    Int_t RunNo;
    Int_t TriggerNo;
    // Int_t TriggerType;
    // Int_t DetectorID;
    vector<unsigned short>* waveform=nullptr;
    Int_t second;
    Int_t nanosecond;
    vector<unsigned short>* ChannelId=nullptr;
    Redata->SetBranchAddress("Waveform",&waveform);
    Redata->SetBranchAddress("ChannelId",&ChannelId);
    Redata->SetBranchAddress("Sec",&second);
    Redata->SetBranchAddress("RunNo",&RunNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    // Redata->SetBranchAddress("TriggerType",&TriggerType);
    // Redata->SetBranchAddress("DetectorID",&DetectorID);
    Redata->SetBranchAddress("NanoSec",&nanosecond);

    if (!Redata->GetEntry(entry)){
        printf("Data read error:entry %d read failed in %lld entries.\n",entry, Redata->GetEntries());
        return 1;
    }

    //read SimInfo 
    TChain *Truth = new TChain("SimTriggerInfo","SimTriggerInfo");
    Truth->Add(filepath);
    Int_t RunNo_Truth;
    Int_t TriggerNo_Truth;
    std::vector<JPSimTruthTree_t> *truthList = nullptr;

    Truth->SetBranchAddress("RunNo",&RunNo_Truth);
    Truth->SetBranchAddress("TriggerNo",&TriggerNo_Truth);
    Truth->SetBranchAddress("truthList", &truthList);

    Truth->GetEntry(entry);
    cout << "------------ Truth Read ---------------"<<endl;
    if (RunNo_Truth!=RunNo) {
        cout << "Error: the RunNo of Readout don't match with the SimTruth!";
    }
    if (TriggerNo!=TriggerNo_Truth) Truth->GetEntry(entry+TriggerNo-TriggerNo_Truth);

    // 打开输出文件
    std::ofstream outfile(outputpath);
    if (!outfile.is_open()) {
        std::cerr << "Error: failed to open output file " << outputpath << std::endl;
        return 1;
    }
    // cout << "writing entry info"<<endl;
    outfile << "run:"<<RunNo<<" TriggerNo:"<<TriggerNo<<" Second:"<<second<<endl;
    if (truthList==nullptr||truthList->size()==0){ 
        cout << "Warning: TruthInfo empty!";
    }else if ((truthList->at(0).PrimaryParticleList).size()==0){
        cout << "Warning: ParticleInfo empty!";
    }else{
        TVector3 vertexpos(truthList->at(0).x, truthList->at(0).y, truthList->at(0).z);
        TVector3 primarymom(truthList->at(0).PrimaryParticleList.at(0).px, truthList->at(0).PrimaryParticleList.at(0).py, truthList->at(0).PrimaryParticleList.at(0).pz);
        double distance = vertexpos.Mag();
        double dis_TC = distance*TMath::Sin(primarymom.Angle(vertexpos));
        outfile <<"info: "<<endl
        <<"Primary Particle: "<<ParticleName(truthList->at(0).PrimaryParticleList.at(0).PdgId) << endl
        <<"Particle Energy: "<<truthList->at(0).PrimaryParticleList.at(0).Ek<<"MeV"<<endl
        <<"Vertex Position: ("<<truthList->at(0).x/1000<<", "<<truthList->at(0).y/1000<<", "<<truthList->at(0).z/1000<<")m"<<endl
        <<"Primary Momentum: ("<<truthList->at(0).PrimaryParticleList.at(0).px<<", "<<truthList->at(0).PrimaryParticleList.at(0).py<<", "<<truthList->at(0).PrimaryParticleList.at(0).pz<<")MeV"<<endl
        <<"Distance from Track to Tank Center: "<<dis_TC/1000<<" m"<<endl
        <<"Distance from Vertex to Tank Center: "<<distance/1000<<" m"<<endl
        <<"info end"<<endl;
    }

    if (title==nullptr){
        // TDatime eventtime(second+8);
        title = Form("run%dTrigger%d @ %s ",RunNo, TriggerNo, TimePrint(second).c_str());
    }else if (title[0]=='+'){
        // TDatime eventtime(second+8);
        title = Form("run%dTrigger%d @ %s %s",RunNo, TriggerNo, TimePrint(second).c_str(), title+1);
    }
    outfile <<"title: "<<title << endl;

    // waveform analysis
    int NChannel=ChannelId->size();
    int FormLength=waveform->size()/NChannel;
    vector<int> PMTread(NChannels,0);
    for (int i = 0; i < NChannel; i++) {
        int channel = ChannelId->at(i);
        // cout <<"processing ch "<<channel<<endl;
        // cout <<"calculating baseline"<<endl;
        double baseline = WaveForm_BaseLine(waveform,i*FormLength, (i+1)*FormLength);
        // cout <<"Analysising peaks"<<endl;
        std::vector<PeakInfo> peaks = WaveForm_GetPeak(channel,waveform,i*FormLength, (i+1)*FormLength, baseline);

        double risetime =0;
        double charge =0;
        if (peaks.size()>0){
            risetime =peaks[0].risetime;
            for (PeakInfo peak: peaks){
                charge+=peak.charge;
            }
        }
        // cout <<"data writing"<<endl;
        if (timeflag) risetime -= Toffset[channel];
        outfile << std::fixed << std::setprecision(3)
                << channel<< 
                " " << PMTPosition[channel].X() << " "<< PMTPosition[channel].Y() << " "<< PMTPosition[channel].Z()
                << " " << charge
                << " " << risetime
                << std::endl;
        cout << std::fixed << std::setprecision(3)<<"ch"
                << channel<< 
                " \tposition:" << PMTPosition[channel].X() << " "<< PMTPosition[channel].Y() << " "<< PMTPosition[channel].Z()
                << " \tcharge:" << charge
                << " \trisetime:" << risetime
                << std::endl;
        PMTread[channel]=1;
    }
    // write not lighted PMTs
    for(int i=0; i<NChannels; i++){
        if(!PMTread[i]){
            outfile << std::fixed << std::setprecision(3)
            << i<< 
            " " << PMTPosition[i].X() << " "<< PMTPosition[i].Y() << " "<< PMTPosition[i].Z()
            << " " << 0
            << " " << 0
            << std::endl;
        }
    }

    outfile.close();
    std::cout << "PMT charge and time information written to " << outputpath << std::endl;

    delete Redata;
    return 0;
}

void PrintUsage(const char* cmd){
    cout <<"Usage: "<<cmd<<" inputfile outputfile [--entryNo entryNo] [--triggerNo triggerNo] <[--title plot_title]> <[--position PMTPositionFile]> <[--time TimeCalibrationFile]>"<<endl;
    cout <<"triggerNo is in higher priority than entryNo."<<endl;
    cout <<"\"--time off\" to shut off the time calibration."<<endl;
}
int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;
    const char* title = nullptr;
    const char* PMTPosition_File="/home/wangyp/1ton/ReConstruction/WavePlot/data/PMT_Position.txt";
    const char* TimeOffset_File="/home/wangyp/1ton/ReConstruction/WavePlot/data/TCali_iter43845.txt";
    int entry = -1;
    int TriggerNo=0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--triggerNo") == 0 && i + 1 < argc) {
            TriggerNo = atoi(argv[++i]);
        }else if (strcmp(argv[i], "--title") == 0 && i + 1 < argc){
            title = argv[++i];
        }else if (strcmp(argv[i], "--position") == 0 && i + 1 < argc){
            PMTPosition_File = argv[++i];
        }else if (strcmp(argv[i], "--time") == 0 && i + 1 < argc){
            TimeOffset_File = argv[++i];
        }else if (strcmp(argv[i], "--entry") == 0 && i + 1 < argc){
            entry = atoi(argv[++i]);
        }else if (filepath == nullptr) {
            filepath = argv[i];
        } else if (outputpath == nullptr) {
            outputpath = argv[i];
        } 
    }
    if (filepath == nullptr || outputpath == nullptr){
        PrintUsage(argv[0]);
        return 1;
    }
    if (!ends_with(outputpath,".txt")) outputpath=change_extension(outputpath,".txt");
    if (TriggerNo >0 ) SearchTriggerNo(filepath, TriggerNo, entry);
    if (entry>=0) {
        GetEventData(filepath, outputpath, title, entry, PMTPosition_File, TimeOffset_File);
        return 0;
    }
    cout <<"No valid Trigger or entry number."<<endl;
    PrintUsage(argv[0]);
    return 1;
}   
