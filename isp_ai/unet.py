import torch
import torch.nn as nn
import torch.nn.functional as F

class UNet(nn.Module):
    def __init__(self, in_channels=4, out_channels=3):
        super().__init__()

        # 將 BatchNorm 改為 InstanceNorm，更適合圖像重建/ISP 任務
        def CIR(in_ch, out_ch):
            return nn.Sequential(
                nn.Conv2d(in_ch, out_ch, 3, padding=1),
                nn.InstanceNorm2d(out_ch, affine=True),
                nn.LeakyReLU(0.2, inplace=True)
            )

        self.enc1 = CIR(in_channels, 32)
        self.enc2 = CIR(32, 64)
        self.enc3 = CIR(64, 128)
        self.enc4 = CIR(128, 256)
        self.bottom = CIR(256, 512)

        self.up1 = nn.ConvTranspose2d(512, 256, 2, stride=2)
        self.dec1 = CIR(512, 256)
        self.up2 = nn.ConvTranspose2d(256, 128, 2, stride=2)
        self.dec2 = CIR(256, 128)
        self.up3 = nn.ConvTranspose2d(128, 64, 2, stride=2)
        self.dec3 = CIR(128, 64)
        self.up4 = nn.ConvTranspose2d(64, 32, 2, stride=2)
        self.dec4 = CIR(64, 32)

        self.outc = nn.Conv2d(32, out_channels, 1)

    def forward(self, x):
        e1 = self.enc1(x)
        e2 = self.enc2(F.max_pool2d(e1, 2))
        e3 = self.enc3(F.max_pool2d(e2, 2))
        e4 = self.enc4(F.max_pool2d(e3, 2))
        b = self.bottom(F.max_pool2d(e4, 2))

        d1 = self.dec1(torch.cat([self.up1(b), e4], dim=1))
        d2 = self.dec2(torch.cat([self.up2(d1), e3], dim=1))
        d3 = self.dec3(torch.cat([self.up3(d2), e2], dim=1))
        d4 = self.dec4(torch.cat([self.up4(d3), e1], dim=1))

        out = self.outc(d4)
        
        # 使用 clamp 確保數值在 [0, 1]，不會有 Sigmoid 的梯度飽和問題
        return torch.clamp(out, 0.0, 1.0)
