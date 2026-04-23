import numpy as np
import sys
import matplotlib.pyplot as plt

def read_data(filename):
    """读取数据文件，返回通道号、平均值、平均误差和条目数"""
    channels = []
    means = []
    mean_errs = []
    entries = []
    badchannels=[2,5,10,11,18,26,29,34,38,40,42,46,50,51,53,54,58,17,41,37,59,6,13,23,24]
    with open(filename, 'r') as f:
        for line in f:
            # 跳过注释行和分隔线
            if line.startswith('#') or line.startswith('-'):
                continue
            data = line.split()
            # 确保有足够的数据列
            if len(data) < 6:
                continue
            # 转换数据类型
            try:
                entry = float(data[5])
                if entry > 0 and int(data[0]) not in badchannels:  # 只处理条目数大于0的通道
                    channels.append(int(data[0]))
                    means.append(float(data[1]))
                    mean_errs.append(float(data[2]))
                    entries.append(entry)
            except ValueError:
                continue
    
    return np.array(channels), np.array(means), np.array(mean_errs), np.array(entries)

def compare_files(file1, file2):
    """比较两个文件的数据并绘制差异图"""
    # 读取数据
    ch1, mean1, err1, ent1 = read_data(file1)
    ch2, mean2, err2, ent2 = read_data(file2)
    
    # 找出两个文件共有的通道
    common_ch = np.intersect1d(ch1, ch2)
    
    # 提取共有通道的数据
    idx1 = np.where(np.isin(ch1, common_ch))[0]
    idx2 = np.where(np.isin(ch2, common_ch))[0]
    
    mean1_common = mean1[idx1]
    # err1_common = err1[idx1]
    mean2_common = mean2[idx2]
    # err2_common = err2[idx2]
    
    # 归一化处理
    norm1 = np.mean(mean1_common)
    norm2 = np.mean(mean2_common)
    
    norm_mean1 = mean1_common - norm1
    # norm_err1 = err1_common 
    norm_mean2 = mean2_common - norm2
    # norm_err2 = err2_common
    
    error=[]
    for id in idx1:
        error.append(norm_mean1[id]-norm_mean2[id])

    plt.figure(figsize=(12, 6))
    
    # 添加y=0的横线
    plt.axhline(y=0, color='r', linestyle='-', linewidth=1)
    
    # 绘制所有通道的差异
    plt.errorbar(common_ch, error, fmt='s', color='b', 
                 ecolor='lightgray', elinewidth=2, capsize=4, label='C - W',)
    # plt.errorbar(common_ch, norm_mean2, yerr=norm_err2, fmt='o', color='r', 
                #  ecolor='lightgray', elinewidth=2, capsize=4, label='W')
    plt.plot(common_ch, error, color='b', linestyle='-', alpha=0.6)
    # plt.plot(common_ch, norm_mean2, color='r', linestyle='-', alpha=0.6)
    plt.ylim(-2.5,2.5)
    # 标记显著差异点
    # if len(sig_idx) > 0:
    #     plt.errorbar(common_ch[sig_idx], diff[sig_idx], yerr=diff_err[sig_idx], 
    #                  fmt='s', color='r', ecolor='darkred', elinewidth=2, capsize=5,
    #                  label='Significant Difference (>3σ)')
    
    plt.xlabel('Channel')
    plt.ylabel('TimeOffset LS (C-W)')
    plt.title('Comparison of TimeOffset')
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # 设置x轴为整数刻度
    plt.xticks(np.arange(min(common_ch), max(common_ch)+1, 5))
    
    # 添加额外的网格线在y=0处
    plt.axhline(y=0, color='r', linestyle='-', linewidth=1, zorder=0)
    
    plt.tight_layout()
    plt.savefig('error_display.png')
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_channels.py <file1.txt> <file2.txt>")
        sys.exit(1)
    
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    compare_files(file1, file2)