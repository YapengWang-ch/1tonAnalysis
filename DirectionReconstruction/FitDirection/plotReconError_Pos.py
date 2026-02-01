import uproot
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from matplotlib.gridspec import GridSpec

def gaussian(x, mu, sigma, amplitude):
    """高斯函数定义"""
    return amplitude * np.exp(-(x - mu)**2 / (2 * sigma**2))

def plot_residuals(root_file, tree_name="Direction"):
    """
    绘制方向重建残差分布图（带参数不确定度）
    """
    # 读取数据
    try:
        with uproot.open(root_file) as file:
            tree = file[tree_name]
            # 使用to_numpy()方法获取numpy数组
            cosalpha_truth = tree["cosalpha_truth"].array(library="np")
            beta_truth = tree["beta_truth"].array(library="np")
            cosalpha_rec = tree["cosalpha_rec"].array(library="np")
            beta_rec = tree["beta_rec"].array(library="np")
    except Exception as e:
        print(f"无法读取文件 {root_file}: {e}")
        return
    
    print(f"原始数据点数: {len(cosalpha_truth)}")
    
    # 检查并过滤NaN值
    valid_mask = (
        np.isfinite(cosalpha_truth) & 
        np.isfinite(beta_truth) & 
        np.isfinite(cosalpha_rec) & 
        np.isfinite(beta_rec)
    )
    
    cosalpha_truth = cosalpha_truth[valid_mask]
    beta_truth = beta_truth[valid_mask]
    cosalpha_rec = cosalpha_rec[valid_mask]
    beta_rec = beta_rec[valid_mask]
    
    print(f"过滤NaN后数据点数: {len(cosalpha_truth)}")
    
    if len(cosalpha_truth) == 0:
        print("错误: 所有数据都是NaN或无效值")
        return
    
    # 计算残差
    res_cos_alpha = cosalpha_rec - cosalpha_truth
    res_beta = beta_rec - beta_truth
    res_beta = np.arctan2(np.sin(res_beta), np.cos(res_beta))  # 自动映射到[-π, π]
    res_beta = res_beta * 180 / np.pi  # 转换为度
    
    # 再次检查残差中的NaN
    res_cos_alpha = res_cos_alpha[np.isfinite(res_cos_alpha)]
    res_beta = res_beta[np.isfinite(res_beta)]
    
    print(f"cosα残差有效点数: {len(res_cos_alpha)}")
    print(f"β残差有效点数: {len(res_beta)}")
    print(f"cosα残差范围: [{np.min(res_cos_alpha):.6f}, {np.max(res_cos_alpha):.6f}]")
    print(f"β残差范围: [{np.min(res_beta):.6f}, {np.max(res_beta):.6f}]")
    
    if len(res_cos_alpha) == 0 or len(res_beta) == 0:
        print("错误: 残差计算后无有效数据")
        return
    
    # 创建画布
    plt.figure(figsize=(12, 8), dpi=100)
    gs = GridSpec(3, 2, height_ratios=[3, 3, 1])
    
    plt.rcParams.update({
        'font.size': 12,
        'mathtext.fontset': 'stix',
        'axes.unicode_minus': False
    })
    
    # ========== cosα残差 ==========
    ax1 = plt.subplot(gs[0, 0])
    n_bins = 50
    
    # 绘制直方图
    if len(res_cos_alpha) > 0:
        counts, bins, _ = ax1.hist(res_cos_alpha, 
                                  bins=n_bins,
                                  alpha=0.7,
                                  color='royalblue',
                                  label=f'Data (N={len(res_cos_alpha)})')
    else:
        ax1.text(0.5, 0.5, 'No data available', 
                ha='center', va='center', transform=ax1.transAxes)
        ax1.set_title(r'$\cos\alpha$ Residual')
        ax1.grid(True, alpha=0.3)
    
    # 计算基本统计量
    if len(res_cos_alpha) > 0:
        mu0 = np.mean(res_cos_alpha)
        sigma0 = np.std(res_cos_alpha)
        print(f"初始cosα均值: {mu0:.6f}, 标准差: {sigma0:.6f}")
        
        # 自适应截断范围
        if sigma0 > 0 and np.isfinite(sigma0):
            # 如果标准差较小，扩大截断范围
            if sigma0 < 0.001:  # 非常小的标准差
                low_bound, high_bound = mu0 - 5*sigma0, mu0 + 5*sigma0
            else:
                low_bound, high_bound = mu0 - 3*sigma0, mu0 + 3*sigma0
        else:
            # 标准差为0或NaN，使用完整数据范围
            low_bound, high_bound = np.min(res_cos_alpha), np.max(res_cos_alpha)
        
        print(f"cosα截断范围: [{low_bound:.6f}, {high_bound:.6f}]")
        
        # 截断离群值
        mask = (res_cos_alpha >= low_bound) & (res_cos_alpha <= high_bound)
        filtered_data = res_cos_alpha[mask]
        print(f"cosα截断后数据点: {len(filtered_data)}/{len(res_cos_alpha)}")
        
        # 检查是否有足够数据拟合
        if len(filtered_data) >= 5:  # 至少5个点才能拟合
            # 重新计算直方图以便拟合
            hist_counts, hist_bins = np.histogram(filtered_data, bins=n_bins)
            bin_centers_filt = (hist_bins[:-1] + hist_bins[1:]) / 2
            bin_width = hist_bins[1] - hist_bins[0]
            
            # 确保有非零计数
            if np.sum(hist_counts) > 0:
                # 初始参数估计
                amplitude0 = np.max(hist_counts) * bin_width
                
                # 避免过小的标准差
                sigma_est = max(np.std(filtered_data), 1e-6)
                p0 = [np.mean(filtered_data), sigma_est, amplitude0]
                
                try:
                    # 执行拟合（添加边界条件，增加maxfev和更好的初始参数）
                    popt, pcov = curve_fit(gaussian, 
                                         bin_centers_filt, 
                                         hist_counts, 
                                         p0=p0,
                                         bounds=([-np.inf, 1e-6, 0], 
                                                [np.inf, np.inf, np.inf]),
                                         maxfev=20000,
                                         method='trf')  # 使用不同的方法
                    
                    mu, sigma, amplitude = popt
                    mu_err, sigma_err, _ = np.sqrt(np.diag(pcov))
                    
                    # 计算拟合曲线
                    x = np.linspace(low_bound, high_bound, 300)
                    fit_curve = gaussian(x, mu, sigma, amplitude)
                    ax1.plot(x, fit_curve, 
                            'r-', lw=2, 
                            label=f'Gaussian fit\nμ = {mu:.4f} ± {mu_err:.4f}\nσ = {sigma:.4f} ± {sigma_err:.4f}')
                    
                    fit_success = True
                    fit_params_cos = (mu, mu_err, sigma, sigma_err, len(filtered_data), len(res_cos_alpha))
                    
                except Exception as e:
                    print(f"cosα拟合失败: {e}")
                    print("尝试使用更简单的拟合方法...")
                    # 尝试不使用边界条件
                    try:
                        popt, pcov = curve_fit(gaussian, 
                                             bin_centers_filt, 
                                             hist_counts, 
                                             p0=p0,
                                             maxfev=20000)
                        
                        mu, sigma, amplitude = popt
                        mu_err, sigma_err, _ = np.sqrt(np.diag(pcov))
                        
                        # 计算拟合曲线
                        x = np.linspace(low_bound, high_bound, 300)
                        fit_curve = gaussian(x, mu, sigma, amplitude)
                        ax1.plot(x, fit_curve, 
                                'r-', lw=2, 
                                label=f'Gaussian fit (no bounds)\nμ = {mu:.4f} ± {mu_err:.4f}\nσ = {sigma:.4f} ± {sigma_err:.4f}')
                        
                        fit_success = True
                        fit_params_cos = (mu, mu_err, sigma, sigma_err, len(filtered_data), len(res_cos_alpha))
                        
                    except Exception as e2:
                        print(f"cosα再次拟合失败: {e2}")
                        fit_success = False
            else:
                print("cosα截断后直方图计数为0")
                fit_success = False
        else:
            print(f"cosα数据点不足进行拟合: {len(filtered_data)}个点")
            fit_success = False
    else:
        fit_success = False
    
    # 如果拟合失败，使用简单统计
    if not fit_success:
        if len(res_cos_alpha) > 0:
            mu = np.mean(res_cos_alpha)
            sigma = np.std(res_cos_alpha)
            mu_err = sigma / np.sqrt(len(res_cos_alpha)) if len(res_cos_alpha) > 0 else 0
            sigma_err = sigma / np.sqrt(2*(len(res_cos_alpha)-1)) if len(res_cos_alpha) > 1 else 0
            fit_params_cos = (mu, mu_err, sigma, sigma_err, len(res_cos_alpha), len(res_cos_alpha))
            ax1.plot([], [], 'r-', label=f'Simple stats\nμ = {mu:.4f}\nσ = {sigma:.4f}')
        else:
            fit_params_cos = (0, 0, 0, 0, 0, 0)
    
    # 添加截断范围标记
    if len(res_cos_alpha) > 0 and 'low_bound' in locals() and np.isfinite(low_bound):
        ax1.axvline(low_bound, color='gray', linestyle='--', alpha=0.7)
        ax1.axvline(high_bound, color='gray', linestyle='--', alpha=0.7)
    
    ax1.set_ylabel('Counts')
    ax1.set_title(r'$\cos\alpha$ Residual')
    if len(res_cos_alpha) > 0:
        ax1.legend(fontsize=10)
    ax1.grid(True, alpha=0.3)
    
    # ========== β残差 ==========
    ax2 = plt.subplot(gs[0, 1])
    
    if len(res_beta) > 0:
        counts_beta, bins_beta, _ = ax2.hist(res_beta, 
                                           bins=n_bins,
                                           alpha=0.7,
                                           color='forestgreen',
                                           label=f'Data (N={len(res_beta)})')
    else:
        ax2.text(0.5, 0.5, 'No data available', 
                ha='center', va='center', transform=ax2.transAxes)
        ax2.set_title(r'$\beta$ Residual (degrees)')
        ax2.grid(True, alpha=0.3)
    
    # β残差的处理（与cosα类似）
    if len(res_beta) > 0:
        mu0_beta = np.mean(res_beta)
        sigma0_beta = np.std(res_beta)
        print(f"初始β均值: {mu0_beta:.6f}, 标准差: {sigma0_beta:.6f}")
        
        # 对于β残差，由于周期性，可能需要特殊处理
        # 检查是否有数据在边界附近
        near_boundary = np.sum(np.abs(res_beta) > 170) > len(res_beta) * 0.1  # 超过10%的数据在边界附近
        
        if near_boundary:
            print("注意: 大量β残差在边界附近，可能不适合高斯拟合")
        
        # 自适应截断范围
        if sigma0_beta > 0 and np.isfinite(sigma0_beta):
            if sigma0_beta < 0.1:  # 非常小的标准差
                low_bound_beta, high_bound_beta = mu0_beta - 5*sigma0_beta, mu0_beta + 5*sigma0_beta
            else:
                low_bound_beta, high_bound_beta = mu0_beta - 3*sigma0_beta, mu0_beta + 3*sigma0_beta
        else:
            low_bound_beta, high_bound_beta = np.min(res_beta), np.max(res_beta)
        
        print(f"β截断范围: [{low_bound_beta:.6f}, {high_bound_beta:.6f}]")
        
        # 截断离群值
        mask_beta = (res_beta >= low_bound_beta) & (res_beta <= high_bound_beta)
        filtered_data_beta = res_beta[mask_beta]
        print(f"β截断后数据点: {len(filtered_data_beta)}/{len(res_beta)}")
        
        # 检查是否有足够数据拟合
        if len(filtered_data_beta) >= 5:
            # 重新计算直方图以便拟合
            hist_counts_beta, hist_bins_beta = np.histogram(filtered_data_beta, bins=n_bins)
            bin_centers_beta_filt = (hist_bins_beta[:-1] + hist_bins_beta[1:]) / 2
            bin_width_beta = hist_bins_beta[1] - hist_bins_beta[0]
            
            # 确保有非零计数
            if np.sum(hist_counts_beta) > 0:
                # 初始参数估计
                amplitude0_beta = np.max(hist_counts_beta) * bin_width_beta
                
                # 避免过小的标准差
                sigma_est_beta = max(np.std(filtered_data_beta), 1e-6)
                p0_beta = [np.mean(filtered_data_beta), sigma_est_beta, amplitude0_beta]
                
                try:
                    # 执行拟合
                    popt_beta, pcov_beta = curve_fit(gaussian, 
                                                   bin_centers_beta_filt, 
                                                   hist_counts_beta, 
                                                   p0=p0_beta,
                                                   bounds=([-np.inf, 1e-6, 0], 
                                                          [np.inf, np.inf, np.inf]),
                                                   maxfev=20000,
                                                   method='trf')
                    
                    mu_beta, sigma_beta, amplitude_beta = popt_beta
                    mu_beta_err, sigma_beta_err, _ = np.sqrt(np.diag(pcov_beta))
                    
                    # 计算拟合曲线
                    x_beta = np.linspace(low_bound_beta, high_bound_beta, 300)
                    fit_curve_beta = gaussian(x_beta, mu_beta, sigma_beta, amplitude_beta)
                    ax2.plot(x_beta, fit_curve_beta, 
                            'r-', lw=2,
                            label=f'Gaussian fit\nμ = {mu_beta:.4f} ± {mu_beta_err:.4f}\nσ = {sigma_beta:.4f} ± {sigma_beta_err:.4f}')
                    
                    fit_success_beta = True
                    fit_params_beta = (mu_beta, mu_beta_err, sigma_beta, sigma_beta_err, 
                                     len(filtered_data_beta), len(res_beta))
                    
                except Exception as e:
                    print(f"β拟合失败: {e}")
                    print("尝试使用更简单的拟合方法...")
                    # 尝试不使用边界条件
                    try:
                        popt_beta, pcov_beta = curve_fit(gaussian, 
                                                       bin_centers_beta_filt, 
                                                       hist_counts_beta, 
                                                       p0=p0_beta,
                                                       maxfev=20000)
                        
                        mu_beta, sigma_beta, amplitude_beta = popt_beta
                        mu_beta_err, sigma_beta_err, _ = np.sqrt(np.diag(pcov_beta))
                        
                        # 计算拟合曲线
                        x_beta = np.linspace(low_bound_beta, high_bound_beta, 300)
                        fit_curve_beta = gaussian(x_beta, mu_beta, sigma_beta, amplitude_beta)
                        ax2.plot(x_beta, fit_curve_beta, 
                                'r-', lw=2,
                                label=f'Gaussian fit (no bounds)\nμ = {mu_beta:.4f} ± {mu_beta_err:.4f}\nσ = {sigma_beta:.4f} ± {sigma_beta_err:.4f}')
                        
                        fit_success_beta = True
                        fit_params_beta = (mu_beta, mu_beta_err, sigma_beta, sigma_beta_err, 
                                         len(filtered_data_beta), len(res_beta))
                        
                    except Exception as e2:
                        print(f"β再次拟合失败: {e2}")
                        fit_success_beta = False
            else:
                print("β截断后直方图计数为0")
                fit_success_beta = False
        else:
            print(f"β数据点不足进行拟合: {len(filtered_data_beta)}个点")
            fit_success_beta = False
    else:
        fit_success_beta = False
    
    # 如果拟合失败，使用简单统计
    if not fit_success_beta:
        if len(res_beta) > 0:
            mu_beta = np.mean(res_beta)
            sigma_beta = np.std(res_beta)
            mu_beta_err = sigma_beta / np.sqrt(len(res_beta)) if len(res_beta) > 0 else 0
            sigma_beta_err = sigma_beta / np.sqrt(2*(len(res_beta)-1)) if len(res_beta) > 1 else 0
            fit_params_beta = (mu_beta, mu_beta_err, sigma_beta, sigma_beta_err, 
                             len(res_beta), len(res_beta))
            ax2.plot([], [], 'r-', label=f'Simple stats\nμ = {mu_beta:.4f}\nσ = {sigma_beta:.4f}')
        else:
            fit_params_beta = (0, 0, 0, 0, 0, 0)
    
    # 添加截断范围标记
    if len(res_beta) > 0 and 'low_bound_beta' in locals() and np.isfinite(low_bound_beta):
        ax2.axvline(low_bound_beta, color='gray', linestyle='--', alpha=0.7)
        ax2.axvline(high_bound_beta, color='gray', linestyle='--', alpha=0.7)
    
    ax2.set_title(r'$\beta$ Residual (degrees)')
    if len(res_beta) > 0:
        ax2.legend(fontsize=10)
    ax2.grid(True, alpha=0.3)
    
    # ========== 统计信息表格 ==========
    ax3 = plt.subplot(gs[1, :])
    ax3.axis('off')
    
    # 创建统计表格
    table_data = []
    
    # 检查是否成功获取参数
    if 'fit_params_cos' in locals() and len(fit_params_cos) >= 6:
        mu_cos, mu_err_cos, sigma_cos, sigma_err_cos, n_used_cos, n_total_cos = fit_params_cos
        table_data.append([
            r'$\cos\alpha$ Residual', 
            f'{mu_cos:.4f} ± {mu_err_cos:.4f}', 
            f'{sigma_cos:.4f} ± {sigma_err_cos:.4f}',
            f'{n_used_cos}/{n_total_cos}'
        ])
    else:
        table_data.append([
            r'$\cos\alpha$ Residual', 
            'N/A', 
            'N/A',
            f'0/{len(res_cos_alpha)}'
        ])
    
    if 'fit_params_beta' in locals() and len(fit_params_beta) >= 6:
        mu_beta, mu_err_beta, sigma_beta, sigma_err_beta, n_used_beta, n_total_beta = fit_params_beta
        table_data.append([
            r'$\beta$ Residual', 
            f'{mu_beta:.4f} ± {mu_err_beta:.4f}', 
            f'{sigma_beta:.4f} ± {sigma_err_beta:.4f}',
            f'{n_used_beta}/{n_total_beta}'
        ])
    else:
        table_data.append([
            r'$\beta$ Residual', 
            'N/A', 
            'N/A',
            f'0/{len(res_beta)}'
        ])
    
    col_labels = ['Variable', 'Mean ± SE', 'Std Dev ± SE', 'Points Used']
    
    if table_data:
        table = ax3.table(
            cellText=table_data,
            colLabels=col_labels,
            loc='center',
            cellLoc='center',
            colColours=['#f0f0f0'] * 4
        )
        
        table.auto_set_font_size(False)
        table.set_fontsize(11)
        table.scale(1.2, 1.8)
        
        # 设置表头样式
        for i in range(len(col_labels)):
            table[(0, i)].set_facecolor('#404060')
            table[(0, i)].set_text_props(color='white', weight='bold')
    
    # 添加标题
    plt.suptitle('Reconstruction Residuals with Data Quality Check', fontsize=16)
    plt.tight_layout(rect=[0, 0, 1, 0.97])
    
    # 保存图像
    output_file = "ReconErrorIncident_WCT_pos.pdf"
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"图像已保存到: {output_file}")
    
    plt.close()

# 使用示例
plot_residuals("Output/ReconMC/Test_WCT_t4/ReconMC_k10.root")