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
# from torchmetrics.image import StructuralSimilarityIndexMeasure

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from isp_ai.ToneMappedL1SSIMLoss import ToneMappedL1SSIMLoss
from isp_ai.unet import UNet
from common.PreprocessedCropDataset import PreprocessedCropDataset
from common.BayerDataset import BayerDataset
from common.common_func import *
import cv2
from onnx import numpy_helper
import gc
from onnxconverter_common import float16


device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
# l1_loss = nn.L1Loss()
# # ssim = StructuralSimilarityIndexMeasure(data_range=1.0).to(device)

# def l1_ssim_loss(output, target, alpha=0.84):
#     output = output.float()
#     target = target.float()
#     # SSIM 值在 [0,1] 越大越相似
#     ssim_val = ssim(output, target)
#     return alpha * (1 - ssim_val) + (1 - alpha) * l1_loss(output, target)

# def l1_ssim_brightness_weighted(output, target, alpha=0.84):
#     output = output.float()
#     target = target.float()
#     brightness_weight = torch.clamp(target.mean(dim=1, keepdim=True) * 2.0, 0, 1)
#     ssim_val = ssim(output, target)
#     l1_val = l1_loss(output, target)
#     return alpha * (1 - ssim_val) + (1 - alpha) * (l1_val * brightness_weight).mean()


def main():
    train_dataset   = PreprocessedCropDataset("./data/raw/Sony/Train_Crops", mmap_mode="r")
    val_dataset = PreprocessedCropDataset("./data/raw/Sony/Val_Crops", mmap_mode="r")

    train_loader = DataLoader(train_dataset, batch_size=12, shuffle=True, num_workers=6, pin_memory=False, persistent_workers=True)
    val_loader = DataLoader(val_dataset, batch_size=12, shuffle=False, num_workers=6, pin_memory=False, persistent_workers=True)

    # 使用 GPU (如果可用)
    print(f"Using device: {device}")

    # 建立模型、損失函數和優化器
    model = UNet().to(device)

    # 💡 建議將 lr 從 1e-5 改為 1e-4
    optimizer = optim.AdamW(model.parameters(), lr=2e-4, weight_decay=1e-5)

    # 💡 加入 Cosine Annealing Scheduler
    num_epochs = 500
    scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=num_epochs, eta_min=1e-6)

    # 初始化 AMP Scaler
    scaler = torch.cuda.amp.GradScaler()

    # criterion = nn.SmoothL1Loss()
    # criterion = l1_ssim_loss
    criterion = ToneMappedL1SSIMLoss(device=device, alpha=0.84)

    # 建立 checkpoints 目錄
    os.makedirs("checkpoints", exist_ok=True)
    writer = SummaryWriter("runs/ISP_Demosaic")
    best_loss = float('inf')
    print_interval = 10  # 每隔多少秒打印一次

    # 從斷點繼續訓練
    ckpt = torch.load("checkpoints/unet_ToneMapped_best269.pth")
    model.load_state_dict(ckpt['model_state_dict'])
    optimizer.load_state_dict(ckpt['optimizer_state_dict'])

    # 設定 start_epoch 
    start_epoch = ckpt['epoch'] + 1
    # start_epoch = 0

    for epoch in range(start_epoch, num_epochs):

        model.train()
        total_loss = 0

        # -------------------------------------------------------------
        # 1. 訓練階段 (Train)
        # -------------------------------------------------------------
        gc.disable() # 💡 進入訓練迴圈前停用自動 GC，避免每個 Batch 交接時卡頓

        last_print = time.time()
        for batch_idx, (short, long) in enumerate(train_loader):
            # short/long shape: B x N x C x H x W
            B, N, C_in, H, W = short.shape
            C_out = long.shape[2]  # RGB target 3ch

            x = short.view(B*N, C_in, H, W).to(device)  # Bayer 4ch
            y = long.view(B*N, C_out, H, W).to(device) # RGB 3ch

            # 重新計算 SSIM 狀態
            # ssim.reset()  # 可加上這行
            
            optimizer.zero_grad()

            with torch.cuda.amp.autocast(): # 開啟 FP16 加速
                out = model(x)
                loss = criterion(out, y)
            
            scaler.scale(loss).backward()   # 💡 scaler 放大梯度並反向傳播
            scaler.step(optimizer)          # 💡 scaler 檢查梯度沒問題後更新權重
            scaler.update()                 # 💡 scaler 動態調整下一階段的放大倍率



            # out = model(x)
            # loss = criterion(out, y)
            # loss.backward()
            # optimizer.step()

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

        # 💡 Epoch 結束後手動清理一次並重新開啟
        gc.collect()
        gc.enable()

        avg_train_loss = total_loss / len(train_loader)
        print(f"\nEpoch [{epoch+1}/{num_epochs}], Train Loss: {avg_train_loss:.4f}")
        writer.add_scalar("Loss/train", avg_train_loss, epoch)



        # -------------------------------------------------------------
        # 2. 驗證階段 (Validation)
        # -------------------------------------------------------------
        gc.disable()  # 💡 進入 Val 迴圈「前」再次關閉

        model.eval()
        val_loss = 0
        with torch.no_grad():
            for short, long in val_loader:
                B, N, C_in, H, W = short.shape
                C_out = long.shape[2]
                
                # 💡 1. 加入 non_blocking=True 加速異步搬移
                x_val = short.view(B * N, C_in, H, W).to(device, non_blocking=True)
                y_val = long.view(B * N, C_out, H, W).to(device, non_blocking=True)
                
                # 💡 2. 推論階段搭配 AMP/autocast 能減半 Val 階段的 VRAM 佔用
                with torch.cuda.amp.autocast():
                    out_val = model(x_val)
                    loss = criterion(out_val, y_val)

                # 💡 3. 確保使用 .item() 提取純純數值，避免殘留計算圖
                val_loss += loss.item()

        avg_val_loss = val_loss / len(val_loader)
        print(f"[Val]   Epoch [{epoch+1}/{num_epochs}], Loss: {avg_val_loss:.4f}")
        writer.add_scalar("Loss/val", avg_val_loss, epoch + 1)

        # 💡 Val 迴圈結束後，手動清理並恢復 GC
        gc.collect()
        gc.enable()

        # 💡 更新學習率
        scheduler.step()
        
        # 保存模型
        if avg_val_loss < best_loss:
            best_loss = avg_val_loss
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict()
            }, f"checkpoints/unet_ToneMapped_best{epoch+1}.pth")
            print(f"Model saved. (unet_ToneMapped_best{epoch+1}.pth)")





if __name__ == "__main__":

    # 訓練
    # ---------------------------------------------------------------------------------
    # main()




    # 優化
    # ---------------------------------------------------------------------------------

    # --- 1. 設定路徑 ---
    ckpt_path = "checkpoints/unet_ToneMapped_best363.pth"
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
            # dynamic_axes={"input": {0: "batch_size"}, "output": {0: "batch_size"}},
            # 💡 將 Height(2) 與 Width(3) 也加入 dynamic_axes
            dynamic_axes={
                "input": {0: "batch_size", 2: "height", 3: "width"},
                "output": {0: "batch_size", 2: "height", 3: "width"},
            },
            external_data=False,
            dynamo=False
        )
        
        # 優化 ONNX 模型
        model = onnx.load(model_fp32)

        # 測試模型可以用 cv.dnn 讀取
        for node in model.graph.node:
            if node.op_type == "Conv":
                print("Node:", node.name)
                for attr in node.attribute:
                    print(attr.name, end=" = ")

                    if attr.type == onnx.AttributeProto.INT:
                        print(attr.i)

                    elif attr.type == onnx.AttributeProto.INTS:
                        print(list(attr.ints))

                    else: print(f" {attr.name} (type={attr.type})")

        try:
            net = cv2.dnn.readNetFromONNX(model_fp32)
            print("OpenCV DNN: PASS")
        except cv2.error as e:
            print("OpenCV DNN: FAIL")
            print(e)

        passes = onnxoptimizer.get_fuse_and_elimination_passes()
        optimized_model = onnxoptimizer.optimize(model, passes)
        onnx.save(optimized_model, model_fp32_opt)


        # 1. 載入優化後的 FP32 模型
        onnx_model = onnx.load(model_fp32_opt)
        # 2. 直接轉換為 FP16 (半精度浮點數)
        model_fp16 = float16.convert_float_to_float16(onnx_model, keep_io_types=True)
        # 3. 儲存 FP16 模型 (副檔名建議標註 _fp16.onnx)
        onnx.save(model_fp16, "model_fp16.onnx")
        print("✅ FP16 模型轉換成功！可以直接丟給 C++ CUDA 執行。")

        # # 動態量化
        # quantize_dynamic(
        #     model_input=model_fp32_opt,
        #     model_output=model_int8,
        #     weight_type=QuantType.QInt8,   # 將權重轉為 int8
        # )

        # print("Quantization done. Saved to:", model_int8)
