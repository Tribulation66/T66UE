# Tool Setup Instructions

## Local Paths

Blender:

```text
C:\Program Files\Blender Foundation\Blender 5.1\blender.exe
```

External cache:

```text
C:\UE\T66\Model Generation\Rigging and Animation\External
```

Reusable setup script:

```powershell
.\Model Generation\Rigging and Animation\Tools\setup_rigging_animation_infrastructure.ps1
```

## Rigodotify

Rigodotify is pulled from GitHub:

```powershell
git clone https://github.com/catprisbrey/Rigodotify.git "C:\UE\T66\Model Generation\Rigging and Animation\External\Rigodotify"
```

Blender add-on installation needs a zip with a top-level `Rigodotify/` directory. A raw `git archive` without a prefix fails because `__init__.py` lands at zip root.

Working package command:

```powershell
git -C "C:\UE\T66\Model Generation\Rigging and Animation\External\Rigodotify" archive --format=zip --prefix=Rigodotify/ --output="C:\UE\T66\Model Generation\Rigging and Animation\External\Rigodotify.zip" HEAD
```

Known Blender setup command:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --python-expr "import bpy; bpy.ops.preferences.addon_enable(module='rigify'); bpy.ops.preferences.addon_install(filepath=r'C:\UE\T66\Model Generation\Rigging and Animation\External\Rigodotify.zip', overwrite=True); bpy.ops.preferences.addon_enable(module='Rigodotify'); bpy.ops.wm.save_userpref()"
```

## Quaternius Packages

Current Downloads source files:

```text
C:\Users\DoPra\Downloads\Universal Animation Library[Standard].zip
C:\Users\DoPra\Downloads\Universal Animation Library 2[Standard].zip
C:\Users\DoPra\Downloads\Universal Base Characters[Standard].zip
C:\Users\DoPra\Downloads\Universal Animation Library[Source].zip
C:\Users\DoPra\Downloads\Universal Animation Library 2[Source].zip
C:\Users\DoPra\Downloads\Universal Base Characters[Source].zip
```

Extracted cache:

```text
External\Quaternius\Universal Animation Library Standard
External\Quaternius\Universal Animation Library 2 Standard
External\Quaternius\Universal Base Characters Standard
External\Quaternius\Universal Animation Library Source
External\Quaternius\Universal Animation Library 2 Source
External\Quaternius\Universal Base Characters Source
```

## GLTF / GLB Note

The Quaternius setup note says Unreal exports use GLTF/GLB because rigged FBX from Blender can trigger a scaling bug and break retargeting in some cases. Prefer GLTF/GLB for Quaternius reference import/export unless a specific Unreal import test proves FBX is safer for a T66 target.

For Unreal import, use the package guidance:

- leave the skeleton unset on first import when creating the library skeleton
- enable import animations
- use 30 kHz to bake bone animation
- snap to closest frame boundary

Use Source `.blend` files for editable animation work. Use GLTF/GLB for Unreal library import/export tests when retargeting or scale fidelity is the main concern.

## Reverification

After changing Blender version, Rigodotify commit, or Quaternius packages:

1. Re-run add-on enable/install check.
2. Run `Tools/inspect_animation_assets.py` on the core Quaternius files.
3. Update `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` with any behavior change.
