import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os

def plot_risetime_results(filename):
    # 读取数据文件
    if not os.path.exists(filename):
        print(f"错误: 文件 {filename} 不存在")
        return
    
    # 读取数据到DataFrame
    df = pd.read_csv(filename, sep='\t', comment='#', header=None, 
                     names=['Channel', 'Mean', 'Mean_Error', 'Sigma', 'Sigma_Error', 'Entries'])
    # 过滤掉数据不足的通道
    valid_data = df[df['Entries'] > 50]
    weights = 1 / valid_data['Sigma_Error']**2
    weighted_mean_sigma = (valid_data['Sigma'] * weights).sum() / weights.sum()

    # 创建画布和子图
    plt.figure(figsize=(14, 10))
    
    # 1. 均值图
    plt.subplot(2, 1, 1)
    plt.errorbar(valid_data['Channel'], valid_data['Mean']-180, 
                 yerr=valid_data['Mean_Error'], 
                 fmt='o-', color='b', ecolor='lightblue', 
                 capsize=5, label='mean')
    plt.title('TimeOffset', fontsize=14)
    plt.xlabel('Channel', fontsize=12)
    plt.ylabel('Risetime (ns)', fontsize=12)
    plt.ylim(-10, 10)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend()
    
    # 2. 标准差图
    plt.subplot(2, 1, 2)
    plt.errorbar(valid_data['Channel'], (valid_data['Sigma']-weighted_mean_sigma)/valid_data['Sigma_Error'], 
                #  yerr=valid_data['Sigma_Error'], 
                 fmt='s-', color='r', ecolor='lightcoral', 
                 capsize=5, label='normalized sigma')
    plt.title('sigma', fontsize=14)
    plt.xlabel('Channel', fontsize=12)
    plt.ylabel('(sigma-sigma_mean)/sigma_Error', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend()
    
    # 添加整体标题
    plt.suptitle('TimeOffset', fontsize=16)
    
    # 调整布局
    plt.tight_layout()
    plt.subplots_adjust(top=0.92)
    
    # 保存图像
    output_png = os.path.splitext(filename)[0] + '_plot.png'
    plt.savefig(output_png, dpi=300)
    print(f"图表已保存为 {output_png}")
    
    # 显示图表
    plt.show()
    
    # 返回数据框用于进一步分析
    return df

if __name__ == "__main__":
    # 输入文件路径
    input_file = "risetime_fit_results.txt"
    
    # 绘制结果
    data = plot_risetime_results(input_file)
    
    # 打印统计摘要
    print("\n数据统计摘要:")
    print(data.describe())
    
    # 显示前5行数据
    print("\n前5行数据:")
    print(data.head())