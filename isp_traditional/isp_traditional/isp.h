#pragma once
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <stdexcept>
#include <onnxruntime_cxx_api.h>

class ISP
{
public:
    // 錯誤碼回傳
    enum class ErrCode {
        Ok = 0,
        FileOpenFailed,
        UnpackFailed,
        EmptyImage,
        InvalidInput,
        DemosaicFailed,
        ColorCorrectionFailed,
        ToneMappingFailed,
        SharpeningFailed,
        PreviewFailed,
        Exception,
        Unknown
    };

    // Enum 定義
    enum class AWB_Method { Default = 1, GrayWorld = 2, WhitePoint = 3 };
    enum class Demosaic_Method { Default = 1, AI = 2 };

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
        enumAWB_params["AWB_Method"] = AWB_Method::WhitePoint;       // 預設 AWB 方法
        enumDemosaic_params["Demosaic_Method"] = Demosaic_Method::Default; // 預設 demosaic 方法
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
    // 從 RAW 檔案加載影像
    ErrCode loadRawWithLibRaw(
        const std::string& filename,
        int& Width,
        int& Height,
        int& black,        // 輸出黑階
        int& white,        // 輸出白階
        std::vector<float>& cam_mul,  // AWB before demosaic
        std::vector<float>& pre_mul,  // AWB after demosaic
        cv::Mat& cam_xyz,             // 輸出 3x3 相機→XYZ 矩陣
        cv::Mat& xyz_srgb,            // 輸出 3x3 XYZ→sRGB 矩陣
        cv::Mat& raw32);              // 輸出 raw 32F

    // 黑白電平校正（normalize）
    ErrCode BlackAndWhiteLevelCorrection(cv::Mat& raw, float black_level, float white_level);

    // Bayer去馬賽克
    ErrCode demosaic(const cv::Mat& raw, cv::Mat& out_bgr32);

    // 色彩校正（CCM）
    ErrCode colorCorrection(const cv::Mat& img, const cv::Mat& ccm, cv::Mat& out);

    // 套用 pre_mul 白平衡係數
    ErrCode applyPreMul(cv::Mat& img, const std::vector<float> pre_mul);

    // 白平衡（依增益調整RGB通道）
    ErrCode CalAWBGain_GrayWorld(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B);

    // 白平衡 (亮點做白平衡)
    ErrCode CalAWBGain_WhitePatch(const cv::Mat& img, double& gain_R, double& gain_G, double& gain_B);

    // 套用白平衡增益
    ErrCode ApplyAWBGain(cv::Mat& img, int height, int width, double gainR, double gainG, double gainB);

    // 依照 P50 亮度做曝光校正
    ErrCode normalizeExposureByP50(cv::Mat& img, float Target_P50);
    
    // 色調映射與Gamma校正
    ErrCode applyToneMapping(cv::Mat& img, float gamma = 1);

    // 銳化
    ErrCode sharpening(cv::Mat& img, double Sharpening_Level = 0);

    // 壓縮與輸出
    ErrCode showPreview(const cv::Mat& img, const std::string& title, double scale = 1.0, bool autoContrast = true);

    // ISP_native_api.h
    ErrCode SetAiDemosaicModel(const char* modelPath);
    ErrCode AiDemosaicWithModel(cv::Mat& raw, cv::Mat& out_bgr32, const char* modelPath);
    // 若想先設定再呼叫：
    ErrCode AiDemosaic(cv::Mat& raw, cv::Mat& out_bgr32);

private:
    int ImgCount = 1;

    // ----------------- Bool 參數 -----------------
    std::unordered_map<std::string, bool> bool_params;

    // ----------------- Enum 參數 -----------------
    std::unordered_map<std::string, AWB_Method> enumAWB_params;
    std::unordered_map<std::string, Demosaic_Method> enumDemosaic_params;

    // ============================
    // AI Demosaic
    // ============================
    std::string ai_model_path;     // 目前載入的模型路徑

    // ONNX Runtime 核心成員
    // 使用 unique_ptr 延遲初始化，避免全域/建構時未準備好 API 的崩潰問題
    std::unique_ptr<Ort::Env> ort_env;
    std::unique_ptr<Ort::Session> ort_session;

    std::string ort_input_name;
    std::string ort_output_name;
};