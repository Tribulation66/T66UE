# Tool Setup Instructions

## Local Paths

Blender:

```text
C:\Program Files\Blender Foundation\Blender 5.1\blender.exe
```

Mob animation workspace:

```text
C:\UE\T66\Model Generation\Rigging and Animation
```

The previous humanoid rigging vendor setup is no longer part of the active mob workflow. Leave any local vendor cache in `External/` alone unless the user explicitly asks to clean it; do not route future mob work through those retired tools.

## Blender Preview Requirements

Mob previews should be rendered natively from Blender as MP4 files, not stitched from still frames. Use the current preview tool unless a mob needs a new specialized renderer:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python "C:\UE\T66\Model Generation\Rigging and Animation\Tools\render_easy_mob_movement_preview.py" -- --enemy-id Slime --out-root "C:\UE\T66\Model Generation\Rigging and Animation\Runs\Slime_MoveTowardBouncyStutterPreview_V2_20260521" --frames 72 --fps 15 --width 1280 --height 720
```

Preview materials should use unlit/emissive display so the video represents the intended unlit game-read and does not hide form issues behind lighting.

## Unreal VAT Tooling

UE 5.7 includes the Experimental AnimToTexture plugin at:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\Experimental\AnimToTexture
```

The project path for imported mob VAT data is documented in:

- `04_MOB_VERTEX_ANIMATION_PIPELINE_INSTRUCTIONS.md`
- `05_EASY_MOB_VERTEX_ANIMATION_BATCH_INSTRUCTIONS.md`

Use `Tools/import_easy_mob_vat_to_unreal.py` and `Tools/verify_easy_mob_vat_in_unreal.py` for the current Easy mob VAT path.

## Reverification

After changing Blender, Unreal, the VAT import tool, or the preview renderer:

1. Render a short Slime movement preview.
2. Compile the changed Python scripts.
3. If Unreal assets or data rows changed, run the matching VAT verifier.
4. If playable content changed, refresh the staged standalone build per the root rule.
