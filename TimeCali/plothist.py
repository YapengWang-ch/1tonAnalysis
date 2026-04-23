import matplotlib.pyplot as plt
import numpy as np

# 自定义bin边界
bins = [-60,-50, -40, -30, -20, -15, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 15, 20, 30, 40, 50,60]

# 对应数值
frequencies = [0,0, 0 ,4 ,24 ,178 ,360 ,1456,10648 ,9631 ,2752, 948, 149, 32, 17, 18, 40 ,23 ,14, 0, 0,0]
# frequencies=[0 ,0 ,0 ,5 ,20, 195 ,323 ,1530 ,6185 ,9999 ,9273 ,2695 ,330 ,149 ,86 ,49, 138, 85, 41, 2, 0, 0 ]
# 0 0 0 4 24 178 360 1456 10648 9631 2752 948 149 32 17 18 40 23 14 0 0 0 
bin_centers = [(bins[i] + bins[i+1])/2 for i in range(len(bins)-1)]
bin_widths = [bins[i+1] - bins[i] for i in range(len(bins)-1)]

total_frequency = sum(frequencies)
weighted_sum = sum(center * freq for center, freq in zip(bin_centers, frequencies))
estimated_mean = weighted_sum / total_frequency

# 创建图形和坐标轴
fig, ax = plt.subplots(figsize=(12, 7))

# 绘制条形图（模拟直方图）
bars = ax.bar(bin_centers, frequencies, width=bin_widths, 
              edgecolor='black', alpha=0.7, color='skyblue', align='center')

# 添加数值标签
for i, (center, freq, width) in enumerate(zip(bin_centers, frequencies, bin_widths)):
    ax.text(center, freq + 5, str(freq), ha='center', va='bottom', fontsize=10)
    # 在x轴下方显示区间范围
    # ax.text(center, -max(frequencies)*0.05, f"{bins[i]}-{bins[i+1]}", 
            # ha='center', va='top', fontsize=9, rotation=0)
ax.axvline(x=estimated_mean, color='red', linestyle='--', linewidth=2, 
           label=f'mean: {estimated_mean:.2f}')
# 设置标题和标签
ax.set_title('time dist ch 23', fontsize=16)
ax.set_xlabel('time [ns]', fontsize=12)
ax.set_ylabel('count', fontsize=12)

# 设置x轴刻度
ax.set_xticks(bin_centers)
ax.set_xticklabels([f"{bins[i]}-{bins[i+1]}" for i in range(len(bins)-1)], rotation=45)

# 调整y轴范围，留出空间显示标签
ax.set_ylim(0, max(frequencies) * 1.15)

# 添加网格
ax.grid(axis='y', alpha=0.3)

# 调整布局
# plt.tight_layout()

# 显示图形
plt.savefig('plothist.png')