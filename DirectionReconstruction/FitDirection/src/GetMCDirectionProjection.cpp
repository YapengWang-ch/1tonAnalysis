#include "Utils/JPUtils.h"
#include "TStopwatch.h"
#include <algorithm>
#include <cmath>
#include "TString.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom3.h"
#include <iostream>
#include <numeric>
#include "TMath.h"
#include "TChain.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "MyStructs.h"
#include "TDecompSVD.h"
using namespace std;

const int NChannels = 60;

bool MatrixAlmostEqual(const TMatrixD& A, const TMatrixD& B, double epsilon = 1e-12) {
    if (A.GetNrows() != B.GetNrows() || A.GetNcols() != B.GetNcols()) 
        return false;
    
    for (int i = 0; i < A.GetNrows(); ++i) {
        for (int j = 0; j < A.GetNcols(); ++j) {
            if (std::abs(A(i,j) - B(i,j)) > epsilon) {
                return false;
            }
        }
    }
    return true;
}

class PCAProjection {
    public:
        PCAProjection(int target_dim) : n(target_dim) {}
    
        // 训练模型并生成投影矩阵
        void Fit(const std::vector<TVectorD>& data) {
            if (data.empty()) return;
            
            const int m = data[0].GetNrows(); // 原始维度
            const int k = data.size();        // 样本数量
            
            // 1. 数据中心化
            TVectorD mean(m);
            for (const auto& vec : data) {
                mean += vec;
            }
            mean *= 1.0 / k;
            
            TMatrixD X_centered(m, k);
            for (int i = 0; i < k; ++i) {
                for (int row = 0; row < m; ++row) {
                    X_centered(row, i) = (data[i] - mean)[row];
                }
            }
            
            // 2. 计算协方差矩阵 (1/(k-1) * X * X^T)
            TMatrixD XT(TMatrixD::kTransposed, X_centered);
            TMatrixD Cov = X_centered * XT;
            Cov *= 1.0 / (k - 1);

            double trace = 0.0;
            for (int i = 0; i < Cov.GetNrows(); ++i) {
                trace += Cov(i, i);
            }
            std::cout << "Covariance matrix trace: " << trace << std::endl;
            if (trace < 1e-12) {
                std::cerr << "Error: Covariance matrix is zero. Check input data." << std::endl;
                return;
            }

            // 3. 特征分解
            TDecompSVD svd(Cov);
            svd.Decompose();
            if (!svd.Decompose()) {
                std::cerr << "Error: SVD decomposition failed." << std::endl;
                return;
            }
            U = svd.GetU();      // 特征向量矩阵
            S = svd.GetSig();    // 奇异值（平方根为特征值）
            
            // 4. 选择前n个主成分
            TMatrixD Un(m, n);
            for (int i = 0; i < n; ++i) {
                for (int row = 0; row < U.GetNrows(); ++row) {
                    Un(row, i) = U(row, i);
                }
            }
            
            P.ResizeTo(m, m);
            TMatrixD UnT(TMatrixD::kTransposed, Un);
            P.Mult(Un, UnT);
            
            TMatrixD P2(P);
            P2 *= P;
            if(!MatrixAlmostEqual(P, P2, 1e-12)){
                cout << "Error: Projection matrix not equals its square!";
            };
        }
    
        TVectorD Transform(const TVectorD& vec) const {
            return P * vec;
        }
        double GetVarianceExplained(int n_components) const {
            TVectorD variances(S);
            variances.Sqr();
            
            double total = variances.Sum();
            double cumulative = 0;
            for (int i=0; i<n_components; ++i) {
                cumulative += variances[i];
            }
            return cumulative / total;
        }
        const TMatrixD& GetProjectionMatrix() const { return P; }
    
    private:
        int n;       // 目标维度
        TMatrixD P;  // 投影矩阵
        TVectorD mean;
        TVectorD S;
        TMatrixD U;
};


int ProjectionReconstruction(std::vector<FitResult> fittmps, DataOut &output, int Dimension,int k ){
    std::vector<TVectorD> TimeData;
    for (size_t i=0; i<fittmps.size(); i++){
        TimeData.push_back(TVectorD(NChannels, fittmps[i].FitNode.Time2Mean));
    }
    PCAProjection pca(Dimension);
    pca.Fit(TimeData);
    // cout << output.MCNode.Time2Mean[0] << endl;
    // cout << fittmps[0].FitNode.Time2Mean[0] << endl;
    TVectorD MC(NChannels,output.MCNode.Time2Mean);
    // MC.Print("v");
    TVectorD MCproj = pca.Transform(TVectorD(NChannels,output.MCNode.Time2Mean));
    cout << "VarianceExplained: "<< pca.GetVarianceExplained(Dimension)<<endl;
    TVector3 direction(0,0,0);
    TVector3 position(0,0,0);
    for (size_t i=0; i<fittmps.size()&& (i< k); i++){
        TVectorD temp(NChannels,fittmps[i].FitNode.Time2Mean);
        // temp.Print("v");
        TVectorD temproj = pca.Transform(TVectorD(NChannels, fittmps[i].FitNode.Time2Mean));
        double projchi2=(MCproj-temproj).Norm2Sqr();
        double chi2=(MC-temp).Norm2Sqr();
        cout << "projchi2/chi2: " << projchi2 << "/"<< chi2<<endl;
        TVector3 tempdirection(1,0,0);
        TVector3 tempposition(1,0,0);
        tempdirection.SetMagThetaPhi(1,fittmps[i].FitNode.cosTheta,fittmps[i].FitNode.Phi);
        tempposition.SetMagThetaPhi(1,fittmps[i].FitNode.cosAlpha,fittmps[i].FitNode.Beta);
        direction+=1.0/sqrt(projchi2)*tempdirection;
        position+=1.0/sqrt(projchi2)*tempposition;
    }
    output.ProjectionDimension=Dimension;
    direction.SetMag(1);
    position.SetMag(1);
    output.cosTheta_rec=direction.CosTheta();
    output.Phi_rec=direction.Phi();
    output.cosAlpha_rec=position.CosTheta();
    output.Beta_rec=position.Phi();

    TVector3 truedirection(1,0,0);
    TVector3 trueposition(1,0,0);
    truedirection.SetMagThetaPhi(1,output.MCNode.cosTheta,output.MCNode.Phi);
    trueposition.SetMagThetaPhi(1,output.MCNode.cosAlpha,output.MCNode.Beta);
    output.deltaAngle=truedirection.Angle(direction);
    output.deltaPosition=(trueposition-position).Mag();
    return 0;
}

int main(int argc, char **argv)
{
    TString inputFilename;
    TString outputfilename;
    Float_t sigma; // ns

    if (argc == 5)
    {
        inputFilename = argv[1];
        outputfilename = argv[2];
    }
    else
    {
        cout << "Usage:" << endl;
        cout << "  ./GetMCDirection inputFilename outputFilename [Int_t Dimension] [Int_t knn]" << endl;
        cout << endl;
        return 1;
    }
    
    const int knn = TString(argv[4]).Atoi();
    const int Dimension = TString(argv[3]).Atoi();
    cout << "knn= " << knn <<", Dimension= "<< Dimension <<endl;

    TChain *tData = new TChain("Test");
   
    tData->Add(inputFilename);
    cout << "Total entries: " << tData->GetEntries() << endl;

    Readin *MCMuon = nullptr;
    std::vector<FitResult> *fittmps=nullptr;
    tData->SetBranchAddress("MCMuon",&MCMuon);
    tData->SetBranchAddress("FitResult",&fittmps);
    cout << "read successfully"<<endl;
    //output:
    TFile *fout = new TFile(outputfilename, "recreate");
    TTree *tout = new TTree("Direction", "Direction");
    DataOut output;
    tout->Branch("TotalPE", &output.TotalPE);
    tout->Branch("PEmax2Sum", &output.PEmax2Sum);
    tout->Branch("costheta_truth", &output.MCNode.cosTheta);
    tout->Branch("phi_truth", &output.MCNode.Phi);
    tout->Branch("costheta_rec", &output.cosTheta_rec);
    tout->Branch("phi_rec", &output.Phi_rec);
    tout->Branch("DeltaAngle", &output.deltaAngle);
    tout->Branch("cosalpha_truth", &output.MCNode.cosAlpha);
    tout->Branch("beta_truth", &output.MCNode.Beta);
    tout->Branch("cosalpha_rec", &output.cosAlpha_rec);
    tout->Branch("beta_rec", &output.Beta_rec);
    // tout->Branch("trackL_truth",&trackL_truth);
    // tout->Branch("trackL_rec",&trackL_rec);
    // tout->Branch("DeltaL",&DeltaL);
    tout->Branch("DeltaPosition", &output.deltaPosition);
    cout << "Out Branchs Set" <<endl;
    TH1D *hDeltaAngle = new TH1D("hDeltaAngle", "hDeltaAngle", 100, 0., 180.);
    //hDeltaAngle->SetTitle(";#Delta#Theta[degree];Counts");
    
    int nCalc = knn;
    cout << "start loop" <<endl;
    // gEnv->SetValue("Root.StackContinuation", 1);
    for (int i = 0; i < tData->GetEntries(); i++)
    {
        tData->GetEntry(i);
        output.TotalPE=MCMuon->TotalPE;
        output.PEmax2Sum=MCMuon->PEmax2Sum;
        output.MCNode=MCMuon->MCNode;
        std::cout << "MCNode.Time2Mean length: " << sizeof(output.MCNode.Time2Mean)/sizeof(double) << std::endl;

        // cout << "start Projection" <<endl;
        ProjectionReconstruction(*fittmps,output,Dimension,knn);
        // cout << "end Projection" <<endl;
        hDeltaAngle->Fill(output.deltaAngle / TMath::Pi() * 180.);
        tout->Fill();
        if (i>10) break;
    }

    cout <<"Mean Include Angle: "<< hDeltaAngle->GetMean() << endl;

    tout->Write();
    fout->Close();

    return 0;
}
