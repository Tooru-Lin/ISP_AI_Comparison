import torch
import torch.nn as nn
import torch.optim as optim
from tqdm import tqdm
import os
import sys
from torch.utils.data import DataLoader, Dataset
from torchvision import transforms
from torch.utils.tensorboard import SummaryWriter

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from isp_ai.unet import UNet
from data.BayerDataset import BayerDataset  # 你可以自訂這個class

import torch
import torch.nn as nn
import torch.optim as optim
from tqdm import tqdm
import os
import sys
from torch.utils.data import DataLoader, Dataset
from torchvision import transforms
from torch.utils.tensorboard import SummaryWriter

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from isp_ai.unet import UNet
from data.BayerDataset import BayerDataset  # 你可以自訂這個class

def main():
    # 1️⃣ Dataset
    train_dataset = BayerDataset("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony_train_list.txt")
    val_dataset   = BayerDataset("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony_val_list.txt")

    # 2️⃣ DataLoader
    train_loader = DataLoader(train_dataset, batch_size=4, shuffle=True, num_workers=4, pin_memory=True)
    val_loader   = DataLoader(val_dataset, batch_size=4, shuffle=False, num_workers=4, pin_memory=True)

    # 使用 GPU (如果可用)
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")

    # 建立模型、損失函數和優化器
    model = UNet().to(device)
    optimizer = optim.Adam(model.parameters(), lr=1e-4, weight_decay=1e-5)
    criterion = nn.SmoothL1Loss()

    num_epochs = 200
    writer = SummaryWriter("runs/ISP_Demosaic")

    for epoch in range(num_epochs):
        model.train()
        total_loss = 0

        for short, long in train_loader:
            # short/long shape: B x N x C x H x W
            B, N, C_in, H, W = short.shape
            C_out = long.shape[2]  # RGB target 3ch

            x = short.view(B*N, C_in, H, W).to(device)  # Bayer 4ch
            y = long.view(B*N, C_out, H, W).to(device) # RGB 3ch

            optimizer.zero_grad()
            out = model(x)
            loss = criterion(out, y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()

        avg_train_loss = total_loss / len(train_loader)
        print(f"Epoch [{epoch+1}/{num_epochs}], Train Loss: {avg_train_loss:.4f}")
        writer.add_scalar("Loss/train", avg_train_loss, epoch)

        # 驗證
        model.eval()
        val_loss = 0
        with torch.no_grad():
            for short, long in val_loader:
                B, N, C, H, W = short.shape
                x_val = short.view(B*N, C, H, W).to(device)
                y_val = long.view(B*N, C, H, W).to(device)
                out_val = model(x_val)
                val_loss += criterion(out_val, y_val).item()

        avg_val_loss = val_loss / len(val_loader)
        print(f"[Val]   Epoch [{epoch+1}/{num_epochs}], Loss: {avg_val_loss:.4f}")

        # 保存模型
        if (epoch + 1) % 10 == 0:
            os.makedirs("checkpoints", exist_ok=True)
            torch.save(model.state_dict(), f"checkpoints/unet_epoch{epoch+1}.pth")
            print("Model saved.")


if __name__ == "__main__":
    main()