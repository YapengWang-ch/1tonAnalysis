#!/usr/bin/env python3
'''
读入 ser 处理后的波形信息，并用初步重建结果筛选刻度源事例
'''
import h5py
import numpy as np
from tqdm import tqdm
from ReconReader import Reader
import argparse

psr = argparse.ArgumentParser()
psr.add_argument("-i", dest="ipt", type=str, nargs="+")
psr.add_argument("-o", dest="opt", type=str)
psr.add_argument("-k", dest="key", type=str)
args = psr.parse_args()

# 判断这个数据集来源
if "Co" in args.key:
    Ecut0 = 2.2
    Ecut1 = 2.8
else:
    # TODO: Th energy cut
    Ecut0 = 2.2
    Ecut1 = 2.8 
if "up" in args.key:
    zcut = 0.31
elif "down" in args.key:
    zcut = - 0.31
else:
    zcut = 0
R = 0.2

# 读入死亡 PMT 列表
Death = np.loadtxt("DeathTable.txt", dtype=np.int16)

# 读入波形信息和重建结果
opt_dtype = [
    ("eid", np.int32),
    ("ch", np.int32),
    ("No", np.int32),
    ("sig2w", np.float64),
    ("charge", np.float64),
    ("peak", np.float64),
    ("up20", np.float64),
]
waves = np.array([], dtype=opt_dtype)
for i, filer in tqdm(enumerate(args.ipt)):
    with h5py.File(filer.replace("smoke/tvE", "ser/baseline"), 'r') as h:
        wave = h['baseline'][:]
    recon = Reader(filer)
    # 用重建结果的能量位置筛选刻度源事例
    cutE = (recon['E'] > Ecut0) & (recon['E'] < Ecut1)
    cutR = ((recon['x'] ** 2) < (R ** 2)) & ((recon['y'] ** 2) < (R ** 2)) & ((recon['z'] - zcut) ** 2 < (R ** 2))
    eids = recon['EventID'][cutE & cutR]
    Eventcut = np.isin(wave['eid'], eids)
    Channelcut = np.isin(wave['ch'], Death)
    wave_select = wave[Eventcut & (~Channelcut)][['eid', 'ch', 'offset', 'sig2w', 'charge', 'peak', 'up20']].astype(opt_dtype)
    wave_select['No'] = i
    waves = np.append(waves, wave_select)

# 保存筛选出来的波形相关信息
opts = {"compression": "gzip", "shuffle": True}
with h5py.File(args.opt, "w") as opt:
    opt.create_dataset("wave", data = waves, **opts)
