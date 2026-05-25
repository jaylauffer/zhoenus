#!/usr/bin/env python3
"""Generate the tracked Zhoenus launcher icon PNGs.

The icon is intentionally vector-like so the 36px Android density asset remains
legible: a ship, reticle, gate arc, flyer rings, and a subtle Z flight path.
"""

from __future__ import annotations

from pathlib import Path
import subprocess
import tempfile

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[2]
ICON_DIR = Path(__file__).resolve().parent
SCALE = 4
BASE_SIZE = 1024
CANVAS_SIZE = BASE_SIZE * SCALE


def rgba(color: tuple[int, int, int], alpha: int = 255) -> tuple[int, int, int, int]:
    return color[0], color[1], color[2], alpha


def scaled(points: list[tuple[float, float]]) -> list[tuple[int, int]]:
    return [(round(x * SCALE), round(y * SCALE)) for x, y in points]


def blend(a: int, b: int, t: float) -> int:
    return round(a + (b - a) * t)


def draw_gradient_background(image: Image.Image) -> None:
    pixels = image.load()
    w, h = image.size
    for y in range(h):
        ny = y / (h - 1)
        for x in range(w):
            nx = x / (w - 1)
            dx = nx - 0.5
            dy = ny - 0.48
            radial = min(1.0, (dx * dx + dy * dy) ** 0.5 * 1.65)
            vertical = ny
            r = blend(14, 5, radial)
            g = blend(30, 9, radial)
            b = blend(50, 20, radial)
            r = blend(r, 22, max(0.0, 0.45 - vertical) * 0.24)
            g = blend(g, 45, max(0.0, 0.55 - vertical) * 0.30)
            b = blend(b, 68, max(0.0, 0.60 - vertical) * 0.34)
            pixels[x, y] = (r, g, b, 255)


def draw_glow_line(
    layer: Image.Image,
    xy: list[tuple[float, float]],
    color: tuple[int, int, int],
    width: float,
    alpha: int,
    joint: str = "curve",
) -> None:
    draw = ImageDraw.Draw(layer, "RGBA")
    for mult, blur_alpha in [(5.2, 26), (3.0, 44), (1.6, 72)]:
        glow = Image.new("RGBA", layer.size, (0, 0, 0, 0))
        glow_draw = ImageDraw.Draw(glow, "RGBA")
        glow_draw.line(
            scaled(xy),
            fill=rgba(color, min(alpha, blur_alpha)),
            width=round(width * mult * SCALE),
            joint=joint,
        )
        layer.alpha_composite(glow.filter(ImageFilter.GaussianBlur(radius=round(width * 0.8 * SCALE))))
    draw.line(scaled(xy), fill=rgba(color, alpha), width=round(width * SCALE), joint=joint)


def draw_glow_arc(
    layer: Image.Image,
    bbox: tuple[float, float, float, float],
    start: float,
    end: float,
    color: tuple[int, int, int],
    width: float,
    alpha: int,
) -> None:
    draw = ImageDraw.Draw(layer, "RGBA")
    box = tuple(round(v * SCALE) for v in bbox)
    for mult, blur_alpha in [(5.0, 24), (2.7, 48), (1.45, 80)]:
        glow = Image.new("RGBA", layer.size, (0, 0, 0, 0))
        glow_draw = ImageDraw.Draw(glow, "RGBA")
        glow_draw.arc(box, start, end, fill=rgba(color, min(alpha, blur_alpha)), width=round(width * mult * SCALE))
        layer.alpha_composite(glow.filter(ImageFilter.GaussianBlur(radius=round(width * 0.75 * SCALE))))
    draw.arc(box, start, end, fill=rgba(color, alpha), width=round(width * SCALE))


def draw_flyer(layer: Image.Image, cx: float, cy: float, r: float, angle: float) -> None:
    draw = ImageDraw.Draw(layer, "RGBA")
    box = (
        round((cx - r) * SCALE),
        round((cy - r) * SCALE),
        round((cx + r) * SCALE),
        round((cy + r) * SCALE),
    )
    inner = (
        round((cx - r * 0.46) * SCALE),
        round((cy - r * 0.46) * SCALE),
        round((cx + r * 0.46) * SCALE),
        round((cy + r * 0.46) * SCALE),
    )
    draw.ellipse(box, fill=rgba((255, 185, 62), 235), outline=rgba((255, 245, 178), 240), width=round(4 * SCALE))
    draw.ellipse(inner, fill=rgba((14, 27, 43), 245), outline=rgba((255, 107, 55), 230), width=round(3 * SCALE))
    tail_len = r * 1.25
    from math import cos, sin, radians

    a = radians(angle)
    p1 = (cx - cos(a) * r * 0.9, cy - sin(a) * r * 0.9)
    p2 = (cx - cos(a) * tail_len, cy - sin(a) * tail_len)
    draw.line(scaled([p1, p2]), fill=rgba((255, 106, 50), 170), width=round(5 * SCALE))


def draw_icon() -> Image.Image:
    image = Image.new("RGBA", (CANVAS_SIZE, CANVAS_SIZE), (0, 0, 0, 255))
    draw_gradient_background(image)
    layer = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(layer, "RGBA")

    # Safe-area shield, so the mark stays readable under platform-rounded masks.
    shield = tuple(round(v * SCALE) for v in (74, 74, 950, 950))
    draw.rounded_rectangle(
        shield,
        radius=round(184 * SCALE),
        fill=rgba((7, 16, 30), 110),
        outline=rgba((93, 237, 255), 42),
        width=round(5 * SCALE),
    )

    # Z-shaped flight path behind the ship.
    draw_glow_line(
        layer,
        [(274, 324), (686, 250), (382, 620), (748, 704)],
        (72, 229, 255),
        17,
        170,
    )

    # Gate of Oblivion as a clean readable arc, not trap geometry.
    draw_glow_arc(layer, (230, 210, 842, 836), -38, 222, (75, 226, 255), 18, 188)
    draw_glow_arc(layer, (274, 258, 798, 788), 206, 318, (255, 88, 54), 16, 170)

    # World-space reticle triangle: direct nod to the current in-game aiming aid.
    reticle = [(512, 178), (800, 760), (224, 760), (512, 178)]
    draw_glow_line(layer, reticle, (165, 255, 44), 13, 235)
    draw.line(scaled([(512, 228), (512, 666)]), fill=rgba((165, 255, 44), 150), width=round(5 * SCALE))
    draw.line(scaled([(330, 716), (694, 716)]), fill=rgba((165, 255, 44), 130), width=round(5 * SCALE))

    # DonutFlyers in the reticle influence envelope.
    draw_flyer(layer, 710, 300, 58, 228)
    draw_flyer(layer, 296, 660, 46, 35)

    # Central ship silhouette: nose upward, engine glow aft.
    ship_shadow = [(512, 244), (624, 598), (560, 566), (532, 784), (492, 784), (464, 566), (400, 598)]
    draw.polygon(scaled([(x + 16, y + 22) for x, y in ship_shadow]), fill=rgba((0, 0, 0), 112))
    ship = [(512, 230), (632, 608), (562, 572), (534, 798), (512, 860), (490, 798), (462, 572), (392, 608)]
    draw.polygon(scaled(ship), fill=rgba((220, 248, 255), 246))
    draw.line(scaled([(512, 230), (562, 572), (534, 798)]), fill=rgba((116, 211, 255), 170), width=round(8 * SCALE))
    draw.line(scaled([(512, 230), (462, 572), (490, 798)]), fill=rgba((255, 255, 255), 150), width=round(6 * SCALE))
    draw.polygon(scaled([(512, 858), (548, 794), (476, 794)]), fill=rgba((255, 96, 42), 228))
    draw_glow_line(layer, [(512, 822), (512, 946)], (255, 92, 35), 18, 145)

    # Nose and cockpit accents.
    draw.polygon(scaled([(512, 278), (548, 426), (512, 492), (476, 426)]), fill=rgba((22, 45, 70), 230))
    draw.polygon(scaled([(512, 294), (536, 418), (512, 462), (488, 418)]), fill=rgba((92, 235, 255), 210))

    image.alpha_composite(layer)
    return image.resize((BASE_SIZE, BASE_SIZE), Image.Resampling.LANCZOS)


def save_resized(base: Image.Image, size: int, path: Path, rgb: bool = False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    resized = base.resize((size, size), Image.Resampling.LANCZOS)
    if rgb:
        resized = resized.convert("RGB")
    resized.save(path)


def write_android_icons(base: Image.Image) -> None:
    android_root = ROOT / "Build" / "Android" / "res"
    for relative_path, size in (
        ("drawable/icon.png", 48),
        ("drawable-ldpi/icon.png", 36),
        ("drawable-mdpi/icon.png", 48),
        ("drawable-hdpi/icon.png", 72),
        ("drawable-xhdpi/icon.png", 96),
    ):
        save_resized(base, size, android_root / relative_path, rgb=True)


def write_ios_icons(base: Image.Image) -> None:
    ios_root = ROOT / "Build" / "IOS" / "Resources" / "Graphics"
    for filename, size in (
        ("Icon1024.png", 1024),
        ("Icon60@2x.png", 120),
        ("Icon76@2x.png", 152),
        ("Icon83.5@2x.png", 167),
        ("Icon60@3x.png", 180),
        ("Icon40@3x.png", 120),
        ("Icon40@2x.png", 80),
        ("Icon29@3x.png", 87),
        ("Icon29@2x.png", 58),
        ("Icon20@3x.png", 60),
        ("Icon20@2x.png", 40),
    ):
        save_resized(base, size, ios_root / filename, rgb=True)


def write_ios_launch_screen(base: Image.Image) -> None:
    save_resized(base, 2048, ROOT / "Build" / "IOS" / "Resources" / "Graphics" / "LaunchScreenIOS.png", rgb=True)


def write_desktop_icons(base: Image.Image) -> None:
    save_resized(base, 256, ROOT / "Build" / "Linux" / "Application.png", rgb=True)

    windows_icon = ROOT / "Build" / "Windows" / "Application.ico"
    windows_icon.parent.mkdir(parents=True, exist_ok=True)
    base.convert("RGB").save(
        windows_icon,
        sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
    )

    mac_root = ROOT / "Build" / "Mac"
    mac_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="zhoenus-iconset-") as tmp_dir:
        iconset = Path(tmp_dir) / "Application.iconset"
        iconset.mkdir(parents=True, exist_ok=True)
        for filename, size in (
            ("icon_16x16.png", 16),
            ("icon_16x16@2x.png", 32),
            ("icon_32x32.png", 32),
            ("icon_32x32@2x.png", 64),
            ("icon_128x128.png", 128),
            ("icon_128x128@2x.png", 256),
            ("icon_256x256.png", 256),
            ("icon_256x256@2x.png", 512),
            ("icon_512x512.png", 512),
            ("icon_512x512@2x.png", 1024),
        ):
            # Keep alpha so AppKit reports 4 samples at 8 bits for Unreal's ICNS reader.
            save_resized(base, size, iconset / filename)

        subprocess.run(
            ["iconutil", "-c", "icns", "-o", str(mac_root / "Application.icns"), str(iconset)],
            check=True,
        )


def write_icon_set(base: Image.Image) -> None:
    ICON_DIR.mkdir(parents=True, exist_ok=True)
    base.save(ICON_DIR / "ZhoenusIcon-1024.png")
    base.resize((192, 192), Image.Resampling.LANCZOS).save(ROOT / "Zhoenus.png")
    for size in (96, 72, 48, 36):
        base.resize((size, size), Image.Resampling.LANCZOS).save(ROOT / f"Zhoenus-{size}.png")
    write_android_icons(base)
    write_ios_icons(base)
    write_ios_launch_screen(base)
    write_desktop_icons(base)


def main() -> None:
    write_icon_set(draw_icon())


if __name__ == "__main__":
    main()
