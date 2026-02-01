import ROOT
from ROOT import TFile, TCanvas, TLegend, kRed, kBlue, kGreen

# 配置参数（用户可修改部分）----------------------------------------------
file_config = [
    {
        "path": "Output/AngularResolution/k_Resolution_Test_AA.root",      # 第一个文件路径
        "hist_name": "Resolution",   
        "label": "AA(Guo)",      
        "color": kRed,            
        "line_style": 1,          
        "line_width": 2           
    },
    {
        "path": "Output/AngularResolution/k_Resolution_Test_CT.root",      # 第一个文件路径
        "hist_name": "Resolution",   
        "label": "CT(Zhang)",      
        "color": kBlue,            
        "line_style": 1,          
        "line_width": 2           
    },
    {
        "path": "Output/AngularResolution/k_Resolution_Test_WCT_t4.root",      # 第一个文件路径
        "hist_name": "Resolution",   
        "label": "WCT T_{s}= 4 ns",      
        "color": kGreen,            
        "line_style": 1,          
        "line_width": 2           
    },
    # {
    #     "path": "Output/AngularResolution/k_Resolution_AA_75_Terror5e-1.root",      # 第一个文件路径
    #     "hist_name": "Resolution",   
    #     "label": "AA(Guo) #sigma_{TCali}=0.5ns",      
    #     "color": kRed,            
    #     "line_style": 2,          
    #     "line_width": 2           
    # },
    # {
    #     "path": "Output/AngularResolution/k_Resolution_CT_75_Tsigma5e-1.root",      # 第一个文件路径
    #     "hist_name": "Resolution",   
    #     "label": "CT(Zhang) #sigma_{TCali}=0.5ns",      
    #     "color": kBlue,            
    #     "line_style": 2,          
    #     "line_width": 2           
    # },
    # {
    #     "path": "Output/AngularResolution/k_Resolution_WCT_75T_5.root",      # 第一个文件路径
    #     "hist_name": "Resolution",   
    #     "label": "WCT T_{s}= 5 ns #sigma_{TCali}=0.5ns",
    #     "color": kGreen,            
    #     "line_style": 2,          
    #     "line_width": 2           
    # },

]
output_image = "Resolution_3.pdf"  # 输出文件名（支持.png, .pdf等）#
# ------------------------------------------------------------------------
plot_title = ""  # 主标题
x_title = "k"                      # X轴标题
y_title = "Direction Resolution (degree)"                  # Y轴标题

def load_histogram(file_info):
    """从单个文件加载直方图"""
    file_path = file_info["path"]
    hist_name = file_info["hist_name"]
    
    # 打开文件
    root_file = TFile.Open(file_path, "READ")
    if not root_file or root_file.IsZombie():
        raise RuntimeError(f"错误: 无法打开文件 {file_path}!")
    
    # 获取直方图
    hist = root_file.Get(hist_name)
    if not hist:
        root_file.Close()
        raise RuntimeError(f"错误: 文件 {file_path} 中未找到直方图 {hist_name}!")
    
    if not isinstance(hist, ROOT.TH1D) and not isinstance(hist, ROOT.TH1F):
        root_file.Close()
        raise RuntimeError(f"错误: {hist_name} 不是 TH1F 类型!")
    
    # 样式设置
    hist.SetLineColor(file_info["color"])
    hist.SetLineStyle(file_info["line_style"])
    hist.SetLineWidth(file_info["line_width"])
    
    # 保持文件打开（直方图依赖文件句柄）
    file_info["file_handle"] = root_file
    return hist

# 主程序 --------------------------------------------------------------
if __name__ == "__main__":
    hists = []
    file_handles = []
    
    # 加载所有直方图
    for config in file_config:
        try:
            hist = load_histogram(config)
            hists.append( (hist, config["label"]) )
            file_handles.append(config["file_handle"])
            print(f"成功加载: {config['path']} -> {config['hist_name']}")
        except Exception as e:
            print(f"加载失败: {str(e)}")
            continue
    
    if len(hists) < 1:
        raise RuntimeError("没有可用的直方图!")
    
    # 创建画布并绘制
    canvas = TCanvas("canvas", "Multi-file Histogram", 1200, 800)
    # canvas.SetGrid(True, True)
    # canvas.SetLogy()


    # 绘制第一个直方图
    first_hist, first_label = hists[0]
    first_hist.SetTitle(plot_title)            # 设置主标题
    first_hist.GetXaxis().SetTitle(x_title)    # 设置X轴标题
    first_hist.GetYaxis().SetTitle(y_title)    # 设置Y轴标题
    first_hist.Draw("HIST")
    
    # 自动调整Y轴范围（查找最大值）
    max_val = max([h.GetMaximum() for h, _ in hists])
    first_hist.SetMaximum(25)
    first_hist.SetMinimum(0)
    # 叠加其他直方图
    ROOT.gStyle.SetOptStat(0)
    for hist, label in hists[1:]:
        hist.Draw("HIST SAME")
    
    # 添加图例
    legend = TLegend(0.55, 0.5, 0.8, 0.7)
    # legend.SetHeader("Data Sources", "C")  # 标题居中
    legend.SetTextSize(0.035)
    
    for hist, label in hists:
        legend.AddEntry(hist, label, "l")
    
    legend.Draw()
    
    # 保存图像
    canvas.SaveAs(output_image)
    print(f"已保存图像: {output_image}")
    
    # 关闭所有文件
    for fh in file_handles:
        fh.Close()
    
    # # 保持窗口打开（可选）
    # input("按回车键退出...")