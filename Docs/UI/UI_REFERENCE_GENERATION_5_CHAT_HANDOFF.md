# UI Reference Generation Five-Chat Handoff

## Purpose

Use this document to split the first reference-generation pass across five parallel chats while this chat remains the integrator and validator.

Each worker chat should generate screen-specific reference images only. Do not implement runtime widgets, import assets, slice sprite sheets, or rebuild Unreal screens in the worker chats unless the integrator explicitly assigns that later.

## Source Of Truth

- Project root: `C:\UE\T66`
- Canonical main-menu style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- Raw approved source image: `C:\UE\T66\SourceAssets\Reference 3.png`
- Main-menu normalization note: `C:\UE\T66\UI\screens\main_menu\reference\normalization_reference_3.json`
- Per-screen workspace root: `C:\UE\T66\UI\screens\<screen_slug>`
- Current target screenshot path: `C:\UE\T66\UI\screens\<screen_slug>\current\current_state_1920x1080.png`
- Required generated reference path: `C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png`

Current screenshot status as of 2026-04-25: 37 of 46 assigned screens have valid `1920x1080` current screenshots. The remaining nine are listed as blocked below.

Read these before generating:

- `C:\UE\T66\Docs\UI\MASTER.md`
- `C:\UE\T66\Docs\UI\MasterStyle.md`
- `C:\UE\T66\Docs\UI\UI_Screen_Reference_Pipeline.md`
- `C:\UE\T66\Docs\UI\UI_CONTENT_OWNERSHIP_AUDIT.md`
- `C:\UE\T66\Docs\UI\SCREEN_REVIEW_GATE.md`

## Global Output Contract

For each assigned screen, produce exactly one full-screen `1920x1080` canonical reference image that:

- uses the main-menu anchor as the visual style authority only
- uses the current target screenshot as the authority for layout, content count, arrangement, major regions, and control positions
- reinterprets panels, shells, buttons, chrome, tabs, rows, modals, and background treatment in the same Chadpocalypse visual language
- keeps runtime-owned labels, values, names, portraits, icons, timers, avatars, scores, and data regions visibly represented but not treated as final baked runtime art
- is an offline comparison target only, not a runtime background plate

Do not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, explanatory labels, or any other content not present in the current screenshot and layout list.

Style direction is sleek, modern, minimalist, clean planar surfaces, crisp borders, flat or satin metallic accents, restrained gold, and a clean red/black/charcoal scheme while preserving the same font, layout, and content.

Negative style guardrails: no grain, no cracked stone, no gemstone/crystal/beveled fantasy surface, no noisy distressed panels, no rubble texture, and no micro-detail borders.

Save the accepted output to:

`C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png`

Also save a small note beside it:

`C:\UE\T66\UI\screens\<screen_slug>\reference\reference_generation_notes.md`

The note should include:

- chat number
- source current screenshot path
- anchor path
- whether the source screenshot already existed or had to be captured
- any runtime-owned regions that must be masked/replaced during later runtime reconstruction
- whether the reference is accepted, blocked, or needs regeneration

## Hard Rules

- Do not redesign the screen structure.
- Do not generate active references for deprecated or stale targets: `wheel_overlay`, `party_size_picker`, `casino_overlay`, `gambler_overlay`, `vendor_overlay`, standalone `leaderboard`, legacy `Lobby`, `LobbyReadyCheck`, `LobbyBackConfirm`, `HeroLore`, or `CompanionLore`.
- Use canonical active naming: `powerup` instead of `shop`, and `minigames` instead of `unlocks`.
- Do not make sprite sheets, component boards, layout diagrams, annotated boards, or variant grids during this pass.
- Do not generate multiple competing visual directions unless the integrator asks for variants.
- Do not use the full reference as a runtime background in later implementation.
- Do not manually pixel-edit, paint over, erase, patch, clone, repaint, or screenshot-repair generated outputs.
- If generated pixels are wrong, regenerate the reference.
- If the current screenshot is missing, capture it first if the capture route is known. If capture is not currently automated, report `blocked` and name the missing screenshot.
- If a generated image is not exactly `1920x1080`, keep the raw output and normalize an accepted landscape-safe copy with:

```powershell
python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> <screen_slug>_reference_1920x1080.png --target-width 1920 --target-height 1080
```

## Current Screenshot Capture

Use this command shape for missing current screenshots when the screen key is known:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenKey> -Output C:\UE\T66\UI\screens\<screen_slug>\current\current_state_1920x1080.png
```

Known capture keys from the current resolver include:

| Screen slug | Screen key |
|---|---|
| `achievements` | `Achievements` |
| `companion_grid` | `CompanionGrid` |
| `companion_selection` | `CompanionSelection` |
| `hero_grid` | `HeroGrid` |
| `hero_selection` | `HeroSelection` |
| `run_summary` | `RunSummary` |
| `save_slots` | `SaveSlots` |
| `settings` | `Settings` |
| `powerup` | `PowerUp` |
| `temporary_buff_shop` | `TemporaryBuffShop` |
| `minigames` | `Minigames` |

Gameplay overlay captures use the gameplay automation path:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen "" -Output C:\UE\T66\UI\screens\<screen_slug>\current\current_state_1920x1080.png -DelaySeconds 5.0 -TimeoutSeconds 90 -ExtraArgs @('/Game/Maps/GameplayLevel','-T66GameplayAutoScreenshot=C:\UE\T66\UI\screens\<screen_slug>\current\current_state_1920x1080.png','-T66GameplayAutoCapture=<mode>','-T66GameplayAutoScreenshotDelay=5.0')
```

Confirmed gameplay capture modes from this pass:

| Screen slug | Gameplay capture mode | Status |
|---|---|---|
| `inventory` | `inventory` | captured |

The following assigned screens need a current screenshot, but a reliable automated key was not confirmed in this pass:

- `arcade_popup`
- `cowardice_prompt`
- `enemy_health_bar`
- `enemy_lock`
- `floating_combat_text`
- `hero_cooldown_bar`
- `loading_screen`
- `minimap`
- `whack_a_mole_arcade`

For these, do not guess. Report `blocked: missing current screenshot` unless the integrator supplies a capture path or confirms a capture command.

## Explicit Exclusions For This Pass

Do not generate references for these in the five worker chats:

- `main_menu` - this is the style anchor for this pass
- `wheel_overlay` - deprecated active target
- `party_size_picker` - deprecated active target
- `casino_overlay` - deprecated shell target
- `gambler_overlay` - covered by `casino_gambling_tab`
- `vendor_overlay` - covered by `casino_vendor_tab`
- `leaderboard` - standalone reference target is deprecated
- legacy `Lobby`, `LobbyReadyCheck`, `LobbyBackConfirm`, `HeroLore`, and `CompanionLore` concepts
- `mini_character_select`
- `mini_companion_select`
- `mini_difficulty_select`
- `mini_idol_select`
- `mini_main_menu`
- `mini_run_summary`
- `mini_save_slots`
- `mini_shared`
- `mini_shop`
- `td_battle`
- `td_difficulty_select`
- `td_main_menu`

`arcade_popup` and `whack_a_mole_arcade` are still assigned below because they are not under the `mini_*` or `td_*` naming groups. If the integrator later decides all arcade/minigame surfaces are deferred, skip them.

## Prompt Skeleton

Use this structure for each image-generation request:

```text
Generate one full-screen 1920x1080 canonical reference image for the T66 screen "<screen_slug>".

Input images:
1. Main-menu style anchor: C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png
2. Current target screenshot: C:\UE\T66\UI\screens\<screen_slug>\current\current_state_1920x1080.png

Goal:
Preserve the current target screenshot's exact layout, content count, arrangement, major regions, spacing, control positions, and modal/screen hierarchy. Use the main-menu anchor for style only. Reinterpret the visual design as sleek, modern, minimalist game UI: clean planar surfaces, crisp borders, flat or satin metallic accents, restrained gold, clean red/black/charcoal scheme, punchy readable panels, and sharp stateful controls while preserving the same font/layout/content.

Do not make a sprite sheet, asset board, layout diagram, callout sheet, multi-screen comparison, or variant grid. Do not redesign the structure. Do not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, explanatory labels, or any other content not present in the current screenshot and layout list. No grain, cracked stone, gemstone/crystal/beveled fantasy surfaces, noisy distressed panels, rubble texture, or micro-detail borders. Do not bake runtime/localizable text or live values as final runtime art. This is an offline comparison target only.
```

## Chat 1 - Core Frontend And Meta Screens

| Screen slug | Current screenshot | Capture key/status |
|---|---|---|
| `account_status` | exists | ready |
| `achievements` | exists | ready |
| `language_select` | exists | ready |
| `settings` | exists | ready |
| `minigames` | exists | ready |
| `powerup` | exists | ready |
| `save_slots` | exists | ready |
| `save_preview` | exists | ready |
| `loading_screen` | missing | blocked until capture route is confirmed |

Primary risk: these screens are text/value heavy. Preserve layout and hierarchy, but later runtime implementation must own most labels, values, rows, and dynamic data.

## Chat 2 - Hero, Companion, Party, And Modal Flow

| Screen slug | Current screenshot | Capture key/status |
|---|---|---|
| `hero_selection` | exists | ready |
| `hero_grid` | exists | ready |
| `companion_selection` | exists | ready |
| `companion_grid` | exists | ready |
| `party_invite_modal` | exists | ready |
| `player_summary_picker` | exists | ready |
| `quit_confirmation_modal` | exists | ready |
| `report_bug` | exists | ready |

Primary risk: hero/companion previews, portraits, skins, party avatars, player names, and selectable states are runtime-owned. Do not flatten them into final runtime art.

## Chat 3 - Challenges, Run Flow, And Progression Modals

| Screen slug | Current screenshot | Capture key/status |
|---|---|---|
| `challenges` | exists | ready |
| `daily_climb` | exists | ready |
| `temporary_buff_selection` | exists | ready |
| `temporary_buff_shop` | exists | ready |
| `run_summary` | exists | ready |
| `pause_menu` | exists | ready |
| `cowardice_prompt` | missing | blocked until capture route is confirmed |
| `inventory` | exists | ready |
| `whack_a_mole_arcade` | missing | blocked until capture route is confirmed |

Primary risk: run results, buffs, inventory entries, challenge rows, vendor/powerup offers, and summary values are live data. Keep those regions readable and socketed for later runtime ownership.

## Chat 4 - Gameplay HUD And Combat Overlays

| Screen slug | Current screenshot | Capture key/status |
|---|---|---|
| `gameplay_hud` | exists | ready |
| `gameplay_hudfull_map` | exists | ready |
| `gameplay_hudinventory_inspect` | exists | ready |
| `minimap` | missing | blocked until capture route is confirmed |
| `enemy_health_bar` | missing | blocked until capture route is confirmed |
| `enemy_lock` | missing | blocked until capture route is confirmed |
| `floating_combat_text` | missing | blocked until capture route is confirmed |
| `hero_cooldown_bar` | missing | blocked until capture route is confirmed |
| `arcade_popup` | missing | blocked until capture route is confirmed |

Primary risk: HUD work should preserve gameplay visibility. Do not bury the game scene under decorative chrome. HUD references should establish style, spacing, and visual language while leaving live health, cooldown, numbers, map content, and combat text as runtime-owned.

## Chat 5 - Casino, Vendor-Tab, And In-Run Interaction Overlays

| Screen slug | Current screenshot | Capture key/status |
|---|---|---|
| `casino_alchemy_tab` | exists | ready |
| `casino_gambling_tab` | exists | ready |
| `casino_vendor_tab` | exists | ready |
| `collector_overlay` | exists | ready |
| `crate_overlay` | exists | ready |
| `idol_altar_overlay` | exists | ready |
| `lab_overlay` | exists | ready |

Primary risk: vendor/gambler standalone overlays are intentionally excluded. Use `casino_gambling_tab` and `casino_vendor_tab` as the casino-specific surfaces. Offer rows, prices, item icons, wheel outcomes, crate contents, and altar/lab values remain runtime-owned.

## Worker Completion Format

Each worker chat should finish with:

```text
Chat number:
Screens completed:
Screens blocked:
Generated reference paths:
Screens needing regeneration:
Runtime-owned regions to preserve later:
Notes for integrator:
```

Do not claim a screen is production-complete. For this pass, "completed" means only that the screen-specific offline reference exists at the required path and is ready for integrator review.
