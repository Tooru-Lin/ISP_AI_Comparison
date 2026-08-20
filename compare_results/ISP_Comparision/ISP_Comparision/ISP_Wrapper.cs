using System;
using System.Runtime.InteropServices;

namespace ISP_CSharp
{
    // ========================================
    // ISP DLL 錯誤碼定義
    // ========================================
    public enum ISP_ErrCode
    {
        Ok = 0,
        FileOpenFailed = 1,
        UnpackFailed = 2,
        EmptyImage = 3,
        InvalidInput = 4,
        DemosaicFailed = 5,
        ColorCorrectionFailed = 6,
        ToneMappingFailed = 7,
        SharpeningFailed = 8,
        PreviewFailed = 9,
        Exception = 10,
        Unknown = 11
    }

    // ========================================
    // AWB 方法枚舉
    // ========================================
    public enum ISP_AWB_Method
    {
        Default = 1,
        GrayWorld = 2,
        WhitePoint = 3
    }

    // ========================================
    // Demosaic 方法枚舉
    // ========================================
    public enum ISP_Demosaic_Method
    {
        CCM = 1,
        AI = 2
    }

    // ========================================
    // ISP_Mat 結構體（對應 C++ 的 ISP_Mat）
    // ========================================
    [StructLayout(LayoutKind.Sequential)]
    public struct ISP_Mat
    {
        public IntPtr data;      // 指向影像資料的指針
        public int rows;         // 高度
        public int cols;         // 寬度
        public int channels;     // 通道數
        public int type;         // OpenCV 型別
        public int step;         // 行步長（bytes）
    }

    // ========================================
    // ISP_Context 結構體（對應 C++ 的 ISP_Context）
    // ========================================
    public struct ISP_Context
    {
        public IntPtr isp_instance;
    }

    // ========================================
    // P/Invoke 聲明 - 生命週期管理
    // ========================================
    public static class ISP_NativeMethods
    {
        private const string DLL_NAME = "isp_traditional.dll";

        // 創建 ISP 上下文
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ISP_Create();

        // 銷毀 ISP 上下文
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ISP_Destroy(IntPtr ctx);

        // ========================================
        // 參數操作函數
        // ========================================

        //// 設定布爾參數
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        //public static extern ISP_ErrCode ISP_SetParamBool(IntPtr ctx, string key, int value);

        //// 取得布爾參數
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        //public static extern int ISP_GetParamBool(IntPtr ctx, string key);

        //// 設定 AWB 方法
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern ISP_ErrCode ISP_SetParamAWB(IntPtr ctx, ISP_AWB_Method method);

        //// 取得 AWB 方法
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern ISP_AWB_Method ISP_GetParamAWB(IntPtr ctx);

        //// 設定 Demosaic 方法
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern ISP_ErrCode ISP_SetParamDemosaic(IntPtr ctx, ISP_Demosaic_Method method);

        //// 取得 Demosaic 方法
        //[DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        //public static extern ISP_Demosaic_Method ISP_GetParamDemosaic(IntPtr ctx);

        // ========================================
        // 影像處理函數
        // ========================================

        // 從 RAW 檔案加載影像
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ISP_ErrCode ISP_LoadRawWithLibRaw(
            IntPtr ctx,
            string filename,
            out int width,
            out int height,
            out int black,
            out int white,
            float[] cam_mul,      // 輸出：4 個 float
            float[] pre_mul,      // 輸出：4 個 float
            out ISP_Mat cam_xyz,
            out ISP_Mat xyz_srgb,
            out ISP_Mat raw32);

        // 黑白電平校正
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_BlackAndWhiteLevelCorrection(
            IntPtr ctx,
            ref ISP_Mat raw,
            float black_level,
            float white_level);

        // Demosaic (傳統)
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_Demosaic(
            IntPtr ctx,
            ref ISP_Mat raw,
            out ISP_Mat out_bgr32);

        // ---------------------------
        // AI Demosaic native exports
        // ---------------------------

        // 設定 AI demosaic 模型（回傳 ISP_ErrCode）
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ISP_ErrCode ISP_SetAiDemosaicModel(IntPtr ctx, string modelPath);

        // AI demosaic（使用已設定的模型）
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_AiDemosaic(
            IntPtr ctx,
            ref ISP_Mat raw,
            out ISP_Mat out_bgr32);

        // AI demosaic（直接帶 modelPath 的單次呼叫）
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ISP_ErrCode ISP_AiDemosaicWithModel(
            IntPtr ctx,
            ref ISP_Mat raw,
            out ISP_Mat out_bgr32,
            string modelPath);

        // 計算 CCM 矩陣：
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_CalculateCCM(
            ref ISP_Mat xyz_srgb,
            ref ISP_Mat cam_xyz,
            out ISP_Mat out_ccm);

        // 色彩校正
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_ColorCorrection(
            IntPtr ctx,
            ref ISP_Mat img,
            ref ISP_Mat ccm,
            out ISP_Mat out_img);

        // 套用 pre_mul
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_ApplyPreMul(
            IntPtr ctx,
            ref ISP_Mat img,
            float[] pre_mul);

        // 計算 AWB Gain - Gray World
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_CalAWBGain_GrayWorld(
            IntPtr ctx,
            ref ISP_Mat img,
            out double gain_R,
            out double gain_G,
            out double gain_B);

        // 計算 AWB Gain - White Patch
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_CalAWBGain_WhitePatch(
            IntPtr ctx,
            ref ISP_Mat img,
            out double gain_R,
            out double gain_G,
            out double gain_B);

        // 套用 AWB Gain
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_ApplyAWBGain(
            IntPtr ctx,
            ref ISP_Mat raw,
            int height,
            int width,
            double gainR,
            double gainG,
            double gainB);

        // 色調映射
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_ApplyToneMapping(
            IntPtr ctx,
            ref ISP_Mat img,
            float gamma);

        // 銳化
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern ISP_ErrCode ISP_Sharpening(
            IntPtr ctx,
            ref ISP_Mat img,
            double sharpening_level);

        // 預覽
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ISP_ErrCode ISP_ShowPreview(
            IntPtr ctx,
            ref ISP_Mat img,
            string title,
            double scale,
            int auto_contrast);

        // ========================================
        // Mat 管理函數
        // ========================================

        // 創建 Mat
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ISP_Mat_Create(int rows, int cols, int type);

        // 釋放 Mat
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ISP_Mat_Release(ref ISP_Mat mat);

        // 複製 Mat
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr ISP_Mat_Clone(ref ISP_Mat mat);

        // 取得 Mat 資訊
        [DllImport(DLL_NAME, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ISP_Mat_GetInfo(
            ref ISP_Mat mat,
            out int rows,
            out int cols,
            out int channels,
            out int type);
    }

    // ========================================
    // 高層 C# 包裝類別
    // ========================================
    public class ISP_Processor : IDisposable
    {
        private IntPtr ctx;
        private bool disposed = false;

        // 建構子
        public ISP_Processor()
        {
            ctx = ISP_NativeMethods.ISP_Create();
            if (ctx == IntPtr.Zero)
            {
                throw new Exception("Failed to create ISP context");
            }
        }

        // 解構子
        ~ISP_Processor()
        {
            Dispose(false);
        }

        // IDisposable 實現
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (ctx != IntPtr.Zero)
                {
                    ISP_NativeMethods.ISP_Destroy(ctx);
                    ctx = IntPtr.Zero;
                }
                disposed = true;
            }
        }

        // ========================================
        // 參數操作方法
        // ========================================

        ///// <summary>
        ///// 設定布爾參數
        ///// </summary>
        //public ISP_ErrCode SetParamBool(string key, bool value)
        //{
        //    return ISP_NativeMethods.ISP_SetParamBool(ctx, key, value ? 1 : 0);
        //}

        ///// <summary>
        ///// 取得布爾參數
        ///// </summary>
        //public bool GetParamBool(string key)
        //{
        //    return ISP_NativeMethods.ISP_GetParamBool(ctx, key) != 0;
        //}

        ///// <summary>
        ///// 設定 AWB 方法
        ///// </summary>
        //public ISP_ErrCode SetParamAWB(ISP_AWB_Method method)
        //{
        //    return ISP_NativeMethods.ISP_SetParamAWB(ctx, method);
        //}

        ///// <summary>
        ///// 取得 AWB 方法
        ///// </summary>
        //public ISP_AWB_Method GetParamAWB()
        //{
        //    return ISP_NativeMethods.ISP_GetParamAWB(ctx);
        //}

        ///// <summary>
        ///// 設定 Demosaic 方法
        ///// </summary>
        //public ISP_ErrCode SetParamDemosaic(ISP_Demosaic_Method method)
        //{
        //    return ISP_NativeMethods.ISP_SetParamDemosaic(ctx, method);
        //}

        ///// <summary>
        ///// 取得 Demosaic 方法
        ///// </summary>
        //public ISP_Demosaic_Method GetParamDemosaic()
        //{
        //    return ISP_NativeMethods.ISP_GetParamDemosaic(ctx);
        //}

        // ========================================
        // 影像處理方法
        // ========================================

        /// <summary>
        /// 從 RAW 檔案加載影像
        /// </summary>
        public ISP_ErrCode LoadRawWithLibRaw(
            string filename,
            out int width,
            out int height,
            out int black,
            out int white,
            out float[] cam_mul,
            out float[] pre_mul,
            out ISP_Mat cam_xyz,
            out ISP_Mat xyz_srgb,
            out ISP_Mat raw32)
        {
            cam_mul = new float[4];
            pre_mul = new float[4];

            ISP_ErrCode ec = ISP_NativeMethods.ISP_LoadRawWithLibRaw(
                ctx,
                filename,
                out width,
                out height,
                out black,
                out white,
                cam_mul,
                pre_mul,
                out cam_xyz,
                out xyz_srgb,
                out raw32);

            return ec;
        }

        /// <summary>
        /// 黑白電平校正
        /// </summary>
        public ISP_ErrCode BlackAndWhiteLevelCorrection(
            ref ISP_Mat raw,
            float black_level,
            float white_level)
        {
            return ISP_NativeMethods.ISP_BlackAndWhiteLevelCorrection(
                ctx,
                ref raw,
                black_level,
                white_level);
        }

        /// <summary>
        /// Demosaic - 去馬賽克 (傳統)
        /// </summary>
        public ISP_ErrCode Demosaic(ref ISP_Mat raw, out ISP_Mat out_bgr32)
        {
            return ISP_NativeMethods.ISP_Demosaic(ctx, ref raw, out out_bgr32);
        }

        /// <summary>
        /// 設定 AI Demosaic 模型（若 DLL 支援）
        /// </summary>
        public ISP_ErrCode SetAiDemosaicModel(string modelPath)
        {
            return ISP_NativeMethods.ISP_SetAiDemosaicModel(ctx, modelPath);
        }

        /// <summary>
        /// AI Demosaic（使用已設定模型）
        /// </summary>
        public ISP_ErrCode AiDemosaic(ref ISP_Mat raw, out ISP_Mat out_bgr32)
        {
            return ISP_NativeMethods.ISP_AiDemosaic(ctx, ref raw, out out_bgr32);
        }

        /// <summary>
        /// AI Demosaic（嘗試直接帶 modelPath 的導出，若不可用 fallback 為先設定模型再呼叫）
        /// </summary>
        public ISP_ErrCode AiDemosaic(ref ISP_Mat raw, out ISP_Mat out_bgr32, string modelPath)
        {
            // 優先嘗試直接帶 modelPath 的導出
            try
            {
                return ISP_NativeMethods.ISP_AiDemosaicWithModel(ctx, ref raw, out out_bgr32, modelPath);
            }
            catch (EntryPointNotFoundException)
            {
                // 若沒有該導出，再改用先設定模型再呼叫 ISP_AiDemosaic
                ISP_ErrCode setEc = ISP_NativeMethods.ISP_SetAiDemosaicModel(ctx, modelPath);
                if (setEc != ISP_ErrCode.Ok)
                {
                    out_bgr32 = new ISP_Mat();
                    return setEc;
                }
                return ISP_NativeMethods.ISP_AiDemosaic(ctx, ref raw, out out_bgr32);
            }
        }

        /// <summary>
        /// 計算色彩校正矩陣 (CCM)，公式：ccm = xyz_srgb * cam_xyz
        /// </summary>
        public ISP_ErrCode CalculateCCM(
            ref ISP_Mat xyz_srgb,
            ref ISP_Mat cam_xyz,
            out ISP_Mat out_ccm)
        {
            return ISP_NativeMethods.ISP_CalculateCCM(
                ref xyz_srgb,
                ref cam_xyz,
                out out_ccm);
        }

        /// <summary>
        /// 色彩校正
        /// </summary>
        public ISP_ErrCode ColorCorrection(
            ref ISP_Mat img,
            ref ISP_Mat ccm,
            out ISP_Mat out_img)
        {
            return ISP_NativeMethods.ISP_ColorCorrection(ctx, ref img, ref ccm, out out_img);
        }

        /// <summary>
        /// 套用 pre_mul 白平衡係數
        /// </summary>
        public ISP_ErrCode ApplyPreMul(ref ISP_Mat img, float[] pre_mul)
        {
            return ISP_NativeMethods.ISP_ApplyPreMul(ctx, ref img, pre_mul);
        }

        /// <summary>
        /// 計算 AWB Gain - Gray World 方法
        /// </summary>
        public ISP_ErrCode CalAWBGain_GrayWorld(
            ref ISP_Mat img,
            out double gain_R,
            out double gain_G,
            out double gain_B)
        {
            return ISP_NativeMethods.ISP_CalAWBGain_GrayWorld(
                ctx,
                ref img,
                out gain_R,
                out gain_G,
                out gain_B);
        }

        /// <summary>
        /// 計算 AWB Gain - White Patch 方法
        /// </summary>
        public ISP_ErrCode CalAWBGain_WhitePatch(
            ref ISP_Mat img,
            out double gain_R,
            out double gain_G,
            out double gain_B)
        {
            return ISP_NativeMethods.ISP_CalAWBGain_WhitePatch(
                ctx,
                ref img,
                out gain_R,
                out gain_G,
                out gain_B);
        }

        /// <summary>
        /// 套用 AWB Gain
        /// </summary>
        public ISP_ErrCode ApplyAWBGain(
            ref ISP_Mat raw,
            int height,
            int width,
            double gainR,
            double gainG,
            double gainB)
        {
            return ISP_NativeMethods.ISP_ApplyAWBGain(
                ctx,
                ref raw,
                height,
                width,
                gainR,
                gainG,
                gainB);
        }

        /// <summary>
        /// 色調映射
        /// </summary>
        public ISP_ErrCode ApplyToneMapping(ref ISP_Mat img, float gamma)
        {
            return ISP_NativeMethods.ISP_ApplyToneMapping(ctx, ref img, gamma);
        }

        /// <summary>
        /// 銳化
        /// </summary>
        public ISP_ErrCode Sharpening(ref ISP_Mat img, double sharpening_level)
        {
            return ISP_NativeMethods.ISP_Sharpening(ctx, ref img, sharpening_level);
        }

        /// <summary>
        /// 預覽影像
        /// </summary>
        public ISP_ErrCode ShowPreview(
            ref ISP_Mat img,
            string title,
            double scale = 1.0,
            bool auto_contrast = true)
        {
            return ISP_NativeMethods.ISP_ShowPreview(
                ctx,
                ref img,
                title,
                scale,
                auto_contrast ? 1 : 0);
        }
    }

    // ========================================
    // Mat 管理輔助類別
    // ========================================
    public static class ISP_MatHelper
    {
        // OpenCV 型別常數
        public const int CV_8U = 0;
        public const int CV_32F = 5;
        public const int CV_16U = 2;

        /// <summary>
        /// 從 IntPtr 讀取浮點數據
        /// </summary>
        public static float[] ReadFloatData(IntPtr data, int count)
        {
            float[] result = new float[count];
            Marshal.Copy(data, result, 0, count);
            return result;
        }

        /// <summary>
        /// 從 ISP_Mat 讀取資料為 byte 陣列
        /// </summary>
        public static byte[] GetMatData(ISP_Mat mat)
        {
            if (mat.data == IntPtr.Zero)
                return null;

            byte[] data = new byte[mat.step * mat.rows];
            Marshal.Copy(mat.data, data, 0, data.Length);
            return data;
        }

        /// <summary>
        /// 計算資料型別的每像素字節數
        /// </summary>
        public static int GetBytesPerPixel(int type)
        {
            // type 的低 3 bits 表示深度，bits 3-7 表示通道數
            int depth = type & 0x07;
            int channels = (type >> 3) + 1;

            int bytesPerElement = 1 << depth;  // 2^depth
            return bytesPerElement * channels;
        }
    }
}