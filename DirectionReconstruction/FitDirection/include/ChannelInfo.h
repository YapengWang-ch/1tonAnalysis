#ifndef _CHANNELINFO_H
#define _CHANNELINFO_H

#include <vector>
using namespace std;

class ChannelInfo_t
{
public :
    int ChannelId;
    double Pedestal;
    double Charge;
    double Peak;
    int nPeaks;
	vector<int> PeakLoc;
	vector<double> PeakAmp;
    double FrontBslnMean;
    double FrontBslnStdDev;
    double BackBslnMean;
    double BackBslnStdDev;
    double RiseTime;
    double PE;
    ChannelInfo_t() : ChannelId(-1), Pedestal(0), Charge(0), Peak(0), nPeaks(0), 
	FrontBslnMean(0), FrontBslnStdDev(0), BackBslnMean(0), BackBslnStdDev(0), RiseTime(0), PE(0) {};
	void Reset() {PeakLoc.clear(); PeakAmp.clear();};
};


#endif
