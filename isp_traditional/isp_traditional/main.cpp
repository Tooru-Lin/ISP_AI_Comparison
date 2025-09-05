#include <iostream>
#include <opencv2/opencv.hpp>
#include <libraw/libraw.h>



cv::Mat loadRawWithLibRaw(const std::string& filename);

// ���JRaw�]�ܽd�H��q�D16 - bit TIFF��RAW��Ū����Mat�^
cv::Mat loadRawWithLibRawRGB(const std::string& filename);

// �¹q���ե�
void blackLevelCorrection(cv::Mat& raw, float black_level);

// �չq���ե��]normalize�^
void whiteLevelNormalization(cv::Mat& raw, float white_level);

// ²��h���]�����o�i�ܨҡ^
void noiseReduction(cv::Mat& raw);

cv::Mat reorderBayer_RGGB(const cv::Mat& raw);

// Bayer�h���ɧJ
cv::Mat demosaic(const cv::Mat& raw);

// ��m�ե��]CCM�^
cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm);

// �ե��š]�̼W�q�վ�RGB�q�D�^
void whiteBalanceGrayWorld(cv::Mat& img);

// ��լM�g�PGamma�ե�
void toneMapping(cv::Mat& img, float gamma);

// �U��
void sharpening(cv::Mat& img);

// ���Y�P��X
void showPreview(const cv::Mat& img, const std::string& title, double scale);

int main() {

    
    // cv::Mat raw = loadRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    // cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");

    cv::Mat raw16 = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/long/00001_00_10s.ARW");
    
    if (raw16.empty()) {
        std::cerr << "Failed to load image!" << std::endl;
        // �i����|�����ɮװ��D
    }

    // �� raw
    double scale = 0.3;
    showPreview(raw16, "Raw 8-bit", scale);
    cv::imwrite("Test.tiff", raw16);
    //......................................................................................................//

    // �N 16bit raw �i�������A��X1%�M99%�G�צ�m
    cv::Mat sorted;
    raw16.reshape(1, 1).copyTo(sorted);
    cv::sort(sorted, sorted, cv::SORT_ASCENDING);

    // ���� 1 �ʤ��쪺�ȡ]�����̷t�� 1%�^
    int idx_min1 = static_cast<int>(sorted.total() * 0.01);
    uint16_t blackLevel = sorted.at<uint16_t>(idx_min1);
    std::cout << "Estimated black level (1th percentile): " << blackLevel << std::endl;

    // ���� 99 �ʤ��쪺�ȡ]�����̫G�� 1%�^
    int idx_max99 = static_cast<int>(sorted.total() * 0.99);
    uint16_t whiteLevel = sorted.at<uint16_t>(idx_max99);
    std::cout << "Estimated white level (99th percentile): " << whiteLevel << std::endl;

    //......................................................................................................//

    // �¹q���ե��]���]��� raw�^
    blackLevelCorrection(raw16, blackLevel);
    showPreview(raw16, "After Black Level Correction", 0.3);
    
    //......................................................................................................//

    // �չq���ե�
    whiteLevelNormalization(raw16, whiteLevel - blackLevel); // �]���w�g���F blackLevel, �ҥH�o��]�n����
    showPreview(raw16, "After White Level Normalization", 0.3);
    cv::imwrite("whiteLevel.tiff", raw16);
    
    //......................................................................................................//
    
    // �h��
    //noiseReduction(raw16);
    //showPreview(raw16, "After Noise Reduction", 0.3);

    //......................................................................................................//

    // �h���ɧJ (Demosaic)
    cv::Mat bgr = demosaic(raw16);
    showPreview(bgr, "After Demosaic", 0.3);

    cv::Mat bgr32;
    bgr.convertTo(bgr32, CV_32F, 65535);
    //......................................................................................................//

    // �ե���
    whiteBalanceGrayWorld(bgr32);
    showPreview(bgr32, "After White Balance", 0.3);

    //......................................................................................................//

    // ��m�ե�
    cv::Mat ccm = (cv::Mat_<float>(3, 3) << 1.5, -0.5, 0, -0.3, 1.2, 0.1, 0, -0.2, 1.3);
    bgr32 = colorCorrection(bgr, ccm);
    showPreview(bgr32, "After Color Correction", 0.3);

    //......................................................................................................//
    
    // ��լM�g
    toneMapping(bgr32, 2.2f);
    showPreview(bgr32, "After Tone Mapping", 0.3);

    //......................................................................................................//

    // �U��
    sharpening(bgr32);
    showPreview(bgr32, "After Sharpening", 0.3);

    //......................................................................................................//

    cv::Mat rgb8bit;
    bgr32.convertTo(rgb8bit, CV_8U, 255.0);
    // �x�s��X
    cv::imwrite("output.png", rgb8bit);

    return 0;
}


cv::Mat loadRawWithLibRaw(const std::string& filename) {
    
    LibRaw RawProcessor;  // stack �W������
    int ret = RawProcessor.open_file(filename.c_str());
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot open file: " << filename << " Error: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    ret = RawProcessor.unpack();  // �ѥ] RAW �ƾ�
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
    int width = RawProcessor.imgdata.sizes.width;// ���ļe��
    int height = RawProcessor.imgdata.sizes.height;//���İ���

    int left = (raw_width - width) / 2; // �}�l�C
    int top = (raw_height - height) / 2; // �}�l��

    cv::Mat raw16(height, width, CV_16U);

    // ����������A�u�� active area
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

    // �� LibRaw �ۤv�� demosaic & ��m�ഫ
    ret = RawProcessor->dcraw_process();
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot process raw data" << std::endl;
        return cv::Mat();
    }

    // ���o�B�z�᪺�v��
    libraw_processed_image_t* image = RawProcessor->dcraw_make_mem_image();

    if (!image) {
        std::cerr << "Cannot make memory image" << std::endl;
        return cv::Mat();
    }

    // �ƻs�X�ӡ]OpenCV �ۤv�޲z�O����^
    cv::Mat rgbCopy(image->height, image->width, CV_8UC3);
    memcpy(rgbCopy.data, image->data, image->height * image->width * 3);

    LibRaw::dcraw_clear_mem(image);

    return rgbCopy;
}

void blackLevelCorrection(cv::Mat& raw, float black_level) {
    raw -= black_level;                           // ������A�O�d�쫬�O
    cv::threshold(raw, raw, 0, 0, cv::THRESH_TOZERO); // �קK�t��
}


void whiteLevelNormalization(cv::Mat& raw, float white_level) {
    double minVal, maxVal;
    cv::minMaxLoc(raw, &minVal, &maxVal);
    
    raw = raw / white_level * 65535;           // �Y��
    cv::threshold(raw, raw, 65535, 65535, cv::THRESH_TRUNC); // �j��1�]��1
    cv::minMaxLoc(raw, &minVal, &maxVal);
}

void noiseReduction(cv::Mat& raw) {
    cv::medianBlur(raw, raw, 3);
}

cv::Mat demosaic(const cv::Mat& rawIn) {

    cv::Mat raw16;

    // �p�G��J�O float (0~1)�A�ন 16-bit (0~65535)
    if (rawIn.type() == CV_32F) {
        cv::Mat clipped;
        cv::threshold(rawIn, clipped, 1.0, 1.0, cv::THRESH_TRUNC);   // >1 �� 1
        cv::threshold(clipped, clipped, 0.0, 0.0, cv::THRESH_TOZERO); // <0 �� 0
        clipped.convertTo(raw16, CV_16U, 65535.0);
    }
    else if (rawIn.type() == CV_16U) {
        raw16 = rawIn.clone();  // �w�g�O 16-bit�A���ݭn�A�Y��
    }
    else {
        throw std::runtime_error("Unsupported input type: only CV_32F or CV_16U allowed.");
    }

    cv::Mat rgb16;
    cv::cvtColor(raw16, rgb16, cv::COLOR_BayerRG2BGR);  // Bayer pattern���A��sensor�өw

    return rgb16; // 16U, �C�ӳq�D0~1
}

// �N raw �q R G / B G �ন R G / G B
cv::Mat reorderBayer_RGGB(const cv::Mat& raw32F) {
    CV_Assert(raw32F.channels() == 1);

    cv::Mat reordered = raw32F.clone();
    int rows = raw32F.rows;
    int cols = raw32F.cols;

    for (int y = 1; y < rows; y += 2) {       // �C�j�@�� (�ĤG�C�B�ĥ|�C...)
        for (int x = 0; x < cols; x += 2) {   // �C�j��C
            // �洫�ĤG�C�� B �M G
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
    img32F.convertTo(img, img.type());  // �^��쥻���A
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


