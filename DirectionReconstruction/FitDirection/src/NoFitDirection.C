#include <TChain.h>
#include <TMath.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct AnalysisResult {
    long entries;
    double mean_rad;
    double mean_deg;
};

AnalysisResult calculate_theta_mean(const vector<string>& files) {
    TChain chain("ma");  // 创建TChain并指定树名
    AnalysisResult result{0, 0.0, 0.0};

    // 添加文件到TChain
    for (const auto& file : files) {
        chain.Add(file.c_str());
    }

    const long total_entries = chain.GetEntries();
    if (total_entries == 0) {
        cerr << "错误: 没有找到有效的entry" << endl;
        return result;
    }

    // 设置分支地址
    double cos_theta = 0.0;
    chain.SetBranchAddress("cosTheta", &cos_theta);

    // 进度显示设置
    const int print_interval = max(1, static_cast<int>(total_entries/10));
    cout << "开始处理 " << total_entries << " 个entry..." << endl;

    double total_rad = 0.0;
    for (long i = 0; i < total_entries; ++i) {
        chain.GetEntry(i);

        // 限制数值范围并计算角度
        const double val = max(min(cos_theta, 1.0), -1.0);
        total_rad += TMath::ACos(val);

        // 显示进度
        if (i % print_interval == 0) {
            cout << "已处理: " << i << "/" << total_entries << " (" 
                 << (i*100.0/total_entries) << "%)" << endl;
        }
    }

    // 计算结果
    result.entries = total_entries;
    result.mean_rad = total_rad / total_entries;
    result.mean_deg = result.mean_rad * TMath::RadToDeg();

    return result;
}

int main() {
    // 文件列表
    vector<string> files = {
        "../Output/MCMuons/MCRun_*.root"
    };

    // 执行分析
    auto result = calculate_theta_mean(files);

    // 输出结果
    if (result.entries > 0) {
        cout << "\n结果统计:" << endl;
        cout << "处理文件数量: " << files.size() << endl;
        cout << "总entry数: " << result.entries << endl;
        cout.precision(6);
        cout << fixed << "Theta平均值（弧度）: " << result.mean_rad << endl;
        cout << fixed << "Theta平均值（角度）: " << result.mean_deg << endl;
    } else {
        cerr << "没有有效数据" << endl;
    }

    return 0;
}