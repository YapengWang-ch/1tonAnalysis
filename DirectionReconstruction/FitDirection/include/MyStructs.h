// MyStructs.h
#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H

// #include <vector>
#include "TObject.h"
// #include "TVectorD.h"

struct node {
    double cosTheta;
    double Phi;
    double cosAlpha;
    double Beta;
    double Time2Mean[60];
    double Energy[60];
};

struct FitResult : public TObject {
    double chi2;
    node FitNode;
    bool operator<(const FitResult &v) const {
        return chi2 < v.chi2;
    }
    ClassDef(FitResult, 1); // ROOT宏
};

struct Readin : public TObject {
    double TotalPE, PEmax2Sum, TimeRange;
    int RunNo, FileNo, TriggerNo;
    node MCNode;
    ClassDef(Readin, 1);
};

struct DataOut : public TObject {
    double TotalPE, PEmax2Sum;
    node MCNode;
    int ProjectionDimension;
    double cosTheta_rec, Phi_rec, cosAlpha_rec, Beta_rec;
    double deltaAngle, deltaPosition;
    ClassDef(DataOut, 1);
};

#endif