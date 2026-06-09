# Decision Block: Female Hero 1 Rubber Material Source Asset

## Status

Waiting for Pablo confirmation before any Blender material/look-development work.

## Decision Needed

Confirm the source asset path for the Blender-only rubber/vinyl material target pass.

## Key Finding

There is no confirmed female Hero 1 FriendSlop 3D mesh in the live repo.

The repo currently splits the user's wording into two different asset classes:

1. Recent FriendSlop female Hero 1 reference image:
   `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`
2. Existing female Hero 1 3D mesh/source asset:
   `C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb`

The first is the recent FriendSlop-direction female reference, but it is a 2D PNG and cannot directly host Blender material look-dev. The second is a 3D Pixel/Pixal3D Stacy mesh suitable for Blender material look-dev, but it is not the newer FriendSlop female image.

## Option A: Existing 3D Mesh, Blender Can Start Now

Use the current female Hero 1 non-demo 3D source:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb`

Evidence:

- File exists, 11,122,448 bytes, last modified 2026-05-21 22:58:02 local time.
- `Content\Data\CharacterVisuals.csv` has active row `Hero_1_Stacy` using the female Hero 1 Stacy runtime visual path.
- `Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\hero_demo_lineup_accuRig_manifest.json` names `Hero_1_Stacy` as display name `Hero 1 Stacy`, category `hero`, source image `SourceAssets/ToonStyle/ImageGen/HeroDesign/Hero_1_Stacy/hero_1_stacy_detail_parity_bust_20260522_v01.png`.
- The same run also has `AccuRig_Textured\Hero_1_Stacy\Hero_1_Stacy_Textured.fbx`, but the mission asks for the Pixel3D model/source target and a Blender material visual target, so the raw generated GLB is the cleaner baseline for "raw Pixel3D output vs rubber version."
- Caveat: this is the Pixal3D/ToonStyle female 3D mesh, not a confirmed FriendSlop female 3D mesh.

## Option B: FriendSlop Female Direction, Blender Is Blocked Until Mesh Generation

Use the recent FriendSlop female Hero 1 PNG as the look/source direction:

`C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`

Evidence:

- File exists, 952,198 bytes, last modified 2026-06-04 10:54:04 local time.
- This is the recent FriendSlop female Hero 1 reference, but it is a 2D PNG, not a Blender-ingestable 3D mesh.
- A female FriendSlop 3D mesh was not found in `FriendSlop`, `Model Generation\Runs\Pixal3D`, or `Content\Characters\Heroes\Hero_1`.

## Alternate Paths Found

- Recent FriendSlop female 2D reference PNG:
  `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`
- Processed production FBX:
  `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Stacy\Working\Hero_1_Stacy.fbx`
- Textured AccuRig FBX:
  `C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\AccuRig_Textured\Hero_1_Stacy\Hero_1_Stacy_Textured.fbx`
- Demo skin / beachgoer source:
  `C:\UE\T66\SourceAssets\ToonStyle\Pixal3D\Production\Hero_1_Stacy_Beachgoer\Working\Hero_1_Stacy_Beachgoer.fbx`
- Existing male FriendSlop 3D raw mesh exists in Unreal content, but it is male-only and out of scope for this female-source decision:
  `C:\UE\T66\Content\Characters\Heroes\Hero_1\Chad\FriendSlopRaw\SM_Hero_1_Chad_Male.uasset`

## Choices

1. Use the existing Pixal3D/ToonStyle `Hero_1_Stacy.glb` as the Blender baseline now, accepting that it is not the newer FriendSlop female style.
2. Treat the 2026-06-04 FriendSlop female 2D PNG as the required source direction, which means a female FriendSlop 3D mesh must be generated before this Blender material mission can proceed.
3. Provide a different exact source path.

## Stop Rule

Do not create the lighting rig, material sweep, renders, turntables, or recipe until Pablo confirms one source path.
