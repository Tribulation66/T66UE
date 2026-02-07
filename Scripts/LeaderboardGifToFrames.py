"""
Export individual frames from SourceAssets/Leaderboard/Streamers.gif.
Run: python Scripts/LeaderboardGifToFrames.py
Requires: pip install Pillow
Output: SourceAssets/Leaderboard/Streamers_Frame_00.png, _01.png, etc.
Then import them into /Game/UI/Leaderboard/ via CreateLeaderboardFilterAssets.py.
"""
import os, sys

def main():
    try:
        from PIL import Image
    except ImportError:
        print("Install Pillow: pip install Pillow")
        sys.exit(1)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    gif_path = os.path.join(project_root, "SourceAssets", "Leaderboard", "Streamers.gif")
    out_dir = os.path.join(project_root, "SourceAssets", "Leaderboard")
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
    for i, frame in enumerate(frames):
        out_path = os.path.join(out_dir, f"Streamers_Frame_{i:02d}.png")
        frame.save(out_path)
        print(f"Saved frame {i}: {out_path}")
    print(f"Total: {len(frames)} frames")

if __name__ == "__main__":
    main()
