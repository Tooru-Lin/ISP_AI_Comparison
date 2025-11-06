
import torch
import torch.nn as nn
import torch.optim as optim
from tqdm import tqdm
import os
import sys
import time
import numpy as np
import pytorch_ssim
import onnx
import onnxoptimizer
from onnxruntime.quantization import quantize_dynamic, QuantType
from torch.utils.data import DataLoader, Dataset
from torchvision import transforms
from torch.utils.tensorboard import SummaryWriter
from torchmetrics.image import StructuralSimilarityIndexMeasure

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from isp_ai.unet import UNet
from common.PreprocessedCropDataset import PreprocessedCropDataset
from common.BayerDataset import BayerDataset
from common.common_func import *


device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
l1_loss = nn.L1Loss()
ssim = StructuralSimilarityIndexMeasure(data_range=1.0).to(device)
def l1_ssim_loss(output, target, alpha=0.84):
    output = output.float()
    target = target.float()
    # SSIM 值在 [0,1] 越大越相似
    ssim_val = ssim(output, target)
    return alpha * (1 - ssim_val) + (1 - alpha) * l1_loss(output, target)

def l1_ssim_brightness_weighted(output, target, alpha=0.84):
    output = output.float()
    target = target.float()
    brightness_weight = torch.clamp(target.mean(dim=1, keepdim=True) * 2.0, 0, 1)
    ssim_val = ssim(output, target)
    l1_val = l1_loss(output, target)
    return alpha * (1 - ssim_val) + (1 - alpha) * (l1_val * brightness_weight).mean()


def main():
    # # 1️⃣ Dataset
    # train_dataset = BayerDataset("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony_train_list.txt")
    # val_dataset   = BayerDataset("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony_val_list.txt")

    # # 2️⃣ DataLoader
    # train_loader = DataLoader(train_dataset, batch_size=4, shuffle=True, num_workers=4, pin_memory=True)
    # val_loader   = DataLoader(val_dataset, batch_size=4, shuffle=False, num_workers=4, pin_memory=True)

    train_dataset   = PreprocessedCropDataset("./data/raw/Sony/Train_Crops")
    val_dataset = PreprocessedCropDataset("./data/raw/Sony/Val_Crops")

    train_loader = DataLoader(train_dataset, batch_size=7, shuffle=True, num_workers=4, pin_memory=True)
    val_loader = DataLoader(val_dataset, batch_size=7, shuffle=False, num_workers=4, pin_memory=True)

    # 使用 GPU (如果可用)
    print(f"Using device: {device}")

    # 建立模型、損失函數和優化器
    model = UNet().to(device)
    optimizer = optim.AdamW(model.parameters(), lr=1e-5, weight_decay=1e-5)
    # criterion = nn.SmoothL1Loss()
    criterion = l1_ssim_loss

    # 建立 checkpoints 目錄
    os.makedirs("checkpoints", exist_ok=True)

    num_epochs = 500
    writer = SummaryWriter("runs/ISP_Demosaic")
    best_loss = float('inf')
    print_interval = 10  # 每隔多少秒打印一次

    # 從斷點繼續訓練
    # ckpt = torch.load("checkpoints/unet_epoch204.pth")
    # model.load_state_dict(ckpt['model_state_dict'])
    # optimizer.load_state_dict(ckpt['optimizer_state_dict'])
    # start_epoch = ckpt['epoch'] + 1
    
    for epoch in range(1, num_epochs):
        model.train()
        total_loss = 0

        last_print = time.time()
        for batch_idx, (short, long) in enumerate(train_loader):
            # short/long shape: B x N x C x H x W
            B, N, C_in, H, W = short.shape
            C_out = long.shape[2]  # RGB target 3ch

            x = short.view(B*N, C_in, H, W).to(device)  # Bayer 4ch
            y = long.view(B*N, C_out, H, W).to(device) # RGB 3ch

            # 重新計算 SSIM 狀態
            ssim.reset()  # 可加上這行
            
            optimizer.zero_grad()
            out = model(x)
            loss = criterion(out, y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()

            # -------------------------------
            # 每 print_interval 秒打印一次
            # -------------------------------
            if time.time() - last_print > print_interval:
                avg_loss = total_loss / (batch_idx + 1)
                progress = (batch_idx + 1) / len(train_loader) * 100
                sys.stdout.write(f"\rBatch {batch_idx+1}/{len(train_loader)} ({progress:.1f}%), Avg Loss: {avg_loss:.6f}")
                sys.stdout.flush()
                last_print = time.time()

        avg_train_loss = total_loss / len(train_loader)
        print(f"\nEpoch [{epoch+1}/{num_epochs}], Train Loss: {avg_train_loss:.4f}")
        writer.add_scalar("Loss/train", avg_train_loss, epoch)

        # 驗證
        model.eval()
        val_loss = 0
        with torch.no_grad():
            for short, long in val_loader:
                B, N, C_in, H, W = short.shape
                C_out = long.shape[2]
                x_val = short.view(B*N, C_in, H, W).to(device)
                y_val = long.view(B*N, C_out, H, W).to(device)
                out_val = model(x_val)
                val_loss += criterion(out_val, y_val).item()

        avg_val_loss = val_loss / len(val_loader)
        print(f"[Val]   Epoch [{epoch+1}/{num_epochs}], Loss: {avg_val_loss:.4f}")

        # 保存模型
        if avg_val_loss < best_loss:
            best_loss = avg_val_loss
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict()
            }, f"checkpoints/unet_L1_SSIM_epoch{epoch+1}.pth")
            print("Model saved.")





if __name__ == "__main__":


    # --- 1. 設定路徑 ---
    ckpt_path = "checkpoints/unet_epoch205.pth"
    # input_path = "data/raw/Sony/Sony/short/00001_00_0.1s.ARW"
    save_path = "data/sample_output.tiff"

    # --- 2. 建立模型 ---
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = UNet().to(device)

    # --- 3. 載入訓練權重 ---
    ckpt = torch.load(ckpt_path, map_location=device)
    model.load_state_dict(ckpt["model_state_dict"])
    model.eval()

    # --- 4. 讀取並預處理輸入圖 ---
    # 假設你是輸入 Bayer pattern (4 channel)

    txt_path = "./data/raw/Sony/Sony_train_list.txt"
    with open(txt_path, "r", encoding="utf-8") as f:
        lines = f.readlines()


    for line in tqdm(lines):
        parts = line.strip().split()
        if len(parts) < 2:
            continue

        short_path = os.path.join("./data/raw/Sony", parts[0])
        short_img = BayerDataset.process_raw(short_path)

        show_image(short_img)

        short_img = np.transpose(short_img, (2, 0, 1))
        img = torch.from_numpy(short_img).unsqueeze(0).float().to(device)

        # --- 5. 推論 ---
        with torch.no_grad():
            output = model(img)  # [1, 3, H, W]

        # --- 6. 後處理與亮度壓縮 ---
        output_img = output.squeeze(0).permute(1, 2, 0).cpu().numpy()  # [H, W, 3]

        # 取出亮度的 99% 百分位作為最高強度
        max_val = torch.quantile(output, 0.99)

        # 以 99% 百分位為基準正規化，再 clamp 到 [0, 1]
        output = output / max_val
        output = output.clamp(0, 1)
                
        # --- 6. 後處理與儲存 ---
        output = output.squeeze(0).cpu().clamp(0, 1)

        # 轉為 PIL 圖片並儲存
        output_img = transforms.ToPILImage()(output)
        show_image(output_img, None)
        output_img.save(save_path)


        # 原始模型路徑
        model_fp32 = "model_raw.onnx"
        # 優化後模型路徑
        model_fp32_opt = "model_optimized.onnx"
        # 儲存量化後模型
        model_int8 = "model_int8.onnx"


        torch.onnx.export(
            model, 
            img, 
            model_fp32,
            input_names=["input"],
            output_names=["output"],
            opset_version=17,  # 建議最新支持版本
            dynamic_axes={"input": {0: "batch_size"}, "output": {0: "batch_size"}},
            external_data=False
        )
        
        # 優化 ONNX 模型
        model = onnx.load(model_fp32)
        passes = onnxoptimizer.get_fuse_and_elimination_passes()
        optimized_model = onnxoptimizer.optimize(model, passes)
        onnx.save(optimized_model, model_fp32_opt)



        # 動態量化
        quantize_dynamic(
            model_input=model_fp32,
            model_output=model_int8,
            weight_type=QuantType.QInt8,   # 將權重轉為 int8
        )

        print("Quantization done. Saved to:", model_int8)


    main()