# HUD and In-Run Overlay First Styled Pass

Date: 2026-04-24

Status: screen-specific reference gate complete, first runtime styled pass implemented, packaged captures and review diffs complete.

Canonical style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Element Checklist

| Element | Generated shell/frame regions | Runtime-owned live content | Status |
| --- | --- | --- | --- |
| HUD shell/frame pieces | `gameplay_hud` shell panels, stage strip, pickup/reward surfaces | HUD child widgets, timers, values, visibility, gameplay state | First styled pass implemented; packaged capture saved |
| Player portrait/status frame | Portrait/status socket frame in `gameplay_hud` | Portrait brush, hero/status text, state indicators | First styled pass implemented; live portrait well preserved |
| Health/ability/ultimate/passive sockets | Health, ability, idol, passive, and ultimate socket frames | HP, ability icons, cooldowns, passive/ultimate state | First styled pass implemented; runtime state remains live |
| Inventory slot normal/hover/selected/disabled | Slot frame family in `gameplay_hud` and `gameplay_hudinventory_inspect` | Item icons, counts, selection, inspect scaling | First styled pass implemented; inspect capture saved |
| Minimap/full-map frame | Minimap frame in `gameplay_hud`; full-map shell in `gameplay_hudfull_map` | Map pixels, fog/reveal data, tower art, markers | First styled pass implemented; live map data preserved |
| Pickup/chest/reward HUD panels | Reward/chest card shell regions in `gameplay_hud` and crate family | Reward text, item icons, values, choices | First styled pass implemented for shared HUD surfaces |
| Idol altar card shells/buttons | Altar overlay shell and card/action button frames | Idol icons, names, descriptions, selection state | First styled pass implemented; needs richer card art in later sprite pass |
| Casino gambling tab buttons/content panels | Casino tab shell packs and `casino_gambling_tab` layout | Tab labels, cards, bets, gold/debt, game state | First styled pass implemented; canonical casino tab naming active; clean tab/button plates applied |
| Casino vendor/gambling/alchemy tabs | `casino_vendor_tab`, `casino_gambling_tab`, `casino_alchemy_tab` shells | Embedded vendor/gambler/alchemy state and inventory | First styled pass implemented; all three tab captures saved |
| Collector/Lab/Crate family | Scoped overlay shell packs | Items, rewards, crate results, lab state | First styled pass implemented; packaged captures and review diffs saved |

## Screen Packs

Each assigned pack now contains a 1920x1080 current capture, `layout/layout_list.md`, and `reference/<screen_slug>_reference_1920x1080.png` generated from the required three inputs: the main-menu anchor, the current target screenshot, and the screen layout list.

- `C:\UE\T66\UI\screens\gameplay_hud`
- `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect`
- `C:\UE\T66\UI\screens\gameplay_hudfull_map`
- `C:\UE\T66\UI\screens\idol_altar_overlay`
- `C:\UE\T66\UI\screens\casino_vendor_tab`
- `C:\UE\T66\UI\screens\casino_gambling_tab`
- `C:\UE\T66\UI\screens\casino_alchemy_tab`
- `C:\UE\T66\UI\screens\collector_overlay`
- `C:\UE\T66\UI\screens\lab_overlay`
- `C:\UE\T66\UI\screens\crate_overlay`

## Runtime / Organization Files

Primary runtime files touched by this pass:

- `C:\UE\T66\Source\T66\Core\T66LocalizationSubsystem.cpp`
- `C:\UE\T66\Source\T66\Core\T66LocalizationSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController.h`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_WorldDialogue.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66VendorNPC.cpp`
- `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.h`
- `C:\UE\T66\Source\T66\UI\Gambler\T66GamblerOverlayWidget_Build.cpp`
- `C:\UE\T66\Source\T66\UI\Gambler\T66GamblerOverlayWidget_Economy.cpp`
- `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Build.cpp`
- `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Overlays.cpp`
- `C:\UE\T66\Source\T66\UI\Style\T66Style.cpp`
- `C:\UE\T66\Source\T66\UI\T66CasinoOverlayShared.h`
- `C:\UE\T66\Source\T66\UI\T66CasinoOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66CasinoOverlayWidget.h`
- `C:\UE\T66\Source\T66\UI\T66CollectorOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66CrateOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66IdolAltarOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66LabOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66VendorOverlayWidget.cpp`
- `C:\UE\T66\Source\T66\UI\T66VendorOverlayWidget.h`

Generated reference-layout headers added for the active assigned packs:

- `C:\UE\T66\Source\T66\UI\Style\T66GameplayHUDReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66GameplayHUDInventoryInspectReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66GameplayHUDFullMapReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66IdolAltarOverlayReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66CasinoVendorTabReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66CasinoGamblingTabReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66CasinoAlchemyTabReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66CollectorOverlayReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66LabOverlayReferenceLayout.generated.h`
- `C:\UE\T66\Source\T66\UI\Style\T66CrateOverlayReferenceLayout.generated.h`

## Packaged Outputs

- `C:\UE\T66\UI\screens\gameplay_hud\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\gameplay_hudinventory_inspect\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\gameplay_hudfull_map\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\idol_altar_overlay\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\casino_vendor_tab\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\casino_gambling_tab\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\casino_alchemy_tab\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\collector_overlay\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\lab_overlay\outputs\2026-04-24\packaged_runtime_1920x1080.png`
- `C:\UE\T66\UI\screens\crate_overlay\outputs\2026-04-24\packaged_runtime_1920x1080.png`

## Packaged Review Diffs

Each pack has `review\2026-04-24\review_notes.md`, `diff_overlay_1920x1080.png`, and `diff_metrics.txt`.

| Screen pack | Mean RGB diff |
| --- | ---: |
| `gameplay_hud` | 47.56 |
| `gameplay_hudinventory_inspect` | 53.44 |
| `gameplay_hudfull_map` | 12.39 |
| `idol_altar_overlay` | 43.59 |
| `casino_vendor_tab` | 32.64 |
| `casino_gambling_tab` | 41.03 |
| `casino_alchemy_tab` | 19.99 |
| `collector_overlay` | 27.18 |
| `lab_overlay` | 53.43 |
| `crate_overlay` | 48.62 |

## Validation

- Non-unity build command succeeded: `Build.bat T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -DisableUnity`
- Requested stage command succeeded: `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook`
- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Scoped search found no remaining obsolete vendor dialogue, teleport-to-brother prompt, or non-canonical casino-family references in `Source\T66`, `UI\screens`, or this checklist.

## Remaining Visual Deltas

- This is a first styled runtime pass using procedural Slate surfaces plus reused main-menu button/tab plate assets. Several overlays still need a later sprite-family pass to reach the richer generated references.
- Idol altar cards, casino embedded panels, and powerup item rows remain flatter than their generated references, though visible action and tab buttons now use clean plates.
- Full-map capture is visually sparse because the current runtime data set has little revealed map content.
- Casino vendor tab now avoids the previous clipped sell button; item rows and inventory sockets still need richer component art.
