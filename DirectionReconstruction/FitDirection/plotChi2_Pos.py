import uproot
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.special import gamma
import traceback

def geodesic_distance(theta1, phi1, theta2, phi2):
    """计算球面上两点间的测地线距离（弧度）"""
    x1 = np.sin(theta1) * np.cos(phi1)
    y1 = np.sin(theta1) * np.sin(phi1)
    z1 = np.cos(theta1)
    x2 = np.sin(theta2) * np.cos(phi2)
    y2 = np.sin(theta2) * np.sin(phi2)
    z2 = np.cos(theta2)
    dot_product = np.clip(x1*x2 + y1*y2 + z1*z2, -1.0, 1.0)
    return np.arccos(dot_product)

def chi2_pdf(x, k, scale=1.0):
    """卡方分布概率密度函数"""
    return (x/scale)**(k/2-1) * np.exp(-x/(2*scale)) / (2**(k/2) * gamma(k/2) * scale)

def plot_geodesic_chi2(root_file, tree_name="Direction"):
    with uproot.open(root_file) as file:
        tree = file[tree_name]
        data = tree.arrays(["costheta_truth", "phi_truth", "costheta_rec", "phi_rec"])
    
    # 处理数据有效性并避免NaN
    costheta_true = np.clip(data["costheta_truth"], -1.0, 1.0)
    costheta_rec = np.clip(data["costheta_rec"], -1.0, 1.0)
    theta_true = np.arccos(costheta_true)
    theta_rec = np.arccos(costheta_rec)
    phi_true, phi_rec = data["phi_truth"], data["phi_rec"]
    
    # 计算测地距离并筛选有效数据
    geodesic_dists = geodesic_distance(theta_true, phi_true, theta_rec, phi_rec)
    mask = ~np.isnan(geodesic_dists)  # 过滤NaN
    geodesic_dists = geodesic_dists[mask]
    
    dist_sq_rad = geodesic_dists**2
    max_sq_rad = 0.25
    valid_mask = dist_sq_rad <= max_sq_rad
    filtered_dist_sq_rad = dist_sq_rad[valid_mask]
    
    if len(filtered_dist_sq_rad) == 0:
        raise ValueError("无有效数据用于拟合")
    
    # 转换为角度
    rad_to_deg = 180 / np.pi
    filtered_dists_deg = geodesic_dists[valid_mask] * rad_to_deg
    
    plt.figure(figsize=(12, 8))
    ax1 = plt.subplot(2, 1, 1)
    n_bins = 50
    ax1.hist(filtered_dists_deg, bins=n_bins, density=True, alpha=0.7, color='steelblue', label='Geodesic Distance')
    ax1.set(xlabel='Distance (deg)', ylabel='Density', title='Geodesic Distance Distribution')
    
    # 卡方拟合
    ax2 = plt.subplot(2, 1, 2)
    counts, bins, _ = ax2.hist(filtered_dist_sq_rad * (rad_to_deg**2), bins=n_bins, density=True, 
                              alpha=0.7, color='darkorange', label='Squared Distance')
    bin_centers = (bins[:-1] + bins[1:]) / 2
    
    try:
        # 确保使用NumPy数组
        x_data = np.asarray(bin_centers / (rad_to_deg**2))  # 转换回弧度²
        y_data = np.asarray(counts) * (rad_to_deg**2)       # 调整密度单位
    
        # 初始参数估计
        k0 = 2.0
        scale0 = np.mean(filtered_dist_sq_rad) / k0
        
        # 执行拟合
        popt, pcov = curve_fit(lambda x, k, s: chi2_pdf(x, k, s), 
                              x_data, y_data, p0=[k0, scale0], 
                              bounds=([1, 1e-3], [10, 10]))
        k, scale = popt
        
        # 绘制拟合曲线
        x_fit = np.linspace(0, max_sq_rad, 500)
        y_fit = chi2_pdf(x_fit, k, scale) * (rad_to_deg**2)
        ax2.plot(x_fit * (rad_to_deg**2), y_fit, 'r-', lw=2, 
                label=f'Fit: k={k:.2f}, scale={scale:.4f} rad²')
    except Exception as e:
        print(f"拟合失败: {e}")
        traceback.print_exc()
    
    ax2.legend()
    plt.tight_layout()
    plt.show()

# 示例调用
plot_geodesic_chi2("Output/ReconMC/AA_75/ReconMC_k10.root")