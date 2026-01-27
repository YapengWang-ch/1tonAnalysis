import matplotlib.pyplot as plt
import numpy as np

def read_data(filename):
    """读取数据文件，返回通道和增益列表"""
    channels = []
    gains = []
    
    try:
        with open(filename, 'r') as file:
            lines = file.readlines()
            for line in lines[1:]:
                if line.strip():
                    parts = line.split()
                    channels.append(int(parts[0]))
                    gains.append(float(parts[1]))
        return channels, gains
    except Exception as e:
        print(f"读取文件 {filename} 时出错：{e}")
        return [], []

def calculate_statistics(gains, name):
    """计算并显示统计信息"""
    mean_gain = np.mean(gains)
    max_gain = np.max(gains)
    min_gain = np.min(gains)
    std_gain = np.std(gains)
    
    print(f"\n{name}统计信息:")
    print(f"  平均增益: {mean_gain:.2f}")
    print(f"  最大增益: {max_gain:.2f}")
    print(f"  最小增益: {min_gain:.2f}")
    print(f"  标准差: {std_gain:.2f}")
    
    return mean_gain, max_gain, min_gain

# 读取两个数据文件
file1 = './output/gain/CAEN_new_mask2.txt'  # 请替换为实际文件名
file2 = './output/gain/Elec_100_reselect_mask.txt'  # 请替换为实际文件名

channels1, gains1 = read_data(file1)
channels2, gains2 = read_data(file2)

if not channels1 or not channels2:
    print("数据读取失败，请检查文件路径和格式")
    exit()

# 计算统计信息
mean1, max1, min1 = calculate_statistics(gains1, "数据集1")
mean2, max2, min2 = calculate_statistics(gains2, "数据集2")

# 创建子图
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10))

# 第一个子图：两条折线对比
ax1.plot(channels1, gains1, 'b-o', linewidth=2, markersize=4, label='Fit', alpha=0.8)
ax1.plot(channels2, gains2, 'r-s', linewidth=2, markersize=4, label='Mean', alpha=0.8)

ax1.set_xlabel('Channel Index')
ax1.set_ylabel('Gain')
ax1.set_title('Channel Gain Comparison')
ax1.grid(True, alpha=0.3)
ax1.legend()
ax1.set_xticks(np.arange(0, max(max(channels1), max(channels2))+1, 5))

# 第二个子图：增益差异
gain_differences = [g2/g1 for g1, g2 in zip(gains1, gains2)]
ax2.plot(channels1, gain_differences, 'g-^', linewidth=2, markersize=4, label='Elec/CAEN', alpha=0.8)
ax2.axhline(y=0, color='black', linestyle='-', alpha=0.3)

ax2.set_xlabel('Channel Index')
ax2.set_ylabel('Gain Ratio')
ax2.set_title('Elec/CAEN ratio')
ax2.grid(True, alpha=0.3)
ax2.legend()
ax2.set_xticks(np.arange(0, max(max(channels1), max(channels2))+1, 5))

# 自动调整布局
plt.tight_layout()

# 显示图表
# plt.show()

# 可选：保存图表
plt.savefig('channel_gain_comparison_100_2.png', dpi=300, bbox_inches='tight')