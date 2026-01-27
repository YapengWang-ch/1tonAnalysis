#include "TChain.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include <iostream>
// #include "ChannelInfo.h"
#include "TTree.h"
#include <vector>
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


// vector<int> badchannellist = {2,5,10,11,17,37,18,26,29,34,40,41,42,50,51,53,58};
// vector<int> badchannellist = {5,11,18,26,29,38,46,51,53,54};
// vector<int> badchannellist = {5,11,18,26,29,40,51,53,58};
// std::vector<int> *BadChannelList = &badchannellist;
// bool IsBadChannel(UInt_t PMTId, std::vector<int> *badchannellist ){ //检查是否为badchannel
//     for (size_t i=0; i<badchannellist->size(); i++){
//         if (badchannellist->at(i)==PMTId){
//             return true;
//         }
//     }
//     return false;
// }
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

    // // crate outputpath
    // std::string outputDir = std::string(outputpath).substr(0, std::string(outputpath).find_last_of('/'));
    // struct stat info;
    // if (stat(outputDir.c_str(), &info) != 0) {
    //     if (mkdir(outputDir.c_str(), 0755) != 0) {
    //         std::cerr << "Error: failed to create directory " << outputDir << std::endl;
    //         return 1;
    //     }
    // }

    // 打开输出文件
    std::ofstream outfile(outputpath);
    if (!outfile.is_open()) {
        std::cerr << "Error: failed to open output file " << outputpath << std::endl;
        return 1;
    }
    // cout << "writing entry info"<<endl;
    outfile << "run:"<<RunNo<<" TriggerNo:"<<TriggerNo<<" Second:"<<second<<endl;
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
