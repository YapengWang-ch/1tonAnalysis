import numpy as np
import sys
import matplotlib.pyplot as plt

def read_data(filename):
    """读取数据文件，返回通道号、平均值、平均误差和条目数"""
    channels = []
    means = []
    # mean_errs = []
    # entries = []
    badchannels=[2,5,10,11,18,26,29,34,38,40,42,46,50,51,53,54,58,17,41,37]
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
                    # mean_errs.append(float(data[2]))
                    # entries.append(entry)
            except ValueError:
                continue
    
    return np.array(channels), np.array(means)

def compare_files(file1, file2):
    """比较两个文件的数据并绘制差异图"""
    # 读取数据
    ch1, mean1 = read_data(file1)
    ch2, mean2 = read_data(file2)
    
    # 找出两个文件共有的通道
    common_ch = np.intersect1d(ch1, ch2)
    
    # 提取共有通道的数据
    idx1 = np.where(np.isin(ch1, common_ch))[0]
    idx2 = np.where(np.isin(ch2, common_ch))[0]
    
    mean1_common = mean1[idx1]
    mean2_common = mean2[idx2]
    
    for i in range(len(common_ch)):
        print(f"Channel {common_ch[i]}: File1 Mean = {mean1_common[i]:.4f}, File2 Mean = {mean2_common[i]:.4f}, Difference = {mean1_common[i]-mean2_common[i]:.4f}")
 
if __name__ == "__main__":
    # if len(sys.argv) != 3:
    #     print("Usage: python compare_channels.py <file1.txt> <file2.txt>")
    #     sys.exit(1)
    
    # file1 = sys.argv[1]
    # file2 = sys.argv[2]
    file1="output/WaterCali200/run00043845_TCali.txt"
    file2="output/WaterCali223/run00043845_TCali.txt"
    compare_files(file1, file2)