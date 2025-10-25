import cv2
import torch
from torch.utils.data import Dataset
import os
import numpy as np
import rawpy
import sys
import random
from tqdm import tqdm
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from common.common_func import show_image, show_two_images, show_three_images, show_four_images


# class BayerDataset(Dataset):
#     def __init__(self, file_list):
#         self.files = []
#         with open(file_list, "r") as f:
#             for line in f:
#                 input_path, target_path = line.strip().split(",")
#                 self.files.append((input_path, target_path))

#     def __len__(self):
#         return len(self.input_files)
    



#     def process_raw(raw_path, black=512, white=16383, cam_mul=[1976, 1024, 2316, 1024]):
#         """
#         raw_path: RAW file 路徑
#         black: 黑電平
#         white: 白電平
#         cam_mul: 相機 AWB multiplier (R, G1, B, G2 ordering depends on sensor)
#         """

#         # 1. 讀取 RAW
#         with rawpy.imread(raw_path) as raw:
#             raw_image = raw.raw_image_visible.astype(np.float32)  # 原始 Bayer，float32
#             # show_image(raw_image, None)
#         # 2. 黑白電平校正
#         raw32 = (raw_image - black) / (white - black)  # normalize to 0~1
#         raw32 = np.clip(raw32, 0.0, 1.0)               # clip <0 or >1

#         # 3. 計算正規化 AWB multiplier
#         g_ref = cam_mul[1]  # 第一個 G 作為 reference
#         cam_mul_normalized = [
#             cam_mul[0] / g_ref,  # R
#             cam_mul[1] / g_ref,  # G1
#             cam_mul[3] / g_ref,  # G2
#             cam_mul[2] / g_ref   # B
#         ]

#         # 4. 套用 AWB multiplier
#         # 假設 Bayer pattern 為 RGGB (行列偶數奇數)
#         h, w = raw32.shape
#         raw32_awb = raw32.copy()
#         # R channel
#         raw32_awb[0:h:2, 0:w:2] *= cam_mul_normalized[0]
#         # G1 channel
#         raw32_awb[0:h:2, 1:w:2] *= cam_mul_normalized[1]
#         # G2 channel
#         raw32_awb[1:h:2, 0:w:2] *= cam_mul_normalized[2]
#         # B channel
#         raw32_awb[1:h:2, 1:w:2] *= cam_mul_normalized[3]

#         # 5. Clip 0~1 再返回
#         raw32_awb = np.clip(raw32_awb, 0.0, 1.0)
#         # show_image(raw32_awb, None)
#         return raw32_awb
    
#     def read_txt_and_process(txt_path, base_dir=".", black=512, white=16383, cam_mul=[1976, 1024, 2316, 1024]):
#         """
#         從文字檔讀取 RAW 路徑資訊並處理短曝與長曝影像
#         txt 格式:
#         ./Sony/short/00001_00_0.04s.ARW ./Sony/long/00001_00_10s.ARW ISO200 F8
#         """

#         if not os.path.exists(txt_path):
#             raise FileNotFoundError(f"找不到檔案: {txt_path}")

#         with open(txt_path, "r", encoding="utf-8") as f:
#             lines = f.readlines()

#         short_long_pairs = []
#         for line in lines:
#             parts = line.strip().split()
#             if len(parts) < 2:
#                 continue
#             short_path = os.path.join(base_dir, "data/raw/Sony", parts[0])
#             long_path = os.path.join(base_dir, "data/raw/Sony", parts[1])
#             short_long_pairs.append((short_path, long_path))

#         processed_list = []
#         for short_path, long_path in short_long_pairs:
#             print(f"\n🔹 處理短曝光: {short_path}")
#             short_img = BayerDataset.process_raw(short_path, black, white, cam_mul)

#             print(f"🔸 處理長曝光: {long_path}")
#             long_img = BayerDataset.process_raw(long_path, black, white, cam_mul)

#             processed_list.append({
#                 "short_path": short_path,
#                 "long_path": long_path,
#                 "short_img": short_img,
#                 "long_img": long_img
#             })
#             # show_image(short_img, None)

        
#         return processed_list


class BayerDataset(Dataset):
    def __init__(self, txt_path, base_dir=".", black=512, white=16383, cam_mul=[1976, 1024, 2316, 1024], crop_size=512, num_crops=4):
        """
        txt_path: RAW 檔案列表文字檔，每行格式：
                  ./Sony/short/00001_00_0.04s.ARW ./Sony/long/00001_00_10s.ARW ISO200 F8
        base_dir: RAW 檔案根目錄
        black, white: 黑白電平
        cam_mul: 相機 AWB multiplier (R, G1, B, G2)
        """
        if not os.path.exists(txt_path):
            raise FileNotFoundError(f"找不到檔案: {txt_path}")

        self.black = black
        self.white = white
        self.cam_mul = cam_mul
        self.base_dir = base_dir
        self.pairs = []
        self.crop_size = crop_size
        self.num_crops = num_crops

        with open(txt_path, "r", encoding="utf-8") as f:
            lines = f.readlines()
        for line in lines:
            parts = line.strip().split()
            if len(parts) < 2:
                continue
            short_path = os.path.join(base_dir, "data/raw/Sony", parts[0])
            long_path = os.path.join(base_dir, "data/raw/Sony", parts[1])
            self.pairs.append((short_path, long_path))

    def __len__(self):
        return len(self.pairs)

    @staticmethod
    def process_raw(raw_path, black=512, white=16383, cam_mul=[1976,1024,2316,1024], crop_size=512, num_crops=2):
        """讀 RAW、黑白電平 normalize、AWB、clip"""
        with rawpy.imread(raw_path) as raw:
            raw_image = raw.raw_image_visible.astype(np.float32)

        # 黑白電平 normalize
        raw32 = (raw_image - black) / (white - black)
        raw32 = np.clip(raw32, 0.0, 1.0)

        # AWB multiplier
        g_ref = cam_mul[1]
        cam_mul_normalized = [
            cam_mul[0] / g_ref,  # R
            cam_mul[1] / g_ref,  # G1
            cam_mul[3] / g_ref,  # G2
            cam_mul[2] / g_ref   # B
        ]

        # 4. 拆通道 (4 channels)
        h, w = raw32.shape
        ch_R  = raw32[0:h:2, 0:w:2] * cam_mul_normalized[0]  # R
        ch_G1 = raw32[0:h:2, 1:w:2] * cam_mul_normalized[1]  # G1
        ch_G2 = raw32[1:h:2, 0:w:2] * cam_mul_normalized[2]  # G2
        ch_B  = raw32[1:h:2, 1:w:2] * cam_mul_normalized[3]  # B

        # 5. Clip 0~1
        ch_R  = np.clip(ch_R, 0.0, 1.0)
        ch_G1 = np.clip(ch_G1, 0.0, 1.0)
        ch_G2 = np.clip(ch_G2, 0.0, 1.0)
        ch_B  = np.clip(ch_B, 0.0, 1.0)

        # 6. 合成 4 通道影像 (H/2, W/2, 4)
        raw4ch = np.stack([ch_R, ch_G1, ch_G2, ch_B], axis=-1)
        return raw4ch

    def random_crop(self, img):
        """從 4 channel raw 影像隨機 crop"""
        H, W, C = img.shape
        ch = self.crop_size
        if H <= ch or W <= ch:
            # 如果圖比 crop 小就直接 return 原圖
            return img
        top = random.randint(0, H - ch)
        left = random.randint(0, W - ch)
        crop = img[top:top+ch, left:left+ch, :]
        return crop
    
    def __getitem__(self, idx):
        short_path, long_path = self.pairs[idx]
        short_img = self.process_raw(short_path)  # 4 通道 Bayer
        long_img  = self.process_raw(long_path)   # 4 通道 Bayer

        short_crops = []
        long_crops  = []

        for _ in range(self.num_crops):
            # 隨機 crop
            short_crop = self.random_crop(short_img)
            long_crop  = self.random_crop(long_img)

            # -----------------------------
            # 轉成 torch Tensor
            # -----------------------------
            # short: 4 通道 Bayer
            short_tensor = torch.from_numpy(short_crop).permute(2,0,1).float()  # C x H x W

            # long: Bayer → RGB 3通道
            long_rgb = np.stack([
                long_crop[:,:,0],                    # R
                (long_crop[:,:,1] + long_crop[:,:,2])/2,  # G = G1+G2 平均
                long_crop[:,:,3]                     # B
            ], axis=-1)
            long_tensor = torch.from_numpy(long_rgb).permute(2,0,1).float()  # C x H x W

            short_crops.append(short_tensor)
            long_crops.append(long_tensor)

        # 堆成 tensor: N x C x H x W
        short_crops_tensor = torch.stack(short_crops, dim=0)
        long_crops_tensor  = torch.stack(long_crops, dim=0)

        return short_crops_tensor, long_crops_tensor
    


# txt_path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony_val_list.txt"
# with open(txt_path, "r", encoding="utf-8") as f:
#     lines = f.readlines()


# black=512
# white=16383
# cam_mul=[1976, 1024, 2316, 1024]
# crop_size=256
# num_crops=10
# crop_h = crop_w = crop_size

# for line in tqdm(lines):
#     parts = line.strip().split()
#     if len(parts) < 2:
#         continue

#     short_path = os.path.join("./data/raw/Sony", parts[0])
#     long_path  = os.path.join("./data/raw/Sony", parts[1])

#     short_img = BayerDataset.process_raw(short_path, black, white, cam_mul)
#     long_img  = BayerDataset.process_raw(long_path, black, white, cam_mul)

#     # 保證 crop 對齊
#     H, W, C = short_img.shape
#     crops_short = []
#     crops_long  = []
#     for _ in range(num_crops):
#         y = np.random.randint(0, H - crop_h + 1)
#         x = np.random.randint(0, W - crop_w + 1)

#         short_crop = short_img[y:y+crop_h, x:x+crop_w, :]
#         long_crop  = long_img[y:y+crop_h, x:x+crop_w, :]

#         # -----------------------------
#         # 轉 long_crop 為 RGB 3 通道
#         # R, G = (G1+G2)/2, B
#         long_rgb = np.stack([
#             long_crop[:,:,0],                    # R
#             (long_crop[:,:,1] + long_crop[:,:,2])/2,  # G = (G1+G2)/2
#             long_crop[:,:,3]                     # B
#         ], axis=-1)

#         crops_short.append(short_crop)
#         crops_long.append(long_rgb)

#     # 存檔
#     output_dir = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Val_Crops"
#     base_name = os.path.splitext(os.path.basename(short_path))[0]
#     np.save(os.path.join(output_dir, f"{base_name}_short.npy"), np.array(crops_short))  # 4通道 Bayer
#     np.save(os.path.join(output_dir, f"{base_name}_long.npy"),  np.array(crops_long))   # 3通道 RGB
