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
#include "TChain.h"
#include <vector>
#include <cmath>
#include <algorithm>
// chisquare/NDF=2.45605/9 P=0.5400
using namespace std;

const int NChannels = 60;
const char* MCFile = "../Output/MCMuons/MCRun*.root";
const char* OutputFile = "../data/TimeVariance_MC.txt";
const int MaxMuons = 1000000;

bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
    for (size_t i=0; i<badchannellist->size(); i++) if (badchannellist->at(i)==PMTId) return true;
    return false;
}

void ClearLowBins(TH2F* hist) {
    if (!hist) return;
    Double_t maxVal = 0.0;
    Int_t nX = hist->GetNbinsX();
    Int_t nY = hist->GetNbinsY();
    
    for (Int_t ix = 1; ix <= nX; ++ix) {
        for (Int_t iy = 1; iy <= nY; ++iy) {
            Double_t content = hist->GetBinContent(ix, iy);
            if (content > maxVal) {
                maxVal = content;
            }
        }
    }

    Double_t threshold = maxVal * 0.01;

    for (Int_t ix = 1; ix <= nX; ++ix) {
        for (Int_t iy = 1; iy <= nY; ++iy) {
            Double_t content = hist->GetBinContent(ix, iy);
            if (content < threshold) {
                hist->SetBinContent(ix, iy, 0.0);
            }
        }
    }
}

int main(){
    TH2F * tdist = new TH2F("tdist","tdist",NChannels,0,NChannels,100,-10,10);
    TH2F * tLast = new TH2F("tlast","tlast",100,0,10,100,0,100);
    TH2F * tFirst = new TH2F("tfirst","tfirst",100,0,10,100,-100,0);
    
    vector<int> badchannellist={5,11,18,29,40,51,53,58};
    std::vector<int> *BadChannelList = &badchannellist;
    
    TChain* ma=new TChain("ma","ma");
    ma->Add(MCFile);
    if (ma->GetEntries()<=0){
        cout << "Error: no entry found in muonfile." <<endl;
        return 1;
    }
    int totalentries= min((int)ma->GetEntries(),MaxMuons);
    cout << "total "<< totalentries <<" entries used in " <<ma->GetEntries()<<" entries."<<endl;
    // int ChannelIdList[NChannels];
    double Time[NChannels];
    ma->SetBranchAddress("Time",Time);
    int validevents=0;
    for (int i=0; i<totalentries; i++){
        ma->GetEntry(i);
        std::vector<double> tVector;
        
        for (int j=0; j<NChannels ; j++){
            if (IsBadChannel(j,BadChannelList)) continue;
            tVector.push_back(Time[j]);
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

    ofstream outfile(OutputFile);
    for (int i=0; i<tdist->GetNbinsX(); i++){
        TH1D* projY = tdist->ProjectionY("proj", i+1, i+1);
        outfile << i << " " << projY->GetStdDev()*projY->GetStdDev() << endl;
        delete projY;
    }
    outfile.close();

    tdist->SaveAs("../data/tdist_MC.root");
    cout << "valid percentage: "<< (float)validevents/totalentries<<endl;

    return 0;
}

