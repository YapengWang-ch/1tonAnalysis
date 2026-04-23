import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import pearsonr

# 文件路径 - 请根据实际情况修改
time_file = "output/Test/Beta_Uni5.txt"  # 时间数据文件
timeref_file = "output/Test/Beta_All.txt"
pos_file = "data/PMTPosition.txt"    # 位置数据文件

# 解析时间数据文件
time_values = {}
with open(time_file, 'r') as f:
    for line in f:
        # 跳过注释行和分隔线
        if line.startswith('#') or line.startswith('-') or not line.strip():
            continue
        parts = line.split()
        # 确保有足够的数据列
        if len(parts) < 6:
            continue
        try:
            channel = int(parts[0])
            entries = int(parts[5])
            # 只考虑有数据记录的通道
            if entries > 0:
                time_val = float(parts[1])
                time_values[channel] = time_val
        except (ValueError, IndexError):
            continue

timeref_values = {}
with open(timeref_file, 'r') as f:
    for line in f:
        # 跳过注释行和分隔线
        if line.startswith('#') or line.startswith('-') or not line.strip():
            continue
        parts = line.split()
        # 确保有足够的数据列
        if len(parts) < 6:
            continue
        try:
            channel = int(parts[0])
            entries = int(parts[5])
            # 只考虑有数据记录的通道
            if entries > 0:
                time_val = float(parts[1])
                timeref_values[channel] = time_val
        except (ValueError, IndexError):
            continue

# 解析位置数据文件
x_positions = {}
with open(pos_file, 'r') as f:
    for line in f:
        if not line.strip():
            continue
        parts = line.split()
        if len(parts) < 4:
            continue
        try:
            channel = int(parts[0])
            x_pos = float(parts[1])
            x_positions[channel] = x_pos
        except (ValueError, IndexError):
            continue

# 匹配通道数据
channels = set(time_values.keys()) & set(timeref_values.keys()) & set(x_positions.keys())
if not channels:
    print("错误：没有找到匹配的通道数据！")
    exit()

times = [time_values[ch]-timeref_values[ch] for ch in channels]
x_vals = [x_positions[ch] for ch in channels]

# 计算相关系数
corr_coef, p_value = pearsonr(times, x_vals)

# 输出结果
print("="*60)
print(f"分析结果：")
print(f"有效通道数量: {len(channels)}")
print(f"时间范围: [{min(times):.6f}, {max(times):.6f}] ns")
print(f"X位置范围: [{min(x_vals):.6f}, {max(x_vals):.6f}] m")
print(f"皮尔逊相关系数: {corr_coef:.6f}")
print(f"P值: {p_value:.6e}")
print("="*60)

# 绘制散点图
plt.figure(figsize=(10, 6))
plt.scatter(x_vals, times, c='blue', alpha=0.7, edgecolors='w', s=80)

# # 添加回归线
# if abs(corr_coef) > 0.1:  # 仅在有明显相关性时添加回归线
#     z = np.polyfit(x_vals, times, 1)
#     p = np.poly1d(z)
#     plt.plot(x_vals, p(x_vals), "r--", linewidth=1.5, 
#              label=f'Linear fit: y = {z[0]:.4f}x + {z[1]:.4f}')

plt.title('Time Offset vs x_PMT', fontsize=14)
plt.xlabel('X (m)', fontsize=12)
plt.ylabel('Time Offset [nonuniform - uniform] (ns)', fontsize=12)
plt.grid(True, linestyle='--', alpha=0.7)

# 添加统计信息
stats_text = (f'Valid Channels: {len(channels)}\n'
              f'#rho: {corr_coef:.4f}\n'
              f'P-value: {p_value:.2e}')
plt.annotate(stats_text, xy=(0.05, 0.95), xycoords='axes fraction',
             fontsize=11, bbox=dict(boxstyle="round,pad=0.3", 
             fc="white", ec="gray", alpha=0.9))

# 添加图例（如果有回归线）
if abs(corr_coef) > 0.1:
    plt.legend(loc='best')

plt.tight_layout()
plt.savefig('time_vs_x_position_correlation5_ref.png', dpi=300, bbox_inches='tight')
print("已保存相关性图表: time_vs_x_position_correlation.png")
plt.show()