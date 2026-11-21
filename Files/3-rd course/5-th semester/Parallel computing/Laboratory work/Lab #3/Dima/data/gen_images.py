#!/usr/bin/env python3
# Генерация 4 тестовых BMP-изображений (24 бита) для Задания 3.2.
# Разрешения: от 1280x720 до 2560x1440 (по заданию).
# Добавлен импульсный (соль-перец) шум, чтобы эффект медианного фильтра
# был визуально заметен и измерим.
import numpy as np
from PIL import Image

np.random.seed(42)

SIZES = [
    (1280, 720),
    (1600, 900),
    (1920, 1080),
    (2560, 1440),
]

def make_base(w, h):
    x = np.linspace(0, 1, w)
    y = np.linspace(0, 1, h)
    xx, yy = np.meshgrid(x, y)
    r = (np.sin(xx * 10) * 0.5 + 0.5) * 255
    g = (np.cos(yy * 8) * 0.5 + 0.5) * 255
    b = ((xx + yy) / 2) * 255
    img = np.stack([r, g, b], axis=-1).astype(np.uint8)
    # несколько геометрических фигур для разнообразия контента
    cx, cy = w // 2, h // 2
    yy_idx, xx_idx = np.mgrid[0:h, 0:w]
    circle_mask = (xx_idx - cx) ** 2 + (yy_idx - cy) ** 2 < (min(w, h) // 4) ** 2
    img[circle_mask] = [255, 255, 255]
    return img

def add_salt_pepper(img, amount=0.03):
    out = img.copy()
    h, w, _ = out.shape
    n_noisy = int(amount * h * w)
    ys = np.random.randint(0, h, n_noisy)
    xs = np.random.randint(0, w, n_noisy)
    half = n_noisy // 2
    out[ys[:half], xs[:half]] = [255, 255, 255]
    out[ys[half:], xs[half:]] = [0, 0, 0]
    return out

for idx, (w, h) in enumerate(SIZES, start=1):
    base = make_base(w, h)
    noisy = add_salt_pepper(base, amount=0.03)
    im = Image.fromarray(noisy, mode="RGB")
    fname = f"images_in/input_{idx}.bmp"
    im.save(fname, format="BMP")
    print(f"input_{idx}.bmp: {w}x{h}")
