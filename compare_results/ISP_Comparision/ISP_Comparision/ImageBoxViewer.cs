using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

namespace ISP_Comparision
{
    public class ImageBoxViewer : IDisposable
    {
        private readonly PictureBox pictureBox;
        private Bitmap image;
        private float zoom = 1.0f;
        private float minZoom = 0.1f;
        private float maxZoom = 16f;
        private float startZoom;
        private Point pan = Point.Empty;
        private Point mouseDownPt;
        private Point panStart;
        private bool dragging = false;

        private ImageBoxViewer(PictureBox pb)
        {
            pictureBox = pb ?? throw new ArgumentNullException(nameof(pb));
            pictureBox.Paint += PictureBox_Paint;
            pictureBox.MouseWheel += PictureBox_MouseWheel;
            pictureBox.MouseDown += PictureBox_MouseDown;
            pictureBox.MouseMove += PictureBox_MouseMove;
            pictureBox.MouseDoubleClick += PictureBox_MouseDoubleClick;
            pictureBox.MouseUp += PictureBox_MouseUp;
            pictureBox.MouseEnter += (s, e) => pictureBox.Focus();
            pictureBox.Resize += (s, e) => pictureBox.Invalidate();
            // Ensure control can receive focus
            pictureBox.TabStop = true;
        }

        public static ImageBoxViewer Attach(PictureBox pb)
        {
            if (pb == null) throw new ArgumentNullException(nameof(pb));
            var v = new ImageBoxViewer(pb);
            // store reference in Tag to avoid multiple attachments
            pb.Tag = v;
            return v;
        }

        public static ImageBoxViewer FromPictureBox(PictureBox pb)
        {
            if (pb?.Tag is ImageBoxViewer v) return v;
            return Attach(pb);
        }

        public void SetImage(Bitmap bmp)
        {
            var old = image;
            image = bmp != null ? (Bitmap)bmp.Clone() : null;
            // reset zoom and pan for new image
            zoom = 1.0f;
            pan = Point.Empty;
            pictureBox.Invalidate();
            old?.Dispose();
        }

        public void ClearImage()
        {
            var old = image;
            image = null;
            zoom = 1.0f;
            pan = Point.Empty;
            pictureBox.Invalidate();
            old?.Dispose();
        }

        public void ResetZoom()
        {
            zoom = 1.0f;
            pan = Point.Empty;
            pictureBox.Invalidate();
        }

        public void ZoomTo(float factor)
        {
            if (factor <= 0) return;
            zoom = Math.Max(minZoom, Math.Min(maxZoom, factor));
            pictureBox.Invalidate();
        }

        // zoom about a given control point (client coords)
        private void DoZoom(float factor, Point clientPt)
        {
            if (image == null) return;
            var oldZoom = zoom;
            var newZoom = Math.Max(minZoom, Math.Min(maxZoom, zoom * factor));
            if (Math.Abs(newZoom - oldZoom) < 1e-6) return;

            // compute image coordinates of clientPt before zoom
            var fit = GetFitScale();
            var oldScale = fit * oldZoom;
            var newScale = fit * newZoom;

            // image coordinate (in pixels)
            float imgX = (clientPt.X - (pictureBox.Width - image.Width * oldScale) / 2f - pan.X) / oldScale;
            float imgY = (clientPt.Y - (pictureBox.Height - image.Height * oldScale) / 2f - pan.Y) / oldScale;

            // update zoom
            zoom = newZoom;

            // compute new pan so that that image point stays under cursor
            float newPanX = clientPt.X - (pictureBox.Width - image.Width * newScale) / 2f - imgX * newScale;
            float newPanY = clientPt.Y - (pictureBox.Height - image.Height * newScale) / 2f - imgY * newScale;

            pan = new Point((int)Math.Round(newPanX), (int)Math.Round(newPanY));
            pictureBox.Invalidate();
        }

        private float GetFitScale()
        {
            if (image == null || pictureBox.Width <= 0 || pictureBox.Height <= 0) return 1f;
            float sx = (float)pictureBox.Width / image.Width;
            float sy = (float)pictureBox.Height / image.Height;
            return Math.Min(sx, sy);
        }

        private void PictureBox_MouseWheel(object sender, MouseEventArgs e)
        {
            // use 1.15 ^ ticks
            float factor = (e.Delta > 0) ? 1.15f : (1.0f / 1.15f);
            DoZoom(factor, e.Location);
        }
        private void PictureBox_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            zoom = 1.0f;
            pan = Point.Empty;

            pictureBox.Invalidate();   // ­«·sÄ²µo Paint
        }

        private void PictureBox_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left && image != null)
            {
                dragging = true;
                mouseDownPt = e.Location;
                panStart = pan;
                pictureBox.Capture = true;
            }
        }

        private void PictureBox_MouseMove(object sender, MouseEventArgs e)
        {
            if (dragging)
            {
                pan = new Point(panStart.X + (e.Location.X - mouseDownPt.X), panStart.Y + (e.Location.Y - mouseDownPt.Y));
                pictureBox.Invalidate();
            }
        }

        private void PictureBox_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                dragging = false;
                pictureBox.Capture = false;
            }
        }

        private void PictureBox_Paint(object sender, PaintEventArgs e)
        {
            var g = e.Graphics;
            // fill background black
            g.Clear(Color.Black);

            if (image == null) return;

            g.InterpolationMode = InterpolationMode.HighQualityBicubic;
            g.PixelOffsetMode = PixelOffsetMode.HighQuality;
            g.SmoothingMode = SmoothingMode.None;

            float fit = GetFitScale();
            float scale = fit * zoom;
            float drawW = image.Width * scale;
            float drawH = image.Height * scale;

            float x = (pictureBox.Width - drawW) / 2f + pan.X;
            float y = (pictureBox.Height - drawH) / 2f + pan.Y;

            var dest = new RectangleF(x, y, drawW, drawH);
            g.DrawImage(image, dest);
        }

        public void Dispose()
        {
            try
            {
                pictureBox.Paint -= PictureBox_Paint;
                pictureBox.MouseWheel -= PictureBox_MouseWheel;
                pictureBox.MouseDown -= PictureBox_MouseDown;
                pictureBox.MouseMove -= PictureBox_MouseMove;
                pictureBox.MouseUp -= PictureBox_MouseUp;
                pictureBox.Resize -= (s, e) => pictureBox.Invalidate();
                pictureBox.Tag = null;
            }
            catch { }

            image?.Dispose();
            image = null;
        }
    }
}