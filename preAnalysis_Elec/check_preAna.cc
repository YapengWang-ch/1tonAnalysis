#include "ChannelInfo.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include <iostream>

using namespace std;

int main()
{
    TChain* to = new TChain("SimpleAnalysis");
    to->Add("/home/fuhaoyang/Mywork/one_ton_neutron/02_preAnalysis/43660/PreAnalysis_00043660.root");

    std::vector<ChannelInfo_t>* chinfo = nullptr;
    double TotalPE = 0;

    to->SetBranchAddress("ChannelInfo",&chinfo);
    to->SetBranchAddress("TotalPE",&TotalPE);

    TH1D* hcharge[60];
    TH1D* hped[60];
    TH1D* hPE[60];

    TH1D* htotalPE = new TH1D("htotalPE","htotalPE",200,0,1000);

    for(int ich=0;ich<60;++ich)
    {
        string hcname = "hcharge_" + to_string(ich);
        hcharge[ich] = new TH1D(hcname.c_str(),hcname.c_str(),150,0,3000);

        string hpdname = "hped_" + to_string(ich);
        hped[ich] = new TH1D(hpdname.c_str(),hpdname.c_str(),1000,940,980);

        string hpename = "hPE_" + to_string(ich);
        hPE[ich] = new TH1D(hpename.c_str(),hpename.c_str(),100,0,100);

    }

    int nentry = to->GetEntries();

    cout<<"nentry: "<<nentry<<endl;
    
    for(int ientry=0;ientry<nentry;++ientry)
    {
        int nch = chinfo->size();
        to->GetEntry(ientry);

        //cout<<"entry: "<<ientry<<"   nch:"<<nch<<endl;

        for(int ich=0;ich<nch;++ich)
        {
            int channelId = (*chinfo)[ich].ChannelId;
            hcharge[channelId]->Fill((*chinfo)[ich].Charge);
            hped[channelId]->Fill((*chinfo)[ich].Pedestal);
            hPE[channelId]->Fill((*chinfo)[ich].PE);
        }

        htotalPE->Fill(TotalPE);
    }

    //saveing 
    TFile* output_file1 = new TFile("/home/fuhaoyang/Mywork/one_ton_neutron/0A_test/preAna/charge.root","RECREATE");
    for(int ich=0;ich<60;++ich)
    {
        hcharge[ich]->Write();
        
    }
    output_file1->Close();

    TFile* output_file2 = new TFile("/home/fuhaoyang/Mywork/one_ton_neutron/0A_test/preAna/pedestal.root","RECREATE");
    for(int ich=0;ich<60;++ich)
    {
        hped[ich]->Write();
        
    }
    output_file2->Close();

    TFile* output_file3 = new TFile("/home/fuhaoyang/Mywork/one_ton_neutron/0A_test/preAna/PE.root","RECREATE");
    for(int ich=0;ich<60;++ich)
    {
        hPE[ich]->Write();
        
    }
    output_file3->Close();


    TFile* output_file4 = new TFile("/home/fuhaoyang/Mywork/one_ton_neutron/0A_test/preAna/TotalPE.root","RECREATE");
    htotalPE->Write();
    output_file4->Close();

    return 0;

}