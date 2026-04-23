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
badchannels = [26,29,38,54]
boardID=[0,1,2,3,4,6,2,2,1,2,0,5,4,5,6,7,0,1,5,3,4,5,6,7,0,1,3,3,4,2,6,7,0,1,5,3,4,5,6,7,3,1,4,3,4,5,6,7,0,1,7,5,6,0,4,7,0,1,2,3]
if len(boardID)!=60:
    print("boardID长度错误")
    exit()

channels = [[] for _ in range(60)]

# 查找所有数据文件
data_files = glob.glob('./output_phaseII/WaterCali2w/*.txt')
boardfix_file="data/boardfix.txt"
if os.path.exists(boardfix_file):
    os.remove(boardfix_file)
os.makedirs(os.path.dirname(boardfix_file), exist_ok=True)
with open(boardfix_file, 'w') as file:
    pass  # 初始化文件

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

        # 先收集 0 号板的通道均值（用于基准）
        ref_means = []
        channel_info = {}  # channel -> (mean, mean_error, entries)
        for line in lines[2:62]:
            parts = line.split()
            if len(parts) < 6:
                continue
            try:
                channel = int(parts[0])
                mean_val = float(parts[1])
                mean_error = float(parts[2])
                entries = int(parts[5])
            except (ValueError, IndexError):
                continue
            channel_info[channel] = (mean_val, mean_error, entries)
            if channel in badchannels:
                continue
            if entries <= 10000 and entries > 0:
                effkey = False
                break
            if entries > 10000 and boardID[channel] == 0:
                ref_means.append(mean_val)

        if not effkey:
            print(f"文件 {file_path} 统计量较小，跳过处理。")
            continue
        if not ref_means:
            print(f"文件 {file_path} 没有足够的 0 号板数据，跳过。")
            continue

        file_mean = float(np.mean(ref_means))  # 0号板均值基准

        # 为每个板收集差值
        board_diffs = {b: [] for b in range(8)}
        for ch, (mean_val, mean_err, entries) in channel_info.items():
            if ch in badchannels:
                continue
            if entries > 10000:
                diff = mean_val - file_mean
                b = boardID[ch]
                board_diffs[b].append(diff)

        # 计算每板的漂移（以16为单位四舍五入）
        boardfixList = [0]*8
        for b in range(8):
            if b == 0:
                boardfixList[b] = 0
                continue
            diffs = board_diffs.get(b, [])
            if not diffs:
                boardfixList[b] = 0
                continue
            mean_diff = float(np.mean(diffs))
            # 以16为步长四舍五入到最近的整数倍
            shift_units = int(round(mean_diff / 16.0))
            boardfixList[b] = int(shift_units * 16)

        # 写入文件
        with open(boardfix_file, 'a', encoding='utf-8') as file:
            file.write(f"{run_id} {' '.join(map(str, boardfixList))}\n")

    except Exception as e:
        print(f"处理文件 {file_path} 时出错: {str(e)}")

# 仅输出通道统计（保留原程序最后一段的简单统计）
for channel in range(60):
    if channels[channel]:
        print(f"通道 {channel}: {len(channels[channel])} 个数据点")
        low_points = [d for d in channels[channel] if d[2] < -10]
        if low_points:
            runs = sorted(set(d[3] for d in low_points))
            print(f"  通道 {channel} 中 {len(low_points)} 个点 < -10: {', '.join(runs)}")