using ISP_CSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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
    public enum enumDenoise { None = 0, Bilateral = 1 }
    public enum enumToneMapping { None = 0, Gamma1_8 = 1, Gamma2_2 = 2 }
    public enum enumDistortionCorrection { None = 0, Default = 1 }
    public enum enumSharpening { None = 0, UnsharpMask = 1 }

    // 新增：AI 模型選擇 enum
    public enum enumAiModelType { ModelRaw = 0, ModelOptimized = 1, ModelFloat16 = 2 }

    // Pipeline key 改成 enum
    public enum PipelineKey
    {
        BlackWhiteLevel,
        LensShading,
        BadPixelCorrection,
        LinearityCorrection,
        Denoise,
        Demosaic,
        AutoWhiteBalance,
        ColorCorrection,
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

        // 新增：儲存上一次 Measure 的每模組耗時
        public Dictionary<string, TimeSpan> LastModuleTimings { get; private set; } = new Dictionary<string, TimeSpan>();


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
                { PipelineKey.Denoise, typeof(enumDenoise) },
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
                { PipelineKey.Denoise, enumDenoise.Bilateral },
                { PipelineKey.ToneMapping, enumToneMapping.Gamma1_8 },
                { PipelineKey.DistortionCorrection, enumDistortionCorrection.Default },
                { PipelineKey.Sharpening, enumSharpening.UnsharpMask },
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

        public int Measure(Dictionary<string, object> inputs, out Dictionary<string, object> outputs)
        {
            outputs = new Dictionary<string, object>(StringComparer.OrdinalIgnoreCase);
            var moduleTimings = new Dictionary<string, TimeSpan>(StringComparer.OrdinalIgnoreCase);

            int ErrCode = 0;
            string ErrMsg = "";
            object obj;
            ISP_Mat Output_Color = new ISP_Mat();
            ISP_Mat Output_Grey = new ISP_Mat();

            try
            {
                // 解析 inputs
                string rawPath = null;
                float Target_P50 = 0.18f;
                if (inputs != null)
                {
                    if (inputs.TryGetValue("ImagePath", out var ip) && ip is string s) rawPath = s;
                    if (inputs.TryGetValue("Target_P50", out var p50obj))
                    {
                        if (p50obj is float f) Target_P50 = f;
                        else if (p50obj is double d) Target_P50 = (float)d;
                        else if (p50obj is decimal dec) Target_P50 = (float)dec;
                        else
                        {
                            float.TryParse(p50obj?.ToString() ?? "", out Target_P50);
                        }
                    }
                }

                if (string.IsNullOrWhiteSpace(rawPath))
                {
                    ErrMsg = "ImagePath not provided";
                    return ErrCode;
                }

                NativeDiagnostics.DiagnoseIspDll(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "isp_traditional.dll"));

                using (var isp = new ISP_Processor())
                {
                    if (!File.Exists(rawPath))
                    {
                        ErrMsg = $"File not found: {rawPath}";
                        return ErrCode;
                    }

                    var sw = new Stopwatch();
                    float[] cam_mul = new float[4];
                    ISP_ErrCode ec = isp.LoadRawWithLibRaw(
                        rawPath,
                        out int width,
                        out int height,
                        out int black,
                        out int white,
                        cam_mul,
                        out ISP_Mat cam_rgb,
                        out Output_Grey);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        ErrMsg = $"Failed to load RAW file: {ec}";
                        return ErrCode;
                    }

                    // 1. 黑白電平校正
                    sw.Restart();
                    mPipeProcess.TryGetValue(PipelineKey.BlackWhiteLevel, out obj);
                    switch (obj)
                    {
                        case enumBlackWhiteLevel.Default:
                            ec = isp.BlackAndWhiteLevelCorrection(ref Output_Grey, black, white);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"Black/white correction failed: {ec}"; }
                            break;
                    }
                    sw.Stop();
                    moduleTimings["BlackWhiteLevel"] = sw.Elapsed;

                    // 2. 白平衡
                    sw.Restart();
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
                            ec = isp.CalAWBGain_GrayWorld(ref Output_Grey, out gainR, out gainG, out gainB);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"AWB failed: {ec}"; }
                            break;
                        case enumAutoWhiteBalance.WhitePatch:
                            ec = isp.CalAWBGain_WhitePatch(ref Output_Grey, out gainR, out gainG, out gainB);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"AWB failed: {ec}"; }
                            break;
                    }
                    ec = isp.ApplyAWBGain(ref Output_Grey, height, width, gainR, gainG, gainB);
                    if (ec != ISP_ErrCode.Ok) { ErrMsg = $"AWB failed: {ec}"; }
                    sw.Stop();
                    moduleTimings["AutoWhiteBalance"] = sw.Elapsed;


                    // 3. Denoise (Bilateral)
                    sw.Restart();
                    mPipeProcess.TryGetValue(PipelineKey.Denoise, out obj);
                    switch (obj)
                    {
                        case enumDenoise.Bilateral:
                            ec = isp.Denoise_Bilateral(ref Output_Grey, 0.03f, 5);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"Denoise failed: {ec}"; }
                            break;
                    }
                    sw.Stop();
                    moduleTimings["Denoise"] = sw.Elapsed;

                    // 4. Demosaic (含 AI)
                    sw.Restart();
                    enumAiModelType modelType;
                    mPipeProcess.TryGetValue(PipelineKey.Demosaic, out obj);
                    switch (obj)
                    {
                        case enumDemosaic.Default:
                            ec = isp.Demosaic(ref Output_Grey, out Output_Color);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"Demosaic failed: {ec}"; }
                            break;
                        case enumDemosaic.Ai_RawModel:
                        case enumDemosaic.Ai_OptimizedModel:
                        case enumDemosaic.Ai_Fp16Model:
                            if (obj.Equals(enumDemosaic.Ai_RawModel)) modelType = enumAiModelType.ModelRaw;
                            else if (obj.Equals(enumDemosaic.Ai_OptimizedModel)) modelType = enumAiModelType.ModelOptimized;
                            else modelType = enumAiModelType.ModelFloat16;

                            string modelPath = GetModelPath(modelType);
                            if (!File.Exists(modelPath)) { ErrMsg = $"AI model file not found: {modelPath}";}

                            ec = isp.AiDemosaic(ref Output_Grey, out Output_Color, modelPath);
                            if (ec != ISP_ErrCode.Ok) { ErrMsg = $"AI Demosaic failed: {ec}, model: {modelPath}"; }
                            break;
                        case enumDemosaic.None:
                            Output_Color = Output_Grey;
                            break;
                    }
                    sw.Stop();
                    moduleTimings["Demosaic"] = sw.Elapsed;


                    // 5. 色彩校正
                    sw.Restart();
                    mPipeProcess.TryGetValue(PipelineKey.ColorCorrection, out obj);

                    //IntPtr dataPtr = cam_rgb.data; // 你的 IntPtr 指標
                    //float[] matrix = new float[9];

                    //// 從 IntPtr 記憶體位置複製 9 個 float (36 bytes) 到 C# 陣列
                    //Marshal.Copy(dataPtr, matrix, 0, 9);

                    //Console.WriteLine("cam_rgb (3x3):");
                    //for (int i = 0; i < 3; i++)
                    //{
                    //    for (int j = 0; j < 3; j++)
                    //    {
                    //        float val = matrix[i * 3 + j];
                    //        Console.Write($"{val,12:F6} ");
                    //    }
                    //    Console.WriteLine();
                    //}

                    switch (obj)
                    {
                        case enumColorCorrection.Default:
                            ISP_ErrCode colorEc = isp.ColorCorrection(ref Output_Color, ref cam_rgb, out Output_Color);
                            if (colorEc != ISP_ErrCode.Ok) { ErrMsg = $"Color correction failed: {colorEc}"; }
                            break;
                    }
                    sw.Stop();
                    moduleTimings["ColorCorrection"] = sw.Elapsed;
                    

                    // 6. Normalize by P50
                    sw.Restart();
                    ec = isp.NormalizeExposureByP50(ref Output_Color, Target_P50);
                    if (ec != ISP_ErrCode.Ok) { ErrMsg = $"NormalizeExposureByP50 failed: {ec}";}
                    sw.Stop();
                    moduleTimings["NormalizeExposureByP50"] = sw.Elapsed;

                    // 6. Tone mapping
                    sw.Restart();
                    mPipeProcess.TryGetValue(PipelineKey.ToneMapping, out obj);
                    switch (obj)
                    {
                        case enumToneMapping.Gamma1_8:
                            ec = isp.ApplyToneMapping(ref Output_Color, 1.8f);
                            if (ec != ISP_ErrCode.Ok) { Console.WriteLine($"Tone mapping failed: {ec}");}
                            break;
                        case enumToneMapping.Gamma2_2:
                            ec = isp.ApplyToneMapping(ref Output_Color, 2.2f);
                            if (ec != ISP_ErrCode.Ok) { Console.WriteLine($"Tone mapping failed: {ec}");}
                            break;
                    }
                    sw.Stop();
                    moduleTimings["ToneMapping"] = sw.Elapsed;

                    // 7. Sharpening
                    sw.Restart();
                    mPipeProcess.TryGetValue(PipelineKey.Sharpening, out obj);
                    switch (obj)
                    {
                        case enumSharpening.UnsharpMask:
                            ec = isp.Sharpening(ref Output_Color, 0.5);
                            if (ec != ISP_ErrCode.Ok) { Console.WriteLine($"Sharpening failed: {ec}"); return ErrCode; }
                            break;
                    }
                    sw.Stop();
                    moduleTimings["Sharpening"] = sw.Elapsed;

                    // 完成 — 把 results 放回 outputs
                    outputs["Output_Color"] = Output_Color;
                    outputs["Output_Grey"] = Output_Grey;
                    outputs["Timings"] = moduleTimings;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"StackTrace: {ex.StackTrace}");
                ErrMsg = ex.Message;
            }

            // 若需要，可把 ErrMsg / ErrCode 加回 outputs
            outputs["ErrMsg"] = ErrMsg;
            outputs["ErrCode"] = ErrCode;

            return ErrCode;
        }
    }
}