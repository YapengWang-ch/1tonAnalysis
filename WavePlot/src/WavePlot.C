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

int plotentrych(const char* filepath,const char* outputpath, const char* title, int i, int ID){
    cout << "plotting channel " << ID << " in entry " << i << endl;

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
    Redata->SetBranchAddress("ChannelId",&ChannelId);
    Redata->SetBranchAddress("Sec",&second);
    Redata->SetBranchAddress("RunNo",&RunNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    Redata->SetBranchAddress("TriggerType",&TriggerType);
    Redata->SetBranchAddress("DetectorID",&DetectorID);
    Redata->SetBranchAddress("NanoSec",&nanosecond);

    float q;
    TCanvas* c1=new TCanvas();
    int EN=Redata->GetEntries();
    
    if(EN<=i){
      printf("Entry %d no exist.\n",i);
      return 1;
    }else{
        Redata->GetEntry(i);
        ChannelN=ChannelId->size();
        FormLength=waveform->size()/ChannelN;
        int j=-1;
        for(int k=0; k<ChannelN; k++){
            if (ID==(*ChannelId)[k]){
                j=k;
                break;
            }
        }
          if (j>=0){
          // ID=(*ChannelId)[j];
            float baseline;
            WaveForm_BaseLine(baseline,waveform,j*FormLength,(j+1)*FormLength);
            vector<int> peaks =WaveForm_PeakFind(waveform,j*FormLength,(j+1)*FormLength,baseline,5);
            TH1F *hwave = (TH1F*)gROOT->FindObject("hwave");
            if (hwave) hwave->Delete();
            hwave = new TH1F("hwave", title, FormLength, 0, FormLength);
            for(int k=j*FormLength ; k < (j+1)*FormLength ; k++){
                hwave->SetBinContent(k-j*FormLength+1,(*waveform)[k]);
            }
            gStyle->SetOptStat(0);
            hwave->SetFillStyle(0);
            if (title == nullptr){
                hwave->SetTitle(Form("run%d trigger%d ch%d",RunNo,TriggerNo,ID));
            }
            hwave->SetXTitle("time [ns]");
            hwave->SetYTitle("voltage [bins]");
            hwave->Draw("");
            c1->SaveAs(outputpath);
            hwave->Delete();
            delete c1;
            printf("ch %02d: %s",ID,Form("baseline %.2f, ",baseline));
            if (!peaks.empty()){
                printf("peaks:");
                for (int ll=0; ll<peaks.size(); ll++){
                  printf(" %d/%.2f",peaks[ll]-j*FormLength,baseline-(*waveform)[peaks[ll]]);
                }  
                printf("\n");
            }else{
                printf("peaks not found\n");
            }
          }else{
            printf("doesn't find ch %02d\n",ID);
          }

    }
    delete Redata;
    return 0;}

int plotch(const char* filepath,const char* outputpath, const char* title, int ch, int icount){
    Int_t ChannelN=60;
    Int_t FormLength=900;
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);

    vector<unsigned short>* ChannelId=nullptr;
    Redata->SetBranchAddress("ChannelId",&ChannelId);

    int EN=Redata->GetEntries();
    
    if(EN<=0) return 0;
    int a=1;
        for(int i=0; i<EN; i++){
            Redata->GetEntry(i);
            ChannelN=ChannelId->size();
            for(int k=0; k<ChannelN; k++){
                if (ch==(*ChannelId)[k]){
                    if (a>=icount){
                        cout << "signal found. Entry: " << i << ", Channel: " << ch << endl;
                        return plotentrych(filepath,outputpath,title,i,ch);
                    }
                    a++;
                }
            }
        }
    
    delete Redata;
    return 0;
}

int plotentry(const char* filepath, const char* outputpath, const char* title,  int i){
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
    Redata->SetBranchAddress("ChannelId",&ChannelId);
    Redata->SetBranchAddress("Sec",&second);
    Redata->SetBranchAddress("RunNo",&RunNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    Redata->SetBranchAddress("TriggerType",&TriggerType);
    Redata->SetBranchAddress("DetectorID",&DetectorID);
    Redata->SetBranchAddress("NanoSec",&nanosecond);

    float q;
    TCanvas* c1=new TCanvas();
    float baseline=1000;
    // c1->SetCanvasSize(1600,1200);
    int EN=Redata->GetEntries();
    TH1F *hwave[60];
    std::vector<float> peaklist;
    TLegend *lg=new TLegend(0.7,0.1,0.9,0.5);
    if(EN<=i){
      printf("Entry %d no exist.\n",i);
      return 1;
    }
        Redata->GetEntry(i);
        ChannelN=ChannelId->size();
        FormLength=waveform->size()/ChannelN;
        for (int j=0; j<ChannelN; j++){
            int ID=(*ChannelId)[j];
            hwave[j] = new TH1F(Form("hwave%02d",ID), title, FormLength, 0, FormLength);
            hwave[j]->SetFillStyle(0); // 添加这行
            float baseline=1000;
            WaveForm_BaseLine(baseline,waveform,j*FormLength,(j+1)*FormLength);
            vector<int> peaks =WaveForm_PeakFind(waveform,j*FormLength,(j+1)*FormLength,baseline,5);
            for(int k=j*FormLength ; k < (j+1)*FormLength ; k++){
                hwave[j]->SetBinContent(k-j*FormLength+1,(*waveform)[k]-baseline);
            }
            hwave[j]->SetLineColor(j/9*10+j%9+1);//skip the transparement clors
            // hwave[j]->SetFillStyle(0);
            lg->AddEntry(hwave[j],Form("ch%02d: %.1f",ID,baseline),"l");

            printf("ch %02d: %s",ID,Form("baseline %.2f, ",baseline));
            if (!peaks.empty()){
                printf("peaks :");
                for (int ll=0; ll<peaks.size(); ll++){
                  printf(" %d/%.2f",peaks[ll]-j*FormLength,baseline-(*waveform)[peaks[ll]]);
                  peaklist.push_back((*waveform)[peaks[ll]]-baseline);
                }  
                printf("\n");
            }else{
                printf("peaks not found\n");
            }

        }       
            gStyle->SetOptStat(0);
            hwave[0]->SetTitle(title);
            if (title == nullptr){
                hwave[0]->SetTitle(Form("run%d trigger%d",RunNo,TriggerNo));
            }
            hwave[0]->SetXTitle("time [ns]");
            hwave[0]->SetYTitle("voltage-baseline [bins]");
            int maxpeak=0;
                for (size_t i=0; i<peaklist.size(); i++){
                    if (maxpeak>peaklist[i]){
                        maxpeak=peaklist[i];
                    }
                }
            hwave[0]->SetAxisRange(maxpeak*1.2,-maxpeak*0.3,"Y");
            hwave[0]->Draw();
            for(int j=1; j<ChannelN; j++){
                hwave[j]->Draw("SAME");
            }
            lg->Draw("SAME");
            c1->SaveAs(outputpath);
            delete c1;
    
    delete Redata;
    return 0;
}

int plotTrigger(const char* filepath, const char* outputpath, const char* title,  int i){
    Int_t ChannelN=60;
    Int_t FormLength=900;
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);

    // Int_t RunNo;
    Int_t TriggerNo;
    // Int_t TriggerType;
    // Int_t DetectorID;
    // vector<unsigned short>* waveform=nullptr;
    // Int_t second;
    // Int_t nanosecond;
    // vector<unsigned short>* ChannelId=nullptr;
    // Redata->SetBranchAddress("Waveform",&waveform);
    // Redata->SetBranchAddress("ChannelId",&ChannelId);
    // Redata->SetBranchAddress("Sec",&second);
    // Redata->SetBranchAddress("RunNo",&RunNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    // Redata->SetBranchAddress("TriggerType",&TriggerType);
    // Redata->SetBranchAddress("DetectorID",&DetectorID);
    // Redata->SetBranchAddress("NanoSec",&nanosecond);


    int EN=Redata->GetEntries();

    int k=0;
    for (k=0; k<EN; k++){
        Redata->GetEntry(k);
        if (i==TriggerNo){
            break;
        }
    }

    if(i!=TriggerNo){
        cout << "TriggerNo error: the last triggerNo is " << TriggerNo << " not "<< i <<endl;
        return 1;
    }
    plotentry(filepath,outputpath,title,k);

    delete Redata;
    return 0;
}


int plotTriggerch(const char* filepath,const char* outputpath, const char* title, int trino, int ID){
    cout << "plotting channel " <<  ID<< " in TriggerNo " << trino << endl;
    Int_t ChannelN=60;
    Int_t FormLength=900;
    TChain *Redata = new TChain("Readout","Readout");
    Redata->Add(filepath);

    // Int_t RunNo;
    Int_t TriggerNo;
    // Int_t TriggerType;
    // Int_t DetectorID;
    // vector<unsigned short>* waveform=nullptr;
    // Int_t second;
    // Int_t nanosecond;
    // vector<unsigned short>* ChannelId=nullptr;
    // Redata->SetBranchAddress("Waveform",&waveform);
    // Redata->SetBranchAddress("ChannelId",&ChannelId);
    // Redata->SetBranchAddress("Sec",&second);
    // Redata->SetBranchAddress("RunNo",&RunNo);
    Redata->SetBranchAddress("TriggerNo",&TriggerNo);
    // Redata->SetBranchAddress("TriggerType",&TriggerType);
    // Redata->SetBranchAddress("DetectorID",&DetectorID);
    // Redata->SetBranchAddress("NanoSec",&nanosecond);


    int EN=Redata->GetEntries();

    int k=0;
    for (k=0; k<EN; k++){
        Redata->GetEntry(k);
        if (trino==TriggerNo){
            break;
        }
    }

    if(trino!=TriggerNo){
        cout << "TriggerNo error: the last triggerNo is " << TriggerNo << " not "<< trino <<endl;
        return 1;
    }
    plotentrych(filepath,outputpath,title,k,ID);

    delete Redata;
    return 0;
}

    
void PrintUsage(){
    std::cout<<"Usage: ./plot <input file> <output file> [<entry>] [--triggerNo triggerNo] [--title title] [--channel ch] \n";
    std::cout<<"TriggerNo is in higher priority than entry\n";
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

    // TriggerNo 优先级高于 entry
    if (TriggerNo>0){
        if(ch>=0){
            cout << "processing file:"<<filepath<<" triggerNo:"<<TriggerNo<<" ch:"<<ch<<endl;
            return plotTriggerch(filepath, outputpath, title, TriggerNo,ch);
        }else{
            cout << "processing file:"<<filepath<<" triggerNo:"<<TriggerNo<<endl;
            return plotTrigger(filepath, outputpath, title, TriggerNo);
        }
    }


    if (entry >= 0){
        if (ch == -1){
            cout << "processing file:"<<filepath<<" entry:"<<entry<<endl;
            return plotentry(filepath, outputpath, title, entry);
        }else{
            cout << "processing file:"<<filepath<<" entry:"<<entry<< " ch:" << ch<<endl;
            return plotentrych(filepath, outputpath, title, entry, ch);
        }
    }
    if (ch>=0) {
        cout << "processing file:"<<filepath<< " ch:" << ch<<" count:"<<cc<<endl;
        return plotch(filepath, outputpath, title, ch, cc);
    }
    PrintUsage();

    return 0;
}

