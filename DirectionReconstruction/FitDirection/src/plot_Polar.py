import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import uproot
import os

# 文件路径
MuonFile = "../Output/ReconData/ReconMuonsData_WCT_t4.root"
FigureFile = "../pic/PolarDistribution_WCT_t4.pdf"
ModelFile = "../Output/ReconMC/Test_WCT_t4/ReconMC_k20.root"

def main():
    # 读取模型数据
    try:
        f_model = uproot.open(ModelFile)
        t_model = f_model["Direction"]
        costheta_rec = t_model["costheta_rec"].array(library="np")
        phi_rec = t_model["phi_rec"].array(library="np")
        theta_model = np.arccos(costheta_rec)
        phi_model = phi_rec
    except Exception as e:
        print(f"Error reading model data: {e}")
        return

    # 读取实际数据
    try:
        f_muon = uproot.open(MuonFile)
        t_muon = f_muon["Direction"]
        costheta_rec = t_muon["costheta_rec"].array(library="np")
        phi_rec = t_muon["phi_rec"].array(library="np")
        theta_data = np.arccos(costheta_rec)
        phi_data = phi_rec
    except Exception as e:
        print(f"Error reading muon data: {e}")
        return

    # 创建图形
    plt.figure(figsize=(8, 8))
    ax = plt.subplot(111, projection='polar')
    
    # 绘制模型数据的密度分布
    n_theta_bins = 30
    n_phi_bins = 36
    
    # 创建cosθ等间距分bin
    cos_bins = np.linspace(1, 0, n_theta_bins + 1)
    theta_bins = np.arccos(cos_bins)
    
    # 创建phi分bin边界
    phi_bins = np.linspace(-np.pi, np.pi, n_phi_bins + 1)
    
    # 计算二维直方图
    hist, phi_edges, theta_edges = np.histogram2d(
        phi_model, 
        theta_model, 
        bins=[phi_bins, theta_bins],
    )
    
    # 创建自定义色阶 - 接近0时使用浅色（反转的viridis色阶）
    cmap = plt.cm.viridis_r.copy()  # 使用反转的viridis色阶
    cmap.set_under('white', alpha=0)  # 设置0计数为透明
    
    # 创建网格点
    phi_grid, theta_grid = np.meshgrid(phi_edges, theta_edges)
    
    # 绘制伪彩色图，设置vmin使0值透明
    mesh = ax.pcolormesh(phi_grid, theta_grid, hist.T, 
                         cmap=cmap, 
                         shading='auto', 
                         alpha=0.9,  # 整体透明度
                         vmin=0.1)    # 低于此值显示为透明
    
    # 添加颜色条
    cbar = plt.colorbar(mesh, ax=ax, pad=0.1)
    cbar.set_label('Event Density')
    
    # 绘制数据点 - 使用"x"符号
    ax.scatter(phi_data, theta_data, s=50,  # 增大符号尺寸
               marker='x',  # 使用"x"符号
               color='red',  # 红色更醒目
               alpha=0.9, 
               linewidths=1.5,  # 线宽
               label='Data', 
               zorder=5)  # 确保在顶部
    
    # 为每个数据点添加序号标签
    for i, (phi, theta) in enumerate(zip(phi_data, theta_data)):
        # 计算标签位置 - 稍微偏移避免重叠
        label_theta = theta + 0.02  # 径向偏移
        label_phi = phi + 0.05     # 角度偏移
        
        # 添加序号标签
        ax.text(label_phi, label_theta, str(i), 
                color='red', fontsize=8, 
                ha='center', va='center',
                bbox=dict(boxstyle='round,pad=0.2', 
                          fc='white', ec='red', alpha=0.7))
    
    # 设置极坐标图属性
    ax.set_theta_offset(np.pi/2)  # 设置0°在顶部
    ax.set_theta_direction(-1)    # 顺时针方向
    ax.set_rlabel_position(135)   # 径向标签位置
    ax.set_rlim(0, np.pi/2)       # 设置径向范围 [0, π/2]
    
    # 添加径向刻度
    r_ticks = np.linspace(0, np.pi/2, 5)
    r_tick_labels = [f"{np.rad2deg(t):.0f}°" for t in r_ticks]
    ax.set_rticks(r_ticks)
    ax.set_yticklabels(r_tick_labels)
    
    # 添加角度刻度
    ax.set_xticks(np.linspace(0, 2*np.pi, 8, endpoint=False))
    ax.set_xticklabels(['0°', '45°', '90°', '135°', '180°', '225°', '270°', '315°'])
    
    # 添加标题和图例
    plt.title('Muon Angular Distribution', pad=20, fontsize=14)
    plt.legend(loc='upper right', bbox_to_anchor=(1.15, 1.15), framealpha=0.9)
    
    # 添加网格线增强可读性
    ax.grid(True, color='gray', linestyle='--', alpha=0.5)
    
    # 保存图像
    plt.tight_layout()
    plt.savefig(FigureFile, dpi=300, bbox_inches='tight')
    print(f"Plot saved to {os.path.abspath(FigureFile)}")
    
    # 显示图像
    plt.show()

if __name__ == "__main__":
    main()