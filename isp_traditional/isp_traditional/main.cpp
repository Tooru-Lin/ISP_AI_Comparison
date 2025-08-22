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

// Bayer去馬賽克
cv::Mat demosaic(const cv::Mat& raw);

// 色彩校正（CCM）
cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm);

// 白平衡（依增益調整RGB通道）
void whiteBalance(cv::Mat& img, float gainR, float gainG, float gainB);

// 色調映射與Gamma校正
void toneMapping(cv::Mat& img, float gamma);

// 銳化
void sharpening(cv::Mat& img);

// 壓縮與輸出
void showPreview(const cv::Mat& img, const std::string& title, double scale);

int main() {

    
    // cv::Mat raw = loadRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    cv::Mat raw = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");
    //cv::Mat raw = loadRawWithLibRawRGB("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    
    if (raw.empty()) {
        std::cerr << "Failed to load image!" << std::endl;
        // 可能路徑錯或檔案問題
    }

    // 原 raw
    double scale = 0.3;
    cv::Mat raw32F;
    raw.convertTo(raw32F, CV_32F);
    //showPreview(raw32F, "Raw 8-bit", scale);

    //......................................................................................................//

    // 將 16bit raw 展平成單行，找出1%和99%亮度位置
    cv::Mat sorted;
    raw32F.reshape(1, 1).copyTo(sorted);
    cv::sort(sorted, sorted, cv::SORT_ASCENDING);

    // 取第 1 百分位的值（忽略最暗的 1%）
    int idx_min1 = static_cast<int>(sorted.total() * 0.01);
    float blackLevel = sorted.at<float>(idx_min1);
    std::cout << "Estimated black level (1th percentile): " << blackLevel << std::endl;

    // 取第 99 百分位的值（忽略最亮的 1%）
    int idx_max99 = static_cast<int>(sorted.total() * 0.99);
    float whiteLevel = sorted.at<float>(idx_max99);
    std::cout << "Estimated white level (99th percentile): " << whiteLevel << std::endl;

    //......................................................................................................//

    // 黑電平校正（假設改動 raw）
    blackLevelCorrection(raw32F, blackLevel);
    //showPreview(raw32F, "After Black Level Correction", 0.3);
    
    //......................................................................................................//

    // 白電平校正
    whiteLevelNormalization(raw32F, whiteLevel - blackLevel); // 因為已經扣了 blackLevel, 所以這邊也要扣除
    //showPreview(raw32F, "After White Level Normalization", 0.3);

    //......................................................................................................//
    
    // 去噪
    //noiseReduction(raw32F);
    //showPreview(raw32F, "After Noise Reduction", 0.3);

    //......................................................................................................//

    // 去馬賽克 (Demosaic)
    cv::Mat rgb = demosaic(raw32F);
    showPreview(rgb, "After Demosaic", 0.3);

    //......................................................................................................//

    // 色彩校正
    cv::Mat ccm = (cv::Mat_<float>(3, 3) << 1.5, -0.5, 0, -0.3, 1.2, 0.1, 0, -0.2, 1.3);
    rgb = colorCorrection(rgb, ccm);
    showPreview(rgb, "After Color Correction", 0.3);

    //......................................................................................................//

    // 白平衡
    whiteBalance(rgb, 2.0f, 1.0f, 1.5f);
    showPreview(rgb, "After White Balance", 0.3);

    //......................................................................................................//
    
    // 色調映射
    toneMapping(rgb, 2.2f);
    showPreview(rgb, "After Tone Mapping", 0.3);

    //......................................................................................................//

    // 銳化
    sharpening(rgb);
    showPreview(rgb, "After Sharpening", 0.3);

    //......................................................................................................//

    cv::Mat rgb8bit;
    rgb.convertTo(rgb8bit, CV_8U, 255.0);
    // 儲存輸出
    cv::imwrite("output.png", rgb8bit);

    return 0;
}


cv::Mat loadRawWithLibRaw(const std::string& filename) {
    
    auto RawProcessor = std::make_unique<LibRaw>();
    int ret = RawProcessor->open_file(filename.c_str());
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot open file: " << filename << " Error: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    ret = RawProcessor->unpack();  // 解包 RAW 數據
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot unpack raw data: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    // CFA pattern
    std::cout << "CFA description: "
        << RawProcessor->imgdata.idata.cdesc << std::endl;

    int pattern = RawProcessor->imgdata.idata.filters; // pattern == 9 -> RGGB

    // 獲取圖像寬高
    int width = RawProcessor->imgdata.sizes.width;
    int height = RawProcessor->imgdata.sizes.height;

    // Bayer RAW 通常是16-bit
    // RawProcessor.imgdata.rawdata.raw_image 是指向 uint16_t* 的指標

    // 將原始 Bayer 資料複製到 cv::Mat (單通道16位)
    cv::Mat rawImage(height, width, CV_16UC1);
    memcpy(rawImage.data, RawProcessor->imgdata.rawdata.raw_image, width * height * sizeof(uint16_t));

    return rawImage;
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
    raw /= white_level;  // 直接縮放
}

void noiseReduction(cv::Mat& raw) {
    cv::medianBlur(raw, raw, 3);
}

cv::Mat demosaic(const cv::Mat& raw) {
    cv::Mat raw16;
    cv::Mat rgb16;
    cv::Mat rgb32F;
    raw.convertTo(raw16, CV_16U, 65535.0);
    cv::cvtColor(raw16, rgb16, cv::COLOR_BayerRG2BGR);  // BayerBG視你的pattern而定
    rgb16.convertTo(rgb32F, CV_32F, 1.0 / 65535.0); // 16-bit -> 0~1
    return rgb32F;
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

void whiteBalance(cv::Mat& img, float gainR, float gainG, float gainB) {
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);
    channels[2] *= gainR; // R
    channels[1] *= gainG; // G
    channels[0] *= gainB; // B
    cv::merge(channels, img);
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

void showPreview(const cv::Mat& raw32, const std::string& title, double scale = 1.0) {
    cv::Mat raw8;
    cv::Mat temp;
    cv::Mat preview;
    //double minVal, maxVal;
    //cv::minMaxLoc(raw32, &minVal, &maxVal);
    raw32.convertTo(temp, CV_8U, 255.0); // scale 到 8-bit, 不然會太暗或太亮

    if (scale != 1.0) {
        cv::resize(temp, preview, cv::Size(), scale, scale);
    }
    else 
    {
        preview = temp;
    }
    cv::imshow(title, preview);
    cv::waitKey(0);
}


