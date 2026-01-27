#pragma once
#include "ChannelInfo.h"
#include "JPWaveformAdvprocess.h"
#include "JPWaveformPreprocess.h"
#include "TMath.h"

#ifndef _FindPeakAlgorithm_
#define _FindPeakAlgorithm_ wpp::GetPeaksFast
#endif

class PreAnalyzer
{
public :
	int WindowSize;
	int ped_upperlimit;
	int ped_lowerlimit;
	int frnt_blur;
	int back_blur;
	int bl_begin;
	int bl_end;
	int inte_begin;
	int inte_end;
	int end_begin;
	int end_end;
	bool is_baseline_unstable;
	int ThreMaskLen, ChargeMaskLen, PedMaskLen;
	bool* ThreMask;
	bool* ChargeMask;
	bool* PedMask;
	ChannelInfo_t chinfo;
	int SaturatedTime;
	PreAnalyzer() {};
	PreAnalyzer(int WindowSize, int front_baseline_end, int back_baseline_begin, int front_extension=10, int back_extension=50) : WindowSize(WindowSize), ped_upperlimit(0), ped_lowerlimit(0), frnt_blur(front_extension), back_blur(back_extension), bl_begin(0), bl_end(front_baseline_end), inte_begin(front_baseline_end), inte_end(back_baseline_begin), end_begin(back_baseline_begin), end_end(WindowSize), is_baseline_unstable(false)
	{
		ThreMask = new bool[WindowSize];
		ChargeMask = new bool[WindowSize];
		PedMask = new bool[WindowSize];
	}
	~PreAnalyzer()
	{
		delete[] ThreMask;
		delete[] PedMask;
		delete[] ChargeMask;
	}

	template<typename iterator>
	void WuYy_Calculate(iterator vbegin);
	template<typename iterator>
	void Ziyi_Calculate_Step1(iterator vbegin, int& avg, int& nc);
};

template<typename iterator>
void PreAnalyzer::WuYy_Calculate(iterator vbegin)
{
	is_baseline_unstable = wuyy_wap::GetThreMask(ThreMask,vbegin,WindowSize,ped_upperlimit,ped_lowerlimit,SaturatedTime)!=0; // Get ThreMask
	wuyy_wap::Dynamic_ExpandMask(ThreMask,ChargeMask,WindowSize,WindowSize-bl_end,frnt_blur,back_blur,5,20);
	double fullped, fullpedstd;
	wuyy_wap::GetPedMask(vbegin,ChargeMask,WindowSize,ped_upperlimit,PedMask,PedMaskLen);
	if(PedMaskLen<WindowSize/15) // in cas lenghth of PedMask too small
	{
		chinfo.FrontBslnStdDev = chinfo.BackBslnStdDev = TMath::StdDev(vbegin,vbegin+WindowSize);
		chinfo.BackBslnMean = chinfo.FrontBslnMean = chinfo.Pedestal = TMath::Mean(vbegin,vbegin+WindowSize);
	}
	else
	{
		wuyy_wap::GetPedinfo(vbegin,WindowSize,bl_begin,bl_end,PedMask,chinfo.FrontBslnMean,chinfo.FrontBslnStdDev);
		wuyy_wap::GetPedinfo(vbegin,WindowSize,inte_end,WindowSize,PedMask,chinfo.BackBslnMean,chinfo.BackBslnStdDev);
		wuyy_wap::GetPedinfo(vbegin,WindowSize,bl_begin,WindowSize,PedMask,fullped,fullpedstd);
		if(fullpedstd<chinfo.FrontBslnStdDev) chinfo.Pedestal = fullped;
		else chinfo.Pedestal = chinfo.FrontBslnMean;
	}
	chinfo.Charge = wuyy_wap::GetCharge(vbegin,ChargeMask,WindowSize,chinfo.Pedestal,ChargeMaskLen,bl_end,inte_end);

	auto peakIter = wpp::GetPeakFast(vbegin, vbegin+WindowSize);
	chinfo.Peak = chinfo.Pedestal-(*peakIter);
	auto peakList = _FindPeakAlgorithm_(vbegin,vbegin+WindowSize,chinfo.Pedestal,chinfo.FrontBslnStdDev*3);
	chinfo.nPeaks = peakList.size();
	for(auto&& item : peakList)
	{
		chinfo.PeakLoc.push_back(item-vbegin);
		chinfo.PeakAmp.push_back(chinfo.Pedestal-*item);
	}
	chinfo.RiseTime = wap::GetRiseTime(vbegin, vbegin+WindowSize, chinfo.Pedestal,  peakList, peakIter);
}

template<typename iterator>
void PreAnalyzer::Ziyi_Calculate_Step1(iterator vbegin, int& avg, int& nc)
{
	auto vend = vbegin + WindowSize;
	// Calculate Baseline
	double baseline = wap::GetPedestal(vbegin, vend, bl_begin, bl_end);
	chinfo.Pedestal = baseline;
	auto peakList = _FindPeakAlgorithm_(vbegin,vbegin+WindowSize,chinfo.Pedestal,5);
	auto peakIter = wpp::GetPeakFast(vbegin, vend);
	double peak = baseline-(*peakIter);
	chinfo.nPeaks = peakList.size();
	chinfo.Peak = peak;
	for(auto&& item : peakList)
	{
		chinfo.PeakLoc.push_back(item-vbegin);
		chinfo.PeakAmp.push_back(baseline-*item);
	}
	if(chinfo.nPeaks>0 && chinfo.PeakLoc[0]<=400)
	{
		avg += chinfo.PeakLoc[0];
		nc += 1;
	}
	chinfo.RiseTime = wap::GetRiseTime(vbegin, vend, baseline,  peakList, peakIter);

	// Data quality check
	chinfo.FrontBslnMean = wpp::GetPedestalFast(vbegin, vend, bl_begin, bl_end);
	chinfo.FrontBslnStdDev = TMath::StdDev(vbegin+bl_begin, vbegin+bl_end);
	chinfo.BackBslnMean = wpp::GetPedestalFast(vbegin, vend, end_begin, end_end);
	chinfo.BackBslnStdDev = TMath::StdDev(vbegin+end_begin, vbegin+end_end);
}
