/**
 * PreAnalysis Tree
 *
 * Mar. 10, 2018 Created by Ziyi Guo
 */

 #ifndef _PREANALYSIS_TREE_H_
 #define _PREANALYSIS_TREE_H_
 
 #include "TFile.h"
 #include "TTree.h"
 #include "TChain.h"
 #include <iostream>
 #include <iterator>
 #include "ChannelInfo.h"
 #include "Utils/JPUtils.h"
 
 using namespace std;
 
 class PreAnalysisTree
 {
 public:
	 /// Tree creation
	 PreAnalysisTree(TString TreeName, TFile* file, TString mode="recreate");
	 PreAnalysisTree(TChain* t);
	 ~PreAnalysisTree();
 
 public:
	 /// Reset all numbers into Invalid
	 int Reset();
	 /// Fill one entries
	 int Fill();
	 /// Close the root file
	 int Write();
 
 public:
 
	 /// Invalid number 
	 const int    Invalid;
 
	 /// Stream information
	 const static Int_t MAXCHANNEL = 50000;
	 const static Int_t MAXPARTICLE = 50000;
	 const static Int_t MAXPEAKS = 1000;
	 
	 Int_t DetectorID;
	 Int_t TriggerType;
	 Int_t RunNo;
	 Int_t FileNo;
	 Int_t TriggerNo;
	 Int_t Sec;
	 Int_t NanoSec;
	 std::vector<ChannelInfo_t>* ChannelInfo;
 
	 // Data quality variables
	 Int_t nSaturatedChannels;
	 Int_t SaturatedChannel[MAXCHANNEL];
	 Int_t SaturatedTime[MAXCHANNEL];
	 
	 // Simple Reconstruction, filled by reconstruction code
	 double TotalPE;
	 double x;
	 double y;
	 double z;
	 double energy;
 
	 TTree* GetTreePtr() {return m_tree;}
 
 private:
	 TFile* m_file;
	 TTree* m_tree;
	 Int_t m_mode;
 
 };
 
 #endif  // _PREANALYSIS_TREE_H_
 