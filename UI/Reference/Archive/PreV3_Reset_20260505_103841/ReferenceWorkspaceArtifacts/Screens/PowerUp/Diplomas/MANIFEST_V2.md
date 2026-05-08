# PowerUpDiplomas V2 Manifest

Target: PowerUpDiplomas
Base screen/modal: PowerUp
State: DIPLOMAS / permanent power-up tab
Reference used: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png`
Preferred reference missing: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUpDiplomas.png`

## Pass 01

### Reference Gate

- Preferred state-specific reference was missing.
- Fallback reference visibly shows the Diplomas/Permanent PowerUp tab, so it was used as the exact comparison target.

### Geometry Map At 1920x1080

Reference visible owned UI:

- Content frame: x 36, y 112, w 1848, h 958, resize role 9-slice/fullscreen shell.
- Title: center x 960, y 132, live text.
- Tabs: x 648, y 197, w 624, h 56 total, two horizontal 3-slice plates.
- Info strip: x 118, y 255, w 1684, h 62, 9-slice parchment strip.
- Diploma cards: first x 104, y 340, w 345, h 514; gap about 32, repeated horizontal scroll row.
- Card art wells: x 132, y 454, w 246, h 216, fixed/9-slice well.
- Graduate buttons: x 85, y 763, w 336, h 75, horizontal 3-slice plates with live text and coupon icon.
- Horizontal scrollbar: x 104, y 878, w 1690, h 31, horizontal 3-slice rail/thumb with fixed arrows.

Current packaged capture before new staged cook:

- Content frame: x 31, y 146, w 1858, h 910, 9-slice/fullscreen shell.
- Title: center x 960, y 178, live text.
- Tabs: x 639, y 271, w 640, h 55 total, two horizontal 3-slice plates.
- Info strip: x 103, y 350, w 1698, h 54, 9-slice parchment strip.
- Diploma cards: first x 79, y 425, w 377, h 570; gap about 48, repeated horizontal scroll row.
- Card art wells: x 142, y 565, w 242, h 205, live data art inside frame.
- Graduate buttons: x 92, y 920, w 350, h 65, horizontal 3-slice plates with live text and coupon icon.
- Horizontal scrollbar: x 154, y 1025, w 1584, h 28, horizontal scrollbar.

### Difference List Before Editing

- top-bar-shared: shared top bar/header is taller and uses live current currency/avatar state; frozen by prompt.
- layout: owned content appears about 45-85 px lower than reference after the shared top bar.
- layout: cards are wider/taller than reference placeholders.
- live-data: runtime card titles, stat names, effect text, costs, and diploma art differ from placeholder reference text/data.
- asset: active PowerUp runtime art was pre-v2 generated art and required reset.
- asset: missing state-specific preferred reference, fallback accepted because it visibly shows Diplomas.

### Generated Candidates

- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Working\Pass_01\Candidates\powerup_diplomas_reference_derived_sheet_pass01.png`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Working\Pass_01\Candidates\powerup_diplomas_coupon_ticket_chromakey_pass01.png`

### Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Controls\powerup_controls_controls_sheet.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Controls\powerup_controls_reference_dropdown_field_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Controls\powerup_controls_scrollbar_horizontal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Icons\powerup_iconsgenerated_icon_07_coupon_ticket_white_v1.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_fullscreen_fullscreen_panel_wide.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_info_strip.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_item_art_well.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_upgrade_card_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\ScreenArt\powerup_screen_art_mainmenu_main_menu_scene_plate_v1.png`

Resize contracts:

- Fullscreen shell: 9-slice.
- Info strip: 9-slice.
- Upgrade card: 9-slice.
- Item art well: 9-slice.
- Pill buttons: horizontal 3-slice via existing sliced button path, nearest filtering, zero brush margin.
- Scrollbar art: existing scrollbar brush path; needs runtime proof after stage is unblocked.
- Coupon ticket: fixed image with alpha.

### Archived/Reset Assets

- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Archive\PreV2_Reset_20260505_100033\Buttons`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Archive\PreV2_Reset_20260505_100033\Panels`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Archive\PreV2_Reset_20260505_100033\Controls`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Archive\PreV2_Reset_20260505_100033\Icons`
- `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Archive\PreV2_Reset_20260505_100033\ScreenArt`

### Proof Captures

- Before new staged cook: `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass01_before_packaged_1920x1080.png`

### Source Files Changed

- None.

### Build/Stage Blocker

Command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipShortcutRefresh
```

Recovery performed:

- First attempt failed on AutomationTool singleton/mutex.
- Waited 35 seconds and retried exact command.
- Inspected active UAT/UBT/dotnet processes for `C:\UE\T66`.
- Found an active unattended cook already running for this repo and waited until it exited.
- Reran the full stage command without `-SkipCook`.

External compile failure:

```text
C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp(435,35): error C2664: 'const FSlateBrush *`anonymous-namespace'::ResolveRunSummarySpriteBrush(...)': cannot convert argument 5 from 'TextureFilter' to 'const ESlateBrushDrawType::Type'
C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp(440,19): note: Conversion to enumeration type requires an explicit cast
```

This is outside the PowerUpDiplomas target source file and prevented a new cooked/staged proof capture.

## Resume Handoff

Status remains BLOCKED until the staged packaged screenshot below exists at 1920x1080 and is compared against the reference.

Do not edit `C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp` from this target pass. That compile error is external to PowerUpDiplomas.

After the external RunSummary compile blocker is fixed, resume with:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipShortcutRefresh
```

Then capture:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen PowerUp -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass02_packaged_1920x1080.png -ExtraArgs "-T66PowerUpTab=Permanent"
```

Then compare `C:\UE\T66\UI\Reference\Screens\PowerUp\Diplomas\Proof\PowerUpDiplomas_pass02_packaged_1920x1080.png` against `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png`.

## Sibling-State Coordination Note

PowerUpDiplomas and PowerUpDrugs share:

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp`
- `C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp`

Future PowerUpDrugs work must detect this v2 reset and accepted PowerUp v2 asset set before resetting assets. Do not blindly wipe or overwrite the accepted v2 assets listed above; coordinate shared runtime ownership or create a target-safe handoff first.
