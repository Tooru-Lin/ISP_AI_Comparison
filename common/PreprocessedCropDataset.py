import os
import torch
from torch.utils.data import Dataset
import numpy as np
from tqdm import tqdm  # 可選：用來顯示初始化載入進度



class PreprocessedCropDataset(Dataset):
    """
    讀取預先生成的 crops (short/long)
    crops shape: (num_crops, crop_h, crop_w, 4)
    Dataset 回傳 shape: N x C x H x W
    """
    def __init__(self, crop_dir, short_files=None, mmap_mode=None):
        self.crop_dir = crop_dir
        self.mmap_mode = mmap_mode
        
        # 💡 優先使用外部傳入的檔案列表；若沒傳入才在本地掃描（相容舊寫法）
        if short_files is not None:
            self.short_files = short_files
        else:
            self.short_files = sorted([f for f in os.listdir(crop_dir) if f.endswith("_short.npy")])

    def __len__(self):
        return len(self.short_files)

    def __getitem__(self, idx):
        short_name = self.short_files[idx]
        long_name = short_name.replace("_short.npy", "_long.npy")

        short_path = os.path.join(self.crop_dir, short_name)
        long_path  = os.path.join(self.crop_dir, long_name)

        # 💡 可選 mmap_mode="r" 來加速讀取（若硬碟支援）
        short_crops = np.load(short_path, mmap_mode=self.mmap_mode)
        long_crops  = np.load(long_path, mmap_mode=self.mmap_mode)

        # 轉成 Tensor 並調整 Dimension (N, H, W, C) -> (N, C, H, W)
        short_tensor = torch.from_numpy(short_crops).permute(0, 3, 1, 2).float()
        long_tensor  = torch.from_numpy(long_crops).permute(0, 3, 1, 2).float()

        return short_tensor, long_tensor
