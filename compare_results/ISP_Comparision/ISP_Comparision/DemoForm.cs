using ISP_CSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace ISP_Comparision
{
    public partial class DemoForm : Form
    {
        private Bitmap originalImage;
        private string originalImagePath;
        private ISP_Pipeline isp1;
        private ISP_Pipeline isp2;
        private Controller mController1;
        private Controller mController2;
        private ImageBoxViewer viewer1;
        private ImageBoxViewer viewer2;
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

            mController1 = new Controller();
            mController2 = new Controller();

            // 動態用 enum 填充 ComboBox 項目，並綁定通用 handler
            PopulateComboBoxesFromEnums();

            viewer1 = ImageBoxViewer.Attach(pbDisplay1);
            viewer2 = ImageBoxViewer.Attach(pbDisplay2);

            // 自動讀取 Assembly 版本號並設定視窗標題
            SetAutoVersionTitle("ISP Comparison");
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
                            //cb.Enabled = true;
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
                string rawPath = originalImagePath;

                mController1.Measure(originalImagePath, out ISP_Mat Output_Color, out ISP_Mat Output_Channel, (float)nud_P50.Value);

                // 先 dispose/clear 由 viewer 處理
                if (Output_Color.data != IntPtr.Zero)
                    viewer1.SetImage(Analysis.ToBitmap(Output_Color));
                else if (Output_Color.channels == 3)
                    viewer1.SetImage(ToBitmapSelectChannel(Output_Color, 1, 1.0f));
                else
                    viewer1.SetImage(Analysis.ToBitmap(Output_Channel));

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

            try
            {
                string rawPath = originalImagePath;

                mController2.Measure(originalImagePath, out ISP_Mat Output_Color, out ISP_Mat Output_Channel, (float)nud_P50.Value);

                // 先 dispose/clear 由 viewer 處理
                if (Output_Color.data != IntPtr.Zero)
                    viewer2.SetImage(Analysis.ToBitmap(Output_Color));
                else if (Output_Color.channels == 3)
                    viewer2.SetImage(ToBitmapSelectChannel(Output_Color, 1, 1.0f));
                else
                    viewer2.SetImage(Analysis.ToBitmap(Output_Channel));

            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"StackTrace: {ex.StackTrace}");
            }
        }

        /// <summary>
        /// 從多通道 ISP_Mat 中擷取單一 channel 並回傳為 24bpp Bitmap（灰階複製成 RGB）。
        /// 支援 CV_8U (depth==0) 與 CV_32F (depth==5)；每列使用 mat.step 處理 padding。
        /// </summary>
        private Bitmap ToBitmapSelectChannel(ISP_Mat mat, int channelIndex, float floatScale = 1.0f)
        {
            if (mat.data == IntPtr.Zero) return null;
            int rows = mat.rows;
            int cols = mat.cols;
            int channels = mat.channels;
            if (rows <= 0 || cols <= 0) return null;
            if (channelIndex < 0 || channelIndex >= channels) return null;

            int depth = mat.type & 0x07; // OpenCV style depth
            PixelFormat pixelFormat = PixelFormat.Format24bppRgb;
            Bitmap bmp = new Bitmap(cols, rows, pixelFormat);
            BitmapData bd = bmp.LockBits(new Rectangle(0, 0, cols, rows), ImageLockMode.WriteOnly, pixelFormat);

            try
            {
                int bmpStride = Math.Abs(bd.Stride);
                byte[] dstRow = new byte[bmpStride];

                if (depth == 0) // CV_8U
                {
                    int srcRowBytes = cols * channels;
                    byte[] srcRow = new byte[srcRowBytes];
                    for (int r = 0; r < rows; r++)
                    {
                        IntPtr srcPtr = IntPtr.Add(mat.data, r * mat.step);
                        Marshal.Copy(srcPtr, srcRow, 0, srcRowBytes);

                        int dstOff = 0;
                        for (int c = 0; c < cols; c++)
                        {
                            byte v = srcRow[c * channels + channelIndex];
                            // Bitmap expects B,G,R
                            dstRow[dstOff++] = v;
                            dstRow[dstOff++] = v;
                            dstRow[dstOff++] = v;
                        }
                        // zero padding if any
                        for (int i = dstOff; i < dstRow.Length; i++) dstRow[i] = 0;
                        Marshal.Copy(dstRow, 0, IntPtr.Add(bd.Scan0, r * bd.Stride), dstRow.Length);
                    }
                }
                else if (depth == 5) // CV_32F
                {
                    int floatsPerRow = cols * channels;
                    float[] srcRow = new float[floatsPerRow];
                    for (int r = 0; r < rows; r++)
                    {
                        IntPtr srcPtr = IntPtr.Add(mat.data, r * mat.step);
                        Marshal.Copy(srcPtr, srcRow, 0, floatsPerRow);

                        int dstOff = 0;
                        for (int c = 0; c < cols; c++)
                        {
                            float fv = srcRow[c * channels + channelIndex] * floatScale;
                            int iv = (int)Math.Round(fv * 255f);
                            if (iv < 0) iv = 0;
                            if (iv > 255) iv = 255;
                            byte v = (byte)iv;
                            dstRow[dstOff++] = v;
                            dstRow[dstOff++] = v;
                            dstRow[dstOff++] = v;
                        }
                        for (int i = dstOff; i < dstRow.Length; i++) dstRow[i] = 0;
                        Marshal.Copy(dstRow, 0, IntPtr.Add(bd.Scan0, r * bd.Stride), dstRow.Length);
                    }
                }
                else
                {
                    // 不支援的深度
                    bmp.UnlockBits(bd);
                    bmp.Dispose();
                    return null;
                }
            }
            finally
            {
                try { bmp.UnlockBits(bd); } catch { }
            }

            return bmp;
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

        private void cbBWLevel1_SelectedIndexChanged(object sender, EventArgs e)
        {
        }

        private void cbLinearity1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        // ---------- 新增：根據 enum 動態填充 ComboBox 並綁定 handler ----------
        private void PopulateComboBoxesFromEnums()
        {
            // pipeline1 / pipeline2 對應的 ComboBox，依你窗體上的命名
            RegisterEnumCombo(cbBWLevel1, PipelineKey.BlackWhiteLevel, typeof(enumBlackWhiteLevel));
            RegisterEnumCombo(cbBWLevel2, PipelineKey.BlackWhiteLevel, typeof(enumBlackWhiteLevel));

            RegisterEnumCombo(cbLensShading1, PipelineKey.LensShading, typeof(enumLensShading));
            RegisterEnumCombo(cbLensShading2, PipelineKey.LensShading, typeof(enumLensShading));

            RegisterEnumCombo(cbBadPixel1, PipelineKey.BadPixelCorrection, typeof(enumBadPixelCorrection));
            RegisterEnumCombo(cbBadPixel2, PipelineKey.BadPixelCorrection, typeof(enumBadPixelCorrection));

            RegisterEnumCombo(cbLinearity1, PipelineKey.LinearityCorrection, typeof(enumLinearityCorrection));
            RegisterEnumCombo(cbLinearity2, PipelineKey.LinearityCorrection, typeof(enumLinearityCorrection));

            RegisterEnumCombo(cbDemosaic1, PipelineKey.Demosaic, typeof(enumDemosaic));
            RegisterEnumCombo(cbDemosaic2, PipelineKey.Demosaic, typeof(enumDemosaic));

            RegisterEnumCombo(cbAWB1, PipelineKey.AutoWhiteBalance, typeof(enumAutoWhiteBalance));
            RegisterEnumCombo(cbAWB2, PipelineKey.AutoWhiteBalance, typeof(enumAutoWhiteBalance));

            RegisterEnumCombo(cbCCM1, PipelineKey.ColorCorrection, typeof(enumColorCorrection));
            RegisterEnumCombo(cbCCM2, PipelineKey.ColorCorrection, typeof(enumColorCorrection));

            RegisterEnumCombo(cbNoise1, PipelineKey.NoiseReduction, typeof(enumNoiseReduction));
            RegisterEnumCombo(cbNoise2, PipelineKey.NoiseReduction, typeof(enumNoiseReduction));

            RegisterEnumCombo(cbTone1, PipelineKey.ToneMapping, typeof(enumToneMapping));
            RegisterEnumCombo(cbTone2, PipelineKey.ToneMapping, typeof(enumToneMapping));

            RegisterEnumCombo(cbDistort1, PipelineKey.DistortionCorrection, typeof(enumDistortionCorrection));
            RegisterEnumCombo(cbDistort2, PipelineKey.DistortionCorrection, typeof(enumDistortionCorrection));

            RegisterEnumCombo(cbSharpen1, PipelineKey.Sharpening, typeof(enumSharpening));
            RegisterEnumCombo(cbSharpen2, PipelineKey.Sharpening, typeof(enumSharpening));
        }

        private void RegisterEnumCombo(ComboBox cb, PipelineKey key, Type enumType)
        {
            if (cb == null || enumType == null) return;

            cb.BeginUpdate();
            cb.Items.Clear();
            var names = Enum.GetNames(enumType);
            foreach (var n in names) cb.Items.Add(n);
            // 保證至少有一項
            if (cb.Items.Count == 0) cb.Items.Add("None");
            
            cb.SelectedIndex = (cb.Items.Count > 0) ? 1 : 0;

            if (!cb.Enabled) cb.SelectedIndex = 0; // 如果 ComboBox 被停用，選擇第一個項目（通常是 "None"）

            // 使用 Tag 存放對應資訊： (PipelineKey, enumType)
            cb.Tag = new Tuple<PipelineKey, Type>(key, enumType);

            // 綁定通用 handler（不會移除 designer 綁定的 handler；雙重設定會被接受）
            cb.SelectedIndexChanged -= EnumCombo_SelectedIndexChanged;
            cb.SelectedIndexChanged += EnumCombo_SelectedIndexChanged;
            cb.EndUpdate();
        }

        private void EnumCombo_SelectedIndexChanged(object sender, EventArgs e)
        {
            var cb = sender as ComboBox;
            if (cb == null) return;
            if (cb.SelectedIndex < 0) return;
            var t = cb.Tag as Tuple<PipelineKey, Type>;
            if (t == null) return;

            var key = t.Item1;
            var enumType = t.Item2;

            // 以選取的名稱解析 enum 值，再用字串 overload 更新 controller
            var selectedName = cb.SelectedItem?.ToString();
            if (string.IsNullOrEmpty(selectedName)) return;

            try
            {
                var enumValueObj = Enum.Parse(enumType, selectedName);
                // 使用 Controller 的 string overload（會解析 key name）或直接使用 PipelineKey 轉換
                // 這裡直接呼叫 string overload以避免泛型反射

                if (((ComboBox)sender).Name.EndsWith("1"))
                {
                    mController1.SetParam(key.ToString(), (Enum)enumValueObj);
                }
                else if (((ComboBox)sender).Name.EndsWith("2"))
                {
                    mController2.SetParam(key.ToString(), (Enum)enumValueObj);
                }
            }
            catch
            {
                // 忽略解析錯誤（不應該發生）
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

        //private void CopyModelsFromSolutionToExe()
        //{
        //    // 嘗試向上尋找 .sln 檔案
        //    var solutionDir = FindSolutionRoot();
        //    if (solutionDir == null) return;

        //    string modelsSource = Path.Combine(solutionDir.FullName, "ISP_Comparision", "models");
        //    if (!Directory.Exists(modelsSource)) return;

        //    string exeDir = AppDomain.CurrentDomain.BaseDirectory;
        //    // 遞迴複製
        //    CopyDirectoryRecursive(modelsSource, exeDir);
        //}

        //private DirectoryInfo FindSolutionRoot()
        //{
        //    DirectoryInfo dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
        //    while (dir != null)
        //    {
        //        try
        //        {
        //            var slnFiles = dir.GetFiles("*.sln", SearchOption.TopDirectoryOnly);
        //            if (slnFiles != null && slnFiles.Length > 0)
        //            {
        //                return dir;
        //            }
        //        }
        //        catch
        //        {
        //            // 忽略無權限或 IO 問題
        //        }
        //        dir = dir.Parent;
        //    }
        //    return null;
        //}

        //private void CopyDirectoryRecursive(string sourceDir, string targetDir)
        //{
        //    // 建立 target 子目錄 models/* 保持結構
        //    foreach (var dirPath in Directory.GetDirectories(sourceDir, "*", SearchOption.AllDirectories))
        //    {
        //        string relative = GetRelativePathCompat(sourceDir, dirPath);
        //        string destSub = Path.Combine(targetDir, relative);
        //        if (!Directory.Exists(destSub)) Directory.CreateDirectory(destSub);
        //    }

        //    foreach (var filePath in Directory.GetFiles(sourceDir, "*", SearchOption.AllDirectories))
        //    {
        //        string relative = GetRelativePathCompat(sourceDir, filePath);
        //        string destPath = Path.Combine(targetDir, relative);

        //        try
        //        {
        //            string destFolder = Path.GetDirectoryName(destPath);
        //            if (!Directory.Exists(destFolder)) Directory.CreateDirectory(destFolder);
        //            File.Copy(filePath, destPath, true); // overwrite
        //        }
        //        catch (Exception ex)
        //        {
        //            // 不中斷流程，記錄偵錯資訊
        //            System.Diagnostics.Debug.WriteLine($"Copy file failed: {filePath} -> {destPath} : {ex.Message}");
        //        }
        //    }
        //}

        //private string GetRelativePathCompat(string basePath, string path)
        //{
        //    if (string.IsNullOrEmpty(basePath)) return path ?? string.Empty;
        //    if (string.IsNullOrEmpty(path)) return string.Empty;

        //    // 取得絕對路徑
        //    string baseFull = Path.GetFullPath(basePath);
        //    string targetFull = Path.GetFullPath(path);

        //    // 確保 baseFull 以目錄分隔符結尾，否則 MakeRelativeUri 行為會不正確
        //    if (!baseFull.EndsWith(Path.DirectorySeparatorChar.ToString(), StringComparison.Ordinal))
        //        baseFull += Path.DirectorySeparatorChar;

        //    var baseUri = new Uri(baseFull);
        //    var targetUri = new Uri(targetFull);
        //    var relUri = baseUri.MakeRelativeUri(targetUri);
        //    // Uri 會用 '/'，轉回平台分隔符
        //    string relative = Uri.UnescapeDataString(relUri.ToString()).Replace('/', Path.DirectorySeparatorChar);
        //    return relative;
        //}

        private void SetAutoVersionTitle(string baseTitle)
        {
            // 讀取當前 Assembly 的版本號 (Major.Minor.Build.Revision)
            Version version = Assembly.GetExecutingAssembly().GetName().Version;

            if (version != null)
            {
                // 格式化為 Major.Minor.Build.Revision (如: 0.0.6.0902)
                string verStr = $"{version.Major}.{version.Minor}.{version.Build}.{version.Revision}";

                // 動態組合標題
                this.Text = $"{baseTitle} (Ver. {verStr})";
            }
            else
            {
                this.Text = baseTitle;
            }
        }
    }
}