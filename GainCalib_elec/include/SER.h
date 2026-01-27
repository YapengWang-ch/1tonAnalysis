#include <iostream>
#include <boost/math/distributions/poisson.hpp>
#include <boost/math/distributions/gamma.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/exponential.hpp>
using namespace std;
double Gamma(double x, double mu, double sigma) {
    double alpha = mu * mu / (sigma * sigma);
    double scale = sigma * sigma / mu;
    // if (alpha <= 0 || scale <= 0 || x < 0 ){
    //     return 0.0; // 非法参数处理
    // }
    // if (isinf(alpha) || isinf(scale)) {
    //     return 0.0; // 非法参数处理
    // }
    boost::math::gamma_distribution<double> dist(alpha, scale);
    return pdf(dist, x);
}

double tweedie(double x, double lam, double mu, double sigma) 
{
    int n = 30;
    double tw = 0;
    // if (lam <= 0 || mu <= 0 || sigma <= 0 || x < 0) {
    //     return 0.0; // 非法参数处理
    // }
    // if (isinf(lam) || isinf(mu) || isinf(sigma)) {
    //     return 0.0; // 非法参数处理
    // }
    boost::math::poisson_distribution<double> poisson(lam);

    for (int i = 0; i <= n; ++i)
    {
        tw += pdf(poisson, i + 1) * Gamma(x, (i + 1) * mu, (1 + i) * sigma);
    }
    tw += (1 - cdf(poisson, n + 1)) * Gamma(x, (n + 1) * mu, (1 + n) * sigma);
    return tw;
}

double SER(double* x, double* par) 
{
    double A = par[0];
    double frac = par[1];
    double mu = par[2];
    double sigma = par[3]*mu;
    double lam = par[4];
    double mu_ts = par[5]*mu;
    double sigma_ts = par[6]*mu_ts;
    return A * frac * Gamma(x[0], mu, sigma) + A * (1 - frac) * tweedie(x[0], lam, mu_ts, sigma_ts);
}

double CombinedSER(double* x, double* par)
{
    // par数组结构:
    // par[0]: 总归一化因子
    // par[1]: 高斯成分的比例
    // par[2]: 指数成分的比例
    // par[3]: 高斯分布的均值
    // par[4]: 高斯分布的标准差
    // par[5]: 指数分布的lambda参数
    // par[6]: SER成分的参数开始位置（需要额外6个参数）
    if (par[1] + par[2] > 1.0) {
        // 比例和超过1，返回0
        return 0.0;
    }
    // 检查比例和是否为1
    double N_gauss = par[0]*par[1];
    double N_exp = par[0]*par[2];
    double N_ser = par[0]-N_gauss-N_exp;
    double mu_gauss = par[3];
    double sigma_gauss = par[4];
    double lambda_exp = par[5];
    // double* ser_params = &par[6];

    double result = 0.0;
    
    // 1. 高斯成分
    boost::math::normal_distribution<double> normal_dist(mu_gauss, sigma_gauss);
    result += N_gauss * pdf(normal_dist, x[0]);
    
    // 2. 指数成分
    boost::math::exponential_distribution<double> exp_dist(lambda_exp);
    result += N_exp * pdf(exp_dist, x[0]);
    
    // 3. SER成分
    // SER参数在par[7]到par[13]中
    double ser_params[7] = {N_ser,par[6],par[7],par[8],par[9],par[10],par[11]};

    result += SER(x, ser_params);

    
    return result;
}

double DrawGauss(double* x, double *par){
    double N_gauss = par[0]*par[1];
    double N_exp = par[0]*par[2];
    double N_ser = par[0]-N_gauss-N_exp;
    double mu_gauss = par[3];
    double sigma_gauss = par[4];
    double lambda_exp = par[5];
    boost::math::normal_distribution<double> normal_dist(mu_gauss, sigma_gauss);
    return N_gauss * pdf(normal_dist, x[0]);
}

double DrawExp(double* x, double *par){
    double N_gauss = par[0]*par[1];
    double N_exp = par[0]*par[2];
    double N_ser = par[0]-N_gauss-N_exp;
    double mu_gauss = par[3];
    double sigma_gauss = par[4];
    double lambda_exp = par[5];
    boost::math::exponential_distribution<double> exp_dist(lambda_exp);
    return N_exp * pdf(exp_dist, x[0]);
}

double DrawGammaFull(double* x, double *par){
    double N_gauss = par[0]*par[1];
    double N_exp = par[0]*par[2];
    double N_ser = par[0]-N_gauss-N_exp;
    double frac = par[6];
    double mu = par[7];
    double sigma = par[8]*mu;
    return N_ser * frac * Gamma(x[0], mu, sigma);
}

double DrawGamma(double* x, double *par){
    double N_ser = par[0];
    double frac = par[1];
    double mu = par[2];
    double sigma = par[3]*mu;
    return N_ser * frac * Gamma(x[0], mu, sigma);
}

double DrawTweedieFull(double* x, double *par){
    double N_gauss = par[0]*par[1];
    double N_exp = par[0]*par[2];
    double N_ser = par[0]-N_gauss-N_exp;

    double frac = par[6];
    double mu = par[7];
    double sigma = par[8];
    double lam = par[9];
    double mu_ts = par[10]*mu;
    double sigma_ts = par[11]*mu_ts;
    return N_ser * (1 - frac) * tweedie(x[0], lam, mu_ts, sigma_ts);
}

double DrawTweedie(double* x, double *par){
    double N_ser = par[0];

    double frac = par[1];
    double mu = par[2];
    // double sigma = par[3];
    double lam = par[4];
    double mu_ts = par[5]*mu;
    double sigma_ts = par[6]*mu_ts;
    return N_ser * (1 - frac) * tweedie(x[0], lam, mu_ts, sigma_ts);
}