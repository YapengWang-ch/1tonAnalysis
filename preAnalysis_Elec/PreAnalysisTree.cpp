/**
 * PreAnalysis Tree
 *
 * Mar. 10, 2018 Created by Ziyi Guo
 */

 #include "PreAnalysisTree.h"
 #include <sstream>
 #include "ChannelInfo.h"
 
 PreAnalysisTree::PreAnalysisTree(TChain* t):Invalid(0)
 {
	 ChannelInfo = nullptr;
	 m_mode = 1;
	 m_tree = t;
 
	 m_tree->SetBranchAddress("DetectorID", &DetectorID);
	 m_tree->SetBranchAddress("TriggerType", &TriggerType);
	 m_tree->SetBranchAddress("RunNo", &RunNo);
	 m_tree->SetBranchAddress("FileNo", &FileNo);
	 m_tree->SetBranchAddress("TriggerNo", &TriggerNo);
	 m_tree->SetBranchAddress("Sec", &Sec);
	 m_tree->SetBranchAddress("NanoSec", &NanoSec);
 
	 m_tree->SetBranchAddress("ChannelInfo", &ChannelInfo);
 
	 m_tree->SetBranchAddress("nSaturatedChannels", &nSaturatedChannels);
	 m_tree->SetBranchAddress("SaturatedChannel", SaturatedChannel);
	 m_tree->SetBranchAddress("SaturatedTime", SaturatedTime);
 
	 m_tree->SetBranchAddress("TotalPE", &TotalPE);
	 m_tree->SetBranchAddress("ReconX", &x);
	 m_tree->SetBranchAddress("ReconY", &y);
	 m_tree->SetBranchAddress("ReconZ", &z);
	 m_tree->SetBranchAddress("ReconE", &energy);
 
 }
 
 PreAnalysisTree::PreAnalysisTree(TString TreeName, TFile* file, TString mode)
 :Invalid(0)
 {
	 m_file = file;
	 ChannelInfo = nullptr;
 
	 if(mode=="recreate")
	 {
		 m_mode = 0;
		 m_tree = new TTree(TreeName,"Analysis Stream Tree");    /// create tree
 
		 m_tree->Branch("DetectorID", &DetectorID);
		 m_tree->Branch("TriggerType", &TriggerType);
		 m_tree->Branch("RunNo", &RunNo);
		 m_tree->Branch("FileNo", &FileNo);
		 m_tree->Branch("TriggerNo", &TriggerNo);
		 m_tree->Branch("Sec", &Sec);
		 m_tree->Branch("NanoSec", &NanoSec);
		 
		 m_tree->Branch("ChannelInfo", &ChannelInfo);
 
		 m_tree->Branch("nSaturatedChannels", &nSaturatedChannels);
		 m_tree->Branch("SaturatedChannel", SaturatedChannel, "SaturatedChannel[nSaturatedChannels]/I");
		 m_tree->Branch("SaturatedTime", SaturatedTime, "SaturatedTime[nSaturatedChannels]/I");
		 
		 m_tree->Branch("TotalPE", &TotalPE);
		 m_tree->Branch("ReconX", &x);
		 m_tree->Branch("ReconY", &y);
		 m_tree->Branch("ReconZ", &z);
		 m_tree->Branch("ReconE", &energy);
 
	 }
	 else if(mode=="read")
	 {
		 //truthList = new std::vector<JPSimTruthTree_t>;
		 m_mode = 1;
		 m_tree = (TTree*)m_file->Get(TreeName);
		 m_tree->SetBranchAddress("RunNo", &RunNo);
		 m_tree->SetBranchAddress("FileNo", &FileNo);
		 m_tree->SetBranchAddress("TriggerNo", &TriggerNo);
		 m_tree->SetBranchAddress("Sec", &Sec);
		 m_tree->SetBranchAddress("NanoSec", &NanoSec);
 
		 m_tree->SetBranchAddress("ChannelInfo", &ChannelInfo);
 
		 m_tree->SetBranchAddress("TotalPE", &TotalPE);
		 m_tree->SetBranchAddress("ReconX", &x);
		 m_tree->SetBranchAddress("ReconY", &y);
		 m_tree->SetBranchAddress("ReconZ", &z);
		 m_tree->SetBranchAddress("ReconE", &energy);
	 }
 }
 
 PreAnalysisTree::~PreAnalysisTree()
 {
	 //if(m_mode==1)
	 //	delete truthList;
 }
 
 
 int PreAnalysisTree::Reset()
 {
	 RunNo = Invalid;
	 FileNo = Invalid;
	 TriggerNo = Invalid;
 
	 Sec = Invalid;
	 NanoSec = Invalid;
 
	 ChannelInfo->clear();
 
	 nSaturatedChannels = Invalid;
	 TotalPE = Invalid;
 
	 x = Invalid;
	 y = Invalid;
	 z = Invalid;
	 energy = Invalid;
 
	 return 1;  // 1 for SUCCESS;
 }
 
 int PreAnalysisTree::Fill()
 {
	 m_tree->Fill();
	 return  1; // 1 for SUCCESS;
 }
 
 int PreAnalysisTree::Write()
 {
	 m_tree->Write("", TObject::kOverwrite);
	 return  1; // 1 for SUCCESS;
 }
 