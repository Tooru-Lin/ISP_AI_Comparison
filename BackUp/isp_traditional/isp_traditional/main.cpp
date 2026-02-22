#include "isp.h"
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

void printMat(const cv::Mat& m, const std::string& name);


int main() {

    std::string path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/short/00001_00_0.04s.ARW";
    //std::string path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Sony/Sony/long/00001_00_10s.ARW";


    ISP isp; // stack 上創建，呼叫建構子

    int black, white, width, height;
    std::vector<float> cam_mul;
    std::vector<float> pre_mul;
    cv::Mat cam_xyz, xyz_srgb;
    cv::Mat raw32;

    // 使用 ErrCode 回傳
    ISP::ErrCode ec = isp.loadRawWithLibRaw(path,
        width,
        height,
        black,                      // 輸出黑階
        white,                      // 輸出白階
        cam_mul,                    // AWB before demosaic
        pre_mul,                    // AWB after demosaic
        cam_xyz,                    // 輸出 3x3 相機→XYZ 矩陣
        xyz_srgb,
        raw32);

    if (ec != ISP::ErrCode::Ok || raw32.empty()) {
        std::cerr << "Failed to load raw file. ErrCode=" << static_cast<int>(ec) << std::endl;
        return -1;
    }

    // raw32 已為 CV_32F
    // 原 raw
    double scale = 0.5;
    isp.showPreview(raw32, "Origin", scale);

    //......................................................................................................//

    if (isp.getParamBool("DoWhiteBlackLevel"))
    {
        // 白電平校正
        ec = isp.BlackAndWhiteLevelCorrection(raw32, black, white);
        if (ec != ISP::ErrCode::Ok) {
            std::cerr << "BlackAndWhiteLevelCorrection failed with ErrCode=" << static_cast<int>(ec) << std::endl;
        }

        isp.showPreview(raw32, "White Level Normalization", scale);
    }


    //......................................................................................................//
    // 白平衡

    if (isp.getParamBool("DoAWB"))
    {
        double gain_R = 0;
        double gain_G = 0;
        double gain_B = 0;
        cv::Mat bgr32_tmp; // 在 switch 外初始化

        switch (isp.getParamAWB())
        {
        case ISP::AWB_Method::Default:
        {
            float g_ref = cam_mul[1]; // choose G as reference
            gain_R = cam_mul[0] / g_ref; // R
            gain_G = cam_mul[1] / g_ref; // G (first G)
            gain_G = cam_mul[1] / g_ref; // second G (depends on ordering)
            gain_B = cam_mul[2] / g_ref; // B

            ec = isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
            if (ec != ISP::ErrCode::Ok) {
                std::cerr << "ApplyAWBGain failed with ErrCode=" << static_cast<int>(ec) << std::endl;
            }

            isp.showPreview(raw32, "AWB (Default)", scale);
        }
        break;
        case ISP::AWB_Method::GrayWorld:
        {
            // 先 Demosaic 後算 RBG Gain
            ec = isp.demosaic(raw32, bgr32_tmp);
            if (ec == ISP::ErrCode::Ok) {
                ec = isp.CalAWBGain_GrayWorld(bgr32_tmp, gain_R, gain_G, gain_B);
                if (ec == ISP::ErrCode::Ok) {
                    // 套用 RBG Gain 
                    ec = isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
                    if (ec == ISP::ErrCode::Ok) {
                        isp.showPreview(raw32, "AWB (Gray World)", scale);
                    }
                    else {
                        std::cerr << "ApplyAWBGain failed with ErrCode=" << static_cast<int>(ec) << std::endl;
                    }
                }
                else {
                    std::cerr << "CalAWBGain_GrayWorld failed with ErrCode=" << static_cast<int>(ec) << std::endl;
                }
            }
            else {
                std::cerr << "Demosaic failed for GrayWorld AWB with ErrCode=" << static_cast<int>(ec) << std::endl;
            }
        }
        break;

        case ISP::AWB_Method::WhitePoint:
        {
            // 先 Demosaic 後算 RBG Gain
            ec = isp.demosaic(raw32, bgr32_tmp);
            if (ec == ISP::ErrCode::Ok) {
                ec = isp.CalAWBGain_WhitePatch(bgr32_tmp, gain_R, gain_G, gain_B);
                if (ec == ISP::ErrCode::Ok) {
                    // 套用 RBG Gain 
                    ec = isp.ApplyAWBGain(raw32, height, width, gain_R, gain_G, gain_B);
                    if (ec == ISP::ErrCode::Ok) {
                        isp.showPreview(raw32, "AWB (White Patch)", scale);
                    }
                    else {
                        std::cerr << "ApplyAWBGain failed with ErrCode=" << static_cast<int>(ec) << std::endl;
                    }
                }
                else {
                    std::cerr << "CalAWBGain_WhitePatch failed with ErrCode=" << static_cast<int>(ec) << std::endl;
                }
            }
            else {
                std::cerr << "Demosaic failed for WhitePoint AWB with ErrCode=" << static_cast<int>(ec) << std::endl;
            }
        }
        break;
        default:
            break;
        }
    }


    //......................................................................................................//


    // 去馬賽克 (Demosaic)
    cv::Mat bgr32;
    if (isp.getParamBool("DoDemosaic") && isp.getParamDemosaic() == ISP::Demosaic_Method::CCM)
    {
        ec = isp.demosaic(raw32, bgr32);
        if (ec != ISP::ErrCode::Ok) {
            std::cerr << "Demosaic failed with ErrCode=" << static_cast<int>(ec) << std::endl;
            return -1;
        }

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

        cv::Mat bgr32_cc;
        ec = isp.colorCorrection(bgr32, ccm, bgr32_cc);
        if (ec == ISP::ErrCode::Ok) {
            bgr32 = bgr32_cc;
            isp.showPreview(bgr32, "Color Correction", scale);
        }
        else {
            std::cerr << "Color correction failed with ErrCode=" << static_cast<int>(ec) << std::endl;
        }
    }


    //......................................................................................................//


    // 色調映射
    if (isp.getParamBool("DoGamma"))
    {
        ec = isp.applyToneMapping(bgr32, 1.8f);
        if (ec != ISP::ErrCode::Ok) {
            std::cerr << "applyToneMapping failed with ErrCode=" << static_cast<int>(ec) << std::endl;
        }

        cv::threshold(bgr32, bgr32, 0.0, 0.0, cv::THRESH_TOZERO); // clip <0
        cv::threshold(bgr32, bgr32, 1.0, 1.0, cv::THRESH_TRUNC);  // clip >1
        isp.showPreview(bgr32, "Tone Mapping", scale);
    }

    //......................................................................................................//

    // 銳化

    if (isp.getParamBool("DoSharpen"))
    {
        ec = isp.sharpening(bgr32);
        if (ec != ISP::ErrCode::Ok) {
            std::cerr << "sharpening failed with ErrCode=" << static_cast<int>(ec) << std::endl;
        }

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