import torch
import torch.nn as nn
import torch.nn.functional as F
from torchmetrics.image import StructuralSimilarityIndexMeasure

class ToneMappedL1SSIMLoss(nn.Module):
    def __init__(self, device, alpha=0.84, eps=1e-4):
        super().__init__()
        self.alpha = alpha
        self.eps = eps
        self.l1_loss = nn.L1Loss()
        # SSIM 初始化並搬移至指定 device
        self.ssim = StructuralSimilarityIndexMeasure(data_range=1.0).to(device)

    def forward(self, output, target):
        output = output.float()
        target = target.float()

        # 1. 轉至 Log 空間計算 L1 Loss
        # 效果：放大暗部數值 (如 0.005) 的殘差權重，強迫模型精確還原黑階與暗部細節
        output_log = torch.log(output + self.eps)
        target_log = torch.log(target + self.eps)
        loss_l1 = self.l1_loss(output_log, target_log)

        # 2. 轉至 Gamma 空間 (^0.454 ≈ 1/2.2) 計算 SSIM
        # 效果：符合人眼感知空間 (Perceptual Domain)，不會因為 Linear 空間的高光權重過大而懲罰失真
        output_gamma = torch.pow(torch.clamp(output, min=self.eps), 0.454)
        target_gamma = torch.pow(torch.clamp(target, min=self.eps), 0.454)
        ssim_val = self.ssim(output_gamma, target_gamma)

        # 3. 結合 Loss
        return self.alpha * (1.0 - ssim_val) + (1.0 - self.alpha) * loss_l1