# Phase 1B - Shaders And Materials Report

Date: 2026-05-17

## Scope

Workstreams A and B built the ToonStyle shader infrastructure and material layer for the TestRoom-only Phase 1B foundation.

## Shader Directory Registration

Shader mapping is registered in the T66 module startup, not in `T66.uproject`.

- File: `Source/T66/T66.cpp`
- Mapping: `/Project/ToonStyle`
- Disk path: `FPaths::ProjectDir() / TEXT("ToonStyle/Shaders/Public")`
- Build dependency added: `RenderCore` in `Source/T66/T66.Build.cs`

The corrected path is `ToonStyle/Shaders/Public`, not `T66/ToonStyle/Shaders/Public`, because `ProjectDir()` already resolves to `C:/UE/T66/`.

## Shader Files

Created:

- `ToonStyle/Shaders/Public/ToonShadingCommon.ush`
- `ToonStyle/Shaders/Public/ToonOutline.ush`

`ToonShadingCommon.ush` includes:

- `ToonCelBand`
- `ToonThreeTone`
- `ToonRim`
- `ToonCharacterShade`
- `ToonEnvironmentShade`

The character/environment ramp uses three tones and smoothstep AA based on `ddx`/`ddy`, with a minimum transition width to avoid unstable hard edges.

`ToonOutline.ush` includes `ToonOutlineWorldPositionOffset`. The final Phase 1B version takes geometric normal, base width, vertex G width multiplier, vertex B depth offset, and a mask. Camera-distance width modulation is deferred because the cooked SM6 outline material failed when the WPO Custom node used world/camera position expressions.

## Master Materials

Created:

- `/Game/ToonStyle/Materials/M_Toon_Character`
- `/Game/ToonStyle/Materials/M_Toon_Environment`
- `/Game/ToonStyle/Materials/M_Toon_Character_Outline`

All are Unlit materials. Character and environment output to Emissive Color. The outline material is an inverted-hull material implemented as a two-sided masked material using `TwoSidedSign` to render back faces only.

Important implementation detail: Custom nodes use `IncludeFilePaths` for `.ush` includes. Inline `#include` text inside Custom node code was avoided because Unreal wraps Custom node code inside generated functions and the include dependency was not cook-safe.

## Material Instances

Created/reparented character material instances for:

- `lubu_validation`
- `aria`
- `gambler`
- `slime`
- `tombspider`
- `cavebat`
- `idolaltar`
- `arcademachine`
- `lootchest`
- `lootcrate`

Created matching outline instances for each asset. Created/reparented environment instances:

- `/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Wall`
- `/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Ceiling`
- `/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Floor`

The instance binding evidence is in `Saved/Codex/ToonStyle/Phase1B/toon_material_instances_verify.json`.

## Gates

G1 passed.

- Evidence: `Saved/Codex/ToonStyle/Phase1B/gate_g1_verify.json`
- Test material: `/Game/ToonStyle/Materials/M_ToonStyle_GateG1`
- Include: `/Project/ToonStyle/_test.ush`
- The temporary test material and `_test.ush` were removed after validation.

G2 passed after one outline-material correction cycle.

- Evidence: `Saved/Codex/ToonStyle/Phase1B/gate_g2_verify.json`
- `M_Toon_Character` compiled successfully with the Custom HLSL include path.
- The outline master initially failed SM6 cook due vertex color alpha masking and vertex-stage world/camera inputs. Final fix: constant outline mask 1.0, no world/camera position inputs in the WPO Custom node.

## Known Deferred Items

- Camera-distance outline thickness modulation is deferred until a cook-safe vertex-stage implementation is added.
- Vertex color A outline mask is authored by Blender but not consumed by the current material graph. The outline mask is constant 1.0 for Phase 1B.
