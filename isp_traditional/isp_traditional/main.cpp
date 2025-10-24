#include <iostream>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>
#include <unordered_map>
#include <any>
#include <string>

#pragma once
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>

class ISP
{
public:
    // Enum 定義
    enum class AWB_Method { Default = 1, GrayWorld = 2, WhitePoint = 3 };
    enum class Demosaic_Method { CCM = 1, AI = 2 };

    // Constructor
    ISP() {
        // ----------------- Bool 參數 map -----------------
        // map 直接存值，不用 reference
        bool_params["DoWhiteBlackLevel"] = true;  // 是否執行黑白電平校正
        bool_params["DoAWB"] = true;             // 是否執行白平衡
        bool_params["DoDemosaic"] = true;        // 是否執行 demosaic
        bool_params["DoCCM"] = false;             // 是否執行色彩校正(CCM)
        bool_params["DoGamma"] = true;           // 是否執行 Gamma 校正
        bool_params["DoSharpen"] = true;         // 是否執行銳化

        // ----------------- Enum 參數 map -----------------
        enumAWB_params["AWB_Method"] = AWB_Method::Default;       // 預設 AWB 方法
        enumDemosaic_params["Demosaic_Method"] = Demosaic_Method::CCM; // 預設 demosaic 方法
    }

    // ----------------- Bool 參數操作 -----------------
    void setParamBool(const std::string& key, bool value) {
        if (bool_params.count(key))
            bool_params[key] = value;
        else throw std::invalid_argument("Invalid bool key");
    }

    bool getParamBool(const std::string& key) const {
        if (bool_params.count(key))
            return bool_params.at(key);
        else throw std::invalid_argument("Invalid bool key");
    }

    // ----------------- Enum 參數操作 -----------------
    void setParamAWB(AWB_Method value) { enumAWB_params["AWB_Method"] = value; }
    void setParamDemosaic(Demosaic_Method value) { enumDemosaic_params["Demosaic_Method"] = value; }

    AWB_Method getParamAWB() const { return enumAWB_params.at("AWB_Method"); }
    Demosaic_Method getParamDemosaic() const { return enumDemosaic_params.at("Demosaic_Method"); }

    // ----------------- 影像處理函數 -----------------
    cv::Mat loadRawWithLibRaw(
        const std::string& filename,
        int& Width,
        int& Height,
        int& black,        // 輸出黑階
        int& white,        // 輸出白階
        std::vector<float>& cam_mul,  // AWB before demosaic
        std::vector<float>& pre_mul,  // AWB after demosaic
        cv::Mat& cam_xyz,             // 輸出 3x3 相機→XYZ 矩陣
        cv::Mat& xyz_srgb);

    // 黑電平校正
    void blackLevelCorrection(cv::Mat& raw, float black_level);

    // 白電平校正（normalize）
    void whiteLevelNormalization(cv::Mat& raw, float white_level);

    // 簡單去噪（中值濾波示例）
    void noiseReduction(cv::Mat& raw);

    // Bayer去馬賽克
    cv::Mat demosaic(const cv::Mat& raw);

    // 色彩校正（CCM）
    cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm);

    // 白平衡（依增益調整RGB通道）
    void whiteBalanceGrayWorld(cv::Mat& img);

    // White Patch Method: 找亮點做白平衡
    void whitePatchAWB(cv::Mat& img);

    // 色調映射與Gamma校正
    void applyToneMapping(cv::Mat& img, float gamma = 1);

    // 銳化
    void sharpening(cv::Mat& img, double Sharpening_Level = 0);

    // 壓縮與輸出
    void showPreview(const cv::Mat& img, const std::string& title, double scale = 1.0, bool autoContrast = true);

    // 輔助函數
    void printMat(const cv::Mat& m, const std::string& name);
    void applyPreMul(cv::Mat& img, const std::vector<float> pre_mul);

    // 取前幾大均值
    double getBiggestMean(const cv::Mat& channel, double ratio = 0.05);

    // 完整 pipeline
    //int DoPipeline(std::string& path);

private:
    int ImgCount = 1;

    // ----------------- Bool 參數 -----------------
    std::unordered_map<std::string, bool> bool_params;

    // ----------------- Enum 參數 -----------------
    std::unordered_map<std::string, AWB_Method> enumAWB_params;
    std::unordered_map<std::string, Demosaic_Method> enumDemosaic_params;
};


int main() {

    std::string path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/short/00001_00_0.04s.ARW";
    //std::string path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/long/00001_00_10s.ARW";

    
    // cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");
    
    ISP isp; // stack 上創建，呼叫建構子

    //int ErrCode = isp.DoPipeline(path);



    int black, white, width, height;
    std::vector<float> cam_mul;
    std::vector<float> pre_mul;
    cv::Mat cam_xyz, xyz_srgb;
    cv::Mat raw32;
    cv::Mat raw16 = isp.loadRawWithLibRaw(path,
        width,
        height,
        black,                      // 輸出黑階
        white,                      // 輸出白階
        cam_mul,                    // AWB before demosaic
        pre_mul,                    // AWB after demosaic
        cam_xyz,                    // 輸出 3x3 相機→XYZ 矩陣
        xyz_srgb);


    if (raw16.empty()) {
        std::cerr << "Failed to load image!" << std::endl;
        // 可能路徑錯或檔案問題
    }
    raw16.convertTo(raw32, CV_32F);


    // 原 raw
    double scale = 0.5;
    isp.showPreview(raw32, "Origin", scale);
    //cv::imwrite("Test.tiff", raw16);

    //......................................................................................................//

    if (isp.getParamBool("DoWhiteBlackLevel"))
    {
        // 黑電平校正（假設改動 raw）
        isp.blackLevelCorrection(raw32, black);
        // 白電平校正
        isp.whiteLevelNormalization(raw32, white - black); // 因為已經扣了 blackLevel, 所以這邊也要扣除

        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(raw32, "White Level Normalization", scale);
    }



    //......................................................................................................//

    //// 去噪
    //isp.noiseReduction(raw16);
    //isp.showPreview(raw16, "Noise Reduction", scale);

    //......................................................................................................//


    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::Default)
    {
        if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::Default)
        {
            std::vector<float> cam_mul_normalized(4);
            float g_ref = cam_mul[1]; // choose G as reference

            cam_mul_normalized[0] = cam_mul[0] / g_ref; // R
            cam_mul_normalized[1] = cam_mul[1] / g_ref; // G (first G)
            cam_mul_normalized[2] = cam_mul[1] / g_ref; // second G (depends on ordering)
            cam_mul_normalized[3] = cam_mul[2] / g_ref; // B

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


            cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
            cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
            isp.showPreview(raw32, "AWB 1", scale);
        }
    }
    //......................................................................................................//

    // 去馬賽克 (Demosaic)
    cv::Mat bgr32;
    if (isp.getParamBool("DoDemosaic") && isp.getParamDemosaic() == ISP::Demosaic_Method::CCM)
    {
        bgr32 = isp.demosaic(raw32);

        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        cv::Mat flat = bgr32.reshape(1, bgr32.total() * bgr32.channels());
        std::vector<float> vals;
        flat.copyTo(vals);
        std::nth_element(vals.begin(), vals.begin() + vals.size() * 0.99, vals.end());
        float maxVal = vals[vals.size() * 0.99];
        cv::Mat bgr32_norm;
        bgr32.convertTo(bgr32_norm, CV_32F, 1.0 / maxVal);

        cv::threshold(bgr32_norm, bgr32_norm, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32_norm, bgr32_norm, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32_norm, "Demosaic_norm", scale);


        isp.showPreview(bgr32, "Demosaic", scale);
    }

    //......................................................................................................//

    // 白平衡
    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::GrayWorld)
    {
        isp.whiteBalanceGrayWorld(bgr32);
        isp.showPreview(bgr32, "GrayWorld", scale);

        isp.applyPreMul(bgr32, pre_mul);
        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32, "awb 2", scale);
    }

    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::WhitePoint)
    {
        isp.whitePatchAWB(bgr32);
        isp.showPreview(bgr32, "whitePatch", scale);
    }


    //isp.applyPreMul(bgr32, pre_mul);
    //cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
    //cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
    //isp.showPreview(bgr32, "awb 2", scale);

    //......................................................................................................//


    // 色彩校正
    if (isp.getParamBool("DoCCM"))
    {
        cv::Mat ccm = xyz_srgb * cam_xyz;

        isp.printMat(xyz_srgb, "xyz_srgb");
        isp.printMat(cam_xyz.t(), "cam_xyz");
        isp.printMat(ccm, "ccm");
        bgr32 = isp.colorCorrection(bgr32, ccm);



        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32, "Color Correction", scale);
    }


    //......................................................................................................//


    // 色調映射
    if (isp.getParamBool("DoGamma"))
    {
        isp.applyToneMapping(bgr32, 1.8f);

        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32, "Tone Mapping", scale);
    }

    //......................................................................................................//

    // 銳化

    if (isp.getParamBool("DoSharpen"))
    {
        isp.sharpening(bgr32);

        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32, "Sharpening", scale);
    }


    //......................................................................................................//

    //cv::Mat rgb8bit;
    //bgr32.convertTo(rgb8bit, CV_8U, 255.0);
    //// 儲存輸出
    //cv::imwrite("output.png", rgb8bit);


    
    return 0;
}


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

void ISP::printMat(const cv::Mat& m, const std::string& name) {
    std::cout << name << " (" << m.rows << "x" << m.cols << "):" << std::endl;
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            std::cout << m.at<float>(i, j) << "  ";
        }
        std::cout << std::endl;
    }
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
void ISP::whitePatchAWB(cv::Mat& img)
{
    CV_Assert(img.depth() == CV_32F); // 支援 32F
    CV_Assert(img.channels() == 3);   // 支援 BGR 彩色

    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    double R_mean = getBiggestMean(channels[2]);
    double G_mean = getBiggestMean(channels[1]);
    double B_mean = getBiggestMean(channels[0]);

    // G 作為參考
    double gain_R = G_mean / R_mean;
    double gain_G = 1.0;
    double gain_B = G_mean / B_mean;

    channels[2] *= gain_R;
    channels[1] *= gain_G;
    channels[0] *= gain_B;

    cv::merge(channels, img);
}

double ISP::getBiggestMean(const cv::Mat& channel, double ratio)
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

void ISP::noiseReduction(cv::Mat& raw) {
    cv::medianBlur(raw, raw, 3);
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

void ISP::whiteBalanceGrayWorld(cv::Mat& img) {
    cv::Mat img32F;
    img.convertTo(img32F, CV_32F);

    std::vector<cv::Mat> channels(3);
    cv::split(img32F, channels);

    double meanR = cv::mean(channels[2])[0];
    double meanG = cv::mean(channels[1])[0];
    double meanB = cv::mean(channels[0])[0];
    double meanGray = (meanR + meanG + meanB) / 3.0;

    double eps = 1e-6;
    double gainR = meanGray / (meanR + eps);
    double gainG = meanGray / (meanG + eps);
    double gainB = meanGray / (meanB + eps);

    channels[2] *= gainR;
    channels[1] *= gainG;
    channels[0] *= gainB;

    cv::merge(channels, img32F);

    cv::Mat img_clipped = img32F.clone();
    img32F.convertTo(img, img.type());  // 回到原本型態
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
            cv::minMaxLoc(tmp.reshape(1), &minVal, &maxVal); // reshape(1) 針對所有 channel
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

//int ISP::DoPipeline(std::string& path)
//{
//    
//    int black, white, width, height;
//    std::vector<float> cam_mul;
//    std::vector<float> pre_mul;
//    cv::Mat cam_xyz, xyz_srgb;
//    cv::Mat raw32;
//    // cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");
//    cv::Mat raw16 = ISP::loadRawWithLibRaw(path,
//        width,
//        height,
//        black,                      // 輸出黑階
//        white,                      // 輸出白階
//        cam_mul,                    // AWB before demosaic
//        pre_mul,                    // AWB after demosaic
//        cam_xyz,                    // 輸出 3x3 相機→XYZ 矩陣
//        xyz_srgb);
//
//
//    if (raw16.empty()) {
//        std::cerr << "Failed to load image!" << std::endl;
//        // 可能路徑錯或檔案問題
//    }
//    raw16.convertTo(raw32, CV_32F);
//
//
//    // 原 raw
//    double scale = 0.3;
//    ISP::showPreview(raw32, "Origin", scale);
//    //cv::imwrite("Test.tiff", raw16);
//
//    //......................................................................................................//
//
//    if (ISP::DoWhiteBlackLevel)
//    {
//        // 黑電平校正（假設改動 raw）
//        ISP::blackLevelCorrection(raw32, black);
//        // 白電平校正
//        ISP::whiteLevelNormalization(raw32, white - black); // 因為已經扣了 blackLevel, 所以這邊也要扣除
//
//        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(raw32, "White Level Normalization", scale);
//    }
//
//    
//
//    //......................................................................................................//
//
//    //// 去噪
//    //isp.noiseReduction(raw16);
//    //isp.showPreview(raw16, "Noise Reduction", scale);
//
//    //......................................................................................................//
//
//
//
//    if (ISP::DoAWB && getParamAWB() == ISP::AWB_Method::Default)
//    {
//        std::vector<float> cam_mul_normalized(4);
//        float g_ref = cam_mul[1]; // choose G as reference
//
//        cam_mul_normalized[0] = cam_mul[0] / g_ref; // R
//        cam_mul_normalized[1] = cam_mul[1] / g_ref; // G (first G)
//        cam_mul_normalized[2] = cam_mul[1] / g_ref; // second G (depends on ordering)
//        cam_mul_normalized[3] = cam_mul[2] / g_ref; // B
//
//        // 套用 AWB Gain
//        for (int y = 0; y < height; y++)
//        {
//            float* row = raw32.ptr<float>(y);
//            for (int x = 0; x < width; x++)
//            {
//                int idx = ((y & 1) << 1) | (x & 1);
//                row[x] *= cam_mul_normalized[idx];
//            }
//        }
//
//
//        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(raw32, "AWB 1", scale);
//    }
//    
//        //......................................................................................................//
//
//    // 去馬賽克 (Demosaic)
//    cv::Mat bgr32 = ISP::demosaic(raw32);
//
//    cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//    cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//    ISP::showPreview(bgr32, "Demosaic", 0.3);
//
//
//    //......................................................................................................//
//
//    // 白平衡
//    if (ISP::DoAWB && getParamAWB() == ISP::AWB_Method::GrayWorld)
//    {
//        ISP::whiteBalanceGrayWorld(bgr32);
//        ISP::showPreview(bgr32, "GrayWorld", scale);
//
//        ISP::applyPreMul(bgr32, pre_mul);
//        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(bgr32, "awb 2", scale);
//    }
//
//    if (ISP::DoAWB && getParamAWB() == ISP::AWB_Method::WhitePoint)
//    {
//        ISP::whitePatchAWB(bgr32);
//        ISP::showPreview(bgr32, "whitePatch", scale);
//    }
//
//    
//    //ISP::applyPreMul(bgr32, pre_mul);
//    //cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//    //cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//    //ISP::showPreview(bgr32, "awb 2", scale);
//
//    //......................................................................................................//
//
//
//    // 色彩校正
//    if (ISP::DoCCM)
//    {
//        cv::Mat ccm = xyz_srgb * cam_xyz;
//
//        ISP::printMat(xyz_srgb, "xyz_srgb");
//        ISP::printMat(cam_xyz.t(), "cam_xyz");
//        ISP::printMat(ccm, "ccm");
//        bgr32 = ISP::colorCorrection(bgr32, ccm);
//
//
//
//        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(bgr32, "Color Correction", scale);
//    }
//
//
//    //......................................................................................................//
//
//
//    // 色調映射
//    if (ISP::DoGamma)
//    {
//        ISP::applyToneMapping(bgr32, 1.8f);
//
//        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(bgr32, "Tone Mapping", scale);
//    }
//
//    //......................................................................................................//
//
//    // 銳化
//
//    if (ISP::DoSharpen)
//    {
//        ISP::sharpening(bgr32);
//
//        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
//        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
//        ISP::showPreview(bgr32, "Sharpening", scale);
//    }
//
//
//    //......................................................................................................//
//
//    //cv::Mat rgb8bit;
//    //bgr32.convertTo(rgb8bit, CV_8U, 255.0);
//    //// 儲存輸出
//    //cv::imwrite("output.png", rgb8bit);
//
//    
////}


