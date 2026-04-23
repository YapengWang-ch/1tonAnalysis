import numpy as np
import matplotlib.pyplot as plt
import re
import os
import matplotlib as mpl

# 设置全局字体为英文
mpl.rcParams['font.family'] = 'sans-serif'
mpl.rcParams['font.sans-serif'] = ['Arial', 'Helvetica', 'DejaVu Sans']
mpl.rcParams['mathtext.fontset'] = 'stix'  # 数学公式字体

def parse_risetime_file(filename):
    """Parse the first format file (risetime_fit_results.txt)"""
    data = {}
    with open(filename, 'r') as f:
        for line in f:
            # Skip comment lines
            if line.startswith('#'):
                continue
            # Parse data line
            parts = line.strip().split('\t')
            if len(parts) >= 5:
                ch = int(parts[0])
                mean = float(parts[1])
                # Skip values less than 100
                if mean < 100:
                    continue
                mean_err = float(parts[2])
                # Store channel, mean and error
                data[ch] = (mean, mean_err)
    return data

def parse_timescale_file(filename):
    """Parse the second format file (time_scale_results.txt)"""
    data = {}
    pattern = r"PMTid: (\d+) Time scale: ([\d.]+)\+/-([\d.]+)"
    with open(filename, 'r') as f:
        for line in f:
            match = re.match(pattern, line.strip())
            if match:
                ch = int(match.group(1))
                mean = float(match.group(2))
                # Skip values less than 100
                if mean < 100:
                    continue
                mean_err = float(match.group(3))
                data[ch] = (mean, mean_err)
    return data

def plot_comparison(data1, data2, label1, label2, output_filename=None):
    """Plot comparison of two datasets"""
    # Get common channels
    common_channels = sorted(set(data1.keys()) & set(data2.keys()))
    
    if not common_channels:
        print("Warning: No common channels found")
        return
    
    # Extract data
    chans = np.array(common_channels)
    means1 = np.array([data1[ch][0] for ch in common_channels])
    errs1 = np.array([data1[ch][1] for ch in common_channels])
    means2 = np.array([data2[ch][0] for ch in common_channels])
    errs2 = np.array([data2[ch][1] for ch in common_channels])
    
    # Calculate differences
    differences = means1 - means2
    relative_diff = (differences / np.maximum(means1, means2)) * 100  # percentage
    
    # Create figure
    plt.figure(figsize=(16, 12))
    plt.suptitle('Time Scale Comparison', fontsize=16, fontweight='bold')
    
    # 1. Mean comparison plot
    ax1 = plt.subplot(2, 1, 1)
    plt.errorbar(chans, means1, yerr=errs1, fmt='o-', color='blue', 
                 capsize=5, label=label1)
    plt.errorbar(chans, means2, yerr=errs2, fmt='s-', color='red', 
                 capsize=5, label=label2)
    plt.title('Time Scale Comparison', fontsize=14)
    plt.xlabel('Channel ID', fontsize=12)
    plt.ylabel('Time Scale (ns)', fontsize=12)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # Add value labels
    for i, chan in enumerate(chans):
        plt.text(chan, max(means1[i], means2[i]) + 2, 
                 f"{means1[i]:.1f}\n{means2[i]:.1f}", 
                 ha='center', fontsize=8)
    
    # 2. Difference analysis plot
    ax2 = plt.subplot(2, 1, 2)
    bars = plt.bar(chans, differences, color='green', alpha=0.7, 
                   label='Absolute Difference')
    plt.title('Time Scale Difference Analysis', fontsize=14)
    plt.xlabel('Channel ID', fontsize=12)
    plt.ylabel('Absolute Difference (ns)', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    
    # Add relative difference on secondary axis
    ax2b = ax2.twinx()
    line = ax2b.plot(chans, relative_diff, 'm--o', label='Relative Difference')
    ax2b.set_ylabel('Relative Difference (%)', fontsize=12)
    
    # Combine legends
    lines, labels = ax2.get_legend_handles_labels()
    lines2, labels2 = ax2b.get_legend_handles_labels()
    ax2b.legend(lines + lines2, labels + labels2, loc='best')
    
    # Add difference value labels
    for i, chan in enumerate(chans):
        ax2.text(chan, differences[i] + (0.5 if differences[i] >= 0 else -1.5), 
                 f"{differences[i]:.1f}ns\n({relative_diff[i]:.1f}%)", 
                 ha='center', fontsize=8, 
                 va='bottom' if differences[i] >= 0 else 'top')
    
    # Adjust layout
    plt.tight_layout()
    plt.subplots_adjust(top=0.93)
    
    # Save figure
    if output_filename:
        plt.savefig(output_filename, dpi=300, bbox_inches='tight')
        print(f"Comparison plot saved as {output_filename}")
    
    # Show plot
    plt.show()
    
    # Return difference analysis results
    diff_results = {
        'channels': chans,
        'mean_diff': differences,
        'relative_diff': relative_diff,
        'max_abs_diff': np.max(np.abs(differences)),
        'mean_abs_diff': np.mean(np.abs(differences)),
        'max_rel_diff': np.max(np.abs(relative_diff)),
        'mean_rel_diff': np.mean(np.abs(relative_diff))
    }
    
    print("\nDifference Statistics:")
    print(f"Max Absolute Difference: {diff_results['max_abs_diff']:.2f} ns")
    print(f"Mean Absolute Difference: {diff_results['mean_abs_diff']:.2f} ns")
    print(f"Max Relative Difference: {diff_results['max_rel_diff']:.2f}%")
    print(f"Mean Relative Difference: {diff_results['mean_rel_diff']:.2f}%")
    
    return diff_results

def main():
    # File paths
    risetime_file = "risetime_fit_results.txt"  # First format
    timescale_file = "temp.txt"   # Second format
    
    # Check if files exist
    if not os.path.exists(risetime_file):
        print(f"Error: File {risetime_file} not found")
        return
    
    if not os.path.exists(timescale_file):
        print(f"Error: File {timescale_file} not found")
        return
    
    # Parse files
    risetime_data = parse_risetime_file(risetime_file)
    timescale_data = parse_timescale_file(timescale_file)
    
    print(f"Parsed {len(risetime_data)} channels from risetime data")
    print(f"Parsed {len(timescale_data)} channels from timescale data")
    
    # Plot comparison
    output_png = "time_scale_comparison.png"
    diff_results = plot_comparison(
        risetime_data, 
        timescale_data, 
        "Risetime Analysis", 
        "Timescale Analysis",
        output_png
    )
    
    # Save difference results to text file
    if diff_results:
        diff_file = "time_scale_differences.csv"
        with open(diff_file, 'w') as f:
            f.write("Channel,Risetime_Mean(ns),Timescale_Mean(ns),Absolute_Diff(ns),Relative_Diff(%)\n")
            
            for i, chan in enumerate(diff_results['channels']):
                f.write(f"{chan},{risetime_data[chan][0]:.3f},"
                        f"{timescale_data[chan][0]:.3f},"
                        f"{diff_results['mean_diff'][i]:.3f},"
                        f"{diff_results['relative_diff'][i]:.2f}\n")
        
        print(f"\nDifference results saved as {diff_file}")

if __name__ == "__main__":
    main()