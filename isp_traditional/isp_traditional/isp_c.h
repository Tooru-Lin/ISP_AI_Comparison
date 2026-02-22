#pragma once

// ========================================
// ISP C 語言包裝層 (extern "C" DLL Interface)
// 使用 C 語言風格的接口，允許跨語言調用
// ========================================

#ifdef _WIN32
#ifdef ISP_DLL_EXPORT
#define ISP_API __declspec(dllexport)
#else
#define ISP_API __declspec(dllimport)
#endif
#else
#define ISP_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // ========================================
    // 錯誤碼定義 (ErrCode 轉為 int)
    // ========================================
    typedef enum {
        ISP_OK = 0,
        ISP_FileOpenFailed = 1,
        ISP_UnpackFailed = 2,
        ISP_EmptyImage = 3,
        ISP_InvalidInput = 4,
        ISP_DemosaicFailed = 5,
        ISP_ColorCorrectionFailed = 6,
        ISP_ToneMappingFailed = 7,
        ISP_SharpeningFailed = 8,
        ISP_PreviewFailed = 9,
        ISP_Exception = 10,
        ISP_Unknown = 11
    } ISP_ErrCode;

    // AWB 方法枚舉
    typedef enum {
        ISP_AWB_Default = 1,
        ISP_AWB_GrayWorld = 2,
        ISP_AWB_WhitePoint = 3
    } ISP_AWB_Method;

    // Demosaic 方法枚舉
    typedef enum {
        ISP_Demosaic_CCM = 1,
        ISP_Demosaic_AI = 2
    } ISP_Demosaic_Method;

    // ========================================
    // ISP 影像處理上下文結構體
    // 用於保存 C++ ISP 物件指針
    // ========================================
    typedef struct ISP_Context {
        void* isp_instance;  // 指向 C++ ISP 物件的指針
    } ISP_Context;

    // ========================================
    // Mat 資料結構 (簡化版本，用於跨 DLL 邊界)
    // ========================================
    typedef struct {
        unsigned char* data;
        int rows;
        int cols;
        int channels;
        int type;  // OpenCV type (CV_8U, CV_32F, etc.)
        int step;  // 行步長 (bytes)
    } ISP_Mat;

    // ========================================
    // 生命週期管理函數
    // ========================================

    // 創建 ISP 上下文
    ISP_API ISP_Context* ISP_Create(void);

    // 銷毀 ISP 上下文
    ISP_API void ISP_Destroy(ISP_Context* ctx);

    // ========================================
    // 參數操作函數
    // ========================================

    //// 設定布爾參數
    //ISP_API ISP_ErrCode ISP_SetParamBool(ISP_Context* ctx, const char* key, int value);

    //// 取得布爾參數
    //ISP_API int ISP_GetParamBool(ISP_Context* ctx, const char* key);

    //// 設定 AWB 方法
    //ISP_API ISP_ErrCode ISP_SetParamAWB(ISP_Context* ctx, ISP_AWB_Method method);

    //// 取得 AWB 方法
    //ISP_API ISP_AWB_Method ISP_GetParamAWB(ISP_Context* ctx);

    //// 設定 Demosaic 方法
    //ISP_API ISP_ErrCode ISP_SetParamDemosaic(ISP_Context* ctx, ISP_Demosaic_Method method);

    //// 取得 Demosaic 方法
    //ISP_API ISP_Demosaic_Method ISP_GetParamDemosaic(ISP_Context* ctx);

    // ========================================
    // 影像處理函數
    // ========================================

    // 從 RAW 檔案加載影像
    ISP_API ISP_ErrCode ISP_LoadRawWithLibRaw(
        ISP_Context* ctx,
        const char* filename,
        int* width,
        int* height,
        int* black,
        int* white,
        float* cam_mul,      // 輸出：4 個 float 的陣列
        float* pre_mul,      // 輸出：4 個 float 的陣列
        ISP_Mat* cam_xyz,    // 輸出：3x3 矩陣
        ISP_Mat* xyz_srgb,   // 輸出：3x3 矩陣
        ISP_Mat* raw32);     // 輸出：原始影像

    // 黑白電平校正
    ISP_API ISP_ErrCode ISP_BlackAndWhiteLevelCorrection(
        ISP_Context* ctx,
        ISP_Mat* raw,
        float black_level,
        float white_level);

    // Demosaic
    ISP_API ISP_ErrCode ISP_Demosaic(
        ISP_Context* ctx,
        ISP_Mat* raw,
        ISP_Mat* out_bgr32);

    // 計算色彩校正矩陣 (CCM)，公式：ccm = xyz_srgb * cam_xyz
    ISP_API ISP_ErrCode ISP_CalculateCCM(
        ISP_Mat* xyz_srgb,    // 輸入：XYZ→sRGB 矩陣 (3x3)
        ISP_Mat* cam_xyz,     // 輸入：相機 RGB→XYZ 矩陣 (3x3)
        ISP_Mat* out_ccm);    // 輸出：計算結果的 CCM 矩陣 (3x3)

    // 色彩校正
    ISP_API ISP_ErrCode ISP_ColorCorrection(
        ISP_Context* ctx,
        ISP_Mat* img,
        ISP_Mat* ccm,
        ISP_Mat* out);

    // 套用 pre_mul
    ISP_API ISP_ErrCode ISP_ApplyPreMul(
        ISP_Context* ctx,
        ISP_Mat* img,
        const float* pre_mul);

    // 計算 AWB Gain - Gray World
    ISP_API ISP_ErrCode ISP_CalAWBGain_GrayWorld(
        ISP_Context* ctx,
        ISP_Mat* img,
        double* gain_R,
        double* gain_G,
        double* gain_B);

    // 計算 AWB Gain - White Patch
    ISP_API ISP_ErrCode ISP_CalAWBGain_WhitePatch(
        ISP_Context* ctx,
        ISP_Mat* img,
        double* gain_R,
        double* gain_G,
        double* gain_B);

    // 套用 AWB Gain
    ISP_API ISP_ErrCode ISP_ApplyAWBGain(
        ISP_Context* ctx,
        ISP_Mat* raw,
        int height,
        int width,
        double gainR,
        double gainG,
        double gainB);

    // 色調映射
    ISP_API ISP_ErrCode ISP_ApplyToneMapping(
        ISP_Context* ctx,
        ISP_Mat* img,
        float gamma);

    // 銳化
    ISP_API ISP_ErrCode ISP_Sharpening(
        ISP_Context* ctx,
        ISP_Mat* img,
        double sharpening_level);

    // 預覽
    ISP_API ISP_ErrCode ISP_ShowPreview(
        ISP_Context* ctx,
        ISP_Mat* img,
        const char* title,
        double scale,
        int auto_contrast);

    // ========================================
    // Mat 管理函數
    // ========================================

    // 創建 Mat
    ISP_API ISP_Mat* ISP_Mat_Create(int rows, int cols, int type);

    // 釋放 Mat
    ISP_API void ISP_Mat_Release(ISP_Mat* mat);

    // 複製 Mat
    ISP_API ISP_Mat* ISP_Mat_Clone(ISP_Mat* mat);

    // 取得 Mat 資訊
    ISP_API void ISP_Mat_GetInfo(ISP_Mat* mat, int* rows, int* cols, int* channels, int* type);

#ifdef __cplusplus
}
#endif