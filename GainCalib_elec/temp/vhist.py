import numpy as np
import matplotlib.pyplot as plt

# 提供的直方图数据
histogram_data = {
    14800: 0, 14801: 0, 14802: 0, 14803: 0, 14804: 0, 14805: 0, 14806: 0, 14807: 1, 14808: 1, 14809: 0,
    14810: 0, 14811: 0, 14812: 0, 14813: 0, 14814: 0, 14815: 0, 14816: 0, 14817: 0, 14818: 0, 14819: 0,
    14820: 0, 14821: 2, 14822: 0, 14823: 0, 14824: 0, 14825: 0, 14826: 0, 14827: 0, 14828: 0, 14829: 0,
    14830: 0, 14831: 0, 14832: 2, 14833: 0, 14834: 0, 14835: 1, 14836: 0, 14837: 1, 14838: 2, 14839: 1,
    14840: 1, 14841: 4, 14842: 10, 14843: 11, 14844: 14, 14845: 15, 14846: 20, 14847: 33, 14848: 33, 14849: 33,
    14850: 34, 14851: 13, 14852: 9, 14853: 11, 14854: 5, 14855: 8, 14856: 2, 14857: 0, 14858: 0, 14859: 0,
    14860: 1, 14861: 0, 14862: 1, 14863: 0, 14864: 0, 14865: 1, 14866: 0, 14867: 1, 14868: 1, 14869: 1,
    14870: 0, 14871: 0, 14872: 1, 14873: 2, 14874: 0, 14875: 2, 14876: 1, 14877: 0, 14878: 1, 14879: 2,
    14880: 2, 14881: 1, 14882: 0, 14883: 2, 14884: 1, 14885: 2, 14886: 2, 14887: 0, 14888: 2, 14889: 0,
    14890: 0, 14891: 1, 14892: 1, 14893: 0, 14894: 0, 14895: 0, 14896: 0, 14897: 1, 14898: 1, 14899: 0,
    14900: 2, 14901: 0, 14902: 0, 14903: 1, 14904: 0, 14905: 1, 14906: 4, 14907: 2, 14908: 4, 14909: 6,
    14910: 10, 14911: 14, 14912: 12, 14913: 11, 14914: 18, 14915: 14, 14916: 26, 14917: 23, 14918: 17, 14919: 17,
    14920: 19, 14921: 22, 14922: 13, 14923: 18, 14924: 20, 14925: 7, 14926: 12, 14927: 12, 14928: 20, 14929: 14,
    14930: 15, 14931: 16, 14932: 11, 14933: 15, 14934: 11, 14935: 4, 14936: 9, 14937: 6, 14938: 5, 14939: 5,
    14940: 0, 14941: 2, 14942: 0, 14943: 1, 14944: 1, 14945: 1, 14946: 0, 14947: 0, 14948: 0, 14949: 0
}

# 转换为数组
bins = list(histogram_data.keys())
values = list(histogram_data.values())

# 常数定义
RATIO = 1 / 2.51164  # 1/2.51164 ≈ 0.3981

def calculate_thresholds(bins, values):
    """计算上下阈值"""
    # 1. 找到峰值
    peak_idx = np.argmax(values)
    peak_value = values[peak_idx]
    peak_bin = bins[peak_idx]
    
    print(f"峰值位置: {peak_bin}, 峰值: {peak_value}")
    
    # 2. 从左侧向峰值逼近，找到值为峰值1/2.51164的位置
    threshold_value = peak_value * RATIO
    print(f"阈值水平: {threshold_value}")
    
    lower_bound_idx = -1
    for i in range(peak_idx, -1, -1):
        if values[i] <= threshold_value:
            lower_bound_idx = i
            break
    
    # 3. 从右侧向峰值逼近，找到值为峰值1/2.51164的位置
    upper_bound_idx = len(values)-1
    # for i in range(peak_idx, len(values)):
    #     if values[i] <= threshold_value:
    #         upper_bound_idx = i
    #         break
    while values[upper_bound_idx]<= threshold_value:
        upper_bound_idx-=1
    
    # 如果没有找到合适的边界，使用整个范围
    if lower_bound_idx == -1:
        lower_bound_idx = 0
    if upper_bound_idx == -1:
        upper_bound_idx = len(values) - 1
    
    lower_bound = bins[lower_bound_idx]
    upper_bound = bins[upper_bound_idx]
    
    print(f"下边界位置: {lower_bound} (索引: {lower_bound_idx})")
    print(f"上边界位置: {upper_bound} (索引: {upper_bound_idx})")
    
    # 4. 在边界范围内计算均值和方差
    range_values = values[lower_bound_idx:upper_bound_idx+1]
    range_bins = bins[lower_bound_idx:upper_bound_idx+1]
    
    # 计算加权平均值（考虑bin位置）
    weighted_sum = sum(v * b for v, b in zip(range_values, range_bins))
    total_count = sum(range_values)
    
    if total_count > 0:
        baseline = weighted_sum / total_count
    else:
        baseline = np.mean(range_bins)
    
    # 计算方差（考虑bin位置）
    if len(range_values) > 1:
        variance = sum(v * (b - baseline) ** 2 for v, b in zip(range_values, range_bins)) / total_count
        std_dev = np.sqrt(variance)
    else:
        variance = 0
        std_dev = 0
    
    print(f"基线（均值）: {baseline:.2f}")
    print(f"方差: {variance:.2f}")
    print(f"标准差: {std_dev:.2f}")
    
    return lower_bound, upper_bound, baseline, std_dev, peak_bin, peak_value

def plot_histogram(bins, values, lower_bound, upper_bound, baseline, std_dev, peak_bin, peak_value):
    """绘制直方图并标出阈值"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    # 绘制直方图
    ax.bar(bins, values, width=1, alpha=0.7, color='skyblue', edgecolor='black')
    
    # 标记峰值
    ax.scatter(peak_bin, peak_value, color='red', s=100, zorder=5, label=f'peak ({peak_bin}, {peak_value})')
    
    # 标记上下边界
    ax.axvline(x=lower_bound, color='green', linestyle='--', linewidth=2, 
               label=f'bottom bound: {lower_bound}')
    ax.axvline(x=upper_bound, color='green', linestyle='--', linewidth=2,
               label=f'above bound: {upper_bound}')
    
    # 标记基线
    ax.axvline(x=baseline, color='orange', linestyle='-', linewidth=2,
               label=f'baseline_raw: {baseline:.2f}')
    
    # 标记阈值区间（均值±标准差）
    ax.axvspan(baseline - std_dev, baseline + std_dev, alpha=0.2, color='yellow',
               label=f'baseline_mask: ±{std_dev:.2f}')
    
    # 标记阈值水平线
    threshold_level = peak_value * RATIO
    ax.axhline(y=threshold_level, color='purple', linestyle=':', linewidth=1.5,
               label=f'bound height: {threshold_level:.2f}')
    
    ax.set_xlabel('voltage')
    ax.set_ylabel('count')
    ax.set_title('voltage hist')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 设置x轴范围
    ax.set_xlim(min(bins) - 10, max(bins) + 10)
    
    plt.tight_layout()
    plt.savefig("temp.pdf")

# 执行计算和绘图
lower_bound, upper_bound, baseline, std_dev, peak_bin, peak_value = calculate_thresholds(bins, values)

print("\n" + "="*50)
print(f"计算结果总结:")
print(f"峰值位置: {peak_bin}")
print(f"下阈值边界: {lower_bound}")
print(f"上阈值边界: {upper_bound}")
print(f"初始基线: {baseline:.2f}")
print(f"初始阈值（标准差）: {std_dev:.2f}")
print("="*50)

# 绘制图形
plot_histogram(bins, values, lower_bound, upper_bound, baseline, std_dev, peak_bin, peak_value)

# 提供更详细的边界内数据分析
# lower_idx = bins.index(lower_bound)
# upper_idx = bins.index(u_bound)

print(f"\n边界内数据统计:")
# print(f"边界内数据点数量: {upper_idx - lower_idx + 1}")
# print(f"边界内总计数: {sum(values[lower_idx:upper_idx+1])}")
# print(f"边界内最小值: {min(values[lower_idx:upper_idx+1])}")
# print(f"边界内最大值: {max(values[lower_idx:upper_idx+1])}")