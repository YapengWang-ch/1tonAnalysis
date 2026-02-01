#!/usr/bin/env python3
import os
import numpy as np

# channel -> board id 映射（保留原列表）
boardID = [0,1,2,3,4,6,2,2,1,2,0,5,4,5,6,7,0,1,5,3,4,5,6,7,0,1,3,3,4,2,6,7,0,1,5,3,4,5,6,7,3,1,4,3,4,5,6,7,0,1,7,5,6,0,4,7,0,1,2,3]

def read_boardfix(boardfix_path):
    """读取 boardfix.txt，返回 dict: run(int) -> [8 ints]"""
    fixes = {}
    with open(boardfix_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            try:
                run = int(parts[0])
                vals = [int(x) for x in parts[1:1+8]]
                if len(vals) < 8:
                    # 填充为 8 个 0
                    vals += [0] * (8 - len(vals))
                fixes[run] = vals
            except ValueError:
                continue
    return fixes

def read_timeoffset(timeoffset_path):
    """读取 TimeOffset.txt，返回 (header_lines, data_rows)
    data_rows: dict channel->(mean, other_cols_list, raw_cols)
    """
    header = []
    data = {}
    with open(timeoffset_path, 'r') as f:
        for line in f:
            if line.startswith('#'):
                header.append(line.rstrip('\n'))
            else:
                line = line.strip()
                if not line:
                    continue
                cols = line.split()
                # 期望至少 6 列：channel mean meanErr sigma sigmaErr entries
                try:
                    ch = int(cols[0])
                    mean = float(cols[1])
                except Exception:
                    continue
                # 保存其余列，以便写回（保持原格式）
                other = cols[2:]
                data[ch] = (mean, other, cols)
    return header, data

def apply_boardfix_and_write(run, header, time_data, board_fix, out_dir):
    """对单个 run 应用 board 修正并写文件"""
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"TimeOffset_{run}.txt")

    # 先计算每个通道的修正后值（但还未做均值归零）
    corrected = {}
    for ch, (mean, other, rawcols) in time_data.items():
        if mean == 0:
            corrected_mean = 0.0
        else:
            # 找到该通道对应的板
            if ch < len(boardID):
                bid = boardID[ch]
            else:
                # 若越界，保守处理为 0
                bid = 0
            board_correction = board_fix[bid] if bid < len(board_fix) else 0
            corrected_mean = mean + board_correction
            # 原脚本逻辑：若 < -8 加 16（环绕）
            # if corrected_mean < -8:
            #     corrected_mean += 16.0
        corrected[ch] = corrected_mean

    # 计算非零通道的整体均值并归零
    nonzero = [v for v in corrected.values() if v != 0.0]
    if len(nonzero) > 0:
        overall_mean = float(np.mean(nonzero))
    else:
        overall_mean = 0.0

    final = {}
    for ch, val in corrected.items():
        if val == 0.0:
            final[ch] = 0.0
        else:
            final[ch] = val - overall_mean

    # 写入文件（保留 header）
    with open(out_path, 'w') as f:
        for h in header:
            f.write(h + '\n')
        # 按通道从小到大输出（与原 TimeOffset 顺序一致）
        for ch in sorted(time_data.keys()):
            mean_orig, other, rawcols = time_data[ch]
            mean_out = final[ch]
            # 组装其余列（若缺少列，填充）
            # 假设原来至少有 6 列，写回格式：ch \t mean \t col2 \t col3 ...
            other_out = '\t'.join(other) if other else ''
            if other_out:
                f.write(f"{ch}\t{mean_out:.6f}\t{other_out}\n")
            else:
                f.write(f"{ch}\t{mean_out:.6f}\n")

    return out_path, overall_mean

def main():
    # 假设脚本放在 data/ 目录下
    base_dir = os.path.dirname(os.path.abspath(__file__))
    timeoffset_path = os.path.join(base_dir, "TimeOffset.txt")
    boardfix_path = os.path.join(base_dir, "boardfix.txt")
    out_dir = os.path.join(base_dir, "TCaliFixed")

    if not os.path.exists(timeoffset_path):
        print(f"ERROR: 找不到 {timeoffset_path}")
        return
    if not os.path.exists(boardfix_path):
        print(f"ERROR: 找不到 {boardfix_path}")
        return

    boardfixes = read_boardfix(boardfix_path)
    header, time_data = read_timeoffset(timeoffset_path)

    if not boardfixes:
        print("WARNING: boardfix.txt 中没有有效条目，退出")
        return
    if not time_data:
        print("WARNING: TimeOffset.txt 中没有有效通道数据，退出")
        return

    print(f"读取 TimeOffset 模板: {timeoffset_path}")
    print(f"读取 boardfix 表: {boardfix_path}，共 {len(boardfixes)} 个 run")

    for run, fixvals in boardfixes.items():
        out_path, overall_mean = apply_boardfix_and_write(run, header, time_data, fixvals, out_dir)
        print(f"run {run}: 写入 {out_path}（调整后整体均值 = {overall_mean:.6f}）")

if __name__ == "__main__":
    main()