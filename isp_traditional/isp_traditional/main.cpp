#include "isp.h"
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

void printMat(const cv::Mat& m, const std::string& name);


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

    // 白平衡
    double gain_R = 0;
    double gain_G = 0;
    double gain_B = 0;
    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::GrayWorld)
    {
        // 先 Demosaic 後算 RBG Gain
        cv::Mat bgr32_tmp = isp.demosaic(raw32);
        cv::threshold(bgr32_tmp, bgr32_tmp, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32_tmp, bgr32_tmp, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.CalAWBGain_GrayWorld(bgr32_tmp, gain_R, gain_G, gain_B);


        // 套用 RBG Gain 
        isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(raw32, "AWB (Gray World)", scale);
    }

    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::WhitePoint)
    {
        // 先 Demosaic 後算 RBG Gain
        cv::Mat bgr32_tmp = isp.demosaic(raw32);
        cv::threshold(bgr32_tmp, bgr32_tmp, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32_tmp, bgr32_tmp, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.CalAWBGain_WhitePatch(bgr32_tmp, gain_R, gain_G, gain_B);

        // 套用 RBG Gain 
        isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(raw32, "AWB (White Patch)", scale);
    }

    if (isp.getParamBool("DoAWB") && isp.getParamAWB() == ISP::AWB_Method::Default)
    {
        float g_ref = cam_mul[1]; // choose G as reference
        gain_R = cam_mul[0] / g_ref; // R
        gain_G = cam_mul[1] / g_ref; // G (first G)
        gain_G = cam_mul[1] / g_ref; // second G (depends on ordering)
        gain_B = cam_mul[2] / g_ref; // B

        isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
        cv::threshold(raw32, raw32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(raw32, raw32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(raw32, "AWB (Default)", scale);
    }

    //......................................................................................................//


    // 去馬賽克 (Demosaic)
    cv::Mat bgr32;
    if (isp.getParamBool("DoDemosaic") && isp.getParamDemosaic() == ISP::Demosaic_Method::CCM)
    {
        bgr32 = isp.demosaic(raw32);

        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1

        //cv::Mat flat = bgr32.reshape(1, bgr32.total() * bgr32.channels());
        //std::vector<float> vals;
        //flat.copyTo(vals);
        //std::nth_element(vals.begin(), vals.begin() + vals.size() * 0.99, vals.end());
        //float maxVal = vals[vals.size() * 0.99];
        //cv::Mat bgr32_norm;
        //bgr32.convertTo(bgr32_norm, CV_32F, 1.0 / maxVal);

        //cv::threshold(bgr32_norm, bgr32_norm, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        //cv::threshold(bgr32_norm, bgr32_norm, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        //isp.showPreview(bgr32_norm, "Demosaic_norm", scale);

        isp.showPreview(bgr32, "Demosaic", scale);
    }

    //......................................................................................................//


    // 色彩校正
    if (isp.getParamBool("DoCCM"))
    {
        cv::Mat ccm = xyz_srgb * cam_xyz;

        printMat(xyz_srgb, "xyz_srgb");
        printMat(cam_xyz.t(), "cam_xyz");
        printMat(ccm, "ccm");
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

void printMat(const cv::Mat& m, const std::string& name) {
    std::cout << name << " (" << m.rows << "x" << m.cols << "):" << std::endl;
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            std::cout << m.at<float>(i, j) << "  ";
        }
        std::cout << std::endl;
    }
}




