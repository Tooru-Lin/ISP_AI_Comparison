import torch
import time

# ===============================
# 配置參數
# ===============================
tensor_size = (10000, 10000)   # 單個大矩陣
batch = 5                      # 同時運算 tensor 數量
repeat = 10                    # 外層迴圈，持續負載
inner_repeat = 5               # 內層迴圈，矩陣乘法次數

print("=== Sustained GPU Stress Test ===")
print("PyTorch version:", torch.__version__)

# ===============================
# 檢查 CUDA
# ===============================
if not torch.cuda.is_available():
    print("CUDA 不可用，請確認驅動與 PyTorch 安裝正確。")
    exit()

device = torch.device('cuda')
gpu_name = torch.cuda.get_device_name(0)
print(f"GPU: {gpu_name}")
print(f"GPU count: {torch.cuda.device_count()}")

# ===============================
# 開始高負載測試
# ===============================
for r in range(repeat):
    print(f"\n=== iteration {r+1}/{repeat} ===")
    # 建立 batch tensor
    tensors = [torch.randn(tensor_size, device=device) for _ in range(batch)]
    
    start = time.time()
    
    for i in range(inner_repeat):
        results = []
        for t in tensors:
            # 複雜運算：矩陣乘法 + sin
            y = torch.matmul(t, t.t()) + torch.sin(t)
            results.append(y)
        torch.cuda.synchronize()  # 等待 GPU 完成運算
    
    end = time.time()
    print(f"Iteration time: {end - start:.4f} 秒")
    print(f"Batch size: {batch}, Tensor size: {tensor_size}, Inner repeat: {inner_repeat}")
