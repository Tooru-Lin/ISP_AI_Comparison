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

// Bayer�h���ɧJ
cv::Mat demosaic(const cv::Mat& raw);

// ��m�ե��]CCM�^
cv::Mat colorCorrection(const cv::Mat& img, const cv::Mat& ccm);

// �ե��š]�̼W�q�վ�RGB�q�D�^
void whiteBalance(cv::Mat& img, float gainR, float gainG, float gainB);

// ��լM�g�PGamma�ե�
void toneMapping(cv::Mat& img, float gamma);

// �U��
void sharpening(cv::Mat& img);

// ���Y�P��X
void showPreview(const cv::Mat& img, const std::string& title, double scale);

int main() {

    
    // cv::Mat raw = loadRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    cv::Mat raw = loadRawWithLibRaw("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF");
    //cv::Mat raw = loadRawWithLibRawRGB("C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF");
    
    if (raw.empty()) {
        std::cerr << "Failed to load image!" << std::endl;
        // �i����|�����ɮװ��D
    }

    // �� raw
    double scale = 0.3;
    cv::Mat raw32F;
    raw.convertTo(raw32F, CV_32F);
    //showPreview(raw32F, "Raw 8-bit", scale);

    //......................................................................................................//

    // �N 16bit raw �i�������A��X1%�M99%�G�צ�m
    cv::Mat sorted;
    raw32F.reshape(1, 1).copyTo(sorted);
    cv::sort(sorted, sorted, cv::SORT_ASCENDING);

    // ���� 1 �ʤ��쪺�ȡ]�����̷t�� 1%�^
    int idx_min1 = static_cast<int>(sorted.total() * 0.01);
    float blackLevel = sorted.at<float>(idx_min1);
    std::cout << "Estimated black level (1th percentile): " << blackLevel << std::endl;

    // ���� 99 �ʤ��쪺�ȡ]�����̫G�� 1%�^
    int idx_max99 = static_cast<int>(sorted.total() * 0.99);
    float whiteLevel = sorted.at<float>(idx_max99);
    std::cout << "Estimated white level (99th percentile): " << whiteLevel << std::endl;

    //......................................................................................................//

    // �¹q���ե��]���]��� raw�^
    blackLevelCorrection(raw32F, blackLevel);
    //showPreview(raw32F, "After Black Level Correction", 0.3);
    
    //......................................................................................................//

    // �չq���ե�
    whiteLevelNormalization(raw32F, whiteLevel - blackLevel); // �]���w�g���F blackLevel, �ҥH�o��]�n����
    //showPreview(raw32F, "After White Level Normalization", 0.3);

    //......................................................................................................//
    
    // �h��
    //noiseReduction(raw32F);
    //showPreview(raw32F, "After Noise Reduction", 0.3);

    //......................................................................................................//

    // �h���ɧJ (Demosaic)
    cv::Mat rgb = demosaic(raw32F);
    showPreview(rgb, "After Demosaic", 0.3);

    //......................................................................................................//

    // ��m�ե�
    cv::Mat ccm = (cv::Mat_<float>(3, 3) << 1.5, -0.5, 0, -0.3, 1.2, 0.1, 0, -0.2, 1.3);
    rgb = colorCorrection(rgb, ccm);
    showPreview(rgb, "After Color Correction", 0.3);

    //......................................................................................................//

    // �ե���
    whiteBalance(rgb, 2.0f, 1.0f, 1.5f);
    showPreview(rgb, "After White Balance", 0.3);

    //......................................................................................................//
    
    // ��լM�g
    toneMapping(rgb, 2.2f);
    showPreview(rgb, "After Tone Mapping", 0.3);

    //......................................................................................................//

    // �U��
    sharpening(rgb);
    showPreview(rgb, "After Sharpening", 0.3);

    //......................................................................................................//

    cv::Mat rgb8bit;
    rgb.convertTo(rgb8bit, CV_8U, 255.0);
    // �x�s��X
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

    ret = RawProcessor->unpack();  // �ѥ] RAW �ƾ�
    if (ret != LIBRAW_SUCCESS) {
        std::cerr << "Cannot unpack raw data: " << libraw_strerror(ret) << std::endl;
        return cv::Mat();
    }

    // CFA pattern
    std::cout << "CFA description: "
        << RawProcessor->imgdata.idata.cdesc << std::endl;

    int pattern = RawProcessor->imgdata.idata.filters; // pattern == 9 -> RGGB

    // ����Ϲ��e��
    int width = RawProcessor->imgdata.sizes.width;
    int height = RawProcessor->imgdata.sizes.height;

    // Bayer RAW �q�`�O16-bit
    // RawProcessor.imgdata.rawdata.raw_image �O���V uint16_t* ������

    // �N��l Bayer ��ƽƻs�� cv::Mat (��q�D16��)
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
    raw /= white_level;  // �����Y��
}

void noiseReduction(cv::Mat& raw) {
    cv::medianBlur(raw, raw, 3);
}

cv::Mat demosaic(const cv::Mat& raw) {
    cv::Mat raw16;
    cv::Mat rgb16;
    cv::Mat rgb32F;
    raw.convertTo(raw16, CV_16U, 65535.0);
    cv::cvtColor(raw16, rgb16, cv::COLOR_BayerRG2BGR);  // BayerBG���A��pattern�өw
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
    raw32.convertTo(temp, CV_8U, 255.0); // scale �� 8-bit, ���M�|�ӷt�ΤӫG

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


