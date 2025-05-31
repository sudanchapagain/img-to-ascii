import os
import sys
import time
import tempfile
import subprocess
from pathlib import Path
from PIL import Image, ImageFilter, ImageOps

ASCII_CHARS = " .:-=+*%@#"

def get_video_fps(video_path):
    try:
        cmd = [
            "ffprobe",
            "-v",
            "0",
            "-of",
            "csv=p=0",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=r_frame_rate",
            video_path,
        ]
        output = subprocess.check_output(cmd).decode().strip()
        num, denom = map(int, output.split("/"))

        if denom != 0:
            return num / denom
        else:
            return 24
    except Exception as e:
        print(f"failed to get fps. using default 24. error: {e}")
        return 24


def clear():
    os.system("clear")


def display_image(path, width, mode):
    img = Image.open(path)
    resized = resize_image(img, width, mode)
    ascii_art = image_to_ascii_colored(resized, mode)
    clear()
    print(ascii_art)


def extract_frames(video_path, temp_dir):
    frame_pattern = os.path.join(temp_dir, "frame_%04d.png")
    cmd = f'ffmpeg -i "{video_path}" {frame_pattern} -hide_banner -loglevel error'
    os.system(cmd)


def display_video(video_path, width=100, mode=""):
    fps = get_video_fps(video_path)
    with tempfile.TemporaryDirectory() as temp_dir:
        extract_frames(video_path, temp_dir)
        frame_files = sorted(Path(temp_dir).glob("frame_*.png"))

        for frame_path in frame_files:
            display_image(frame_path, width, mode)
            time.sleep(1 / fps)


def clean_image(img):
    img = img.convert("RGB")
    img = img.filter(ImageFilter.GaussianBlur(radius=1.2))
    img = ImageOps.autocontrast(img)
    return img


def resize_image(img, width, mode=""):
    img = clean_image(img)
    aspect_ratio = img.height / img.width
    if mode in ("hack", "-h", "--mode=hack"):
        factor = 0.5
    else:
        factor = 1
    height = int(width * aspect_ratio * factor)
    return img.resize((width, height))


def rgb_to_ansi256(r, g, b):
    if r == g == b:
        if r < 8:
            return 16
        if r > 248:
            return 231
        return int(((r - 8) / 247) * 24) + 232
    r_val = int(r / 255 * 5)
    g_val = int(g / 255 * 5)
    b_val = int(b / 255 * 5)
    return 16 + 36 * r_val + 6 * g_val + b_val


def image_to_ascii_colored(img, mode=""):
    img = img.convert("RGB")
    width = img.width
    pixels = list(img.getdata())
    ascii_str = ""

    for y in range(img.height):
        line = ""
        for x in range(width):
            r, g, b = pixels[y * width + x]
            gray = int(0.2126 * r + 0.7152 * g + 0.0722 * b)
            char = ASCII_CHARS[gray * (len(ASCII_CHARS) - 1) // 255]
            color_code = rgb_to_ansi256(r, g, b)
            line += f"\x1b[38;5;{color_code}m{char}\x1b[0m"
        if mode in ("hack", "-h", "--mode=hack") and (y % 2 == 1):
            # ascii_str += "\n"
            # TODO: implement properly aligned calculation. neither is good
            continue
        else:
            ascii_str += line + "\n"

    return ascii_str
  

def main():
    if len(sys.argv) < 2:
        print("\nusage: python main.py <file> [width] [mode]")
        print("=============================================\n")
        print("example for video in hack mode:")
        print("  python ascii_viewer.py video.mp4 100 hack")
        sys.exit(1)

    file = sys.argv[1]

    if len(sys.argv) < 2:
        width = 100
    else:
        width = int(sys.argv[2])

    if len(sys.argv) < 3:
        mode = ""
    else:
        mode = sys.argv[3]

    if not os.path.exists(file):
        print("file not found.")
        return

    # TODO: do proper check. idk
    if file.lower().endswith((".jpg", ".jpeg", ".png", ".bmp", ".gif")):
        display_image(file, width, mode)
    else:
        display_video(file, width, mode)


if __name__ == "__main__":
    main()
