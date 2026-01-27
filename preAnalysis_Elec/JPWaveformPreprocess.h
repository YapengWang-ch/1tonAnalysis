#ifndef JPWAVEFORMPREPROCESS
#define JPWAVEFORMPREPROCESS

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <set>
#include "TMinuit.h"
#include "TH1D.h"
#include "TF1.h"
#include "TGraph.h"

#define Threshold 100
namespace wpp {

template<typename iterator>
iterator GetPeakFast(iterator vbegin, iterator vend)
{
	// min_element() method in stl is not compatible with TTreeReader in event loop, we have to manually implement this method here.
	auto minIter = vbegin;
	for(auto iter = vbegin+1; iter!=vend; iter++)
	{
		if(*iter<*minIter)
		{
			minIter = iter;
		}
	}
	return minIter;
}

template<typename iterator>
iterator FindFirstPeak(iterator vbegin, iterator vend, double ped, double thd = Threshold)
{
	double minValue = *vbegin;
	for(auto iter = vbegin; iter!=vend; iter++)
	{
		if(*iter<(ped-thd))
		{
			bool success = true;
			for(int i=-5; i<=3; i++)
			{
				auto currentIter = iter+i;
				if(currentIter<vbegin || currentIter>=vend)
				{
					success = false;
					continue;
				}
				else if(*currentIter<*iter)
				{
					success = false;
					break;
				}

			}
			if(success)
				return iter;
		}
	}
	return vend-1;

}

template<typename iterator>
int GetPeaksFast1(iterator vbegin, iterator vend, double ped, iterator* peakList, double thd=Threshold)
{
    int vlength = vend-vbegin;
    int nPeak = 0;
    for(int i=0; i<vlength; )
    {
        if(*(vbegin+i)-ped>-thd)
        {
            i++;
            continue;
        }
        bool isPeak = true;
        int startIdx = std::max(i-3, 0);
        int endIdx = std::min(i+3, vlength);
        double localMax = *(vbegin+startIdx);
        for(int k=startIdx+1; k<endIdx; k++)
        {
            double w = *(vbegin+k);
            if(w>localMax) localMax = w;
        }
        for(int j=std::max(i-2, 0); j<std::min(i+2, vlength); j++)
        {
            if(*(vbegin+j)-ped>-thd)
            {
                isPeak = false;
                break;
            }
        }
        if(isPeak)
        {
        for(int j=std::max(i-3, 0); j<std::min(i+3, vlength); j++)
        {
            if(*(vbegin+j)<*(vbegin+i))
            {
                isPeak = false;
                break;
            }
        }
        }
        if(isPeak)
        {
            peakList[nPeak] = vbegin+i;
            i += 3;
            nPeak++;
        }
        else
            i++;

    }
    return nPeak;
}

template<typename iterator>
std::vector<iterator> GetPeaksFast(iterator vbegin, iterator vend, double ped, double thd = Threshold)
{
    std::vector<iterator> peakList;
    int vlength = vend-vbegin;
    for(int i=0; i<vlength; )
    {
        if(*(vbegin+i)-ped>-thd)
        {
            i++;
            continue;
        }
        
        double local = *(vbegin+i);
        std::vector<bool> overthd;
        for(int j=std::max(i-5, 0); j<std::min(i+6, vlength); j++)
        {
            if(*(vbegin+j)>=local)
                overthd.push_back(true);
            else
                overthd.push_back(false);
        }
        if(overthd.size()<11)
        {
            i++;
            continue;
        }
        bool check1 = true;
        double maxlocal = *std::max_element(vbegin+i-5, vbegin+i+6);
        for(int j=3; j<8; j++)
        {
            if(!overthd[j])
            {
                check1 = false;
                break;
            }
        }
        bool check2 = true;
        for(int j=2; j<7; j++)
        {
            if(!overthd[j])
            {
                check2 = false;
                break;
            }
        }
        if((check1 || check2) && *(vbegin+i-2)-local>=2 && maxlocal-local>=2)
        {
            peakList.push_back(vbegin+i);
            i += 2;
        }
        else i++;
        
        /*
        bool isPeak = true;
        int startIdx = std::max(i-3, 0);
        int endIdx = std::min(i+3, vlength);
        double localMax = *(vbegin+startIdx);
        for(int k=startIdx+1; k<endIdx; k++)
        {
            double w = *(vbegin+k);
            if(w>localMax) localMax = w;
        }
        for(int j=std::max(i-3, 0); j<std::min(i+3, vlength); j++)
        {
            if(*(vbegin+j)-ped>-thd)
            {
                isPeak = false;
                break;
            }
        }
        if(isPeak)
        {
        for(int j=std::max(i-3, 0); j<std::min(i+3, vlength); j++)
        {
            if(*(vbegin+j)<*(vbegin+i) || *(vbegin+i)-localMax>-3)
            {
                isPeak = false;
                break;
            }
        }
        }
        if(isPeak)
        {
            peakList.push_back(vbegin+i);
            i += 3;
        }
        else
          i++;*/
        
    }
    return peakList;
}


template<typename iterator>
int SaturatedTime(iterator vbegin, iterator vend)
{
	/*int currentV = 0;
	int maxV = 0;
	for(auto iter = vbegin+1; iter!=vend; iter++)
	{
		if(*iter == *(iter-1)  && *iter==0)
			currentV ++;
		else 
			currentV = 0;
		if(currentV>maxV)
			maxV = currentV;
	}

	if(maxV>10)
		return true;
	else return false;*/
	int n = 0;
	for(auto iter = vbegin+1; iter!=vend; iter++)
	{
		if(*iter==0)
			n++;
	}
	return n;

}

template<typename iterator>
int ArrayRangeCheck(iterator vbegin, iterator vend, int nbegin, int nend)
{
    int nsize = std::distance(vbegin, vend);
	if(nbegin > nsize
		|| nend > nsize 
		|| nend<=nbegin )
	{
		std::cout<<"Interval is out of index! Array size is "<<nsize<<
        ". Begin idx is "<< nbegin<<". End idx is "<<nend<<std::endl;
		return -1;
	}

	return 1;
}

template<typename iterator>
double GetPedestalFast(iterator vbegin, iterator vend, size_t nbegin=0, size_t nend=100)
{
	// Array range check
	if(ArrayRangeCheck(vbegin, vend, nbegin, nend)==-1)
		return 0;
	return std::accumulate(vbegin+nbegin, vbegin+nend, 0.0)/(nend-nbegin);

}

// This algorithm is from Jolin
template<typename iterator>
double GetPedestal(iterator vbegin, iterator vend, size_t nbegin=0, size_t nend=110)
{
	// Array range check
	if(ArrayRangeCheck(vbegin, vend, nbegin, nend)==-1)
		return 0;

	// Start calculate
	size_t i, l, nCounter;
	double ped[7];
    double pedFine[7];
	for(int j=0; j<7; j++)
    {
		ped[j] = 0;
        pedFine[j]= 0;
    }
    int nlength = 0;
	for(i=nbegin; i<nend; i++)
	{
        if(*(vbegin+i)<900 || *(vbegin+i)>980) continue;
		ped[0] += *(vbegin+i);
        nlength++;
	}
    if(nlength==0)
        return GetPedestalFast(vbegin, vend, nbegin, nend);
	ped[0] = ped[0]/nlength;
    //cout<<"ped0: "<<ped[0]<<endl;
    int nCounter0 = 0;
	for(l=0;l<6;l++)
	{
		nCounter = 0;
		for(i=nbegin; i<nend; i++)
		{
			//if(abs(*(vbegin+i)-ped[l]) < 4)
			if(*(vbegin+i)-ped[l] > -4 && *(vbegin+i)>900 && *(vbegin+i)<980)
			{
				nCounter += 1;
				ped[l+1] += *(vbegin+i);
			}
		}
        if(nCounter==nCounter0||nCounter==0) break;
		ped[l+1] = ped[l+1]/nCounter;
        nCounter0 = nCounter;
        //cout<<"ped"<<l+1<<": "<<ped[l+1]<<" nCounter: "<<nCounter<<endl;
	}
    pedFine[0] = ped[l];
    nCounter0 = 0;
	for(l=0;l<6;l++)
	{
		nCounter = 0;
		for(i=nbegin; i<nend; i++)
		{
			if(abs(*(vbegin+i)-ped[l]) < 4)
			{
				nCounter += 1;
				pedFine[l+1] += *(vbegin+i);
			}
		}
        if(nCounter==nCounter0 || nCounter==0) break;
		pedFine[l+1] = pedFine[l+1]/nCounter;
        nCounter0 = nCounter;
        //cout<<"ped"<<l+1<<": "<<ped[l+1]<<" nCounter: "<<nCounter<<endl;
	}
	return pedFine[l];

}


// A naive but fast method. If the charge is too small, the fluctuation of baseline would make the charge negtive.
template<typename iterator>
double GetChargeFast(iterator vbegin, iterator vend, double ped, int nbegin=110, int nend=500)
{
    if(nbegin<0) nbegin=0;
    if(nend>vend-vbegin) nend = vend-vbegin;
	// Array range check
	if(ArrayRangeCheck(vbegin, vend, nbegin, nend)==-1)
	{
		nend = (unsigned int)abs(std::distance(vbegin, vend));
	}

	double charge = std::accumulate(vbegin+nbegin, vbegin+nend, 0.0);
	charge -= ped*(nend-nbegin);
	return -charge;
}

// Get the first over-threshold time.
template<typename iterator>
double GetFirstOverThresholdTime(iterator vbegin, iterator vend, double thd, double ped, size_t nbegin = 150, size_t nend = 600)
{
	// Array range check
	if(ArrayRangeCheck(vbegin, vend, nbegin, nend)==-1)
		return 0;

	double searchTime = nend;
    double c1 = -thd;
	for(int k=nbegin; k<nend-3; k++)
	{
        int offset = std::max(k-5, 0);
        /*
        bool valid = true;
        for(int j=offset; j<k; j++)
        {
            if(*(vbegin+j)-ped<c1) 
            {
                valid = false;
                break;
            }
        }*/
        double meanl = 0;
        for(int j=offset; j<k; j++)
        {
            meanl += *(vbegin+j)-ped;
        }
        if(k-offset!=0)
            meanl /= k-offset;
        bool valid = meanl>c1;
        if(valid && k>0 && *(vbegin+k)-ped<=c1 && (*(vbegin+k)+*(vbegin+k+1)+*(vbegin+k+2))/3-ped<=c1)
        {
            double y2 = *(vbegin+k)-ped;
            double y1 = *(vbegin+k-1)-ped;
            if(y1==y2) searchTime = k;
            else
                searchTime = (c1-y2)/(y2-y1)+k;
            //std::cout<<"k: "<<k<<std::endl;
            return searchTime;

        }
	}
	return searchTime;
}

// Get the first 10% peak time.
template<typename iterator>
double GetFirstOverXpctTime(iterator vbegin, iterator vend, double peak, double ped, double X, size_t nbegin = 150, size_t nend = 600)
{
    double thd = std::max(peak*X/100, 3.0);
    return GetFirstOverThresholdTime(vbegin, vend, thd, ped, nbegin, nend);
}

template<typename iterator>
TF1* FitTime(iterator vbegin, iterator vend, double ped, bool& status)
{
    int hlength = vend-vbegin;
    //TH1D* htemp = new TH1D("htemp", "htemp", hlength, 0, hlength);
    //for(int i=1; i<=hlength; i++)
    //    htemp->SetBinContent(i, *(vbegin+i-1)-ped);
    TF1* func = new TF1("ftemp", "-[0]*TMath::Gaus(x, [1],[2], false)", 0, hlength-1);
    double heightguess = ped-(*min_element(vbegin, vend));
    func->SetParameters(heightguess, hlength-3, 2.4);
    func->SetParLimits(0, 0, std::max(1200., 2*heightguess));
    func->SetParLimits(1, -20, 1029);
    func->SetParLimits(2, 0, 100);
    //htemp->Fit("ftemp", "NQRWW");

    double* x = new double[hlength];
    double* y = new double[hlength];
    for(int i=0; i<hlength; i++)
    {
        x[i] = i;
        y[i] =  *(vbegin+i)-ped;
    }
    //cout<<"hlength: "<<hlength<<endl;
    TGraph* g1 = new TGraph(hlength, x, y);
    g1->Fit("ftemp", "NQ");
    //std::cout<<(gMinuit->fCstatu=="CONVERGED ")<<std::endl;
    status = (gMinuit->fCstatu == "CONVERGED ");
    //delete htemp;
    //new TCanvas;
    //htemp->Draw();
    //g1->Draw();
    //func->Draw("same");
    delete g1;
    return func;
    /*
    return func;
    chi2 = func->GetChisquare();
    area = func->GetParameter(0);
    mean = func->GetParameter(1);
    sigma = func->GetParameter(2);
    //newped = func->GetParameter(3);
    double peak = func->GetMinimum(0, hlength);
    double peakx = func->GetMinimumX(0, hlength);
    result = func->GetX(thd*peak, 0, peakx);
    delete func;
    */
}
/*
TString FitTime(TH1D* htemp, double start, double end, double& result)
{
    TF1* func = new TF1("ftemp", "[0]*TMath::Gaus(x, [1],[2],true)", start, end);
    func->SetParameters(-100, end-3, 2.4);
    htemp->Fit("ftemp", "NRQWW");
    double peak = func->GetMinimum(start, end);
    double peakx = func->GetMinimumX(start, end);
    result = func->GetX(0.1*peak, start, peakx);
    delete func;
    return gMinuit->fCstatu;
}
*/

}
#endif
