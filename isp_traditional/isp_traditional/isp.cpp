#include "isp.h"
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <functional>
#include <filesystem>
#include <onnxruntime_cxx_api.h>
#include <algorithm>

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
    cv::Mat& cam_rgb,                // 輸出 3x3 相機 RGB→相機 RGB 矩陣
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

        // 10. 提取相機 RGB → 相機 RGB 矩陣 (3x3)
        cam_rgb = cv::Mat(3, 3, CV_32F);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cam_rgb.at<float>(i, j) = raw->color.rgb_cam[i][j];
            }
        }

        // 11. 防呆檢查
        if (raw16.empty()) {
            std::cerr << "Failed to load image!" << std::endl;
            return ErrCode::EmptyImage;
        }

        // 12. 轉換為 CV_32F 格式
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

//// ========================================
//// 功能：白平衡增益計算 - Gray World 方法
//// 說明：假設場景平均顏色為灰色，計算使各通道平均值相等的增益
//// 輸入參數：
////   - img: BGR 彩色影像 (CV_32F)
//// 輸出參數：
////   - gain_R, gain_G, gain_B: 計算出的 RGB 增益
//// 回傳：
////   - ErrCode::Ok 成功
////   - ErrCode::EmptyImage 影像為空
////   - ErrCode::InvalidInput 影像格式不支援
////   - ErrCode::Exception 例外發生
////   - ErrCode::Unknown 未知錯誤
//// ========================================
//ISP::ErrCode ISP::CalAWBGain_GrayWorld(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B) {
//    try {
//        // 防呆：檢查輸入
//        if (img.empty()) {
//            return ErrCode::EmptyImage;
//        }
//        if (img.depth() != CV_32F || img.channels() != 3) {
//            return ErrCode::InvalidInput;
//        }
//
//        // 複製並確保為 CV_32F
//        cv::Mat img32F;
//        img.convertTo(img32F, CV_32F);
//
//        // 拆成通道
//        std::vector<cv::Mat> channels(3);
//        cv::split(img32F, channels);
//
//        // 計算各通道平均值
//        double meanR = cv::mean(channels[2])[0];
//        double meanG = cv::mean(channels[1])[0];
//        double meanB = cv::mean(channels[0])[0];
//        double meanGray = (meanR + meanG + meanB) / 3.0;
//
//        // 計算增益
//        double eps = 1e-6;
//        gain_R = meanGray / (meanR + eps);
//        gain_G = meanGray / (meanG + eps);
//        gain_B = meanGray / (meanB + eps);
//
//        return ErrCode::Ok;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in CalAWBGain_GrayWorld: " << ex.what() << std::endl;
//        return ErrCode::Exception;
//    }
//    catch (...) {
//        std::cerr << "Unknown exception in CalAWBGain_GrayWorld" << std::endl;
//        return ErrCode::Unknown;
//    }
//}
//
//// ========================================
//// 功能：白平衡增益計算 - White Patch 方法
//// 說明：找影像中最亮的區域作為白點參考，計算增益
//// 輸入參數：
////   - img: BGR 彩色影像 (CV_32F)
//// 輸出參數：
////   - gain_R, gain_G, gain_B: 計算出的 RGB 增益
//// 回傳：
////   - ErrCode::Ok 成功
////   - ErrCode::EmptyImage 影像為空
////   - ErrCode::InvalidInput 影像格式不支援
////   - ErrCode::Exception 例外發生
////   - ErrCode::Unknown 未知錯誤
//// ========================================
//ISP::ErrCode ISP::CalAWBGain_WhitePatch(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B) {
//    try {
//        // 防呆：檢查輸入
//        if (img.empty()) {
//            return ErrCode::EmptyImage;
//        }
//        if (img.depth() != CV_32F || img.channels() != 3) {
//            return ErrCode::InvalidInput;
//        }
//
//        // 拆成通道
//        std::vector<cv::Mat> channels(3);
//        cv::split(img, channels);
//
//        // 取前 5% 最亮像素的平均值
//        double R_mean = getBiggestMean(channels[2]);
//        double G_mean = getBiggestMean(channels[1]);
//        double B_mean = getBiggestMean(channels[0]);
//
//        // G 作為參考
//        gain_R = G_mean / R_mean;
//        gain_G = 1.0;
//        gain_B = G_mean / B_mean;
//
//        return ErrCode::Ok;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in CalAWBGain_WhitePatch: " << ex.what() << std::endl;
//        return ErrCode::Exception;
//    }
//    catch (...) {
//        std::cerr << "Unknown exception in CalAWBGain_WhitePatch" << std::endl;
//        return ErrCode::Unknown;
//    }
//}

// ========================================
// 功能：白平衡增益計算 - Gray World 方法 (RAW BGGR 專用)
// 說明：根據 BGGR 2x2 陣列分別統計 B, G, R 像素平均值
// 輸入參數：
//    - rawImg: 單通道 RAW 影像 (CV_32F, 格式預設為 BGGR)
// 輸出參數：
//    - gain_R, gain_G, gain_B: 計算出的 RGB 增益
// ========================================
ISP::ErrCode ISP::CalAWBGain_GrayWorld(const cv::Mat& rawImg, double& gain_R, double& gain_G, double& gain_B) {
    try {
        // 防呆：檢查輸入 (RAW 應為單通道 CV_32F)
        if (rawImg.empty()) {
            return ErrCode::EmptyImage;
        }
        if (rawImg.depth() != CV_32F || rawImg.channels() != 1) {
            return ErrCode::InvalidInput;
        }

        double sumB = 0.0, sumG = 0.0, sumR = 0.0;
        int countB = 0, countG = 0, countR = 0;

        int height = rawImg.rows;
        int width = rawImg.cols;

        // 走訪像素並根據 BGGR 位置分類統計
        for (int y = 0; y < height; y++) {
            const float* row = rawImg.ptr<float>(y);
            bool isEvenRow = (y % 2 == 0);

            for (int x = 0; x < width; x++) {
                bool isEvenCol = (x % 2 == 0);
                float val = row[x];

                if (isEvenRow && isEvenCol) {
                    // (y:偶, x:偶) -> B
                    sumB += val;
                    countB++;
                }
                else if (isEvenRow && !isEvenCol) {
                    // (y:偶, x:奇) -> Gb
                    sumG += val;
                    countG++;
                }
                else if (!isEvenRow && isEvenCol) {
                    // (y:奇, x:偶) -> Gr
                    sumG += val;
                    countG++;
                }
                else {
                    // (y:奇, x:奇) -> R
                    sumR += val;
                    countR++;
                }
            }
        }

        // 計算各通道平均值
        double eps = 1e-6;
        double meanB = sumB / (countB + eps);
        double meanG = sumG / (countG + eps);
        double meanR = sumR / (countR + eps);

        // 灰度世界假設：Target = (R + G + B) / 3
        double meanGray = (meanR + meanG + meanB) / 3.0;

        // 計算增益
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
// 功能：白平衡增益計算 - White Patch 方法 (RAW BGGR 專用)
// 說明：抽取 BGGR 獨立通道後，分別取前 5% 最亮像素的平均值作為參考
// 輸入參數：
//    - rawImg: 單通道 RAW 影像 (CV_32F, 格式預設為 BGGR)
// 輸出參數：
//    - gain_R, gain_G, gain_B: 計算出的 RGB 增益
// ========================================
ISP::ErrCode ISP::CalAWBGain_WhitePatch(const cv::Mat& rawImg, double& gain_R, double& gain_G, double& gain_B) {
    try {
        // 防呆：檢查輸入 (RAW 應為單通道 CV_32F)
        if (rawImg.empty()) {
            return ErrCode::EmptyImage;
        }
        if (rawImg.depth() != CV_32F || rawImg.channels() != 1) {
            return ErrCode::InvalidInput;
        }

        int height = rawImg.rows;
        int width = rawImg.cols;

        // 建立單通道圖來存放拆解出的 B, G, R 像素
        // G 通道像素數量為 B/R 的兩倍
        cv::Mat matB(height / 2, width / 2, CV_32F);
        cv::Mat matR(height / 2, width / 2, CV_32F);
        cv::Mat matG(height, width / 2, CV_32F); // 包含 Gb 與 Gr

        int gRowIdx = 0;
        for (int y = 0; y < height; y++) {
            const float* srcRow = rawImg.ptr<float>(y);
            int halfY = y / 2;

            if (y % 2 == 0) {
                // 偶數列：B, Gb
                float* bRow = matB.ptr<float>(halfY);
                float* gRow = matG.ptr<float>(gRowIdx++);
                for (int x = 0; x < width; x += 2) {
                    bRow[x / 2] = srcRow[x];     // B
                    gRow[x / 2] = srcRow[x + 1]; // Gb
                }
            }
            else {
                // 奇數列：Gr, R
                float* gRow = matG.ptr<float>(gRowIdx++);
                float* rRow = matR.ptr<float>(halfY);
                for (int x = 0; x < width; x += 2) {
                    gRow[x / 2] = srcRow[x];     // Gr
                    rRow[x / 2] = srcRow[x + 1]; // R
                }
            }
        }

        // 使用原本的 getBiggestMean 取各通道前 5% 最亮像素平均值
        double B_mean = getBiggestMean(matB);
        double G_mean = getBiggestMean(matG);
        double R_mean = getBiggestMean(matR);

        // 以 G 為基準 1.0 計算 Gain
        double eps = 1e-6;
        gain_R = G_mean / (R_mean + eps);
        gain_G = 1.0;
        gain_B = G_mean / (B_mean + eps);

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
        // ------------------------------------------------------------------
        // 0. 防禦機制：將當前模組 (DLL/EXE) 所在目錄註冊至 Windows DLL 搜尋清單
        //    (解決 ONNX Runtime 找不到同目錄下 cuDNN / CUDA DLL 的問題)
        // ------------------------------------------------------------------
        //HMODULE hModule = NULL;
        //// 直接傳入函式內區域變數 &hModule 的位址，代表「取得包含這行程式碼的 DLL/EXE 模組」
        //GetModuleHandleExW(
        //    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        //    (LPCWSTR)&hModule, // 傳入一般的位址，完全合規且不會有編譯警告
        //    &hModule
        //);

        //wchar_t modulePath[MAX_PATH];
        //if (GetModuleFileNameW(hModule, modulePath, MAX_PATH) > 0) {
        //    std::wstring wpath(modulePath);
        //    std::wstring moduleDir = wpath.substr(0, wpath.find_last_of(L"\\/"));
        //    if (AddDllDirectory(moduleDir.c_str())) {
        //        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
        //    }
        //}

        // 1. 只有在尚未建立 Env 時才初始化 (Lazy Init)
        if (!ort_env) {
            ort_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "AIDemosaic");
        }

        // 2. 設定 SessionOptions
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(2);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // 3. 嘗試掛載 CUDA 加速 (若無 GPU 或失敗會自動 fallback 到 CPU)
        bool is_cuda_enabled = false;
        try {
            OrtCUDAProviderOptions cuda_options;
            cuda_options.device_id = 0;

            session_options.AppendExecutionProvider_CUDA(cuda_options);
            is_cuda_enabled = true;
            std::cout << "⚡ 已嘗試掛載 CUDA Provider..." << std::endl;
        }
        catch (const Ort::Exception& e) {
            std::cerr << "⚠️ CUDA 掛載失敗 (Ort::Exception): " << e.what() << std::endl;
            std::cerr << "👉 將自動退回純 CPU 模式執行。" << std::endl;
            is_cuda_enabled = false;
        }

        // ------------------------------------------------------------------
        // 4. 改用 C++ std::ifstream 讀取模型 Buffer
        //    (完全繞過 Windows CreateFileMapping 與權限/唯讀鎖定問題)
        // ------------------------------------------------------------------
        std::ifstream model_file(modelPath, std::ios::binary | std::ios::ate);
        if (!model_file.is_open()) {
            std::cerr << "[AI Demosaic] 無法開啟模型檔案: " << modelPath << std::endl;
            return ErrCode::DemosaicFailed;
        }

        std::streamsize model_size = model_file.tellg();
        model_file.seekg(0, std::ios::beg);

        std::vector<char> model_buffer(model_size);
        if (!model_file.read(model_buffer.data(), model_size)) {
            std::cerr << "[AI Demosaic] 讀取模型內容失敗" << std::endl;
            return ErrCode::DemosaicFailed;
        }
        model_file.close(); // 讀取完成後立刻關閉檔案，不留鎖定

        // 5. 建立 Session (使用 Memory Buffer 多載版本)
        ort_session = std::make_unique<Ort::Session>(
            *ort_env,
            model_buffer.data(),
            model_size,
            session_options
        );

        // 6. 解析 Input/Output Node 名稱
        Ort::AllocatorWithDefaultOptions allocator;
        auto input_name_ptr = ort_session->GetInputNameAllocated(0, allocator);
        auto output_name_ptr = ort_session->GetOutputNameAllocated(0, allocator);

        ort_input_name = input_name_ptr.get();
        ort_output_name = output_name_ptr.get();

        ai_model_path = modelPath;
        return ErrCode::Ok;
    }
    catch (const std::exception& e) {
        std::cerr << "[AI Demosaic] Model loading failed: " << e.what() << std::endl;
        return ErrCode::DemosaicFailed;
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
        if (!ort_session) {
            std::cerr << "AiDemosaic: ONNX Session not initialized! Please call SetAiDemosaicModel first." << std::endl;
            return ErrCode::InvalidInput;
        }

        // 確保 raw 為 single-channel 或多通道 float (0..1)
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

        std::cerr << "AiDemosaic: inputMat size = " << inputMat.cols << "x" << inputMat.rows
            << ", channels = " << inputMat.channels() << ", type = " << inputMat.type() << std::endl;

        // 若輸入為 single-channel (Bayer Raw)，拆成 4 通道 (H/2, W/2, 4)
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

            cv::Mat chR(h2, w2, CV_32F);
            cv::Mat chG1(h2, w2, CV_32F);
            cv::Mat chG2(h2, w2, CV_32F);
            cv::Mat chB(h2, w2, CV_32F);

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

            // Clip 到 0..1
            cv::threshold(chR, chR, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chR, chR, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chG1, chG1, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chG1, chG1, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chG2, chG2, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chG2, chG2, 1.0, 1.0, cv::THRESH_TRUNC);

            cv::threshold(chB, chB, 0.0, 0.0, cv::THRESH_TOZERO);
            cv::threshold(chB, chB, 1.0, 1.0, cv::THRESH_TRUNC);

            std::vector<cv::Mat> raw4ch_vec = { chR, chG1, chG2, chB };
            cv::Mat raw4ch;
            cv::merge(raw4ch_vec, raw4ch); // (H/2, W/2, 4)

            std::cerr << "AiDemosaic: assembled raw4ch size = " << raw4ch.cols << "x" << raw4ch.rows
                << ", channels = " << raw4ch.channels() << std::endl;

            modelInput = raw4ch;
        }
        else {
            modelInput = inputMat;
        }

        // =========================================================================
        // 💡 ONNX Runtime 前處理：將 HWC cv::Mat 轉為 NCHW 記憶體佈局的 float 向量
        // =========================================================================
        int inH = modelInput.rows;
        int inW = modelInput.cols;
        int inC = modelInput.channels();

        std::vector<float> inputTensorValues(1 * inC * inH * inW);

        // 分離通道填入 NCHW 佈局 (Planar layout)
        std::vector<cv::Mat> inputChannels(inC);
        for (int c = 0; c < inC; ++c) {
            // 指向 inputTensorValues 對應通道的起始記憶體區塊
            inputChannels[c] = cv::Mat(inH, inW, CV_32F, inputTensorValues.data() + c * (inH * inW));
        }
        // cv::split 會直接將 HWC Mat 自動解開並複製到 inputTensorValues 連續記憶體中
        cv::split(modelInput, inputChannels);

        // 建立 ONNX Input Tensor
        std::vector<int64_t> inputDims = { 1, inC, inH, inW };
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(), inputTensorValues.size(),
            inputDims.data(), inputDims.size()
        );

        // =========================================================================
        // 💡 執行 ONNX Runtime 推論
        // =========================================================================
        const char* inputNames[] = { ort_input_name.c_str() };
        const char* outputNames[] = { ort_output_name.c_str() };

        std::vector<Ort::Value> outputTensors;
        try {
            outputTensors = ort_session->Run(
                Ort::RunOptions{ nullptr },
                inputNames, &inputTensor, 1,
                outputNames, 1
            );
        }
        catch (const Ort::Exception& e) {
            std::cerr << "ONNX Runtime inference failed: " << e.what() << std::endl;
            return ErrCode::DemosaicFailed;
        }

        if (outputTensors.empty() || !outputTensors[0].IsTensor()) {
            std::cerr << "Invalid output tensor from ONNX Runtime" << std::endl;
            return ErrCode::DemosaicFailed;
        }

        // =========================================================================
        // 💡 解析輸出 Tensor (NCHW -> cv::Mat)
        // =========================================================================
        auto tensorInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        std::vector<int64_t> outDims = tensorInfo.GetShape();

        if (outDims.size() != 4) {
            std::cerr << "Unexpected output dimensions from model: " << outDims.size() << std::endl;
            return ErrCode::DemosaicFailed;
        }

        int outN = static_cast<int>(outDims[0]);
        int outC = static_cast<int>(outDims[1]);
        int outH = static_cast<int>(outDims[2]);
        int outW = static_cast<int>(outDims[3]);

        if (outN < 1 || (outC != 3 && outC != 1)) {
            std::cerr << "Model output channels not 1 or 3: " << outC << std::endl;
            return ErrCode::DemosaicFailed;
        }

        // 取得輸出 float 陣列指標
        float* floatResults = outputTensors[0].GetTensorMutableData<float>();

        // 將 NCHW 轉回 OpenCV 通道列表
        std::vector<cv::Mat> outChannels;
        outChannels.reserve(outC);
        for (int i = 0; i < outC; ++i) {
            // 指向 Channel i 的起點並 clone 以擁有獨占記憶體
            cv::Mat ch(outH, outW, CV_32F, floatResults + i * (outH * outW));
            outChannels.push_back(ch.clone());
        }

        cv::Mat merged;
        if (outC == 1) {
            // 單通道 -> 複製成 3 通道灰階
            cv::Mat gray = outChannels[0];
            cv::cvtColor(gray, merged, cv::COLOR_GRAY2BGR);
        }
        else {
            // 3 通道：假設輸出為 RGB，合成後轉為 BGR
            cv::Mat m;
            cv::merge(outChannels, m);
            cv::cvtColor(m, merged, cv::COLOR_RGB2BGR);
        }

        // Clip 確保輸出範疇在 0..1
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




// =========================================================================
// 功能：亮度對齊與三階段雙分位數強健歸一化 (CV_32FC3)
// 步驟：
//  1. P50 Gain Alignment (基於 BT.709 Luma 視覺主體對齊)
//  2. P99 High-Light Normalization (去除 1% 極端亮點並歸一化)
//  3. 1%~99% Dynamic Range Stretching (基於 CalLowHigh 之極致對比延伸)
// =========================================================================
// =========================================================================
// 功能：亮度對齊與三階段雙分位數強健歸一化 (相容 CV_32FC1 / CV_32FC3)
// =========================================================================
ISP::ErrCode ISP::normalizeExposureByP50(cv::Mat& img_float, float Target_P50) {
    try {
        if (img_float.empty()) {
            return ErrCode::EmptyImage;
        }
        int channels = img_float.channels();
        if (img_float.depth() != CV_32F || (channels != 1 && channels != 3)) {
            return ErrCode::InvalidInput;
        }
        if (Target_P50 <= 0.0f) {
            return ErrCode::InvalidInput;
        }

        // =================================================================
        // STAGE 1: 計算統計用單通道 Luma 並求出 P50 Base Gain
        // =================================================================
        cv::Mat luma_img;
        if (channels == 3) {
            luma_img.create(img_float.size(), CV_32FC1);
            static const cv::Matx13f bgr_to_luma(0.0722f, 0.7152f, 0.2126f);
            cv::transform(img_float, luma_img, bgr_to_luma);
        }
        else {
            luma_img = img_float;
        }

        // 確保抽樣時記憶體連續
        cv::Mat continuous_luma = luma_img.isContinuous() ? luma_img : luma_img.clone();
        const float* flat_ptr = continuous_luma.ptr<float>(0);
        const int total_pixels = continuous_luma.cols * continuous_luma.rows;

        std::vector<float> sample_pixels;
        const int sample_step = std::max<float>(1, total_pixels / 50000);
        sample_pixels.reserve((total_pixels + sample_step - 1) / sample_step);

        for (int i = 0; i < total_pixels; i += sample_step) {
            sample_pixels.push_back(flat_ptr[i]);
        }

        if (sample_pixels.empty()) {
            return ErrCode::Ok;
        }

        // 1. 定義 ISP 相機系統的黑階/噪點底限 (例如 12-bit Raw 轉 float 後的最低有效訊號)
        const float NOISE_FLOOR = 1e-4f; // 相當於 0.0001 (約 10-bit 下的 0.4 LSB)
        const float MIN_VALID_DYNAMIC_RANGE = 1e-3f; // 低於此動態範圍視為極度低曝光場景

        // 自適應估算當前影像 [P5, P95] 動態範圍
        size_t p5_idx = static_cast<size_t>(sample_pixels.size() * 0.05);
        size_t p95_idx = static_cast<size_t>(sample_pixels.size() * 0.95);

        std::nth_element(sample_pixels.begin(), sample_pixels.begin() + p5_idx, sample_pixels.end());
        float p5_val = sample_pixels[p5_idx];

        std::nth_element(sample_pixels.begin() + p5_idx, sample_pixels.begin() + p95_idx, sample_pixels.end());
        float p95_val = sample_pixels[p95_idx];

        float dynamic_range = p95_val - p5_val;

        std::vector<float> valid_pixels;
        valid_pixels.reserve(sample_pixels.size());

        // 2. 判斷是否為「正常曝光」還是「極低曝光/平坦場景」
        if (dynamic_range >= MIN_VALID_DYNAMIC_RANGE) {
            // 正常場景：執行相對邊界裁切 (剔除極端高光與極端死暗)
            float relative_dark_thresh = p5_val + 0.05f * dynamic_range;
            float relative_bright_thresh = p5_val + 0.90f * dynamic_range;

            for (float val : sample_pixels) {
                if (val >= relative_dark_thresh && val <= relative_bright_thresh) {
                    valid_pixels.push_back(val);
                }
            }
        }
        else {
            // 極低曝光場景 (例如 P95 = 0.00196)：
            // 放寬門檻，只過濾低於 Noise Floor 的純黑/噪點像素，保留真實微弱訊號
            for (float val : sample_pixels) {
                if (val > NOISE_FLOOR) {
                    valid_pixels.push_back(val);
                }
            }
        }

        // 3. 計算 P50 (Median)
        float p50_val = 0.0f;
        if (!valid_pixels.empty()) {
            size_t mid_idx = valid_pixels.size() / 2;
            std::nth_element(valid_pixels.begin(), valid_pixels.begin() + mid_idx, valid_pixels.end());
            p50_val = valid_pixels[mid_idx];
        }
        else {
            // 若連 valid_pixels 都空了（全圖近乎純黑），直接取原始採樣的 P50
            size_t p50_idx = static_cast<size_t>(sample_pixels.size() * 0.50);
            std::nth_element(sample_pixels.begin(), sample_pixels.begin() + p50_idx, sample_pixels.end());
            p50_val = sample_pixels[p50_idx];
        }

        // 4. 低曝光安全 Gain 計算與保護
        // 避免 p50_val 為 0 導致除以零，強制給予一個最小可算分母 (NOISE_FLOOR)
        //float safe_p50 = std::max<float>(p50_val, NOISE_FLOOR);

        // 算出提亮 Gain
        float gain = Target_P50 / p50_val;

        // 極低曝光下需有明確的 Gain 上限保護，防止把雜訊放大成巨幅斑塊
        //const float max_allowed_gain = 32.0f; // 依據 ISP 雜訊容忍度設定 (例如 32x / +30dB)
        //const float min_allowed_gain = 0.01f;
        //gain = std::clamp(gain, min_allowed_gain, max_allowed_gain);


        // 乘上 P50 Gain (3 通道或 1 通道皆可直接使用 cv::Mat 的 * 運算符)
        img_float = img_float * gain;

        //// =================================================================
        //// STAGE 2: 針對乘上 Gain 後的影像進行 99% 百分位數 Normalization
        //// =================================================================
        //double stage2_low = 0.0, stage2_p99 = 0.0;
        //CalLowHigh(img_float, stage2_low, stage2_p99);

        //// 將 [0, P99] 歸一化到 [0, 1.0]
        ////if (stage2_p99 > 1e-6) {
        ////    img_float = img_float / static_cast<float>(stage2_p99);
        ////}

        //// =================================================================
        //// STAGE 3: 執行最終 1% ~ 99% Min-Max Contrast Normalization
        //// =================================================================
        //double final_p1 = 0.0, final_p99 = 1.0;
        //CalLowHigh(img_float, final_p1, final_p99);

        //double range_span = final_p99 - final_p1;
        //if (range_span > 1e-6) {
        //    img_float = (img_float - final_p1) / range_span;
        //}

        // 最終邊界安全裁切 [0.0, 1.0]
        cv::threshold(img_float, img_float, 0.0, 0.0, cv::THRESH_TOZERO);
        cv::threshold(img_float, img_float, 1.0, 1.0, cv::THRESH_TRUNC);

        return ErrCode::Ok;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in normalizeExposureByP50: " << ex.what() << std::endl;
        return ErrCode::ToneMappingFailed;
    }
    catch (...) {
        std::cerr << "Unknown exception in normalizeExposureByP50" << std::endl;
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
        //cv::imwrite(std::to_string(ImgCount++) + "_" + title + ".tiff", preview8);

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
    lowVal = 0.0;
    highVal = 0.0;

    if (img.empty()) return;

    // 1. 安全轉為單通道進行統計
    cv::Mat single;
    if (img.channels() == 3) {
        // BGR -> Luminance (BT.709)
        static const cv::Matx13f bgr_to_luma(0.0722f, 0.7152f, 0.2126f);
        cv::transform(img, single, bgr_to_luma);
    }
    else if (img.channels() == 1) {
        single = img;
    }
    else {
        return; // 不支援的通道數
    }

    // 2. 記憶體連續性防禦 (防止 ROI 裁切導致 reshape/calcHist 讀到垃圾記憶體)
    if (!single.isContinuous()) {
        single = single.clone();
    }

    // 3. 取得極值
    double minValD = 0.0, maxValD = 0.0;
    cv::minMaxLoc(single, &minValD, &maxValD);

    float minF = static_cast<float>(minValD);
    float maxF = static_cast<float>(maxValD);

    if (maxF <= minF + 1e-7f) {
        lowVal = minF;
        highVal = maxF;
        return;
    }

    // 4. 強制使用高解析度 4096 bins (避免 maxF > 1.0 時 bin 數掉到剩幾十個)
    const int histSize = 4096;
    float rangeArr[2] = { minF, maxF };
    const float* histRange[] = { rangeArr };
    int channels[] = { 0 };

    cv::Mat hist;
    cv::calcHist(&single, 1, channels, cv::Mat(), hist, 1, &histSize, histRange);

    double total = static_cast<double>(single.total());
    if (total <= 0.0) {
        lowVal = minF;
        highVal = maxF;
        return;
    }

    // 5. 累積直方圖找 1% / 99%
    double acc = 0.0;
    bool foundLow = false;
    double bin_width = static_cast<double>(maxF - minF) / histSize;

    for (int i = 0; i < histSize; ++i) {
        acc += hist.at<float>(i);
        double frac = acc / total;
        if (!foundLow && frac >= 0.01) {
            lowVal = minF + (static_cast<double>(i) + 0.5) * bin_width;
            foundLow = true;
        }
        if (frac >= 0.99) {
            highVal = minF + (static_cast<double>(i) + 0.5) * bin_width;
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