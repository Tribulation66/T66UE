# Leaderboard filter buttons (G / F / S)

The main menu leaderboard has three filter buttons: **Global**, **Friends**, **Streamers**.  
Each is a border-only button whose content is an icon (PNG for Global/Friends, animated sprite sheet for Streamers).

## Setup

1. **Sprite sheet from GIF** (for Streamers icon):
   ```bash
   pip install Pillow
   python Scripts/LeaderboardGifToSpriteSheet.py
   ```
   This creates `SourceAssets/Leaderboard/Streamers_SpriteSheet.png`.

2. **Import and create assets in Unreal**  
   In the Editor: **Tools → Execute Python Script** → `Scripts/CreateLeaderboardFilterAssets.py`  
   This will:
   - Import `Global.png`, `Friends.png`, `Streamers_SpriteSheet.png` into `/Game/UI/Leaderboard/`
   - Create `M_LeaderboardStreamersSprite` (material with `Frame` and `SpriteSheet` parameters)
   - Create `WBP_LeaderboardFilterButton` (widget Blueprint)

3. **Wire the sprite-sheet material**  
   Open `M_LeaderboardStreamersSprite` in the Material Editor. Add the sprite-sheet UV logic:
   - Add scalar parameters: `Frame`, `NumColumns`, `NumRows` (e.g. 4, 5 if 20 frames).
   - Row = `Floor(Frame / NumColumns)`, Col = `Frame - Row * NumColumns`.
   - UV = `( (Col + TexCoord.U) / NumColumns, (Row + TexCoord.V) / NumRows )`.
   - Use that UV to sample the `SpriteSheet` texture and connect to Emissive Color (or Base Color).

4. **Filter button Blueprint (optional – button/image created in C++ if empty)**  
   Open `WBP_LeaderboardFilterButton`:
   - Add a **Button** named exactly `FilterButton` (border-only style if desired).
   - Add an **Image** named exactly `IconImage` as the button’s child (content).
   - Save. You can also leave the hierarchy empty; the C++ code creates the Button and Image at runtime. If icons don't show, confirm `/Game/UI/Leaderboard/` has the texture assets.

After this, the main menu will show the three icons (and the Streamers one will animate when the material and frame count are set).
