#include <iostream>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>



cv::Mat loadRawWithLibRaw(const std::string& filename);

// 載入Raw（示範以單通道16 - bit TIFF或RAW檔讀取成Mat）
cv::Mat loadRawWithLibRawRGB(const std::string& filename);

// 黑電平校正
void blackLevelCorrection(cv::Mat& raw, float black_level);

// 白電平校正（normalize）
void whiteLevelNormalization(cv::Mat& raw, float white_level);

// 簡單去噪（中值濾波示例）
void noiseReduction(cv::Mat& raw);

cv::Mat reorderBayer_RGGB(const cv::Mat& raw);

// Bayer去馬賽克
cv::Mat demosaic(const cv::Mat& raw);

// 色彩校正（CCM）
cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm);

// 白平衡（依增益調整RGB通道）
void whiteBalanceGrayWorld(cv::Mat& img);

// 色調映射與Gamma校正
void toneMapping(cv::Mat& img, float gamma);

// 銳化
void sharpening(cv::Mat& img);

// 壓縮與輸出
void showPreview(const cv::Mat& img, const std::string& title, double scale);

int main() {

    
    // cv::Mat raw = loadRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    // cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");

    cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/long/00001_00_10s.ARW");
    
    if (raw16.empty()) {
        std::cerr << "Failed to load image!" << std::endl;
        // 可能路徑錯或檔案問題
    }

    // 原 raw
    double scale = 0.3;
    showPreview(raw16, "Raw 8-bit", scale);
    cv::imwrite("Test.tiff", raw16);
    //......................................................................................................//

    // 將 16bit raw 展平成單行，找出1%和99%亮度位置
    cv::Mat sorted;
    raw16.reshape(1, 1).copyTo(sorted);
    cv::sort(sorted, sorted, cv::SORT_ASCENDING);

    // 取第 1 百分位的值（忽略最暗的 1%）
    int idx_min1 = static_cast<int>(sorted.total() * 0.01);
    uint16_t blackLevel = sorted.at<uint16_t>(idx_min1);
    std::cout << "Estimated black level (1th percentile): " << blackLevel << std::endl;

    // 取第 99 百分位的值（忽略最亮的 1%）
    int idx_max99 = static_cast<int>(sorted.total() * 0.99);
    uint16_t whiteLevel = sorted.at<uint16_t>(idx_max99);
    std::cout << "Estimated white level (99th percentile): " << whiteLevel << std::endl;

    //......................................................................................................//

    // 黑電平校正（假設改動 raw）
    blackLevelCorrection(raw16, blackLevel);
    showPreview(raw16, "After Black Level Correction", 0.3);
    
    //......................................................................................................//

    // 白電平校正
    whiteLevelNormalization(raw16, whiteLevel - blackLevel); // 因為已經扣了 blackLevel, 所以這邊也要扣除
    showPreview(raw16, "After White Level Normalization", 0.3);
    cv::imwrite("whiteLevel.tiff", raw16);
    
    //......................................................................................................//
    
    // 去噪
    //noiseReduction(raw16);
    //showPreview(raw16, "After Noise Reduction", 0.3);

    //......................................................................................................//

    // 去馬賽克 (Demosaic)
    cv::Mat bgr = demosaic(raw16);
    showPreview(bgr, "After Demosaic", 0.3);

    cv::Mat bgr32;
    bgr.convertTo(bgr32, CV_32F, 65535);
    //......................................................................................................//

    // 白平衡
    whiteBalanceGrayWorld(bgr32);
    showPreview(bgr32, "After White Balance", 0.3);

    //......................................................................................................//

    // 色彩校正
    cv::Mat ccm = (cv::Mat_<float>(3, 3) << 1.5, -0.5, 0, -0.3, 1.2, 0.1, 0, -0.2, 1.3);
    bgr32 = colorCorrection(bgr, ccm);
    showPreview(bgr32, "After Color Correction", 0.3);

    //......................................................................................................//
    
    // 色調映射
    toneMapping(bgr32, 2.2f);
    showPreview(bgr32, "After Tone Mapping", 0.3);

    //......................................................................................................//

    // 銳化
    sharpening(bgr32);
    showPreview(bgr32, "After Sharpening", 0.3);

    //......................................................................................................//

    cv::Mat rgb8bit;
    bgr32.convertTo(rgb8bit, CV_8U, 255.0);
    // 儲存輸出
    cv::imwrite("output.png", rgb8bit);

    return 0;
}


cv::Mat loadRawWithLibRaw(const std::string& filename) {
    
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
            std::cout << RawProcessor.imgdata.idata.cdesc[idx] << " ";
        }
        std::cout << std::endl;
    }


    
    int raw_width = RawProcessor.imgdata.sizes.raw_width;
    int raw_height = RawProcessor.imgdata.sizes.raw_height;
    int width = RawProcessor.imgdata.sizes.width;// 有效寬度
    int height = RawProcessor.imgdata.sizes.height;//有效高度

    int left = (raw_width - width) / 2; // 開始列
    int top = (raw_height - height) / 2; // 開始行

    cv::Mat raw16(height, width, CV_16U);

    // 拷貝掉黑邊，只取 active area
    for (int y = 0; y < height; y++) {
        memcpy(raw16.ptr<ushort>(y),
            RawProcessor.imgdata.rawdata.raw_image + (y + top) * raw_width + left,
            width * sizeof(ushort));
    }

    return raw16;
}

cv::Mat loadRawWithLibRawRGB(const std::string& filename) {
    std::unique_ptr<LibRaw> RawProcessor = std::make_unique<LibRaw>();
    int ret = RawProcessor->open_file(filename.c_str());
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return cv::Mat();
    }

    ret = RawProcessor->unpack();
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot unpack raw data" << std::endl;
        return cv::Mat();
    }

    // 讓 LibRaw 自己做 demosaic & 色彩轉換
    ret = RawProcessor->dcraw_process();
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot process raw data" << std::endl;
        return cv::Mat();
    }

    // 取得處理後的影像
    libraw_processed_image_t* image = RawProcessor->dcraw_make_mem_image();

    if (!image) {
        std::cerr << "Cannot make memory image" << std::endl;
        return cv::Mat();
    }

    // 複製出來（OpenCV 自己管理記憶體）
    cv::Mat rgbCopy(image->height, image->width, CV_8UC3);
    memcpy(rgbCopy.data, image->data, image->height * image->width * 3);

    LibRaw::dcraw_clear_mem(image);

    return rgbCopy;
}

void blackLevelCorrection(cv::Mat& raw, float black_level) {
    raw -= black_level;                           // 直接減，保留原型別
    cv::threshold(raw, raw, 0, 0, cv::THRESH_TOZERO); // 避免負值
}


void whiteLevelNormalization(cv::Mat& raw, float white_level) {
    double minVal, maxVal;
    cv::minMaxLoc(raw, &minVal, &maxVal);
    
    raw = raw / white_level * 65535;           // 縮放
    cv::threshold(raw, raw, 65535, 65535, cv::THRESH_TRUNC); // 大於1設為1
    cv::minMaxLoc(raw, &minVal, &maxVal);
}

void noiseReduction(cv::Mat& raw) {
    cv::medianBlur(raw, raw, 3);
}

cv::Mat demosaic(const cv::Mat& rawIn) {

    cv::Mat raw16;

    // 如果輸入是 float (0~1)，轉成 16-bit (0~65535)
    if (rawIn.type() == CV_32F) {
        cv::Mat clipped;
        cv::threshold(rawIn, clipped, 1.0, 1.0, cv::THRESH_TRUNC);   // >1 變 1
        cv::threshold(clipped, clipped, 0.0, 0.0, cv::THRESH_TOZERO); // <0 變 0
        clipped.convertTo(raw16, CV_16U, 65535.0);
    }
    else if (rawIn.type() == CV_16U) {
        raw16 = rawIn.clone();  // 已經是 16-bit，不需要再縮放
    }
    else {
        throw std::runtime_error("Unsupported input type: only CV_32F or CV_16U allowed.");
    }

    cv::Mat rgb16;
    cv::cvtColor(raw16, rgb16, cv::COLOR_BayerRG2BGR);  // Bayer pattern視你的sensor而定

    return rgb16; // 16U, 每個通道0~1
}

// 將 raw 從 R G / B G 轉成 R G / G B
cv::Mat reorderBayer_RGGB(const cv::Mat& raw32F) {
    CV_Assert(raw32F.channels() == 1);

    cv::Mat reordered = raw32F.clone();
    int rows = raw32F.rows;
    int cols = raw32F.cols;

    for (int y = 1; y < rows; y += 2) {       // 每隔一行 (第二列、第四列...)
        for (int x = 0; x < cols; x += 2) {   // 每隔兩列
            // 交換第二列的 B 和 G
            std::swap(reordered.at<uint16_t>(y, x), reordered.at<uint16_t>(y, x + 1));
        }
    }

    return reordered;
}

cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm) {
    cv::Mat img_f;
    img.convertTo(img_f, CV_32F);
    cv::Mat corrected = cv::Mat::zeros(img.size(), img.type());

    for (int y = 0; y < img.rows; y++) {
        for (int x = 0; x < img.cols; x++) {
            cv::Vec3f pix = img_f.at<cv::Vec3f>(y, x);
            cv::Vec3f new_pix;
            new_pix[0] = pix.dot(ccm.row(0));
            new_pix[1] = pix.dot(ccm.row(1));
            new_pix[2] = pix.dot(ccm.row(2));
            corrected.at<cv::Vec3f>(y, x) = new_pix;
        }
    }
    corrected.convertTo(corrected, img.type());
    return corrected;
}

void whiteBalanceGrayWorld(cv::Mat& img) {
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
    img32F.convertTo(img, img.type());  // 回到原本型態
}

void toneMapping(cv::Mat& img, float gamma) {
    cv::Mat img_float;
    img.convertTo(img_float, CV_32F, 1.0 / 255.0);
    cv::pow(img_float, 1.0 / gamma, img_float);
    img_float.convertTo(img, CV_8UC3, 255.0);
}

void sharpening(cv::Mat& img) {
    cv::Mat blurred;
    cv::GaussianBlur(img, blurred, cv::Size(0, 0), 3);
    cv::addWeighted(img, 1.5, blurred, -0.5, 0, img);
}

void showPreview(const cv::Mat& raw16, const std::string& title, double scale = 1.0) {
    cv::Mat raw8;
    cv::Mat temp8;
    cv::Mat preview;
    //double minVal, maxVal;
    //cv::minMaxLoc(raw32, &minVal, &maxVal);
    cv::normalize(raw16, temp8, 0, 255, cv::NORM_MINMAX, CV_8U);

    if (scale != 1.0) 
    {
        cv::resize(temp8, preview, cv::Size(), scale, scale);
    }
    else 
    {
        preview = temp8;
    }
    cv::imshow(title, preview);
    cv::waitKey(0);
}


