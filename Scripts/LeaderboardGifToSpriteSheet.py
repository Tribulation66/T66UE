"""
Convert SourceAssets/Leaderboard/Streamers.gif to a sprite sheet PNG (grid of frames).
Run from project root with:  python Scripts/LeaderboardGifToSpriteSheet.py
Requires:  pip install Pillow
Output: SourceAssets/Leaderboard/Streamers_SpriteSheet.png
Then import that texture and Global.png, Friends.png into Content/UI/Leaderboard and run
Scripts/CreateLeaderboardFilterAssets.py in the Unreal Editor.
"""

import os
import sys

def main():
    try:
        from PIL import Image
    except ImportError:
        print("Install Pillow: pip install Pillow")
        sys.exit(1)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    gif_path = os.path.join(project_root, "SourceAssets", "Leaderboard", "Streamers.gif")
    out_path = os.path.join(project_root, "SourceAssets", "Leaderboard", "Streamers_SpriteSheet.png")

    if not os.path.isfile(gif_path):
        print(f"GIF not found: {gif_path}")
        sys.exit(1)

    img = Image.open(gif_path)
    frames = []
    try:
        while True:
            frame = img.copy()
            if frame.mode != "RGBA":
                frame = frame.convert("RGBA")
            frames.append(frame)
            img.seek(img.tell() + 1)
    except EOFError:
        pass

    if not frames:
        print("No frames in GIF")
        sys.exit(1)

    n = len(frames)
    cols = int(n ** 0.5)
    if cols < 1:
        cols = 1
    rows = (n + cols - 1) // cols

    w, h = frames[0].size
    sheet_w = w * cols
    sheet_h = h * rows
    sheet = Image.new("RGBA", (sheet_w, sheet_h), (0, 0, 0, 0))

    for i, frame in enumerate(frames):
        row, col = divmod(i, cols)
        sheet.paste(frame, (col * w, row * h))

    sheet.save(out_path)
    print(f"Saved {n} frames to {out_path} ({cols}x{rows})")
    print(f"In CreateLeaderboardFilterAssets / M_LeaderboardStreamersSprite use NumColumns={cols}, NumRows={rows}, FrameCount={n}")


if __name__ == "__main__":
    main()
