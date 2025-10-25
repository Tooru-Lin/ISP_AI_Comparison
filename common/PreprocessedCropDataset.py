import os
import torch
from torch.utils.data import Dataset
import numpy as np

class PreprocessedCropDataset(Dataset):
    """
    讀取預先生成的 crops (short/long)
    crops shape: (num_crops, crop_h, crop_w, 4)
    Dataset 回傳 shape: N x C x H x W
    """
    def __init__(self, crop_dir):
        self.crop_dir = crop_dir
        # 找所有 short crops，並假設對應 long crops 同名
        self.short_files = sorted([f for f in os.listdir(crop_dir) if f.endswith("_short.npy")])

    def __len__(self):
        return len(self.short_files)

    def __getitem__(self, idx):
        short_path = os.path.join(self.crop_dir, self.short_files[idx])
        long_path  = short_path.replace("_short.npy", "_long.npy")

        # 讀取 N x H x W x C
        short_crops = np.load(short_path)
        long_crops  = np.load(long_path)

        # 轉成 N x C x H x W
        short_tensor = torch.from_numpy(short_crops).permute(0, 3, 1, 2).float()
        long_tensor  = torch.from_numpy(long_crops).permute(0, 3, 1, 2).float()

        return short_tensor, long_tensor  # shape: num_crops x 4 x H x W
