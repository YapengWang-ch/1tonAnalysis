import uproot
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm
from matplotlib.gridspec import GridSpec

def plot_residuals(root_file, tree_name="Direction"):
    """
    绘制方向重建残差分布图
    
    参数:
        root_file: ROOT文件路径
        tree_name: TTree名称 (默认为"Direction")
    """
    # 读取数据
    with uproot.open(root_file) as file:
        tree = file[tree_name]
        data = tree.arrays(["costheta_truth", "phi_truth", 
                          "costheta_rec", "phi_rec"])

    # 计算残差
    res_cos_theta = data["costheta_rec"] - data["costheta_truth"]
    res_phi = data["phi_rec"] - data["phi_truth"]
    res_phi = np.arctan2(np.sin(res_phi), np.cos(res_phi))  # 自动映射到[-π, π]
    res_phi = res_phi*180/3.1415926  # 转换为度

    # 创建画布
    plt.figure(figsize=(12, 8), dpi=100)
    gs = GridSpec(3, 2, height_ratios=[3, 3, 1])
    
    # 设置全局绘图参数
    plt.rcParams.update({
        'font.size': 12,
        'mathtext.fontset': 'stix',
        'axes.unicode_minus': False
    })

    # ========== cosα残差 ==========
    ax1 = plt.subplot(gs[0, 0])
    n_bins = 50
    
    # 初始直方图（统计量模式）
    counts, bins, _ = ax1.hist(res_cos_theta, 
                              bins=n_bins,
                              alpha=0.7,
                              color='royalblue',
                              label='Data')
    bin_width = bins[1] - bins[0]
    
    # 初始拟合（用于确定截断范围）
    mu0, sigma0 = norm.fit(res_cos_theta)
    low_bound, high_bound = mu0 - 3*sigma0, mu0 + 3*sigma0
    
    # 截断离群值
    filtered_data = res_cos_theta[(res_cos_theta >= low_bound) & (res_cos_theta <= high_bound)]
    
    # 使用截断后数据重新拟合
    mu, sigma = norm.fit(filtered_data)
    x = np.linspace(low_bound, high_bound, 300)
    
    # 计算拟合曲线（转换为统计量）
    fit_curve = norm.pdf(x, mu, sigma) * len(filtered_data) * bin_width
    ax1.plot(x, fit_curve, 
            'r-', lw=2, 
            label=f'Fit (3σ cut)\nμ={mu:.4f}\nσ={sigma:.4f}')
    
    # 添加截断范围标记
    ax1.axvline(low_bound, color='gray', linestyle='--', alpha=0.7)
    ax1.axvline(high_bound, color='gray', linestyle='--', alpha=0.7)
    
    ax1.set_ylabel('Counts', fontsize=12)
    ax1.set_title(r'$\cos\theta$ Residual', pad=15)
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    # ========== β残差 ==========
    ax2 = plt.subplot(gs[0, 1])
    counts_beta, bins_beta, _ = ax2.hist(res_phi, 
                                       bins=n_bins,
                                       alpha=0.7,
                                       color='forestgreen',
                                       label='Data')
    bin_width_beta = bins_beta[1] - bins_beta[0]
    
    # 初始拟合（确定截断范围）
    mu0_phi, sigma0_phi = norm.fit(res_phi)
    low_bound_beta = mu0_phi - 3*sigma0_phi
    high_bound_beta = mu0_phi + 3*sigma0_phi
    
    # 截断离群值
    filtered_data_beta = res_phi[(res_phi >= low_bound_beta) & (res_phi <= high_bound_beta)]
    
    # 使用截断后数据重新拟合
    mu_phi, sigma_phi = norm.fit(filtered_data_beta)
    x_beta = np.linspace(low_bound_beta, high_bound_beta, 300)
    
    # 计算拟合曲线（转换为统计量）
    fit_curve_beta = norm.pdf(x_beta, mu_phi, sigma_phi) * len(filtered_data_beta) * bin_width_beta
    ax2.plot(x_beta, fit_curve_beta, 
            'r-', lw=2,
            label=f'Fit (3σ cut)\nμ={mu_phi:.4f}\nσ={sigma_phi:.4f}')
    
    # 添加截断范围标记
    ax2.axvline(low_bound_beta, color='gray', linestyle='--', alpha=0.7)
    ax2.axvline(high_bound_beta, color='gray', linestyle='--', alpha=0.7)
    
    ax2.set_title(r'$\phi$ Residual (degrees)', pad=15)
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    # ========== 统计信息表格 ==========
    ax3 = plt.subplot(gs[1, :])
    ax3.axis('off')
    
    # 创建统计表格
    table_data = [
        [r'$\cos\theta$ Residual', f'{mu:.4f}', f'{sigma:.4f}', f'{len(filtered_data)}/{len(res_cos_theta)}'],
        [r'$\phi$ Residual', f'{mu_phi:.4f}', f'{sigma_phi:.4f}', f'{len(filtered_data_beta)}/{len(res_phi)}']
    ]
    
    col_labels = ['Variable', 'Mean (μ)', 'Std Dev (σ)', 'Points Used']
    
    table = ax3.table(cellText=table_data,
                     colLabels=col_labels,
                     loc='center',
                     cellLoc='center')
    
    table.auto_set_font_size(False)
    table.set_fontsize(12)
    table.scale(1.2, 1.5)
    
    # 添加标题
    plt.suptitle('Reconstruction Residuals with 3sigma Outlier Cut', fontsize=16)
    plt.tight_layout(rect=[0, 0, 1, 0.97])  # 为标题留出空间
    plt.savefig("ReconErrorIncident_CT_thetaphi.pdf")
    plt.close()

# 使用示例
plot_residuals("Output/ReconMC/Test_CT/ReconMC_k10.root")