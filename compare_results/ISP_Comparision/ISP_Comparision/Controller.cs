using ISP_CSharp;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;

namespace ISP_Comparision
{
    // 各參數專屬 enum（範例：None / Default，可依需求擴充）
    public enum enumBlackWhiteLevel { None = 0, Default = 1 }
    public enum enumLensShading { None = 0, Default = 1 }
    public enum enumBadPixelCorrection { None = 0, Default = 1 }
    public enum enumLinearityCorrection { None = 0, Default = 1 }
    public enum enumDemosaic { None = 0, Default = 1, Ai_RawModel = 2, Ai_OptimizedModel, Ai_Fp16Model }
    public enum enumAutoWhiteBalance { None = 0, Default = 1, GrayWorld = 2, WhitePatch = 3 }
    public enum enumColorCorrection { None = 0, Default = 1 }
    public enum enumNoiseReduction { None = 0, Default = 1 }
    public enum enumToneMapping { None = 0, Default = 1 }
    public enum enumDistortionCorrection { None = 0, Default = 1 }
    public enum enumSharpening { None = 0, Default = 1 }

    // 新增：AI 模型選擇 enum
    public enum enumAiModelType { ModelRaw = 0, ModelOptimized = 1, ModelFloat16 = 2 }

    // Pipeline key 改成 enum
    public enum PipelineKey
    {
        BlackWhiteLevel,
        LensShading,
        BadPixelCorrection,
        LinearityCorrection,
        Demosaic,
        AutoWhiteBalance,
        ColorCorrection,
        NoiseReduction,
        ToneMapping,
        DistortionCorrection,
        Sharpening,
        AiModelType  // 新增：用於選擇 AI 模型
    }

    internal class Controller
    {
        private readonly Dictionary<PipelineKey, object> mPipeProcess;
        private readonly Dictionary<PipelineKey, Type> parameterTypes;
        private readonly object sync = new object();

        public Controller()
        {
            // 初始化類型表與預設值（key -> enum type ; key -> enum value）
            parameterTypes = new Dictionary<PipelineKey, Type>
            {
                { PipelineKey.BlackWhiteLevel, typeof(enumBlackWhiteLevel) },
                { PipelineKey.LensShading, typeof(enumLensShading) },
                { PipelineKey.BadPixelCorrection, typeof(enumBadPixelCorrection) },
                { PipelineKey.LinearityCorrection, typeof(enumLinearityCorrection) },
                { PipelineKey.Demosaic, typeof(enumDemosaic) },
                { PipelineKey.AutoWhiteBalance, typeof(enumAutoWhiteBalance) },
                { PipelineKey.ColorCorrection, typeof(enumColorCorrection) },
                { PipelineKey.NoiseReduction, typeof(enumNoiseReduction) },
                { PipelineKey.ToneMapping, typeof(enumToneMapping) },
                { PipelineKey.DistortionCorrection, typeof(enumDistortionCorrection) },
                { PipelineKey.Sharpening, typeof(enumSharpening) },
                { PipelineKey.AiModelType, typeof(enumAiModelType) }  // 新增
            };

            mPipeProcess = new Dictionary<PipelineKey, object>
            {
                { PipelineKey.BlackWhiteLevel, enumBlackWhiteLevel.Default },
                { PipelineKey.LensShading, enumLensShading.Default },
                { PipelineKey.BadPixelCorrection, enumBadPixelCorrection.Default },
                { PipelineKey.LinearityCorrection, enumLinearityCorrection.Default },
                { PipelineKey.Demosaic, enumDemosaic.Default },
                { PipelineKey.AutoWhiteBalance, enumAutoWhiteBalance.Default },
                { PipelineKey.ColorCorrection, enumColorCorrection.Default },
                { PipelineKey.NoiseReduction, enumNoiseReduction.Default },
                { PipelineKey.ToneMapping, enumToneMapping.Default },
                { PipelineKey.DistortionCorrection, enumDistortionCorrection.Default },
                { PipelineKey.Sharpening, enumSharpening.Default },
                { PipelineKey.AiModelType, enumAiModelType.ModelOptimized }  // 預設用優化模型
            };

        }

        // ---------- 強型別取/設 方法 (enum key) ----------

        public TEnum GetParam<TEnum>(PipelineKey key) where TEnum : struct, Enum
        {
            lock (sync)
            {
                if (!mPipeProcess.TryGetValue(key, out var val))
                    throw new KeyNotFoundException($"Parameter '{key}' not found.");
                var expected = parameterTypes[key];
                if (expected != typeof(TEnum))
                    throw new InvalidOperationException($"Parameter '{key}' expects enum type {expected.Name} not {typeof(TEnum).Name}.");
                return (TEnum)val;
            }
        }

        public bool TryGetParam<TEnum>(PipelineKey key, out TEnum value) where TEnum : struct, Enum
        {
            value = default(TEnum);
            lock (sync)
            {
                if (!mPipeProcess.TryGetValue(key, out var val)) return false;
                var expected = parameterTypes[key];
                if (expected != typeof(TEnum)) return false;
                value = (TEnum)val;
                return true;
            }
        }

        public void SetParam<TEnum>(PipelineKey key, TEnum enumValue) where TEnum : struct, Enum
        {
            lock (sync)
            {
                if (!mPipeProcess.ContainsKey(key))
                    throw new KeyNotFoundException($"Parameter '{key}' not found.");
                var expected = parameterTypes[key];
                if (expected != typeof(TEnum))
                    throw new InvalidOperationException($"Parameter '{key}' expects enum type {expected.Name} not {typeof(TEnum).Name}.");
                mPipeProcess[key] = enumValue;
            }
        }

        public void SetParams<TEnum>(IDictionary<PipelineKey, TEnum> dict) where TEnum : struct, Enum
        {
            if (dict == null) throw new ArgumentNullException(nameof(dict));
            lock (sync)
            {
                // 驗證全部存在並型別吻合
                foreach (var kv in dict)
                {
                    if (!mPipeProcess.ContainsKey(kv.Key))
                        throw new KeyNotFoundException($"Parameter '{kv.Key}' not found.");
                    var expected = parameterTypes[kv.Key];
                    if (expected != typeof(TEnum))
                        throw new InvalidOperationException($"Parameter '{kv.Key}' expects enum type {expected.Name}.");
                }
                // 套用
                foreach (var kv in dict)
                    mPipeProcess[kv.Key] = kv.Value;
            }
        }

        // ---------- 字串 overload（方便與舊程式整合） ----------

        private static string RemoveNonAlnum(string s)
        {
            if (string.IsNullOrEmpty(s)) return s;
            var arr = s.Where(c => char.IsLetterOrDigit(c)).ToArray();
            return new string(arr);
        }

        private static bool TryParseKey(string keyName, out PipelineKey key)
        {
            key = default(PipelineKey);
            if (string.IsNullOrWhiteSpace(keyName)) return false;
            if (Enum.TryParse<PipelineKey>(keyName, true, out key)) return true;
            var compact = RemoveNonAlnum(keyName);
            return Enum.TryParse<PipelineKey>(compact, true, out key);
        }

        public void SetParam(string keyName, Enum enumValue)
        {
            if (!TryParseKey(keyName, out var key))
                throw new KeyNotFoundException($"Parameter '{keyName}' not recognized.");
            lock (sync)
            {
                var expected = parameterTypes[key];
                if (enumValue == null) throw new ArgumentNullException(nameof(enumValue));
                if (enumValue.GetType() != expected)
                    throw new InvalidOperationException($"Parameter '{key}' expects enum type {expected.Name} not {enumValue.GetType().Name}.");
                mPipeProcess[key] = enumValue;
            }
        }

        public TEnum GetParam<TEnum>(string keyName) where TEnum : struct, Enum
        {
            if (!TryParseKey(keyName, out var key))
                throw new KeyNotFoundException($"Parameter '{keyName}' not recognized.");
            return GetParam<TEnum>(key);
        }

        public bool TryGetParam<TEnum>(string keyName, out TEnum value) where TEnum : struct, Enum
        {
            value = default(TEnum);
            if (!TryParseKey(keyName, out var key)) return false;
            return TryGetParam<TEnum>(key, out value);
        }

        public void SetParams(IDictionary<string, object> dict)
        {
            if (dict == null) throw new ArgumentNullException(nameof(dict));
            var temp = new List<KeyValuePair<PipelineKey, object>>();
            foreach (var kv in dict)
            {
                if (!TryParseKey(kv.Key, out var key))
                    throw new KeyNotFoundException($"Parameter '{kv.Key}' not recognized.");
                if (!(kv.Value is Enum))
                    throw new InvalidOperationException($"Parameter '{kv.Key}' expects an enum value.");
                var expected = parameterTypes[key];
                if (kv.Value.GetType() != expected)
                    throw new InvalidOperationException($"Parameter '{kv.Key}' expects enum type {expected.Name} not {kv.Value.GetType().Name}.");
                temp.Add(new KeyValuePair<PipelineKey, object>(key, kv.Value));
            }
            lock (sync)
            {
                foreach (var t in temp) mPipeProcess[t.Key] = t.Value;
            }
        }

        public Dictionary<PipelineKey, object> GetAllParametersSnapshot()
        {
            lock (sync)
            {
                return mPipeProcess.ToDictionary(kv => kv.Key, kv => kv.Value);
            }
        }

        public IEnumerable<PipelineKey> GetAllKeys()
        {
            lock (sync) return mPipeProcess.Keys.ToArray();
        }

        // 新增：根據 enum 取得模型檔案路徑
        private string GetModelPath(enumAiModelType modelType)
        {
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;
            switch (modelType)
            {
                case enumAiModelType.ModelRaw:
                    return Path.Combine(baseDir, "models/model_raw.onnx");
                case enumAiModelType.ModelOptimized:
                    return Path.Combine(baseDir, "models/model_optimized.onnx");
                case enumAiModelType.ModelFloat16:
                    return Path.Combine(baseDir, "models/model_fp16.onnx");
                default:
                    return Path.Combine(baseDir, "models/model_optimized.onnx");
            }
        }

        // 新增：直接用 modelPath 執行 AI Demosaic（可被外部呼叫）
        public ISP_ErrCode ApplyAiDemosaicOnnx(string modelPath, ref ISP_Mat raw, out ISP_Mat outColor)
        {
            outColor = new ISP_Mat();

            if (string.IsNullOrWhiteSpace(modelPath))
                return ISP_ErrCode.InvalidInput;

            if (!File.Exists(modelPath))
                return ISP_ErrCode.FileOpenFailed;

            try
            {
                // 確保 DLL 可用（與 Measure 中一致）
                NativeDiagnostics.DiagnoseIspDll(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "isp_traditional.dll"));

                using (var isp = new ISP_Processor())
                {
                    ISP_ErrCode ec = isp.AiDemosaic(ref raw, out outColor, modelPath);
                    return ec;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"ApplyAiDemosaicOnnx Error: {ex.Message}");
                Console.WriteLine(ex.StackTrace);
                outColor = new ISP_Mat();
                return ISP_ErrCode.Exception;
            }
        }

        // 新增：使用 enumAiModelType 選擇模型並執行 AI Demosaic
        public ISP_ErrCode ApplyAiDemosaicOnnx(enumAiModelType modelType, ref ISP_Mat raw, out ISP_Mat outColor)
        {
            string modelPath = GetModelPath(modelType);
            return ApplyAiDemosaicOnnx(modelPath, ref raw, out outColor);
        }

        public int Measure(string ImagePath, out ISP_Mat Output_Color, out ISP_Mat Output_Channel, float Target_P50 = 0.18f)
        {
            int ErrCode = 0;
            string ErrMsg = "";
            object obj;
            Output_Color = new ISP_Mat();
            Output_Channel = new ISP_Mat();
            try
            {
                NativeDiagnostics.DiagnoseIspDll(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "isp_traditional.dll"));

                using (var isp = new ISP_Processor())
                {
                    string rawPath = ImagePath;
                    if (!File.Exists(rawPath))
                    {
                        ErrMsg = $"File not found: {rawPath}";
                        return ErrCode;
                    }

                    ISP_ErrCode ec = isp.LoadRawWithLibRaw(
                        rawPath,
                        out int width,
                        out int height,
                        out int black,
                        out int white,
                        out float[] cam_mul,
                        out float[] pre_mul,
                        out ISP_Mat cam_xyz,
                        out ISP_Mat xyz_srgb,
                        out Output_Channel);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        ErrMsg = $"Failed to load RAW file: {ec}";
                        return ErrCode;
                    }

                    // ========================================
                    // 1. 黑白電平校正
                    // ========================================
                    mPipeProcess.TryGetValue(PipelineKey.BlackWhiteLevel, out obj);
                    switch (obj)
                    {
                        case enumBlackWhiteLevel.Default:
                            ec = isp.BlackAndWhiteLevelCorrection(ref Output_Channel, black, white);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"Black/white correction failed: {ec}";
                                return ErrCode;
                            }
                            break;
                    }

                    // ========================================
                    // 2. 白平衡
                    // ========================================
                    mPipeProcess.TryGetValue(PipelineKey.AutoWhiteBalance, out obj);
                    double gainR = 0, gainG = 0, gainB = 0;
                    switch (obj)
                    {
                        case enumAutoWhiteBalance.Default:
                            gainR = cam_mul[0] / cam_mul[1];
                            gainG = 1.0;
                            gainB = cam_mul[2] / cam_mul[1];
                            break;
                        case enumAutoWhiteBalance.GrayWorld:
                            ec = isp.CalAWBGain_GrayWorld(ref Output_Channel, out gainR, out gainG, out gainB);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"AWB failed: {ec}";
                                return ErrCode;
                            }
                            break;
                        case enumAutoWhiteBalance.WhitePatch:
                            ec = isp.CalAWBGain_WhitePatch(ref Output_Channel, out gainR, out gainG, out gainB);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"AWB failed: {ec}";
                                return ErrCode;
                            }
                            break;
                    }
                    ec = isp.ApplyAWBGain(ref Output_Channel, height, width, gainR, gainG, gainB);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        ErrMsg = $"AWB failed: {ec}";
                        return ErrCode;
                    }

                    // ========================================
                    // 3. Demosaic - 已修改支援 AI 模型
                    // ========================================
                    enumAiModelType modelType;
                    mPipeProcess.TryGetValue(PipelineKey.Demosaic, out obj);
                    switch (obj)
                    {
                        case enumDemosaic.Default:
                            ec = isp.Demosaic(ref Output_Channel, out Output_Color);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"Demosaic failed: {ec}";
                                return ErrCode;
                            }
                            break;
                        case enumDemosaic.Ai_RawModel:
                        case enumDemosaic.Ai_OptimizedModel:
                        case enumDemosaic.Ai_Fp16Model:

                            // 1. 根據包含的類型賦值 (將 C++ 的 :: 全部修正為 C# 的 .)
                            if (obj.Equals(enumDemosaic.Ai_RawModel))
                                modelType = enumAiModelType.ModelRaw;
                            else if (obj.Equals(enumDemosaic.Ai_OptimizedModel))
                                modelType = enumAiModelType.ModelOptimized;
                            else
                                modelType = enumAiModelType.ModelFloat16;


                            // 2. 取得模型檔案路徑
                            string modelPath = GetModelPath(modelType);

                            // 3. 檢查模型檔案是否存在
                            if (!File.Exists(modelPath))
                            {
                                ErrMsg = $"AI model file not found: {modelPath}";
                                return ErrCode;
                            }

                            // 4. 呼叫 AI Demosaic
                            ec = isp.AiDemosaic(ref Output_Channel, out Output_Color, modelPath);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"AI Demosaic failed: {ec}, model: {modelPath}";
                                return ErrCode;
                            }
                            break;
                    }

                    // ========================================
                    // 4. 色彩校正
                    // ========================================
                    mPipeProcess.TryGetValue(PipelineKey.ColorCorrection, out obj);
                    switch (obj)
                    {
                        case enumColorCorrection.Default:
                            ISP_ErrCode ccmEc = isp.CalculateCCM(ref xyz_srgb, ref cam_xyz, out ISP_Mat ccm);
                            if (ccmEc != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"CCM calculation failed: {ccmEc}";
                                return ErrCode;
                            }

                            ec = isp.ColorCorrection(ref Output_Color, ref ccm, out Output_Color);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                ErrMsg = $"Color correction failed: {ec}";
                                return ErrCode;
                            }
                            break;
                    }

                    // ========================================
                    // 使用 P50 做正規化
                    // ========================================
                    ec = isp.NormalizeExposureByP50(ref Output_Color, Target_P50);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        ErrMsg = $"NormalizeExposureByP50 failed: {ec}";
                        return ErrCode;
                    }


                    // ========================================
                    // 5. 色調映射
                    // ========================================
                    mPipeProcess.TryGetValue(PipelineKey.ToneMapping, out obj);
                    switch (obj)
                    {
                        case enumToneMapping.Default:
                            ec = isp.ApplyToneMapping(ref Output_Color, 1.8f);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                Console.WriteLine($"Tone mapping failed: {ec}");
                                return ErrCode;
                            }
                            break;
                    }

                    // ========================================
                    // 6. 銳化
                    // ========================================
                    mPipeProcess.TryGetValue(PipelineKey.Sharpening, out obj);
                    switch (obj)
                    {
                        case enumSharpening.Default:
                            ec = isp.Sharpening(ref Output_Color, 0.5);
                            if (ec != ISP_ErrCode.Ok)
                            {
                                Console.WriteLine($"Sharpening failed: {ec}");
                                return ErrCode;
                            }
                            break;
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"StackTrace: {ex.StackTrace}");
            }
            return ErrCode;
        }
    }
}