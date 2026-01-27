#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <set>
#include "TMath.h"
#include "TMinuit.h"
#include "TH1D.h"
#include "TF1.h"
#include "JPWaveformPreprocess.h"

#define Threshold 100
// 1960/91=
using namespace std;
namespace wap {
    

template<typename iterator>
double GetPedestal(iterator vbegin, iterator vend, size_t nbegin=0, size_t nend=100)
{
    int step = nend-nbegin;
    int windowsize = vend-vbegin;
    int m = min(3, windowsize/step);
    if(windowsize<1000) m=min(6, windowsize/step);
    double* stdList = new double[m];
    for(int i=0; i<m; i++)
    {
        //stdList[i] = TMath::StdDev(vbegin+nbegin+i*step, vbegin+nend+i*step);
        stdList[i] = *max_element(vbegin+nbegin+i*step, vbegin+nend+i*step)-*min_element(vbegin+nbegin+i*step, vbegin+nend+i*step);
        double mean = accumulate(vbegin+nbegin+i*step, vbegin+nend+i*step, 0.0)/step;
        if(mean<900) stdList[i] = 1000;
        //cout<<i<<"\t"<<stdList[i]<<endl;
    }
    //stdList[1] = TMath::StdDev(vbegin+nend, vbegin+min(int(nend+step), windowsize));
    //stdList[2] = TMath::StdDev(vbegin+nend+step, vbegin+min(int(nend+2*step), windowsize));
    int minstdIdx = min_element(stdList, stdList+m)-stdList;
    //cout<<"std1: "<<stdList[0]<<endl;
    //cout<<"std2: "<<stdList[1]<<endl;
    //cout<<"std3: "<<stdList[2]<<endl;
    double baseline;
    baseline = wpp::GetPedestal(vbegin, vend, nbegin+minstdIdx*step, nend+minstdIdx*step);
    delete[] stdList;
    return baseline;
}

template<typename iterator>
double GetRiseTime(iterator vbegin, iterator vend, double baseline, vector<iterator>& peakList, iterator peakIter)
{
    iterator firstPeakIter = peakIter;
    if(peakList.size()>0)
    {
        for(int i=0; i<peakList.size(); i++)
        {
            if(baseline-(*peakList[i])>0.1*(baseline-(*peakIter)))
            {
                firstPeakIter = peakList[i];
                break;
            }
        }
    }
    else
        firstPeakIter = peakIter;
    double firstPeak = baseline-(*firstPeakIter);
    double peakForTimeSearch = firstPeak;
    //if(firstPeak<peakMax/10) peakForTimeSearch = peakMax;
    //cout<<"peakForTimeSearch: "<<peakForTimeSearch<<endl;
    double First10pctTime = wpp::GetFirstOverXpctTime(vbegin, vend, peakForTimeSearch, baseline,  10, 1, vend-vbegin);
    return First10pctTime;
    
}

template<typename iterator>
double GetFitRiseTime(iterator vbegin, iterator vend, double baseline, vector<iterator>& peakList, iterator peakIter,
                    double& Chi2, double& Height, double& Mean, double& Sigma, bool& status)
{
    iterator firstPeakIter;
    if(peakList.size()>0)
        firstPeakIter = peakList[0];
    else
        firstPeakIter = peakIter;
    int peakLoc = firstPeakIter-vbegin;
    double peakHeight = baseline-(*firstPeakIter);
    int windowSize = vend-vbegin;
    int fitstart = max(peakLoc-25, 0);
    int r1 = 2;
    if(*firstPeakIter==0) r1 = -1;
    //cout<<"r1: "<<r1<<endl;
    int fitend = min(peakLoc+r1, windowSize);
    TF1* func = wpp::FitTime(vbegin+fitstart, vbegin+fitend, baseline, status);

    int hlength = fitend-fitstart;
    Chi2 = func->GetChisquare();
    Height = func->GetParameter(0);
    Mean = func->GetParameter(1)+fitstart;
    Sigma = func->GetParameter(2);
    double peak = func->GetMinimum(0, hlength);
    double peakx = func->GetMinimumX(0, hlength);
    double TimeFit10 = func->GetX(0.1*peak, 0, peakx)+fitstart;
    delete func;
    //if(status)
        return TimeFit10;
    //else return  wpp::GetFirstOverXpctTime(vbegin, vend, peakHeight, baseline,  10, 0, vend-vbegin);

}

template<typename iterator>
double GetCharge(iterator vbegin, iterator vend, double baseline, size_t inte_begin, size_t inte_end, vector<iterator>& peakList)
{
    int wvl = vend-vbegin;
    double charge0 =  wpp::GetChargeFast(vbegin, vend, baseline, inte_begin, inte_end);
   
    // No peak or the only peak is out of the integral range
    if(peakList.size()==0 || ( peakList.size()==1 && (peakList[0]-vbegin<inte_begin || peakList[0]-vbegin>inte_end))
       || ( peakList.size()>0 && peakList[0]-vbegin>inte_end) )
    {
        int peakLoc = wpp::GetPeakFast(vbegin+inte_begin, vbegin+inte_end)-vbegin;
        double charge1 =  wpp::GetChargeFast(vbegin, vend, baseline, peakLoc-10, peakLoc+20);
        return charge1;
    }

    // Other situation
    // Find the first peak after inte_begin. If inte_begin is in the waveform cluster
    int firstLoc = peakList[0]-vbegin;
    for(size_t i=0; i<peakList.size(); i++)
    {
        auto item = peakList[i];
        int loc = item-vbegin;
        if(loc>inte_begin || (i<peakList.size()-1 && abs(peakList[i+1]-item)<50) )
        {
            firstLoc = loc;
            break;
        }
    }
    //cout<<"Find here "<<firstLoc<<endl;
    if(firstLoc>inte_end)
    {
        int peakLoc = wpp::GetPeakFast(vbegin+inte_begin, vbegin+inte_end)-vbegin;
        double charge1 =  wpp::GetChargeFast(vbegin, vend, baseline, peakLoc-10, peakLoc+20);
        return charge1;
    }
    //cout<<"First Loc: "<<firstLoc<<endl;
    int j = firstLoc;
    for(; j>=0; j--)
    {
        double wv = baseline-*(vbegin+j);
        if(wv<Threshold)
        {
            break;
        }
    }
    int inteL = max(j-10, 0);
   
    int lastLoc = peakList.back()-vbegin;
    for(int i=peakList.size()-1; i>=0; i--)
    {
        auto item = peakList[i];
        int loc = item-vbegin;
        if(loc<inte_end)
        {
            lastLoc = loc;
            break;
        }
    }
    //cout<<"Last Loc: "<<lastLoc<<endl;
    j = lastLoc;
    for(; j<inte_end+20; j++)
    {
        double wv = baseline-*(vbegin+j);
        if(wv<Threshold)
        {
            break;
        }
    }
    int inteR = min(j+20, (int)inte_end);
    //cout<<"InteL: "<<inteL<<endl;
    //cout<<"InteR: "<<inteR<<endl;
    if(inteL>inteR)
    {
        //cout<<"Left: "<<inteL<<endl;
        //cout<<"Right: "<<inteR<<endl;
        //for(size_t i=0; i<peakList.size(); i++)
        //{
        //    auto item = peakList[i];
        //    int loc = item-vbegin;
        //    cout<<loc<<endl;
        //}
        int peakLoc = wpp::GetPeakFast(vbegin+inte_begin, vbegin+inte_end)-vbegin;
        double charge1 =  wpp::GetChargeFast(vbegin, vend, baseline, peakLoc-10, peakLoc+20);
        return charge1;
    }
    double charge1 =  wpp::GetChargeFast(vbegin, vend, baseline, inteL, inteR);
    return charge1;

}



template<typename iterator>
double GetCharge(iterator vbegin, iterator vend, double baseline, size_t inte_begin, size_t inte_end, vector<int> PeakLoc, double avg)
{
    int wvl = vend-vbegin;
    int peakLoc = wpp::GetPeakFast(vbegin+inte_begin, vbegin+inte_end)-vbegin;
    double charge1 =  wpp::GetChargeFast(vbegin, vend, baseline, peakLoc-10, peakLoc+20);

    if(PeakLoc.size()==0)
    {
        return charge1;
    }
    int firstLoc = PeakLoc[0];
    if(firstLoc>inte_end)
    {
        return charge1;
    }
    else if(firstLoc<inte_end)
    {
        if(firstLoc-avg<-60)
        {
            bool success = false;
            for(int i=1; i<PeakLoc.size(); i++)
            {
                int loc = PeakLoc[i];
                if(loc-avg>-60 || loc>=inte_begin)
                {
                    firstLoc = loc;
                    success = true;
                    break;
                }
            }
            if(!success)
            {
                return charge1;
            }
        }
    }
    
    //cout<<"First Loc: "<<firstLoc<<endl;
    int j = firstLoc;
    for(; j>=0; j--)
    {
        double wv = baseline-*(vbegin+j);
        if(wv<Threshold)
        {
            break;
        }
    }
    int inteL = max(j-10, 0);
    
    int lastLoc = PeakLoc[0];
    for(int i=0; i<PeakLoc.size(); i++)
    {
        if(PeakLoc[i]<=inte_end)
        {
            lastLoc = PeakLoc[i];
        }
    }
    
    //cout<<"Last Loc: "<<lastLoc<<endl;
    j = lastLoc;
    for(; j<inte_end+20; j++)
    {
        double wv = baseline-*(vbegin+j);
        if(wv<Threshold)
        {
            break;
        }
    }
    int inteR = min(j+20, (int)inte_end);
    //cout<<"InteL: "<<inteL<<endl;
    //cout<<"InteR: "<<inteR<<endl;
    if(inteL>inteR)
    {
        //cout<<"Left: "<<inteL<<endl;
        //cout<<"Right: "<<inteR<<endl;
        //for(size_t i=0; i<PeakLoc.size(); i++)
        //{
        //    int loc = PeakLoc[i];
        //    cout<<loc<<endl;
        //}
        return charge1;
    }
    double charge2 =  wpp::GetChargeFast(vbegin, vend, baseline, inteL, inteR);
    return charge2;

}

}

#define MAX_ADC 16383
namespace wuyy_wap {

// estimate pedestal range, and using the estimated threshold, find the over-threshold point marked as true in ThreMask
template<typename iterator>
uint8_t GetThreMask(bool* ThreMask,iterator v_begin,int WindowSize,int& ped_upperlimit,int& ped_lowerlimit, int& SaturatedTime)
{
	int histogram[MAX_ADC+1]={0};
	for(int i=0;i<WindowSize;i++)
	{
		histogram[*(v_begin+i)]++;
	} //  draw histogram of waveform ADC value
	SaturatedTime=histogram[0];
	
	double ped_mean=0, ped_std=0;
	int count=0;
	int most_ADC = max_element(histogram+1, histogram+MAX_ADC) - histogram;
	int most_ADC_statistic = histogram[most_ADC];
	int bottom=1, top=MAX_ADC; //  bottom and top : pedestal estimation range, is full-width-1/2.51164-maximum
    // cout << "Most ADC: " << most_ADC << ", statistic: " << most_ADC_statistic << endl;
	for(;histogram[bottom]<most_ADC_statistic/2.51164;bottom++);
	for(;histogram[top]<most_ADC_statistic/2.51164;top--);
    // cout << "Pedestal estimation range: " << bottom << " to " << top << endl;
	for(int i=bottom;i<top+1;i++)
	{
		count += histogram[i];
		ped_mean += histogram[i]*i;
		ped_std += (double)histogram[i]*i*i;
	}
	ped_mean = ped_mean/count;
	ped_std = TMath::Sqrt(ped_std/count - ped_mean*ped_mean); // calculate estimated ped and pedstd
    // cout << "Initial Pedestal estimation: mean = " << ped_mean << ", std = " << ped_std << endl;
    
    // for (int i=14800; i<14950; i++) {
    //     cout << "Histogram[" << i << "] = " << histogram[i] << endl;
    // }
	// in most cases there are only 3-4 bins. Consider baseline noise as gaussian distribution and reshape it with 3~4 bins, the actual calculated std is larger than real gaussian std. So calculated_ped_std*2.20308 = 3*real_ped_std
	// ped_std = TMath::Max(ped_std*2.20308,1.0); // in case std too small (range collapse into a point)
	// ped_std = TMath::Max(ped_std*5,1.0); // in case std too small (range collapse into a point)
    ped_std=Threshold;
	ped_lowerlimit = TMath::Nint(ped_mean - ped_std);
	ped_upperlimit = TMath::Nint(ped_mean + ped_std); // Nint: round function
    // cout << "Pedestal estimation: mean = " << ped_mean << ", std = " << ped_std << ", lower limit = " << ped_lowerlimit << ", upper limit = " << ped_upperlimit << endl;
	for(int i=0;i<WindowSize;i++)
	{
		ThreMask[i] = *(v_begin+i)<ped_lowerlimit;
        // if (ThreMask[i])
        // cout << "Waveform[" << i << "] = " << *(v_begin+i) << ", ThreMask = " << ThreMask[i] << endl;
	}
	return 0;
}

// for every over threshold area, expand it. Extend frnt_blur(defalut=10) ns in the front. The expanded mask is called ChargeMask for charge integration
void ExpandMask(bool* ThreMask, bool* ChargeMask,int WindowSize,int frnt_blur=10,int back_blur=50)
{
	copy(ThreMask, ThreMask+WindowSize, ChargeMask);
	for(int i=1;i<WindowSize;i++)
	{
		// if(!ThreMask[i-1]) continue;
		if(!ThreMask[i]){
            // cout << "remove ChargeMask at: " << i-1 << endl;
            continue; //  only two adjacent over threshold point are treated as a actual over threshold area
        } 
		for(int j=TMath::Max(i-frnt_blur,0);j<i;j++)  
        {    ChargeMask[j] = true;
            //  cout << "Expanding ChargeMask at front: " << j << endl;
        }
        // ChargeMask[i-1] = true; // ensure the point before over-threshold point is also included
		while(ThreMask[i] && i<WindowSize) {
            ChargeMask[i] = true;
            i++;
        }
		for(int j=i;j<TMath::Min((Int_t)(i+back_blur),(Int_t)WindowSize);j++){   ChargeMask[j] = true;
                // cout << "Expanding ChargeMask at back: " << j << endl;
        }
        break; // only expand the first over-threshold area
	}
}

void ExpandMask_full(bool* ThreMask, bool* ChargeMask,int WindowSize,int frnt_blur=10,int back_blur=50)
{
	copy(ThreMask, ThreMask+WindowSize, ChargeMask);
	for(int i=1;i<WindowSize;i++)
	{
		if(!ThreMask[i-1]) continue;
		if(!ThreMask[i])continue; //  only two adjacent over threshold point are treated as a actual over threshold area
		for(int j=TMath::Max(i-frnt_blur,0);j<i-1;j++) ChargeMask[j] = true;
		while(ThreMask[i] && i<WindowSize) i++;
		for(int j=i;j<TMath::Min((Int_t)(i+back_blur),(Int_t)WindowSize);j++) ChargeMask[j] = true;
        // break; // only expand the first over-threshold area
	}
}

// Automatically adjust back_blur to prevent short PedMaskLen which result in bad Ped calculation
int Dynamic_ExpandMask(bool* ThreMask, bool* ChargeMask,int WindowSize, int max_chargemask_len,int frnt_blur=10,int back_blur_start=50, int back_blur_step=5, int back_blur_stop=20)
{
	if(back_blur_start<back_blur_stop) back_blur_stop=back_blur_start;
	for(int ChargeMaskLen=WindowSize;back_blur_start>=back_blur_stop;back_blur_start-=back_blur_step)
	{
		ExpandMask(ThreMask, ChargeMask, WindowSize, frnt_blur, back_blur_start);
		ChargeMaskLen = accumulate(ChargeMask,ChargeMask+WindowSize,0);
		if(ChargeMaskLen<max_chargemask_len) break;
	}
	return back_blur_start;
}

// Get Pedestal Mask
template<typename iterator>
void GetPedMask(iterator v_begin,bool* ChargeMask,int WindowSize,double ped_upperlimit, bool* PedMask,int& PedMask_Len)
{
	PedMask_Len=0;
	for(int i=0;i<WindowSize;i++)
	{
		if(ChargeMask[i]) { PedMask[i]=false; continue; }
		if(*(v_begin+i)>ped_upperlimit) { PedMask[i]=false; continue; }
		PedMask[i] = true;
		PedMask_Len++;
	}
}

// Calculate Pedestal. Given baseline end point, calcaulate both front baseline and full basline.
template<typename iterator>
void GetPedinfo(iterator v_begin,int WindowSize,int bl_begin,int bl_end,bool* PedMask, double& Ped,double& PedStd)
{
	int min_statistic = WindowSize/15;
	Ped=0; PedStd=0;
	int i;
	for(i=bl_begin;i<bl_end;i++)
	{
		if(PedMask[i])
		{
			Ped += v_begin[i];
			PedStd += v_begin[i] * v_begin[i];
			min_statistic--;
		}
	}
	for(int j=bl_begin-1;min_statistic>0;) // not enough statistic for ped
	{
		if(i!=WindowSize) // if ped region not touch the end, expand backward
		{
			if(PedMask[i])
			{
				Ped += v_begin[i];
				PedStd += v_begin[i] * v_begin[i];
				min_statistic--;
			}
			i++;
		}
		if(j!=-1) // if ped region not touch the start, expand forward
		{
			if(PedMask[j])
			{
				Ped += v_begin[j];
				PedStd += v_begin[j] * v_begin[j];
				min_statistic--;
			}
			j--;
		}
		else if(i==WindowSize) break; // both reach start/end, stop
	}
	int count = WindowSize/15 - min_statistic;
	Ped = Ped/count;
	PedStd = TMath::Sqrt(PedStd/count - Ped*Ped);
}

// Calculate charge. Integration area is the intersection of integration range and ChargeMask, while we ensure every connected domain in ChargeMask are not cutted. debug=true is used for deuge case
template<typename iterator>
double GetCharge_full(iterator v_begin,bool* ChargeMask,int WindowSize,double Ped,int& count,int inte_begin=50,int inte_end=400)
{   
    // cout << "Calculate charge with inte_begin: " << inte_begin << ", inte_end: " << inte_end << endl;
	int i=inte_begin; 
	count=0;
	int64_t charge=0;
	for(;ChargeMask[i];i--) if(i==-1) break; // ensure inte_begin does not cut inside a ChargeMask.
	i++;
	for(;i<inte_end;i++)
	{
		if(ChargeMask[i])
		{
			charge += *(v_begin+i);
			count++;
#ifdef __DEBUG__
			cout<<if(debug) i<<": "<<*(v_begin+i)<<", "<<count<<": "<<charge<<endl;
#endif
        // cout<<"charge loc: "<<i<<", value: "<<*(v_begin+i)<<", Ped: "<<Ped<<endl;
		}
	}
	for(;ChargeMask[i];i++) // ensure inte_end does not cut inside a ChargeMask.
	{
		if(i==WindowSize) break;
		charge += *(v_begin+i);
		count++;
        // cout<<"charge loc: "<<i<<", value: "<<*(v_begin+i)<<", Ped: "<<Ped<<endl;
#ifdef __DEBUG__
		if(debug) cout<<i<<": "<<*(v_begin+i)<<", "<<count<<": "<<charge<<endl;
#endif
	}
#ifdef __DEBUG__
	if(debug) cout<<Ped*count-charge<<endl;
    cout<<"total charge count:"<<count<<endl;
#endif
	return Ped*count-charge;
}

// Integration only the first ChargeMask area without considering inte_begin and inte_end
template<typename iterator>
double GetCharge(iterator v_begin,bool* ChargeMask,int WindowSize,double Ped,int& count,int inte_begin=50,int inte_end=400)
{
	int i=0; 
	count=0;
	int64_t charge=0;
    // for (;i<WindowSize;i++) if(ChargeMask[i]) break;
    // while(i<WindowSize && ChargeMask[i]){
    for(i=0; i<140; i++){
        // remove data of peak in [0,10] and [110,140] to avoid partial waveform
        charge += *(v_begin+i);
        count++;
        i++;
        // cout<<"charge loc: "<<i<<", value: "<<*(v_begin+i)<<", Ped: "<<Ped<<endl;
    }
	return Ped*count-charge;
}

}
