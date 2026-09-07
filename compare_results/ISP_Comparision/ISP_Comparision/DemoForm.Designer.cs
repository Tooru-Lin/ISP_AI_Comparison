using System;
using System.Windows.Forms;

namespace ISP_Comparision
{
    partial class DemoForm
    {
        private System.ComponentModel.IContainer components = null;
        private GroupBox gbPipeline1;
        private GroupBox gbPipeline2;
        private ComboBox cbBWLevel1;
        private ComboBox cbLensShading1;
        private ComboBox cbBadPixel1;
        private ComboBox cbLinearity1;
        private ComboBox cbDemosaic1;
        private ComboBox cbAWB1;
        private ComboBox cbCCM1;
        private ComboBox cbDenoise1;
        private ComboBox cbTone1;
        private ComboBox cbDistort1;
        private ComboBox cbSharpen1;
        private Button btnMeasure1;
        private Button btnLive1;
        private Button btnMeasure2;
        private Button btnLive2;
        private PictureBox pbDisplay1;
        private PictureBox pbDisplay2;
        private Panel panelMetrics1;
        private Panel panelMetrics2;
        private ComboBox cbBWLevel2;
        private ComboBox cbLensShading2;
        private ComboBox cbBadPixel2;
        private ComboBox cbLinearity2;
        private ComboBox cbDemosaic2;
        private ComboBox cbAWB2;
        private ComboBox cbCCM2;
        private ComboBox cbDenoise2;
        private ComboBox cbTone2;
        private ComboBox cbDistort2;
        private ComboBox cbSharpen2;
        private Label lblMetrics1;
        private Label lblMetrics2;
        private TextBox txtSourcePath;
        private Button btnBrowseSource;
        private Label lblSource;

        // Labels for pipeline1
        private Label lblBWLevel1;
        private Label lblLensShading1;
        private Label lblBadPixel1;
        private Label lblLinearity1;
        private Label lblDemosaic1;
        private Label lblAWB1;
        private Label lblCCM1;
        private Label lblDenoise1;
        private Label lblTone1;
        private Label lblDistort1;
        private Label lblSharpen1;
        // Labels for pipeline2
        private Label lblBWLevel2;
        private Label lblLensShading2;
        private Label lblBadPixel2;
        private Label lblLinearity2;
        private Label lblDemosaic2;
        private Label lblAWB2;
        private Label lblCCM2;
        private Label lblDenoise2;
        private Label lblTone2;
        private Label lblDistort2;
        private Label lblSharpen2;

        // Metrics controls placeholders
        private Label lblMTF1;
        private Label lblSNR1;
        private Label lblDE1;
        private Label lblFPS1;
        private PictureBox pbToneCurve1;

        private Label lblMTF2;
        private Label lblSNR2;
        private Label lblDE2;
        private Label lblFPS2;
        private PictureBox pbToneCurve2;

        private Label lblBWTime1;
        private Label lblAWBTime1;
        private Label lblDenoiseTime1;
        private Label lblDemosaicTime1;
        private Label lblCCMTime1;
        private Label lblToneTime1;
        private Label lblSharpenTime1;

        private Label lblBWTime2;
        private Label lblAWBTime2;
        private Label lblDenoiseTime2;
        private Label lblDemosaicTime2;
        private Label lblCCMTime2;
        private Label lblToneTime2;
        private Label lblSharpenTime2;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.gbPipeline1 = new System.Windows.Forms.GroupBox();
            this.lblOption1 = new System.Windows.Forms.Label();
            this.lblStage1 = new System.Windows.Forms.Label();
            this.lblBWLevel1 = new System.Windows.Forms.Label();
            this.cbBWLevel1 = new System.Windows.Forms.ComboBox();
            this.cbCCM1 = new System.Windows.Forms.ComboBox();
            this.cbDenoise1 = new System.Windows.Forms.ComboBox();
            this.cbAWB1 = new System.Windows.Forms.ComboBox();
            this.btnLive1 = new System.Windows.Forms.Button();
            this.lblCCM1 = new System.Windows.Forms.Label();
            this.btnMeasure1 = new System.Windows.Forms.Button();
            this.lblAWB1 = new System.Windows.Forms.Label();
            this.lblTone1 = new System.Windows.Forms.Label();
            this.cbTone1 = new System.Windows.Forms.ComboBox();
            this.cbSharpen1 = new System.Windows.Forms.ComboBox();
            this.cbDemosaic1 = new System.Windows.Forms.ComboBox();
            this.lblSharpen1 = new System.Windows.Forms.Label();
            this.lblBWTime1 = new System.Windows.Forms.Label();
            this.lblAWBTime1 = new System.Windows.Forms.Label();
            this.lblDenoise1 = new System.Windows.Forms.Label();
            this.lblDemosaicTime1 = new System.Windows.Forms.Label();
            this.lblCCMTime1 = new System.Windows.Forms.Label();
            this.lblToneTime1 = new System.Windows.Forms.Label();
            this.lblDemosaic1 = new System.Windows.Forms.Label();
            this.lblElapsedTime1 = new System.Windows.Forms.Label();
            this.lblSharpenTime1 = new System.Windows.Forms.Label();
            this.lblDenoiseTime1 = new System.Windows.Forms.Label();
            this.lblDemosaic2 = new System.Windows.Forms.Label();
            this.cbDenoise2 = new System.Windows.Forms.ComboBox();
            this.lblLensShading1 = new System.Windows.Forms.Label();
            this.cbLensShading1 = new System.Windows.Forms.ComboBox();
            this.lblBadPixel1 = new System.Windows.Forms.Label();
            this.cbBadPixel1 = new System.Windows.Forms.ComboBox();
            this.lblLinearity1 = new System.Windows.Forms.Label();
            this.cbLinearity1 = new System.Windows.Forms.ComboBox();
            this.lblDistort1 = new System.Windows.Forms.Label();
            this.cbDistort1 = new System.Windows.Forms.ComboBox();
            this.lblFPS1 = new System.Windows.Forms.Label();
            this.gbPipeline2 = new System.Windows.Forms.GroupBox();
            this.lblElapsedTime2 = new System.Windows.Forms.Label();
            this.lblOption2 = new System.Windows.Forms.Label();
            this.lblStage2 = new System.Windows.Forms.Label();
            this.lblBWLevel2 = new System.Windows.Forms.Label();
            this.cbBWLevel2 = new System.Windows.Forms.ComboBox();
            this.cbCCM2 = new System.Windows.Forms.ComboBox();
            this.lblDenoise2 = new System.Windows.Forms.Label();
            this.cbAWB2 = new System.Windows.Forms.ComboBox();
            this.btnMeasure2 = new System.Windows.Forms.Button();
            this.lblCCM2 = new System.Windows.Forms.Label();
            this.cbSharpen2 = new System.Windows.Forms.ComboBox();
            this.lblAWB2 = new System.Windows.Forms.Label();
            this.lblTone2 = new System.Windows.Forms.Label();
            this.cbTone2 = new System.Windows.Forms.ComboBox();
            this.btnLive2 = new System.Windows.Forms.Button();
            this.cbDemosaic2 = new System.Windows.Forms.ComboBox();
            this.lblSharpen2 = new System.Windows.Forms.Label();
            this.lblBWTime2 = new System.Windows.Forms.Label();
            this.lblAWBTime2 = new System.Windows.Forms.Label();
            this.lblDemosaicTime2 = new System.Windows.Forms.Label();
            this.lblCCMTime2 = new System.Windows.Forms.Label();
            this.lblToneTime2 = new System.Windows.Forms.Label();
            this.lblSharpenTime2 = new System.Windows.Forms.Label();
            this.lblDenoiseTime2 = new System.Windows.Forms.Label();
            this.lblLensShading2 = new System.Windows.Forms.Label();
            this.cbLensShading2 = new System.Windows.Forms.ComboBox();
            this.lblBadPixel2 = new System.Windows.Forms.Label();
            this.cbBadPixel2 = new System.Windows.Forms.ComboBox();
            this.lblLinearity2 = new System.Windows.Forms.Label();
            this.cbLinearity2 = new System.Windows.Forms.ComboBox();
            this.lblDistort2 = new System.Windows.Forms.Label();
            this.cbDistort2 = new System.Windows.Forms.ComboBox();
            this.lblFPS2 = new System.Windows.Forms.Label();
            this.pbDisplay1 = new System.Windows.Forms.PictureBox();
            this.pbDisplay2 = new System.Windows.Forms.PictureBox();
            this.panelMetrics1 = new System.Windows.Forms.Panel();
            this.lblMetrics1 = new System.Windows.Forms.Label();
            this.lblMTF1 = new System.Windows.Forms.Label();
            this.lblSNR1 = new System.Windows.Forms.Label();
            this.lblDE1 = new System.Windows.Forms.Label();
            this.pbToneCurve1 = new System.Windows.Forms.PictureBox();
            this.panelMetrics2 = new System.Windows.Forms.Panel();
            this.lblMetrics2 = new System.Windows.Forms.Label();
            this.lblMTF2 = new System.Windows.Forms.Label();
            this.lblSNR2 = new System.Windows.Forms.Label();
            this.lblDE2 = new System.Windows.Forms.Label();
            this.pbToneCurve2 = new System.Windows.Forms.PictureBox();
            this.txtSourcePath = new System.Windows.Forms.TextBox();
            this.btnBrowseSource = new System.Windows.Forms.Button();
            this.lblSource = new System.Windows.Forms.Label();
            this.nud_P50 = new System.Windows.Forms.NumericUpDown();
            this.lbl_P50 = new System.Windows.Forms.Label();
            this.gbPipeline1.SuspendLayout();
            this.gbPipeline2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay2)).BeginInit();
            this.panelMetrics1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbToneCurve1)).BeginInit();
            this.panelMetrics2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbToneCurve2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nud_P50)).BeginInit();
            this.SuspendLayout();
            // 
            // gbPipeline1
            // 
            this.gbPipeline1.Controls.Add(this.lblOption1);
            this.gbPipeline1.Controls.Add(this.lblStage1);
            this.gbPipeline1.Controls.Add(this.lblBWLevel1);
            this.gbPipeline1.Controls.Add(this.cbBWLevel1);
            this.gbPipeline1.Controls.Add(this.cbCCM1);
            this.gbPipeline1.Controls.Add(this.cbDenoise1);
            this.gbPipeline1.Controls.Add(this.cbAWB1);
            this.gbPipeline1.Controls.Add(this.btnLive1);
            this.gbPipeline1.Controls.Add(this.lblCCM1);
            this.gbPipeline1.Controls.Add(this.btnMeasure1);
            this.gbPipeline1.Controls.Add(this.lblAWB1);
            this.gbPipeline1.Controls.Add(this.lblTone1);
            this.gbPipeline1.Controls.Add(this.cbTone1);
            this.gbPipeline1.Controls.Add(this.cbSharpen1);
            this.gbPipeline1.Controls.Add(this.cbDemosaic1);
            this.gbPipeline1.Controls.Add(this.lblSharpen1);
            this.gbPipeline1.Controls.Add(this.lblBWTime1);
            this.gbPipeline1.Controls.Add(this.lblAWBTime1);
            this.gbPipeline1.Controls.Add(this.lblDenoise1);
            this.gbPipeline1.Controls.Add(this.lblDemosaicTime1);
            this.gbPipeline1.Controls.Add(this.lblCCMTime1);
            this.gbPipeline1.Controls.Add(this.lblToneTime1);
            this.gbPipeline1.Controls.Add(this.lblDemosaic1);
            this.gbPipeline1.Controls.Add(this.lblElapsedTime1);
            this.gbPipeline1.Controls.Add(this.lblSharpenTime1);
            this.gbPipeline1.Controls.Add(this.lblDenoiseTime1);
            this.gbPipeline1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.gbPipeline1.Location = new System.Drawing.Point(12, 54);
            this.gbPipeline1.Name = "gbPipeline1";
            this.gbPipeline1.Size = new System.Drawing.Size(606, 342);
            this.gbPipeline1.TabIndex = 3;
            this.gbPipeline1.TabStop = false;
            this.gbPipeline1.Text = "ISP 1";
            // 
            // lblOption1
            // 
            this.lblOption1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblOption1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblOption1.Location = new System.Drawing.Point(260, 26);
            this.lblOption1.Name = "lblOption1";
            this.lblOption1.Size = new System.Drawing.Size(153, 24);
            this.lblOption1.TabIndex = 207;
            this.lblOption1.Text = "Option";
            this.lblOption1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblStage1
            // 
            this.lblStage1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblStage1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblStage1.Location = new System.Drawing.Point(23, 26);
            this.lblStage1.Name = "lblStage1";
            this.lblStage1.Size = new System.Drawing.Size(203, 24);
            this.lblStage1.TabIndex = 206;
            this.lblStage1.Text = "Stage";
            this.lblStage1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblBWLevel1
            // 
            this.lblBWLevel1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblBWLevel1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblBWLevel1.Location = new System.Drawing.Point(23, 57);
            this.lblBWLevel1.Name = "lblBWLevel1";
            this.lblBWLevel1.Size = new System.Drawing.Size(203, 24);
            this.lblBWLevel1.TabIndex = 0;
            this.lblBWLevel1.Text = "Black & White Level";
            this.lblBWLevel1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbBWLevel1
            // 
            this.cbBWLevel1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBWLevel1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbBWLevel1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbBWLevel1.Location = new System.Drawing.Point(263, 54);
            this.cbBWLevel1.Name = "cbBWLevel1";
            this.cbBWLevel1.Size = new System.Drawing.Size(150, 28);
            this.cbBWLevel1.TabIndex = 1;
            // 
            // cbCCM1
            // 
            this.cbCCM1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbCCM1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbCCM1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbCCM1.Location = new System.Drawing.Point(263, 187);
            this.cbCCM1.Name = "cbCCM1";
            this.cbCCM1.Size = new System.Drawing.Size(150, 28);
            this.cbCCM1.TabIndex = 13;
            // 
            // cbDenoise1
            // 
            this.cbDenoise1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDenoise1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDenoise1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDenoise1.Location = new System.Drawing.Point(263, 120);
            this.cbDenoise1.Name = "cbDenoise1";
            this.cbDenoise1.Size = new System.Drawing.Size(150, 28);
            this.cbDenoise1.TabIndex = 15;
            // 
            // cbAWB1
            // 
            this.cbAWB1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbAWB1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbAWB1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbAWB1.Location = new System.Drawing.Point(263, 87);
            this.cbAWB1.Name = "cbAWB1";
            this.cbAWB1.Size = new System.Drawing.Size(150, 28);
            this.cbAWB1.TabIndex = 11;
            // 
            // btnLive1
            // 
            this.btnLive1.Enabled = false;
            this.btnLive1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.btnLive1.Location = new System.Drawing.Point(263, 294);
            this.btnLive1.Name = "btnLive1";
            this.btnLive1.Size = new System.Drawing.Size(150, 39);
            this.btnLive1.TabIndex = 25;
            this.btnLive1.Text = "Live";
            this.btnLive1.Visible = false;
            this.btnLive1.Click += new System.EventHandler(this.BtnLive1_Click);
            // 
            // lblCCM1
            // 
            this.lblCCM1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblCCM1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblCCM1.Location = new System.Drawing.Point(23, 190);
            this.lblCCM1.Name = "lblCCM1";
            this.lblCCM1.Size = new System.Drawing.Size(203, 24);
            this.lblCCM1.TabIndex = 12;
            this.lblCCM1.Text = "Color Correction Matrix";
            this.lblCCM1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // btnMeasure1
            // 
            this.btnMeasure1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.btnMeasure1.Location = new System.Drawing.Point(27, 294);
            this.btnMeasure1.Name = "btnMeasure1";
            this.btnMeasure1.Size = new System.Drawing.Size(150, 39);
            this.btnMeasure1.TabIndex = 24;
            this.btnMeasure1.Text = "Measure";
            this.btnMeasure1.Click += new System.EventHandler(this.BtnMeasure1_Click);
            // 
            // lblAWB1
            // 
            this.lblAWB1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblAWB1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblAWB1.Location = new System.Drawing.Point(23, 90);
            this.lblAWB1.Name = "lblAWB1";
            this.lblAWB1.Size = new System.Drawing.Size(203, 24);
            this.lblAWB1.TabIndex = 10;
            this.lblAWB1.Text = "Auto White Balance";
            this.lblAWB1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblTone1
            // 
            this.lblTone1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblTone1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblTone1.Location = new System.Drawing.Point(23, 223);
            this.lblTone1.Name = "lblTone1";
            this.lblTone1.Size = new System.Drawing.Size(203, 24);
            this.lblTone1.TabIndex = 16;
            this.lblTone1.Text = "Tone Mapping";
            this.lblTone1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbTone1
            // 
            this.cbTone1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbTone1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbTone1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbTone1.Location = new System.Drawing.Point(263, 220);
            this.cbTone1.Name = "cbTone1";
            this.cbTone1.Size = new System.Drawing.Size(150, 28);
            this.cbTone1.TabIndex = 17;
            // 
            // cbSharpen1
            // 
            this.cbSharpen1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSharpen1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbSharpen1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbSharpen1.Location = new System.Drawing.Point(263, 254);
            this.cbSharpen1.Name = "cbSharpen1";
            this.cbSharpen1.Size = new System.Drawing.Size(150, 28);
            this.cbSharpen1.TabIndex = 21;
            // 
            // cbDemosaic1
            // 
            this.cbDemosaic1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDemosaic1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDemosaic1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDemosaic1.Location = new System.Drawing.Point(263, 154);
            this.cbDemosaic1.Name = "cbDemosaic1";
            this.cbDemosaic1.Size = new System.Drawing.Size(150, 28);
            this.cbDemosaic1.TabIndex = 9;
            // 
            // lblSharpen1
            // 
            this.lblSharpen1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblSharpen1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblSharpen1.Location = new System.Drawing.Point(23, 257);
            this.lblSharpen1.Name = "lblSharpen1";
            this.lblSharpen1.Size = new System.Drawing.Size(203, 24);
            this.lblSharpen1.TabIndex = 20;
            this.lblSharpen1.Text = "Sharpening";
            this.lblSharpen1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblBWTime1
            // 
            this.lblBWTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblBWTime1.Location = new System.Drawing.Point(458, 54);
            this.lblBWTime1.Name = "lblBWTime1";
            this.lblBWTime1.Size = new System.Drawing.Size(70, 24);
            this.lblBWTime1.TabIndex = 200;
            this.lblBWTime1.Text = "-";
            this.lblBWTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblAWBTime1
            // 
            this.lblAWBTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblAWBTime1.Location = new System.Drawing.Point(458, 91);
            this.lblAWBTime1.Name = "lblAWBTime1";
            this.lblAWBTime1.Size = new System.Drawing.Size(70, 24);
            this.lblAWBTime1.TabIndex = 201;
            this.lblAWBTime1.Text = "-";
            this.lblAWBTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDenoise1
            // 
            this.lblDenoise1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblDenoise1.Enabled = false;
            this.lblDenoise1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDenoise1.Location = new System.Drawing.Point(23, 122);
            this.lblDenoise1.Name = "lblDenoise1";
            this.lblDenoise1.Size = new System.Drawing.Size(203, 24);
            this.lblDenoise1.TabIndex = 14;
            this.lblDenoise1.Text = "Denoise";
            this.lblDenoise1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDemosaicTime1
            // 
            this.lblDemosaicTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblDemosaicTime1.Location = new System.Drawing.Point(458, 157);
            this.lblDemosaicTime1.Name = "lblDemosaicTime1";
            this.lblDemosaicTime1.Size = new System.Drawing.Size(70, 24);
            this.lblDemosaicTime1.TabIndex = 202;
            this.lblDemosaicTime1.Text = "-";
            this.lblDemosaicTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblCCMTime1
            // 
            this.lblCCMTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblCCMTime1.Location = new System.Drawing.Point(458, 187);
            this.lblCCMTime1.Name = "lblCCMTime1";
            this.lblCCMTime1.Size = new System.Drawing.Size(70, 24);
            this.lblCCMTime1.TabIndex = 203;
            this.lblCCMTime1.Text = "-";
            this.lblCCMTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblToneTime1
            // 
            this.lblToneTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblToneTime1.Location = new System.Drawing.Point(458, 220);
            this.lblToneTime1.Name = "lblToneTime1";
            this.lblToneTime1.Size = new System.Drawing.Size(70, 24);
            this.lblToneTime1.TabIndex = 204;
            this.lblToneTime1.Text = "-";
            this.lblToneTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDemosaic1
            // 
            this.lblDemosaic1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblDemosaic1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDemosaic1.Location = new System.Drawing.Point(23, 157);
            this.lblDemosaic1.Name = "lblDemosaic1";
            this.lblDemosaic1.Size = new System.Drawing.Size(203, 24);
            this.lblDemosaic1.TabIndex = 8;
            this.lblDemosaic1.Text = "Demosaic";
            this.lblDemosaic1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblElapsedTime1
            // 
            this.lblElapsedTime1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblElapsedTime1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblElapsedTime1.Location = new System.Drawing.Point(457, 26);
            this.lblElapsedTime1.Name = "lblElapsedTime1";
            this.lblElapsedTime1.Size = new System.Drawing.Size(113, 24);
            this.lblElapsedTime1.TabIndex = 208;
            this.lblElapsedTime1.Text = "Elapsed Time";
            this.lblElapsedTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblSharpenTime1
            // 
            this.lblSharpenTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblSharpenTime1.Location = new System.Drawing.Point(458, 254);
            this.lblSharpenTime1.Name = "lblSharpenTime1";
            this.lblSharpenTime1.Size = new System.Drawing.Size(70, 24);
            this.lblSharpenTime1.TabIndex = 205;
            this.lblSharpenTime1.Text = "-";
            this.lblSharpenTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDenoiseTime1
            // 
            this.lblDenoiseTime1.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblDenoiseTime1.Location = new System.Drawing.Point(458, 124);
            this.lblDenoiseTime1.Name = "lblDenoiseTime1";
            this.lblDenoiseTime1.Size = new System.Drawing.Size(70, 24);
            this.lblDenoiseTime1.TabIndex = 206;
            this.lblDenoiseTime1.Text = "-";
            this.lblDenoiseTime1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDemosaic2
            // 
            this.lblDemosaic2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblDemosaic2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDemosaic2.Location = new System.Drawing.Point(23, 156);
            this.lblDemosaic2.Name = "lblDemosaic2";
            this.lblDemosaic2.Size = new System.Drawing.Size(203, 24);
            this.lblDemosaic2.TabIndex = 8;
            this.lblDemosaic2.Text = "Demosaic";
            this.lblDemosaic2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbDenoise2
            // 
            this.cbDenoise2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDenoise2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDenoise2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDenoise2.Location = new System.Drawing.Point(263, 120);
            this.cbDenoise2.Name = "cbDenoise2";
            this.cbDenoise2.Size = new System.Drawing.Size(150, 28);
            this.cbDenoise2.TabIndex = 15;
            // 
            // lblLensShading1
            // 
            this.lblLensShading1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblLensShading1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblLensShading1.Location = new System.Drawing.Point(22, 204);
            this.lblLensShading1.Name = "lblLensShading1";
            this.lblLensShading1.Size = new System.Drawing.Size(203, 24);
            this.lblLensShading1.TabIndex = 2;
            this.lblLensShading1.Text = "Lens Shading";
            this.lblLensShading1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblLensShading1.Visible = false;
            // 
            // cbLensShading1
            // 
            this.cbLensShading1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbLensShading1.Enabled = false;
            this.cbLensShading1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbLensShading1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbLensShading1.Location = new System.Drawing.Point(262, 204);
            this.cbLensShading1.Name = "cbLensShading1";
            this.cbLensShading1.Size = new System.Drawing.Size(200, 28);
            this.cbLensShading1.TabIndex = 3;
            this.cbLensShading1.Visible = false;
            // 
            // lblBadPixel1
            // 
            this.lblBadPixel1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblBadPixel1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblBadPixel1.Location = new System.Drawing.Point(22, 238);
            this.lblBadPixel1.Name = "lblBadPixel1";
            this.lblBadPixel1.Size = new System.Drawing.Size(203, 24);
            this.lblBadPixel1.TabIndex = 4;
            this.lblBadPixel1.Text = "Bad Pixel";
            this.lblBadPixel1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblBadPixel1.Visible = false;
            // 
            // cbBadPixel1
            // 
            this.cbBadPixel1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBadPixel1.Enabled = false;
            this.cbBadPixel1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbBadPixel1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbBadPixel1.Location = new System.Drawing.Point(262, 238);
            this.cbBadPixel1.Name = "cbBadPixel1";
            this.cbBadPixel1.Size = new System.Drawing.Size(200, 28);
            this.cbBadPixel1.TabIndex = 5;
            this.cbBadPixel1.Visible = false;
            // 
            // lblLinearity1
            // 
            this.lblLinearity1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblLinearity1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblLinearity1.Location = new System.Drawing.Point(22, 272);
            this.lblLinearity1.Name = "lblLinearity1";
            this.lblLinearity1.Size = new System.Drawing.Size(203, 24);
            this.lblLinearity1.TabIndex = 6;
            this.lblLinearity1.Text = "Linearity";
            this.lblLinearity1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblLinearity1.Visible = false;
            // 
            // cbLinearity1
            // 
            this.cbLinearity1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbLinearity1.Enabled = false;
            this.cbLinearity1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbLinearity1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbLinearity1.Location = new System.Drawing.Point(262, 272);
            this.cbLinearity1.Name = "cbLinearity1";
            this.cbLinearity1.Size = new System.Drawing.Size(200, 28);
            this.cbLinearity1.TabIndex = 7;
            this.cbLinearity1.Visible = false;
            // 
            // lblDistort1
            // 
            this.lblDistort1.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDistort1.Location = new System.Drawing.Point(22, 306);
            this.lblDistort1.Name = "lblDistort1";
            this.lblDistort1.Size = new System.Drawing.Size(203, 24);
            this.lblDistort1.TabIndex = 18;
            this.lblDistort1.Text = "Distortion Correction";
            this.lblDistort1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblDistort1.Visible = false;
            // 
            // cbDistort1
            // 
            this.cbDistort1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDistort1.Enabled = false;
            this.cbDistort1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDistort1.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDistort1.Location = new System.Drawing.Point(262, 306);
            this.cbDistort1.Name = "cbDistort1";
            this.cbDistort1.Size = new System.Drawing.Size(200, 28);
            this.cbDistort1.TabIndex = 19;
            this.cbDistort1.Visible = false;
            // 
            // lblFPS1
            // 
            this.lblFPS1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblFPS1.Location = new System.Drawing.Point(5, 105);
            this.lblFPS1.Name = "lblFPS1";
            this.lblFPS1.Size = new System.Drawing.Size(200, 20);
            this.lblFPS1.TabIndex = 23;
            this.lblFPS1.Text = "fps: -";
            // 
            // gbPipeline2
            // 
            this.gbPipeline2.Controls.Add(this.lblElapsedTime2);
            this.gbPipeline2.Controls.Add(this.lblOption2);
            this.gbPipeline2.Controls.Add(this.lblStage2);
            this.gbPipeline2.Controls.Add(this.lblBWLevel2);
            this.gbPipeline2.Controls.Add(this.cbBWLevel2);
            this.gbPipeline2.Controls.Add(this.cbCCM2);
            this.gbPipeline2.Controls.Add(this.lblDenoise2);
            this.gbPipeline2.Controls.Add(this.cbAWB2);
            this.gbPipeline2.Controls.Add(this.cbDenoise2);
            this.gbPipeline2.Controls.Add(this.btnMeasure2);
            this.gbPipeline2.Controls.Add(this.lblCCM2);
            this.gbPipeline2.Controls.Add(this.cbSharpen2);
            this.gbPipeline2.Controls.Add(this.lblAWB2);
            this.gbPipeline2.Controls.Add(this.lblDemosaic2);
            this.gbPipeline2.Controls.Add(this.lblTone2);
            this.gbPipeline2.Controls.Add(this.cbTone2);
            this.gbPipeline2.Controls.Add(this.btnLive2);
            this.gbPipeline2.Controls.Add(this.cbDemosaic2);
            this.gbPipeline2.Controls.Add(this.lblSharpen2);
            this.gbPipeline2.Controls.Add(this.lblBWTime2);
            this.gbPipeline2.Controls.Add(this.lblAWBTime2);
            this.gbPipeline2.Controls.Add(this.lblDemosaicTime2);
            this.gbPipeline2.Controls.Add(this.lblCCMTime2);
            this.gbPipeline2.Controls.Add(this.lblToneTime2);
            this.gbPipeline2.Controls.Add(this.lblSharpenTime2);
            this.gbPipeline2.Controls.Add(this.lblDenoiseTime2);
            this.gbPipeline2.Location = new System.Drawing.Point(12, 432);
            this.gbPipeline2.Name = "gbPipeline2";
            this.gbPipeline2.Size = new System.Drawing.Size(606, 342);
            this.gbPipeline2.TabIndex = 4;
            this.gbPipeline2.TabStop = false;
            this.gbPipeline2.Text = "ISP 2";
            // 
            // lblElapsedTime2
            // 
            this.lblElapsedTime2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblElapsedTime2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblElapsedTime2.Location = new System.Drawing.Point(457, 21);
            this.lblElapsedTime2.Name = "lblElapsedTime2";
            this.lblElapsedTime2.Size = new System.Drawing.Size(113, 24);
            this.lblElapsedTime2.TabIndex = 308;
            this.lblElapsedTime2.Text = "Elapsed Time";
            this.lblElapsedTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblOption2
            // 
            this.lblOption2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblOption2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblOption2.Location = new System.Drawing.Point(260, 21);
            this.lblOption2.Name = "lblOption2";
            this.lblOption2.Size = new System.Drawing.Size(153, 24);
            this.lblOption2.TabIndex = 307;
            this.lblOption2.Text = "Option";
            this.lblOption2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblStage2
            // 
            this.lblStage2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblStage2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblStage2.Location = new System.Drawing.Point(23, 21);
            this.lblStage2.Name = "lblStage2";
            this.lblStage2.Size = new System.Drawing.Size(203, 24);
            this.lblStage2.TabIndex = 306;
            this.lblStage2.Text = "Stage";
            this.lblStage2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblBWLevel2
            // 
            this.lblBWLevel2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblBWLevel2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblBWLevel2.Location = new System.Drawing.Point(23, 58);
            this.lblBWLevel2.Name = "lblBWLevel2";
            this.lblBWLevel2.Size = new System.Drawing.Size(203, 24);
            this.lblBWLevel2.TabIndex = 0;
            this.lblBWLevel2.Text = "Black & White Level";
            this.lblBWLevel2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbBWLevel2
            // 
            this.cbBWLevel2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBWLevel2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbBWLevel2.ItemHeight = 20;
            this.cbBWLevel2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbBWLevel2.Location = new System.Drawing.Point(263, 54);
            this.cbBWLevel2.Name = "cbBWLevel2";
            this.cbBWLevel2.Size = new System.Drawing.Size(150, 28);
            this.cbBWLevel2.TabIndex = 1;
            // 
            // cbCCM2
            // 
            this.cbCCM2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbCCM2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbCCM2.ItemHeight = 20;
            this.cbCCM2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbCCM2.Location = new System.Drawing.Point(263, 185);
            this.cbCCM2.Name = "cbCCM2";
            this.cbCCM2.Size = new System.Drawing.Size(150, 28);
            this.cbCCM2.TabIndex = 13;
            // 
            // lblDenoise2
            // 
            this.lblDenoise2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblDenoise2.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDenoise2.Location = new System.Drawing.Point(23, 120);
            this.lblDenoise2.Name = "lblDenoise2";
            this.lblDenoise2.Size = new System.Drawing.Size(203, 24);
            this.lblDenoise2.TabIndex = 14;
            this.lblDenoise2.Text = "Denoise";
            this.lblDenoise2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbAWB2
            // 
            this.cbAWB2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbAWB2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbAWB2.ItemHeight = 20;
            this.cbAWB2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbAWB2.Location = new System.Drawing.Point(263, 87);
            this.cbAWB2.Name = "cbAWB2";
            this.cbAWB2.Size = new System.Drawing.Size(150, 28);
            this.cbAWB2.TabIndex = 11;
            // 
            // btnMeasure2
            // 
            this.btnMeasure2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.btnMeasure2.Location = new System.Drawing.Point(27, 290);
            this.btnMeasure2.Name = "btnMeasure2";
            this.btnMeasure2.Size = new System.Drawing.Size(150, 39);
            this.btnMeasure2.TabIndex = 24;
            this.btnMeasure2.Text = "Measure";
            this.btnMeasure2.Click += new System.EventHandler(this.BtnMeasure2_Click);
            // 
            // lblCCM2
            // 
            this.lblCCM2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblCCM2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblCCM2.Location = new System.Drawing.Point(23, 189);
            this.lblCCM2.Name = "lblCCM2";
            this.lblCCM2.Size = new System.Drawing.Size(203, 24);
            this.lblCCM2.TabIndex = 12;
            this.lblCCM2.Text = "Color Correction Matrix";
            this.lblCCM2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbSharpen2
            // 
            this.cbSharpen2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbSharpen2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbSharpen2.ItemHeight = 20;
            this.cbSharpen2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbSharpen2.Location = new System.Drawing.Point(263, 251);
            this.cbSharpen2.Name = "cbSharpen2";
            this.cbSharpen2.Size = new System.Drawing.Size(150, 28);
            this.cbSharpen2.TabIndex = 21;
            // 
            // lblAWB2
            // 
            this.lblAWB2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblAWB2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblAWB2.Location = new System.Drawing.Point(23, 91);
            this.lblAWB2.Name = "lblAWB2";
            this.lblAWB2.Size = new System.Drawing.Size(203, 24);
            this.lblAWB2.TabIndex = 10;
            this.lblAWB2.Text = "Auto White Balance";
            this.lblAWB2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblTone2
            // 
            this.lblTone2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblTone2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblTone2.Location = new System.Drawing.Point(23, 222);
            this.lblTone2.Name = "lblTone2";
            this.lblTone2.Size = new System.Drawing.Size(203, 24);
            this.lblTone2.TabIndex = 16;
            this.lblTone2.Text = "Tone Mapping";
            this.lblTone2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // cbTone2
            // 
            this.cbTone2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbTone2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbTone2.ItemHeight = 20;
            this.cbTone2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbTone2.Location = new System.Drawing.Point(263, 218);
            this.cbTone2.Name = "cbTone2";
            this.cbTone2.Size = new System.Drawing.Size(150, 28);
            this.cbTone2.TabIndex = 17;
            // 
            // btnLive2
            // 
            this.btnLive2.Enabled = false;
            this.btnLive2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.btnLive2.Location = new System.Drawing.Point(263, 290);
            this.btnLive2.Name = "btnLive2";
            this.btnLive2.Size = new System.Drawing.Size(150, 39);
            this.btnLive2.TabIndex = 25;
            this.btnLive2.Text = "Live";
            this.btnLive2.Visible = false;
            this.btnLive2.Click += new System.EventHandler(this.BtnLive2_Click);
            // 
            // cbDemosaic2
            // 
            this.cbDemosaic2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDemosaic2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDemosaic2.ItemHeight = 20;
            this.cbDemosaic2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDemosaic2.Location = new System.Drawing.Point(263, 152);
            this.cbDemosaic2.Name = "cbDemosaic2";
            this.cbDemosaic2.Size = new System.Drawing.Size(150, 28);
            this.cbDemosaic2.TabIndex = 9;
            // 
            // lblSharpen2
            // 
            this.lblSharpen2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblSharpen2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblSharpen2.Location = new System.Drawing.Point(23, 255);
            this.lblSharpen2.Name = "lblSharpen2";
            this.lblSharpen2.Size = new System.Drawing.Size(203, 24);
            this.lblSharpen2.TabIndex = 20;
            this.lblSharpen2.Text = "Sharpening";
            this.lblSharpen2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblBWTime2
            // 
            this.lblBWTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblBWTime2.Location = new System.Drawing.Point(458, 57);
            this.lblBWTime2.Name = "lblBWTime2";
            this.lblBWTime2.Size = new System.Drawing.Size(70, 24);
            this.lblBWTime2.TabIndex = 300;
            this.lblBWTime2.Text = "-";
            this.lblBWTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblAWBTime2
            // 
            this.lblAWBTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblAWBTime2.Location = new System.Drawing.Point(458, 90);
            this.lblAWBTime2.Name = "lblAWBTime2";
            this.lblAWBTime2.Size = new System.Drawing.Size(70, 24);
            this.lblAWBTime2.TabIndex = 301;
            this.lblAWBTime2.Text = "-";
            this.lblAWBTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDemosaicTime2
            // 
            this.lblDemosaicTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblDemosaicTime2.Location = new System.Drawing.Point(458, 155);
            this.lblDemosaicTime2.Name = "lblDemosaicTime2";
            this.lblDemosaicTime2.Size = new System.Drawing.Size(70, 24);
            this.lblDemosaicTime2.TabIndex = 302;
            this.lblDemosaicTime2.Text = "-";
            this.lblDemosaicTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblCCMTime2
            // 
            this.lblCCMTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblCCMTime2.Location = new System.Drawing.Point(458, 188);
            this.lblCCMTime2.Name = "lblCCMTime2";
            this.lblCCMTime2.Size = new System.Drawing.Size(70, 24);
            this.lblCCMTime2.TabIndex = 303;
            this.lblCCMTime2.Text = "-";
            this.lblCCMTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblToneTime2
            // 
            this.lblToneTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblToneTime2.Location = new System.Drawing.Point(458, 221);
            this.lblToneTime2.Name = "lblToneTime2";
            this.lblToneTime2.Size = new System.Drawing.Size(70, 24);
            this.lblToneTime2.TabIndex = 304;
            this.lblToneTime2.Text = "-";
            this.lblToneTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblSharpenTime2
            // 
            this.lblSharpenTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblSharpenTime2.Location = new System.Drawing.Point(458, 254);
            this.lblSharpenTime2.Name = "lblSharpenTime2";
            this.lblSharpenTime2.Size = new System.Drawing.Size(70, 24);
            this.lblSharpenTime2.TabIndex = 305;
            this.lblSharpenTime2.Text = "-";
            this.lblSharpenTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblDenoiseTime2
            // 
            this.lblDenoiseTime2.Font = new System.Drawing.Font("微軟正黑體", 9F);
            this.lblDenoiseTime2.Location = new System.Drawing.Point(458, 119);
            this.lblDenoiseTime2.Name = "lblDenoiseTime2";
            this.lblDenoiseTime2.Size = new System.Drawing.Size(70, 24);
            this.lblDenoiseTime2.TabIndex = 306;
            this.lblDenoiseTime2.Text = "-";
            this.lblDenoiseTime2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lblLensShading2
            // 
            this.lblLensShading2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblLensShading2.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblLensShading2.Location = new System.Drawing.Point(22, 231);
            this.lblLensShading2.Name = "lblLensShading2";
            this.lblLensShading2.Size = new System.Drawing.Size(203, 24);
            this.lblLensShading2.TabIndex = 2;
            this.lblLensShading2.Text = "Lens Shading";
            this.lblLensShading2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblLensShading2.Visible = false;
            // 
            // cbLensShading2
            // 
            this.cbLensShading2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbLensShading2.Enabled = false;
            this.cbLensShading2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbLensShading2.ItemHeight = 20;
            this.cbLensShading2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbLensShading2.Location = new System.Drawing.Point(262, 231);
            this.cbLensShading2.Name = "cbLensShading2";
            this.cbLensShading2.Size = new System.Drawing.Size(200, 28);
            this.cbLensShading2.TabIndex = 3;
            this.cbLensShading2.Visible = false;
            // 
            // lblBadPixel2
            // 
            this.lblBadPixel2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblBadPixel2.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblBadPixel2.Location = new System.Drawing.Point(22, 265);
            this.lblBadPixel2.Name = "lblBadPixel2";
            this.lblBadPixel2.Size = new System.Drawing.Size(203, 24);
            this.lblBadPixel2.TabIndex = 4;
            this.lblBadPixel2.Text = "Bad Pixel";
            this.lblBadPixel2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblBadPixel2.Visible = false;
            // 
            // cbBadPixel2
            // 
            this.cbBadPixel2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbBadPixel2.Enabled = false;
            this.cbBadPixel2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbBadPixel2.ItemHeight = 20;
            this.cbBadPixel2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbBadPixel2.Location = new System.Drawing.Point(262, 265);
            this.cbBadPixel2.Name = "cbBadPixel2";
            this.cbBadPixel2.Size = new System.Drawing.Size(200, 28);
            this.cbBadPixel2.TabIndex = 5;
            this.cbBadPixel2.Visible = false;
            // 
            // lblLinearity2
            // 
            this.lblLinearity2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblLinearity2.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblLinearity2.Location = new System.Drawing.Point(22, 299);
            this.lblLinearity2.Name = "lblLinearity2";
            this.lblLinearity2.Size = new System.Drawing.Size(203, 24);
            this.lblLinearity2.TabIndex = 6;
            this.lblLinearity2.Text = "Linearity";
            this.lblLinearity2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblLinearity2.Visible = false;
            // 
            // cbLinearity2
            // 
            this.cbLinearity2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbLinearity2.Enabled = false;
            this.cbLinearity2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbLinearity2.ItemHeight = 20;
            this.cbLinearity2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbLinearity2.Location = new System.Drawing.Point(262, 299);
            this.cbLinearity2.Name = "cbLinearity2";
            this.cbLinearity2.Size = new System.Drawing.Size(200, 28);
            this.cbLinearity2.TabIndex = 7;
            this.cbLinearity2.Visible = false;
            // 
            // lblDistort2
            // 
            this.lblDistort2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lblDistort2.Font = new System.Drawing.Font("微軟正黑體", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDistort2.Location = new System.Drawing.Point(22, 333);
            this.lblDistort2.Name = "lblDistort2";
            this.lblDistort2.Size = new System.Drawing.Size(203, 24);
            this.lblDistort2.TabIndex = 18;
            this.lblDistort2.Text = "Distortion Correction";
            this.lblDistort2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblDistort2.Visible = false;
            // 
            // cbDistort2
            // 
            this.cbDistort2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbDistort2.Enabled = false;
            this.cbDistort2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.cbDistort2.ItemHeight = 20;
            this.cbDistort2.Items.AddRange(new object[] {
            "None",
            "Default"});
            this.cbDistort2.Location = new System.Drawing.Point(262, 333);
            this.cbDistort2.Name = "cbDistort2";
            this.cbDistort2.Size = new System.Drawing.Size(200, 28);
            this.cbDistort2.TabIndex = 19;
            this.cbDistort2.Visible = false;
            // 
            // lblFPS2
            // 
            this.lblFPS2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblFPS2.Location = new System.Drawing.Point(5, 105);
            this.lblFPS2.Name = "lblFPS2";
            this.lblFPS2.Size = new System.Drawing.Size(200, 20);
            this.lblFPS2.TabIndex = 23;
            this.lblFPS2.Text = "fps: -";
            // 
            // pbDisplay1
            // 
            this.pbDisplay1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pbDisplay1.Location = new System.Drawing.Point(624, 66);
            this.pbDisplay1.Name = "pbDisplay1";
            this.pbDisplay1.Size = new System.Drawing.Size(535, 330);
            this.pbDisplay1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pbDisplay1.TabIndex = 6;
            this.pbDisplay1.TabStop = false;
            // 
            // pbDisplay2
            // 
            this.pbDisplay2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pbDisplay2.Location = new System.Drawing.Point(626, 442);
            this.pbDisplay2.Name = "pbDisplay2";
            this.pbDisplay2.Size = new System.Drawing.Size(535, 332);
            this.pbDisplay2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pbDisplay2.TabIndex = 7;
            this.pbDisplay2.TabStop = false;
            // 
            // panelMetrics1
            // 
            this.panelMetrics1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelMetrics1.Controls.Add(this.lblMetrics1);
            this.panelMetrics1.Controls.Add(this.lblMTF1);
            this.panelMetrics1.Controls.Add(this.lblSNR1);
            this.panelMetrics1.Controls.Add(this.lblDE1);
            this.panelMetrics1.Controls.Add(this.lblLensShading1);
            this.panelMetrics1.Controls.Add(this.lblFPS1);
            this.panelMetrics1.Controls.Add(this.cbLensShading1);
            this.panelMetrics1.Controls.Add(this.pbToneCurve1);
            this.panelMetrics1.Controls.Add(this.lblLinearity1);
            this.panelMetrics1.Controls.Add(this.cbBadPixel1);
            this.panelMetrics1.Controls.Add(this.lblBadPixel1);
            this.panelMetrics1.Controls.Add(this.cbDistort1);
            this.panelMetrics1.Controls.Add(this.cbLinearity1);
            this.panelMetrics1.Controls.Add(this.lblDistort1);
            this.panelMetrics1.Location = new System.Drawing.Point(1191, 72);
            this.panelMetrics1.Name = "panelMetrics1";
            this.panelMetrics1.Size = new System.Drawing.Size(520, 371);
            this.panelMetrics1.TabIndex = 8;
            this.panelMetrics1.Visible = false;
            // 
            // lblMetrics1
            // 
            this.lblMetrics1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblMetrics1.Location = new System.Drawing.Point(5, 5);
            this.lblMetrics1.Name = "lblMetrics1";
            this.lblMetrics1.Size = new System.Drawing.Size(200, 20);
            this.lblMetrics1.TabIndex = 0;
            this.lblMetrics1.Text = "ISP1 Metrics";
            // 
            // lblMTF1
            // 
            this.lblMTF1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblMTF1.Location = new System.Drawing.Point(5, 30);
            this.lblMTF1.Name = "lblMTF1";
            this.lblMTF1.Size = new System.Drawing.Size(200, 20);
            this.lblMTF1.TabIndex = 1;
            this.lblMTF1.Text = "MTF: -";
            // 
            // lblSNR1
            // 
            this.lblSNR1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblSNR1.Location = new System.Drawing.Point(5, 55);
            this.lblSNR1.Name = "lblSNR1";
            this.lblSNR1.Size = new System.Drawing.Size(200, 20);
            this.lblSNR1.TabIndex = 2;
            this.lblSNR1.Text = "SNR: -";
            // 
            // lblDE1
            // 
            this.lblDE1.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDE1.Location = new System.Drawing.Point(5, 80);
            this.lblDE1.Name = "lblDE1";
            this.lblDE1.Size = new System.Drawing.Size(220, 20);
            this.lblDE1.TabIndex = 3;
            this.lblDE1.Text = "ΔE color accuracy: -";
            // 
            // pbToneCurve1
            // 
            this.pbToneCurve1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pbToneCurve1.Location = new System.Drawing.Point(246, 7);
            this.pbToneCurve1.Name = "pbToneCurve1";
            this.pbToneCurve1.Size = new System.Drawing.Size(264, 181);
            this.pbToneCurve1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pbToneCurve1.TabIndex = 24;
            this.pbToneCurve1.TabStop = false;
            // 
            // panelMetrics2
            // 
            this.panelMetrics2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelMetrics2.Controls.Add(this.lblMetrics2);
            this.panelMetrics2.Controls.Add(this.lblMTF2);
            this.panelMetrics2.Controls.Add(this.lblLensShading2);
            this.panelMetrics2.Controls.Add(this.cbLensShading2);
            this.panelMetrics2.Controls.Add(this.lblSNR2);
            this.panelMetrics2.Controls.Add(this.lblBadPixel2);
            this.panelMetrics2.Controls.Add(this.lblDE2);
            this.panelMetrics2.Controls.Add(this.cbBadPixel2);
            this.panelMetrics2.Controls.Add(this.lblFPS2);
            this.panelMetrics2.Controls.Add(this.pbToneCurve2);
            this.panelMetrics2.Controls.Add(this.lblLinearity2);
            this.panelMetrics2.Controls.Add(this.cbDistort2);
            this.panelMetrics2.Controls.Add(this.cbLinearity2);
            this.panelMetrics2.Controls.Add(this.lblDistort2);
            this.panelMetrics2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.panelMetrics2.Location = new System.Drawing.Point(1191, 520);
            this.panelMetrics2.Name = "panelMetrics2";
            this.panelMetrics2.Size = new System.Drawing.Size(520, 409);
            this.panelMetrics2.TabIndex = 9;
            this.panelMetrics2.Visible = false;
            // 
            // lblMetrics2
            // 
            this.lblMetrics2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblMetrics2.Location = new System.Drawing.Point(5, 5);
            this.lblMetrics2.Name = "lblMetrics2";
            this.lblMetrics2.Size = new System.Drawing.Size(200, 20);
            this.lblMetrics2.TabIndex = 0;
            this.lblMetrics2.Text = "ISP2 Metrics";
            // 
            // lblMTF2
            // 
            this.lblMTF2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblMTF2.Location = new System.Drawing.Point(5, 30);
            this.lblMTF2.Name = "lblMTF2";
            this.lblMTF2.Size = new System.Drawing.Size(200, 20);
            this.lblMTF2.TabIndex = 1;
            this.lblMTF2.Text = "MTF: -";
            // 
            // lblSNR2
            // 
            this.lblSNR2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblSNR2.Location = new System.Drawing.Point(5, 55);
            this.lblSNR2.Name = "lblSNR2";
            this.lblSNR2.Size = new System.Drawing.Size(200, 20);
            this.lblSNR2.TabIndex = 2;
            this.lblSNR2.Text = "SNR: -";
            // 
            // lblDE2
            // 
            this.lblDE2.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblDE2.Location = new System.Drawing.Point(5, 80);
            this.lblDE2.Name = "lblDE2";
            this.lblDE2.Size = new System.Drawing.Size(220, 20);
            this.lblDE2.TabIndex = 3;
            this.lblDE2.Text = "ΔE color accuracy: -";
            // 
            // pbToneCurve2
            // 
            this.pbToneCurve2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pbToneCurve2.Location = new System.Drawing.Point(246, 9);
            this.pbToneCurve2.Name = "pbToneCurve2";
            this.pbToneCurve2.Size = new System.Drawing.Size(264, 181);
            this.pbToneCurve2.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pbToneCurve2.TabIndex = 24;
            this.pbToneCurve2.TabStop = false;
            // 
            // txtSourcePath
            // 
            this.txtSourcePath.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.txtSourcePath.Location = new System.Drawing.Point(142, 13);
            this.txtSourcePath.Margin = new System.Windows.Forms.Padding(0);
            this.txtSourcePath.Name = "txtSourcePath";
            this.txtSourcePath.ReadOnly = true;
            this.txtSourcePath.Size = new System.Drawing.Size(760, 29);
            this.txtSourcePath.TabIndex = 1;
            // 
            // btnBrowseSource
            // 
            this.btnBrowseSource.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.btnBrowseSource.Location = new System.Drawing.Point(905, 13);
            this.btnBrowseSource.Name = "btnBrowseSource";
            this.btnBrowseSource.Size = new System.Drawing.Size(90, 30);
            this.btnBrowseSource.TabIndex = 2;
            this.btnBrowseSource.Text = "Browse";
            this.btnBrowseSource.Click += new System.EventHandler(this.BtnBrowseSource_Click);
            // 
            // lblSource
            // 
            this.lblSource.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lblSource.Location = new System.Drawing.Point(12, 18);
            this.lblSource.Name = "lblSource";
            this.lblSource.Size = new System.Drawing.Size(129, 24);
            this.lblSource.TabIndex = 0;
            this.lblSource.Text = "Source Image :";
            // 
            // nud_P50
            // 
            this.nud_P50.DecimalPlaces = 2;
            this.nud_P50.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.nud_P50.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.nud_P50.Location = new System.Drawing.Point(244, 785);
            this.nud_P50.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nud_P50.Name = "nud_P50";
            this.nud_P50.Size = new System.Drawing.Size(68, 29);
            this.nud_P50.TabIndex = 10;
            this.nud_P50.Value = new decimal(new int[] {
            18,
            0,
            0,
            131072});
            // 
            // lbl_P50
            // 
            this.lbl_P50.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.lbl_P50.BackColor = System.Drawing.Color.Transparent;
            this.lbl_P50.Font = new System.Drawing.Font("微軟正黑體", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(136)));
            this.lbl_P50.Location = new System.Drawing.Point(12, 807);
            this.lbl_P50.Name = "lbl_P50";
            this.lbl_P50.Size = new System.Drawing.Size(226, 24);
            this.lbl_P50.TabIndex = 21;
            this.lbl_P50.Text = "P50 Brightness Target (0~1) :";
            this.lbl_P50.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // DemoForm
            // 
            this.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.ClientSize = new System.Drawing.Size(1178, 865);
            this.Controls.Add(this.lbl_P50);
            this.Controls.Add(this.nud_P50);
            this.Controls.Add(this.lblSource);
            this.Controls.Add(this.txtSourcePath);
            this.Controls.Add(this.btnBrowseSource);
            this.Controls.Add(this.gbPipeline1);
            this.Controls.Add(this.gbPipeline2);
            this.Controls.Add(this.pbDisplay1);
            this.Controls.Add(this.pbDisplay2);
            this.Controls.Add(this.panelMetrics1);
            this.Controls.Add(this.panelMetrics2);
            this.Name = "DemoForm";
            this.Text = "ISP Comparison";
            this.gbPipeline1.ResumeLayout(false);
            this.gbPipeline2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pbDisplay2)).EndInit();
            this.panelMetrics1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbToneCurve1)).EndInit();
            this.panelMetrics2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbToneCurve2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nud_P50)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private NumericUpDown nud_P50;
        private Label lbl_P50;
        private Label lblElapsedTime1;
        private Label lblOption1;
        private Label lblStage1;
        private Label lblElapsedTime2;
        private Label lblOption2;
        private Label lblStage2;
    }
}