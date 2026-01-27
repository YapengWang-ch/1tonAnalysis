#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include <iostream>
#include "ChannelInfo.h"
#include "TTree.h"
#include <vector>
#include "TLegend.h"
#include "TMath.h"
#include <iomanip>
#include <cstring> 
#include <fstream>
#include "TStyle.h" // 添加这一行
#include <sys/stat.h> // 添加这一行
#include <sys/types.h> // 添加这一行
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

float CherenAngle(float px,float py,float pz, float photonx, float photony, float photonz, float PMTx, float PMTy, float PMTz){
    double cosx=(px*(PMTx-photonx)+py*(PMTy-photony)+pz*(PMTz-photonz))/sqrt(px*px+py*py+pz*pz)/sqrt((PMTx-photonx)*(PMTx-photonx)+(PMTy-photony)*(PMTy-photony)+(PMTz-photonz)*(PMTz-photonz));
	cout << "costheta" << cosx <<endl;
    return TMath::ACos(cosx)*180/TMath::Pi();
}

int ReadPMTTruth(std::vector<JPSimPE_t> *PEList,int* PMT_PE_Count, double* PMT_HitTime, double **PMT_Position){
    for(int j=0 ; j<NChannels; j++){
        PMT_PE_Count[j] = 0;
        PMT_HitTime[j] = 1e9;
    }
    int id;
    double angle;
    for(size_t i=0; i<PEList->size(); i++){
        id = PEList->at(i).PMTId;
        PMT_PE_Count[id] += 1;
        angle=CherenAngle(0,0,-1,PEList->at(i).photonX*0.001,PEList->at(i).photonY*0.001,PEList->at(i).photonZ*0.001,PMT_Position[id][0],PMT_Position[id][1],PMT_Position[id][2]);
        if (PEList->at(i).HitTime < PMT_HitTime[id] && std::abs(angle-45)<0.5)
        PMT_HitTime[id]=PEList->at(i).HitTime;
    }
    return 1;
}


int main(int argc, char** argv){
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input file> <output file> <entryNo>" << std::endl;
        return 1;
    }
    const char* filepath = argv[1];
    const char* outputpath = argv[2];
    int entry = atoi(argv[3]);

    cout << "input file: " << filepath << endl;
    cout << "output file: " << outputpath << endl;
    
    TChain *data = new TChain("MCTruth", "MCTruth");
    data->Add(filepath);
    std::vector<JPSimPE_t> *PEList=nullptr;
    data->SetBranchAddress("PEList",&PEList);

    printf("data read. total %lld entries.\n", data->GetEntries());
    
    // read PMT position
    double PMT_Position[NChannels][3];
    bool PMTRead[NChannels];
    double *PMT_Position_ptr[NChannels];
    for (int i = 0; i < NChannels; ++i) {
        PMT_Position_ptr[i] = PMT_Position[i];
        PMTRead[i]=false;
    }
    if (ReadPosition("../PMT_Position.txt", PMT_Position_ptr) == 0) {
        std::cerr << "Error: failed to read PMT position!" << std::endl;
        return 1;
    }

    // Read PEList and Arrive Time
    data->GetEntry(entry);
    int PMT_PE_Count[NChannels];
    double PMT_HitTime[NChannels];
    ReadPMTTruth(PEList,PMT_PE_Count,PMT_HitTime,PMT_Position_ptr);
    // 检查并创建输出路径
    std::string outputDir = std::string(outputpath).substr(0, std::string(outputpath).find_last_of('/'));
    struct stat info;
    if (stat(outputDir.c_str(), &info) != 0) {
        if (mkdir(outputDir.c_str(), 0755) != 0) {
            std::cerr << "Error: failed to create directory " << outputDir << std::endl;
            return 1;
        }
    }

    // open file
    std::ofstream outfile(outputpath);
    if (!outfile.is_open()) {
        std::cerr << "Error: failed to open output file " << outputpath << std::endl;
        return 1;
    }

    // write data
    for (int i = 0; i < NChannels; ++i) {
        if (PMT_HitTime[i]>1e6)
        PMT_HitTime[i]=0;
        outfile << std::fixed << std::setprecision(3)
                << i << 
                " " << PMT_Position[i][0] << " "<< PMT_Position[i][1] << " "<< PMT_Position[i][2]
                << " " << PMT_PE_Count[i]
                << " " << PMT_HitTime[i]
                << std::endl;
    }
    // write not lighted PMTs
    // for(int i=0; i<NChannels; i++){
    //     if(!PMTRead[i]){
    //         outfile << std::fixed << std::setprecision(3)
    //         << i<< 
    //         " " << PMT_Position[i][0] << " "<< PMT_Position[i][1] << " "<< PMT_Position[i][2]
    //         << " " << 0
    //         << " " << 0
    //         << std::endl;
    //     }
    // }

    outfile.close();
    std::cout << "PMT charge and time information written to " << outputpath << std::endl;

    delete data;
    return 0;
}
