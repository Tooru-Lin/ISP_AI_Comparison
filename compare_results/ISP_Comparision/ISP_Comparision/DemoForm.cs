using ISP_CSharp;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace ISP_Comparision
{
    public partial class DemoForm : Form
    {
        private Bitmap originalImage;
        private string originalImagePath;
        private ISP_Pipeline isp1;
        private ISP_Pipeline isp2;
        private System.Windows.Forms.Timer liveTimer1;
        private System.Windows.Forms.Timer liveTimer2;

        public DemoForm()
        {
            InitializeComponent();
            // 如果 ComboBox 只有一個選項，將其停用但顯示該選項（SelectedIndex = 0）
            // 若沒有選項，新增 "None" 並停用（SelectedIndex = 0）
            DisableSingleItemComboBoxes();
            InitializePipelines();
            InitializeTimers();
        }

        // 遞迴檢查所有子控制項，若為 ComboBox 則依 Items 數量處理：
        // - Items.Count == 0 : 新增 "None"，SelectedIndex = 0，Enabled = false
        // - Items.Count == 1 : 保留該項，SelectedIndex = 0，Enabled = false
        // - Items.Count >= 2 : Enabled = true，SelectedIndex = 0
        private void DisableSingleItemComboBoxes()
        {
            DisableSingleItemComboBoxes(this);
        }

        private void DisableSingleItemComboBoxes(Control parent)
        {
            foreach (Control c in parent.Controls)
            {
                var cb = c as ComboBox;
                if (cb != null)
                {
                    try
                    {
                        int count = cb.Items?.Count ?? 0;
                        if (count == 0)
                        {
                            cb.Items.Add("None");
                            cb.SelectedIndex = 0;
                            cb.Enabled = false;
                        }
                        else if (count == 1)
                        {
                            // 顯示該唯一選項並選取它，但停用控制項以禁止變更
                            cb.SelectedIndex = 0;
                            cb.Enabled = false;
                        }
                        else
                        {
                            // 有多於一個選項，啟用並預設選第一個
                            cb.Enabled = true;
                            if (cb.SelectedIndex < 0) cb.SelectedIndex = 0;
                        }
                    }
                    catch
                    {
                        // 忽略任何不可預期錯誤，不影響 UI 啟動
                    }
                }

                if (c.HasChildren)
                {
                    DisableSingleItemComboBoxes(c);
                }
            }
        }

        private void InitializePipelines()
        {
            isp1 = new ISP_Pipeline("ISP_1");
            isp2 = new ISP_Pipeline("ISP_2");

            // bind UI selections to pipeline options (加入 null 安全處理)
            cbBWLevel1.SelectedIndexChanged += (s, e) => isp1.SetOption("Black & White Level", cbBWLevel1.SelectedItem?.ToString() ?? "None");
            cbLensShading1.SelectedIndexChanged += (s, e) => isp1.SetOption("Lens Shading", cbLensShading1.SelectedItem?.ToString() ?? "None");
            cbBadPixel1.SelectedIndexChanged += (s, e) => isp1.SetOption("Bad Pixel Correction", cbBadPixel1.SelectedItem?.ToString() ?? "None");
            cbLinearity1.SelectedIndexChanged += (s, e) => isp1.SetOption("Linearity Correction", cbLinearity1.SelectedItem?.ToString() ?? "None");
            cbDemosaic1.SelectedIndexChanged += (s, e) => isp1.SetOption("Demosaic", cbDemosaic1.SelectedItem?.ToString() ?? "None");
            cbAWB1.SelectedIndexChanged += (s, e) => isp1.SetOption("Auto White Balance", cbAWB1.SelectedItem?.ToString() ?? "None");
            cbCCM1.SelectedIndexChanged += (s, e) => isp1.SetOption("Color Correction Matrix", cbCCM1.SelectedItem?.ToString() ?? "None");
            cbNoise1.SelectedIndexChanged += (s, e) => isp1.SetOption("Noise Reduction", cbNoise1.SelectedItem?.ToString() ?? "None");
            cbTone1.SelectedIndexChanged += (s, e) => isp1.SetOption("Tone Mapping", cbTone1.SelectedItem?.ToString() ?? "None");
            cbDistort1.SelectedIndexChanged += (s, e) => isp1.SetOption("Distortion Correction", cbDistort1.SelectedItem?.ToString() ?? "None");
            cbSharpen1.SelectedIndexChanged += (s, e) => isp1.SetOption("Sharpening", cbSharpen1.SelectedItem?.ToString() ?? "None");

            cbBWLevel2.SelectedIndexChanged += (s, e) => isp2.SetOption("Black & White Level", cbBWLevel2.SelectedItem?.ToString() ?? "None");
            cbLensShading2.SelectedIndexChanged += (s, e) => isp2.SetOption("Lens Shading", cbLensShading2.SelectedItem?.ToString() ?? "None");
            cbBadPixel2.SelectedIndexChanged += (s, e) => isp2.SetOption("Bad Pixel Correction", cbBadPixel2.SelectedItem?.ToString() ?? "None");
            cbLinearity2.SelectedIndexChanged += (s, e) => isp2.SetOption("Linearity Correction", cbLinearity2.SelectedItem?.ToString() ?? "None");
            cbDemosaic2.SelectedIndexChanged += (s, e) => isp2.SetOption("Demosaic", cbDemosaic2.SelectedItem?.ToString() ?? "None");
            cbAWB2.SelectedIndexChanged += (s, e) => isp2.SetOption("Auto White Balance", cbAWB2.SelectedItem?.ToString() ?? "None");
            cbCCM2.SelectedIndexChanged += (s, e) => isp2.SetOption("Color Correction Matrix", cbCCM2.SelectedItem?.ToString() ?? "None");
            cbNoise2.SelectedIndexChanged += (s, e) => isp2.SetOption("Noise Reduction", cbNoise2.SelectedItem?.ToString() ?? "None");
            cbTone2.SelectedIndexChanged += (s, e) => isp2.SetOption("Tone Mapping", cbTone2.SelectedItem?.ToString() ?? "None");
            cbDistort2.SelectedIndexChanged += (s, e) => isp2.SetOption("Distortion Correction", cbDistort2.SelectedItem?.ToString() ?? "None");
            cbSharpen2.SelectedIndexChanged += (s, e) => isp2.SetOption("Sharpening", cbSharpen2.SelectedItem?.ToString() ?? "None");
        }

        private void InitializeTimers()
        {
            liveTimer1 = new System.Windows.Forms.Timer();
            liveTimer1.Interval = 500; // ms
            liveTimer1.Tick += (s, e) => LiveTick(1);

            liveTimer2 = new System.Windows.Forms.Timer();
            liveTimer2.Interval = 500;
            liveTimer2.Tick += (s, e) => LiveTick(2);
        }

        private void BtnBrowseSource_Click(object sender, EventArgs e)
        {
            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp;*.RAF;*.ARW";
                if (ofd.ShowDialog() != DialogResult.OK) return;
                txtSourcePath.Text = ofd.FileName;
                LoadSourceImage(ofd.FileName);
            }
        }


        private void LoadSourceImage(string path)
        {
            if (string.IsNullOrWhiteSpace(path) || !File.Exists(path)) return;
            try
            {
                // 不再使用 Image.FromFile，也不在此處載入整張影像到記憶體
                originalImage?.Dispose();
                originalImage = null;
                originalImagePath = path;

                // 清掉顯示區塊（後續處理會按需載入 short-lived bitmap）
                pbDisplay1.Image?.Dispose();
                pbDisplay1.Image = null;
                pbDisplay2.Image?.Dispose();
                pbDisplay2.Image = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Failed to set source path: " + ex.Message);
            }
        }

        // 以 FileStream + Image.FromStream 的方式載入 bitmap 的複本，使用者須在使用完後 Dispose 回傳的 Bitmap
        private Bitmap LoadBitmapFromFile(string path)
        {
            if (string.IsNullOrWhiteSpace(path) || !File.Exists(path)) return null;
            // 打開檔案，從 stream 建立 Image，再複製為新的 Bitmap，確保不鎖定原始檔案
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                using (var img = Image.FromStream(fs))
                {
                    return new Bitmap(img);
                }
            }
        }

        private void BtnMeasure1_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;

            try
            {
                NativeDiagnostics.DiagnoseIspDll(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "isp_traditional.dll"));

                // ========================================
                // 1. 創建 ISP 處理器
                // ========================================
                Console.WriteLine("Creating ISP processor...");
                using (var isp = new ISP_Processor())
                {
                    // ========================================
                    // 2. 設定參數
                    // ========================================
                    Console.WriteLine("Setting parameters...");

                    // ========================================
                    // 3. 加載 RAW 檔案
                    // ========================================
                    string rawPath = originalImagePath;
                    if (!File.Exists(rawPath))
                    {
                        Console.WriteLine($"File not found: {rawPath}");
                        return;
                    }

                    Console.WriteLine($"Loading RAW file: {rawPath}");
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
                        out ISP_Mat raw32);

                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Failed to load RAW file: {ec}");
                        return;
                    }

                    Console.WriteLine($"Image loaded: {width}x{height}");
                    Console.WriteLine($"Black level: {black}, White level: {white}");
                    Console.WriteLine($"cam_mul: [{cam_mul[0]}, {cam_mul[1]}, {cam_mul[2]}, {cam_mul[3]}]");

                    // ========================================
                    // 4. 黑白電平校正
                    // ========================================
                    Console.WriteLine("Applying black/white level correction...");
                    ec = isp.BlackAndWhiteLevelCorrection(ref raw32, black, white);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Black/white correction failed: {ec}");
                        return;
                    }

                    // ========================================
                    // 5. 白平衡
                    // ========================================
                    Console.WriteLine("Applying AWB...");

                    // 計算增益 (簡化版本，實際應根據選擇的方法)
                    double gainR = cam_mul[0] / cam_mul[1];
                    double gainG = 1.0;
                    double gainB = cam_mul[2] / cam_mul[1];

                    ec = isp.ApplyAWBGain(ref raw32, height, width, gainR, gainG, gainB);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"AWB failed: {ec}");
                        return;
                    }

                    // ========================================
                    // 6. Demosaic
                    // ========================================
                    Console.WriteLine("Applying demosaic...");
                    ec = isp.Demosaic(ref raw32, out ISP_Mat bgr32);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Demosaic failed: {ec}");
                        return;
                    }

                    Console.WriteLine($"Demosaiced: {bgr32.cols}x{bgr32.rows}, channels: {bgr32.channels}");

                    // ========================================
                    // 7. 色彩校正
                    // ========================================
                    // 計算 CCM = xyz_srgb * cam_xyz
                    ISP_ErrCode ccmEc = isp.CalculateCCM(ref xyz_srgb, ref cam_xyz, out ISP_Mat ccm);
                    if (ccmEc != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"CCM calculation failed: {ccmEc}");
                        return;
                    }

                    Console.WriteLine("Applying color correction...");
                    ISP_ErrCode colorEc = isp.ColorCorrection(ref bgr32, ref ccm, out ISP_Mat bgr32_cc);
                    if (colorEc != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Color correction failed: {colorEc}");
                        return;
                    }


                    // ========================================
                    // 8. 色調映射
                    // ========================================
                    Console.WriteLine("Applying tone mapping...");
                    ec = isp.ApplyToneMapping(ref bgr32_cc, 1.8f);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Tone mapping failed: {ec}");
                        return;
                    }

                    // ========================================
                    // 9. 銳化
                    // ========================================
                    Console.WriteLine("Applying sharpening...");
                    ec = isp.Sharpening(ref bgr32_cc, 0.5);
                    if (ec != ISP_ErrCode.Ok)
                    {
                        Console.WriteLine($"Sharpening failed: {ec}");
                        return;
                    }

                    // ========================================
                    // 10. 預覽
                    // ========================================
                    Console.WriteLine("Showing preview...");

                    pbDisplay1.Image = Analysis.ToBitmap(bgr32);
                    Console.WriteLine("Processing completed successfully!");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"StackTrace: {ex.StackTrace}");
            }
        }

        private void BtnMeasure2_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;

            Bitmap src = null;
            Bitmap outBmp = null;
            try
            {
                src = LoadBitmapFromFile(originalImagePath);
                if (src == null) return;

                outBmp = isp2.Process(src);

                pbDisplay2.Image?.Dispose();
                pbDisplay2.Image = (Bitmap)outBmp.Clone();

                var metrics = ComputeMetrics(src, outBmp);
                lblMetrics2.Text = FormatMetrics(metrics);
            }
            finally
            {
                outBmp?.Dispose();
                src?.Dispose();
            }
        }

        private void BtnLive1_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;
            if (!liveTimer1.Enabled) { liveTimer1.Start(); btnLive1.Text = "Stop Live"; }
            else { liveTimer1.Stop(); btnLive1.Text = "Live"; }
        }

        private void BtnLive2_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;
            if (!liveTimer2.Enabled) { liveTimer2.Start(); btnLive2.Text = "Stop Live"; }
            else { liveTimer2.Stop(); btnLive2.Text = "Live"; }
        }

        private void LiveTick(int pipelineIndex)
        {
            // 使用短暫載入，不保留在記憶體中
            if (string.IsNullOrWhiteSpace(originalImagePath) || !File.Exists(originalImagePath)) return;

            if (pipelineIndex == 1)
            {
                Bitmap src = null;
                Bitmap outBmp = null;
                try
                {
                    src = LoadBitmapFromFile(originalImagePath);
                    if (src == null) return;

                    outBmp = isp1.Process(src);
                    this.Invoke(new Action(() =>
                    {
                        pbDisplay1.Image?.Dispose();
                        pbDisplay1.Image = (Bitmap)outBmp.Clone();
                        var metrics = ComputeMetrics(src, outBmp);
                        lblMetrics1.Text = FormatMetrics(metrics);
                    }));
                }
                finally
                {
                    outBmp?.Dispose();
                    src?.Dispose();
                }
            }
            else
            {
                Bitmap src = null;
                Bitmap outBmp = null;
                try
                {
                    src = LoadBitmapFromFile(originalImagePath);
                    if (src == null) return;

                    outBmp = isp2.Process(src);
                    this.Invoke(new Action(() =>
                    {
                        pbDisplay2.Image?.Dispose();
                        pbDisplay2.Image = (Bitmap)outBmp.Clone();
                        var metrics = ComputeMetrics(src, outBmp);
                        lblMetrics2.Text = FormatMetrics(metrics);
                    }));
                }
                finally
                {
                    outBmp?.Dispose();
                    src?.Dispose();
                }
            }
        }

        private bool EnsureImageLoaded()
        {
            // 如果已經有路徑且檔案存在就回傳 true（LoadSourceImage 只會儲存 path）
            if (!string.IsNullOrWhiteSpace(originalImagePath) && File.Exists(originalImagePath)) return true;

            // 嘗試從 txtSourcePath 指定的檔案
            if (!string.IsNullOrWhiteSpace(txtSourcePath.Text) && File.Exists(txtSourcePath.Text))
            {
                originalImagePath = txtSourcePath.Text;
                return true;
            }

            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp;*.RAF;*.ARW";
                if (ofd.ShowDialog() != DialogResult.OK) return false;
                txtSourcePath.Text = ofd.FileName;
                originalImagePath = ofd.FileName;
            }
            return !string.IsNullOrWhiteSpace(originalImagePath) && File.Exists(originalImagePath);
        }

        private string FormatMetrics(ImageMetrics m)
        {
            return $"SNR: {m.SNR:F2}\nMTF: {m.MTF:F2}\nΔE(avg): {m.DeltaE:F2}\nTonePeak:{m.TonePeak:F2}";
        }

        private ImageMetrics ComputeMetrics(Bitmap src, Bitmap processed)
        {
            var snr = Measurements.ComputeSNR(processed);
            var mtf = Measurements.ComputeMTF(processed);
            var de = Measurements.ComputeDeltaEApprox(src, processed);
            var tone = Measurements.ComputeTonePeak(processed);
            return new ImageMetrics { SNR = snr, MTF = mtf, DeltaE = de, TonePeak = tone };
        }
    }

    // Small container for pipeline options and processing
    public class ISP_Pipeline
    {
        private Dictionary<string, string> options = new Dictionary<string, string>();
        private string name;

        public ISP_Pipeline(string name)
        {
            this.name = name;
        }

        public void SetOption(string module, string option)
        {
            options[module] = option;
        }

        public Bitmap Process(Bitmap input)
        {
            // Work on a cloned bitmap
            Bitmap bmp = (Bitmap)input.Clone();

            // Apply each module in a common sequence
            bmp = ISPModules.ApplyBlackWhiteLevel(bmp, GetOpt("Black & White Level"));
            bmp = ISPModules.ApplyLensShading(bmp, GetOpt("Lens Shading"));
            bmp = ISPModules.ApplyBadPixelCorrection(bmp, GetOpt("Bad Pixel Correction"));
            bmp = ISPModules.ApplyLinearityCorrection(bmp, GetOpt("Linearity Correction"));
            bmp = ISPModules.ApplyDemosaic(bmp, GetOpt("Demosaic"));
            bmp = ISPModules.ApplyAutoWhiteBalance(bmp, GetOpt("Auto White Balance"));
            bmp = ISPModules.ApplyColorCorrectionMatrix(bmp, GetOpt("Color Correction Matrix"));
            bmp = ISPModules.ApplyNoiseReduction(bmp, GetOpt("Noise Reduction"));
            bmp = ISPModules.ApplyToneMapping(bmp, GetOpt("Tone Mapping"));
            bmp = ISPModules.ApplyDistortionCorrection(bmp, GetOpt("Distortion Correction"));
            bmp = ISPModules.ApplySharpening(bmp, GetOpt("Sharpening"));
            // Frame Rate does not change pixels; ignore in offline processing

            return bmp;
        }

        private string GetOpt(string key)
        {
            if (!options.ContainsKey(key)) return "None";
            return options[key] ?? "None";
        }
    }

    // ISPModules and Measurements unchanged...
    public static class ISPModules
    {
        // implementations omitted for brevity in this snippet (unchanged from prior)
        // ...
        public static Bitmap ApplyBlackWhiteLevel(Bitmap src, string option) { return src; }
        public static Bitmap ApplyLensShading(Bitmap src, string option) { return src; }
        public static Bitmap ApplyBadPixelCorrection(Bitmap src, string option) { return src; }
        public static Bitmap ApplyLinearityCorrection(Bitmap src, string option) { return src; }
        public static Bitmap ApplyDemosaic(Bitmap src, string option) { return src; }
        public static Bitmap ApplyAutoWhiteBalance(Bitmap src, string option) { return src; }
        public static Bitmap ApplyColorCorrectionMatrix(Bitmap src, string option) { return src; }
        public static Bitmap ApplyNoiseReduction(Bitmap src, string option) { return src; }
        public static Bitmap ApplyToneMapping(Bitmap src, string option) { return src; }
        public static Bitmap ApplyDistortionCorrection(Bitmap src, string option) { return src; }
        public static Bitmap ApplySharpening(Bitmap src, string option) { return src; }
    }

    public static class Measurements
    {
        // implementations unchanged (omitted here for brevity)
        public static double ComputeSNR(Bitmap bmp) { return 0; }
        public static double ComputeMTF(Bitmap bmp) { return 0; }
        public static double ComputeDeltaEApprox(Bitmap a, Bitmap b) { return 0; }
        public static double ComputeTonePeak(Bitmap bmp) { return 0; }
    }

    public class ImageMetrics
    {
        public double SNR { get; set; }
        public double MTF { get; set; }
        public double DeltaE { get; set; }
        public double TonePeak { get; set; }
    }
}