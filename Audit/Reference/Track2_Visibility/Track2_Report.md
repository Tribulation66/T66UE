# Track 2 Visibility Report

Generated: 2026-05-12

## Interruption Check

The stopped loop was a capture/setup problem, not a useful repeated rendering pass. The first blank captures were caused by the screenshot camera aiming above the slime bounds; the final capture setup centers on the mesh bounds and produces visible actors in all required screenshots.

The remaining visual limitation is that the current Dungeon Slime surface renders nearly black in both the baseline and view-space test variants. That matches the ongoing texture/UV darkness investigation and makes black outline and lighting differences hard to evaluate visually in this pass.

## Task 1 Implementation

### Assets

| Asset | Purpose |
| --- | --- |
| `/Game/Materials/M_GLB_ViewSpaceLit_Character` | New view-space lit character master material |
| `/Game/Materials/MI_GLB_ViewSpaceLit_Character_Test` | Test instance parented to the new master, default parameters |
| `/Game/Materials/MI_TestSlime_ViewSpaceLit` | Test-only slime instance with the slime texture pre-bound |
| `/Game/Materials/MI_TestSlime_Unlit` | Test-only unlit baseline slime instance |

### Material Setup

`M_GLB_ViewSpaceLit_Character` is a Surface material using the Unlit shading model, Opaque blend mode, and emissive output. The material keeps the existing texture binding names used by the current runtime path:

| Parameter | Default |
| --- | --- |
| `BaseColorTexture` | unset on master |
| `EmissiveTexture` | alias parameter present for DMI compatibility |
| `DiffuseColorMap` | alias parameter present for DMI compatibility |
| `Brightness` | `1.0` |
| `Tint` | `(1.0, 1.0, 1.0, 1.0)` |
| `LightDirection_ViewSpace` | `(0.4, 0.4, 1.0)` |
| `ShadowTint` | `(0.55, 0.55, 0.65)` |
| `MidtoneTint` | `(0.85, 0.85, 0.9)` |
| `HighlightTint` | `(1.0, 1.0, 1.0)` |
| `RampStep1` | `0.4` |
| `RampStep2` | `0.75` |
| `RimStrength` | `0.35` |
| `RimColor` | `(1.0, 1.0, 1.0)` |
| `RimPower` | `2.5` |

The material samples the base texture, transforms the world normal to view space, computes `NdotL` against `LightDirection_ViewSpace`, applies a three-band cel ramp, adds a view-space rim term, and writes the final color to Emissive Color.

Deviation from the node-level spec: the core view-space lighting math is implemented in a Custom HLSL material node rather than an explicit graph of `Lerp` and `If` nodes. The math follows the requested formula and preserves the compatible parameter names.

## Task 2 Implementation

### Assets

| Asset | Purpose |
| --- | --- |
| `/Game/Materials/Retro/M_T66_OutlinePostProcess` | New character outline post-process material |

`M_T66_OutlinePostProcess` is a Post Process material intended for Before Tonemapping. It samples `PostProcessInput0` plus `CustomStencil` at the center UV and 8 neighboring UVs offset by `OutlineThickness` in screen-pixel space. A pixel is considered an edge when the center stencil differs from any sampled neighbor around `CharacterStencilValue`, then the scene color is lerped toward `OutlineColor` by `OutlineOpacity`.

The existing `UE5RFX_CustomDepthTest` function was not reused; the stencil comparison was built inline in the outline material so it could compare center and neighbor samples directly.

### Parameters

| Parameter | Default |
| --- | --- |
| `OutlineColor` | `(0.0, 0.0, 0.0)` |
| `OutlineThickness` | `1.5` |
| `OutlineOpacity` | `1.0` |
| `CharacterStencilValue` | `2.0` |

### Runtime Integration

| File | Lines | Current state |
| --- | ---: | --- |
| `Source/T66/Core/T66RetroFXSettings.h` | `95` | Adds `bEnableCharacterOutline`, default `true` for testing |
| `Source/T66/Core/T66RetroFXSubsystem.h` | `91`, `119`, `150` | Adds outline parameter helper, outline material loader, and outline DMI storage |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `51` | Adds `/Game/Materials/Retro/M_T66_OutlinePostProcess` load path |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `355` | Disabled Retro FX settings also disable outline |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `529` | Queues outline material for Retro FX preload |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `622-626` | Ensures outline blendable entry and applies outline params |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `680-683` | Creates outline DMI and blendable entry |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `720-722` | Uses `bEnableCharacterOutline` to set blendable weight |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `781-791` | Writes outline color, thickness, opacity, and stencil value to the DMI |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `1005-1045` | Keeps character CustomDepth/stencil enabled when either character pixelation or outline is enabled |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `1373-1404` | Classifies `/Game/Characters/` mesh components as Character for the stencil path |
| `Source/T66/Core/T66RetroFXSubsystem.cpp` | `1518-1521` | Loads the outline post-process material |

## Task 3 Verification

### Test Map

| Asset | Purpose |
| --- | --- |
| `/Game/Maps/Test_Track2_Visibility` | Test map with four Dungeon Slime static actors in a row |
| `/Game/Materials/M_Track2_NeutralBackdrop` | Neutral gray backdrop/floor material for screenshots |
| `/Game/Characters/Enemies/Regular/Dungeon_Slime/QuadRetro/SM_Dungeon_Slime_QuadRetro` | Test mesh |

The test map has no directional or sky light actors. Four slime actors use fixed scale and orientation:

| Variant | Material | Outline |
| --- | --- | --- |
| A | `MI_TestSlime_Unlit` | disabled |
| B | `MI_TestSlime_ViewSpaceLit` | disabled |
| C | `MI_TestSlime_Unlit` | enabled |
| D | `MI_TestSlime_ViewSpaceLit` | enabled |

### Screenshots

![Four variant front](Slimes_FourVariants_Front.png)

![Four variant closeup](Slimes_FourVariants_Closeup.png)

![Slime D front](SlimeD_Turntable_front.png)

![Slime D three-quarter left](SlimeD_Turntable_threequarter_left.png)

![Slime D side](SlimeD_Turntable_side.png)

![Slime D three-quarter right](SlimeD_Turntable_threequarter_right.png)

![Slime D back](SlimeD_Turntable_back.png)

### Observations

- All four variants are present in the front and closeup captures.
- The current slime render is a near-black silhouette in every variant. Because the base rendered surface is already black, the black outline pass is difficult to distinguish from the silhouette.
- The view-space lit variant does not show strong visible banding on this slime because the surface color feeding the material is near black. Across the turntable, the visual read remains consistently dark from front, side, and back angles; there is no obvious camera-angle-dependent world-light swing.
- The combined D variant captures are framed consistently enough for turntable comparison, but the current surface darkness limits visual evaluation of the lighting model.

## Standalone Build Status

| Verification | Result |
| --- | --- |
| `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE` | Succeeded |
| `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE` | Succeeded, target up to date after final stage |
| `Scripts/StageStandaloneBuild.ps1` | Succeeded with `BuildCookRun`, `-build`, and `-cook`; no `-SkipCook` |
| Staged executable | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe` |
| Staged pak timestamp | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Content\Paks\T66-Windows.pak`, last write `2026-05-12 20:22:12` |
| Project standalone shortcut | `C:\UE\T66\T66 Standalone.lnk` targets the staged exe |
| Taskbar standalone shortcut | `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk` targets the staged exe |
| Standalone smoke launch | Exit code `0`; `/Game/Maps/GameplayLevel` loaded and exited cleanly |

Standalone smoke screenshot:

![Standalone gameplay smoke](Standalone_GameplaySmoke.png)

Standalone log evidence from `C:\UE\T66\Saved\StandaloneLogs\Track2_StandaloneSmoke.log`:

```text
Browse: /Game/Maps/GameplayLevel?Name=Player
LoadMap: /Game/Maps/GameplayLevel?Name=Player
Bringing World /Game/Maps/GameplayLevel.GameplayLevel up for play
UEngine::LoadMap Load map complete /Game/Maps/GameplayLevel
FPlatformMisc::RequestExitWithStatus(0, 0, UGameEngine::HandleExitCommand)
LogExit: Exiting.
```

## Deviations and Issues

- The view-space lighting material uses a Custom HLSL node for the requested math rather than explicit material graph `Lerp` and `If` nodes.
- The outline post-process uses inline stencil comparison rather than `UE5RFX_CustomDepthTest`.
- The screenshots are valid and actors are visible, but the current Dungeon Slime rendered surface is almost entirely black. That makes both the view-space lighting improvement and the black outline pass hard to judge visually on this specific test mesh.
