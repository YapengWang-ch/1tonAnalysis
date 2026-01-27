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

#define NCHANNELS 60
#if !defined(_Ziyi_Algorithm_) && !defined(_WuYy_Algorithm_)
// #define _Ziyi_Algorithm_
#define _WuYy_Algorithm_
#endif

using namespace std;

void PrintUsage()
{
	cout << "Usage:" << endl;
	cout << "  ./PreAnalysisData RunNo StartFileNo EndFileNo OutputDir" << endl;
	cout << "  or  ./PreAnalysisData RunNo outputFilename" << endl;
	cout << "  or  ./PreAnalysisData filename outputFilename" << endl;
	cout << endl;
}

Int_t Filename_to_RunNo(TString filename)
{
	filename.Remove(filename.Last('.'));
	string fn = filename.Data();
	int pos = fn.find_last_not_of("0123456789");
	return atoi(fn.substr(pos + 1).data());
}

void EventLoop(TString outputfilename, TChain *tc, int16_t runNo);

map<Int_t, TVector3> pos;
map<int, double> GainTable;
bool Read_Gain_Success;
VRecon *recon1 = new Recon1();

int main(int argc, char **argv)
{
	Int_t runNo = -1;
	TString outputdir;
	TString inputFilename;
	TString outputfilename;
	int16_t startno = -1;
	int16_t endno = -1;
	if (argc == 3)
	{
		inputFilename = argv[1];
		outputfilename = argv[2];
	}
	else if (argc == 5)
	{
		inputFilename = argv[1];
		startno = TString(argv[2]).Atoi();
		endno = TString(argv[3]).Atoi();
		outputdir = argv[4];
	}
	else
	{
		PrintUsage();
		return 1;
	}

	gSystem->Load("libCHANNELINFO.so");
	gSystem->Load("libXMLREADER.so");
	Double_t PMTcentercorr = 77;
	if (PMTPosRead("new_1t_Water", pos, PMTcentercorr) == 1)
		exit(1);

	recon1->SetPMTPosition(pos);
	recon1->SetEnergyCalibConst(1);

	TStopwatch w;
	w.Start();
	TChain *tc;
	if (argc == 3) // runNo, ouputfilename
	{
		tc = new TChain("Readout");
		int16_t nFlag = -1;
		if (inputFilename.IsAlnum())
		{
			runNo = inputFilename.Atoi();
			nFlag = jputils::ReadRawDataRootFiles(runNo, tc);
		}
		else
		{
			runNo = Filename_to_RunNo(inputFilename);
			nFlag = tc->Add(inputFilename);
		}
		if (nFlag <= 0)
		{
			cout << "Cannot find this run or file." << endl;
		}
		Read_Gain_Success = ReadPMTGain("../calibResult/GainList.csv", GainTable) == 0;
		cout << "GainPath: " << "../calibResult/GainList.csv" << endl;
		cout << "Added " << nFlag << " files." << endl;
		EventLoop(outputfilename, tc, runNo);          
		delete tc;
	}
	else if (argc == 5) // runNo,startfileno,endfileno,ouputdir
	{
		if (inputFilename.IsAlnum())
			runNo = inputFilename.Atoi();
		else
		{
			PrintUsage();
			return 1;
		}
		Read_Gain_Success = ReadPMTGain(runNo, GainTable) == 0;
		for (int16_t fn = startno; fn <= endno; fn++)
		{
			tc = new TChain("Readout");
			cout << "File number: " << fn << endl;
			int16_t nFlag;
			nFlag = jputils::ReadRawDataRootFiles(runNo, tc, fn, fn);
			if (nFlag == -1)
			{
				cout << "Cannot find this file number." << endl;
				delete tc;
				break;
			}
			cout << "Added " << nFlag << " files." << endl;
			outputfilename = outputdir + TString::Format("/PreAnalysis_Run%d_File%d.root", runNo, fn);
			EventLoop(outputfilename, tc, runNo);
			delete tc;
		}
	}

	// for (int i = 0; i < 60; i++)
	// {
	// 	cout << "Gain Table: PMT " << i << ", Gain=" << GainTable[i] << endl;
	// }
	w.Stop();
	std::cout << "\nTime: \t" << w.RealTime() << " , " << w.CpuTime() << std::endl;
	cout << "Finish" << endl;
}
void EventLoop(TString outputfilename, TChain *tc, int16_t runNo)
{
	TFile *file = new TFile(outputfilename, "recreate");
	PreAnalysisTree stream("SimpleAnalysis", file);

	// 确保 TChain 已经正确加载
	tc->LoadTree(0);
	
	// 创建 TTreeReader，但要确保 TChain 的当前树已经设置
	TTreeReader TR(tc);
	
	// 检查 TTreeReader 是否有效
	if (TR.IsInvalid()) {
		cout << "Error: TTreeReader is not valid!" << endl;
		file->Close();
		delete file;
		return;
	}
	
	// 设置 TTreeReader 的值
	TTreeReaderValue<int> DetectorID(TR, "DetectorID");
	TTreeReaderValue<int> TriggerType(TR, "TriggerType");
	TTreeReaderValue<int> RunNo(TR, "RunNo");
	TTreeReaderValue<int> TriggerNo(TR, "TriggerNo");
	TTreeReaderValue<int> Sec(TR, "Sec");
	TTreeReaderValue<int> NanoSec(TR, "NanoSec");
	TTreeReaderValue<std::vector<unsigned short>> ChannelId(TR, "ChannelId");
	TTreeReaderValue<std::vector<unsigned short>> Waveform(TR, "Waveform");

	long totalEntries = tc->GetEntries();
	if (totalEntries <= 0)
	{
		cout << "No entries found in this file, please check the inputpath." << endl;
		return;
	}
	cout << "Total readout entries: " << totalEntries << endl;
	Int_t entry = 0;
	Int_t nProcessCheck = TMath::Min(10000., totalEntries / 10.);
	if (nProcessCheck < 1)
		nProcessCheck = 1;

	// 首先移动到第一个有效的条目来获取 WindowSize
	if (!TR.Next()) {
		cout << "No entries to read!" << endl;
		file->Close();
		delete file;
		return;
	}
	
	// 检查 ChannelId 和 Waveform 是否有效
	if (ChannelId->size() == 0) {
		cout << "Warning: ChannelId is empty in first entry!" << endl;
	}
	
	int16_t WindowSize = 0;
	if (Waveform->size() > 0 && ChannelId->size() > 0) {
		WindowSize = Waveform->size() / ChannelId->size();
	} else {
		cout << "Error: Cannot determine WindowSize!" << endl;
		file->Close();
		delete file;
		return;
	}
	
	cout << "WindowSize: " << WindowSize << endl;
	int16_t bl_end, inte_begin, inte_end;
	if (WindowSize >= 1000)
	{
		bl_end = inte_begin = 60, inte_end = 600;
	}
	else
	{
		bl_end = inte_begin = 60, inte_end = 800;
	}
	PreAnalyzer waveform_analyzer(WindowSize, bl_end, inte_end);
	ChannelInfo_t &chinfo = waveform_analyzer.chinfo;

	vector<double> PEList, FirstHitTimeList;
	PEList.resize(NCHANNELS);
	
	// 重置 TTreeReader 到开始位置
	TR.Restart();
	
	// 获取文件编号
	Int_t fileNo = 0;
	if (tc->GetCurrentFile()) {
		fileNo = jputils::GetFileNumber(tc->GetCurrentFile()->GetName());
	}
	stream.FileNo = fileNo;

	while (TR.Next())
	{
		if (entry % nProcessCheck == 0)
		{
			cout << "Processing entry " << entry << " (" << entry / (double)totalEntries * 100 << "%)" << endl;
			stream.Write();
		}
		entry++;
		// if (*RunNo!=50991) continue; // test only run 47269
		// if (*TriggerNo!=1296) continue;
		stream.Reset();
		stream.DetectorID = *DetectorID;
		stream.TriggerType = *TriggerType;
		stream.RunNo = *RunNo;
		stream.TriggerNo = *TriggerNo;
		stream.Sec = *Sec;
		stream.NanoSec = *NanoSec;
		
		// 确保 ChannelId 和 Waveform 的数据是有效的
		if (!DetectorID.Get() || !ChannelId.Get() || !Waveform.Get()) {
			cout << "Warning: Null data at entry " << entry << endl;
			continue;
		}
		
		int nChannels = (*ChannelId).size();
		if (nChannels == 0)
			continue;
		stream.nSaturatedChannels = 0;
		PEList.clear();
#ifdef _Ziyi_Algorithm_
		int avg, nc = 0;
#endif

		for (Int_t j = 0; j < nChannels; j++)
		{
			chinfo.Reset();
			UInt_t PMTId = (*ChannelId)[j];
			// cout << "Processing PMTId: " << PMTId << endl;
			// if (PMTId!=3) continue; // test only PMT 2
			if (PMTId >= NCHANNELS) // avoid no PMT channel signal
			{
				continue;
			}
			chinfo.ChannelId = PMTId;
			
			if (j * WindowSize >= Waveform->size()) {
				cout << "Warning: Waveform index out of bounds at entry " << entry 
					 << ", channel " << j << endl;
				continue;
			}
			
			auto vbegin = &((*Waveform)[0]) + WindowSize * j;

#ifdef _Ziyi_Algorithm_
			waveform_analyzer.Ziyi_Calculate_Step1(vbegin, avg, nc);
#endif
#ifdef _WuYy_Algorithm_
			waveform_analyzer.WuYy_Calculate(vbegin);
			if (waveform_analyzer.is_baseline_unstable || 
				std::isnan(waveform_analyzer.chinfo.Pedestal) || 
				std::isnan(waveform_analyzer.chinfo.Charge)) // exception catching
			{
				// 可以添加调试输出
			}
#endif
			stream.ChannelInfo->emplace_back(waveform_analyzer.chinfo);
			
			if (waveform_analyzer.SaturatedTime > 0)
			{
				if (stream.nSaturatedChannels < NCHANNELS) {
					stream.SaturatedChannel[stream.nSaturatedChannels] = PMTId;
					stream.SaturatedTime[stream.nSaturatedChannels] = waveform_analyzer.SaturatedTime;
					stream.nSaturatedChannels++;
				}
			}
		}
#ifdef _Ziyi_Algorithm_
		if (nc > 0)
			avg /= nc;
		else
			avg = 100;
#endif
		for (int j = 0; j < stream.ChannelInfo->size(); j++)
		{
			ChannelInfo_t *chinfo = &(*stream.ChannelInfo)[j];
			UInt_t PMTId = chinfo->ChannelId;
			
#ifdef _Ziyi_Algorithm_
			// 这里需要确保 Waveform 数据仍然有效
			if (j < nChannels) {
				auto vbegin = &((*Waveform)[0]) + WindowSize * j;
				auto vend = vbegin + WindowSize;
				chinfo.Charge = wap::GetCharge(vbegin, vend, chinfo.Pedestal, 
											   inte_begin, inte_end, chinfo.PeakLoc, avg);
			}
#endif
			if (Read_Gain_Success && GainTable.find(PMTId) != GainTable.end())
			{
				double gain = GainTable[PMTId];
				if (gain > 0) {
					chinfo->PE = chinfo->Charge / gain;
					stream.TotalPE += chinfo->PE;
					PEList.push_back(chinfo->PE);
				}
			}
		}
		
		// SimpleRecon (BarryCenter Algorithm)
		if (Read_Gain_Success && PEList.size() > 0)
		{
			std::vector<unsigned int> tempChannelId(ChannelId->begin(), ChannelId->end());
			if (tempChannelId.size() == PEList.size()) {
				recon1->Reconstruct(&tempChannelId, &PEList, &FirstHitTimeList);
				stream.energy = recon1->GetVisibleEnergy();
				stream.x = recon1->GetX();
				stream.y = recon1->GetY();
				stream.z = recon1->GetZ();
			}
		}
		stream.Fill();
	}
	stream.Write();
	file->Close();
	cout << "Finish Calculation" << endl;
	delete file;
}