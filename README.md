# 專案名稱
ISP_AI_Comparison

---

## 📌 開發環境
- 作業系統：Windows
- 程式語言：C++, C#, python 

---

## 🗂 版本更新紀錄
| 版號 | 更新內容 | 日期 |
|:--------|:------------|------:|
| v0.0.6.0902 | 1. 將Int8_Model改為Fp16改善計算速度。<br>2. 修正Cudnn版本使其使用GPU運行。<br> | 2026/09/02 |
| v0.0.5.0829 | 1. 重新使用加入Gamma後的Loss訓練模型。<br>2. 使用onnxruntime取代opencv_dnn修正無法跑int8_Model問題。<br>3. 加入使用Cuda_13.3.1和Cudnn_9.25.0環境。 | 2026/08/29 |
| v0.0.4.0820 | 1. 可成功套用ONNX模型。<br>2. 修正 Controller 套用錯誤的 ComboBox 參數問題。 | 2026/08/20 |
| v0.0.3.0308 | 1. 新增 Controller 控制算法和參數。<br>2. UI更新至可以切換None, Default。 | 2026/03/08 |
| v0.0.2.0222 | 1. 將 Traditional Algo 包成 DLL。<br>2. 將 Traditional Algo DLL 初步整合至 UI 並測試。 | 2026/02/22 |
| v0.0.1.1214 | 1. 初始版本。 | 2025/12/14 |