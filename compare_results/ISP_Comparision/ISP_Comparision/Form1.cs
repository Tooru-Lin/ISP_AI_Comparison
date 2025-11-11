using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace ISP_Comparision
{
    public partial class Form1 : Form
    {
        private Bitmap originalImage;
        private ISP_Pipeline isp1;
        private ISP_Pipeline isp2;
        private System.Windows.Forms.Timer liveTimer1;
        private System.Windows.Forms.Timer liveTimer2;

        public Form1()
        {
            InitializeComponent();
            InitializePipelines();
            InitializeTimers();
        }

        private void InitializePipelines()
        {
            isp1 = new ISP_Pipeline("ISP_1");
            isp2 = new ISP_Pipeline("ISP_2");

            // bind UI selections to pipeline options (safe to attach in code-behind)
            cbBWLevel1.SelectedIndexChanged += (s, e) => isp1.SetOption("Black & White Level", cbBWLevel1.SelectedItem.ToString());
            cbLensShading1.SelectedIndexChanged += (s, e) => isp1.SetOption("Lens Shading", cbLensShading1.SelectedItem.ToString());
            cbBadPixel1.SelectedIndexChanged += (s, e) => isp1.SetOption("Bad Pixel Correction", cbBadPixel1.SelectedItem.ToString());
            cbLinearity1.SelectedIndexChanged += (s, e) => isp1.SetOption("Linearity Correction", cbLinearity1.SelectedItem.ToString());
            cbDemosaic1.SelectedIndexChanged += (s, e) => isp1.SetOption("Demosaic", cbDemosaic1.SelectedItem.ToString());
            cbAWB1.SelectedIndexChanged += (s, e) => isp1.SetOption("Auto White Balance", cbAWB1.SelectedItem.ToString());
            cbCCM1.SelectedIndexChanged += (s, e) => isp1.SetOption("Color Correction Matrix", cbCCM1.SelectedItem.ToString());
            cbNoise1.SelectedIndexChanged += (s, e) => isp1.SetOption("Noise Reduction", cbNoise1.SelectedItem.ToString());
            cbTone1.SelectedIndexChanged += (s, e) => isp1.SetOption("Tone Mapping", cbTone1.SelectedItem.ToString());
            cbDistort1.SelectedIndexChanged += (s, e) => isp1.SetOption("Distortion Correction", cbDistort1.SelectedItem.ToString());
            cbSharpen1.SelectedIndexChanged += (s, e) => isp1.SetOption("Sharpening", cbSharpen1.SelectedItem.ToString());
            
            cbBWLevel2.SelectedIndexChanged += (s, e) => isp2.SetOption("Black & White Level", cbBWLevel2.SelectedItem.ToString());
            cbLensShading2.SelectedIndexChanged += (s, e) => isp2.SetOption("Lens Shading", cbLensShading2.SelectedItem.ToString());
            cbBadPixel2.SelectedIndexChanged += (s, e) => isp2.SetOption("Bad Pixel Correction", cbBadPixel2.SelectedItem.ToString());
            cbLinearity2.SelectedIndexChanged += (s, e) => isp2.SetOption("Linearity Correction", cbLinearity2.SelectedItem.ToString());
            cbDemosaic2.SelectedIndexChanged += (s, e) => isp2.SetOption("Demosaic", cbDemosaic2.SelectedItem.ToString());
            cbAWB2.SelectedIndexChanged += (s, e) => isp2.SetOption("Auto White Balance", cbAWB2.SelectedItem.ToString());
            cbCCM2.SelectedIndexChanged += (s, e) => isp2.SetOption("Color Correction Matrix", cbCCM2.SelectedItem.ToString());
            cbNoise2.SelectedIndexChanged += (s, e) => isp2.SetOption("Noise Reduction", cbNoise2.SelectedItem.ToString());
            cbTone2.SelectedIndexChanged += (s, e) => isp2.SetOption("Tone Mapping", cbTone2.SelectedItem.ToString());
            cbDistort2.SelectedIndexChanged += (s, e) => isp2.SetOption("Distortion Correction", cbDistort2.SelectedItem.ToString());
            cbSharpen2.SelectedIndexChanged += (s, e) => isp2.SetOption("Sharpening", cbSharpen2.SelectedItem.ToString());
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

    public static class ISPModules
    {
        // 每個測項一個 function：輸入 Bitmap 與選項字串，回傳處理後 Bitmap
        // 留空或簡單 placeholder（不使用 unsafe）。實際演算法由外部 DLL 引入時替換這些實作。

        public static Bitmap ApplyBlackWhiteLevel(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: adjust brightness slightly
            float factor = option == "Aggressive" ? 1.1f : 1.02f;
            return ApplyBrightness(src, factor);
        }

        public static Bitmap ApplyLensShading(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: slight center brighten using safe Graphics operations
            if (option == "Aggressive") return ApplyBrightness(src, 1.05f);
            return src;
        }

        public static Bitmap ApplyBadPixelCorrection(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: no-op (real implementation in external DLL)
            return src;
        }

        public static Bitmap ApplyLinearityCorrection(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: gamma adjust
            float gamma = option == "Aggressive" ? 0.9f : 0.98f;
            return ApplyGamma(src, gamma);
        }

        public static Bitmap ApplyDemosaic(Bitmap src, string option)
        {
            // if src already RGB, passthrough; placeholder
            return src;
        }

        public static Bitmap ApplyAutoWhiteBalance(Bitmap src, string option)
        {
            if (option == "None") return src;
            // safe, simple AWB via GetPixel/SetPixel sampling
            float scale = option == "Aggressive" ? 1.1f : 1.02f;
            return SimpleAWB(src, scale);
        }

        public static Bitmap ApplyColorCorrectionMatrix(Bitmap src, string option)
        {
            if (option == "None") return src;
            float scale = option == "Aggressive" ? 1.1f : 1.02f;
            return ApplyColorScale(src, scale);
        }

        public static Bitmap ApplyNoiseReduction(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: no-op (external DLL should implement)
            return src;
        }

        public static Bitmap ApplyToneMapping(Bitmap src, string option)
        {
            if (option == "None") return src;
            float intensity = option == "Aggressive" ? 1.2f : 1.05f;
            return ApplyToneCurve(src, intensity);
        }

        public static Bitmap ApplyDistortionCorrection(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: no-op
            return src;
        }

        public static Bitmap ApplySharpening(Bitmap src, string option)
        {
            if (option == "None") return src;
            // placeholder: no-op (external DLL should implement)
            return src;
        }

        // Helpers — all safe (no unsafe code)
        private static Bitmap ApplyBrightness(Bitmap src, float factor)
        {
            Bitmap bmp = new Bitmap(src.Width, src.Height);
            using (Graphics g = Graphics.FromImage(bmp))
            {
                float b = factor - 1f;
                var cm = new ColorMatrix(new float[][]
                {
                    new float[]{factor,0,0,0,0},
                    new float[]{0,factor,0,0,0},
                    new float[]{0,0,factor,0,0},
                    new float[]{0,0,0,1,0},
                    new float[]{b,b,b,0,1}
                });
                var ia = new ImageAttributes();
                ia.SetColorMatrix(cm);
                g.DrawImage(src, new Rectangle(0, 0, src.Width, src.Height), 0, 0, src.Width, src.Height, GraphicsUnit.Pixel, ia);
            }
            return bmp;
        }

        private static Bitmap ApplyGamma(Bitmap src, float gamma)
        {
            Bitmap bmp = new Bitmap(src.Width, src.Height);
            using (Graphics g = Graphics.FromImage(bmp))
            {
                var ia = new ImageAttributes();
                ia.SetGamma(gamma);
                g.DrawImage(src, new Rectangle(0, 0, src.Width, src.Height), 0, 0, src.Width, src.Height, GraphicsUnit.Pixel, ia);
            }
            return bmp;
        }

        private static Bitmap SimpleAWB(Bitmap src, float scale)
        {
            // safe but not optimal: iterate pixels
            Bitmap dst = new Bitmap(src.Width, src.Height, PixelFormat.Format24bppRgb);
            for (int y = 0; y < src.Height; y++)
            {
                for (int x = 0; x < src.Width; x++)
                {
                    Color c = src.GetPixel(x, y);
                    int r = (int)Math.Min(255, c.R * scale * 1.02f);
                    int g = (int)Math.Min(255, c.G * scale * 0.98f);
                    int b = (int)Math.Min(255, c.B * scale);
                    dst.SetPixel(x, y, Color.FromArgb(r, g, b));
                }
            }
            return dst;
        }

        private static Bitmap ApplyColorScale(Bitmap src, float scale)
        {
            Bitmap bmp = new Bitmap(src.Width, src.Height);
            using (Graphics g = Graphics.FromImage(bmp))
            {
                var cm = new ColorMatrix(new float[][]
                {
                    new float[]{scale,0,0,0,0},
                    new float[]{0,scale,0,0,0},
                    new float[]{0,0,scale,0,0},
                    new float[]{0,0,0,1,0},
                    new float[]{0,0,0,0,1}
                });
                var ia = new ImageAttributes();
                ia.SetColorMatrix(cm);
                g.DrawImage(src, new Rectangle(0, 0, src.Width, src.Height), 0, 0, src.Width, src.Height, GraphicsUnit.Pixel, ia);
            }
            return bmp;
        }

        private static Bitmap ApplyToneCurve(Bitmap src, float intensity)
        {
            // simple contrast-like transform via ColorMatrix (safe)
            Bitmap bmp = new Bitmap(src.Width, src.Height);
            using (Graphics g = Graphics.FromImage(bmp))
            {
                float t = intensity;
                float[][] ptsArray ={
                    new float[] {t, 0, 0, 0, 0},
                    new float[] {0, t, 0, 0, 0},
                    new float[] {0, 0, t, 0, 0},
                    new float[] {0, 0, 0, 1f, 0},
                    new float[] {0, 0, 0, 0, 1f}
                };
                var cm = new ColorMatrix(ptsArray);
                var ia = new ImageAttributes();
                ia.SetColorMatrix(cm);
                g.DrawImage(src, new Rectangle(0, 0, src.Width, src.Height), 0, 0, src.Width, src.Height, GraphicsUnit.Pixel, ia);
            }
            return bmp;
        }
    }

    public static class Measurements
    {
        // All measurements implemented without unsafe; using sampling or GetPixel (prototype quality)

        public static double ComputeSNR(Bitmap bmp)
        {
            // basic SNR estimate: mean / stddev on luminance using sampling
            double sum = 0, sum2 = 0;
            int samples = 0;
            int stepX = Math.Max(1, bmp.Width / 100);
            int stepY = Math.Max(1, bmp.Height / 100);
            for (int y = 0; y < bmp.Height; y += stepY)
            {
                for (int x = 0; x < bmp.Width; x += stepX)
                {
                    Color c = bmp.GetPixel(x, y);
                    double lum = 0.2126 * c.R + 0.7152 * c.G + 0.0722 * c.B;
                    sum += lum;
                    sum2 += lum * lum;
                    samples++;
                }
            }
            if (samples == 0) return 0;
            double mean = sum / samples;
            double variance = sum2 / samples - mean * mean;
            double std = Math.Sqrt(Math.Max(variance, 1e-9));
            return mean / std;
        }

        public static double ComputeMTF(Bitmap bmp)
        {
            // rough MTF proxy: local contrast on subsampled neighbor pixels
            double contrastSum = 0;
            int count = 0;
            int stepY = Math.Max(1, bmp.Height / 50);
            int stepX = Math.Max(1, bmp.Width / 50);
            for (int y = 1; y < bmp.Height - 1; y += stepY)
            {
                for (int x = 1; x < bmp.Width - 1; x += stepX)
                {
                    Color c = bmp.GetPixel(x, y);
                    Color r = bmp.GetPixel(Math.Min(bmp.Width - 1, x + 1), y);
                    double lum = (c.R + c.G + c.B) / 3.0;
                    double lumr = (r.R + r.G + r.B) / 3.0;
                    contrastSum += Math.Abs(lum - lumr) / 255.0;
                    count++;
                }
            }
            return contrastSum / Math.Max(1, count);
        }

        public static double ComputeDeltaEApprox(Bitmap a, Bitmap b)
        {
            // simplified ΔE-like estimate using RGB Euclidean distance on subsampled grid
            int w = Math.Min(a.Width, b.Width);
            int h = Math.Min(a.Height, b.Height);
            int stepX = Math.Max(1, w / 50);
            int stepY = Math.Max(1, h / 50);
            double sum = 0;
            int n = 0;
            for (int y = 0; y < h; y += stepY)
            {
                for (int x = 0; x < w; x += stepX)
                {
                    Color c1 = a.GetPixel(x, y);
                    Color c2 = b.GetPixel(x, y);
                    double d = Math.Sqrt(Math.Pow(c1.R - c2.R, 2) + Math.Pow(c1.G - c2.G, 2) + Math.Pow(c1.B - c2.B, 2));
                    sum += d;
                    n++;
                }
            }
            return sum / Math.Max(1, n);
        }

        public static double ComputeTonePeak(Bitmap bmp)
        {
            // find peak luminance (0..1) on subsampled grid
            double peak = 0;
            int stepY = Math.Max(1, bmp.Height / 100);
            int stepX = Math.Max(1, bmp.Width / 100);
            for (int y = 0; y < bmp.Height; y += stepY)
            {
                for (int x = 0; x < bmp.Width; x += stepX)
                {
                    Color c = bmp.GetPixel(x, y);
                    double lum = (0.2126 * c.R + 0.7152 * c.G + 0.0722 * c.B) / 255.0;
                    if (lum > peak) peak = lum;
                }
            }
            return peak;
        }
    }

    public class ImageMetrics
    {
        public double SNR { get; set; }
        public double MTF { get; set; }
        public double DeltaE { get; set; }
        public double TonePeak { get; set; }
    }
}