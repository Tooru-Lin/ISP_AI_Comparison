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
                ofd.Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp";
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
                originalImage?.Dispose();
                // load as Bitmap and keep a copy
                using (var tmp = Image.FromFile(path))
                {
                    originalImage = new Bitmap(tmp);
                }

                pbDisplay1.Image?.Dispose();
                pbDisplay1.Image = null;
                pbDisplay2.Image?.Dispose();
                pbDisplay2.Image = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Failed to load image: " + ex.Message);
            }
        }

        private void BtnMeasure1_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;
            var out1 = isp1.Process(originalImage);
            pbDisplay1.Image?.Dispose();
            pbDisplay1.Image = (Bitmap)out1.Clone();
            var metrics = ComputeMetrics(originalImage, out1);
            lblMetrics1.Text = FormatMetrics(metrics);
        }

        private void BtnMeasure2_Click(object sender, EventArgs e)
        {
            if (!EnsureImageLoaded()) return;
            var out2 = isp2.Process(originalImage);
            pbDisplay2.Image?.Dispose();
            pbDisplay2.Image = (Bitmap)out2.Clone();
            var metrics = ComputeMetrics(originalImage, out2);
            lblMetrics2.Text = FormatMetrics(metrics);
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
            if (originalImage == null) return;
            if (pipelineIndex == 1)
            {
                var out1 = isp1.Process(originalImage);
                this.Invoke(new Action(() =>
                {
                    pbDisplay1.Image?.Dispose();
                    pbDisplay1.Image = (Bitmap)out1.Clone();
                    var metrics = ComputeMetrics(originalImage, out1);
                    lblMetrics1.Text = FormatMetrics(metrics);
                }));
            }
            else
            {
                var out2 = isp2.Process(originalImage);
                this.Invoke(new Action(() =>
                {
                    pbDisplay2.Image?.Dispose();
                    pbDisplay2.Image = (Bitmap)out2.Clone();
                    var metrics = ComputeMetrics(originalImage, out2);
                    lblMetrics2.Text = FormatMetrics(metrics);
                }));
            }
        }

        private bool EnsureImageLoaded()
        {
            if (originalImage != null) return true;
            // try from txtSourcePath first
            if (!string.IsNullOrWhiteSpace(txtSourcePath.Text) && File.Exists(txtSourcePath.Text))
            {
                LoadSourceImage(txtSourcePath.Text);
                return originalImage != null;
            }

            using (var ofd = new OpenFileDialog())
            {
                ofd.Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp";
                if (ofd.ShowDialog() != DialogResult.OK) return false;
                txtSourcePath.Text = ofd.FileName;
                LoadSourceImage(ofd.FileName);
            }
            return originalImage != null;
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