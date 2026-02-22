// ========================================
// ISP C 語言包裝層實現 (extern "C" DLL)
// ========================================

#define ISP_DLL_EXPORT
#include "isp_c.h"
#include "isp.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstring>

// ========================================
// 全局 ISP 實例 (簡單的單例模式)
// ========================================
static ISP* g_isp_instance = nullptr;

// ========================================
// 輔助函數：cv::Mat 轉換為 ISP_Mat
// ========================================
static ISP_Mat* cvmat_to_isp_mat(const cv::Mat& mat) {
    if (mat.empty()) {
        return nullptr;
    }

    ISP_Mat* result = new ISP_Mat();
    result->rows = mat.rows;
    result->cols = mat.cols;
    result->channels = mat.channels();
    result->type = mat.type();
    result->step = mat.step;

    // 複製資料
    size_t data_size = mat.step * mat.rows;
    result->data = new unsigned char[data_size];
    memcpy(result->data, mat.data, data_size);

    return result;
}

// ========================================
// 輔助函數：ISP_Mat 轉換為 cv::Mat
// ========================================
static cv::Mat isp_mat_to_cvmat(ISP_Mat* isp_mat) {
    if (isp_mat == nullptr || isp_mat->data == nullptr) {
        return cv::Mat();
    }

    // 建立 Mat（共享資料，不複製）
    cv::Mat result(isp_mat->rows, isp_mat->cols, isp_mat->type, isp_mat->data, isp_mat->step);
    return result;
}

// ========================================
// 生命週期管理函數實現
// ========================================

ISP_Context* ISP_Create(void) {
    try {
        ISP_Context* ctx = new ISP_Context();
        ctx->isp_instance = new ISP();
        return ctx;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Create: " << ex.what() << std::endl;
        return nullptr;
    }
}

void ISP_Destroy(ISP_Context* ctx) {
    try {
        if (ctx != nullptr) {
            if (ctx->isp_instance != nullptr) {
                delete reinterpret_cast<ISP*>(ctx->isp_instance);
            }
            delete ctx;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Destroy: " << ex.what() << std::endl;
    }
}

// ========================================
// 參數操作函數實現
// ========================================

//ISP_ErrCode ISP_SetParamBool(ISP_Context* ctx, const char* key, int value) {
//    try {
//        if (ctx == nullptr || key == nullptr) {
//            return ISP_InvalidInput;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        isp->setParamBool(key, value != 0);
//        return ISP_OK;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_SetParamBool: " << ex.what() << std::endl;
//        return ISP_Exception;
//    }
//}
//
//int ISP_GetParamBool(ISP_Context* ctx, const char* key) {
//    try {
//        if (ctx == nullptr || key == nullptr) {
//            return 0;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        return isp->getParamBool(key) ? 1 : 0;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_GetParamBool: " << ex.what() << std::endl;
//        return 0;
//    }
//}
//
//ISP_ErrCode ISP_SetParamAWB(ISP_Context* ctx, ISP_AWB_Method method) {
//    try {
//        if (ctx == nullptr) {
//            return ISP_InvalidInput;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        isp->setParamAWB(static_cast<ISP::AWB_Method>(method));
//        return ISP_OK;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_SetParamAWB: " << ex.what() << std::endl;
//        return ISP_Exception;
//    }
//}
//
//ISP_AWB_Method ISP_GetParamAWB(ISP_Context* ctx) {
//    try {
//        if (ctx == nullptr) {
//            return ISP_AWB_Default;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        return static_cast<ISP_AWB_Method>(isp->getParamAWB());
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_GetParamAWB: " << ex.what() << std::endl;
//        return ISP_AWB_Default;
//    }
//}
//
//ISP_ErrCode ISP_SetParamDemosaic(ISP_Context* ctx, ISP_Demosaic_Method method) {
//    try {
//        if (ctx == nullptr) {
//            return ISP_InvalidInput;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        isp->setParamDemosaic(static_cast<ISP::Demosaic_Method>(method));
//        return ISP_OK;
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_SetParamDemosaic: " << ex.what() << std::endl;
//        return ISP_Exception;
//    }
//}
//
//ISP_Demosaic_Method ISP_GetParamDemosaic(ISP_Context* ctx) {
//    try {
//        if (ctx == nullptr) {
//            return ISP_Demosaic_CCM;
//        }
//
//        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
//        return static_cast<ISP_Demosaic_Method>(isp->getParamDemosaic());
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Exception in ISP_GetParamDemosaic: " << ex.what() << std::endl;
//        return ISP_Demosaic_CCM;
//    }
//}

// ========================================
// 影像處理函數實現
// ========================================

ISP_ErrCode ISP_LoadRawWithLibRaw(
    ISP_Context* ctx,
    const char* filename,
    int* width,
    int* height,
    int* black,
    int* white,
    float* cam_mul,
    float* pre_mul,
    ISP_Mat* cam_xyz,
    ISP_Mat* xyz_srgb,
    ISP_Mat* raw32) {
    try {
        if (ctx == nullptr || filename == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);

        int w, h, b, wh;
        std::vector<float> cam_mul_vec, pre_mul_vec;
        cv::Mat cam_xyz_mat, xyz_srgb_mat, raw32_mat;

        ISP::ErrCode ec = isp->loadRawWithLibRaw(
            filename,
            w, h, b, wh,
            cam_mul_vec, pre_mul_vec,
            cam_xyz_mat, xyz_srgb_mat,
            raw32_mat);

        if (ec != ISP::ErrCode::Ok) {
            return static_cast<ISP_ErrCode>(ec);
        }

        // 複製輸出參數
        *width = w;
        *height = h;
        *black = b;
        *white = wh;

        // 複製陣列
        for (int i = 0; i < 4; i++) {
            cam_mul[i] = cam_mul_vec[i];
            pre_mul[i] = pre_mul_vec[i];
        }

        // 轉換 Mat
        *cam_xyz = *cvmat_to_isp_mat(cam_xyz_mat);
        *xyz_srgb = *cvmat_to_isp_mat(xyz_srgb_mat);
        *raw32 = *cvmat_to_isp_mat(raw32_mat);

        return ISP_OK;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_LoadRawWithLibRaw: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_BlackAndWhiteLevelCorrection(
    ISP_Context* ctx,
    ISP_Mat* raw,
    float black_level,
    float white_level) {
    try {
        if (ctx == nullptr || raw == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat raw_mat = isp_mat_to_cvmat(raw);

        ISP::ErrCode ec = isp->BlackAndWhiteLevelCorrection(raw_mat, black_level, white_level);

        // 複製修改回 ISP_Mat
        memcpy(raw->data, raw_mat.data, raw->step * raw->rows);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_BlackAndWhiteLevelCorrection: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_Demosaic(
    ISP_Context* ctx,
    ISP_Mat* raw,
    ISP_Mat* out_bgr32) {
    try {
        if (ctx == nullptr || raw == nullptr || out_bgr32 == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat raw_mat = isp_mat_to_cvmat(raw);
        cv::Mat out_mat;

        ISP::ErrCode ec = isp->demosaic(raw_mat, out_mat);

        if (ec == ISP::ErrCode::Ok) {
            *out_bgr32 = *cvmat_to_isp_mat(out_mat);
        }

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Demosaic: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_ColorCorrection(
    ISP_Context* ctx,
    ISP_Mat* img,
    ISP_Mat* ccm,
    ISP_Mat* out) {
    try {
        if (ctx == nullptr || img == nullptr || ccm == nullptr || out == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);
        cv::Mat ccm_mat = isp_mat_to_cvmat(ccm);
        cv::Mat out_mat;

        ISP::ErrCode ec = isp->colorCorrection(img_mat, ccm_mat, out_mat);

        if (ec == ISP::ErrCode::Ok) {
            *out = *cvmat_to_isp_mat(out_mat);
        }

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_ColorCorrection: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_ApplyPreMul(
    ISP_Context* ctx,
    ISP_Mat* img,
    const float* pre_mul) {
    try {
        if (ctx == nullptr || img == nullptr || pre_mul == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        std::vector<float> pre_mul_vec(pre_mul, pre_mul + 3);
        ISP::ErrCode ec = isp->applyPreMul(img_mat, pre_mul_vec);

        // 複製修改回 ISP_Mat
        memcpy(img->data, img_mat.data, img->step * img->rows);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_ApplyPreMul: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_CalAWBGain_GrayWorld(
    ISP_Context* ctx,
    ISP_Mat* img,
    double* gain_R,
    double* gain_G,
    double* gain_B) {
    try {
        if (ctx == nullptr || img == nullptr || gain_R == nullptr || gain_G == nullptr || gain_B == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        ISP::ErrCode ec = isp->CalAWBGain_GrayWorld(img_mat, *gain_R, *gain_G, *gain_B);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_CalAWBGain_GrayWorld: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_CalAWBGain_WhitePatch(
    ISP_Context* ctx,
    ISP_Mat* img,
    double* gain_R,
    double* gain_G,
    double* gain_B) {
    try {
        if (ctx == nullptr || img == nullptr || gain_R == nullptr || gain_G == nullptr || gain_B == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        ISP::ErrCode ec = isp->CalAWBGain_WhitePatch(img_mat, *gain_R, *gain_G, *gain_B);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_CalAWBGain_WhitePatch: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_ApplyAWBGain(
    ISP_Context* ctx,
    ISP_Mat* raw,
    int height,
    int width,
    double gainR,
    double gainG,
    double gainB) {
    try {
        if (ctx == nullptr || raw == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat raw_mat = isp_mat_to_cvmat(raw);

        ISP::ErrCode ec = isp->ApplyAWBGain(raw_mat, height, width, gainR, gainG, gainB);

        // 複製修改回 ISP_Mat
        memcpy(raw->data, raw_mat.data, raw->step * raw->rows);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_ApplyAWBGain: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_ApplyToneMapping(
    ISP_Context* ctx,
    ISP_Mat* img,
    float gamma) {
    try {
        if (ctx == nullptr || img == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        ISP::ErrCode ec = isp->applyToneMapping(img_mat, gamma);

        // 複製修改回 ISP_Mat
        memcpy(img->data, img_mat.data, img->step * img->rows);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_ApplyToneMapping: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_Sharpening(
    ISP_Context* ctx,
    ISP_Mat* img,
    double sharpening_level) {
    try {
        if (ctx == nullptr || img == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        ISP::ErrCode ec = isp->sharpening(img_mat, sharpening_level);

        // 複製修改回 ISP_Mat
        memcpy(img->data, img_mat.data, img->step * img->rows);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Sharpening: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

ISP_ErrCode ISP_ShowPreview(
    ISP_Context* ctx,
    ISP_Mat* img,
    const char* title,
    double scale,
    int auto_contrast) {
    try {
        if (ctx == nullptr || img == nullptr || title == nullptr) {
            return ISP_InvalidInput;
        }

        ISP* isp = reinterpret_cast<ISP*>(ctx->isp_instance);
        cv::Mat img_mat = isp_mat_to_cvmat(img);

        ISP::ErrCode ec = isp->showPreview(img_mat, title, scale, auto_contrast != 0);

        return static_cast<ISP_ErrCode>(ec);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_ShowPreview: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

// ========================================
// Mat 管理函數實現
// ========================================

ISP_Mat* ISP_Mat_Create(int rows, int cols, int type) {
    try {
        ISP_Mat* result = new ISP_Mat();
        cv::Mat mat(rows, cols, type);

        result->rows = rows;
        result->cols = cols;
        result->channels = mat.channels();
        result->type = type;
        result->step = mat.step;

        // 複製資料
        size_t data_size = mat.step * rows;
        result->data = new unsigned char[data_size];
        memcpy(result->data, mat.data, data_size);

        return result;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Mat_Create: " << ex.what() << std::endl;
        return nullptr;
    }
}

void ISP_Mat_Release(ISP_Mat* mat) {
    try {
        if (mat != nullptr) {
            if (mat->data != nullptr) {
                delete[] mat->data;
            }
            delete mat;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Mat_Release: " << ex.what() << std::endl;
    }
}

ISP_Mat* ISP_Mat_Clone(ISP_Mat* mat) {
    try {
        if (mat == nullptr) {
            return nullptr;
        }

        ISP_Mat* result = new ISP_Mat();
        result->rows = mat->rows;
        result->cols = mat->cols;
        result->channels = mat->channels;
        result->type = mat->type;
        result->step = mat->step;

        // 複製資料
        size_t data_size = mat->step * mat->rows;
        result->data = new unsigned char[data_size];
        memcpy(result->data, mat->data, data_size);

        return result;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Mat_Clone: " << ex.what() << std::endl;
        return nullptr;
    }
}

// ========================================
// 矩陣乘法輔助函數
// 計算：result = mat1 (3x3) * mat2 (3x3) = result (3x3)
// ========================================
static ISP_Mat MatrixMultiply(const ISP_Mat& mat1, const ISP_Mat& mat2)
{
    // 防呆檢查
    if (mat1.data == nullptr || mat2.data == nullptr ||
        mat1.rows != 3 || mat1.cols != 3 ||
        mat2.rows != 3 || mat2.cols != 3)
    {
        ISP_Mat empty = { nullptr, 0, 0, 0, 0, 0 };
        return empty;
    }

    // 轉換為 cv::Mat 進行計算
    cv::Mat m1 = isp_mat_to_cvmat((ISP_Mat*)&mat1);
    cv::Mat m2 = isp_mat_to_cvmat((ISP_Mat*)&mat2);

    // 確保為 CV_32F 類型
    if (m1.type() != CV_32F) m1.convertTo(m1, CV_32F);
    if (m2.type() != CV_32F) m2.convertTo(m2, CV_32F);

    // 執行矩陣乘法：result = m1 * m2
    cv::Mat result = m1 * m2;

    // 轉換回 ISP_Mat
    ISP_Mat isp_result;
    isp_result.rows = result.rows;
    isp_result.cols = result.cols;
    isp_result.channels = result.channels();
    isp_result.type = result.type();
    isp_result.step = result.step;

    // 複製資料
    size_t data_size = result.step * result.rows;
    isp_result.data = new unsigned char[data_size];
    memcpy(isp_result.data, result.data, data_size);

    return isp_result;
}

// ========================================
// 添加至 DLL 導出的函數：計算 CCM 矩陣
// 功能：計算色彩校正矩陣 (Color Correction Matrix)
// 公式：ccm = xyz_srgb * cam_xyz
// 輸入參數：
//   - xyz_srgb: XYZ→sRGB 矩陣 (3x3)
//   - cam_xyz: 相機 RGB→XYZ 矩陣 (3x3)
// 輸出參數：
//   - out_ccm: 計算結果的 CCM 矩陣 (3x3)
// 回傳：
//   - ISP_OK 成功
//   - ISP_InvalidInput 輸入無效
//   - ISP_Exception 例外發生
// ========================================
ISP_ErrCode ISP_CalculateCCM(
    ISP_Mat* xyz_srgb,
    ISP_Mat* cam_xyz,
    ISP_Mat* out_ccm)
{
    try
    {
        // 防呆檢查
        if (xyz_srgb == nullptr || cam_xyz == nullptr || out_ccm == nullptr)
        {
            return ISP_InvalidInput;
        }

        // 檢查矩陣有效性
        if (xyz_srgb->data == nullptr || cam_xyz->data == nullptr)
        {
            return ISP_InvalidInput;
        }

        if (xyz_srgb->rows != 3 || xyz_srgb->cols != 3 ||
            cam_xyz->rows != 3 || cam_xyz->cols != 3)
        {
            std::cerr << "Invalid matrix dimensions for CCM calculation" << std::endl;
            return ISP_InvalidInput;
        }

        // 執行矩陣乘法
        ISP_Mat result = MatrixMultiply(*xyz_srgb, *cam_xyz);

        if (result.data == nullptr)
        {
            std::cerr << "CCM calculation failed" << std::endl;
            return ISP_Exception;
        }

        // 複製結果到輸出參數
        *out_ccm = result;

        return ISP_OK;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception in ISP_CalculateCCM: " << ex.what() << std::endl;
        return ISP_Exception;
    }
}

void ISP_Mat_GetInfo(ISP_Mat* mat, int* rows, int* cols, int* channels, int* type) {
    try {
        if (mat != nullptr) {
            *rows = mat->rows;
            *cols = mat->cols;
            *channels = mat->channels;
            *type = mat->type;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception in ISP_Mat_GetInfo: " << ex.what() << std::endl;
    }
}