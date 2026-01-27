import matplotlib
matplotlib.use('Agg')  # 在导入pyplot之前设置后端
import matplotlib.pyplot as plt
import numpy as np
import os
from math import sqrt
import argparse
import sys
import re


def AitoffProjection(lon, lat, lon_0=0):
    """
    Aitoff projection.
    """
    lon = np.asarray(lon)
    lat = np.asarray(lat)
    delta_lon = lon - lon_0
    alpha = np.arccos(np.cos(lat) * np.cos(delta_lon / 2))
    # 使用 sinc，避免中心空白
    sinc_alpha = np.sinc(alpha / np.pi)
    x = 2 * np.cos(lat) * np.sin(delta_lon / 2) / (sinc_alpha + 1e-10)
    y = np.sin(lat) / (sinc_alpha + 1e-10)
    return x, y

def CylindricalEqualAreaProjection(lon, lat, lon_0=0):
    """
    Cylindrical Equal-Area projection.
    """
    # phi_1 = 0  # Winkel Tripel标准纬线
    phi_1 = np.arccos(2 / np.pi)
    x = (lon - lon_0) * np.cos(phi_1)
    y = np.sin(lat) / np.cos(phi_1)
    return x, y

def WinkelTripelProjection(lon, lat, lon_0=0):
    """
    Winkel Tripel projection.
    """
    x_aitoff, y_aitoff = AitoffProjection(lon, lat, lon_0)
    x_cyl, y_cyl = CylindricalEqualAreaProjection(lon, lat, lon_0)
    x = (x_aitoff + x_cyl) / 2
    y = (y_aitoff + y_cyl) / 2
    return x, y

def WinkelProjectionFromPosition(x, y, z):
    """
    Convert Cartesian coordinates (x, y, z) to Winkel Tripel projection coordinates.
    """
    lon = np.arctan2(y, x)  # 经度
    lat = np.arcsin(z / np.sqrt(x**2 + y**2 + z**2))  # 纬度
    return WinkelTripelProjection(lon, lat)

def NorthProjectionFromPosition(x,y,z):
    """
    Convert Cartesian coordinates (x, y, z) to North Stereographic projection
    for z > 0
    """ 
    R=np.sqrt(x**2+y**2+z**2)
    X_ster=x/(R+z)
    Y_ster=y/(R+z)
    return X_ster, Y_ster

def SouthProjectionFromPosition(x,y,z):
    """
    Convert Cartesian coordinates (x, y, z) to South Stereographic projection
    for z < 0
    """ 
    R=np.sqrt(x**2+y**2+z**2)
    X_ster=x/(R-z)
    Y_ster=y/(R-z)
    return X_ster, Y_ster

def plot_withInfo(title, info, pmt_data, output_dir):
    """
    Plots the PMT positions using Winkel Tripel projection and stereographic projections for poles.
    """
    # 首先收集所有非零时间数据，确定时间范围
    all_non_zero_times = []
    for data in pmt_data:
        id, x, y, z, charge, time = data
        if time != 0 and charge > 0:
            all_non_zero_times.append(time)
    
    # 确定时间范围用于颜色映射
    if all_non_zero_times:
        vmin = min(all_non_zero_times)
        vmax = max(all_non_zero_times)
    else:
        vmin = 0
        vmax = 1
    
    # Create a figure with subplots: main Winkel projection on top, polar projections below
    fig = plt.figure(figsize=(12, 10))
    
    # Create grid: 2 rows, 1 column for main plot, then 1 row with 2 columns for polar projections
    gs = plt.GridSpec(2, 3, height_ratios=[3, 2],width_ratios=[1,1,0.5])
    
    # Main Winkel Tripel projection - spans both columns of first row
    ax_main = plt.subplot(gs[0, :])
    
    # North pole stereographic projection - first column of second row
    ax_north = plt.subplot(gs[1, 0])
    
    # South pole stereographic projection - second column of second row  
    ax_south = plt.subplot(gs[1, 1])
    
    ax_info = plt.subplot(gs[1,2])
    ax_info.text(0.5, 0.5, 
             info,
             horizontalalignment='center',
             verticalalignment='center',
             transform=ax_info.transAxes,
             fontsize=10,
             bbox=dict(boxstyle="round,pad=0.3", facecolor="lightblue", alpha=0.7))

    # 隐藏坐标轴
    ax_info.set_xticks([])
    ax_info.set_yticks([])
    ax_info.spines['top'].set_visible(False)
    ax_info.spines['right'].set_visible(False)
    ax_info.spines['bottom'].set_visible(False)
    ax_info.spines['left'].set_visible(False)

    # Plot main Winkel Tripel projection
    scatter_main = plot_winkel_projection(ax_main, pmt_data, title, vmin, vmax)
    
    # Plot north pole stereographic projection
    plot_stereographic_projection(ax_north, pmt_data, 'north', 'Top of Detector', vmin, vmax)
    
    # Plot south pole stereographic projection
    plot_stereographic_projection(ax_south, pmt_data, 'south', 'Bottom of Detector', vmin, vmax)
    

    # 为主图添加颜色条
    if scatter_main is not None:
        cbar = plt.colorbar(scatter_main, ax=ax_main, shrink=0.8, aspect=15)
        cbar.set_label('Time [ns]')
    
    plt.tight_layout()
    
    # Save plot
    plt.savefig(output_dir)
    print("fig saved to: ", output_dir)
    plt.close()

def plotEvent(title, pmt_data, output_dir):
    """
    Plots the PMT positions using Winkel Tripel projection and stereographic projections for poles.
    """
    # 首先收集所有非零时间数据，确定时间范围
    all_non_zero_times = []
    for data in pmt_data:
        id, x, y, z, charge, time = data
        if time != 0 and charge > 0:
            all_non_zero_times.append(time)
    
    # 确定时间范围用于颜色映射
    if all_non_zero_times:
        vmin = min(all_non_zero_times)
        vmax = max(all_non_zero_times)
    else:
        vmin = 0
        vmax = 1
    
    # Create a figure with subplots: main Winkel projection on top, polar projections below
    fig = plt.figure(figsize=(12, 10))
    
    # Create grid: 2 rows, 1 column for main plot, then 1 row with 2 columns for polar projections
    gs = plt.GridSpec(2, 2, height_ratios=[3, 2])
    
    # Main Winkel Tripel projection - spans both columns of first row
    ax_main = plt.subplot(gs[0, :])
    
    # North pole stereographic projection - first column of second row
    ax_north = plt.subplot(gs[1, 0])
    
    # South pole stereographic projection - second column of second row  
    ax_south = plt.subplot(gs[1, 1])

    # Plot main Winkel Tripel projection
    scatter_main = plot_winkel_projection(ax_main, pmt_data, title, vmin, vmax)
    
    # Plot north pole stereographic projection
    plot_stereographic_projection(ax_north, pmt_data, 'north', 'Top of Detector', vmin, vmax)
    
    # Plot south pole stereographic projection
    plot_stereographic_projection(ax_south, pmt_data, 'south', 'Bottom of Detector', vmin, vmax)
    

    # 为主图添加颜色条
    if scatter_main is not None:
        cbar = plt.colorbar(scatter_main, ax=ax_main, shrink=0.8, aspect=15)
        cbar.set_label('Time [ns]')
    
    plt.tight_layout()
    
    # Save plot
    plt.savefig(output_dir)
    print("fig saved to: ", output_dir)
    plt.close()


def plot_winkel_projection(ax, pmt_data, title, vmin, vmax):
    """
    Plot Winkel Tripel projection on the given axes.
    """
    # 绘制经纬网格
    # 经线：每45度一条
    for lon in np.arange(-180, 181, 45):
        lon_rad = np.radians(lon)
        lat_points = np.radians(np.linspace(-90, 90, 100))
        lon_points = np.full_like(lat_points, lon_rad)
        grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
        ax.plot(grid_x, grid_y, 'gray', linestyle='--', alpha=0.5, linewidth=0.5)
        
    # 纬线：每30度一条
    for lat in np.arange(-90, 91, 30):
        lat_rad = np.radians(lat)
        lon_points = np.radians(np.linspace(-180, 180, 200))
        lat_points = np.full_like(lon_points, lat_rad)
        grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
        ax.plot(grid_x, grid_y, 'gray', linestyle='--', alpha=0.5, linewidth=0.5)
        
    # 绘制边界
    lon_prime = np.radians(180)
    lat_points = np.radians(np.linspace(-90, 90, 100))
    lon_points = np.full_like(lat_points, lon_prime)
    grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
    ax.plot(grid_x, grid_y, 'black', linestyle='-', alpha=0.7, linewidth=1)

    lon_prime = np.radians(-180)
    lat_points = np.radians(np.linspace(-90, 90, 100))
    lon_points = np.full_like(lat_points, lon_prime)
    grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
    ax.plot(grid_x, grid_y, 'black', linestyle='-', alpha=0.7, linewidth=1)

    lat_equator = np.radians(90)
    lon_points = np.radians(np.linspace(-180, 180, 200))
    lat_points = np.full_like(lon_points, lat_equator)
    grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
    ax.plot(grid_x, grid_y, 'black', linestyle='-', alpha=0.7, linewidth=1)

    lat_equator = np.radians(-90)
    lon_points = np.radians(np.linspace(-180, 180, 200))
    lat_points = np.full_like(lon_points, lat_equator)
    grid_x, grid_y = WinkelTripelProjection(lon_points, lat_points)
    ax.plot(grid_x, grid_y, 'black', linestyle='-', alpha=0.7, linewidth=1)

    # Separate data into time=0 and time≠0 groups
    non_zero_x, non_zero_y, non_zero_charges, non_zero_times = [], [], [], []
    zero_x, zero_y, zero_charges = [], [], []
    pmtids, projected_x, projected_y = [], [], []
    
    for data in pmt_data:
        id, x, y, z, charge, time = data
        [x_Winkel, y_Winkel] = WinkelProjectionFromPosition(x, y, z)
        
        # Separate data based on time
        pmtids.append(id)
        projected_x.append(x_Winkel)
        projected_y.append(y_Winkel)
        if (time == 0 or charge <= 0):
            zero_x.append(x_Winkel)
            zero_y.append(y_Winkel)
            zero_charges.append(0)
        else:
            non_zero_x.append(x_Winkel)
            non_zero_y.append(y_Winkel)
            non_zero_charges.append(charge)
            non_zero_times.append(time)

    scatter_obj = None
    
    # Plot non-zero time points with colormap
    if non_zero_x:
        sizes_non_zero = [sqrt(charge)*8 for charge in non_zero_charges]
        scatter_obj = ax.scatter(
            non_zero_x, non_zero_y,
            c=non_zero_times,
            s=sizes_non_zero,
            cmap='RdYlGn_r',
            vmin=vmin,
            vmax=vmax,
            marker='o',
            edgecolors='black',
            linewidths=0.5
        )

    # Plot zero time points in gray
    if zero_x:
        sizes_zero = [sqrt(50)*8 for charge in zero_charges]
        ax.scatter(
            zero_x, zero_y,
            c='#D0D0D0',
            s=sizes_zero,
            marker='o',
            edgecolors='black',
            linewidths=0.5,
            label='Time = 0'
        )

    ax.set_title(title)

    # 标注每个PMT的编号
    for i, (x, y, id_val) in enumerate(zip(projected_x, projected_y, pmtids)):
        ax.annotate(str(id_val), (x, y), xytext=(5, 5), textcoords='offset points', 
                    fontsize=8, alpha=0.8)

    # 保持纵横比
    ax.set_aspect('equal')

    # 设置坐标轴范围，确保所有内容都可见
    all_x = projected_x + list(grid_x) if 'grid_x' in locals() else projected_x
    all_y = projected_y + list(grid_y) if 'grid_y' in locals() else projected_y

    if all_x and all_y:
        x_min, x_max = min(all_x), max(all_x)
        y_min, y_max = min(all_y), max(all_y)
        x_margin = (x_max - x_min) * 0.1
        y_margin = (y_max - y_min) * 0.1
        ax.set_xlim(x_min - x_margin, x_max + x_margin)
        ax.set_ylim(y_min - y_margin, y_max + y_margin)

    # 去除坐标轴和网格
    ax.set_axis_off()
    
    return scatter_obj

def plot_stereographic_projection(ax, pmt_data, pole, title, vmin, vmax):
    """
    Plot stereographic projection for north or south pole.
    
    Parameters:
    ax: matplotlib axes
    pmt_data: list of PMT data
    pole: 'north' or 'south'
    title: subplot title
    vmin: minimum value for color mapping
    vmax: maximum value for color mapping
    """
    # Filter data for the specified pole
    pole_data = []
    for data in pmt_data:
        id, x, y, z, charge, time = data
        if pole == 'north' and z > 0:
            pole_data.append(data)
        elif pole == 'south' and z < 0:
            pole_data.append(data)
    
    if not pole_data:
        ax.text(0.5, 0.5, f'No PMTs in {title}', 
                ha='center', va='center', transform=ax.transAxes)
        ax.set_title(title)
        ax.set_aspect('equal')
        # 即使没有数据也保留坐标轴
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        return
    
    # Separate data into time=0 and time≠0 groups
    non_zero_x, non_zero_y, non_zero_charges, non_zero_times = [], [], [], []
    zero_x, zero_y, zero_charges = [], [], []
    
    for data in pole_data:
        id, x, y, z, charge, time = data
        
        if pole == 'north':
            x_proj, y_proj = NorthProjectionFromPosition(x, y, z)
        else:  # south
            x_proj, y_proj = SouthProjectionFromPosition(x, y, z)
        
        if (time == 0 or charge <= 0):
            zero_x.append(x_proj)
            zero_y.append(y_proj)
            zero_charges.append(0)
        else:
            non_zero_x.append(x_proj)
            non_zero_y.append(y_proj)
            non_zero_charges.append(charge)
            non_zero_times.append(time)
    
    # Create circular boundary
    theta = np.linspace(0, 2*np.pi, 100)
    circle_x = np.cos(theta)
    circle_y = np.sin(theta)
    ax.plot(circle_x, circle_y, 'black', linewidth=1)
    
    # Plot grid circles for stereographic projection
    for lat in [30,60]:
        lat_rad=np.deg2rad(lat)
        circle_x = np.cos(lat_rad)/(1+np.sin(lat_rad)) * np.cos(theta)
        circle_y = np.cos(lat_rad)/(1+np.sin(lat_rad)) * np.sin(theta)
        ax.plot(circle_x, circle_y, 'gray', linestyle='--', alpha=0.5, linewidth=0.5)
        label_x=np.cos(lat_rad)/(1+np.sin(lat_rad))
        label_y=0
        ax.text(label_x, label_y, f'{lat}°', fontsize=8, 
                ha='center', va='center', 
                bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', pad=1))
    
    # Plot radial lines with angle labels
    for angle in np.arange(0, 360, 45):
        angle_rad = np.radians(angle)
        x_line = [0, np.cos(angle_rad)]
        y_line = [0, np.sin(angle_rad)]
        ax.plot(x_line, y_line, 'gray', linestyle='--', alpha=0.5, linewidth=0.5)
        
        # Add angle labels at the edge
        label_x = 1.1 * np.cos(angle_rad)
        label_y = 1.1 * np.sin(angle_rad)
        ax.text(label_x, label_y, f'{angle}°', fontsize=8, 
                ha='center', va='center', 
                bbox=dict(facecolor='white', alpha=0.7, edgecolor='none', pad=1))
    
    # Plot non-zero time points with same colormap and range
    if non_zero_x:
        sizes_non_zero = [sqrt(charge)*8 for charge in non_zero_charges]
        scatter = ax.scatter(
            non_zero_x, non_zero_y,
            c=non_zero_times,
            s=sizes_non_zero,
            cmap='RdYlGn_r',
            vmin=vmin,
            vmax=vmax,
            marker='o',
            edgecolors='black',
            linewidths=0.5
        )
    
    # Plot zero time points in gray
    if zero_x:
        sizes_zero = [sqrt(50)*8 for charge in zero_charges]
        ax.scatter(
            zero_x, zero_y,
            c='#D0D0D0',
            s=sizes_zero,
            marker='o',
            edgecolors='black',
            linewidths=0.5
        )
    
    ax.set_title(title)
    ax.set_aspect('equal')
    
    # Set axis limits to show complete circle with some margin for labels
    ax.set_xlim(-1.2, 1.2)
    ax.set_ylim(-1.2, 1.2)
    ax.set_axis_off()

def load_pmt_positions_from_file(input_file):
    """Load PMT data from file (unchanged)"""
    pmt_data = []
    title= ""
    info=""
    infoswitch=False
    with open(input_file, 'r') as file:
        for line in file:
            if infoswitch:
                if line.startswith('info end'):
                    infoswitch=False
                else:
                    info+="\n"
                    info+=line

            if line.startswith('info:'):
                infoswitch=True

            if line.startswith('title: '):
                title = line[7:]
                continue
            if re.match(r'^\D+',line):
                continue
            parts = line.strip().replace(',', ' ').split()
            if len(parts) >= 6:
                id, x, y, z, charge, time = parts[:6]
                if id in (26,29,38,54):
                    charge = 0
                pmt_data.append((int(id),float(x), float(y), float(z), float(charge), float(time)))
    return title,info,pmt_data

def change_extension(path, new_exten):
    if path is None:
        return None
    
    pattern = r'\.\w*$'
    match = re.search(pattern, path)
    
    if match:
        result = path[:match.start()]
    else:
        result = path
    
    return f"{result}{new_exten}"

if __name__ == "__main__":
    if len(sys.argv) <= 3:
        print("Usage: python plot2D.py datapath.txt")
        sys.exit(1)
    inputpath = str(sys.argv[2])

    if not inputpath.endswith('.txt'):
        inputpath=change_extension(inputpath,".txt")
    outputpath=inputpath[:-4]+".pdf"

    if os.path.exists(inputpath):
        [title,info,pmt_data] = load_pmt_positions_from_file(inputpath)
        if (info != ""):
            plot_withInfo(title,info, pmt_data, str(outputpath))
        else:
            plotEvent(title,pmt_data, str(outputpath))