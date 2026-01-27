import matplotlib.pyplot as plt
import numpy as np

# 读取数据
channels = []
gains = []

with open('./output/gain/gain_ser_7peak_Insert.txt', 'r') as file:
    lines = file.readlines()
    # 跳过第一行标题
    for line in lines[1:]:
        if line.strip():  # 跳过空行
            parts = line.split()
            channels.append(int(parts[0]))
            gains.append(float(parts[1]))

# 创建图表
plt.figure(figsize=(12, 6))
plt.plot(channels, gains, 'b-o', linewidth=2, markersize=4, label='Channel Gain')

# 设置图表属性
plt.xlabel('Channel Index')
plt.ylabel('Gain')
plt.title('Channel Gain Distribution')
plt.grid(True, alpha=0.3)
plt.legend()

# 设置x轴刻度
plt.xticks(np.arange(0, max(channels)+1, 5))

# 自动调整布局
plt.tight_layout()

# 显示图表
# plt.show()

# 可选：保存图表
plt.savefig('channel_gain_plot.png', dpi=300, bbox_inches='tight')