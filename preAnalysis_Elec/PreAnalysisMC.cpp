#include <iostream>
#include "ChannelInfo.h"
#include "FindPeaksSG.h"
#include "TSystem.h"
#include "Utils/JPUtils.h"
#include "TStopwatch.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "JPWaveformPreprocess.h"
#include "JPWaveformAdvprocess.h"
#include "PreAnalysisTree.h"
#include "PreAnalyzer.h"
#include "Utils/PMTPosReader.h"
#include "VRecon.h"
#include "Recon1.h"
#include "Utils/ReadPMTCalib.h"
#include "JPSimOutput.hh"

#define NCHANNELS 60
#if !defined(_Ziyi_Algorithm_) && !defined(_WuYy_Algorithm_)
// #define _Ziyi_Algorithm_
#define _WuYy_Algorithm_
#endif

using namespace std;

void PrintUsage()
{
	cout << "Usage:" << endl;
	cout << "  ./PreAnalysisMC inputFilename outputFilename" << endl;
	cout << "  or" << endl;
	cout << "  ./PreAnalysisMC inputFilename outputFilename gaintable" << endl;
	cout << "  or" << endl;
	cout << "  ./PreAnalysisMC inputFilename_File0 start end outputFilename" << endl;
	cout << "  or" << endl;
	cout << "  ./PreAnalysisMC inputFilename_File0 start end outputFilename gaintable" << endl;
	cout << endl;
}

Int_t bl_begin = 0;
Int_t bl_end = 50; // [bl_begin, bl_end)
Int_t end_begin = 400;
Int_t end_end = 600; // [sideband_begin, sideband_end)
Int_t inte_begin = 40;
Int_t inte_end = 400; // [inte_begin, inte_end)

// double MCGainMap[60] = {477.88,436.21,453.43,473.45,447.42,463.73,
// 						442.34,430.88,461.23,480.57,428.36,458.56,
// 						454.54,480.51,450.56,466.63,424.57,461.33,
// 						436.97,463.79,460.55,460.99,459.76,441.64,
// 						445.63,454.94,476.36,438.27,477.84,461.21,
// 						459.73,444.83,460.00,427.75,448.28,461.35,
// 						476.97,446.89,455.63,460.85,452.33,464.05,
// 						491.10,465.44,448.28,464.26,424.40,452.04,
// 						465.64,448.72,475.72,439.47,468.26,492.73,
// 						449.41,462.88,425.97,449.09,448.51,450.92
// };
// double MCGainMap[60]={
// 	162.16, 147.96, 153.77, 160.55, 151.72, 157.27, 
// 	149.94, 146.06, 156.44, 162.97, 145.29, 155.59, 	
// 	154.15, 162.95, 152.77, 158.24, 143.96, 156.51, 
// 	148.27, 157.29, 156.24, 156.39, 156.00, 149.70, 
// 	151.18, 154.29, 161.61, 148.72, 162.12, 156.31, 
// 	155.79, 150.87, 156.06, 145.08, 152.09, 156.36, 
// 	161.96, 151.57, 154.53, 156.19, 153.49, 157.38, 
// 	166.59, 157.87, 151.99, 157.48, 143.90, 153.39, 
// 	157.94, 152.24, 161.40, 149.11, 158.83, 167.08, 
// 	152.48, 156.99, 144.49, 152.37, 152.17, 153.04
// };
// double MCGainMap[60]={
// 	151.7001, 141.2152, 220.0260, 187.7693, 225.8620, 150, 
// 	180.0983, 142.5006, 158.6087, 102.2138, 191.8023, 150, 
// 	179.5097, 216.6884, 165.2657, 140.8395, 239.7709, 395.5183, 
// 	150, 	  191.6133, 159.2593, 164.1045, 117.1244, 126.5382, 
// 	220.3577, 191.8501, 151.2114, 115.7509, 154.6573, 150, 
// 	169.4989, 109.5820, 171.8902, 146.3809, 162.8047, 201.2286, 
// 	176.0735, 171.9821, 142.8574, 173.1278, 150,     174.1935, 
// 	205.0106, 195.0382, 196.0038, 185.8085, 166.2746, 188.2126, 
// 	193.5043, 212.4113, 171.8706, 150,     176.9085, 150, 
// 	128.9216, 148.5185, 199.2825, 174.5274, 150,     178.5368
// };
double MCGainMap[60]={
    122.821, 101.082, 99.0382, 141.245, 178.273, 145.927,
    129.779, 150.843, 196.325, 90.1683, 180.958, 144.411,
    148.583, 120.759, 163.6, 123.453, 219.796, 181.039,
    165.634, 138.495, 113.775, 98.5437, 101.262, 72.8325,
    163.302, 135.873, 104.154, 88.0709, 95.2878, 107.545,
    107.833, 95.1998, 110.068, 97.7868, 103.107, 134.028,
    119.579, 104.408, 97.0003, 112.927, 139.475, 113.239,
    151.787, 156.117, 113.313, 101.998, 83.8239, 134.503,
    134.458, 176.356, 118.974, 119.32, 153.659, 143.958,
    161.3, 95.5576, 117.892, 95.2941, 118.227, 101.65
};
map<int, double> GainTable;
bool Read_Gain_Success = false;

void EventLoop(TString outputfilename, TChain *tc, TChain *tmc, TChain *tmc0)
{
	long totalEntries = tc->GetEntries();
	long truthEntries = tmc0->GetEntries();
	cout << "Total readout entries: " << totalEntries << endl;
	cout << "Total truth entries: " << truthEntries << endl;
	if (totalEntries <= 0 || truthEntries <= 0)
	{
		cout << "No entries found in this file, please check the inputpath." << endl;
		return;
	}
	
	TFile *file = new TFile(outputfilename, "recreate");
	PreAnalysisTree stream("SimpleAnalysis", file, "recreate");
	vector<JPSimTruthTree_t> *truthList = new vector<JPSimTruthTree_t>;

	// Activate only RunNo and TriggerNo
	tmc->SetBranchStatus("*", 0);
	for (auto activeBranchName : {"RunNo", "TriggerNo"})
		tmc->SetBranchStatus(activeBranchName, 1);
	Int_t Truth_RunNo, Truth_TriggerNo;
	tmc->SetBranchAddress("RunNo", &Truth_RunNo);
	tmc->SetBranchAddress("TriggerNo", &Truth_TriggerNo);

	TTreeReader TR(tc);
	TTreeReaderValue<int> RunNo(TR, "RunNo");
	TTreeReaderValue<int> TriggerNo(TR, "TriggerNo");
	TTreeReaderValue<int> Sec(TR, "Sec");
	TTreeReaderValue<int> NanoSec(TR, "NanoSec");
	TTreeReaderValue<std::vector<unsigned int>> ChannelId(TR, "ChannelId");
	TTreeReaderValue<std::vector<unsigned int>> Waveform(TR, "Waveform");

	Int_t entry = 0;
	Int_t nProcessCheck = TMath::Min(10000., totalEntries / 10.);
	if (nProcessCheck < 1)
		nProcessCheck = 1;

	std::map<Int_t, TVector3> pos;
	Double_t PMTcentercorr = 77;
	if (PMTPosRead("new_1t_Water", pos, PMTcentercorr) == 1)
		exit(1);

	VRecon *recon1 = new Recon1();
	recon1->SetPMTPosition(pos);
	recon1->SetEnergyCalibConst(1);

	TR.SetEntry(0);
	TR.Next();
	int16_t WindowSize = Waveform->size() / ChannelId->size();
	int16_t bl_end, inte_begin, inte_end;
	if (WindowSize >= 1000)
	{
		bl_end = inte_begin = 150, inte_end = 600;
	}
	else
	{
		bl_end = inte_begin = 40, inte_end = 400;
	}
	PreAnalyzer waveform_analyzer(WindowSize, bl_end, inte_end);
	ChannelInfo_t &chinfo = waveform_analyzer.chinfo;

	vector<double> PEList, FirstHitTimeList;
	PEList.resize(NCHANNELS);
	TR.Restart();

	while (TR.Next())
	{
		if (entry % nProcessCheck == 0)
			cout << "Processing entry " << entry << " (" << entry / (double)totalEntries * 100 << "%)" << endl;

		stream.Reset();
		stream.RunNo = *(RunNo);
		stream.FileNo = jputils::GetFileNumber(tc->GetFile()->GetName());
		stream.TriggerNo = *(TriggerNo);
		stream.Sec = *(Sec);
		stream.NanoSec = *(NanoSec);
		int nChannels = (*ChannelId).size();
		int windowSize = 1000;
		if (nChannels > 0)
			windowSize = (*Waveform).size() / nChannels;

#ifdef _Ziyi_Algorithm_
		double avg = 0;
		int nc = 0;
#endif
		for (Int_t j = 0; j < nChannels; j++)
		{
			chinfo.Reset();
			UInt_t PMTId = (*ChannelId)[j];
			if (PMTId >= NCHANNELS) // avoid no PMT channel signal
			{
				(*ChannelId).erase((*ChannelId).begin() + j);
				j--;
				nChannels--;
				continue;
			}
			chinfo.ChannelId = PMTId;
			// auto vbegin = (*Waveform).begin()+WindowSize*j;
			auto vbegin = &((*Waveform)[0]) + WindowSize * j;

#ifdef _Ziyi_Algorithm_
			waveform_analyzer.Ziyi_Calculate_Step1(vbegin, avg, nc);
#endif
#ifdef _WuYy_Algorithm_
			waveform_analyzer.WuYy_Calculate(vbegin);
			if (waveform_analyzer.is_baseline_unstable || isnan(waveform_analyzer.chinfo.Pedestal) || isnan(waveform_analyzer.chinfo.Charge)) // exception catching
			{
				cout << "Entry=" << entry << ", TriggerNo=" << stream.TriggerNo << ", PMTID=" << PMTId << ", Ped=" << waveform_analyzer.chinfo.Pedestal << ", PedStd" << waveform_analyzer.chinfo.FrontBslnStdDev << ", Charge" << waveform_analyzer.chinfo.Charge;
				cout << endl;
			}
#endif
			stream.ChannelInfo->emplace_back(waveform_analyzer.chinfo);
			// Saturated channels
			if (waveform_analyzer.SaturatedTime > 0)
			{
				stream.SaturatedChannel[stream.nSaturatedChannels] = PMTId;
				stream.SaturatedTime[stream.nSaturatedChannels] = waveform_analyzer.SaturatedTime;
				stream.nSaturatedChannels++;
			}
		}
#ifdef _Ziyi_Algorithm_
		if (nc > 0)
			avg /= nc;
		else
			avg = 100;
#endif

		vector<unsigned int> ChannelId;
		for (int j = 0; j < nChannels; j++)
		{
			ChannelInfo_t &chinfo = (*(stream.ChannelInfo))[j];
#ifdef _Ziyi_Algorithm_
			auto vbegin = Waveform->begin() + windowSize * j;
			auto vend = vbegin + windowSize;
			// Calculate charge
			chinfo.Charge = wap::GetCharge(vbegin, vend, chinfo.Pedestal, inte_begin, inte_end, chinfo.PeakLoc, avg);
#endif
			if (Read_Gain_Success)
				chinfo.PE = chinfo.Charge / GainTable[chinfo.ChannelId];
			else
				chinfo.PE = chinfo.Charge / MCGainMap[chinfo.ChannelId];
			stream.TotalPE += chinfo.PE;
			PEList.push_back(chinfo.PE);
			ChannelId.push_back(chinfo.ChannelId);
			// FirstHitTimeList.push_back(chinfo.RiseTime);
		}
		recon1->Reconstruct(&ChannelId, &PEList, &FirstHitTimeList);
		stream.x = recon1->GetX();
		stream.y = recon1->GetY();
		stream.z = recon1->GetZ();
		stream.energy = recon1->GetVisibleEnergy();

		tmc->GetEntry(entry);
		// vector<JPSimPrimaryParticle_t> PrimaryParticleList = (*truthList)[0].PrimaryParticleList;
		if (Truth_RunNo != *(RunNo) || Truth_TriggerNo != *(TriggerNo))
		{
			cout << "The trigger number is not matched!" << endl;
			cout << "MC truth RunNo: " << Truth_RunNo << endl;
			cout << "Waveform RunNo: " << *(RunNo) << endl;
			cout << "MC truth TriggerNo: " << Truth_TriggerNo << endl;
			cout << "Waveform TriggerNo: " << *(TriggerNo) << endl;
			cout << endl;

			if (Truth_TriggerNo>*(TriggerNo))
			{
				entry--;
			}else if (Truth_TriggerNo<*(TriggerNo))
			{
				entry++;
			}
		}
		// tTruth->Fill();
		stream.Fill();
		entry++;
	}
	stream.Write();
	// Clone SimTruth: all 
	tmc->SetBranchStatus("*", 1); // all branches active
	TTree *tTruth = tmc->CloneTree();
	tTruth->SetTitle("MCTruth");
	tTruth->SetName("MCTruth");
	tTruth->Write();
	// tmc->SetBranchStatus("*", 0);
	// for (auto activeBranchName : {"truthList*", "PEList*", })
	// 	tmc->SetBranchStatus(activeBranchName, 1);
	// TTree *tTruth = tmc->CloneTree();
	// tTruth->SetTitle("MCTruth");
	// tTruth->SetName("MCTruth");
	// tTruth->Write();
	file->Close();
	delete truthList;
	delete recon1;
	delete file;
}

int main(int argc, char **argv)
{
	TString outputfilename;
	TString inputFilename;
	int start = 0, end = 0;
	if (argc == 3 || argc == 4)
	{
		inputFilename = argv[1];
		outputfilename = argv[2];
		if (argc == 4)
			Read_Gain_Success = ReadPMTGain(argv[3], GainTable) == 0;
	}
	else if (argc == 5)
	{
		inputFilename = argv[1];
		start = TString(argv[2]).Atoi();
		end = TString(argv[3]).Atoi();
		outputfilename = argv[4];
		if (argc == 6)
			Read_Gain_Success = ReadPMTGain(argv[5], GainTable) == 0;
	}
	else
	{
		PrintUsage();
		return 1;
	}

	gSystem->Load("libCHANNELINFO.so");
	gSystem->Load("libXMLREADER.so");

	TChain *tc = new TChain("Readout");
	TChain *tmc = new TChain("SimTriggerInfo");
	TChain *tmc0 = new TChain("SimTruth");

	int nFlag = 0;
	cout << "Adding files... " << flush;
	// inputFilename.Remove(inputFilename.Length() - 5, 5);
	// TString newName;
	// 	if (i == 0)
	// 		newName = inputFilename + ".root";
	// 	else
	// 		newName = inputFilename + TString::Format("_%d.root", i);
		if (gSystem->AccessPathName(inputFilename))
		{
			cout << "Cannot Add More files.";
			// break;
		}
		tc->Add(inputFilename);
		tmc->Add(inputFilename);
		tmc0->Add(inputFilename);
		nFlag++;
	if (nFlag == 0)
	{
		cout << "PreAnalysis terminated." << endl;
		return 1;
	}

	cout << "Added " << nFlag << " files." << endl;
	EventLoop(outputfilename, tc, tmc, tmc0);
	delete tc;
	delete tmc;
	delete tmc0;
}
