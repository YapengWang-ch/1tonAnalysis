import os
import re
import glob
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime
import matplotlib.dates as mdates
from matplotlib.cm import ScalarMappable
from matplotlib.colors import Normalize

# 需要排除的通道列表（用于计算均值）
excluded_channels = [2,6,7,9,58,1,8,17,25,33,41,49,57]
badchannels = [26,29,38,54]
board1=[1,8,17,25,33,41,49,57]
board2=[2,6,7,9,58]
borad3=[]
# excluded_channels = [2, 10, 18, 26, 34, 42, 50, 58]
# badchannels = [5,11,18,26,29,40,51,53,57,58,17,41,37]
# 初始化数据结构：60个通道的列表
# 每个元素存储 (时间, 原始Mean值, 减去均值后的值, run号, Mean误差, Entries)
channels = [[] for _ in range(60)]

# 查找所有数据文件
data_files = glob.glob('./output_phaseII/WaterCali2w/*.txt')

if not data_files:
    print("未找到数据文件(*.txt)")
    exit()

for file_path in data_files:
    try:
        with open(file_path, 'r') as f:
            lines = f.readlines()
        if len(lines) < 3:
            continue
        run_match = re.search(r'run(\d+)', lines[0])
        if not run_match:
            continue
        run_id = run_match.group(1)
        time_match = re.search(r'beginning time:(\d+)', lines[0])
        if not time_match:
            continue
        timestamp = int(time_match.group(1))
        dt = datetime.fromtimestamp(timestamp)
        effkey = True    
        file_means = []
        for line in lines[2:62]:
            parts = line.split()
            if len(parts) < 6:
                continue
            try:
                channel = int(parts[0])
                if channel in badchannels:
                    continue
                entries = int(parts[5])
                # if float(parts[1]) < -7 and float(parts[5]) > 10:
                #     effkey = False
                #     break
                if entries <= 30000 and entries > 0:
                    effkey = False
                    break
                if entries > 30000 and channel not in excluded_channels:
                    mean_val = float(parts[1])
                    file_means.append(mean_val)
            except (ValueError, IndexError):
                continue
        if not effkey:
            print(f"文件 {file_path} 统计量较小，跳过处理。")
            continue
        if not file_means:
            continue
        file_mean = np.mean(file_means)
        for line in lines[2:62]:
            parts = line.split()
            if len(parts) < 6:
                continue
            try:
                channel = int(parts[0])
                if channel in badchannels:
                    continue
                entries = int(parts[5])
                if entries > 10000:
                    mean_val = float(parts[1])
                    mean_error = float(parts[2])
                    adjusted_mean = mean_val - file_mean
                    # 存储新增的 Mean Error 和 Entries
                    channels[channel].append((dt, mean_val, adjusted_mean, run_id, mean_error, entries))
            except (ValueError, IndexError):
                continue
    except Exception as e:
        print(f"处理文件 {file_path} 时出错: {str(e)}")

# 创建输出目录
output_dir = "Water_phaseII2w_plots"
os.makedirs(output_dir, exist_ok=True)


# 定义8个余数分组
remainder_groups = {r: [] for r in range(8)}

# 将通道分配到各自的余数组
for channel in range(60):
    if channel in badchannels:
        continue
    if not channels[channel]:
        continue
    
    remainder = channel % 8
    remainder_groups[remainder].append(channel)

# 创建颜色映射
cmap = plt.get_cmap('tab10')
markers = ['o', 's', '^', 'v', '<', '>', 'p', '*', 'D', 'X']

# 为每个余数组创建图表
for remainder in range(8):
    group_channels = remainder_groups[remainder]
    if not group_channels:
        continue
        
    # 创建新图表
    plt.figure(figsize=(12, 8))
    
    # 设置图表标题
    plt.title(f'Adjusted Mean Variation for Channels ≡ {remainder} mod 8')
    plt.xlabel('Time')
    plt.ylabel('Adjusted Mean (ns)')
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # 设置日期格式
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
    plt.gcf().autofmt_xdate()
    
    # 绘制该组中每个通道的数据
    for i, channel in enumerate(group_channels):
        data = sorted(channels[channel], key=lambda x: x[0])
        times = [d[0] for d in data]
        adjusted_means = [d[2] for d in data]
        run_ids = [d[3] for d in data]
        
        # 为每个通道选择颜色和标记
        color = cmap(i % 10)  # 使用tab10颜色映射
        marker = markers[i % len(markers)]  # 循环使用标记
        
        # 绘制数据
        plt.plot(times, adjusted_means, 
                 marker=marker, linestyle='-', 
                 markersize=6, 
                 color=color, 
                 label=f'Channel {channel}',
                 alpha=0.8)
        
        # 标注小于-10的数据点
        for t, adj_mean, run_id in zip(times, adjusted_means, run_ids):
            if adj_mean < -10:
                plt.annotate(run_id, 
                            (t, adj_mean),
                            textcoords="offset points",
                            xytext=(0, 10),
                            ha='center',
                            fontsize=8,
                            alpha=0.7,
                            color='red')
    
    # 添加图例
    plt.legend(loc='best', ncol=2)
    
    # 保存图表
    plt.tight_layout()
    plt.savefig(f"{output_dir}/LS_mod8_group_{remainder}.pdf")
    plt.close()
    print(f"已保存余数 {remainder} 的图表 ({len(group_channels)} 个通道)")

if True:
    # 创建新图表
    plt.figure(figsize=(12, 8))
    
    # 设置图表标题
    plt.title(f'Adjusted Mean Variation for Channels ≡ {remainder} mod 8')
    plt.xlabel('Time')
    plt.ylabel('Adjusted Mean (ns)')
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # 设置日期格式
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
    plt.gcf().autofmt_xdate()
    
    # 绘制该组中每个通道的数据
    for i, channel in enumerate(board1):
        data = sorted(channels[channel], key=lambda x: x[0])
        times = [d[0] for d in data]
        adjusted_means = [d[2] for d in data]
        run_ids = [d[3] for d in data]
        
        # 为每个通道选择颜色和标记
        color = cmap(i % 10)  # 使用tab10颜色映射
        marker = markers[i % len(markers)]  # 循环使用标记
        
        # 绘制数据
        plt.plot(times, adjusted_means, 
                 marker=marker, linestyle='-', 
                 markersize=6, 
                 color=color, 
                 label=f'Channel {channel}',
                 alpha=0.8)
        
        # 标注小于-10的数据点
        for t, adj_mean, run_id in zip(times, adjusted_means, run_ids):
            if adj_mean < -10:
                plt.annotate(run_id, 
                            (t, adj_mean),
                            textcoords="offset points",
                            xytext=(0, 10),
                            ha='center',
                            fontsize=8,
                            alpha=0.7,
                            color='red')
    
    # 添加图例
    plt.legend(loc='best', ncol=2)
    
    # 保存图表
    plt.tight_layout()
    plt.savefig(f"{output_dir}/board1.pdf")
    plt.close()
    print(f"已保存board1的图表 ({len(board1)} 个通道)")

# plot for not excluded channels
for channel in range(60):
    if channel in excluded_channels or channel in badchannels:
        continue
    if not channels[channel]:
        continue
    
    plt.figure(figsize=(10, 6))
    data = sorted(channels[channel], key=lambda x: x[0])
    times = [d[0] for d in data]
    adjusted_means = [d[2] for d in data]
    run_ids = [d[3] for d in data]
    
    plt.plot(times, adjusted_means, 'o-', markersize=5, color='blue', alpha=0.7)
    
    # 标注小于-10的数据点
    for t, adj_mean, run_id in zip(times, adjusted_means, run_ids):
        if adj_mean < -10:
            plt.annotate(run_id, 
                        (t, adj_mean),
                        textcoords="offset points",
                        xytext=(0, 10),
                        ha='center',
                        fontsize=8,
                        alpha=0.7,
                        color='red')
    
    plt.title(f'Channel {channel} - Adjusted Mean Value Variation')
    plt.xlabel('Time')
    plt.ylabel('Adjusted Mean (ns)')
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # 设置日期格式
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
    plt.gcf().autofmt_xdate()
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/LS_channel_{channel}.pdf")
    plt.close()
    print(f"已保存通道 {channel} 的图表")

# plot for excluded channels
for channel in excluded_channels:
    if channel in badchannels:
        continue
    if not channels[channel]:
        continue
    
    plt.figure(figsize=(10, 6))
    data = sorted(channels[channel], key=lambda x: x[0])
    times = [d[0] for d in data]
    adjusted_means = [d[2] for d in data]
    run_ids = [d[3] for d in data]
    
    plt.plot(times, adjusted_means, 'o-', markersize=5, color='orange', alpha=0.7)
    
    # 标注小于-10的数据点
    for t, adj_mean, run_id in zip(times, adjusted_means, run_ids):
        if adj_mean < -10:
            plt.annotate(run_id, 
                        (t, adj_mean),
                        textcoords="offset points",
                        xytext=(0, 10),
                        ha='center',
                        fontsize=8,
                        alpha=0.7,
                        color='red')
    
    plt.title(f'Channel {channel} (Excluded) - Adjusted Mean Value Variation')
    plt.xlabel('Time')
    plt.ylabel('Adjusted Mean (ns)')
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # 设置日期格式
    plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
    plt.gcf().autofmt_xdate()
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/LS_channel_{channel}_excluded.pdf")
    plt.close()
    print(f"已保存排除通道 {channel} 的图表")

# plot board in 1 fig



# 创建汇总图表（所有通道在同一图中）
plt.figure(figsize=(15, 10))
plotted_channels = 0

# 创建颜色映射
cmap = plt.get_cmap('viridis')
norm = Normalize(vmin=0, vmax=59)

# 绘制每个有效通道的数据
for channel in range(60):
    if channel in badchannels:
        continue
    data = channels[channel]
    if not data:
        continue
        
    # 按时间排序数据点
    data = sorted(data, key=lambda x: x[0])
    times = [d[0] for d in data]
    adjusted_means = [d[2] for d in data]
    
    # 为通道分配颜色
    color = cmap(norm(channel))
    
    # 绘制带标记的线条
    plt.plot(times, adjusted_means, 'o-', markersize=4, 
             color=color, label=f'Channel {channel}')
    plotted_channels += 1

# 图表装饰
plt.title(f'All Channels - Adjusted Mean Value Variation')
plt.xlabel('Time')
plt.ylabel('Adjusted Mean (ns)')
plt.grid(True, linestyle='--', alpha=0.7)

# 设置日期格式
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
plt.gcf().autofmt_xdate()

# 添加图例（如果通道数少）或颜色条
plt.legend(loc='best', ncol=2)


plt.tight_layout()
plt.savefig(f"{output_dir}/LS_all_channels.pdf")
print(f"已保存所有通道的汇总图表")

# 输出统计信息
print(f"处理了 {len(data_files)} 个文件")
print(f"创建了 {len([r for r in range(8) if remainder_groups[r]])} 个余数组图表")
print(f"排除通道: {excluded_channels}")
print(f"坏通道: {badchannels}")


# 新增部分：计算加权平均并写入文件
output_weighted_dir = "."
os.makedirs(output_weighted_dir, exist_ok=True)
output_file = os.path.join(output_weighted_dir, "weighted_averages.txt")

with open(output_file, 'w') as f_out:
    f_out.write("# Weighted averages of adjusted means after calibration\n")
    f_out.write("# Channel Weighted_Adjusted_Mean Weighted_Error Sum_Entries Number_of_Runs\n")
    for channel in range(60):
        if channel in badchannels:
            f_out.write(f"{channel}\t0\t0\t0\t0\n")
            continue
        data_points = channels[channel]
        if not data_points:
            continue
        adj_means = []
        mean_errors = []
        entries_list = []
        run_ids = set()
        for dp in data_points:
            adj_means.append(dp[2])
            mean_errors.append(dp[4])
            entries_list.append(dp[5])
            run_ids.add(dp[3])
        # 计算加权平均值和误差
        weights = [en for en in entries_list]
        sum_weights = sum(weights)
        if sum_weights == 0:
            continue
        weighted_avg = np.dot(adj_means, weights) / sum_weights
        weighted_error = np.sqrt(1.0 / sum_weights)
        sum_entries = sum(entries_list)
        num_runs = len(run_ids)
        f_out.write(f"{channel}\t{weighted_avg:.4f}\t{weighted_error:.4f}\t{sum_entries}\t{num_runs}\n")

print(f"保存加权平均结果到 {output_file}")


# 输出每个通道的数据点统计
for channel in range(60):
    if channels[channel]:
        print(f"通道 {channel}: {len(channels[channel])} 个数据点")
        low_points = [d for d in channels[channel] if d[2] < -10]
        if low_points:
            print(f"  通道 {channel} 中 {len(low_points)} 个点 < -10: ", end="")
            runs = sorted(set(d[3] for d in low_points))
            print(", ".join(runs))