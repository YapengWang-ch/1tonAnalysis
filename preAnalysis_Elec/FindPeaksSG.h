#pragma once
#include "TVirtualFFT.h"
#include <map>
#include <unordered_map>
using namespace std;

double n5M4[5][11] = {
    { 0.00174825, -0.00174825, -0.00174825, -0.000291375, 0.0011655, 0.00174825, 0.0011655, -0.000291375,-0.00174825, -0.00174825, 0.00174825},
    {-0.00582751, 0.0011655,    0.0042735,  0.00446775,   0.0027195, -2.57706e-20, -0.0027195,  -0.00446775,   -0.0042735,   -0.0011655,  0.00582751},
    {-0.0262238,  0.0506993,    0.0425408,  0.000291375,  -0.039627, -0.0553613,   -0.039627,  0.000291375,    0.0425408,    0.0506993,   -0.0262238},
    {0.0582751,   -0.0571096,   -0.103341,  -0.0977078,   -0.0574981, -4.54678e-18,0.0574981,    0.0977078,     0.103341,    0.0571096,   -0.0582751},
    {0.041958,    -0.104895,    -0.02331,   0.13986,      0.27972,     0.333333,      0.27972,      0.13986,     -0.02331,    -0.104895,  0.041958}
};

double n3M3[4][7] = {
   {-0.0277778,    0.0277778,    0.0277778,  0,   -0.0277778,   -0.0277778,    0.0277778},
   {0.0595238,  0,   -0.0357143,    -0.047619,   -0.0357143,  0,    0.0595238},
   {0.0873016,    -0.265873,    -0.230159, 0,     0.230159,     0.265873,   -0.0873016},
   {-0.0952381,     0.142857,     0.285714,     0.333333,     0.285714,  0.142857,   -0.0952381}
};

double n5M3[4][11] = {
    { -0.00582751,    0.0011655,    0.0042735,   0.00446775,    0.0027195, -1.13708e-19,   -0.0027195,  -0.00446775,   -0.0042735,   -0.0011655,   0.00582751},
    { 0.0174825,   0.00699301,   -0.0011655,  -0.00699301,   -0.0104895,    -0.011655,   -0.0104895,  -0.00699301,   -0.0011655,   0.00699301,    0.0174825},
    { 0.0582751,   -0.0571096,    -0.103341,   -0.0977078,   -0.0574981,            0,    0.0574981,    0.0977078,     0.103341,    0.0571096,   -0.0582751},
    { -0.0839161,     0.020979,     0.102564,     0.160839,     0.195804,     0.207459,     0.195804,     0.160839,     0.102564,     0.020979,   -0.0839161},
};

double n4M3[4][9] = {
 { -0.0117845,   0.00589226,    0.0109428,   0.00757576,  1.38826e-18,  -0.00757576,   -0.0109428,  -0.00589226,    0.0117845},
 {   0.030303,   0.00757576,  -0.00865801,   -0.0183983,    -0.021645,   -0.0183983,  -0.00865801,   0.00757576,     0.030303},
 {  0.0723906,    -0.119529,    -0.162458,    -0.106061, -1.91768e-17,     0.106061,     0.162458,     0.119529,   -0.0723906},
 { -0.0909091,   0.0606061,     0.168831,     0.233766,     0.255411,     0.233766,     0.168831,    0.0606061,   -0.0909091},
};

template<typename iterator>
std::vector<iterator> FindPeaksSG(iterator vbegin, iterator vend, double ped, double thd)
{
    // Low-pass filter
    int nSize = vend-vbegin;
    double* data = new double[nSize];
    for(int i=0; i<nSize; i++)
    {
        data[i] = *(vbegin+i);
    }
    TVirtualFFT *fft = TVirtualFFT::FFT(1, &nSize, "R2C");
    fft->SetPoints(data);
    fft->Transform();

    double *re_full = new double[nSize/2+1];
    double *im_full = new double[nSize/2+1];
    fft->GetPointsComplex(re_full,im_full);
    /* 
    TH1D* hF = new TH1D("Freq", "Freq", nSize/2+1, 0, nSize/2+1);
    for(int i=0; i<nSize/2+1; i++)
    {
        if(i==0)
            hF->SetBinContent(i+1, 0);
        else
            hF->SetBinContent(i+1, sqrt(re_full[i]*re_full[i]+im_full[i]*im_full[i]));
    }
    TCanvas* c00 = new TCanvas;
    c00->SetGridy(true);
    c00->SetGridx(true);
    hF->Draw();
    */
    for(int i=0.8*(nSize/2+1); i<nSize/2+1; i++)
    {
        re_full[i] = 0;
        im_full[i] = 0;
    }
    TVirtualFFT *fft_back = TVirtualFFT::FFT(1, &nSize, "C2R M K");
    fft_back->SetPointsComplex(re_full,im_full);
    fft_back->Transform();
    for(int i=0; i<nSize; i++)
        data[i] = fft_back->GetPointReal(i)/nSize;

    delete[] re_full;
    delete[] im_full;
    delete fft;
    delete fft_back;

    int n = 4;
    int M = 3;
    std::vector<iterator> peakList;
    double* diff = new double[nSize];
    double* diff2 = new double[nSize];
    //double* smooth = new double[nSize];
    
    for(int i=0; i<nSize; i++)
    {
        diff[i] = 0;
        diff2[i] = 0;
        //smooth[i] = 0;
        for(int k=-n; k<=n; k++)
        {
            double aa = 0;
            if(i+k<0 || i+k>nSize-1)
                aa = ped;
            else
                //aa = *(vbegin+i+k);
                aa = data[i+k];
            diff[i] += aa*n4M3[M-1][k+n];
            diff2[i] += aa*n4M3[M-2][k+n];
            //smooth[i] += aa*n4M3[M][k+n];
            //cout<<"smooth: "<<i<<"\t"<<smooth[i]<<endl;
        }
    }

    /*
    TH1D* hL = new TH1D("F", "F", nSize, 0, nSize);
    for(int i=0; i<nSize; i++)
        hL->SetBinContent(i+1, data[i]);
    TCanvas* c0 = new TCanvas;
    c0->SetGridy(true);
    c0->SetGridx(true);
    hL->Draw();
    
    TH1D* hD = new TH1D("dev1", "dev1", nSize, 0, nSize);
    for(int i=0; i<nSize; i++)
        hD->SetBinContent(i+1, diff[i]);
    TCanvas* c1 = new TCanvas;
    c1->SetGridy(true);
    c1->SetGridx(true);
    hD->Draw();
    
    TH1D* hD2 = new TH1D("dev2", "dev2", nSize, 0, nSize);
    for(int i=0; i<nSize; i++)
        hD2->SetBinContent(i+1, diff2[i]);
    TCanvas* c2 = new TCanvas;
    c2->SetGridx(true);
    c2->SetGridy(true);
    hD2->Draw();
    
    TH1D* hS = new TH1D("S1", "S1", nSize, 0, nSize);
    for(int i=0; i<nSize; i++)
        hS->SetBinContent(i+1, smooth[i]);
    new TCanvas;
    hS->Draw();
    */
    
    for(int i=0; i<nSize-1; i++)
    {
        double wv = *(vbegin+i);
        
        bool b1 = false;
        bool b2 = false;
        // Pass 0 in the first deviation
        if(diff[i]<0 && diff[i+1]>0) b1 = true;
        if(i>0 && (diff[i-1]<0 && abs(diff[i])<1E-6 && diff[i+1]>0)) b2 = true;
        // The second deviation > 0.1
        if((b1 || b2))
        {
            double sd = *max_element(diff2+max(i-5,0), diff2+min(i+6, nSize));
            if(sd<0.1) continue;
            double peakAmp1 = *min_element(data+max(i-2,0), data+min(i+3,nSize));
            auto peakLocIter = min_element(vbegin+max(i-2,0), vbegin+min(i+3,nSize));
            int peakLoc = peakLocIter-vbegin;
            double peakAmp2 = *peakLocIter;
            if(ped-peakAmp1>thd && ped-peakAmp2>thd)
            {
                bool flat_l = false;
                bool flat_r = false;
                if(i<nSize-5)
                {
                    flat_r = true;
                    for(int j=0; j<5; j++)
                    {
                        double vv = *(vbegin+i+j);
                        if(abs(vv-wv)>1e-6)
                        {   
                            flat_r = false;
                            break;
                        }
                    }
                }
                if(i>5)
                {
                    flat_l = true;
                    for(int j=0; j<5; j++)
                    {
                        double vv = *(vbegin+i-j);
                        if(abs(vv-wv)>1e-6)
                        {   
                            flat_l = false;
                            break;
                        }
                    }
                }
                if(flat_l && flat_r) continue;
                i += 3;
                if(peakList.size()==0 || vbegin+peakLoc!=peakList.back())
                    peakList.emplace_back(vbegin+peakLoc);
                //cout<<"Find "<<peakLoc<<" "<<peakamp<<endl;
            }
        }
    }

    if(peakList.size()==0)
    {
        int mindataLoc = min_element(data, data+nSize)-data;
        if(ped-data[mindataLoc]>thd)
        {
            //cout<<"Minf: "<<ped-data[mindataLoc]<<endl;
            auto minIter = min_element(vbegin+max(mindataLoc-10,0), vbegin+min(mindataLoc+10, nSize));
            //cout<<"Min: "<<ped-*minIter<<endl;
            peakList.emplace_back(minIter);
        }
    }

    delete[] diff;
    delete[] diff2;
    delete[] data;
    //delete[] smooth;
    return peakList;
}

