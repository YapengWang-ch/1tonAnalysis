// plot _Angular_Distribution of muons
// comparison with the model and the P-Value
#include "TTree.h"
#include "TFile.h"
#include <iostream>
#include <fstream>
#include "TMath.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TRandom3.h"
#include <vector>
// chisquare/NDF=2.45605/9 P=0.5400
using namespace std;

const int NChannels = 60;
const char* TempleteFile = "../../TemplateGen/templates/MuonMap_Water.root";
const char* OutputFile = "../data/TimeVariance.txt";
const int MaxTemplate = 10000000;
struct node
{
    double cosTheta;
    double Phi;
    double cosAlpha;
    double Beta;
    double Time2Mean[NChannels];
    double Energy[NChannels];
};

void ReadMap(vector<node> &MeanMap)
{
    TFile *fmap = new TFile(TempleteFile);
    TTree *tmap = (TTree *)fmap->Get("MuonMap");
    Int_t nEntries = tmap->GetEntries();
    Int_t readTemplates = std::min(nEntries, MaxTemplate);
    double Time2Mean[NChannels], Energy[NChannels];
    double cosTheta, Phi, cosAlpha, Beta;
    tmap->SetBranchAddress("cosTheta", &cosTheta);
    tmap->SetBranchAddress("Phi", &Phi);
    tmap->SetBranchAddress("cosAlpha", &cosAlpha);
    tmap->SetBranchAddress("Beta", &Beta);
    tmap->SetBranchAddress("Time2Mean", Time2Mean);
    tmap->SetBranchAddress("Energy", Energy);

    for (int i = 0; i < readTemplates; i++)
    {
        tmap->GetEntry(i);
        node d1;
        d1.cosTheta = cosTheta;
        d1.Phi = Phi;
        d1.cosAlpha = cosAlpha;
        d1.Beta = Beta;

        for (int j = 0; j < NChannels; j++)
        {
           d1.Time2Mean[j] = Time2Mean[j];
           d1.Energy[j] = Energy[j];
        }
        MeanMap.emplace_back(d1);
    }
    cout << "Finish reading map, "<< readTemplates << " map used in " << nEntries << " Entries."<< endl;
}

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++) if (badchannellist->at(i)==PMTId) return true;
    return false;
}

int main(){
    TH2F * tdist = new TH2F("tdist","tdist",NChannels,0,NChannels,100,-10,10);
    
    vector<int> badchannellist={5,11,18,26,29,40,51,53,58};
    std::vector<int> *BadChannelList = &badchannellist;
    
    vector<node> MeanMap;
    ReadMap(MeanMap);
    int totalentries= min((int)(MeanMap.size()),MaxTemplate);
    cout << "total "<< totalentries <<" entries used in " << MeanMap.size() <<" entries."<<endl;

    int validevents=0;
    for (size_t i=0; i<totalentries; i++){
        std::vector<float> tVector;
        for (int j=0; j<NChannels ; j++){
            if (IsBadChannel(j,BadChannelList)) continue;
            tVector.push_back(MeanMap[i].Time2Mean[j]);
        }

        sort(tVector.begin(), tVector.end());
        double MeanTime=std::accumulate(tVector.begin(), tVector.end(), 0.0)/tVector.size(); 
        double sigma2=0;
        for (size_t k=0; k<tVector.size(); k++){
            sigma2+=(tVector[k] - MeanTime)*(tVector[k] - MeanTime);
        }
        double sigma=TMath::Sqrt(sigma2)/(tVector.size()-1);

        if (sigma>0.8 || MeanTime-tVector.front() > 10) continue;
        for (size_t k=0; k<tVector.size(); k++){
            tdist->Fill(k,tVector[k] - MeanTime);
        }
        validevents++;
    }
    // ClearLowBins(tdist);

    ofstream outfile(OutputFile);
    for (int i=0; i<tdist->GetNbinsX(); i++){
        TH1D* projY = tdist->ProjectionY("proj", i+1, i+1);
        outfile << i << " " << projY->GetStdDev()*projY->GetStdDev() << endl;
        delete projY;
    }
    outfile.close();

    tdist->SaveAs("../data/tdist.root");
    cout << "valid percentage: "<< (float)validevents/totalentries<<endl;

    return 0;
}


