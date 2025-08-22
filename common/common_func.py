import matplotlib
matplotlib.use('TkAgg')  # 或 'Agg' (非互動), 'WXAgg'，等等
import matplotlib.pyplot as plt




def show_image(img, title='', cmap=None):
    plt.imshow(img, cmap=cmap if cmap else 'gray')
    plt.title(title)
    plt.axis('off')  # 不顯示座標軸
    plt.show(block=True)  # 強制阻塞直到視窗關閉


def show_two_images(img1, title1, img2, title2, cmap1=None, cmap2=None):
    plt.figure(figsize=(6,4))

    plt.subplot(1, 2, 1)
    plt.imshow(img1, cmap=cmap1)
    plt.title(title1)
    plt.axis('off')  # 不顯示座標軸
    
    plt.subplot(1, 2, 2)
    plt.imshow(img2, cmap=cmap2)
    plt.title(title2)
    plt.axis('off')  # 不顯示座標軸
    
    plt.show(block=True)  # 強制阻塞直到視窗關閉


def show_three_images(img1, title1, img2, title2, img3, title3, cmap1=None, cmap2=None, cmap3=None):
    plt.figure(figsize=(12,4))

    plt.subplot(1, 3, 1)
    plt.imshow(img1, cmap=cmap1)
    plt.title(title1)
    plt.axis('off')  # 不顯示座標軸

    plt.subplot(1, 3, 2)
    plt.imshow(img2, cmap=cmap2)
    plt.title(title2)
    plt.axis('off')  # 不顯示座標軸

    plt.subplot(1, 3, 3)
    plt.imshow(img3, cmap=cmap3)
    plt.title(title3)
    plt.axis('off')  # 不顯示座標軸

    plt.show(block=True)  # 強制阻塞直到視窗關閉



def show_four_images(img1, title1, img2, title2, img3, title3, img4, title4, cmap1=None, cmap2=None, cmap3=None, cmap4=None):
    plt.figure(figsize=(18,4))

    plt.subplot(1, 4, 1)
    plt.imshow(img1, cmap=cmap1)
    plt.title(title1)
    plt.axis('off')  # 不顯示座標軸

    plt.subplot(1, 4, 2)
    plt.imshow(img2, cmap=cmap2)
    plt.title(title2)
    plt.axis('off')  # 不顯示座標軸

    plt.subplot(1, 4, 3)
    plt.imshow(img3, cmap=cmap3)
    plt.title(title3)
    plt.axis('off')  # 不顯示座標軸

    plt.subplot(1, 4, 4)
    plt.imshow(img4, cmap=cmap4)
    plt.title(title4)
    plt.axis('off')  # 不顯示座標軸

    plt.show(block=True)  # 強制阻塞直到視窗關閉
