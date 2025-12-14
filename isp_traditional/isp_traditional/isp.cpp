#include "isp.h"
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <functional>

// 在 includes 之後加入（靠近檔案頂端）
static void CalLowHigh(const cv::Mat& img, double& lowVal, double& highVal);
// 取前幾大均值
static double getBiggestMean(const cv::Mat& channel, double ratio = 0.05);


cv::Mat ISP::loadRawWithLibRaw(
    const std::string& filename,
    int& width,
    int& height,
    int& black,                      // 輸出黑階
    int& white,                      // 輸出白階
    std::vector<float>& cam_mul,     // 輸出 AWB gains
    std::vector<float>& pre_mul,     // 輸出 AWB gains
    cv::Mat& cam_xyz,                // 輸出 3x3 相機→XYZ 矩陣
    cv::Mat& xyz2srgb)               // 輸出 3x3 XYZ→sRGB 矩陣
{

    LibRaw RawProcessor;  // stack 上的物件
    int ret = RawProcessor.open_file(filename.c_str());
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot open file: " << filename << " Error: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    ret = RawProcessor.unpack();  // 解包 RAW 數據
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot unpack raw data: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    // CFA pattern
    std::cout << "CFA description: "
        << RawProcessor.imgdata.idata.cdesc << std::endl;

    int pattern = RawProcessor.imgdata.idata.filters; // pattern == 9 -> RGGB
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

    int raw_width = RawProcessor.imgdata.sizes.raw_width;
    int raw_height = RawProcessor.imgdata.sizes.raw_height;
    width = RawProcessor.imgdata.sizes.width;// 有效寬度
    height = RawProcessor.imgdata.sizes.height;//有效高度

    int left = (raw_width - width) / 2; // 開始列
    int top = (raw_height - height) / 2; // 開始行

    cv::Mat raw16(height, width, CV_16U);

    // 拷貝掉黑邊，只取 active area
    for (int y = 0; y < height; y++) {
        memcpy(raw16.ptr<ushort>(y),
            RawProcessor.imgdata.rawdata.raw_image + (y + top) * raw_width + left,
            width * sizeof(ushort));
    }


    // 讀 CFA buffer
    libraw_data_t* raw = &RawProcessor.imgdata;
    ushort* rawData = raw->rawdata.raw_image;

    // 讀 metadata (黑階、白階、WB係數...)
    black = raw->color.black;
    white = raw->color.maximum;

    // 白平衡係數
    cam_mul.resize(sizeof(raw->color.cam_mul) / sizeof(raw->color.cam_mul[0]));
    for (int i = 0; i < cam_mul.size(); i++)
        cam_mul[i] = raw->color.cam_mul[i];

    pre_mul.resize(sizeof(raw->color.pre_mul) / sizeof(raw->color.pre_mul[0]));
    for (int i = 0; i < pre_mul.size(); i++)
        pre_mul[i] = raw->color.pre_mul[i];

    // 相機 RGB → XYZ 矩陣
    int rows = sizeof(raw->color.cam_xyz) / sizeof(raw->color.cam_xyz[0]);
    int cols = sizeof(raw->color.cam_xyz[0]) / sizeof(raw->color.cam_xyz[0][0]);
    cam_xyz = cv::Mat(rows - 1, cols, CV_32F); // 不拿最後的 bias 項，保持 3x3
    for (int i = 0;i < rows - 1;i++)
        for (int j = 0;j < cols;j++)
            cam_xyz.at<float>(i, j) = raw->color.cam_xyz[i][j];

    // XYZ → sRGB 矩陣
    xyz2srgb = (cv::Mat_<float>(3, 3) <<
        3.2406, -1.5372, -0.4986,
        -0.9689, 1.8758, 0.0415,
        0.0557, -0.2040, 1.0570);

    return raw16;
}

void ISP::applyPreMul(cv::Mat& img, const std::vector<float> pre_mul) {
    CV_Assert(img.type() == CV_32FC3);

    // 拆成通道
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    channels[0] *= pre_mul[0]; // B
    channels[1] *= pre_mul[1]; // G
    channels[2] *= pre_mul[2]; // R

    cv::merge(channels, img);
}

void ISP::blackLevelCorrection(cv::Mat& raw, float black_level) {
    raw -= black_level;                           // 直接減，保留原型別
    cv::threshold(raw, raw, 0, 0, cv::THRESH_TOZERO); // 避免負值
}

void ISP::whiteLevelNormalization(cv::Mat& raw, float white_level) {

    double minVal, maxVal;
    raw.convertTo(raw, CV_32F); // 有除法要用 float，但未處理超出邊界

    cv::minMaxLoc(raw, &minVal, &maxVal);
    raw = raw / white_level;
    cv::minMaxLoc(raw, &minVal, &maxVal);

}

// White Patch Method: 找亮點做白平衡
void ISP::CalAWBGain_WhitePatch(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B)
{
    CV_Assert(img.depth() == CV_32F); // 支援 32F
    CV_Assert(img.channels() == 3);   // 支援 BGR 彩色

    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    double R_mean = getBiggestMean(channels[2]);
    double G_mean = getBiggestMean(channels[1]);
    double B_mean = getBiggestMean(channels[0]);

    // G 作為參考
    gain_R = G_mean / R_mean;
    gain_G = 1.0;
    gain_B = G_mean / B_mean;
}

void ISP::CalAWBGain_GrayWorld(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B) {
    cv::Mat img32F;
    img.convertTo(img32F, CV_32F);

    std::vector<cv::Mat> channels(3);
    cv::split(img32F, channels);

    double meanR = cv::mean(channels[2])[0];
    double meanG = cv::mean(channels[1])[0];
    double meanB = cv::mean(channels[0])[0];
    double meanGray = (meanR + meanG + meanB) / 3.0;

    double eps = 1e-6;
    gain_R = meanGray / (meanR + eps);
    gain_G = meanGray / (meanG + eps);
    gain_B = meanGray / (meanB + eps);
}

void ISP::ApplyAWBGain(cv::Mat& raw32, int height, int width, double gainR, double gainG, double gainB)
{
    std::vector<float> cam_mul_normalized(4);

    cam_mul_normalized[0] = gainR; // R
    cam_mul_normalized[1] = gainG; // G (first G)
    cam_mul_normalized[2] = gainG; // second G (depends on ordering)
    cam_mul_normalized[3] = gainB; // B

    // 套用 AWB Gain
    for (int y = 0; y < height; y++)
    {
        float* row = raw32.ptr<float>(y);
        for (int x = 0; x < width; x++)
        {
            int idx = ((y & 1) << 1) | (x & 1);
            row[x] *= cam_mul_normalized[idx];
        }
    }
}

cv::Mat ISP::demosaic(const cv::Mat& rawIn) {
    cv::Mat raw32;

    // 1. 轉 float 0~1
    if (rawIn.type() == CV_16U) {
        rawIn.convertTo(raw32, CV_32F, 1.0 / 65535.0);
    }
    else if (rawIn.type() == CV_32F) {
        raw32 = rawIn.clone();
    }
    else {
        throw std::runtime_error("Unsupported input type: only CV_32F or CV_16U allowed.");
    }

    // 2. clip <0
    cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO);

    // 3. clip >1
    cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);

    // 4. 先轉成 CV_16U 避免 cvtColor float crash
    cv::Mat raw16;
    raw32.convertTo(raw16, CV_16U, 65535.0);

    // 5. demosaic
    cv::Mat bgr16;
    cv::cvtColor(raw16, bgr16, cv::COLOR_BayerBG2BGR); // Bayer pattern 視 sensor 而定

    // 6. 轉回 float 0~1
    cv::Mat bgr32;
    bgr16.convertTo(bgr32, CV_32F, 1.0 / 65535.0);

    return bgr32;
}

cv::Mat ISP::colorCorrection(const cv::Mat& img, const cv::Mat& ccm) {

    // img_f: float CV_32F, BGR
    cv::Mat img_rgb, corrected_rgb, corrected_bgr;

    cv::cvtColor(img, img_rgb, cv::COLOR_BGR2RGB); // BGR -> RGB
    cv::transform(img_rgb, corrected_rgb, ccm); // RGB -> sRGB

    cv::cvtColor(corrected_rgb, corrected_bgr, cv::COLOR_RGB2BGR); // back to BGR
    return corrected_bgr;
}



void ISP::applyToneMapping(cv::Mat& img_float, float gamma) {

    cv::Mat tmp = img_float.clone();

    std::vector<cv::Mat> channels;
    cv::split(tmp, channels);

    for (auto& c : channels)
    {
        // 負值設 0
        c.setTo(0, c < 0);

        // 避免 NaN
        cv::patchNaNs(c, 0.0);

        // 避免過大
        cv::threshold(c, c, 1e6, 1e6, cv::THRESH_TRUNC);
    }
    cv::merge(channels, tmp);

    double minVal, maxVal;
    cv::minMaxLoc(tmp.reshape(1), &minVal, &maxVal);

    // 再做 gamma 校正
    cv::pow(tmp, 1.0 / gamma, img_float);
}

void ISP::sharpening(cv::Mat& img, double Sharpening_Level) {
    cv::Mat blurred;
    cv::GaussianBlur(img, blurred, cv::Size(0, 0), 3);
    cv::addWeighted(img, 1 + Sharpening_Level, blurred, -Sharpening_Level, 0, img);
}

void ISP::showPreview(const cv::Mat& img, const std::string& title, double scale, bool autoContrast) {
    cv::Mat preview8;

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

            double minVal, maxVal;
            CalLowHigh(tmp, minVal, maxVal);

            tmp = (tmp - minVal) / (maxVal - minVal);        // 正規化到 0~1
            tmp.convertTo(preview8, CV_8U, 255.0);
        }
        else {
            img.convertTo(preview8, CV_8U, 255.0); // 假設輸入在 [0,1] 範圍
        }
    }
    else {
        throw std::runtime_error("Unsupported image depth!");
    }

    // resize if needed
    if (scale != 1.0) {
        cv::Mat temp;
        cv::resize(preview8, temp, cv::Size(), scale, scale, cv::INTER_AREA);
        preview8 = temp;
    }
    cv::imwrite(std::to_string(ImgCount++) + "_" + title + ".tiff", preview8);
    cv::imshow(title, preview8);
    cv::waitKey(0);
}

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