import rawpy
import sys
import os
import cv2
import matplotlib
matplotlib.use('TkAgg')  # 或 'Agg' (非互動), 'WXAgg'，等等
import matplotlib.pyplot as plt

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from common.common_func import show_image, show_two_images, show_three_images, show_four_images


class ImageProcessor:
    def default_ISP(raw): 
        rgb_postprocess = raw.postprocess(
            use_camera_wb=True,      # 使用相機內建白平衡
            no_auto_bright=False,     # 不自動調整亮度
            output_bps=8             # 輸出8位元影像
        )
        return rgb_postprocess



raw_short_path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/short/00001_00_0.1s.RAF"
with rawpy.imread(raw_short_path) as raw:
    default_ISP_img = ImageProcessor.default_ISP(raw)
    small_bayer = cv2.resize(raw.raw_image_visible, (640, 480))
    

raw_long_path = "C:/Users/eevo1/OneDrive/Desktop/ISP_AI_Comparison/data/raw/Fuji/Fuji/long/00001_00_10s.RAF"
with rawpy.imread(raw_long_path) as raw:
    rgb_long = raw.postprocess(
        use_camera_wb=True,      # 使用相機內建白平衡
        no_auto_bright=True,     # 不自動調整亮度
        output_bps=8             # 輸出8位元影像
    )


show_three_images(
    small_bayer, 'Bayer Raw Image',     
    default_ISP_img, 'RawPy Postprocessed RGB Image',
    rgb_long, 'Long Exposure RGB Image',
    cmap1=None, cmap2=None, cmap3=None
)


