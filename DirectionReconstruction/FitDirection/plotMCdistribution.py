import numpy as np
import matplotlib.pyplot as plt
import uproot as rt

file1="Output/ReconMC/AA_75/ReconMC_k10.root"
file2="Output/ReconMC/CT_75/ReconMC_k10.root"
file3="Output/ReconMC/WCT_75T_5/ReconMC_k10.root"

label = ["Truth","AA", "CT","WCT"]

def plot_mc_distribution(file1, file2, file3, label):
    """
    绘制不同重建方法的方向分布图
    """
    # 读取数据
    with rt.open(file1) as f1, rt.open(file2) as f2, rt.open(file3) as f3:
        data0 = f1["Direction"].arrays(["costheta_truth", "phi_truth"])
        data1 = f1["Direction"].arrays(["costheta_rec", "phi_rec"])
        data2 = f2["Direction"].arrays(["costheta_rec", "phi_rec"])
        data3 = f3["Direction"].arrays(["costheta_rec", "phi_rec"])


    # 提取数据
    cosalpha0, beta0 = data0["costheta_truth"], data0["phi_truth"]*180/3.1415926
    cosalpha1, beta1 = data1["costheta_rec"], data1["phi_rec"]*180/3.1415926
    cosalpha2, beta2 = data2["costheta_rec"], data2["phi_rec"]*180/3.1415926
    cosalpha3, beta3 = data3["costheta_rec"], data3["phi_rec"]*180/3.1415926

    # 绘制分布图
    plt.figure(figsize=(12, 6))
    
    plt.subplot(1, 2, 1)
    plt.hist(cosalpha0, bins=50, alpha=1, label=label[0], color='black', histtype='step', linewidth=1.5,range=(0,1))
    plt.hist(cosalpha1, bins=50, alpha=1, label=label[1], color='blue', histtype='step', linewidth=1.5,range=(0,1))
    plt.hist(cosalpha2, bins=50, alpha=1, label=label[2], color='orange', histtype='step', linewidth=1.5,range=(0,1))
    plt.hist(cosalpha3, bins=50, alpha=1, label=label[3], color='green', histtype='step', linewidth=1.5,range=(0,1))
    # plt.hist(cosalpha0, bins=50, alpha=0.5, label=label[0], color='black')
    # plt.hist(cosalpha1, bins=50, alpha=0.5, label=label[1], color='blue')
    # plt.hist(cosalpha2, bins=50, alpha=0.5, label=label[2], color='orange')
    # plt.hist(cosalpha3, bins=50, alpha=0.5, label=label[3], color='green')
    plt.title('cosTheta Distribution')
    plt.xlabel('cosTheta')
    plt.ylabel('Counts')
    plt.legend()

    plt.subplot(1, 2, 2)
    plt.hist(beta0, bins=60, alpha=1, label=label[0], color='black', histtype='step', linewidth=1.5,range=(-180,180))
    plt.hist(beta1, bins=60, alpha=1, label=label[1], color='blue', histtype='step', linewidth=1.5,range=(-180,180))
    plt.hist(beta2, bins=60, alpha=1, label=label[2], color='orange', histtype='step', linewidth=1.5,range=(-180,180))
    plt.hist(beta3, bins=60, alpha=1, label=label[3], color='green', histtype='step', linewidth=1.5,range=(-180,180))
    # plt.hist(beta0, bins=50, alpha=0.5, label=label[0], color='black')
    # plt.hist(beta1, bins=50, alpha=0.5, label=label[1], color='blue')
    # plt.hist(beta2, bins=50, alpha=0.5, label=label[2], color='orange')
    # plt.hist(beta3, bins=50, alpha=0.5, label=label[3], color='green')
    plt.title('Phi Distribution')
    plt.xlabel('Phi (degree)')
    plt.ylabel('Counts')
    plt.xticks(np.arange(-180,181,45))
    plt.xlim(-180, 180)
    # plt.grid(True, alpha=0.3)
    plt.legend()

    plt.tight_layout()
    plt.savefig("MC_Distribution_Comparison.pdf")
    print("绘图完成，保存为 'MC_Distribution_Comparison.pdf'")
    # plt.show()

if __name__ == "__main__":
    plot_mc_distribution(file1, file2, file3, label)
