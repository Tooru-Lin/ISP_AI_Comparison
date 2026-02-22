using ISP_CSharp;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ISP_Comparision
{
    public static class Analysis
    {
        // 將 native ISP_Mat 轉成 System.Drawing.Bitmap (24bpp RGB)
        // 假設 ISP_Mat.data 在呼叫時仍有效，呼叫端在轉換後可釋放 ISP_Mat。
        // 支援格式：
        //  - CV_8U channels==3 -> 直接複製 (BGR)
        //  - CV_8U channels==1 -> 灰階複製成 BGR
        //  - CV_32F channels==3 -> float(0..1) -> byte(0..255) per channel (BGR)
        //  - CV_32F channels==1 -> float 灰階複製成 BGR
        // 若你的 float 範圍不是 [0,1]，請先在 native 或 C# 端正規化。
        public static Bitmap ToBitmap(in ISP_Mat mat)
        {
            if (mat.data == IntPtr.Zero) throw new ArgumentNullException(nameof(mat.data));
            if (mat.rows <= 0 || mat.cols <= 0) throw new ArgumentException("Invalid dimensions");
            if (mat.step <= 0) throw new ArgumentException("Invalid step");

            int width = mat.cols;
            int height = mat.rows;
            int channels = mat.channels;
            int step = mat.step;

            // 取得 depth: OpenCV macros: depth = type & 7
            int depth = mat.type & 0x7; // 0=CV_8U, 5=CV_32F, etc.

            // 簡單支援判斷
            bool is8u = (depth == 0);
            bool is32f = (depth == 5);

            if (!(is8u || is32f))
                throw new NotSupportedException("Only CV_8U or CV_32F supported");

            // 只支援 1 或 3 通道
            if (!(channels == 1 || channels == 3))
                throw new NotSupportedException("Only 1 or 3 channels supported");

            Bitmap bmp = new Bitmap(width, height, PixelFormat.Format24bppRgb);
            BitmapData bd = bmp.LockBits(new Rectangle(0, 0, width, height),
                                         ImageLockMode.WriteOnly, bmp.PixelFormat);
            try
            {
                int dstStride = Math.Abs(bd.Stride);
                int dstRowBytes = width * 3; // 24bpp
                byte[] tmpRow = new byte[dstRowBytes]; // 每列有效像素資料
                                                       // 依類型分流處理，使用可重複利用的緩衝區降低 GC
                if (is8u)
                {
                    int srcRowBytes = width * channels;
                    byte[] srcRow = new byte[srcRowBytes];
                    for (int y = 0; y < height; y++)
                    {
                        IntPtr srcPtr = IntPtr.Add(mat.data, y * step);
                        Marshal.Copy(srcPtr, srcRow, 0, srcRowBytes);

                        // 如果 src 為 3ch，直接複製 BGR
                        if (channels == 3)
                        {
                            // bmp expects BGR for Format24bppRgb, so direct copy is fine
                            IntPtr dstPtr = IntPtr.Add(bd.Scan0, y * dstStride);
                            Marshal.Copy(srcRow, 0, dstPtr, dstRowBytes);
                        }
                        else // channels == 1, 灰階複製到三通道
                        {
                            for (int x = 0; x < width; x++)
                            {
                                byte v = srcRow[x];
                                int idx = x * 3;
                                tmpRow[idx + 0] = v; // B
                                tmpRow[idx + 1] = v; // G
                                tmpRow[idx + 2] = v; // R
                            }
                            IntPtr dstPtr = IntPtr.Add(bd.Scan0, y * dstStride);
                            Marshal.Copy(tmpRow, 0, dstPtr, dstRowBytes);
                        }
                    }
                }
                else // is32f
                {
                    int srcCount = width * channels;
                    float[] srcFloats = new float[srcCount];
                    for (int y = 0; y < height; y++)
                    {
                        IntPtr srcPtr = IntPtr.Add(mat.data, y * step);
                        Marshal.Copy(srcPtr, srcFloats, 0, srcCount);

                        if (channels == 3)
                        {
                            for (int x = 0; x < width; x++)
                            {
                                int sIdx = x * 3;
                                // 假設 float 範圍為 [0,1]，否則會被 clamp
                                float fb = srcFloats[sIdx + 0];
                                float fg = srcFloats[sIdx + 1];
                                float fr = srcFloats[sIdx + 2];

                                byte b = FloatToByteClamped(fb);
                                byte g = FloatToByteClamped(fg);
                                byte r = FloatToByteClamped(fr);

                                int dIdx = x * 3;
                                tmpRow[dIdx + 0] = b;
                                tmpRow[dIdx + 1] = g;
                                tmpRow[dIdx + 2] = r;
                            }
                        }
                        else // channels == 1
                        {
                            for (int x = 0; x < width; x++)
                            {
                                float fv = srcFloats[x];
                                byte v = FloatToByteClamped(fv);
                                int dIdx = x * 3;
                                tmpRow[dIdx + 0] = v;
                                tmpRow[dIdx + 1] = v;
                                tmpRow[dIdx + 2] = v;
                            }
                        }

                        IntPtr dstPtr = IntPtr.Add(bd.Scan0, y * dstStride);
                        Marshal.Copy(tmpRow, 0, dstPtr, dstRowBytes);
                    }
                }

                return bmp;
            }
            finally
            {
                bmp.UnlockBits(bd);
            }
        }

        private static byte FloatToByteClamped(float v)
        {
            if (float.IsNaN(v)) v = 0f;
            // clamp to [0,1]
            if (v < 0f) v = 0f;
            else if (v > 1f) v = 1f;
            return (byte)(v * 255f + 0.5f);
        }
    }
}
