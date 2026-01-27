#include <iostream>
#include "TMath.h"

double Gauss(double x,double mu,double sigma2){
    return 1/TMath::Sqrt(2*TMath::Pi()*sigma2)*TMath::Exp(-(x-mu)*(x-mu)/2/sigma2);
}

double MultiGauss(double* x, double * par){
    double lambda[5]={par[1],par[4],par[7],par[10],0};
    lambda[5]=1-lambda[0]-lambda[1]-lambda[2]-lambda[3]-lambda[4];
    double mu[5]={par[2],par[5],par[8],par[11],par[13]};
    double sigma[5]={par[3],par[6],par[9],par[12],par[14]};
    double scale = par[0];
    double result=0;
    for(int i=0; i<5; i++){
        result+=lambda[i]*Gauss(x[0],mu[i],sigma[i]*sigma[i]);
    }
    return result*scale;
}