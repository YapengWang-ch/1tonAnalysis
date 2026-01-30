#include "TChain.h"
#include "TFile.h"
#include "TMath.h"
#include "TH1F.h"
#include "TCanvas.h"
#include <iostream>
#include <vector>
#include <ctime>
#include "TLegend.h"
#include "TStyle.h"
#include "PeakInfo.h"
#include "Tools.h"
#include "TROOT.h"
using namespace std;

int plotentrych(const char* filepath,const char* outputpath, const char* title, int i){
    cout << "plotting waveform in entry " << i << endl;

    Int_t ChannelN=60;
    Int_t FormLength;
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);

    Int_t RunNo;
    Int_t TriggerNo;
    Int_t TriggerType;
    Int_t DetectorID;
    vector<unsigned short>* waveform=nullptr;
    Int_t second;
    Int_t nanosecond;
    vector<unsigned short>* ChannelId=nullptr;
    Redata->SetBranchAddress("Waveform",&waveform);
    // Redata->SetBranchAddress("ChannelId",&ChannelId);
    // Redata->SetBranchAddress("Sec",&second);
    // Redata->SetBranchAddress("RunNo",&RunNo);
    // Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    // Redata->SetBranchAddress("TriggerType",&TriggerType);
    // Redata->SetBranchAddress("DetectorID",&DetectorID);
    // Redata->SetBranchAddress("NanoSec",&nanosecond);

    float q;
    TCanvas* c1=new TCanvas();
    int EN=Redata->GetEntries();
    
    if(EN<=i){
      printf("Entry %d no exist.\n",i);
      return 1;
    }else{
        Redata->GetEntry(i);
        // ChannelN=ChannelId->size();
        FormLength=waveform->size();
          // ID=(*ChannelId)[j];
            float baseline=1000;
            WaveForm_BaseLine(baseline,waveform,0,FormLength);
            vector<int> peaks =WaveForm_PeakFind(waveform,0,FormLength,baseline,5);
            TH1F *hwave = (TH1F*)gROOT->FindObject("hwave");
            if (hwave) hwave->Delete();
            hwave = new TH1F("hwave", title, FormLength, 0, FormLength);
            for(int k=0 ; k < FormLength ; k++){
                hwave->SetBinContent(k+1,(*waveform)[k]);
            }
            gStyle->SetOptStat(0);
            hwave->SetFillStyle(0);
            if (title == nullptr){
                hwave->SetTitle(Form("entry %d",i));
            }
            hwave->SetXTitle("time [ns]");
            hwave->SetYTitle("voltage [mV]");
            hwave->Draw("");
            c1->SaveAs(outputpath);
            hwave->Delete();
            delete c1;
            printf("%s",Form("baseline %.2f, ",baseline));
            if (!peaks.empty()){
                printf("peaks:");
                for (int ll=0; ll<peaks.size(); ll++){
                  printf(" %d/%.2f",peaks[ll],baseline-(*waveform)[peaks[ll]]);
                }  
                printf("\n");
            }else{
                printf("peaks not found\n");
            }
    }
    delete Redata;
    return 0;
}
    
void PrintUsage(){
    std::cout<<"Usage: ./plot <input file> <output file> <entry> \n";
    // std::cout<<"TriggerNo is in higher priority than entry\n";
}
 
int main(int argc, char** argv){
    const char* filepath = nullptr;
    const char* outputpath = nullptr;
    const char* title = nullptr;
    int ch = -1;
    int entry = -1;
    int cc = 1;
    int TriggerNo=0;
    // if (argc != 4 && argc != 5) {
    //     std::cerr << "Usage: " << argv[0] << " <input file> <output file> <title> <entry> [ch]" << std::endl;
    //     return 1;
    // }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--triggerNo") == 0 && i + 1 < argc) {
            TriggerNo = atoi(argv[++i]);
        }else if (strcmp(argv[i], "--channel") == 0 && i + 1 < argc) {
            ch = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--title") == 0 && i + 1 < argc){
            title = argv[++i];
        }else if (strcmp(argv[i], "--cc") == 0 && i + 1 < argc){
            cc = atoi(argv[++i]);
        } else if (filepath == nullptr) {
            filepath = argv[i];
        } else if (outputpath == nullptr) {
            outputpath = argv[i];
        } else if (entry == -1){
            entry = atoi(argv[i]);
        }
    }
    
    if (filepath == nullptr || outputpath == nullptr) {
        PrintUsage();
        return 1;
    }



    if (entry >= 0){            
        cout << "processing file:"<<filepath<<" entry:"<<entry<<endl;
        return plotentrych(filepath, outputpath, title, entry);
    }
    PrintUsage();

    return 0;
}

