#include "isp.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <libraw/libraw.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <functional>
#include <filesystem>

// 在 includes 之後加入（靠近檔案頂端）
static void CalLowHigh(const cv::Mat& img, double& lowVal, double& highVal);
// 取前幾大均值
static double getBiggestMean(const cv::Mat& channel, double ratio = 0.05);

// ========================================
// 功能：從 RAW 檔案加載影像並提取元數據
// 輸入參數：
//   - filename: RAW 檔案路徑
// 輸出參數：
//   - width, height: 影像尺寸
//   - black, white: 黑階、白階
//   - cam_mul, pre_mul: AWB 增益係數
//   - cam_xyz: 相機 RGB→XYZ 矩陣
//   - xyz2srgb: XYZ→sRGB 矩陣
//   - raw32: 轉換後的 32-bit float 原始影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::FileOpenFailed 檔案開啟失敗
//   - ErrCode::UnpackFailed 解包失敗
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::loadRawWithLibRaw(
    const std::string& filename,
    int& width,
    int& height,
    int& black,                      // 輸出黑階
    int& white,                      // 輸出白階
    std::vector<float>& cam_mul,     // 輸出 AWB gains
    std::vector<float>& pre_mul,     // 輸出 AWB gains
    cv::Mat& cam_xyz,                // 輸出 3x3 相機→XYZ 矩陣
    cv::Mat& xyz2srgb,               // 輸出 3x3 XYZ→sRGB 矩陣
    cv::Mat& raw32                   // 輸出 raw 32F
)
{
    try {
        // 1. 建立 LibRaw 處理器
        LibRaw RawProcessor;
        int ret = RawProcessor.open_file(filename.c_str());
        if (ret != LIBRAW_SUCCESS) {
            std::cerr << "Cannot open file: " << filename << " Error: " << libraw_strerror(ret) << std::endl;
            return ErrCode::FileOpenFailed;
        }

        // 2. 解包 RAW 數據
        ret = RawProcessor.unpack();
        if (ret != LIBRAW_SUCCESS) {
            std::cerr << "Cannot unpack raw data: " << libraw_strerror(ret) << std::endl;
            return ErrCode::UnpackFailed;
        }

        // 3. 讀取並打印 CFA 資訊
        std::cout << "CFA description: "
            << RawProcessor.imgdata.idata.cdesc << std::endl;

        int pattern = RawProcessor.imgdata.idata.filters;
        std::cout << "CFA pattern: " << RawProcessor.imgdata.idata.cdesc << std::endl;

        std::cout << "CFA 2x2 pattern:" << std::endl;
        libraw_data_t* data = &RawProcessor.imgdata;
        for (int y = 0; y < 2; y++) {
            for (int x = 0; x < 2; x++) {
                int idx = libraw_COLOR(data, x, y);
                std::cout << RawProcessor.imgdata.idata.cdesc[idx] << "("
                    << data->rawdata.raw_image[y * data->sizes.width + x] << ") ";
            }
            std::cout << std::endl;
        }

        // 4. 讀取影像尺寸
        int raw_width = RawProcessor.imgdata.sizes.raw_width;
        int raw_height = RawProcessor.imgdata.sizes.raw_height;
        width = RawProcessor.imgdata.sizes.width;  // 有效寬度
        height = RawProcessor.imgdata.sizes.height;  // 有效高度

        int left = (raw_width - width) / 2;  // 開始列
        int top = (raw_height - height) / 2;  // 開始行

        // 5. 創建 16-bit Mat 並拷貝有效區域
        cv::Mat raw16(height, width, CV_16U);
        for (int y = 0; y < height; y++) {
            memcpy(raw16.ptr<ushort>(y),
                RawProcessor.imgdata.rawdata.raw_image + (y + top) * raw_width + left,
                width * sizeof(ushort));
        }

        // 6. 讀取 metadata (黑階、白階、WB係數...)
        libraw_data_t* raw = &RawProcessor.imgdata;
        black = raw->color.black;
        white = raw->color.maximum;

        // 7. 提取白平衡係數
        cam_mul.resize(4);
        pre_mul.resize(4);
        for (int i = 0; i < 4; i++) {
            cam_mul[i] = raw->color.cam_mul[i];
            pre_mul[i] = raw->color.pre_mul[i];
        }

        // 8. 提取相機 RGB → XYZ 矩陣 (3x3)
        cam_xyz = cv::Mat(3, 3, CV_32F);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cam_xyz.at<float>(i, j) = raw->color.cam_xyz[i][j];
            }
        }

        // 9. 設定 XYZ → sRGB 矩陣
        xyz2srgb = (cv::Mat_<float>(3, 3) <<
            3.2406, -1.5372, -0.4986,
            -0.9689, 1.8758, 0.0415,
            0.0557, -0.2040, 1.0570);

        // 10. 防呆檢查
        if (raw16.empty()) {
            std::cerr << "Failed to load image!" << std::endl;
            return ErrCode::EmptyImage;
        }

        // 11. 轉換為 CV_32F 格式
        raw16.convertTo(raw32, CV_32F);
        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in loadRawWithLibRaw: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in loadRawWithLibRaw" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：套用 pre_mul 白平衡係數到影像
// 輸入參數：
//   - img: 輸入影像 (CV_32FC3)
//   - pre_mul: 白平衡係數向量 [B, G, R]
// 輸出參數：
//   - img: 修改後的影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::InvalidInput 輸入格式不支援
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::applyPreMul(cv::Mat& img, const std::vector<float> pre_mul) {
    try {
        // 防呆：檢查輸入格式
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }
        if (img.type() != CV_32FC3) {
            return ErrCode::InvalidInput;
        }
        if (pre_mul.size() != 3) {
            return ErrCode::InvalidInput;
        }

        // 拆成通道
        std::vector<cv::Mat> channels(3);
        cv::split(img, channels);

        // 套用係數
        channels[0] *= pre_mul[0]; // B
        channels[1] *= pre_mul[1]; // G
        channels[2] *= pre_mul[2]; // R

        cv::merge(channels, img);
        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in applyPreMul: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in applyPreMul" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：黑白電平校正，將影像正規化到 [0, 1] 範圍
// 輸入參數：
//   - raw: 原始影像（會被修改）
//   - black_level: 黑電平
//   - white_level: 白電平
// 輸出參數：
//   - raw: 修改後的正規化影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::BlackAndWhiteLevelCorrection(cv::Mat& raw, float black_level, float white_level) {
    try {
        // 防呆：檢查影像是否為空
        if (raw.empty()) {
            return ErrCode::EmptyImage;
        }

        // 1. 先扣黑電平
        raw -= black_level;

        // 2. 轉 float 以進行除法
        raw.convertTo(raw, CV_32F);

        // 3. 再除以 (white_level - black_level) 以 normalize 到 0~1
        raw = raw / (white_level - black_level);

        // 4. clip 到 0~1 範圍
        cv::threshold(raw, raw, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw, raw, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in BlackAndWhiteLevelCorrection: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in BlackAndWhiteLevelCorrection" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：白平衡增益計算 - Gray World 方法
// 說明：假設場景平均顏色為灰色，計算使各通道平均值相等的增益
// 輸入參數：
//   - img: BGR 彩色影像 (CV_32F)
// 輸出參數：
//   - gain_R, gain_G, gain_B: 計算出的 RGB 增益
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::InvalidInput 影像格式不支援
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::CalAWBGain_GrayWorld(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B) {
    try {
        // 防呆：檢查輸入
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }
        if (img.depth() != CV_32F || img.channels() != 3) {
            return ErrCode::InvalidInput;
        }

        // 複製並確保為 CV_32F
        cv::Mat img32F;
        img.convertTo(img32F, CV_32F);

        // 拆成通道
        std::vector<cv::Mat> channels(3);
        cv::split(img32F, channels);

        // 計算各通道平均值
        double meanR = cv::mean(channels[2])[0];
        double meanG = cv::mean(channels[1])[0];
        double meanB = cv::mean(channels[0])[0];
        double meanGray = (meanR + meanG + meanB) / 3.0;

        // 計算增益
        double eps = 1e-6;
        gain_R = meanGray / (meanR + eps);
        gain_G = meanGray / (meanG + eps);
        gain_B = meanGray / (meanB + eps);

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in CalAWBGain_GrayWorld: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in CalAWBGain_GrayWorld" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：白平衡增益計算 - White Patch 方法
// 說明：找影像中最亮的區域作為白點參考，計算增益
// 輸入參數：
//   - img: BGR 彩色影像 (CV_32F)
// 輸出參數：
//   - gain_R, gain_G, gain_B: 計算出的 RGB 增益
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::InvalidInput 影像格式不支援
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::CalAWBGain_WhitePatch(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B) {
    try {
        // 防呆：檢查輸入
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }
        if (img.depth() != CV_32F || img.channels() != 3) {
            return ErrCode::InvalidInput;
        }

        // 拆成通道
        std::vector<cv::Mat> channels(3);
        cv::split(img, channels);

        // 取前 5% 最亮像素的平均值
        double R_mean = getBiggestMean(channels[2]);
        double G_mean = getBiggestMean(channels[1]);
        double B_mean = getBiggestMean(channels[0]);

        // G 作為參考
        gain_R = G_mean / R_mean;
        gain_G = 1.0;
        gain_B = G_mean / B_mean;

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in CalAWBGain_WhitePatch: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in CalAWBGain_WhitePatch" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：套用白平衡增益到原始 Bayer 影像
// 說明：根據 Bayer 排列模式 (RGGB) 在對應位置施加增益
// 輸入參數：
//   - raw32: 原始 Bayer 影像 (CV_32F single-channel)
//   - height, width: 影像尺寸
//   - gainR, gainG, gainB: RGB 增益值
// 輸出參數：
//   - raw32: 修改後的影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::ApplyAWBGain(cv::Mat& raw32, int height, int width, double gainR, double gainG, double gainB) {
    try {
        // 防呆：檢查影像
        if (raw32.empty() || height <= 0 || width <= 0) {
            return ErrCode::InvalidInput;
        }

        // 建立增益對應表 (Bayer RGGB 模式)
        std::vector<float> cam_mul_normalized(4);
        cam_mul_normalized[0] = gainR; // R (y=0, x=0)
        cam_mul_normalized[1] = gainG; // G (y=0, x=1)
        cam_mul_normalized[2] = gainG; // G (y=1, x=0)
        cam_mul_normalized[3] = gainB; // B (y=1, x=1)

        // 套用 AWB 增益：根據像素位置在 2x2 Bayer 模式中的位置選擇增益
        for (int y = 0; y < height; y++) {
            float* row = raw32.ptr<float>(y);
            for (int x = 0; x < width; x++) {
                int idx = ((y & 1) << 1) | (x & 1);
                row[x] *= cam_mul_normalized[idx];
            }
        }

        // 4. clip 到 0~1 範圍
        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ApplyAWBGain: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in ApplyAWBGain" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：Bayer 去馬賽克 (Demosaic)
// 說明：將 Bayer CFA 影像轉換為 BGR 彩色影像
// 步驟：
//   1. 轉換輸入為 float 0~1 範圍
//   2. 轉 16-bit (OpenCV cvtColor 不支援 float Bayer)
//   3. 執行 demosaic 操作 (COLOR_BayerBG2BGR)
//   4. 轉回 float 並 clip 到 0~1
// 輸入參數：
//   - raw: Bayer 原始影像 (CV_16U 或 CV_32F single-channel)
// 輸出參數：
//   - out_bgr32: 輸出 BGR 影像 (CV_32F 3-channel)
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::InvalidInput 輸入格式不支援
//   - ErrCode::DemosaicFailed demosaic 過程失敗
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::demosaic(const cv::Mat& rawIn, cv::Mat& out_bgr32) {
    try {
        // 防呆：檢查輸入
        if (rawIn.empty()) {
            return ErrCode::EmptyImage;
        }

        cv::Mat raw32;

        // 1. 轉 float 0~1
        if (rawIn.type() == CV_16U) {
            rawIn.convertTo(raw32, CV_32F, 1.0 / 65535.0);
        }
        else if (rawIn.type() == CV_32F) {
            raw32 = rawIn.clone();
        }
        else {
            // 不支援的輸入格式
            return ErrCode::InvalidInput;
        }

        // 2. clip <0 (防止負值)
        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO);

        // 3. clip >1 (防止過大)
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);

        // 4. 先轉成 CV_16U 避免 cvtColor float crash
        cv::Mat raw16;
        raw32.convertTo(raw16, CV_16U, 65535.0);

        // 5. 執行 demosaic
        cv::Mat bgr16;
        cv::cvtColor(raw16, bgr16, cv::COLOR_BayerBG2BGR); // Bayer pattern 視 sensor 而定

        if (bgr16.empty()) {
            return ErrCode::DemosaicFailed;
        }

        // 6. 轉回 float 0~1
        cv::Mat bgr32;
        bgr16.convertTo(bgr32, CV_32F, 1.0 / 65535.0);

        // 7. clip 到 0~1 範圍 (確保輸出數值正常)
        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        out_bgr32 = bgr32;
        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in demosaic: " << ex.what() << std::endl;
        return ErrCode::DemosaicFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in demosaic" << std::endl;
        return ErrCode::Unknown;
    }
}


// 設定 / 載入 ONNX 模型
ISP::ErrCode ISP::SetAiDemosaicModel(const char* modelPath)
{
    try {
        if (modelPath == nullptr) return ErrCode::InvalidInput;

        std::string path(modelPath);
        std::ifstream f(path.c_str());
        if (!f.good()) {
            std::cerr << "Model file not found: " << path << std::endl;
            return ErrCode::InvalidInput;
        }
        f.close();

        // 嘗試用 OpenCV DNN 載入 ONNX 模型
        cv::dnn::Net net;
        try {
            net = cv::dnn::readNetFromONNX(path);
        }
        catch (const cv::Exception& e) {
            std::cerr << "Failed to read ONNX model: " << e.what() << std::endl;
            return ErrCode::InvalidInput;
        }

        // 設定 backend / target，優先使用 CPU（可視需要改為 CUDA）
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        // 儲存到物件
        ai_net = net;
        ai_model_path = path;

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in SetAiDemosaicModel: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in SetAiDemosaicModel" << std::endl;
        return ErrCode::Unknown;
    }
}

// 用指定 modelPath 直接推論（單次）
ISP::ErrCode ISP::AiDemosaicWithModel(cv::Mat& raw, cv::Mat& out_bgr32, const char* modelPath)
{
    try {
        // 載入模型（若失敗會回傳錯誤）
        ErrCode ec = SetAiDemosaicModel(modelPath);
        if (ec != ErrCode::Ok) return ec;

        // 呼叫已載入的 AiDemosaic
        return AiDemosaic(raw, out_bgr32);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in AiDemosaicWithModel: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in AiDemosaicWithModel" << std::endl;
        return ErrCode::Unknown;
    }
}

// 使用已載入模型進行推論
ISP::ErrCode ISP::AiDemosaic(cv::Mat& raw, cv::Mat& out_bgr32)
{
    try {
        if (raw.empty()) return ErrCode::EmptyImage;

        if (ai_net.empty()) {
            std::cerr << "AI model not loaded. Call SetAiDemosaicModel first." << std::endl;
            return ErrCode::InvalidInput;
        }

        // 確保 raw 為 single-channel 或 3-channel float (0..1)
        cv::Mat inputMat;
        if (raw.type() == CV_32F) {
            inputMat = raw;
        }
        else if (raw.type() == CV_16U) {
            raw.convertTo(inputMat, CV_32F, 1.0 / 65535.0);
        }
        else if (raw.type() == CV_8U) {
            raw.convertTo(inputMat, CV_32F, 1.0 / 255.0);
        }
        else {
            return ErrCode::InvalidInput;
        }

        // 偵錯輸出：印出輸入尺寸與通道數，確保尺寸正確
        std::cerr << "AiDemosaic: inputMat size = " << inputMat.cols << "x" << inputMat.rows
            << ", channels = " << inputMat.channels() << ", type = " << inputMat.type() << std::endl;

        // 若輸入為 single-channel，且看起來像 Bayer raw（常見情況），將拆成 4 通道 (H/2, W/2, 4)
        // 檢查是否為 single-channel 與偶數尺寸（Bayer 需偶數高寬）
        cv::Mat modelInput;
        if (inputMat.channels() == 1) {
            int H = inputMat.rows;
            int W = inputMat.cols;

            if ((H % 2) != 0 || (W % 2) != 0) {
                std::cerr << "AiDemosaic: Bayer raw dimensions must be even. H=" << H << " W=" << W << std::endl;
                return ErrCode::InvalidInput;
            }

            int h2 = H / 2;
            int w2 = W / 2;

            // 建立四個子通道 (R, G1, G2, B)
            cv::Mat chR(h2, w2, CV_32F);
            cv::Mat chG1(h2, w2, CV_32F);
            cv::Mat chG2(h2, w2, CV_32F);
            cv::Mat chB(h2, w2, CV_32F);

            // 填入子通道：R(y=0,x=0), G1(y=0,x=1), G2(y=1,x=0), B(y=1,x=1)
            for (int y = 0; y < h2; ++y) {
                const float* row0 = inputMat.ptr<float>(y * 2);
                const float* row1 = inputMat.ptr<float>(y * 2 + 1);
                float* pR = chR.ptr<float>(y);
                float* pG1 = chG1.ptr<float>(y);
                float* pG2 = chG2.ptr<float>(y);
                float* pB = chB.ptr<float>(y);
                for (int x = 0; x < w2; ++x) {
                    pR[x] = row0[x * 2];       // (0,0)
                    pG1[x] = row0[x * 2 + 1];   // (0,1)
                    pG2[x] = row1[x * 2];       // (1,0)
                    pB[x] = row1[x * 2 + 1];   // (1,1)
                }
            }

            // Clip 到 0..1 - 使用 cv::threshold（與專案其他地方一致，避免 std::min/std::max 衝突）
            cv::threshold(chR, chR, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chR, chR, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chG1, chG1, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chG1, chG1, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chG2, chG2, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chG2, chG2, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chB, chB, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chB, chB, 1.0, 1.0, cv::THRESH_TRUNC);


            // 合成 4 通道影像 (h2, w2, 4)
            std::vector<cv::Mat> raw4ch_vec;
            raw4ch_vec.push_back(chR);
            raw4ch_vec.push_back(chG1);
            raw4ch_vec.push_back(chG2);
            raw4ch_vec.push_back(chB);
            cv::Mat raw4ch;
            cv::merge(raw4ch_vec, raw4ch); // raw4ch: H/2 x W/2 x 4

            // 顯示組成後尺寸供偵錯
            std::cerr << "AiDemosaic: assembled raw4ch size = " << raw4ch.cols << "x" << raw4ch.rows
                << ", channels = " << raw4ch.channels() << std::endl;

            modelInput = raw4ch; // 作為模型輸入
        }
        else {
            // 若輸入已經是多通道（例如已去馬賽克或其他格式），直接以原輸入為模型輸入
            modelInput = inputMat;
        }



        // 如果是多通道但模型期望 single-channel，可合併或取第一通道（此處保留原始 channel）
        // 建立 blob：保持原始大小，no mean/subtract
        cv::Mat blob = cv::dnn::blobFromImage(modelInput, 1.0, modelInput.size(), cv::Scalar(), false, false);


        // 設定輸入並 forward
        ai_net.setInput(blob);
        cv::Mat outBlob;
        try {
            outBlob = ai_net.forward();
        }
        catch (const cv::Exception& e) {
            std::cerr << "ONNX forward failed: " << e.what() << std::endl;
            return ErrCode::DemosaicFailed;
        }

        // outBlob 可能形狀為 1 x C x H x W 或 1 x H x W x C 等。處理常見 4D (NCHW)
        if (outBlob.dims != 4) {
            // 嘗試 reshape 若可能
            std::cerr << "Unexpected output dimensions from model: " << outBlob.dims << std::endl;
            return ErrCode::DemosaicFailed;
        }

        int n = outBlob.size[0];
        int c = outBlob.size[1];
        int h = outBlob.size[2];
        int w = outBlob.size[3];

        if (n < 1) return ErrCode::DemosaicFailed;
        if (c != 3 && c != 1) {
            std::cerr << "Model output channels not 1 or 3: " << c << std::endl;
            return ErrCode::DemosaicFailed;
        }

        // 轉換成 HxWxC CV_32F
        std::vector<cv::Mat> channels;
        channels.reserve(c);
        // outBlob is NCHW float; extract each channel
        for (int i = 0; i < c; ++i) {
            // create Mat header pointing to data for channel i
            cv::Mat ch(h, w, CV_32F, outBlob.ptr(0, i));
            channels.push_back(ch.clone()); // clone to own memory
        }

        cv::Mat merged;
        if (c == 1) {
            // single channel -> replicate to 3 channels (灰階)
            cv::Mat gray = channels[0];
            cv::Mat bgr;
            cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
            merged = bgr;
        }
        else {
            // c==3 : typical model ordered as BGR or RGB unknown; assume output is RGB -> convert to BGR
            // Many ONNX models output RGB; here attempt to detect reasonable range: if values in [0,1], leave
            cv::Mat m;
            cv::merge(channels, m); // m is HxWx3 with channel order [ch0,ch1,ch2]
            // Try to detect if model produced RGB: we assume RGB -> convert to BGR
            cv::cvtColor(m, merged, cv::COLOR_RGB2BGR);
        }

        // 確保 output 為 CV_32F 及範圍 0..1；若必要可 clip
        cv::threshold(merged, merged, 0.0, 0.0, cv::THRESH_TOZERO);
        cv::threshold(merged, merged, 1.0, 1.0, cv::THRESH_TRUNC);

        out_bgr32 = merged;
        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in AiDemosaic: " << ex.what() << std::endl;
        return ErrCode::Exception;
    }
    catch (...) {
        std::cerr << "Unknown exception in AiDemosaic" << std::endl;
        return ErrCode::Unknown;
    }
}


// ========================================
// 功能：色彩校正 (Color Correction Matrix)
// 說明：使用 CCM 矩陣將相機 RGB 色彩空間轉換到標準 sRGB 色彩空間
// 步驟：
//   1. BGR -> RGB 轉換
//   2. 套用 CCM 矩陣
//   3. RGB -> BGR 轉換 (回到 OpenCV BGR 格式)
//   4. Clip 到 0~1 範圍
// 輸入參數：
//   - img: BGR 彩色影像 (CV_32F 3-channel)
//   - ccm: 色彩校正矩陣 (3x3 float)
// 輸出參數：
//   - out: 色彩校正後的影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像或 CCM 為空
//   - ErrCode::InvalidInput 輸入格式不支援
//   - ErrCode::ColorCorrectionFailed 色彩校正失敗
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::colorCorrection(const cv::Mat& img, const cv::Mat& ccm, cv::Mat& out) {
    try {
        // 防呆：檢查輸入
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }
        if (ccm.empty() || ccm.rows != 3 || ccm.cols != 3) {
            return ErrCode::InvalidInput;
        }
        if (img.channels() != 3) {
            return ErrCode::InvalidInput;
        }

        // 不修改原輸入，先複製到工作 Mat
        cv::Mat tmp;
        img.convertTo(tmp, CV_32F);

        // 1. BGR -> RGB
        cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);

        // 2. 套用 CCM 矩陣轉換
        cv::transform(tmp, tmp, ccm);

        // 3. RGB -> BGR (回到 OpenCV 格式)
        cv::cvtColor(tmp, tmp, cv::COLOR_RGB2BGR);

        // 4. clip 到 0~1 (防止溢位)
        cv::threshold(tmp, tmp, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(tmp, tmp, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        out = tmp;
        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in colorCorrection: " << ex.what() << std::endl;
        return ErrCode::ColorCorrectionFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in colorCorrection" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：色調映射與 Gamma 校正
// 說明：將影像應用 gamma 校正以調整亮度和對比度
// 步驟：
//   1. 檢查並清理無效值 (負值、NaN、過大值)
//   2. 套用 Gamma 校正 (pow(img, 1/gamma))
//   3. Clip 到 0~1 範圍
// 輸入參數：
//   - img: 輸入影像 (會被修改)
//   - gamma: Gamma 值 (預設 1.0)
// 輸出參數：
//   - img: 修改後的影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::ToneMappingFailed 色調映射失敗
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::applyToneMapping(cv::Mat& img_float, float gamma) {
    try {
        // 防呆：檢查輸入
        if (img_float.empty()) {
            return ErrCode::EmptyImage;
        }
        if (gamma <= 0.0f) {
            return ErrCode::InvalidInput;
        }

        cv::Mat tmp = img_float.clone();

        // 1. 拆成通道並清理無效值
        std::vector<cv::Mat> channels;
        cv::split(tmp, channels);

        for (auto& c : channels) {
            // 負值設 0 (防止負數影響 gamma 計算)
            c.setTo(0, c < 0);

            // 避免 NaN
            cv::patchNaNs(c, 0.0);

            // 避免過大 (防止浮點溢位)
            cv::threshold(c, c, 1e6, 1e6, cv::THRESH_TRUNC);
        }
        cv::merge(channels, tmp);

        // 2. 套用 Gamma 校正
        cv::pow(tmp, 1.0 / gamma, img_float);

        // 3. Clip 到 0~1 範圍
        cv::threshold(img_float, img_float, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(img_float, img_float, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in applyToneMapping: " << ex.what() << std::endl;
        return ErrCode::ToneMappingFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in applyToneMapping" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：影像銳化
// 說明：使用 Unsharp Mask 技術進行銳化
// 公式：output = img + (img - blurred) * level
// 輸入參數：
//   - img: 輸入影像 (會被修改)
//   - Sharpening_Level: 銳化強度 (預設 0)
// 輸出參數：
//   - img: 修改後的影像
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::SharpeningFailed 銳化失敗
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::sharpening(cv::Mat& img, double Sharpening_Level) {
    try {
        // 防呆：檢查輸入
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }

        // 1. 建立高斯模糊版本
        cv::Mat blurred;
        cv::GaussianBlur(img, blurred, cv::Size(0, 0), 3);

        // 2. 計算差值並疊加 (Unsharp Mask)
        cv::addWeighted(img, 1 + Sharpening_Level, blurred, -Sharpening_Level, 0, img);

        // 3. Clip 到 0~1 範圍
        cv::threshold(img, img, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(img, img, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in sharpening: " << ex.what() << std::endl;
        return ErrCode::SharpeningFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in sharpening" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 功能：預覽與輸出影像
// 說明：將影像轉為 8-bit 並顯示和儲存
// 支援的輸入格式：CV_8U, CV_16U, CV_32F, CV_32S
// 輸入參數：
//   - img: 輸入影像
//   - title: 視窗標題與檔案名稱前綴
//   - scale: 縮放比例 (預設 1.0)
//   - autoContrast: 是否自動對比度調整 (預設 true)
// 回傳：
//   - ErrCode::Ok 成功
//   - ErrCode::EmptyImage 影像為空
//   - ErrCode::InvalidInput 不支援的影像深度
//   - ErrCode::PreviewFailed 預覽失敗
//   - ErrCode::Exception 例外發生
//   - ErrCode::Unknown 未知錯誤
// ========================================
ISP::ErrCode ISP::showPreview(const cv::Mat& img, const std::string& title, double scale, bool autoContrast) {
    try {
        // 防呆：檢查輸入
        if (img.empty()) {
            return ErrCode::EmptyImage;
        }

        cv::Mat preview8;

        // 1. 根據輸入深度進行轉換
        if (img.depth() == CV_8U) {
            // 8-bit 直接使用
            preview8 = img.clone();
        }
        else if (img.depth() == CV_16U) {
            // 16-bit → 8-bit
            if (autoContrast) {
                cv::normalize(img, preview8, 0, 255, cv::NORM_MINMAX, CV_8U);
            }
            else {
                img.convertTo(preview8, CV_8U, 1.0 / 256.0);
            }
        }
        else if (img.depth() == CV_32F || img.depth() == CV_32S) {
            // 32-bit → 8-bit
            if (autoContrast) {
                cv::Mat tmp = img.clone();

                // 計算 1% 到 99% 的範圍用於對比度調整
                double minVal, maxVal;
                CalLowHigh(tmp, minVal, maxVal);

                tmp = (tmp - minVal) / (maxVal - minVal);  // 正規化到 0~1
                tmp.convertTo(preview8, CV_8U, 255.0);
            }
            else {
                img.convertTo(preview8, CV_8U, 255.0); // 假設輸入在 [0,1] 範圍
            }
        }
        else {
            // 不支援的深度
            return ErrCode::InvalidInput;
        }

        // 2. 根據 scale 進行縮放
        if (scale != 1.0) {
            cv::Mat temp;
            cv::resize(preview8, temp, cv::Size(), scale, scale, cv::INTER_AREA);
            preview8 = temp;
        }

        // 3. 儲存為檔案
        cv::imwrite(std::to_string(ImgCount++) + "_" + title + ".tiff", preview8);

        // 4. 顯示影像
        cv::imshow(title, preview8);
        cv::waitKey(0);

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in showPreview: " << ex.what() << std::endl;
        return ErrCode::PreviewFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in showPreview" << std::endl;
        return ErrCode::Unknown;
    }
}

// ========================================
// 輔助函數：計算影像的低值和高值（1% 和 99% 分位點）
// ========================================
static void CalLowHigh(const cv::Mat& img, double& lowVal, double& highVal)
{
    // 初始化輸出
    lowVal = 0.0;
    highVal = 0.0;

    if (img.empty()) return;

    // 使用 single-channel 視圖（避免 channel 計數混淆）
    cv::Mat single = (img.channels() == 1) ? img : img.reshape(1);

    double minValD = 0.0, maxValD = 0.0;
    cv::minMaxLoc(single, &minValD, &maxValD);

    float minF = static_cast<float>(minValD);
    float maxF = static_cast<float>(maxValD);

    if (maxF <= minF) {
        lowVal = minF;
        highVal = maxF;
        return;
    }

    // 若影像已被壓縮到 [0,1]，強制使用 4096 bins；否則根據動態範圍決定 bin 數並 clamp 到 [1,4096]
    int histSize;
    if (minF >= 0.0f && maxF <= 1.0f) {
        histSize = 4096;
    }
    else {
        // 以 range 長度為基礎，但限制最大 4096
        histSize = static_cast<int>(std::ceil(maxF - minF));
        if (histSize < 1) histSize = 1;
        if (histSize > 4096) histSize = 4096;
    }

    float rangeArr[2] = { minF, maxF };
    const float* histRange[] = { rangeArr };
    int channels[] = { 0 };
    int histSizes[] = { histSize };

    cv::Mat hist;
    cv::calcHist(&single, 1, channels, cv::Mat(), hist, 1, histSizes, histRange);

    double total = static_cast<double>(single.total());
    if (total <= 0.0) {
        lowVal = minF;
        highVal = maxF;
        return;
    }

    // 累積直方圖找 1% / 99%
    double acc = 0.0;
    bool foundLow = false;
    for (int i = 0; i < histSize; ++i) {
        acc += hist.at<float>(i);
        double frac = acc / total;
        if (!foundLow && frac >= 0.01) {
            // 使用 bin 中心估算對應值
            lowVal = minF + (static_cast<double>(i) + 0.5) * (maxF - minF) / histSize;
            foundLow = true;
        }
        if (frac >= 0.99) {
            highVal = minF + (static_cast<double>(i) + 0.5) * (maxF - minF) / histSize;
            break;
        }
    }

    if (!foundLow) lowVal = minF;
    if (highVal == 0.0) highVal = maxF;
}

// ========================================
// 輔助函數：計算通道中前 N% 最亮像素的平均值
// ========================================
static double getBiggestMean(const cv::Mat& channel, double ratio)
{
    cv::Mat flat = channel.reshape(1, 1);
    std::vector<float> vals;
    flat.copyTo(vals);

    std::sort(vals.begin(), vals.end(), std::greater<float>());

    int count = (int)(vals.size() * ratio);
    if (count < 1) count = 1;

    double sum = 0.0;
    for (int i = 0; i < count; i++)
        sum += vals[i];

    return sum / count;
}