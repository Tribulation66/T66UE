# T66 UI Audit

Date: 2026-04-02
Workspace: `C:\UE\T66`
Audit scope: programmatic Slate UI under `Source/T66/UI`, plus UI-hosting frontend dev-console code in `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`, and project UI config in `Config/DefaultEngine.ini` / `Config/DefaultGame.ini`.

## Executive Summary

The current UI stack is not using UE's project-level DPI curve in any meaningful way. Most screens inherit `UT66ScreenBase` and are wrapped in a custom `SDPIScaler` rooted at `FT66Style::MakeResponsiveRoot(ReferenceResolution = 1920x1080)`, but several of the most important frontend widgets bypass that path entirely and do their own viewport math. That leaves the project with three overlapping sizing models at once: raw Slate units with hardcoded literals, a custom viewport scaler, and a few one-off screen-specific scale systems.

The observed clipping issues are real and traceable in code. The clearest examples are the hero/companion skin rows, where `EQUIPPED` and buy buttons live inside fixed `90x36` boxes with fixed inner padding, and the vendor/gambler/card flows, where titles, prices, icons, and CTA rows are packed into rigid card heights. `FT66Style::MakeButton` centralizes button creation, but it does not guarantee text safety: its default text path uses `SScaleBox(ScaleToFit, DownOnly)` rather than content-driven growth or explicit wrap/overflow rules.

The highest-risk files for a 1920x1080 baseline pass are:

- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/T66VendorOverlayWidget.cpp`
- `Source/T66/UI/T66GamblerOverlayWidget.cpp`
- `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
- `Source/T66/UI/T66FrontendTopBarWidget.cpp`
- `Source/T66/UI/Screens/T66SettingsScreen.cpp`
- `Source/T66/UI/T66GameplayHUDWidget.cpp`
- `Source/T66/UI/T66WheelOverlayWidget.cpp`

## 1. All Slate Widget Classes

### 1.1 Custom `SWidget` / `SCompoundWidget` / `SLeafWidget` / `SBorder` subclasses

| Class | File | What it does | Sizing / layout behavior | Text handling | Hardcoded size/style literals | Theme / style refs |
|---|---|---|---|---|---|---|
| `ST66AnimatedBackground` | `Source/T66/UI/ST66AnimatedBackground.cpp` | Animated multi-layer background wrapper used by the main menu. | Child-driven `SOverlay`; every layer is `HAlign_Fill` / `VAlign_Fill`; no `ComputeDesiredSize()` override. | None. | No intrinsic size literals; movement/pulse comes from caller-supplied layer tween amplitudes/frequencies. | No direct theme refs. |
| `ST66AnimatedBorderGlow` | `Source/T66/UI/ST66AnimatedBorderGlow.cpp` | Decorative animated border glow wrapper around arbitrary child content. | Child-driven; paints on top of child in `OnPaint`; no desired-size override. | None. | Argument-driven border inset / inner-mid-outer thickness / sweep length / sweep speed. | No direct theme refs. |
| `ST66PulsingIcon` | `Source/T66/UI/ST66PulsingIcon.cpp` | Icon + optional glow pulse wrapper. | Uses `SImage.DesiredSizeOverride`; defaults are `28x28` icon and `50x50` glow. | None. | `28`, `50`, glow opacity/scale tween defaults. | No direct theme refs. |
| `ST66LeaderboardPanel` | `Source/T66/UI/Components/T66LeaderboardPanel.cpp` | Full leaderboard widget: filters, dropdowns, header row, scrollable rows, portraits, party strip, detail modal hook-up. | Mix of fixed constants plus `AutoWidth`, `FillWidth`, `MinDesiredWidth`, `HeightOverride`, `SScrollBox`. Uses explicit column widths so headers and rows align. | Heavy `STextBlock` usage. Rows use `OverflowPolicy(Ellipsis)` for name/source/time columns. Filter/dropdown labels do not wrap. | `RankColumnWidth=76`, `ScoreColumnWidth=128`, `RowPortraitSize=36`, `SquareIconSize=52`, `DropdownHeight=46`, min widths `120/136/112/84`, multiple `10/12/18/24` paddings, body/title fonts `16`. | Heavy `FT66Style`, `T66SlateTexture`, `CoreStyle`. |
| `ST66LeaderboardRowWheelProxy` | `Source/T66/UI/Components/T66LeaderboardPanel.cpp` (local) | Wheel-event proxy so mouse wheel on a row forwards to the owning `SScrollBox`. | Child-driven. | None. | Scroll delta hardcoded to `96`. | None direct. |
| `ST66DragRotatePreview` | `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp` (local) | Transparent drag-rotate / mouse-wheel zoom overlay for the hero preview stage. | No child content, no desired-size override; relies on parent slot size. | None. | `DegreesPerPixel = 0.25`. | None. |
| `ST66DragRotateCompanionPreview` | `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp` (local) | Transparent drag-rotate / zoom overlay for companion preview stage. | No child content, no desired-size override; relies on parent slot size. | None. | `DegreesPerPixel = 0.25`. | None. |
| `ST66PowerUpStatueFillWidget` | `Source/T66/UI/Screens/T66PowerUpScreen.cpp` (local) | Leaf widget that paints a locked statue tint and a clipped lower fill based on upgrade progress. | Desired size comes from explicit arg or falls back to brush image size. | None. | Default fallback desired size is brush size; callers use `PowerUpStatueImageSize(372x548)`. | Uses caller-supplied brushes; surrounding screen uses `FT66Style`. |
| `ST66TopBarGearGlyph` | `Source/T66/UI/T66FrontendTopBarWidget.cpp` (local) | Painted gear/settings glyph. | `ComputeDesiredSize()` default `48x48`; caller may override. | None. | Default desired size `48x48`; multiple proportional tooth/core radii based on size. | No direct theme refs. |
| `ST66TopBarPowerGlyph` | `Source/T66/UI/T66FrontendTopBarWidget.cpp` (local) | Painted power glyph used by top bar utility readouts. | `ComputeDesiredSize()` default `48x48`; caller may override. | None. | Default desired size `48x48`; multiple proportional stroke thicknesses. | No direct theme refs. |
| `ST66TopBarBadgeBackground` | `Source/T66/UI/T66FrontendTopBarWidget.cpp` (local) | Painted coin/coupon badge background. | `ComputeDesiredSize()` default `56x56`; caller may override. | None. | Default desired size `56x56`; coin/coupon geometry uses several fixed inset values (`1`, `4`, `7`, etc.). | No direct theme refs. |
| `ST66EnemyBullseyeWidget` | `Source/T66/UI/T66EnemyLockWidget.cpp` (local) | Painted enemy lock-on reticle / bullseye. | `ComputeDesiredSize()` fixed `44x44`. | None. | `44`, ring radii `15.5/15/10/5.5`, tick lengths around `18/10/3.2`. | No direct theme refs. |
| `ST66LockDotWidget` | `Source/T66/UI/T66EnemyHealthBarWidget.cpp` (local) | Painted red lock indicator dot above enemy health bar. | `ComputeDesiredSize()` fixed `14x14`. | None. | `14`, outer ring thickness `2`. | No direct theme refs. |
| `ST66RingWidget` | `Source/T66/UI/T66GameplayHUDWidget.cpp` (local) | Circular progress ring used in HUD. | `ComputeDesiredSize()` fixed `52x52`. | None. | `52`, default thickness `4`, `64` segments. | No direct theme refs. |
| `ST66DotWidget` | `Source/T66/UI/T66GameplayHUDWidget.cpp` (local) | Painted circular marker dot. | `ComputeDesiredSize()` fixed `12x12`. | None. | `12`, `24` segments. | No direct theme refs. |
| `ST66CrosshairWidget` | `Source/T66/UI/T66GameplayHUDWidget.cpp` (local) | Painted center crosshair; changes color and adds ring when locked. | `ComputeDesiredSize()` fixed `28x28`. | None. | `28`, `Gap=4`, `Len=8`, `Thick=2`, locked ring radius about `10`. | No direct theme refs. |
| `ST66ScopedSniperWidget` | `Source/T66/UI/T66GameplayHUDWidget.cpp` (local) | Full-screen sniper-scope matte and reticle. | `ComputeDesiredSize()` fixed `1920x1080`; then adapts scope diameter to allotted geometry in paint. | None. | Hard-coded desired size `1920x1080`; scope margin constants such as `18`, reticle thickness `1.5/2`. | No direct theme refs. |
| `ST66WorldMapWidget` | `Source/T66/UI/T66GameplayHUDWidget.cpp` (local) | Minimap / full map painter with markers, optional labels, area overlays, player icon. | `ComputeDesiredSize()` returns `228x228` for minimap and `1024x640` for full map. Uses fixed world bounds defaults and explicit clipping. | No `STextBlock`; optional labels are drawn during paint. | `228x228`, `1024x640`, full-world bounds `-50000..50000`, minimap half extent `2500`, many fixed line widths and insets. | Uses `FT66Style` minimap colors. |
| `SCasinoAlchemyDropBorder` | `Source/T66/UI/T66CasinoOverlayWidget.cpp` (local) | Drop target border for casino alchemy drag-and-drop. | Inherits `SBorder`; child-driven sizing. | None. | No intrinsic size; default padding `0`. | Uses `CoreStyle` white brush; enclosing screen uses `FT66Style`. |
| `SCasinoAlchemyInventoryTile` | `Source/T66/UI/T66CasinoOverlayWidget.cpp` (local) | Inventory tile that starts drag-drop for casino alchemy. | Inherits `SBorder`; child-driven sizing. | None. | No intrinsic size; default padding `0`. | Uses `CoreStyle`; enclosing screen uses `FT66Style`. |
| `SCircusAlchemyDropBorder` | `Source/T66/UI/T66CircusOverlayWidget.cpp` (local) | Drop target border for circus alchemy drag-and-drop. | Inherits `SBorder`; child-driven sizing. | None. | No intrinsic size; default padding `0`. | Uses `CoreStyle`; enclosing screen uses `FT66Style`. |
| `SCircusAlchemyInventoryTile` | `Source/T66/UI/T66CircusOverlayWidget.cpp` (local) | Inventory tile that starts drag-drop for circus alchemy. | Inherits `SBorder`; child-driven sizing. | None. | No intrinsic size; default padding `0`. | Uses `CoreStyle`; enclosing screen uses `FT66Style`. |
| `ST66DevConsole` | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp` (local) | Non-shipping dev console overlay with filterable command list. | Overlay with top-centered panel; list body uses `FillHeight(1)`. No responsive wrapper. | Uses `STextBlock`, `SEditableTextBox`, `SListView`. No wrap/overflow policy on command rows or headings. | Scrim padding `24,60,24,24`; shell padding `12`; list padding `6`; several `10` px vertical gaps. | Raw `CoreStyle` + literal colors; no `FT66Style`. |

### 1.2 Supplementary: Slate-built `UUserWidget` / `UT66ScreenBase` wrappers

Most of the actual UI surface area is not implemented as standalone `SWidget` subclasses. It is implemented as `UUserWidget` / `UT66ScreenBase` classes that build Slate trees in `BuildSlateUI()` or `RebuildWidget()`.

`UT66ScreenBase` children that use the default shared responsive root (`FT66Style::MakeResponsiveRoot`) unless they override `RebuildWidget()`:

- `UT66AccountStatusScreen`
- `UT66AchievementsScreen`
- `UT66CompanionGridScreen`
- `UT66CompanionSelectionScreen`
- `UT66HeroGridScreen`
- `UT66HeroSelectionScreen`
- `UT66LanguageSelectScreen`
- `UT66LeaderboardScreen`
- `UT66LobbyBackConfirmModal`
- `UT66LobbyReadyCheckModal`
- `UT66LobbyScreen`
- `UT66PartySizePickerScreen`
- `UT66PauseMenuScreen`
- `UT66PlayerSummaryPickerScreen`
- `UT66PowerUpScreen`
- `UT66QuitConfirmationModal`
- `UT66ReportBugScreen`
- `UT66SaveSlotsScreen`
- `UT66SettingsScreen`
- `UT66UnlocksScreen`

`UT66ScreenBase` children that bypass or replace the shared responsive root:

- `UT66MainMenuScreen`: overrides `RebuildWidget()` and explicitly comments that the shared responsive root would double-scale the menu. Uses its own 1920x1080 reference math plus viewport-change-triggered rebuilds.
- `UT66FrontendTopBarWidget`: overrides `RebuildWidget()` and uses its own `GetTopBarLayoutScale()` based on effective viewport / `UGameUserSettings` resolution.
- `UT66RunSummaryScreen`: custom `RebuildWidget()`; uses screen-specific layout rather than plain inherited placeholder path.

Gameplay / overlay `UUserWidget` classes that build Slate directly in `RebuildWidget()`:

- `UT66GameplayHUDWidget`
- `UT66VendorOverlayWidget`
- `UT66GamblerOverlayWidget`
- `UT66CircusOverlayWidget`
- `UT66CasinoOverlayWidget`
- `UT66CollectorOverlayWidget`
- `UT66CrateOverlayWidget`
- `UT66LabOverlayWidget`
- `UT66WheelOverlayWidget`
- `UT66IdolAltarOverlayWidget`
- `UT66CowardicePromptWidget`
- `UT66LoadPreviewOverlayWidget`
- `UT66LoadingScreenWidget`
- `UT66EnemyLockWidget`
- `UT66EnemyHealthBarWidget`
- `UT66HeroCooldownBarWidget`
- `UT66FloatingCombatTextWidget`

Key architectural consequence: the repo has one shared helper (`FT66Style::MakeResponsiveRoot`) but several major widgets either bypass it entirely or wrap embedded child widgets differently depending on runtime context (`bEmbeddedInCasinoShell`, `bEmbeddedInCircusShell`, etc.). That inconsistency is a major contributor to the current viewport-specific tuning problem.

### 1.3 Text-constraining patterns observed in practice

- `FT66Style::MakeButton()` does **not** guarantee text safety. Its default text path wraps `STextBlock` in `SScaleBox(EStretch::ScaleToFit, DownOnly)`, so long labels shrink instead of forcing the button to grow, and there is no default `AutoWrapText`, `WrapTextAt`, or `OverflowPolicy`.
- The hero / companion skin rows are the cleanest direct clipping matches to your reported issues. Both `T66HeroSelectionScreen.cpp` and `T66CompanionSelectionScreen.cpp` use fixed `90x36` boxes for the `EQUIPPED` plate and a fixed-width `90x36` box for the stacked buy CTA (`BUY` + `250 AC`), with no wrap rules and tight inner padding.
- Vendor / gambler / casino card UIs usually do set `AutoWrapText(true)` on item names and descriptions, but they still live inside rigid card heights (`60`-high title rows, fixed icon sections, fixed CTA rows). That means the text is safer than the button cases, but the panel composition is still fragile.
- Leaderboard rows are one of the few places that explicitly use `ETextOverflowPolicy::Ellipsis`.
- Tooltips in `UT66GameplayHUDWidget` do use `AutoWrapText(true)` + `WrapTextAt(280.f)`, which is one of the better text-safety patterns currently in the repo.

## 2. Theme & Style System

### 2.1 `FT66Style`

Primary file set:

- `Source/T66/UI/Style/T66Style.h`
- `Source/T66/UI/Style/T66Style.cpp`

What it owns:

- Global Slate style set registration (`FSlateStyleSet`)
- Central button / panel / dropdown factories
- Design tokens (`FT66Style::Tokens`)
- Theme palette bridge
- Font selection and font cycling helpers
- Optional Dota plate / inventory-slot runtime texture loading
- Custom responsive wrapper (`SDPIScaler`)
- Deferred widget rebuild helper (`FT66Style::DeferRebuild`)

Important parameter structs:

- `FT66ButtonParams`
  - Defaults: `MinWidth = 120`, `Height = 0`, `FontSize = 0`, `FontWeight = "Bold"`, `Padding = FMargin(-1)`.
  - Supports per-state text colors, dual-tone text, shadow colors, explicit shadow offset, glow enable/disable, Dota plate overlay, custom child content.
- `FT66PanelParams`
  - Defaults: `Type = Panel`, `Padding = FMargin(16)`, optional border/background visual override and color override.
- `FT66DropdownParams`
  - Defaults: `MinWidth = 0`, `Height = 32`, `Padding = FMargin(8,4)`.

Important token fields that already exist but are only partially respected by calling code:

- Corner tokens: `CornerRadius = 10`, `CornerRadiusSmall = 8`.
- Generic spacing tokens: `Space2 = 8`, `Space3 = 12`, `Space4 = 16`, `Space5 = 20`, `Space6 = 24`, `Space8 = 32`.
- Button padding: `ButtonPadding = FMargin(12,4)`, `ButtonPaddingPressed = FMargin(12,5,12,3)`.
- Button glow tokens: padding `6`, hover intensity `0.75`, pressed intensity `1.10`, fallback opacity `0.28`.
- NPC/shop overlay tokens: inventory slot `160`, item panel icon `200`, game card min width `260`, NPC center width `920`, right panel width `380`, main row height `600`, inventory strip heights `180` / `252`, stats panel width `300`, stats content height `400`, shop card `248x500`, anger circle `170`, bank spinbox `110x44`, overlay padding `24`.
- Font token defaults in active Dota theme: title `56`, heading `26`, body `15`, small `12`, chip `12`, button `18`.

High-value audit findings inside `FT66Style` itself:

- The active theme is effectively hard-locked to Dota.
  - `FT66Style::SetActiveTheme()` resolves every request to `ET66UITheme::Dota`.
  - `FT66Style::GetActiveTheme()` returns `ET66UITheme::Dota`.
  - `FT66Style::IsDotaTheme()` always returns `true`.
- `FT66Style::CornerRadius()` and `FT66Style::CornerRadiusSmall()` both currently return `0.0f`, so the non-zero token corner constants are not the live values under the active theme.
- `FT66Style::MakeResponsiveRoot()` wraps content in `SDPIScaler` with a default reference resolution of `1920x1080` and custom scale clamp logic (`0.70 .. 1.35`, snap to `1.0` above `1600x900`). This is **not** the same as using UE's project DPI curve.
- `FT66Style::MakeButton()` centralizes buttons, but it still allows callers to force fixed heights / widths and uses `SScaleBox` for text instead of a content-first text safety policy.

### 2.2 `FT66DotaTheme` (full API inventory)

Primary file set:

- `Source/T66/UI/Dota/T66DotaTheme.h`
- `Source/T66/UI/Dota/T66DotaTheme.cpp`

Important note: `FT66DotaTheme` has **no data members / fields**. It is a pure static API that returns the live Dota palette and fonts. The full inventory is therefore the set of static functions below.

| API | Controls |
|---|---|
| `Background()` | Root background fill; very dark navy. |
| `PanelOuter()` | Outer shell border fill. |
| `Panel()` | Primary panel fill. |
| `PanelInner()` | Secondary / inner panel fill. |
| `Stroke()` | Panel/button highlight line color. |
| `Scrim()` | Modal / overlay darkener. |
| `Text()` | Primary text color. |
| `TextMuted()` | Secondary / muted text color. |
| `Accent()` | Blue accent color used for selection / highlights. |
| `Accent2()` | Gold accent color. |
| `Border()` | Standard border color. |
| `HeaderBar()` | Header strip fill. |
| `HeaderAccent()` | Header highlight accent. |
| `Success()` | Success / positive accent. |
| `Danger()` | Danger / destructive accent. |
| `ScreenBackground()` | Full screen background variant. |
| `ScreenText()` | Screen-level primary text alias. |
| `ScreenMuted()` | Screen-level muted text alias. |
| `SlotOuter()` | Inventory / slot outer frame color. |
| `SlotInner()` | Inventory / slot inner frame color. |
| `SlotFill()` | Inventory / slot background fill. |
| `BossBarBackground()` | Boss health bar background. |
| `BossBarFill()` | Boss health bar fill. |
| `PromptBackground()` | Prompt / pickup prompt background. |
| `SelectionFill()` | Selection highlight fill. |
| `MinimapBackground()` | Minimap background color. |
| `MinimapTerrain()` | Minimap terrain overlay color. |
| `MinimapGrid()` | Minimap grid line color. |
| `MinimapFriendly()` | Friendly marker color. |
| `MinimapEnemy()` | Enemy marker color. |
| `MinimapNeutral()` | Neutral marker color. |
| `ButtonNeutral()` | Neutral button fill. |
| `ButtonHovered()` | Neutral button hover fill. |
| `ButtonPressed()` | Neutral button pressed fill. |
| `ButtonPrimary()` | Primary/success button fill. |
| `ButtonPrimaryHovered()` | Primary hover fill. |
| `ButtonPrimaryPressed()` | Primary pressed fill. |
| `DangerButton()` | Danger button fill. |
| `DangerButtonHovered()` | Danger hover fill. |
| `DangerButtonPressed()` | Danger pressed fill. |
| `SuccessButton()` | Success button fill (aliases primary). |
| `SuccessButtonHovered()` | Success hover fill (aliases primary hover). |
| `SuccessButtonPressed()` | Success pressed fill (aliases primary pressed). |
| `ToggleButton()` | Toggle-active fill (aliases primary). |
| `ToggleButtonHovered()` | Toggle-active hover fill. |
| `ToggleButtonPressed()` | Toggle-active pressed fill. |
| `CornerRadius()` | Currently `0.0f`. |
| `CornerRadiusSmall()` | Currently `0.0f`. |
| `MakeFont(weight, size)` | Dota font resolver. Bold prefers `Reaver-Bold`; regular falls back to `radiance.ttf`. |

### 2.3 `FT66DotaSlate`

Primary file set:

- `Source/T66/UI/Dota/T66DotaSlate.h`
- `Source/T66/UI/Dota/T66DotaSlate.cpp`

What it does:

- Wraps content in layered Dota-style surfaces using nested `SBorder` / `SOverlay` shells.
- Provides `MakeScreenSurface`, `MakeViewportFrame`, `MakeViewportCutoutFrame`, `MakeSlotFrame`, `MakeHudPanel`, `MakeDivider`, `MakeMinimapFrame`.
- Loads optional button plate and inventory slot frame textures from either imported assets or runtime PNG fallbacks under `SourceAssets/UI/Dota/Generated`.

Notable hardcoded layout values inside the helper layer itself:

- Frame shell thicknesses use repeated `1` and `2` pixel borders/highlights.
- `MakeHudPanel()` title divider gap uses `6` / `8` pixels.
- `MakeMinimapFrame()` paints corner indicators using `10x10` boxes padded by `5`.
- Button plate nine-slice margins use fractional margins (`0.22`, `0.32`) for neutral/danger plates and `0` for the primary plate.

### 2.4 `FT66ButtonVisuals`

Primary file set:

- `Source/T66/UI/Style/T66ButtonVisuals.h`
- `Source/T66/UI/Style/T66ButtonVisuals.cpp`

What it does:

- Supplies decorative border / fill brush sets for `RetroSky`, `RetroWood`, `MainMenuBlueTrim`, `MainMenuArcane`, and `MainMenuBlueWood` visual variants.
- Uses imported assets first and file fallbacks second.
- Keeps fallback textures alive in `TStrongObjectPtr` caches.

Key details:

- Decorative border brushes are layout-neutral (`ImageSize = 1x1`) so they do not change desired size by themselves.
- Border brush sets use a typical decorative thickness of `12`.
- Horizontal trim resources are authored around `256x24`; vertical trims around `24x256`.

### 2.5 Fonts

Active font loading path in the live Dota theme:

- Regular text: `SourceAssets/radiance.ttf`, fallback `Content/Slate/Fonts/radiance.ttf`
- Bold text: prefers `SourceAssets/Reaver-Bold.woff`, then `Content/Slate/Fonts/Reaver-Bold.woff`, then `Content/Slate/Fonts/Reaver-Bold.ttf`

Dormant non-Dota font cycling system still exists in `FT66Style.cpp`:

- Caesar Dressing
- Cinzel
- Cormorant SC
- Germania One
- Grenze
- Alagard
- Ransom

That system is currently mostly inert because the runtime theme is forced to Dota.

### 2.6 Runtime texture / icon loading

Primary helper:

- `Source/T66/UI/T66SlateTextureHelpers.h`
- `Source/T66/UI/T66SlateTextureHelpers.cpp`

Rules implemented there:

- UI code should not call `LoadSynchronous()`.
- UI brushes should bind via `UT66UITexturePoolSubsystem`.
- `BindSharedBrushAsync()` and `BindBrushAsync()` update the Slate brush when the soft texture finishes loading.

Runtime PNG / file-texture caches found in the repo:

| File | Cache / mechanism | What it loads |
|---|---|---|
| `Source/T66/UI/T66FrontendTopBarWidget.cpp` | `TMap<FString, TStrongObjectPtr<UTexture2D>> GTopBarFileTextureCache` | Top bar generated PNGs and icons. |
| `Source/T66/UI/Screens/T66PowerUpScreen.cpp` | `TMap<FString, TStrongObjectPtr<UTexture2D>> GPowerUpFileTextureCache` | Power-up statue / screen PNGs. |
| `Source/T66/UI/Style/T66Style.cpp` | `FT66OptionalTextureBrush` caches | Dota button plates and inventory slot frame fallback PNGs. |
| `Source/T66/UI/Dota/T66DotaSlate.cpp` | `FT66OptionalTextureBrush` caches | Same Dota-style fallback plates / slot frame. |
| `Source/T66/UI/Style/T66ButtonVisuals.cpp` | many static `TStrongObjectPtr<UTexture2D>` fallback handles | Main menu trim/fill and retro trim textures. |
| `Source/T66/UI/T66GameplayHUDWidget.cpp` | rooted atlas texture + brush map | Runtime minimap symbol atlas and icon sub-UV brushes. |

## 3. Layout Patterns

### 3.1 How widgets enter the viewport

| Entry point | Widget(s) | Z-order |
|---|---|---:|
| `UT66UIManager::ShowScreen()` | Main frontend screens | `0` |
| `UT66UIManager::ShowModal()` | Frontend modal screens | `100` |
| `UT66UIManager::ShowFrontendTopBar()` | `UT66FrontendTopBarWidget` | `50` |
| `AT66PlayerController::ShowGameplayHUD()` | `UT66GameplayHUDWidget` | `0` |
| `AT66PlayerController::OpenGamblerOverlay()` | `UT66GamblerOverlayWidget` | `100` |
| `AT66PlayerController::OpenCircusOverlay()` | `UT66CircusOverlayWidget` | `100` |
| `AT66PlayerController::OpenVendorOverlay()` | `UT66VendorOverlayWidget` | `100` |
| `AT66PlayerController::OpenCollectorOverlay()` | `UT66CollectorOverlayWidget` | `100` |
| `AT66PlayerController_Combat` | `UT66IdolAltarOverlayWidget` | `150` |
| `AT66PlayerController::OpenCowardicePrompt()` | `UT66CowardicePromptWidget` | `200` |
| `AT66PlayerController::OpenLoadPreviewOverlay()` | `UT66LoadPreviewOverlayWidget` | `500` |
| `UT66GameInstance` | `UT66LoadingScreenWidget` | `9999` |
| `AT66GameMode` | gameplay loading overlay | `10000` |
| `UT66GameInstance` / `AT66PlayerController` | black curtain via `AddViewportWidgetContent` | `9500` / `9999` |
| `AT66PlayerController_Frontend` | non-shipping dev console | `20000` |

### 3.2 Top bar structure (`UT66FrontendTopBarWidget`)

File: `Source/T66/UI/T66FrontendTopBarWidget.cpp`

Structure:

- Root is a `SVerticalBox`.
- First slot is `AutoHeight()` and reserves a top strip using `SBox.HeightOverride(GetReservedHeight())`.
- Inside that slot is an `SOverlay` containing the top bar surface and its `SHorizontalBox` row.
- The bar is split into three clusters:
  - left utility cluster: settings + language
  - center nav cluster: account status + home + power up + achievements
  - right utility cluster: power coupons + achievement coins + quit
- The second root slot is filler (`FillHeight(1)`) so the widget reserves vertical space without manually anchoring via canvas slots.

Important findings:

- The top bar does not use the shared `SDPIScaler` root.
- It computes `LayoutScale` itself from effective viewport size and `UGameUserSettings::GetScreenResolution()`.
- It rebuilds on viewport changes in `NativeTick()`.
- It uses many explicit sizes: button `92x78`, reserved height `146`, nav heights `72/80`, nav min widths `176/188`, utility slot `78x78`, icons `44x44`, `48x48`, `58x38`, etc.

### 3.3 `SDPIScaler`, `SScaleBox`, `SConstraintCanvas`

- Actual `SDPIScaler` usage exists only in `FT66Style::MakeResponsiveRoot()`.
- `SScaleBox` usage is limited to:
  - `FT66Style::MakeButton()` text content (both single-tone and dual-tone paths)
  - `UT66MainMenuScreen` center menu button group scaling block
- `SConstraintCanvas` is included but there is no actual `SNew(SConstraintCanvas)` construction in the audited files.

Implication: the project is not broadly using Slate containers that naturally preserve aspect or anchoring. Most layout is produced by nested `SOverlay`, `SVerticalBox`, `SHorizontalBox`, `SUniformGridPanel`, and explicit `SBox` size overrides.

### 3.4 Desired size overrides / custom measurement

No custom `GetDesiredSize()` overrides were found.

Custom `ComputeDesiredSize()` overrides exist in:

- `ST66TopBarGearGlyph`
- `ST66TopBarPowerGlyph`
- `ST66TopBarBadgeBackground`
- `ST66EnemyBullseyeWidget`
- `ST66LockDotWidget`
- `ST66RingWidget`
- `ST66DotWidget`
- `ST66CrosshairWidget`
- `ST66ScopedSniperWidget`
- `ST66WorldMapWidget`
- `ST66PowerUpStatueFillWidget`

### 3.5 Slot sizing patterns in use

Current dominant patterns:

- `AutoWidth()` / `AutoHeight()` are extremely common for rows, cards, text stacks, and top-level sections.
- `FillWidth()` / `FillHeight()` are used selectively for major panel columns, scroll areas, and some HUD sections.
- `SBox.WidthOverride()` / `HeightOverride()` / `MinDesiredWidth()` are pervasive and are frequently mixed with `AutoWidth()` in the same widget tree.
- There is no unified "content first, min/max second" sizing discipline. Many widgets still define a fixed outer box first and then try to fit text into it.

### 3.6 Definite 1080p failure candidates

These are not hypothetical; they are already visible in code:

- `T66HeroSelectionScreen.cpp` and `T66CompanionSelectionScreen.cpp`
  - `EQUIPPED` lives in fixed `90x36` shells with `Padding(4,2)` and no wrap/overflow.
  - Buy CTAs stack `BUY` and price inside the same fixed-width shell.
- `T66VendorOverlayWidget.cpp`
  - Shop cards are fixed at `248x500`; title region is fixed `60` high; sell panel is fixed `160x160`; steal prompt is fixed `560x220` with a `360x28` timing bar.
- `T66GamblerOverlayWidget.cpp`
  - Game cards and right-panel portrait/bank section use rigid sizes similar to vendor, plus card art sizes `260`, coin sprites `180`, RPS hands `160`, blackjack back `80x112`.
- `T66MainMenuScreen.cpp`
  - Hard-coded 1920x1080 background assumptions, explicit center column width/height, and custom responsive math separate from the rest of the UI stack.
- `T66FrontendTopBarWidget.cpp`
  - Full custom viewport scaling plus a dense set of fixed button / icon / nav dimensions.
- `T66SettingsScreen.cpp`
  - Modal shell sizes itself to `80%` of the **primary display**, not the current viewport, so its behavior differs from both PIE viewport sizing and in-game viewport sizing.

## 4. Current DPI / Resolution Settings

### 4.1 Config files

`Config/DefaultEngine.ini` contains only:

```ini
[/Script/Engine.UserInterfaceSettings]
bAuthorizeAutomaticWidgetVariableCreation=False
FontDPIPreset=Standard
FontDPI=72
```

Findings:

- No `DPIScaleRule`
- No `UIScaleCurve`
- No `DesignScreenSize`
- No explicit shortest-side DPI curve config

`Config/DefaultGame.ini` does not define any UI DPI or design resolution settings.

### 4.2 C++ DPI / scale code

Findings:

- No usage of `GetDefault<UUserInterfaceSettings>()` was found.
- No manual `SetApplicationScale()` / `GetApplicationScale()` calls were found.
- No project code is configuring UE's built-in DPI curve in C++.

Actual scaling that is happening today:

- `UT66ScreenBase::RebuildWidget()` returns `FT66Style::MakeResponsiveRoot(BuildSlateUI())`.
- `FT66Style::MakeResponsiveRoot()` wraps content in `SDPIScaler` and uses `FT66Style::GetViewportResponsiveScale(ReferenceResolution=1920x1080)`.
- `ComputeResponsiveScale()` uses `min(viewport/reference)` with clamp `0.70 .. 1.35` and snaps to `1.0` above `1600x900` if the computed scale is below `1.0`.
- `UT66MainMenuScreen` bypasses that system and computes its own layout scale from effective viewport size.
- `UT66FrontendTopBarWidget` bypasses it too and computes a separate `sqrt(widthScale * heightScale)` scale with clamp `0.70 .. 3.0`.
- `UT66SettingsScreen` modal layout uses `FSlateApplication::Get().GetCachedDisplayMetrics()` and sizes itself to 80% of the monitor, not 80% of the viewport.

### 4.3 Editor / PIE resolution clues

No project config entry storing a canonical PIE viewport size was found in the repo.

There is, however, a direct code comment in `Source/T66/Gameplay/T66PlayerController.cpp` explaining that UI initialization is delayed briefly so the PIE viewport can stabilize its resolution and avoid a "zoomed-in" flash. That comment is strong evidence that current UI behavior is already sensitive to transient viewport size during startup.

## 5. Inventory of All Hardcoded Sizes

Method used:

- Automated pass over `Source/T66/UI/**/*.cpp`, `Source/T66/UI/**/*.h`, and the frontend dev-console code in `Source/T66/Gameplay/T66PlayerController_Frontend.cpp`.
- Captured size-related literals from explicit widget APIs and constants, including `WidthOverride`, `HeightOverride`, `MinDesiredWidth`, `MinDesiredHeight`, `DesiredSizeOverride`, `WrapTextAt`, `Padding`, `SetPadding`, `ContentPadding`, `SlotPadding`, `ImageSize`, `ShadowOffset`, `RenderTransform` offsets, `ComputeDesiredSize()` returns, font sizes, letter spacing, and size-related `const` / `constexpr` declarations.
- Result count: `2342` unique size-related entries.

Important caveat:

- This appendix is intentionally mechanical. It is meant to answer "where are the hardcoded size assumptions?" rather than "which ones are good or bad?". Theme token definitions appear here too, because they are size literals that will matter during refactoring.

### 5.1 Full file/line inventory

| File | Line | Widget | Property | Value | Notes |
|---|---:|---|---|---|---|
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 146 | SBorder | Padding | FMargin(24.f, 60.f, 24.f, 24.f | .Padding(FMargin(24.f, 60.f, 24.f, 24.f)) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 151 | SBorder | Padding | 12.f | .Padding(12.f) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 165 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | .Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 174 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | .Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 185 | SEditableTextBox | Padding | 0.f, 10.f, 0.f, 0.f | .Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 194 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | .Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/Gameplay/T66PlayerController_Frontend.cpp | 199 | SBorder | Padding | 6.f | .Padding(6.f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 32 | Constant | RankColumnWidth | 76.0f | constexpr float RankColumnWidth = 76.0f;   // content + padding to name |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 33 | Constant | ScoreColumnWidth | 128.0f | constexpr float ScoreColumnWidth = 128.0f; // content + padding, aligned with score digits (used for both Score and Time) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 34 | Constant | RowPortraitSize | 36.0f | constexpr float RowPortraitSize = 36.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 35 | Constant | LeaderboardBodyFontSize | 16 | constexpr int32 LeaderboardBodyFontSize = 16; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 36 | Constant | LeaderboardTitleFontSize | 16 | constexpr int32 LeaderboardTitleFontSize = 16; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 299 | Unknown | ImageSize | 32.0f, 32.0f | DefaultAvatarBrush->ImageSize = FVector2D(32.0f, 32.0f); |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 304 | Constant | SquareIconSize | 52.0f | const float SquareIconSize = 52.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 305 | Constant | FilterButtonGap | 8.0f | const float FilterButtonGap = 8.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 306 | Constant | DropdownHeight | 46.0f | const float DropdownHeight = 46.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 307 | Constant | PartyDropdownMinWidth | 120.0f | const float PartyDropdownMinWidth = 120.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 308 | Constant | DifficultyDropdownMinWidth | 136.0f | const float DifficultyDropdownMinWidth = 136.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 309 | Constant | TypeDropdownMinWidth | 112.0f | const float TypeDropdownMinWidth = 112.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 310 | Constant | StageDropdownMinWidth | 84.0f | const float StageDropdownMinWidth = 84.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 327 | Unknown | ImageSize | 48.0, 48.0 | Brush->ImageSize = FVector2D(48.0, 48.0); |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 353 | Unknown | LetterSpacing | 0 | auto MakeRadianceFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 359 | Unknown | LetterSpacing | 0 | auto MakeReaverBoldFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 380 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 409 | SBorder | Padding | 2.0f | .Padding(2.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 414 | SBorder | Padding | 4.0f | .Padding(4.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 425 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 431 | SBorder | Padding | FMargin(24.f, 12.f, 24.f, 10.f | .Padding(FMargin(24.f, 12.f, 24.f, 10.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 464 | SBox | MinDesiredWidth | 160.f | .MinDesiredWidth(160.f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 483 | SBorder | Padding | FMargin(0.f | .Padding(FMargin(0.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 495 | SComboButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 502 | SBorder | Padding | FMargin(10.f, 9.f, 8.f, 7.f | .Padding(FMargin(10.f, 9.f, 8.f, 7.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 513 | SHorizontalBox | Padding | 8.0f, 0.0f, 0.0f, 0.0f | .Padding(8.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 535 | SHorizontalBox | Padding | 0.0f | .Padding(0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 542 | SVerticalBox | Padding | 0.0f, FilterButtonGap, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, FilterButtonGap, 0.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 546 | SVerticalBox | Padding | 0.0f, FilterButtonGap, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, FilterButtonGap, 0.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 558 | SBorder | Padding | FMargin(18.f, 16.f, 18.f, 16.f | .Padding(FMargin(18.f, 16.f, 18.f, 16.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 565 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | .Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 579 | SVerticalBox | SetMinWidth | 160.f | .SetMinWidth(160.f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 581 | SVerticalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 596 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 599 | SHorizontalBox | Padding | 6.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 601 | SHorizontalBox | Padding | 6.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 608 | SHorizontalBox | Padding | 0.0f, 0.0f, 0.0f, 18.0f | .Padding(0.0f, 0.0f, 0.0f, 18.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 612 | SHorizontalBox | Padding | 0.0f, 0.0f, 6.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(0.95f).Padding(0.0f, 0.0f, 6.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 634 | SVerticalBox | SetMinWidth | 0.f | }), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular"))) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 642 | SVerticalBox | Padding | 0.0f, 0.0f, 6.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(1.10f).Padding(0.0f, 0.0f, 6.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 664 | SVerticalBox | SetMinWidth | 0.f | }), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular"))) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 694 | SVerticalBox | SetMinWidth | 0.f | }), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular"))) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 702 | SVerticalBox | Padding | 4.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 724 | SVerticalBox | SetMinWidth | 0.f | }), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular"))) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 736 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 4.0f | .Padding(0.0f, 0.0f, 0.0f, 4.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 740 | SBorder | Padding | FMargin(10.0f, 6.0f | .Padding(FMargin(10.0f, 6.0f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 783 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 838 | SVerticalBox | Padding | 0.f, 10.f, 0.f, 0.f | .Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 873 | Constant | MemberPortraitSize | FMath::RoundToFloat(NameFontSize + 6.0f) | const float MemberPortraitSize = FMath::RoundToFloat(NameFontSize + 6.0f); |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 874 | Constant | PartyStripMinHeight | FMath::Max(MemberPortraitSize, NameFontSize) + 2.0f | const float PartyStripMinHeight = FMath::Max(MemberPortraitSize, NameFontSize) + 2.0f; |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 875 | Constant | RowVerticalPadding | FMath::Max(4.0f, FMath::RoundToFloat(NameFontSize * 0.25f)) | const float RowVerticalPadding = FMath::Max(4.0f, FMath::RoundToFloat(NameFontSize * 0.25f)); |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 901 | SHorizontalBox | Padding | NameIndex > 0 ? FMargin(12.0f, 0.0f, 0.0f, 0.0f | .Padding(NameIndex > 0 ? FMargin(12.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 919 | SImage | Padding | 6.0f, 0.0f, 0.0f, 0.0f | .Padding(6.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 957 | SBorder | Padding | FMargin(10.0f, RowVerticalPadding | .Padding(FMargin(10.0f, RowVerticalPadding)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 1000 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 1016 | ST66LeaderboardRowWheelProxy | Padding | 0.0f, 1.0f | .Padding(0.0f, 1.0f) |
| Source/T66/UI/Components/T66LeaderboardPanel.cpp | 1656 | ST66LeaderboardRowWheelProxy | ImageSize | 32.0f, 32.0f | Brush->ImageSize = FVector2D(32.0f, 32.0f); |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 45 | Unknown | ImageSize | 1.f, 1.f | Brush.ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 175 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 180 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 195 | SBorder | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 198 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 207 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 210 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 262 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 267 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 293 | SImage | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 296 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 305 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 308 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 339 | STextBlock | Padding | 0.f, 6.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 383 | SOverlay | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 385 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 385 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 395 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 397 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 397 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 407 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 409 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 409 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 419 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 421 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaSlate.cpp | 421 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Dota/T66DotaTheme.cpp | 315 | Unknown | FontSize | 32 | FSlateFontInfo FT66DotaTheme::MakeFont(const TCHAR* Weight, int32 Size) |
| Source/T66/UI/Dota/T66DotaTheme.h | 63 | Unknown | FontSize | 32 | static FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size); |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 101 | Constant | TopInset | bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) | const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f); |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 106 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 110 | STextBlock | FontSize | 42 | .Font(FT66Style::Tokens::FontBold(42)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 114 | STextBlock | Padding | 0.f, 0.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 118 | STextBlock | FontSize | 30 | .Font(FT66Style::Tokens::FontBold(30)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 122 | STextBlock | Padding | 0.f, 0.f, 0.f, bShowReason ? 12.f : 26.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, bShowReason ? 12.f : 26.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 130 | STextBlock | Padding | 0.f, 0.f, 0.f, 26.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 26.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 137 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 156 | STextBlock | SetPadding | FT66Style::Tokens::Space3 | .SetPadding(FT66Style::Tokens::Space3) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 167 | SBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 185 | SBorder | Padding | FMargin(60.f | .Padding(FMargin(60.f)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 189 | SBox | WidthOverride | 780.f | .WidthOverride(780.f) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 193 | SBox | Padding | FMargin(34.f, 28.f, 34.f, 30.f | .Padding(FMargin(34.f, 28.f, 34.f, 30.f)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 209 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66AccountStatusScreen.cpp | 214 | SBox | Padding | FMargin(34.f, 28.f, 34.f, 30.f | .Padding(FMargin(34.f, 28.f, 34.f, 30.f)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 166 | Constant | TopInset | UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f | const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f; |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 173 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 182 | SVerticalBox | Padding | 30.f, 25.f, 30.f, 20.f | .Padding(30.f, 25.f, 30.f, 20.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 195 | STextBlock | FontSize | 50 | .Font(FT66Style::Tokens::FontBold(50)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 201 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | .Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 207 | SVerticalBox | Padding | 14.f, 0.f, 14.f, 10.f | .Padding(14.f, 0.f, 14.f, 10.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 221 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 236 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 254 | SProgressBar | SetPadding | FMargin(14.f, 14.f | .SetPadding(FMargin(14.f, 14.f))) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 259 | SProgressBar | Padding | 30.f, 0.f, 30.f, 20.f | .Padding(30.f, 0.f, 30.f, 20.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 271 | SVerticalBox | Padding | 20.f, 0.f, 0.f, 20.f | .Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 281 | SBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 282 | SBox | SetFontSize | 20 | .SetFontSize(20)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 324 | SBox | Padding | 0.f, 0.f, 0.f, 6.f | .Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 328 | SBorder | Padding | FMargin(20.f, 14.f | .Padding(FMargin(20.f, 14.f)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 341 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 346 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | .Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 350 | STextBlock | FontSize | 19 | .Font(FT66Style::Tokens::FontRegular(19)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 362 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 388 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 394 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | .Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 401 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66AchievementsScreen.cpp | 402 | STextBlock | SetFontSize | 20 | .SetFontSize(20) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 72 | Constant | Columns | FMath::Max(1, FMath::Min(3, IDsWithNone.Num())) | const int32 Columns = FMath::Max(1, FMath::Min(3, IDsWithNone.Num())); |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 73 | Constant | Rows | IDsWithNone.Num() > 0 ? (IDsWithNone.Num() + Columns - 1) / Columns : 0 | const int32 Rows = IDsWithNone.Num() > 0 ? (IDsWithNone.Num() + Columns - 1) / Columns : 0; |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 115 | SGridPanel | ImageSize | 256.f, 256.f | PortraitBrush->ImageSize = FVector2D(256.f, 256.f); |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 131 | SGridPanel | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 135 | SGridPanel | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 163 | SSpacer | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 195 | SBorder | Padding | FMargin(30.0f, 25.0f | .Padding(FMargin(30.0f, 25.0f)) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 201 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | .Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 205 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 221 | SScrollBox | Padding | 0.0f, 20.0f, 0.0f, 0.0f | .Padding(0.0f, 20.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionGridScreen.cpp | 225 | SScrollBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 226 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontRegular(16)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 232 | STextBlock | Padding | 5.0f, 0.0f | Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 234 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 234 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 252 | SWidgetSwitcher | SetMinWidth | 0.f | ET66ButtonType::Primary).SetMinWidth(0.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 256 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 256 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 264 | SBorder | Padding | FMargin(4.0f, 2.0f | .Padding(FMargin(4.0f, 2.0f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 278 | STextBlock | Padding | 5.0f, 0.0f | Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 287 | STextBlock | SetMinWidth | 80.f | ET66ButtonType::Neutral).SetMinWidth(80.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 289 | STextBlock | Padding | 4.0f, 0.0f | Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 291 | SBox | HeightOverride | 36.0f | SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 291 | SBox | MinDesiredWidth | 90.0f | SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 309 | SWidgetSwitcher | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 314 | STextBlock | FontSize | 10 | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BeachgoerPriceText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::Text) ] |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 331 | STextBlock | SetMinWidth | 0.f | ET66ButtonType::Primary).SetMinWidth(0.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 335 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 335 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 343 | SBorder | Padding | FMargin(4.0f, 2.0f | .Padding(FMargin(4.0f, 2.0f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 357 | STextBlock | Padding | 0.0f, 6.0f | .Padding(0.0f, 6.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 365 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3 | .SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 421 | SVerticalBox | ImageSize | 60.f, 60.f | CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f); |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 430 | Constant | SlotIdx | Offset + 2 | const int32 SlotIdx = Offset + 2; |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 444 | Constant | BoxSize | (Offset == 0) ? 60.0f : 45.0f | const float BoxSize = (Offset == 0) ? 60.0f : 45.0f; |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 490 | SImage | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 546 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 549 | SBox | HeightOverride | 56.f | .HeightOverride(56.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 556 | SBox | Padding | 0.0f | .Padding(0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 561 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 0.0f | .Padding(0.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 567 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 573 | SBox | Padding | 10.0f, 0.0f, 0.0f, 0.0f | .Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 580 | SBox | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 583 | SBox | HeightOverride | 86.f | .HeightOverride(86.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 598 | SBorder | Padding | 0 | .Padding(0) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 615 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 626 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 630 | SHorizontalBox | SetMinWidth | 180.f | ET66ButtonType::Neutral).SetMinWidth(180.f).SetPadding(FMargin(16.f, 10.f))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 630 | SHorizontalBox | SetPadding | FMargin(16.f, 10.f | ET66ButtonType::Neutral).SetMinWidth(180.f).SetPadding(FMargin(16.f, 10.f))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 632 | SHorizontalBox | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 636 | SHorizontalBox | SetFontSize | 22 | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 636 | SHorizontalBox | SetHeight | 48.f | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 636 | SHorizontalBox | SetMinWidth | 52.f | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 642 | SHorizontalBox | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 646 | SHorizontalBox | SetFontSize | 22 | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 646 | SHorizontalBox | SetHeight | 48.f | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 646 | SHorizontalBox | SetMinWidth | 52.f | ET66ButtonType::Neutral).SetMinWidth(52.f).SetHeight(48.f).SetFontSize(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 649 | SHorizontalBox | Padding | 20.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 654 | SHorizontalBox | SetMinWidth | 180.f | .SetMinWidth(180.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 655 | SHorizontalBox | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 666 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 670 | SHorizontalBox | SetMinWidth | 160.f | ET66ButtonType::Neutral).SetMinWidth(160.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 670 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(160.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 672 | SHorizontalBox | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 676 | SHorizontalBox | SetFontSize | 20 | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 676 | SHorizontalBox | SetHeight | 40.f | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 676 | SHorizontalBox | SetMinWidth | 40.f | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 682 | SHorizontalBox | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 686 | SHorizontalBox | SetFontSize | 20 | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 686 | SHorizontalBox | SetHeight | 40.f | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 686 | SHorizontalBox | SetMinWidth | 40.f | ET66ButtonType::Neutral).SetMinWidth(40.f).SetHeight(40.f).SetFontSize(20)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 689 | SHorizontalBox | Padding | 20.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 694 | SHorizontalBox | SetMinWidth | 160.f | .SetMinWidth(160.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 695 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 702 | SHorizontalBox | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 709 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 80.0f | .Padding(0.0f, 0.0f, 10.0f, 80.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 716 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 729 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 733 | STextBlock | SetPadding | FMargin(15.0f, 8.0f | .SetPadding(FMargin(15.0f, 8.0f))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 743 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3 | .SetPadding(FMargin(FT66Style::Tokens::Space3))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 748 | STextBlock | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 769 | SBox | Padding | 10.0f, 0.0f, 0.0f, 0.0f | .Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 785 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 794 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 809 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2 | .SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 814 | STextBlock | Padding | 8.0f, 0.0f, 0.0f, 0.0f | .Padding(8.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 819 | STextBlock | SetMinWidth | 110.f | .SetMinWidth(110.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 825 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 829 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 835 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 842 | SBorder | Padding | FMargin(10.0f | .Padding(FMargin(10.0f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 846 | STextBlock | FontSize | 11 | .Font(FT66Style::Tokens::FontRegular(11)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 856 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 867 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 873 | STextBlock | Padding | 0.f, 6.f | .Padding(0.f, 6.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 876 | SBox | WidthOverride | 240.f | .WidthOverride(240.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 877 | SBox | HeightOverride | 14.f | .HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 888 | SProgressBar | Padding | FMargin(240.f * 0.25f - 1.f, 0.f, 0.f, 0.f | .Padding(FMargin(240.f * 0.25f - 1.f, 0.f, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 890 | SBox | HeightOverride | 14.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 890 | SBox | WidthOverride | 2.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 899 | SBorder | Padding | FMargin(240.f * 0.50f - 1.f, 0.f, 0.f, 0.f | .Padding(FMargin(240.f * 0.50f - 1.f, 0.f, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 901 | SBox | HeightOverride | 14.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 901 | SBox | WidthOverride | 2.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 911 | SBox | HeightOverride | 14.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 911 | SBox | WidthOverride | 2.f | SNew(SBox).WidthOverride(2.f).HeightOverride(14.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 926 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 943 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 950 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 971 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2 | .SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 976 | STextBlock | Padding | 8.0f, 0.0f, 0.0f, 0.0f | .Padding(8.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 980 | STextBlock | SetMinWidth | 110.f | ET66ButtonType::Neutral).SetMinWidth(110.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 994 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1004 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space4 | .SetPadding(FMargin(FT66Style::Tokens::Space4))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1011 | STextBlock | Padding | 20.0f, 10.0f, 20.0f, 20.0f | .Padding(20.0f, 10.0f, 20.0f, 20.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1018 | STextBlock | SetMinWidth | 320.f | .SetMinWidth(320.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1025 | STextBlock | SetPadding | FMargin(16.f, 14.f | .SetPadding(FMargin(16.f, 14.f)))) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1029 | STextBlock | SetMinWidth | 240.f | .SetMinWidth(240.f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1040 | STextBlock | Padding | 20.0f, 0.0f, 0.0f, 20.0f | .Padding(20.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1044 | STextBlock | SetMinWidth | 120.f | ET66ButtonType::Neutral).SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1238 | SBox | ImageSize | 60.f, 60.f | CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f); |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1247 | Constant | SlotIdx | Offset + 2 | const int32 SlotIdx = Offset + 2; |
| Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp | 1263 | Constant | BoxSize | (Offset == 0) ? 60.f : 45.f | const float BoxSize = (Offset == 0) ? 60.f : 45.f; |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 74 | Constant | Columns | FMath::Max(1, FMath::Min(3, AllHeroIDs.Num())) | const int32 Columns = FMath::Max(1, FMath::Min(3, AllHeroIDs.Num())); |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 75 | Constant | Rows | AllHeroIDs.Num() > 0 ? (AllHeroIDs.Num() + Columns - 1) / Columns : 0 | const int32 Rows = AllHeroIDs.Num() > 0 ? (AllHeroIDs.Num() + Columns - 1) / Columns : 0; |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 97 | SGridPanel | ImageSize | 256.f, 256.f | PortraitBrush->ImageSize = FVector2D(256.f, 256.f); |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 107 | SGridPanel | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 110 | SGridPanel | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 135 | SSpacer | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 169 | SBorder | Padding | FMargin(30.0f, 25.0f | .Padding(FMargin(30.0f, 25.0f)) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 175 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | .Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 179 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 195 | SScrollBox | Padding | 0.0f, 20.0f, 0.0f, 0.0f | .Padding(0.0f, 20.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroGridScreen.cpp | 198 | SScrollBox | SetMinWidth | 120.f | .SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 220 | SHorizontalBox | Padding | 4.0f, 0.0f | ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 222 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 222 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 251 | SWidgetSwitcher | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 257 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 257 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 265 | SBorder | Padding | FMargin(4.0f, 2.0f | .Padding(FMargin(4.0f, 2.0f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 279 | STextBlock | Padding | 4.0f, 0.0f | ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 296 | STextBlock | SetMinWidth | 80.f | .SetMinWidth(80.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 300 | STextBlock | Padding | 4.0f, 0.0f | ButtonRow->AddSlot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 302 | SBox | HeightOverride | 36.0f | SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 302 | SBox | MinDesiredWidth | 90.0f | SNew(SBox).MinDesiredWidth(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 328 | SWidgetSwitcher | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 333 | STextBlock | FontSize | 10 | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(PriceText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::Text) ] |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 360 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 366 | SBox | HeightOverride | 36.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 366 | SBox | WidthOverride | 90.0f | SNew(SBox).WidthOverride(90.0f).HeightOverride(36.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 374 | SBorder | Padding | FMargin(4.0f, 2.0f | .Padding(FMargin(4.0f, 2.0f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 389 | STextBlock | Padding | 0.0f, 6.0f | .Padding(0.0f, 6.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 399 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontRegular(16)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 405 | STextBlock | Padding | 5.0f, 0.0f, 0.0f, 0.0f | .Padding(5.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 413 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3 | .SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 580 | SHorizontalBox | ImageSize | 60.f, 60.f | HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f); |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 590 | Constant | BoxSize | (Offset == 0) ? 60.0f : 45.0f | const float BoxSize = (Offset == 0) ? 60.0f : 45.0f; // Current hero is larger |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 592 | Constant | SlotIdx | Offset + 2 | const int32 SlotIdx = Offset + 2; |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 644 | SImage | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 678 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 681 | SBox | HeightOverride | 56.f | .HeightOverride(56.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 688 | SBox | Padding | 0.0f | .Padding(0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 693 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 0.0f | .Padding(0.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 699 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 705 | SBox | Padding | 10.0f, 0.0f, 0.0f, 0.0f | .Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 712 | SBox | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 715 | SBox | HeightOverride | 86.f | .HeightOverride(86.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 730 | SBorder | Padding | 0 | .Padding(0) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 747 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 758 | SHorizontalBox | Padding | 10.0f, 0.0f, 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 763 | SHorizontalBox | SetMinWidth | 176.f | .SetMinWidth(176.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 764 | SHorizontalBox | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 768 | STextBlock | Padding | 0.0f, 0.0f, 12.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 12.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 774 | STextBlock | SetHeight | 48.f | .SetMinWidth(52.f).SetHeight(48.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 774 | STextBlock | SetMinWidth | 52.f | .SetMinWidth(52.f).SetHeight(48.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 775 | STextBlock | SetFontSize | 22 | .SetFontSize(22) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 782 | STextBlock | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 788 | STextBlock | SetHeight | 48.f | .SetMinWidth(52.f).SetHeight(48.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 788 | STextBlock | SetMinWidth | 52.f | .SetMinWidth(52.f).SetHeight(48.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 789 | STextBlock | SetFontSize | 22 | .SetFontSize(22) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 801 | SHorizontalBox | Padding | 10.0f, 0.0f, 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.0f, 0.0f, 10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 806 | SHorizontalBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 807 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 811 | STextBlock | Padding | 0.0f, 0.0f, 12.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 12.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 817 | STextBlock | SetHeight | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 817 | STextBlock | SetMinWidth | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 818 | STextBlock | SetFontSize | 20 | .SetFontSize(20) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 825 | STextBlock | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 831 | STextBlock | SetHeight | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 831 | STextBlock | SetMinWidth | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 832 | STextBlock | SetFontSize | 20 | .SetFontSize(20) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 840 | STextBlock | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 847 | SHorizontalBox | Padding | 0.0f, 0.0f, 10.0f, 80.0f | .Padding(0.0f, 0.0f, 10.0f, 80.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 852 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 857 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 867 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 880 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 884 | STextBlock | SetPadding | FMargin(15.0f, 8.0f | .SetPadding(FMargin(15.0f, 8.0f))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 895 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3 | .SetPadding(FMargin(FT66Style::Tokens::Space3))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 901 | STextBlock | Padding | 10.0f, 0.0f, 10.0f, 80.0f | .Padding(10.0f, 0.0f, 10.0f, 80.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 928 | SBox | Padding | 0.0f, -72.0f, 0.0f, 0.0f | .Padding(0.0f, -72.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 940 | SHorizontalBox | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 946 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 957 | STextBlock | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 963 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 976 | STextBlock | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 985 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 991 | SHorizontalBox | SetPadding | FMargin(16.f, 14.f | .SetPadding(FMargin(16.f, 14.f)))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1000 | SHorizontalBox | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1006 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1017 | STextBlock | Padding | 4.0f, 0.0f | .Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1023 | STextBlock | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1036 | STextBlock | Padding | 0.0f, 8.0f, 0.0f, 0.0f | .Padding(0.0f, 8.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1041 | STextBlock | SetMinWidth | 240.f | .SetMinWidth(240.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1051 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 80.0f | .Padding(10.0f, 0.0f, 0.0f, 80.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1067 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1075 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1095 | STextBlock | Padding | 8.0f, 0.0f, 0.0f, 0.0f | .Padding(8.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1100 | STextBlock | SetMinWidth | 110.f | .SetMinWidth(110.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1108 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1111 | SBox | HeightOverride | 360.0f | SNew(SBox).HeightOverride(360.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1131 | STextBlock | SetPadding | FMargin(5.0f | .SetPadding(FMargin(5.0f))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1136 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 0.0f | .Padding(0.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1143 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1156 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1163 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1185 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2 | .SetPadding(FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1190 | STextBlock | Padding | 8.0f, 0.0f, 0.0f, 0.0f | .Padding(8.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1195 | STextBlock | SetMinWidth | 110.f | .SetMinWidth(110.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1208 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1218 | STextBlock | Padding | 0.0f, 10.0f, 0.0f, 0.0f | .Padding(0.0f, 10.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1224 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 0.0f | .Padding(0.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1229 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1230 | SVerticalBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1231 | SVerticalBox | SetFontSize | 12 | .SetFontSize(12) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1238 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 0.0f | .Padding(0.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1244 | SHorizontalBox | Padding | 0.0f, 0.0f, 8.0f, 0.0f | .Padding(0.0f, 0.0f, 8.0f, 0.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1253 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1269 | SVerticalBox | SetMinWidth | 0.f | }), ET66ButtonType::Neutral).SetMinWidth(0.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1273 | SVerticalBox | SetHeight | 40.0f | }).SetMinWidth(0.f).SetHeight(40.0f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1273 | SVerticalBox | SetMinWidth | 0.f | }).SetMinWidth(0.f).SetHeight(40.0f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1281 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1282 | SVerticalBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1283 | SVerticalBox | SetFontSize | 12 | .SetFontSize(12) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1292 | SVerticalBox | SetPadding | FMargin(FT66Style::Tokens::Space4 | .SetPadding(FMargin(FT66Style::Tokens::Space4))) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1300 | SVerticalBox | Padding | 20.0f, 0.0f, 0.0f, 20.0f | .Padding(20.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1305 | SVerticalBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1517 | SVerticalBox | ImageSize | 60.f, 60.f | HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(60.f, 60.f); |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1526 | Constant | SlotIdx | Offset + 2 | const int32 SlotIdx = Offset + 2; |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1547 | Constant | BoxSize | (Offset == 0) ? 60.f : 45.f | const float BoxSize = (Offset == 0) ? 60.f : 45.f; |
| Source/T66/UI/Screens/T66HeroSelectionScreen.cpp | 1890 | SBox | ImageSize | 640.0f, 360.0f | HeroPreviewVideoBrush->ImageSize = FVector2D(640.0f, 360.0f); |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 107 | SVerticalBox | Padding | 10.0f, 5.0f | .Padding(10.0f, 5.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 110 | SBox | WidthOverride | 400.0f | .WidthOverride(400.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 111 | SBox | HeightOverride | 60.0f | .HeightOverride(60.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 122 | SBorder | Padding | FMargin(0.0f | .Padding(FMargin(0.0f)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 126 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 140 | Constant | ScreenPadding | 60.0f | const float ScreenPadding = 60.0f; |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 142 | Constant | TopInset | bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) | const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f); |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 151 | SScrollBox | Padding | FMargin(0.0f, 0.0f, 10.0f, 0.0f | .Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 164 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 26.0f | .Padding(0.0f, 0.0f, 0.0f, 26.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 168 | STextBlock | FontSize | 42 | .Font(FT66Style::Tokens::FontBold(42)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 180 | STextBlock | Padding | 0.0f, 30.0f, 0.0f, 0.0f | .Padding(0.0f, 30.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 186 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 190 | SHorizontalBox | SetMinWidth | 150.f | .SetMinWidth(150.f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 195 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 199 | SHorizontalBox | SetMinWidth | 150.f | .SetMinWidth(150.f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 220 | SBox | WidthOverride | 960.f | .WidthOverride(960.f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 221 | SBox | Padding | FMargin(40.0f, 30.0f | .Padding(FMargin(40.0f, 30.0f)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 232 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 244 | SVerticalBox | Padding | 0.0f, 24.0f, 0.0f, 0.0f | .Padding(0.0f, 24.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66LanguageSelectScreen.cpp | 248 | SVerticalBox | SetMinWidth | 180.f | .SetMinWidth(180.f) |
| Source/T66/UI/Screens/T66LeaderboardScreen.cpp | 47 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/Screens/T66LeaderboardScreen.cpp | 54 | SHorizontalBox | SetMinWidth | 120.f | .SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66LeaderboardScreen.cpp | 60 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66LeaderboardScreen.cpp | 67 | SBox | WidthOverride | 120.f | .WidthOverride(120.f) |
| Source/T66/UI/Screens/T66LeaderboardScreen.cpp | 81 | SBorder | Padding | FMargin(40.f | .Padding(FMargin(40.f)) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 45 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | .Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 49 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 55 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 30.0f | .Padding(0.0f, 0.0f, 0.0f, 30.0f) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 59 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontRegular(18)) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 70 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 73 | SHorizontalBox | SetMinWidth | 200.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 73 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 77 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 80 | SHorizontalBox | SetMinWidth | 200.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 80 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyBackConfirmModal.cpp | 84 | SHorizontalBox | SetPadding | FMargin(40.0f, 30.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(40.0f, 30.0f))) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 43 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | .Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 47 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 57 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 60 | SHorizontalBox | SetMinWidth | 200.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 60 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 64 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 67 | SHorizontalBox | SetMinWidth | 200.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 67 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66LobbyReadyCheckModal.cpp | 71 | SHorizontalBox | SetPadding | FMargin(40.0f, 30.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(40.0f, 30.0f))) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 129 | Unknown | ImageSize | 64.f, 64.f | LocalPlayerHeroPortraitBrush->ImageSize = FVector2D(64.f, 64.f); |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 150 | Unknown | ImageSize | 48.f, 48.f | LocalPlayerCompanionPortraitBrush->ImageSize = FVector2D(48.f, 48.f); |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 196 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 200 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontBold(24)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 224 | SBorder | Padding | 0.0f, 6.0f | .Padding(0.0f, 6.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 231 | SHorizontalBox | Padding | 0.0f, 0.0f, 8.0f, 0.0f | .Padding(0.0f, 0.0f, 8.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 234 | SBox | WidthOverride | 64.0f | .WidthOverride(64.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 235 | SBox | HeightOverride | 64.0f | .HeightOverride(64.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 243 | SBox | Padding | 0.0f, 0.0f, 12.0f, 0.0f | .Padding(0.0f, 0.0f, 12.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 246 | SBox | WidthOverride | 48.0f | .WidthOverride(48.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 247 | SBox | HeightOverride | 48.0f | .HeightOverride(48.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 262 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontRegular(16)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 267 | STextBlock | Padding | 0.0f, 2.0f, 0.0f, 0.0f | .Padding(0.0f, 2.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 271 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 276 | STextBlock | SetPadding | 12.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(12.0f).SetColor(FT66Style::Tokens::Panel) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 287 | SVerticalBox | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 291 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 308 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | .Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 322 | STextBlock | FontSize | 15 | .Font(FT66Style::Tokens::FontRegular(15)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 327 | STextBlock | Padding | 0.0f, 2.0f, 0.0f, 0.0f | .Padding(0.0f, 2.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 331 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 338 | STextBlock | Padding | 12.0f, 0.0f, 0.0f, 0.0f | .Padding(12.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 363 | STextBlock | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 374 | SScrollBox | Padding | 8.0f | .Padding(8.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 404 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 407 | SBox | HeightOverride | 40.0f | .HeightOverride(40.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 408 | SBox | MinDesiredWidth | 200.0f | .MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 418 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 429 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 436 | STextBlock | Padding | 0.0f, 4.0f | .Padding(0.0f, 4.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 442 | STextBlock | Padding | 0.0f, 8.0f | .Padding(0.0f, 8.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 444 | SBox | HeightOverride | 44.0f | SNew(SBox).HeightOverride(44.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 449 | SBox | SetMinWidth | 180.f | .SetMinWidth(180.f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 460 | SHorizontalBox | Padding | 24.0f, 20.0f | .Padding(24.0f, 20.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 466 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 470 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 484 | STextBlock | Padding | 16.0f, 20.0f | .Padding(16.0f, 20.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 490 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 494 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 508 | STextBlock | Padding | 16.0f, 20.0f | .Padding(16.0f, 20.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 523 | SVerticalBox | Padding | 40.0f, 40.0f, 40.0f, 80.0f | .Padding(40.0f, 40.0f, 40.0f, 80.0f) |
| Source/T66/UI/Screens/T66LobbyScreen.cpp | 531 | SVerticalBox | Padding | 20.0f, 0.0f, 0.0f, 20.0f | .Padding(20.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 379 | Unknown | ImageSize | 84.f, 84.f | LastRunHeroBrush->ImageSize = FVector2D(84.f, 84.f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 388 | Unknown | ImageSize | 44.f, 44.f | Brush->ImageSize = FVector2D(44.f, 44.f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 408 | Constant | CenterColumnWidth | 500.f | const float CenterColumnWidth = 500.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 409 | Constant | ColumnHeight | 770.f | const float ColumnHeight = 770.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 410 | Constant | CenterButtonWidth | 500.f | const float CenterButtonWidth = 500.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 411 | Constant | CenterButtonHeight | 114.f | const float CenterButtonHeight = 114.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 412 | Constant | CenterButtonGap | 26.f | const float CenterButtonGap = 26.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 413 | Constant | CenterColumnHeight | (CenterButtonHeight * 2.f) + CenterButtonGap | const float CenterColumnHeight = (CenterButtonHeight * 2.f) + CenterButtonGap; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 414 | Constant | TopInset | UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f | const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 416 | Constant | HorizontalFramePadding | 44.f | const float HorizontalFramePadding = 44.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 417 | Constant | BottomFramePadding | 34.f | const float BottomFramePadding = 34.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 419 | Constant | ReferenceTopInset | 118.f | const float ReferenceTopInset = 118.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 420 | Constant | ReferenceContentWidthBudget | ReferenceViewportSize.X - (HorizontalFramePadding * 2.f) | const float ReferenceContentWidthBudget = ReferenceViewportSize.X - (HorizontalFramePadding * 2.f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 423 | Constant | AvailableContentWidth | FMath::Max(1.f, ViewportSize.X - (HorizontalFramePadding * 2.f)) | const float AvailableContentWidth = FMath::Max(1.f, ViewportSize.X - (HorizontalFramePadding * 2.f)); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 424 | Constant | WidthResponsiveScale | AvailableContentWidth / FMath::Max(ReferenceContentWidthBudget, 1.f) | const float WidthResponsiveScale = AvailableContentWidth / FMath::Max(ReferenceContentWidthBudget, 1.f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 425 | Constant | HeightResponsiveScale | AvailableColumnHeight / FMath::Max(ReferenceAvailableColumnHeight, 1.f) | const float HeightResponsiveScale = AvailableColumnHeight / FMath::Max(ReferenceAvailableColumnHeight, 1.f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 430 | Constant | LeaderboardPanelTargetWidth | FMath::Max(300.f, (ReferenceContentWidthBudget * 0.31f) * LayoutResponsiveScale) | const float LeaderboardPanelTargetWidth = FMath::Max(300.f, (ReferenceContentWidthBudget * 0.31f) * LayoutResponsiveScale); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 431 | Constant | ColumnTargetHeight | FMath::Min(AvailableColumnHeight, FMath::Max(540.f, ColumnHeight * LayoutResponsiveScale)) | const float ColumnTargetHeight = FMath::Min(AvailableColumnHeight, FMath::Max(540.f, ColumnHeight * LayoutResponsiveScale)); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 433 | Constant | CenterSlotMinimumWidth | 280.f | const float CenterSlotMinimumWidth = 280.f; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 435 | Constant | LeaderboardShellTargetWidth | FMath::Max(340.f, UnifiedSidePanelTargetWidth * 0.84f) | const float LeaderboardShellTargetWidth = FMath::Max(340.f, UnifiedSidePanelTargetWidth * 0.84f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 436 | Constant | SocialPanelTargetWidth | FMath::Max(280.f, UnifiedSidePanelTargetWidth * 0.76f) | const float SocialPanelTargetWidth = FMath::Max(280.f, UnifiedSidePanelTargetWidth * 0.76f); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 438 | Constant | SideColumnsMaxWidth | FMath::Max(1.f, AvailableContentWidth - CenterSlotMinimumWidth - (HorizontalFramePadding * 2.f)) | const float SideColumnsMaxWidth = FMath::Max(1.f, AvailableContentWidth - CenterSlotMinimumWidth - (HorizontalFramePadding * 2.f)); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 445 | Constant | LeftPanelBodyFontSize | 14 | const int32 LeftPanelBodyFontSize = 14; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 446 | Constant | LeftPanelTitleFontSize | 14 | const int32 LeftPanelTitleFontSize = 14; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 447 | Constant | CenterButtonFontSize | 42 | const int32 CenterButtonFontSize = 42; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 449 | Unknown | LetterSpacing | 0 | auto MakeRadianceFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 459 | Unknown | LetterSpacing | 140 | HeaderFont.LetterSpacing = 140; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 470 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 481 | SBorder | LetterSpacing | 120 | GroupFont.LetterSpacing = 120; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 490 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 507 | SBox | WidthOverride | 14.f | .WidthOverride(14.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 508 | SBox | HeightOverride | 14.f | .HeightOverride(14.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 521 | SImage | Padding | 12.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 528 | STextBlock | Padding | 10.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 554 | SBox | WidthOverride | 100.f | .WidthOverride(100.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 555 | SBox | HeightOverride | 38.f | .HeightOverride(38.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 559 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 566 | SBorder | Padding | FMargin(12.f, 7.f | .Padding(FMargin(12.f, 7.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 596 | SBorder | Padding | 8.f, 6.f | .Padding(8.f, 6.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 602 | SBox | WidthOverride | 54.f | .WidthOverride(54.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 603 | SBox | HeightOverride | 54.f | .HeightOverride(54.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 613 | SImage | Padding | 12.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 623 | STextBlock | Padding | 0.f, 1.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 631 | STextBlock | Padding | 12.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 672 | STextBlock | SetPadding | FMargin(28.f, 22.f, 28.f, 18.f | .SetPadding(FMargin(28.f, 22.f, 28.f, 18.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 681 | STextBlock | ShadowOffset | 0.f, 1.f | .SetTextShadowOffset(FVector2D(0.f, 1.f)); |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 725 | SVerticalBox | Padding | 0.f, CenterButtonGap, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, CenterButtonGap, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 766 | SVerticalBox | Padding | 0.f, 12.f, 0.f, 0.f | FriendsList->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 774 | SSpacer | Padding | 0.f, 26.f, 0.f, 0.f | FriendsList->AddSlot().AutoHeight().Padding(0.f, 26.f, 0.f, 0.f)[SNew(SSpacer).Size(FVector2D(1.f, 1.f))]; |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 797 | SOverlay | Padding | 0.f, 12.f, 0.f, 0.f | .Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 800 | SBox | WidthOverride | 14.f | .WidthOverride(14.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 801 | SBox | HeightOverride | 14.f | .HeightOverride(14.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 811 | SBorder | Padding | 0.f, 0.f, 0.f, 12.f | .Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 814 | SBox | WidthOverride | 26.f | .WidthOverride(26.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 815 | SBox | HeightOverride | 18.f | .HeightOverride(18.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 825 | SBorder | Padding | SlotIndex > 0 ? FMargin(8.f, 0.f, 0.f, 0.f | .Padding(SlotIndex > 0 ? FMargin(8.f, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 828 | SBox | WidthOverride | 70.f | .WidthOverride(70.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 829 | SBox | HeightOverride | 70.f | .HeightOverride(70.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 841 | SBox | WidthOverride | 78.f | .WidthOverride(78.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 842 | SBox | HeightOverride | 78.f | .HeightOverride(78.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 851 | SImage | Padding | 14.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(14.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 861 | STextBlock | Padding | 0.f, 3.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 870 | STextBlock | Padding | 0.f, 1.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 886 | SBorder | Padding | FMargin(18.f, 18.f, 18.f, 16.f | .Padding(FMargin(18.f, 18.f, 18.f, 16.f)) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 893 | SVerticalBox | Padding | 0.f, 12.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 897 | SVerticalBox | Padding | 0.f, 20.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 20.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 901 | SVerticalBox | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 909 | SScrollBox | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 913 | SScrollBox | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 952 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 973 | SBorder | Padding | 0.f, TopInset, 0.f, 0.f | .Padding(0.f, TopInset, 0.f, 0.f) |
| Source/T66/UI/Screens/T66MainMenuScreen.cpp | 993 | SInvalidationPanel | Padding | 0.f, 0.f, 0.f, BottomFramePadding + CenterButtonLift | .Padding(0.f, 0.f, 0.f, BottomFramePadding + CenterButtonLift) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 43 | Unknown | ImageSize | 560.f, 560.f | SoloCardBrush->ImageSize = FVector2D(560.f, 560.f); |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 49 | Unknown | ImageSize | 560.f, 560.f | CoopCardBrush->ImageSize = FVector2D(560.f, 560.f); |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 104 | SBox | Padding | FMargin(8.0f, 0.0f | .Padding(FMargin(8.0f, 0.0f)) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 111 | SButton | ContentPadding | 0.0f | .ContentPadding(0.0f) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 118 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 122 | SBox | WidthOverride | 560.0f | .WidthOverride(560.0f) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 123 | SBox | HeightOverride | 560.0f | .HeightOverride(560.0f) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 131 | SImage | SetPadding | 4.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(4.f)) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 140 | STextBlock | FontSize | 32 | .Font(FT66Style::Tokens::FontBold(32)) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 142 | STextBlock | SetPadding | FMargin(12.f, 8.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 154 | SOverlay | SetPadding | 0.f | FT66Style::MakePanel(SNullWidget::NullWidget, FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f)) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 174 | SVerticalBox | Padding | 20.0f, 380.0f, 20.0f, 60.0f | .Padding(20.0f, 380.0f, 20.0f, 60.0f) |
| Source/T66/UI/Screens/T66PartySizePickerScreen.cpp | 191 | SHorizontalBox | Padding | 20.0f, 0.0f, 0.0f, 20.0f | .Padding(20.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 178 | SBox | Padding | FMargin(0.f, bDotaTheme ? 5.f : 6.f | .Padding(FMargin(0.f, bDotaTheme ? 5.f : 6.f)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 182 | SBox | SetFontSize | bDotaTheme ? 28 : 44 | .SetFontSize(bDotaTheme ? 28 : 44) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 183 | SBox | SetPadding | bDotaTheme ? FMargin(22.f, 14.f, 22.f, 12.f | .SetPadding(bDotaTheme ? FMargin(22.f, 14.f, 22.f, 12.f) : FMargin(18.f)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 184 | SBox | SetMinWidth | bDotaTheme ? 360.f : 340.f | .SetMinWidth(bDotaTheme ? 360.f : 340.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 185 | SBox | SetHeight | bDotaTheme ? 76.f : 0.f | .SetHeight(bDotaTheme ? 76.f : 0.f)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 220 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 225 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 234 | SBorder | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 247 | SImage | ImageSize | 1.f, 1.f | PortraitBrush->ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 254 | SImage | ImageSize | 40.f, 40.f | HeartBrush->ImageSize = FVector2D(40.f, 40.f); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 271 | SImage | ImageSize | 48.f, 48.f | IdolSlotBrushes[Index]->ImageSize = FVector2D(48.f, 48.f); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 282 | SImage | ImageSize | 42.f, 42.f | InventorySlotBrushes[Index]->ImageSize = FVector2D(42.f, 42.f); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 330 | SHorizontalBox | Padding | 2.f, 0.f | .Padding(2.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 333 | SBox | WidthOverride | 44.f | .WidthOverride(44.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 334 | SBox | HeightOverride | 44.f | .HeightOverride(44.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 349 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 352 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 356 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 359 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 376 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 379 | STextBlock | SetPadding | FMargin(16.f, 14.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f, 14.f))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 383 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 390 | SBox | WidthOverride | 230.f | .WidthOverride(230.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 391 | SBox | HeightOverride | 230.f | .HeightOverride(230.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 413 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 420 | STextBlock | SetPadding | FMargin(14.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 423 | SUniformGridPanel | SlotPadding | FMargin(4.f | .SlotPadding(FMargin(4.f)); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 425 | Constant | IdolColumns | 3 | static constexpr int32 IdolColumns = 3; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 426 | Constant | IdolSlotSize | 50.f | static constexpr float IdolSlotSize = 50.f; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 487 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 490 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 494 | STextBlock | SetPadding | FMargin(16.f, 12.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f, 12.f))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 496 | Constant | InventoryColumns | 10 | static constexpr int32 InventoryColumns = 10; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 497 | Constant | InventorySlotSize | 42.f | static constexpr float InventorySlotSize = 42.f; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 498 | Constant | InventorySlotPad | 2.f | static constexpr float InventorySlotPad = 2.f; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 568 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 571 | STextBlock | Padding | 0.f, 6.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 578 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 581 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 585 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 588 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 592 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 596 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 600 | STextBlock | SetPadding | FMargin(16.f, 12.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f, 12.f))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 607 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 609 | STextBlock | SetPadding | FMargin(16.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 611 | STextBlock | FontSize | 56 | FSlateFontInfo PauseTitleFont = FT66Style::Tokens::FontBold(bDotaTheme ? 56 : 48); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 614 | STextBlock | LetterSpacing | 160 | PauseTitleFont.LetterSpacing = 160; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 619 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 24.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 24.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 638 | STextBlock | SetPadding | FMargin(FT66Style::Tokens::Space8, FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space8, FT66Style::Tokens::Space6))); |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 645 | SVerticalBox | Padding | 0.f, 16.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 655 | SVerticalBox | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 660 | Constant | SideColumnWidth | 460.f | static constexpr float SideColumnWidth = 460.f; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 661 | Constant | ButtonColumnWidth | 420.f | static constexpr float ButtonColumnWidth = 420.f; |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 673 | SOverlay | Padding | 40.f, 40.f, 0.f, 0.f | .Padding(40.f, 40.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 680 | SOverlay | Padding | 50.f, 40.f | .Padding(50.f, 40.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 685 | SHorizontalBox | Padding | 0.f, 0.f, 28.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 28.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 694 | SBox | Padding | 0.f, 0.f, 28.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 28.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 714 | SHorizontalBox | Padding | 0.f, 0.f, 28.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 28.f, 0.f) |
| Source/T66/UI/Screens/T66PauseMenuScreen.cpp | 723 | SBox | Padding | 0.f, 0.f, 28.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 28.f, 0.f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 42 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 55 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 74 | STextBlock | ImageSize | 96.f, 96.f | Brush->ImageSize = FVector2D(96.f, 96.f); |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 102 | SHorizontalBox | Padding | 16.0f, 0.0f | .Padding(16.0f, 0.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 109 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | .Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 113 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 119 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | .Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 122 | SBox | WidthOverride | 96.0f | .WidthOverride(96.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 123 | SBox | HeightOverride | 96.0f | .HeightOverride(96.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 128 | SBorder | Padding | 0.0f | .Padding(0.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 142 | SSpacer | SetMinWidth | 140.f | .SetMinWidth(140.f)) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 145 | SSpacer | SetPadding | FMargin(16.0f, 12.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.0f, 12.0f))) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 161 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 24.0f | .Padding(0.0f, 0.0f, 0.0f, 24.0f) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 165 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66PlayerSummaryPickerScreen.cpp | 174 | STextBlock | SetPadding | FMargin(32.0f, 24.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(32.0f, 24.0f))) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 34 | Constant | PowerUpStatsPerRow | 3 | constexpr int32 PowerUpStatsPerRow = 3; |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 203 | Constant | FillHeight | FMath::Max(0.f, LocalSize.Y * FillFraction) | const float FillHeight = FMath::Max(0.f, LocalSize.Y * FillFraction); |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 536 | SVerticalBox | Padding | 0.f, 2.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 540 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 547 | SBox | MinDesiredHeight | 560.f | .MinDesiredHeight(560.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 550 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 556 | SVerticalBox | Padding | 0.f, 16.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 16.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 560 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 561 | SVerticalBox | SetHeight | 66.f | .SetHeight(66.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 562 | SVerticalBox | SetFontSize | 23 | .SetFontSize(23) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 588 | SHorizontalBox | Padding | LocalIndex < PowerUpStatsPerRow - 1 ? FMargin(0.f, 0.f, FT66Style::Tokens::Space4, 0.f | .Padding(LocalIndex < PowerUpStatsPerRow - 1 ? FMargin(0.f, 0.f, FT66Style::Tokens::Space4, 0.f) : FMargin(0.f)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 605 | SScrollBox | Padding | 0.f, 0.f, 10.f, 0.f | .Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 612 | SVerticalBox | Padding | 0.f, 22.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 616 | SVerticalBox | Padding | 0.f, 1.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 626 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f).HAlign(HAlign_Center) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 630 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontBold(24)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 633 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f).HAlign(HAlign_Center) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 637 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 641 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 644 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 652 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 655 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 659 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 666 | STextBlock | Padding | 12.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 674 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 677 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 681 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 689 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f).HAlign(HAlign_Center) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 693 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 697 | STextBlock | Padding | 0.f, 22.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 700 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 711 | SHorizontalBox | Padding | 10.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 722 | SHorizontalBox | Padding | 10.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 737 | Constant | TopInset | UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f | const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f; |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 741 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 748 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 757 | STextBlock | FontSize | 48 | .Font(FT66Style::Tokens::FontBold(48)) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 770 | SBox | SetMinWidth | 240.f | .SetMinWidth(240.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 787 | SWidgetSwitcher | Padding | 20.f, 0.f, 0.f, 20.f | .Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66PowerUpScreen.cpp | 795 | SBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 50 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | .Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 54 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 61 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 30.0f | .Padding(0.0f, 0.0f, 0.0f, 30.0f) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 65 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontRegular(18)) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 77 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 80 | SHorizontalBox | SetMinWidth | 320.f | .SetMinWidth(320.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 80 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(320.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 84 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 87 | SHorizontalBox | SetMinWidth | 320.f | .SetMinWidth(320.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 87 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(320.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66QuitConfirmationModal.cpp | 91 | SHorizontalBox | SetPadding | FMargin(40.0f, 30.0f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(40.0f, 30.0f))) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 58 | SBorder | Padding | FMargin(40.0f, 30.0f | .Padding(FMargin(40.0f, 30.0f)) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 64 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 16.0f | .Padding(0.0f, 0.0f, 0.0f, 16.0f) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 68 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 73 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 16.0f | .Padding(0.0f, 0.0f, 0.0f, 16.0f) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 76 | SBox | WidthOverride | 400.0f | .WidthOverride(400.0f) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 77 | SBox | HeightOverride | 120.0f | .HeightOverride(120.0f) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 80 | SMultiLineEditableTextBox | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 92 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66ReportBugScreen.cpp | 98 | SHorizontalBox | Padding | 10.0f, 0.0f | .Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 210 | Unknown | ImageSize | 520.f, 520.f | HeroPreviewBrush->ImageSize = FVector2D(520.f, 520.f); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 345 | STableRow<TSharedPtr<FString>>, OwnerTable | Padding | FMargin(4.f, 2.f | .Padding(FMargin(4.f, 2.f)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 349 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 413 | STextBlock | ImageSize | 48.f, 48.f | PowerCouponSpriteBrush->ImageSize = FVector2D(48.f, 48.f); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 447 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 457 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 463 | Constant | PreviewSize | 520.f | constexpr float PreviewSize = 520.f; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 482 | STextBlock | SetPadding | 0.f | : StaticCastSharedRef<SWidget>(FT66Style::MakePanel(PreviewContent, FT66PanelParams(ET66PanelType::Panel).SetPadding(0.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 491 | SSpacer | SetPadding | FT66Style::Tokens::Space2 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space2) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 503 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 512 | SHyperlink | SetPadding | FMargin(10.f, 8.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(10.f, 8.f)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 516 | SHyperlink | Padding | 10.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 519 | SHyperlink | SetPadding | FMargin(14.f, 8.f | .SetPadding(FMargin(14.f, 8.f)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 538 | SEditableTextBox | MinDesiredWidth | 420.f | .MinDesiredWidth(420.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 540 | SEditableTextBox | Padding | 10.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 543 | SEditableTextBox | SetPadding | FMargin(14.f, 8.f | .SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 556 | SEditableTextBox | SetPadding | FMargin(14.f, 10.f | .SetPadding(FMargin(14.f, 10.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 561 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 567 | STextBlock | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 576 | SHorizontalBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 579 | SHorizontalBox | SetPadding | FMargin(14.f, 8.f | .SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 581 | SHorizontalBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 584 | SHorizontalBox | SetPadding | FMargin(14.f, 8.f | .SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 588 | SHorizontalBox | SetPadding | FT66Style::Tokens::Space3 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3)); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 593 | SHorizontalBox | SetMinWidth | 160.f | .SetMinWidth(160.f).SetPadding(FMargin(14.f, 8.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 593 | SHorizontalBox | SetPadding | FMargin(14.f, 8.f | .SetMinWidth(160.f).SetPadding(FMargin(14.f, 8.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 597 | Constant | StatsPanelWidth | FT66Style::Tokens::NPCVendorStatsPanelWidth | const float StatsPanelWidth = FT66Style::Tokens::NPCVendorStatsPanelWidth; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 612 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | PrimaryStatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 629 | SBox | HeightOverride | FT66Style::Tokens::NPCStatsPanelContentHeight | .HeightOverride(FT66Style::Tokens::NPCStatsPanelContentHeight) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 641 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 648 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 684 | Constant | IdolSlotPad | 4.f | static constexpr float IdolSlotPad = 4.f; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 685 | Constant | IdolSlotSize | 52.f | static constexpr float IdolSlotSize = 52.f; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 706 | SHorizontalBox | ImageSize | IdolSlotSize - 4.f, IdolSlotSize - 4.f | IdolBrush->ImageSize = FVector2D(IdolSlotSize - 4.f, IdolSlotSize - 4.f); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 726 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 740 | SBorder | Padding | 4.f | .Padding(4.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 746 | Constant | InvCols | 10 | static constexpr int32 InvCols = 10; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 747 | Constant | InvRows | 2 | static constexpr int32 InvRows = 2; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 748 | Constant | InvSlotSize | 56.f | static constexpr float InvSlotSize = 56.f; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 749 | Constant | InvSlotPad | 3.f | static constexpr float InvSlotPad = 3.f; |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 756 | SBorder | ImageSize | InvSlotSize - 4.f, InvSlotSize - 4.f | InventoryItemIconBrushes[i]->ImageSize = FVector2D(InvSlotSize - 4.f, InvSlotSize - 4.f); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 800 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 805 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 822 | SBorder | Padding | 4.f | .Padding(4.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 830 | SBorder | SetMinWidth | 120.f | .SetMinWidth(120.f).SetPadding(FMargin(18.f, 10.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 830 | SBorder | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(120.f).SetPadding(FMargin(18.f, 10.f))); |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 837 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 841 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 849 | STextBlock | SetMinWidth | 160.f | .SetMinWidth(160.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 849 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(160.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 855 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 858 | SVerticalBox | SetMinWidth | 180.f | .SetMinWidth(180.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 858 | SVerticalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(180.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 860 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 863 | SVerticalBox | SetMinWidth | 200.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 863 | SVerticalBox | SetPadding | FMargin(18.f, 10.f | .SetMinWidth(200.f).SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 879 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 937 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 987 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 990 | SHorizontalBox | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 991 | STextBlock | FontSize | 12 | [SNew(STextBlock).Text(RankHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 992 | STextBlock | Padding | 0.f, 0.f, 80.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 993 | STextBlock | FontSize | 12 | [SNew(STextBlock).Text(SourceHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 995 | STextBlock | FontSize | 12 | [SNew(STextBlock).Text(DamageHeader).TextStyle(&BodyStyle).ColorAndOpacity(FT66Style::Tokens::TextMuted).Font(FT66Style::Tokens::FontBold(12))] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1000 | STextBlock | Padding | 0.f, 0.f, 0.f, 4.f | DamageBySourceBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1003 | SHorizontalBox | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1005 | STextBlock | Padding | 0.f, 0.f, 80.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 80.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1034 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1041 | STextBlock | FontSize | 40 | .Font(FT66Style::Tokens::FontBold(40)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1050 | STextBlock | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1054 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1058 | STextBlock | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1064 | SHorizontalBox | Padding | 0.f, 0.f, 24.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1071 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1074 | STextBlock | Padding | 6.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1078 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1090 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1093 | STextBlock | Padding | 6.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1097 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1104 | STextBlock | Padding | 0.f, -10.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, -10.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1112 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1115 | STextBlock | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1123 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1127 | STextBlock | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1131 | SHorizontalBox | Padding | 0.f, 0.f, 24.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 24.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1134 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[LuckRatingPanel] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1135 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[SkillRatingPanel] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1136 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[CheatProbabilityPanel] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1137 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1142 | SBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1146 | SBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1151 | SBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1162 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1164 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1170 | SVerticalBox | Padding | 24.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(24.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1173 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[BaseStatsPanel] |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1177 | SVerticalBox | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1184 | SVerticalBox | Padding | 20.f, 0.f, 0.f, 20.f | .Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1196 | SBox | Padding | 0.f, 70.f, 0.f, 0.f | .Padding(0.f, 70.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1199 | SBox | WidthOverride | 520.f | .WidthOverride(520.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1200 | SBox | HeightOverride | 620.f | .HeightOverride(620.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1217 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1220 | SBox | WidthOverride | 320.f | .WidthOverride(320.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1221 | SBox | Padding | 24.f | .Padding(24.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1228 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1232 | STextBlock | Padding | 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1235 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1238 | SBox | WidthOverride | 48.f | .WidthOverride(48.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1239 | SBox | HeightOverride | 48.f | .HeightOverride(48.f) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1255 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontBold(24)) |
| Source/T66/UI/Screens/T66RunSummaryScreen.cpp | 1259 | STextBlock | Padding | 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 90 | Unknown | ImageSize | 24.f, 24.f | SlotIdolBrushes[i][k]->ImageSize = FVector2D(24.f, 24.f); |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 96 | Constant | SlotPadding | 20.f | const float SlotPadding = 20.f; |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 97 | Constant | PortraitSize | 80.f | const float PortraitSize = 80.f; |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 173 | SHorizontalBox | Padding | 2.f | IdolRow->AddSlot().AutoWidth().Padding(2.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 175 | SBox | HeightOverride | 24.f | SNew(SBox).WidthOverride(24.f).HeightOverride(24.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 175 | SBox | WidthOverride | 24.f | SNew(SBox).WidthOverride(24.f).HeightOverride(24.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 189 | SVerticalBox | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 191 | SVerticalBox | Padding | 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 195 | STextBlock | FontSize | 14 | .Font(bOccupied ? FT66Style::Tokens::FontBold(14) : FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 199 | STextBlock | Padding | 0.f, 2.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 2.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 203 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 207 | STextBlock | Padding | 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 209 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 212 | SHorizontalBox | Padding | 4.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 214 | SHorizontalBox | SetMinWidth | 90.f | FT66Style::MakeButton(FT66ButtonParams(PreviewText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePreviewClicked, SlotIndex), ET66ButtonType::Neutral).SetMinWidth(90.f).SetEnabled(bOccupied && bSlotVisible)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 216 | SHorizontalBox | Padding | 4.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 218 | SHorizontalBox | SetMinWidth | 90.f | FT66Style::MakeButton(FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleLoadClicked, SlotIndex), ET66ButtonType::Success).SetMinWidth(90.f).SetEnabled(bOccupied && bSlotVisible)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 224 | SBox | WidthOverride | 220.f | SNew(SBox).WidthOverride(220.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 233 | SBorder | Padding | 16.f | .Padding(16.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 235 | SBox | WidthOverride | 220.f | SNew(SBox).WidthOverride(220.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 243 | Constant | TopInset | UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f | const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f; |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 251 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 264 | SVerticalBox | Padding | 0.f, 40.f, 0.f, 30.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 40.f, 0.f, 30.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 265 | STextBlock | FontSize | 48 | [SNew(STextBlock).Text(TitleText).Font(FT66Style::Tokens::FontBold(48)).ColorAndOpacity(FT66Style::Tokens::Text)] |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 266 | STextBlock | Padding | 0.f, 0.f, 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 270 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 273 | STextBlock | Padding | 0.f, 0.f, 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 280 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontRegular(18)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 287 | SHorizontalBox | Padding | SlotPadding, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(0)] |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 288 | SHorizontalBox | Padding | SlotPadding, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(1)] |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 289 | SHorizontalBox | Padding | SlotPadding, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(SlotPadding, 0.f)[MakeSlotCard(2)] |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 291 | SHorizontalBox | Padding | 0.f, 24.f, 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 24.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 294 | SHorizontalBox | Padding | 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 296 | SHorizontalBox | SetMinWidth | 100.f | FT66Style::MakeButton(FT66ButtonParams(PrevText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePrevPageClicked), ET66ButtonType::Neutral).SetMinWidth(100.f).SetEnabled(CurrentPage > 0)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 298 | SHorizontalBox | Padding | 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(16.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 299 | STextBlock | FontSize | 14 | [SNew(STextBlock).Text(PageText).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(FT66Style::Tokens::Text)] |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 300 | STextBlock | Padding | 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 302 | STextBlock | SetMinWidth | 100.f | FT66Style::MakeButton(FT66ButtonParams(NextText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleNextPageClicked), ET66ButtonType::Neutral).SetMinWidth(100.f).SetEnabled(CurrentPage < TotalPages - 1)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 306 | STextBlock | Padding | 20.f, 0.f, 0.f, 20.f | + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.cpp | 311 | SBox | SetMinWidth | 120.f | FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleBackClicked), ET66ButtonType::Neutral).SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66SaveSlotsScreen.h | 54 | Constant | SlotsPerPage | 3 | static constexpr int32 SlotsPerPage = 3; |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 159 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 160 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 166 | SBorder | Padding | FMargin(1.f | .Padding(FMargin(1.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 208 | SBox | Padding | FMargin(2.0f, 4.0f | return SNew(SBox).Padding(FMargin(2.0f, 4.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 212 | SBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 213 | SBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 219 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 228 | Constant | PanelW | 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayWidth) | const float PanelW = 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayWidth); |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 229 | Constant | PanelH | 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayHeight) | const float PanelH = 0.8f * static_cast<float>(DisplayMetrics.PrimaryDisplayHeight); |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 231 | Constant | TopInset | bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) | const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f); |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 238 | SVerticalBox | Padding | 0.0f, 6.0f | .Padding(0.0f, 6.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 282 | SBox | SetHeight | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 282 | SBox | SetMinWidth | 40.f | .SetMinWidth(40.f).SetHeight(40.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 283 | SBox | SetPadding | FMargin(0.f | .SetPadding(FMargin(0.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 286 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 299 | STextBlock | Padding | 20.0f | .Padding(20.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 342 | SBorder | Padding | FMargin(18.f, 14.f, 18.f, 18.f | .Padding(FMargin(18.f, 14.f, 18.f, 18.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 362 | SBorder | Padding | 0.0f | .Padding(0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 381 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 678 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 683 | SBox | MinDesiredWidth | 200.0f | SNew(SBox).MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 686 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 690 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 697 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 698 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 702 | SHorizontalBox | Padding | 4.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 706 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 707 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 734 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 744 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 747 | STextBlock | Padding | 0.0f, 4.0f, 18.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 751 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 756 | STextBlock | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 762 | SBox | WidthOverride | 170.0f | .WidthOverride(170.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 767 | SBorder | Padding | 1.0f | .Padding(1.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 771 | SEditableTextBox | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 783 | SEditableTextBox | Padding | 0.0f, 6.0f, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 787 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontRegular(18)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 799 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 807 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 815 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 823 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 831 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 852 | SHorizontalBox | Padding | FMargin(10.0f, 4.0f | + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(FMargin(10.0f, 4.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 855 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 858 | STextBlock | Padding | FMargin(4.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 861 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 866 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 872 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 877 | STextBlock | SetHeight | 0 | MakeSettingsDropdown(FT66DropdownParams(TriggerContent, MakeMenuContent).SetHeight(0)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 932 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 958 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 967 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 988 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 997 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1011 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1023 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1037 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1048 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1052 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1057 | SBox | MinDesiredWidth | 200.0f | SNew(SBox).MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1061 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1065 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1069 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1075 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1079 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1088 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontRegular(22)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1094 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontRegular(22)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1098 | STextBlock | Padding | 0.0f, 8.0f, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1111 | SSlider | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1125 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1135 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1139 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1144 | SBox | MinDesiredWidth | 200.0f | SNew(SBox).MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1148 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1152 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1159 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1160 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1164 | SHorizontalBox | Padding | 4.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1168 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1169 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1177 | SHorizontalBox | Padding | 0.0f, 15.0f, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1182 | SHorizontalBox | SetFontSize | 32 | .SetFontSize(32).SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1182 | SHorizontalBox | SetMinWidth | 120.f | .SetFontSize(32).SetMinWidth(120.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1194 | SBorder | Padding | FMargin(30.f | .Padding(FMargin(30.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1198 | SBorder | Padding | FMargin(25.f | .Padding(FMargin(25.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1201 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1205 | STextBlock | FontSize | 36 | .Font(FT66Style::Tokens::FontBold(36)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1208 | STextBlock | Padding | 0.f, 0.f, 0.f, 15.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 15.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1212 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1218 | SHorizontalBox | Padding | 6.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1224 | SHorizontalBox | Padding | 6.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1247 | SBox | Padding | FMargin(2.0f, 0.0f | return SNew(SBox).Padding(FMargin(2.0f, 0.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1260 | SBox | SetFontSize | 32 | .SetFontSize(32) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1261 | SBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1262 | SBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1292 | SHorizontalBox | Padding | 4.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center).Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1296 | SBorder | Padding | FMargin(8.f, 4.f | .Padding(FMargin(8.f, 4.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1300 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontBold(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1304 | STextBlock | Padding | 4.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1312 | STextBlock | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1313 | STextBlock | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1317 | STextBlock | Padding | 4.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1333 | STextBlock | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1334 | STextBlock | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1350 | SBorder | Padding | FMargin(15.0f, 10.0f | .Padding(FMargin(15.0f, 10.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1353 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1356 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1359 | STextBlock | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1362 | SHorizontalBox | Padding | 0.f, 0.f, 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1364 | STextBlock | FontSize | 20 | SNew(STextBlock).Text(PrimaryText).Font(FT66Style::Tokens::FontRegular(20)).ColorAndOpacity(GetSettingsPageMuted()) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1374 | SHorizontalBox | Padding | 0.f, 0.f, 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 6.f, 0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1376 | STextBlock | FontSize | 20 | SNew(STextBlock).Text(SecondaryText).Font(FT66Style::Tokens::FontRegular(20)).ColorAndOpacity(GetSettingsPageMuted()) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1389 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1395 | SHorizontalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1399 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1412 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1414 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1416 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1418 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1420 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1422 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1424 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1426 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1428 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1430 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1432 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1434 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1436 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1438 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1440 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1442 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1444 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1446 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1448 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1458 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1460 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1462 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1464 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1466 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1468 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1470 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1472 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1474 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1476 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1478 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1480 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1482 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1484 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1486 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1488 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1490 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1492 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1497 | SVerticalBox | Padding | 0.0f, 15.0f, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1502 | SVerticalBox | SetFontSize | 32 | .SetFontSize(32).SetMinWidth(180.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1502 | SVerticalBox | SetMinWidth | 180.f | .SetFontSize(32).SetMinWidth(180.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1519 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1524 | SBox | MinDesiredWidth | 200.0f | SNew(SBox).MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1527 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1531 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1538 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1539 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1543 | SHorizontalBox | Padding | 4.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1547 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1548 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1560 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1564 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1568 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1576 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1584 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1592 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1616 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1620 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1624 | STextBlock | Padding | 0.0f, 16.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1628 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1633 | SBox | MinDesiredWidth | 200.0f | SNew(SBox).MinDesiredWidth(200.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1637 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1641 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1648 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1649 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1653 | SHorizontalBox | Padding | 4.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1657 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1658 | SHorizontalBox | SetPadding | FMargin(12.f, 6.f | .SetPadding(FMargin(12.f, 6.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1677 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1683 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1686 | STextBlock | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(0.55f).VAlign(VAlign_Center).Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1700 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1708 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1716 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1724 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1732 | SVerticalBox | Padding | 0.f, 10.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1736 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1743 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1750 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1765 | STextBlock | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1769 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1776 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1783 | SHorizontalBox | Padding | FMargin(10.0f, 4.0f | + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(FMargin(10.0f, 4.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1791 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1794 | STextBlock | Padding | FMargin(4.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(4.0f, 0.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1797 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1808 | SVerticalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1812 | SVerticalBox | SetHeight | 0 | }).SetHeight(0)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1816 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1820 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1827 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1834 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1863 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1872 | STextBlock | SetFontSize | 18 | .SetFontSize(18) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1873 | STextBlock | SetPadding | FMargin(12.0f, 6.0f | .SetPadding(FMargin(12.0f, 6.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1900 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1910 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1913 | STextBlock | Padding | 0.0f, 4.0f, 18.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1917 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1922 | STextBlock | Padding | 10.0f, 0.0f | + SHorizontalBox::Slot().FillWidth(0.54f).VAlign(VAlign_Center).Padding(10.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1928 | SBox | WidthOverride | 170.0f | .WidthOverride(170.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1933 | SBorder | Padding | 1.0f | .Padding(1.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1937 | SEditableTextBox | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1949 | SEditableTextBox | Padding | 0.0f, 6.0f, 0.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1953 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontRegular(18)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1965 | SBorder | Padding | FMargin(15.0f, 12.0f | .Padding(FMargin(15.0f, 12.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1975 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontRegular(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1978 | STextBlock | Padding | 0.0f, 4.0f, 18.0f, 0.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 18.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1982 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1987 | STextBlock | Padding | 10.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 1998 | SHorizontalBox | Padding | 4.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f, 0.0f, 0.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2038 | SHorizontalBox | Padding | bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f | + SHorizontalBox::Slot().AutoWidth().Padding(bIncludeReset ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2048 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2052 | STextBlock | FontSize | 32 | .Font(FT66Style::Tokens::FontBold(32)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2055 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 14.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2059 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontRegular(22)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2063 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2071 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2075 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2079 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 18.0f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 0.0f, 0.0f, 18.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2083 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2087 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2091 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2095 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2099 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2103 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2107 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2111 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2115 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2119 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2123 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2127 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2131 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2135 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2139 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2143 | STextBlock | Padding | 0.0f, 6.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2147 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2151 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2155 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2159 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2163 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2167 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2171 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2175 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2179 | STextBlock | Padding | 0.0f, 6.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2183 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2187 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2191 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2195 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2199 | STextBlock | Padding | 0.0f, 6.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2203 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2207 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2211 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2215 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 18.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2219 | STextBlock | Padding | 0.0f, 6.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2223 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2227 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2231 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2235 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 12.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2239 | STextBlock | Padding | 0.0f, 6.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2243 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2247 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2251 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2255 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 8.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2259 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 18.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 18.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2275 | SVerticalBox | Padding | 0.0f, 0.0f, 0.0f, 20.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2279 | STextBlock | FontSize | 32 | .Font(FT66Style::Tokens::FontBold(32)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2283 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2287 | SBorder | Padding | FMargin(20.0f, 15.0f | .Padding(FMargin(20.0f, 15.0f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2294 | STextBlock | FontSize | 26 | .Font(FT66Style::Tokens::FontRegular(26)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2301 | STextBlock | Padding | 0.0f, 15.0f, 0.0f, 20.0f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 15.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2306 | STextBlock | SetFontSize | 32 | .SetFontSize(32).SetMinWidth(220.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2306 | STextBlock | SetMinWidth | 220.f | .SetFontSize(32).SetMinWidth(220.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2309 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 20.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 20.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2314 | SBox | HeightOverride | 1.0f | SNew(SBox).HeightOverride(1.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2318 | SBox | Padding | 0.0f, 0.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2322 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2325 | STextBlock | Padding | 0.0f, 0.0f, 0.0f, 10.0f | + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2329 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontRegular(24)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2338 | STextBlock | SetFontSize | 32 | .SetFontSize(32).SetMinWidth(150.f)) |
| Source/T66/UI/Screens/T66SettingsScreen.cpp | 2338 | STextBlock | SetMinWidth | 150.f | .SetFontSize(32).SetMinWidth(150.f)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 278 | Constant | TopInset | UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f | const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f; |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 304 | Unknown | SetMinWidth | 150.f | .SetMinWidth(150.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 305 | Unknown | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 312 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 328 | SBox | Padding | FMargin(0.f, TopInset, 0.f, 0.f | .Padding(FMargin(0.f, TopInset, 0.f, 0.f)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 337 | SVerticalBox | Padding | 30.f, 25.f, 30.f, 20.f | .Padding(30.f, 25.f, 30.f, 20.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 345 | STextBlock | FontSize | 50 | .Font(FT66Style::Tokens::FontBold(50)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 351 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | .Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 357 | SVerticalBox | Padding | 14.f, 0.f, 14.f, 10.f | .Padding(14.f, 0.f, 14.f, 10.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 368 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 377 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 390 | SProgressBar | SetPadding | FMargin(14.f, 14.f | .SetPadding(FMargin(14.f, 14.f))) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 395 | SProgressBar | Padding | 30.f, 0.f, 30.f, 15.f | .Padding(30.f, 0.f, 30.f, 15.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 413 | SHorizontalBox | Padding | 30.f, 0.f, 30.f, 20.f | .Padding(30.f, 0.f, 30.f, 20.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 425 | SVerticalBox | Padding | 20.f, 0.f, 0.f, 20.f | .Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 435 | SBox | SetMinWidth | 120.f | .SetMinWidth(120.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 436 | SBox | SetFontSize | 20 | .SetFontSize(20)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 457 | SBox | Padding | 0.f, 0.f, 0.f, 8.f | .Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 461 | SBorder | Padding | FMargin(18.f, 14.f | .Padding(FMargin(18.f, 14.f)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 467 | SHorizontalBox | Padding | 0.f, 0.f, 14.f, 0.f | .Padding(0.f, 0.f, 14.f, 0.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 471 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 484 | STextBlock | FontSize | 24 | .Font(FT66Style::Tokens::FontBold(24)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 489 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | .Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 493 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontRegular(20)) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 502 | STextBlock | Padding | 16.f, 0.f, 0.f, 0.f | .Padding(16.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/Screens/T66UnlocksScreen.cpp | 508 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 29 | Constant | BaseOpacityMin | FMath::Clamp(InArgs._BaseOpacityMin, 0.f, 1.f) | const float BaseOpacityMin = FMath::Clamp(InArgs._BaseOpacityMin, 0.f, 1.f); |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 30 | Constant | BaseOpacityMax | FMath::Max(BaseOpacityMin, FMath::Clamp(InArgs._BaseOpacityMax, 0.f, 1.f)) | const float BaseOpacityMax = FMath::Max(BaseOpacityMin, FMath::Clamp(InArgs._BaseOpacityMax, 0.f, 1.f)); |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 88 | Constant | Width | FMath::Max(0.f, Right - Left) | const float Width = FMath::Max(0.f, Right - Left); |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 89 | Constant | Height | FMath::Max(0.f, Bottom - Top) | const float Height = FMath::Max(0.f, Bottom - Top); |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 117 | Constant | ClampedThickness | FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f) | const float ClampedThickness = FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f); |
| Source/T66/UI/ST66AnimatedBorderGlow.cpp | 138 | Constant | ClampedThickness | FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f) | const float ClampedThickness = FMath::Clamp(Thickness, 1.f, FMath::Min(Width, Height) * 0.5f); |
| Source/T66/UI/ST66PulsingIcon.cpp | 20 | Constant | IconSize | InArgs._IconSize.IsNearlyZero() ? FVector2D(28.f, 28.f) : InArgs._IconSize | const FVector2D IconSize = InArgs._IconSize.IsNearlyZero() ? FVector2D(28.f, 28.f) : InArgs._IconSize; |
| Source/T66/UI/ST66PulsingIcon.cpp | 21 | Constant | GlowSize | InArgs._GlowSize.IsNearlyZero() ? FVector2D(50.f, 50.f) : InArgs._GlowSize | const FVector2D GlowSize = InArgs._GlowSize.IsNearlyZero() ? FVector2D(50.f, 50.f) : InArgs._GlowSize; |
| Source/T66/UI/ST66PulsingIcon.cpp | 22 | Constant | GlowOpacityMin | FMath::Clamp(InArgs._GlowOpacityMin, 0.f, 1.f) | const float GlowOpacityMin = FMath::Clamp(InArgs._GlowOpacityMin, 0.f, 1.f); |
| Source/T66/UI/ST66PulsingIcon.cpp | 23 | Constant | GlowOpacityMax | FMath::Max(GlowOpacityMin, FMath::Clamp(InArgs._GlowOpacityMax, 0.f, 1.f)) | const float GlowOpacityMax = FMath::Max(GlowOpacityMin, FMath::Clamp(InArgs._GlowOpacityMax, 0.f, 1.f)); |
| Source/T66/UI/Style/T66ButtonVisuals.cpp | 63 | Unknown | ImageSize | 1.f, 1.f | Brush.ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Style/T66ButtonVisuals.cpp | 76 | Unknown | ImageSize | 1.f, 1.f | Brush.ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Style/T66ButtonVisuals.cpp | 87 | Unknown | ImageSize | 1.f, 1.f | Brush.ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Style/T66Style.cpp | 66 | Constant | GMaxPreviousStyleSets | 4 | static constexpr int32 GMaxPreviousStyleSets = 4; |
| Source/T66/UI/Style/T66Style.cpp | 106 | Unknown | ImageSize | 512.f, 128.f | Brush.ImageSize = FVector2D(512.f, 128.f); |
| Source/T66/UI/Style/T66Style.cpp | 166 | Unknown | ImageSize | 1.f, 1.f | Brush.ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Style/T66Style.cpp | 368 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 373 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 388 | SBorder | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Style/T66Style.cpp | 391 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 400 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Style/T66Style.cpp | 403 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 473 | SBorder | ImageSize | 1.f, 1.f | State->Brush->ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/Style/T66Style.cpp | 521 | SBorder | ImageSize | 128.f, 128.f | Brush.ImageSize = FVector2D(128.f, 128.f); |
| Source/T66/UI/Style/T66Style.cpp | 659 | SBorder | ComputeDesiredSize | 1920.f, 1080.f | return FVector2D(1920.f, 1080.f); |
| Source/T66/UI/Style/T66Style.cpp | 675 | Constant | MaxScale | bAllowUpscale ? 1.35f : 1.f | const float MaxScale = bAllowUpscale ? 1.35f : 1.f; |
| Source/T66/UI/Style/T66Style.cpp | 695 | SBorder | FontSize | 32 | FSlateFontInfo FT66Style::Tokens::FontRegular(int32 Size) |
| Source/T66/UI/Style/T66Style.cpp | 699 | SBorder | FontSize | 32 | FSlateFontInfo FT66Style::Tokens::FontBold(int32 Size) |
| Source/T66/UI/Style/T66Style.cpp | 708 | SBorder | LetterSpacing | 180 | Font.LetterSpacing = 180; |
| Source/T66/UI/Style/T66Style.cpp | 717 | SBorder | LetterSpacing | 120 | Font.LetterSpacing = 120; |
| Source/T66/UI/Style/T66Style.cpp | 726 | SBorder | LetterSpacing | 20 | Font.LetterSpacing = 20; |
| Source/T66/UI/Style/T66Style.cpp | 735 | SBorder | LetterSpacing | 18 | Font.LetterSpacing = 18; |
| Source/T66/UI/Style/T66Style.cpp | 744 | SBorder | LetterSpacing | 90 | Font.LetterSpacing = 90; |
| Source/T66/UI/Style/T66Style.cpp | 753 | SBorder | LetterSpacing | 110 | Font.LetterSpacing = 110; |
| Source/T66/UI/Style/T66Style.cpp | 808 | SBorder | FontSize | 32 | FSlateFontInfo FT66Style::MakeFont(const TCHAR* Weight, int32 Size) |
| Source/T66/UI/Style/T66Style.cpp | 924 | Constant | PanelCornerRadius | IsDotaTheme() ? FT66Style::CornerRadius() : Tokens::CornerRadius | const float PanelCornerRadius = IsDotaTheme() ? FT66Style::CornerRadius() : Tokens::CornerRadius; |
| Source/T66/UI/Style/T66Style.cpp | 925 | Constant | PanelCornerRadiusSmall | IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall | const float PanelCornerRadiusSmall = IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall; |
| Source/T66/UI/Style/T66Style.cpp | 956 | SBorder | ShadowOffset | 1.f, 1.f | .SetShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/Style/T66Style.cpp | 963 | SBorder | ShadowOffset | 1.f, 1.f | .SetShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/Style/T66Style.cpp | 985 | Constant | ButtonCornerRadius | IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall | const float ButtonCornerRadius = IsDotaTheme() ? FT66Style::CornerRadiusSmall() : Tokens::CornerRadiusSmall; |
| Source/T66/UI/Style/T66Style.cpp | 1074 | Constant | RowBorderW | 0.5f | constexpr float RowBorderW = 0.5f; |
| Source/T66/UI/Style/T66Style.cpp | 1335 | Constant | ResolvedFontSize | Params.FontSize > 0 ? Params.FontSize : TextFont.Size | const int32 ResolvedFontSize = Params.FontSize > 0 ? Params.FontSize : TextFont.Size; |
| Source/T66/UI/Style/T66Style.cpp | 1457 | Constant | PrimaryClipHeight | FMath::Max(1.f, FMath::RoundToFloat(static_cast<float>(TextFont.Size) * Params.TextDualToneSplit)) | const float PrimaryClipHeight = FMath::Max(1.f, FMath::RoundToFloat(static_cast<float>(TextFont.Size) * Params.TextDualToneSplit)); |
| Source/T66/UI/Style/T66Style.cpp | 1518 | Constant | EffectiveFontSize | (Params.FontSize > 0) ? Params.FontSize : TextFont.Size | const int32 EffectiveFontSize = (Params.FontSize > 0) ? Params.FontSize : TextFont.Size; |
| Source/T66/UI/Style/T66Style.cpp | 1684 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1685 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1882 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1883 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1920 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1921 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 1934 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 1939 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 1957 | SImage | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Style/T66Style.cpp | 1960 | SBox | HeightOverride | 3.f | .HeightOverride(3.f) |
| Source/T66/UI/Style/T66Style.cpp | 1969 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Style/T66Style.cpp | 1972 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 2004 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 2005 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 2012 | SOverlay | Padding | FMargin(0.f | .Padding(FMargin(0.f)) |
| Source/T66/UI/Style/T66Style.cpp | 2019 | SOverlay | Padding | FMargin(0.f | .Padding(FMargin(0.f)) |
| Source/T66/UI/Style/T66Style.cpp | 2300 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2306 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2321 | SBorder | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Style/T66Style.cpp | 2324 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 2333 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2336 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 2357 | SBorder | Padding | bHasDecorativeBorder ? FMargin(0.f | .Padding(bHasDecorativeBorder ? FMargin(0.f) : Params.Padding) |
| Source/T66/UI/Style/T66Style.cpp | 2407 | SBox | MinDesiredWidth | Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize( | .MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 2408 | SBox | HeightOverride | Params.Height > 0.f ? Params.Height : FOptionalSize( | .HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize()) |
| Source/T66/UI/Style/T66Style.cpp | 2455 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2460 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2486 | SImage | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/Style/T66Style.cpp | 2489 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 2498 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/Style/T66Style.cpp | 2501 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/Style/T66Style.cpp | 2532 | STextBlock | Padding | 0.f, 6.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f) |
| Source/T66/UI/Style/T66Style.cpp | 2576 | SOverlay | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Style/T66Style.cpp | 2578 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2578 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2588 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Style/T66Style.cpp | 2590 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2590 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2600 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Style/T66Style.cpp | 2602 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2602 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2612 | SBorder | Padding | 5.f | .Padding(5.f) |
| Source/T66/UI/Style/T66Style.cpp | 2614 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.cpp | 2614 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/Style/T66Style.h | 48 | Unknown | SetMinWidth | 200.f | *   FT66Style::MakeButton(FT66ButtonParams(Label, OnClicked, ET66ButtonType::Primary).SetMinWidth(200.f)) |
| Source/T66/UI/Style/T66Style.h | 53 | Unknown | SetFontSize | 44 | *       .SetFontSize(44) |
| Source/T66/UI/Style/T66Style.h | 54 | Unknown | SetPadding | FMargin(14.f | *       .SetPadding(FMargin(14.f)) |
| Source/T66/UI/Style/T66Style.h | 179 | Unknown | SetPadding | FMargin(8.f | *       .SetPadding(FMargin(8.f)) |
| Source/T66/UI/Style/T66Style.h | 222 | Unknown | SetHeight | 32.f | *       .SetMinWidth(200.f).SetHeight(32.f)) |
| Source/T66/UI/Style/T66Style.h | 222 | Unknown | SetMinWidth | 200.f | *       .SetMinWidth(200.f).SetHeight(32.f)) |
| Source/T66/UI/Style/T66Style.h | 270 | Unknown | FontSize | 32 | static FSlateFontInfo MakeFont(const TCHAR* Weight, int32 Size); |
| Source/T66/UI/Style/T66Style.h | 341 | Constant | CornerRadius | 10.f | static constexpr float CornerRadius = 10.f; |
| Source/T66/UI/Style/T66Style.h | 342 | Constant | CornerRadiusSmall | 8.f | static constexpr float CornerRadiusSmall = 8.f; |
| Source/T66/UI/Style/T66Style.h | 343 | Constant | StrokeWidth | 1.f | static constexpr float StrokeWidth = 1.f; |
| Source/T66/UI/Style/T66Style.h | 344 | Constant | BorderWidth | 1.f | static constexpr float BorderWidth = 1.f;   // White outline around panels/buttons |
| Source/T66/UI/Style/T66Style.h | 348 | Constant | InventorySlotSize | 160.f | static constexpr float InventorySlotSize = 160.f;   // Inventory slots on Vendor and Gambler screens |
| Source/T66/UI/Style/T66Style.h | 349 | Constant | ItemPanelIconSize | 200.f | static constexpr float ItemPanelIconSize = 200.f;   // Vendor shop item icon; Gambler game card icon |
| Source/T66/UI/Style/T66Style.h | 350 | Constant | GameCardMinWidth | 260.f | static constexpr float GameCardMinWidth = 260.f; |
| Source/T66/UI/Style/T66Style.h | 351 | Constant | GameCardHeight | 200.f | static constexpr float GameCardHeight = 200.f; |
| Source/T66/UI/Style/T66Style.h | 353 | Constant | NPCCenterPanelTotalWidth | 920.f | static constexpr float NPCCenterPanelTotalWidth = 920.f; |
| Source/T66/UI/Style/T66Style.h | 355 | Constant | NPCRightPanelWidth | 380.f | static constexpr float NPCRightPanelWidth = 380.f; |
| Source/T66/UI/Style/T66Style.h | 357 | Constant | NPCMainRowHeight | 600.f | static constexpr float NPCMainRowHeight = 600.f; |
| Source/T66/UI/Style/T66Style.h | 359 | Constant | NPCInventoryPanelHeight | 180.f | static constexpr float NPCInventoryPanelHeight = 180.f; |
| Source/T66/UI/Style/T66Style.h | 361 | Constant | NPCGamblerInventoryPanelHeight | 252.f | static constexpr float NPCGamblerInventoryPanelHeight = 252.f;  // 180 * 1.4 |
| Source/T66/UI/Style/T66Style.h | 363 | Constant | NPCStatsPanelWidth | 300.f | static constexpr float NPCStatsPanelWidth = 300.f; |
| Source/T66/UI/Style/T66Style.h | 365 | Constant | NPCStatsPanelContentHeight | 400.f | static constexpr float NPCStatsPanelContentHeight = 400.f; |
| Source/T66/UI/Style/T66Style.h | 371 | Constant | NPCShopCardWidth | 248.f | static constexpr float NPCShopCardWidth = 248.f; |
| Source/T66/UI/Style/T66Style.h | 372 | Constant | NPCShopCardHeight | 500.f | static constexpr float NPCShopCardHeight = 500.f; |
| Source/T66/UI/Style/T66Style.h | 374 | Constant | NPCAngerCircleSize | 170.f | static constexpr float NPCAngerCircleSize = 170.f; |
| Source/T66/UI/Style/T66Style.h | 376 | Constant | NPCBankSpinBoxWidth | 110.f | static constexpr float NPCBankSpinBoxWidth = 110.f; |
| Source/T66/UI/Style/T66Style.h | 378 | Constant | NPCBankSpinBoxHeight | 44.f | static constexpr float NPCBankSpinBoxHeight = 44.f; |
| Source/T66/UI/Style/T66Style.h | 380 | Constant | NPCOverlayPadding | 24.f | static constexpr float NPCOverlayPadding = 24.f; |
| Source/T66/UI/Style/T66Style.h | 394 | Constant | ButtonGlowPadding | 6.f | static constexpr float ButtonGlowPadding = 6.f; |
| Source/T66/UI/Style/T66Style.h | 400 | Unknown | FontSize | 32 | static FSlateFontInfo FontRegular(int32 Size); |
| Source/T66/UI/Style/T66Style.h | 401 | Unknown | FontSize | 32 | static FSlateFontInfo FontBold(int32 Size); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 147 | SBorder | Padding | FMargin(12.f, 10.f | .Padding(FMargin(12.f, 10.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 150 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 153 | SBox | WidthOverride | 72.f | .WidthOverride(72.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 154 | SBox | HeightOverride | 72.f | .HeightOverride(72.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 166 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 231 | STextBlock | ImageSize | FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize | AlchemyInventorySlotBrushes[Index]->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 238 | STextBlock | ImageSize | FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize | AlchemyTargetIconBrush->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 244 | STextBlock | ImageSize | FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize | AlchemySacrificeIconBrush->ImageSize = FVector2D(FT66Style::Tokens::InventorySlotSize, FT66Style::Tokens::InventorySlotSize); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 277 | SWidgetSwitcher | Padding | FMargin(24.f | .Padding(FMargin(24.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 281 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 288 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 291 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 298 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 301 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 308 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 318 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 321 | SHorizontalBox | SetPadding | FMargin(12.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 382 | Constant | InventorySlotSize | FT66Style::Tokens::InventorySlotSize | const float InventorySlotSize = FT66Style::Tokens::InventorySlotSize; |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 383 | Constant | UpgradePanelWidth | InventorySlotSize + 56.f | const float UpgradePanelWidth = InventorySlotSize + 56.f; |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 389 | SUniformGridPanel | SlotPadding | FMargin(FT66Style::Tokens::Space2, 0.f | .SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f)); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 404 | SUniformGridPanel | SetPadding | FMargin(0.f | .SetPadding(FMargin(0.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 414 | SImage | Padding | 0.f, 6.f, 8.f, 0.f | + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 418 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 420 | STextBlock | ShadowOffset | 1.f, 1.f | .ShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 428 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 431 | STextBlock | SetPadding | FMargin(0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(0.f)), |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 453 | SBox | SetPadding | FMargin(18.f, 12.f | .SetPadding(FMargin(18.f, 12.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 459 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 463 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 466 | STextBlock | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 490 | SImage | Padding | 0.f, 6.f, 8.f, 0.f | + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 494 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 496 | STextBlock | ShadowOffset | 1.f, 1.f | .ShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 502 | STextBlock | Padding | 0.f, 12.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 506 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 511 | STextBlock | SetPadding | FMargin(18.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(18.f)), |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 515 | STextBlock | Padding | 20.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 519 | STextBlock | FontSize | 42 | .Font(FT66Style::Tokens::FontBold(42)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 544 | SImage | Padding | 0.f, 12.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 548 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 553 | STextBlock | SetPadding | FMargin(18.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(18.f)), |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 556 | STextBlock | Padding | 0.f, 14.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 563 | STextBlock | Padding | 0.f, 0.f, 0.f, 18.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 567 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 583 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 591 | SSpacer | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 601 | SScrollBox | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 603 | SScrollBox | SetPadding | FMargin(28.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(28.f)) |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 942 | SScrollBox | ImageSize | 72.f, 72.f | DragIconBrush->ImageSize = FVector2D(72.f, 72.f); |
| Source/T66/UI/T66CasinoOverlayWidget.cpp | 947 | SScrollBox | ImageSize | 72.f, 72.f | DragIconBrush->ImageSize = FVector2D(72.f, 72.f); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 147 | SBorder | Padding | FMargin(12.f, 10.f | .Padding(FMargin(12.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 150 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 153 | SBox | WidthOverride | 72.f | .WidthOverride(72.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 154 | SBox | HeightOverride | 72.f | .HeightOverride(72.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 166 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 233 | STextBlock | ImageSize | 56.f, 56.f | AlchemyInventorySlotBrushes[Index]->ImageSize = FVector2D(56.f, 56.f); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 239 | STextBlock | ImageSize | 76.f, 76.f | AlchemyTargetIconBrush->ImageSize = FVector2D(76.f, 76.f); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 244 | STextBlock | ImageSize | 76.f, 76.f | AlchemySacrificeIconBrush->ImageSize = FVector2D(76.f, 76.f); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 265 | STextBlock | FontSize | 15 | .Font(FT66Style::Tokens::FontBold(15)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 268 | STextBlock | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 272 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 276 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 283 | STextBlock | FontSize | 15 | .Font(FT66Style::Tokens::FontBold(15)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 286 | STextBlock | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 290 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 294 | STextBlock | SetPadding | FMargin(12.f, 10.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 301 | SVerticalBox | Padding | 24.f, 24.f, 24.f, 0.f | .Padding(24.f, 24.f, 24.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 316 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 323 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 333 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 336 | SHorizontalBox | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 343 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 358 | SSpacer | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 361 | SSpacer | SetPadding | FMargin(16.f, 12.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(16.f, 12.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 365 | SSpacer | Padding | 24.f, 16.f, 24.f, 24.f | .Padding(24.f, 16.f, 24.f, 24.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 381 | SWidgetSwitcher | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f).SetColor(FT66Style::Tokens::Bg) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 446 | SUniformGridPanel | SlotPadding | FMargin(8.f | .SlotPadding(FMargin(8.f)); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 455 | SCircusAlchemyInventoryTile | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 463 | SBox | WidthOverride | 84.f | .WidthOverride(84.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 464 | SBox | HeightOverride | 84.f | .HeightOverride(84.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 473 | SImage | Padding | 0.f, 4.f, 6.f, 0.f | + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 4.f, 6.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 477 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 479 | STextBlock | ShadowOffset | 1.f, 1.f | .ShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 487 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 512 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 516 | STextBlock | FontSize | 34 | .Font(FT66Style::Tokens::FontBold(34)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 519 | STextBlock | Padding | 0.f, 0.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 523 | SHorizontalBox | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 527 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 530 | STextBlock | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 534 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 537 | STextBlock | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 541 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 548 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 551 | STextBlock | SetPadding | FMargin(14.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 554 | STextBlock | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 557 | SHorizontalBox | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 561 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 565 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 573 | SCircusAlchemyDropBorder | Padding | FMargin(12.f | .Padding(FMargin(12.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 577 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 580 | SBox | WidthOverride | 76.f | .WidthOverride(76.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 581 | SBox | HeightOverride | 76.f | .HeightOverride(76.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 592 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 598 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 605 | STextBlock | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 608 | STextBlock | SetPadding | FMargin(14.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 611 | STextBlock | Padding | 0.f, 32.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 32.f, 16.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 615 | STextBlock | FontSize | 30 | .Font(FT66Style::Tokens::FontBold(30)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 622 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 626 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 634 | SCircusAlchemyDropBorder | Padding | FMargin(12.f | .Padding(FMargin(12.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 637 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f).VAlign(VAlign_Center) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 640 | SBox | WidthOverride | 76.f | .WidthOverride(76.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 641 | SBox | HeightOverride | 76.f | .HeightOverride(76.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 652 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 658 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 662 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 665 | STextBlock | SetPadding | FMargin(14.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 669 | STextBlock | Padding | 0.f, 0.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 673 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 676 | STextBlock | Padding | 0.f, 0.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 683 | STextBlock | SetPadding | FMargin(18.f, 12.f | .SetPadding(FMargin(18.f, 12.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 686 | STextBlock | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 690 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 697 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 701 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 708 | STextBlock | SetPadding | FMargin(14.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 711 | STextBlock | SetPadding | FMargin(28.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(28.f)) |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 733 | Constant | Minutes | TotalSeconds / 60 | const int32 Minutes = TotalSeconds / 60; |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 1038 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 1117 | STextBlock | ImageSize | 72.f, 72.f | DragIconBrush->ImageSize = FVector2D(72.f, 72.f); |
| Source/T66/UI/T66CircusOverlayWidget.cpp | 1122 | STextBlock | ImageSize | 72.f, 72.f | DragIconBrush->ImageSize = FVector2D(72.f, 72.f); |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 140 | Unknown | SetFontSize | 11 | .SetMinWidth(0.f).SetFontSize(11).SetPadding(FMargin(12.f, 6.f))); |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 140 | Unknown | SetMinWidth | 0.f | .SetMinWidth(0.f).SetFontSize(11).SetPadding(FMargin(12.f, 6.f))); |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 140 | Unknown | SetPadding | FMargin(12.f, 6.f | .SetMinWidth(0.f).SetFontSize(11).SetPadding(FMargin(12.f, 6.f))); |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 144 | SHorizontalBox | Padding | 4.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabItems, 0)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 145 | SHorizontalBox | Padding | 4.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabNPCs, 1)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 146 | SHorizontalBox | Padding | 4.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabEnemies, 2)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 147 | SHorizontalBox | Padding | 4.f | + SHorizontalBox::Slot().AutoWidth().Padding(4.f)[MakeTab(TabInteractables, 3)]; |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 157 | SScrollBox | ImageSize | 64.f, 64.f | IconBrush->ImageSize = FVector2D(64.f, 64.f); |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 166 | SScrollBox | Padding | 6.f | Scroll->AddSlot().Padding(6.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 170 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 172 | SBox | HeightOverride | 64.f | SNew(SBox).WidthOverride(64.f).HeightOverride(64.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 172 | SBox | WidthOverride | 64.f | SNew(SBox).WidthOverride(64.f).HeightOverride(64.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 180 | STextBlock | FontSize | 12 | + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NameText).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FT66Style::Tokens::Text)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 181 | STextBlock | FontSize | 10 | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 181 | STextBlock | Padding | 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 182 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 187 | STextBlock | SetPadding | 8.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(8.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 194 | STextBlock | Padding | 6.f | Scroll->AddSlot().Padding(6.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 201 | STextBlock | FontSize | 12 | + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NameText).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FT66Style::Tokens::Text)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 202 | STextBlock | FontSize | 10 | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 202 | STextBlock | Padding | 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)[SNew(STextBlock).Text(DescText).Font(FT66Style::Tokens::FontRegular(10)).ColorAndOpacity(FT66Style::Tokens::TextMuted).AutoWrapText(true)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 203 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 208 | STextBlock | SetPadding | 8.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(8.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 259 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 260 | STextBlock | FontSize | 20 | [SNew(STextBlock).Text(TitleCollector).Font(FT66Style::Tokens::FontBold(20)).ColorAndOpacity(FT66Style::Tokens::Text)] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 261 | STextBlock | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[TabRow] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 262 | SBox | HeightOverride | 400.f | + SVerticalBox::Slot().FillHeight(1.f)[SNew(SBox).HeightOverride(400.f)[Scroll]] |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 263 | SBox | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 266 | SHorizontalBox | Padding | 0.f, 0.f, 8.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 281 | SBorder | Padding | 40.f | + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(40.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 283 | SBox | WidthOverride | 520.f | SNew(SBox).WidthOverride(520.f) |
| Source/T66/UI/T66CollectorOverlayWidget.cpp | 285 | SBox | SetPadding | 24.f | FT66Style::MakePanel(MainPanel, FT66PanelParams(ET66PanelType::Panel).SetPadding(24.f)) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 38 | SBorder | Padding | 22.f | .Padding(22.f) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 41 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 45 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 48 | STextBlock | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 52 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 58 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66CowardicePromptWidget.cpp | 65 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 198 | SHorizontalBox | Padding | ItemTileGap * 0.5f, 0.f | .Padding(ItemTileGap * 0.5f, 0.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 207 | SBorder | Padding | FMargin(2.f | .Padding(FMargin(2.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 215 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 223 | SBorder | Padding | FMargin(0.f | .Padding(FMargin(0.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 245 | SBorder | Padding | FMargin(8.f, 6.f | .Padding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 252 | STextBlock | FontSize | 11 | .Font(FT66Style::Tokens::FontBold(11)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 257 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 261 | STextBlock | FontSize | 9 | .Font(FT66Style::Tokens::FontRegular(9)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 276 | SOverlay | Padding | 24.f, 0.f, 24.f, 520.f | .Padding(24.f, 0.f, 24.f, 520.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 280 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 284 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 287 | STextBlock | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 294 | SBox | HeightOverride | ItemTileHeight + 8.f | .HeightOverride(ItemTileHeight + 8.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 299 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 310 | SBox | WidthOverride | ItemTileWidth + 4.f | .WidthOverride(ItemTileWidth + 4.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 311 | SBox | HeightOverride | ItemTileHeight + 8.f | .HeightOverride(ItemTileHeight + 8.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 316 | SBorder | Padding | FMargin(0.f | .Padding(FMargin(0.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 324 | SBox | WidthOverride | 3.f | .WidthOverride(3.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 325 | SBox | HeightOverride | ItemTileHeight + 8.f | .HeightOverride(ItemTileHeight + 8.f) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 338 | SBorder | Padding | FMargin(8.f, 4.f | .Padding(FMargin(8.f, 4.f)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 342 | STextBlock | FontSize | 10 | .Font(FT66Style::Tokens::FontRegular(10)) |
| Source/T66/UI/T66CrateOverlayWidget.cpp | 347 | STextBlock | SetPadding | FMargin(14.f, 14.f, 14.f, 10.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f, 14.f, 14.f, 10.f)) |
| Source/T66/UI/T66CrateOverlayWidget.h | 56 | Constant | ItemTileWidth | 132.f | static constexpr float ItemTileWidth = 132.f; |
| Source/T66/UI/T66CrateOverlayWidget.h | 57 | Constant | ItemTileHeight | 164.f | static constexpr float ItemTileHeight = 164.f; |
| Source/T66/UI/T66CrateOverlayWidget.h | 58 | Constant | ItemTileGap | 8.f | static constexpr float ItemTileGap = 8.f; |
| Source/T66/UI/T66CrateOverlayWidget.h | 60 | Constant | ItemPreviewSize | 92.f | static constexpr float ItemPreviewSize = 92.f; |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 28 | Unknown | ComputeDesiredSize | 14.f, 14.f | return FVector2D(14.f, 14.f); |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 35 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 37 | Constant | Radius | (MinDim * 0.5f) - 0.75f | const float Radius = (MinDim * 0.5f) - 0.75f; |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 107 | SBox | HeightOverride | BarHeight + 14.f | .HeightOverride(BarHeight + 14.f) |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 117 | SBox | WidthOverride | 14.f | .WidthOverride(14.f) |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 118 | SBox | HeightOverride | 14.f | .HeightOverride(14.f) |
| Source/T66/UI/T66EnemyHealthBarWidget.cpp | 132 | SBorder | Padding | FMargin(1.f | .Padding(FMargin(1.f)) |
| Source/T66/UI/T66EnemyHealthBarWidget.h | 28 | Constant | BarWidth | 90.f | static constexpr float BarWidth = 90.f; |
| Source/T66/UI/T66EnemyHealthBarWidget.h | 29 | Constant | BarHeight | 10.f | static constexpr float BarHeight = 10.f; |
| Source/T66/UI/T66EnemyLockWidget.cpp | 23 | Unknown | ComputeDesiredSize | 44.f, 44.f | return FVector2D(44.f, 44.f); |
| Source/T66/UI/T66EnemyLockWidget.cpp | 96 | SBox | WidthOverride | 44.f | .WidthOverride(44.f) |
| Source/T66/UI/T66EnemyLockWidget.cpp | 97 | SBox | HeightOverride | 44.f | .HeightOverride(44.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 39 | Constant | GTopBarReservedHeight | 146.f | constexpr float GTopBarReservedHeight = 146.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 40 | Constant | GTopBarButtonWidth | 92.f | constexpr float GTopBarButtonWidth = 92.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 41 | Constant | GTopBarButtonHeight | 78.f | constexpr float GTopBarButtonHeight = 78.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 42 | Constant | GTopBarSurfaceHeight | 118.f | constexpr float GTopBarSurfaceHeight = 118.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 43 | Constant | GTopBarPlateVisibleHeightRatio | 98.f / 108.f | constexpr float GTopBarPlateVisibleHeightRatio = 98.f / 108.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 44 | Constant | GTopBarInactiveNavHeight | 72.f | constexpr float GTopBarInactiveNavHeight = 72.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 45 | Constant | GTopBarActiveNavHeight | 80.f | constexpr float GTopBarActiveNavHeight = 80.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 46 | Constant | GTopBarInactiveNavMinWidth | 176.f | constexpr float GTopBarInactiveNavMinWidth = 176.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 47 | Constant | GTopBarActiveNavMinWidth | 188.f | constexpr float GTopBarActiveNavMinWidth = 188.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 48 | Constant | GTopBarIconOnlyNavWidth | 80.f | constexpr float GTopBarIconOnlyNavWidth = 80.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 49 | Constant | GTopBarNavSeparatorHorizontalPadding | 10.f | constexpr float GTopBarNavSeparatorHorizontalPadding = 10.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 50 | Constant | GTopBarRightClusterGap | 12.f | constexpr float GTopBarRightClusterGap = 12.f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 51 | Constant | GTopBarViewportZOrder | 50 | constexpr int32 GTopBarViewportZOrder = 50; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 147 | Constant | NextWidth | FMath::Max(1, SourceWidth / 2) | const int32 NextWidth = FMath::Max(1, SourceWidth / 2); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 148 | Constant | NextHeight | FMath::Max(1, SourceHeight / 2) | const int32 NextHeight = FMath::Max(1, SourceHeight / 2); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 240 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 243 | Constant | ToothThickness | FMath::Max(3.f, MinDim * 0.10f) | const float ToothThickness = FMath::Max(3.f, MinDim * 0.10f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 244 | Constant | OuterRadius | MinDim * 0.27f | const float OuterRadius = MinDim * 0.27f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 245 | Constant | InnerRadius | MinDim * 0.17f | const float InnerRadius = MinDim * 0.17f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 246 | Constant | CoreRadius | MinDim * 0.08f | const float CoreRadius = MinDim * 0.08f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 336 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 337 | Constant | Radius | MinDim * 0.36f | const float Radius = MinDim * 0.36f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 338 | Constant | OuterThickness | FMath::Max(5.f, MinDim * 0.12f) | const float OuterThickness = FMath::Max(5.f, MinDim * 0.12f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 339 | Constant | InnerThickness | FMath::Max(2.f, MinDim * 0.05f) | const float InnerThickness = FMath::Max(2.f, MinDim * 0.05f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 434 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 440 | Constant | OuterRadius | MinDim * 0.45f | const float OuterRadius = MinDim * 0.45f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 441 | Constant | InnerRadius | MinDim * 0.33f | const float InnerRadius = MinDim * 0.33f; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 544 | Constant | WidthScale | EffectiveViewportSize.X / FMath::Max(ReferenceViewportSize.X, 1.f) | const float WidthScale = EffectiveViewportSize.X / FMath::Max(ReferenceViewportSize.X, 1.f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 545 | Constant | HeightScale | EffectiveViewportSize.Y / FMath::Max(ReferenceViewportSize.Y, 1.f) | const float HeightScale = EffectiveViewportSize.Y / FMath::Max(ReferenceViewportSize.Y, 1.f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 546 | Constant | Scale | FMath::Clamp(FMath::Sqrt(WidthScale * HeightScale), 0.70f, 3.0f) | const float Scale = FMath::Clamp(FMath::Sqrt(WidthScale * HeightScale), 0.70f, 3.0f); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 872 | Constant | TopBarTitleFontSize | 16 | constexpr int32 TopBarTitleFontSize = 16; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 875 | Unknown | LetterSpacing | 14 | NavFont.LetterSpacing = 14; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 878 | Unknown | LetterSpacing | 16 | ActiveNavFont.LetterSpacing = 16; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 881 | Unknown | LetterSpacing | 8 | BalanceFont.LetterSpacing = 8; |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 949 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | .Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 974 | Constant | IconOnlyNavWidth | FMath::Max(TopBarButtonWidth, LanguageIconSize.X + (28.f * LayoutScale)) | const float IconOnlyNavWidth = FMath::Max(TopBarButtonWidth, LanguageIconSize.X + (28.f * LayoutScale)); |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1013 | SButton | ContentPadding | bIconOnly ? FMargin(18.f * LayoutScale | .ContentPadding(bIconOnly ? FMargin(18.f * LayoutScale) : FMargin(24.f * LayoutScale, 12.f * LayoutScale, 24.f * LayoutScale, 12.f * LayoutScale)) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1062 | SButton | ContentPadding | FMargin(0.f | .ContentPadding(FMargin(0.f)) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1097 | SHorizontalBox | Padding | 8.f * LayoutScale, 0.f, 0.f, 0.f | .Padding(8.f * LayoutScale, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1142 | SHorizontalBox | Padding | UtilityClusterGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(UtilityClusterGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1165 | SHorizontalBox | Padding | NavButtonGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1179 | SHorizontalBox | Padding | NavButtonGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1183 | SHorizontalBox | Padding | NavButtonGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1194 | SHorizontalBox | Padding | ReadoutGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(ReadoutGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1198 | SHorizontalBox | Padding | QuitGap, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(QuitGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1215 | SHorizontalBox | Padding | ClusterGap, 0.f | .Padding(ClusterGap, 0.f) |
| Source/T66/UI/T66FrontendTopBarWidget.cpp | 1244 | SImage | Padding | RowHorizontalPadding, RowTopPadding, RowHorizontalPadding, 0.f | .Padding(RowHorizontalPadding, RowTopPadding, RowHorizontalPadding, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 122 | Constant | AngerFaceSize | FT66Style::Tokens::NPCAngerCircleSize | const float AngerFaceSize = FT66Style::Tokens::NPCAngerCircleSize; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 145 | Constant | GameCardIconSize | 260.f | static constexpr float GameCardIconSize = 260.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 159 | Unknown | ImageSize | 80.f, 112.f | BlackJackCardBackBrush.ImageSize = FVector2D(80.f, 112.f); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 195 | Constant | CoinSpriteSize | 180.f | static constexpr float CoinSpriteSize = 180.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 215 | Constant | RpsHandSize | 160.f | static constexpr float RpsHandSize = 160.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 263 | Constant | InvSlotSz | FT66Style::Tokens::InventorySlotSize | const float InvSlotSz = FT66Style::Tokens::InventorySlotSize; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 281 | SVerticalBox | Padding | 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 283 | SVerticalBox | Padding | 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 296 | STextBlock | SetMinWidth | 420.f | .SetMinWidth(420.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 297 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 305 | STextBlock | SetMinWidth | 420.f | .SetMinWidth(420.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 306 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 318 | Constant | GamblerAngerCircleSize | 260.f | static constexpr float GamblerAngerCircleSize = 260.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 323 | SVerticalBox | Padding | 0.f, 6.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 339 | SSpacer | Padding | 0.f, 0.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 343 | SVerticalBox | Padding | 0.f, 0.f, 0.f, FT66Style::Tokens::Space4 | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 350 | STextBlock | Padding | 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 353 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 356 | SBox | WidthOverride | FT66Style::Tokens::NPCBankSpinBoxWidth | .WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 357 | SBox | HeightOverride | FT66Style::Tokens::NPCBankSpinBoxHeight | .HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 361 | SSpinBox<int32> | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 386 | SSpinBox<int32> | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 387 | SSpinBox<int32> | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 390 | SSpinBox<int32> | Padding | 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 393 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 396 | SBox | WidthOverride | FT66Style::Tokens::NPCBankSpinBoxWidth | .WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 397 | SBox | HeightOverride | FT66Style::Tokens::NPCBankSpinBoxHeight | .HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 401 | SSpinBox<int32> | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 412 | SSpinBox<int32> | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 413 | SSpinBox<int32> | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 417 | SSpinBox<int32> | SetPadding | FT66Style::Tokens::Space5 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space5)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 420 | SSpinBox<int32> | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 423 | Constant | GameCardTotalHeight | 420.f | static constexpr float GameCardTotalHeight = 420.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 429 | SSpinBox<int32> | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 430 | SSpinBox<int32> | SetPadding | FMargin(8.f, 6.f | .SetPadding(FMargin(8.f, 6.f))); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 434 | SBox | Padding | FMargin(4.f, 0.f | .Padding(FMargin(4.f, 0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 453 | SImage | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 457 | SImage | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 465 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 470 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 489 | SUniformGridPanel | SlotPadding | FMargin(16.f, 8.f | .SlotPadding(FMargin(16.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 495 | SUniformGridPanel | Padding | 0.f, 28.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 28.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 499 | SUniformGridPanel | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 500 | SUniformGridPanel | SetPadding | FMargin(20.f, 12.f | .SetPadding(FMargin(20.f, 12.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 512 | SUniformGridPanel | SlotPadding | FMargin(16.f, 8.f | .SlotPadding(FMargin(16.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 518 | SUniformGridPanel | Padding | 0.f, FT66Style::Tokens::Space6, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 522 | SUniformGridPanel | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 523 | SUniformGridPanel | SetPadding | FMargin(20.f, 12.f | .SetPadding(FMargin(20.f, 12.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 537 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 545 | SHorizontalBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 545 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 565 | STextBlock | Padding | 0.f, 10.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 572 | STextBlock | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 579 | STextBlock | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 582 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 596 | SImage | Padding | 0.f, 0.f, 0.f, 6.f | + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 603 | STextBlock | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 609 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 615 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 616 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 618 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 624 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 625 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 631 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 639 | SHorizontalBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 639 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 659 | STextBlock | Padding | 0.f, 10.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 666 | STextBlock | Padding | 0.f, 0.f, 0.f, 16.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 669 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 674 | SVerticalBox | Padding | 0.f, 6.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 6.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 688 | SImage | Padding | 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(16.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 713 | STextBlock | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space6)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 719 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 725 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 726 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 728 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 734 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 735 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 737 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 743 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 744 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 749 | Constant | BJMaxCards | 11 | static constexpr int32 BJMaxCards = 11; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 766 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 786 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 788 | STextBlock | Padding | 0.f, 0.f, 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 809 | SHorizontalBox | Padding | 4.f, 0.f | BlackJackDealerCardsRow->AddSlot().AutoWidth().Padding(4.f, 0.f)[ MakeCardImageBox(BlackJackDealerCardImages[i]) ]; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 814 | SHorizontalBox | Padding | 4.f, 0.f | BlackJackPlayerCardsRow->AddSlot().AutoWidth().Padding(4.f, 0.f)[ MakeCardImageBox(BlackJackPlayerCardImages[i]) ]; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 818 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 826 | SHorizontalBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 826 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 846 | STextBlock | Padding | 0.f, 10.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 853 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 859 | SHorizontalBox | Padding | 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 862 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 876 | SHorizontalBox | Padding | 0.f, 0.f, 12.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 881 | STextBlock | Padding | 0.f, 16.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 16.f, 0.f, 12.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 883 | SBox | HeightOverride | FT66Style::Tokens::NPCBankSpinBoxHeight | SNew(SBox).HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 894 | SHorizontalBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 902 | SBox | SetMinWidth | 0.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 902 | SBox | SetPadding | FMargin(14.f, 8.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 905 | SBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 913 | SBox | SetMinWidth | 0.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 913 | SBox | SetPadding | FMargin(14.f, 8.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 916 | SBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 924 | SBox | SetMinWidth | 0.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 924 | SBox | SetPadding | FMargin(14.f, 8.f | ET66ButtonType::Primary).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 927 | SBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 935 | SBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 935 | SBox | SetPadding | FMargin(14.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 938 | SBox | Padding | 6.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(6.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 946 | SBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 946 | SBox | SetPadding | FMargin(14.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 960 | SHorizontalBox | SetMinWidth | 0.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 960 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 974 | SHorizontalBox | Padding | i > 0 ? FMargin(FT66Style::Tokens::Space2, 0.f, 0.f, 0.f | .Padding(i > 0 ? FMargin(FT66Style::Tokens::Space2, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 979 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f).SetPadding(FMargin(10.f, 8.f))), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 979 | SHorizontalBox | SetPadding | FMargin(10.f, 8.f | .SetMinWidth(0.f).SetPadding(FMargin(10.f, 8.f))), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 980 | SHorizontalBox | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 987 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 988 | SVerticalBox | Padding | 0.f, 8.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 990 | STextBlock | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 992 | STextBlock | Padding | 0.f, 8.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) [ LotteryNumberRow ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 993 | STextBlock | Padding | 0.f, 4.f, 0.f, 2.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 995 | STextBlock | Padding | 0.f, 4.f, 0.f, 2.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 2.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 997 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1004 | Constant | PlinkoPinSize | 8.f | static constexpr float PlinkoPinSize = 8.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1005 | Constant | PlinkoPinSpacing | 12.f | static constexpr float PlinkoPinSpacing = 12.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1006 | Constant | PlinkoRowH | 20.f | static constexpr float PlinkoRowH = 20.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1007 | Constant | PlinkoBallSize | 14.f | static constexpr float PlinkoBallSize = 14.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1012 | SBorder | Padding | 0.f | [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.55f, 0.65f, 1.f)).Padding(0.f)[ SNew(SSpacer) ] ]; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1022 | SHorizontalBox | Padding | j > 0 ? FMargin(PlinkoPinSpacing * 0.5f, 0.f, PlinkoPinSpacing * 0.5f, 0.f | Row->AddSlot().AutoWidth().Padding(j > 0 ? FMargin(PlinkoPinSpacing * 0.5f, 0.f, PlinkoPinSpacing * 0.5f, 0.f) : FMargin(0.f))[ MakePinDot() ]; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1032 | SHorizontalBox | Padding | 2.f, 4.f | .Padding(2.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1036 | SBorder | Padding | 4.f | .Padding(4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1046 | SBox | HeightOverride | 36.f | PinGrid->AddSlot().AutoHeight()[ SNew(SBox).HeightOverride(36.f)[ SlotRow ] ]; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1051 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1058 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1059 | SVerticalBox | Padding | 0.f, 8.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1061 | STextBlock | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1063 | STextBlock | Padding | 0.f, 8.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1070 | SOverlay | SetPadding | 4.f | + SOverlay::Slot()[ FT66Style::MakePanel(PinGrid, FT66PanelParams(ET66PanelType::Panel2).SetPadding(4.f)) ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1076 | SOverlay | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1082 | Constant | BoxSquareSize | 48.f | static constexpr float BoxSquareSize = 48.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1083 | Constant | BoxSquareGap | 4.f | static constexpr float BoxSquareGap = 4.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1090 | SHorizontalBox | Padding | Idx > 0 ? FMargin(BoxSquareGap, 0.f, 0.f, 0.f | .Padding(Idx > 0 ? FMargin(BoxSquareGap, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1096 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1105 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) [ MakeGameBackRow() ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1106 | SVerticalBox | Padding | 0.f, 8.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1108 | STextBlock | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1110 | STextBlock | Padding | 0.f, 12.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 4.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1113 | SBox | WidthOverride | 5 * BoxSquareSize + 4 * BoxSquareGap | .WidthOverride(5 * BoxSquareSize + 4 * BoxSquareGap) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1114 | SBox | HeightOverride | BoxSquareSize + 8.f | .HeightOverride(BoxSquareSize + 8.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1123 | SBox | Padding | 2.f * (BoxSquareSize + BoxSquareGap | .Padding(2.f * (BoxSquareSize + BoxSquareGap), 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1127 | SBorder | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1130 | SBox | HeightOverride | BoxSquareSize + 2.f | SNew(SBox).WidthOverride(BoxSquareSize + 2.f).HeightOverride(BoxSquareSize + 2.f)[ SNew(SSpacer) ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1130 | SBox | WidthOverride | BoxSquareSize + 2.f | SNew(SBox).WidthOverride(BoxSquareSize + 2.f).HeightOverride(BoxSquareSize + 2.f)[ SNew(SSpacer) ] |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1135 | SBox | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1140 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1142 | STextBlock | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1146 | SSpinBox<int32> | FontSize | 26 | .Font(FT66Style::Tokens::FontBold(26)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1156 | SSpinBox<int32> | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1157 | SSpinBox<int32> | SetPadding | FMargin(14.f, 8.f | .SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1174 | SWidgetSwitcher | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1192 | SBox | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1195 | Constant | BuybackSlotCount | UT66RunStateSubsystem::BuybackDisplaySlotCount | static constexpr int32 BuybackSlotCount = UT66RunStateSubsystem::BuybackDisplaySlotCount; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1196 | Constant | BuybackCardSize | FT66Style::Tokens::NPCShopCardWidth | const float BuybackCardSize = FT66Style::Tokens::NPCShopCardWidth; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1197 | Constant | BuybackCardHeight | FT66Style::Tokens::NPCShopCardHeight | const float BuybackCardHeight = FT66Style::Tokens::NPCShopCardHeight; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1198 | Constant | BuybackIconSize | BuybackCardSize - FT66Style::Tokens::Space4 * 2.f | const float BuybackIconSize = BuybackCardSize - FT66Style::Tokens::Space4 * 2.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1221 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1222 | SHorizontalBox | SetPadding | FMargin(8.f, 6.f | .SetPadding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1226 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1233 | STextBlock | Padding | i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f | .Padding(i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1244 | SBox | HeightOverride | 60.f | .HeightOverride(60.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1251 | STextBlock | WrapTextAt | BuybackCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(BuybackCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1254 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1268 | SImage | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1272 | SImage | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1279 | STextBlock | WrapTextAt | BuybackCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(BuybackCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1281 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1286 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1295 | SVerticalBox | Padding | 0.f, 0.f, 0.f, FT66Style::Tokens::Space4 | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1298 | SHorizontalBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space4, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1305 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1305 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | .SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1317 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1317 | SHorizontalBox | SetPadding | FMargin(12.f, 8.f | .SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1321 | SHorizontalBox | Padding | 0.f, FT66Style::Tokens::Space4, 0.f, 0.f | + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1337 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1347 | SScrollBox | Padding | 0.f, FT66Style::Tokens::Space6, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1357 | SScrollBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1358 | SScrollBox | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1368 | SScrollBox | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1377 | SScrollBox | SetMinWidth | FT66Style::Tokens::InventorySlotSize | .SetMinWidth(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1378 | SScrollBox | SetHeight | FT66Style::Tokens::InventorySlotSize | .SetHeight(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1379 | SScrollBox | SetPadding | FMargin(0.f | .SetPadding(FMargin(0.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1389 | SImage | Padding | 0.f, 6.f, 8.f, 0.f | + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1393 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1395 | STextBlock | ShadowOffset | 1.f, 1.f | .ShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1407 | STextBlock | SetPadding | FMargin(0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(0.f)), |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1418 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1419 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1423 | SUniformGridPanel | SlotPadding | FMargin(FT66Style::Tokens::Space2, 0.f | .SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1429 | SBox | WidthOverride | FT66Style::Tokens::InventorySlotSize | .WidthOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1430 | SBox | HeightOverride | FT66Style::Tokens::InventorySlotSize | .HeightOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1450 | STextBlock | Padding | 18.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1457 | STextBlock | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1476 | SSpacer | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1489 | SScrollBox | Padding | FT66Style::Tokens::Space6, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1492 | SBox | WidthOverride | FT66Style::Tokens::InventorySlotSize | .WidthOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1493 | SBox | HeightOverride | FT66Style::Tokens::InventorySlotSize | .HeightOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1505 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1513 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1520 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1525 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1530 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4).SetColor(FT66Style::Tokens::Panel)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1534 | SVerticalBox | Padding | 0.f, 0.f, 0.f, FT66Style::Tokens::Space4 | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1541 | STextBlock | Padding | 0.f, 0.f, 0.f, FT66Style::Tokens::Space6 | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space6) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1544 | SHorizontalBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space6, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1547 | SBox | WidthOverride | FT66Style::Tokens::NPCGamblerStatsPanelWidth | .WidthOverride(FT66Style::Tokens::NPCGamblerStatsPanelWidth) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1548 | SBox | HeightOverride | FT66Style::Tokens::NPCMainRowHeight | .HeightOverride(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1553 | SBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space6, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1556 | SBox | MinDesiredHeight | FT66Style::Tokens::NPCMainRowHeight | .MinDesiredHeight(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1561 | SBox | Padding | 0.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1564 | SBox | WidthOverride | FT66Style::Tokens::NPCRightPanelWidth | .WidthOverride(FT66Style::Tokens::NPCRightPanelWidth) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1565 | SBox | MinDesiredHeight | FT66Style::Tokens::NPCMainRowHeight | .MinDesiredHeight(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1571 | SBox | Padding | 0.f, 0.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1590 | SVerticalBox | Padding | FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, FT66Style::Tokens::Space2 | + SVerticalBox::Slot().AutoHeight().Padding(FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, FT66Style::Tokens::Space2) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1601 | STextBlock | Padding | 40.f, 16.f, 40.f, 0.f | + SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Fill).VAlign(VAlign_Top).Padding(40.f, 16.f, 40.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1613 | SWidgetSwitcher | Padding | FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, 0.f | .Padding(FT66Style::Tokens::Space6, FT66Style::Tokens::Space4, FT66Style::Tokens::Space6, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1619 | SWidgetSwitcher | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1620 | SWidgetSwitcher | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1635 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1642 | STextBlock | Padding | 0.f, 0.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1652 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1658 | SHorizontalBox | SetMinWidth | 140.f | .SetMinWidth(140.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1659 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1661 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1667 | SHorizontalBox | SetMinWidth | 140.f | .SetMinWidth(140.f) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1668 | SHorizontalBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f))) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1672 | SHorizontalBox | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)) |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 1676 | SHorizontalBox | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f).SetColor(FT66Style::Tokens::Bg)); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 2387 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 2649 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 3262 | Constant | MaxCards | FMath::Min(11, BlackJackDealerCardImages.Num()) | const int32 MaxCards = FMath::Min(11, BlackJackDealerCardImages.Num()); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 3285 | Constant | MaxPlayer | FMath::Min(11, BlackJackPlayerCardImages.Num()) | const int32 MaxPlayer = FMath::Min(11, BlackJackPlayerCardImages.Num()); |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 3450 | Constant | MaxCards | 11 | constexpr int32 MaxCards = 11; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 3730 | Constant | PlinkoRowH | 20.f | static constexpr float PlinkoRowH = 20.f; |
| Source/T66/UI/T66GamblerOverlayWidget.cpp | 3731 | Constant | PlinkoBallSize | 14.f | static constexpr float PlinkoBallSize = 14.f; |
| Source/T66/UI/T66GamblerOverlayWidget.h | 69 | Constant | InventorySlotCount | 5 | static constexpr int32 InventorySlotCount = 5; |
| Source/T66/UI/T66GamblerOverlayWidget.h | 176 | Constant | PlinkoSlotCount | 9 | static constexpr int32 PlinkoSlotCount = 9; |
| Source/T66/UI/T66GamblerOverlayWidget.h | 190 | Constant | BoxColorCount | 4 | static constexpr int32 BoxColorCount = 4; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 259 | Constant | GT66MediaPanelW | 330.f | static constexpr float GT66MediaPanelW = 330.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 262 | Constant | GT66MediaTabRowH | 30.f | static constexpr float GT66MediaTabRowH = 30.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 263 | Constant | GT66MediaPanelH | GT66MediaVideoH + GT66MediaTabRowH | static constexpr float GT66MediaPanelH = GT66MediaVideoH + GT66MediaTabRowH; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 264 | Constant | GT66BottomLeftHudScale | 0.96f | static constexpr float GT66BottomLeftHudScale = 0.96f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 265 | Constant | GT66BottomLeftPortraitPanelSize | 280.f | static constexpr float GT66BottomLeftPortraitPanelSize = 280.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 266 | Constant | GT66BottomLeftSidePanelHeight | 224.f | static constexpr float GT66BottomLeftSidePanelHeight = 224.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 267 | Constant | GT66BottomLeftSidePanelWidth | 210.f | static constexpr float GT66BottomLeftSidePanelWidth = 210.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 268 | Constant | GT66BottomLeftIdolSlotSize | 86.f | static constexpr float GT66BottomLeftIdolSlotSize = 86.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 269 | Constant | GT66BottomLeftIdolSlotPad | 4.f | static constexpr float GT66BottomLeftIdolSlotPad = 4.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 270 | Constant | GT66BottomLeftAbilityGap | 10.f | static constexpr float GT66BottomLeftAbilityGap = 10.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 271 | Constant | GT66BottomLeftAbilityColumnHeight | GT66BottomLeftPortraitPanelSize | static constexpr float GT66BottomLeftAbilityColumnHeight = GT66BottomLeftPortraitPanelSize; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 272 | Constant | GT66BottomLeftAbilityBoxSize | (GT66BottomLeftAbilityColumnHeight - GT66BottomLeftAbilityGap) * 0.5f | static constexpr float GT66BottomLeftAbilityBoxSize = (GT66BottomLeftAbilityColumnHeight - GT66BottomLeftAbilityGap) * 0.5f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 275 | Constant | GT66HeartsTopOffsetFromBottom | 24.f + GT66BottomLeftPortraitPanelSize + 6.f + 48.f | static constexpr float GT66HeartsTopOffsetFromBottom = 24.f + GT66BottomLeftPortraitPanelSize + 6.f + 48.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 393 | SBorder | Padding | FMargin(8.f, 6.f | .Padding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 400 | STextBlock | WrapTextAt | 280.f | .WrapTextAt(280.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 414 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 417 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 421 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 431 | STextBlock | WrapTextAt | 280.f | .WrapTextAt(280.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 458 | STextBlock | ComputeDesiredSize | 52.f, 52.f | return FVector2D(52.f, 52.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 465 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 467 | Constant | Radius | (MinDim * 0.5f) - Thickness | const float Radius = (MinDim * 0.5f) - Thickness; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 487 | Constant | FillRadius | Radius * 0.5f | const float FillRadius = Radius * 0.5f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 573 | STextBlock | ComputeDesiredSize | 12.f, 12.f | return FVector2D(12.f, 12.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 580 | Constant | MinDim | FMath::Max(1.f, FMath::Min(Size.X, Size.Y)) | const float MinDim = FMath::Max(1.f, FMath::Min(Size.X, Size.Y)); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 582 | Constant | Radius | (MinDim * 0.5f) - 0.5f | const float Radius = (MinDim * 0.5f) - 0.5f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 627 | STextBlock | ComputeDesiredSize | 28.f, 28.f | return FVector2D(28.f, 28.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 640 | Constant | Gap | 4.f | const float Gap = 4.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 666 | Constant | RingRadius | 10.f * Pulse | const float RingRadius = 10.f * Pulse; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 748 | STextBlock | ComputeDesiredSize | 1920.f, 1080.f | return FVector2D(1920.f, 1080.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 757 | Constant | ScopeRadius | ScopeDiameter * 0.5f | const float ScopeRadius = ScopeDiameter * 0.5f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 1048 | STextBlock | FontSize | 14 | FT66Style::Tokens::FontBold(14), |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 1072 | Constant | MarkerSize | bMinimap ? 18.f : 24.f | const float MarkerSize = bMinimap ? 18.f : 24.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 1075 | Constant | InnerSize | FVector2D(MarkerSize - 4.f, MarkerSize - 4.f) | const FVector2D InnerSize = FVector2D(MarkerSize - 4.f, MarkerSize - 4.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 1137 | Constant | IconSize | bMinimap ? M.DrawSize : (M.DrawSize + FVector2D(4.f, 4.f)) | const FVector2D IconSize = bMinimap ? M.DrawSize : (M.DrawSize + FVector2D(4.f, 4.f)); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 1185 | STextBlock | FontSize | 10 | FT66Style::Tokens::FontBold(bMinimap ? 10 : 12), |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2311 | Constant | BossMax | FMath::Max(1, RunState->GetBossMaxHP()) | const int32 BossMax = FMath::Max(1, RunState->GetBossMaxHP()); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2313 | Constant | BarWidth | 600.f | const float BarWidth = 600.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2364 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2812 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2941 | Constant | BossBarWidth | 600.f | static constexpr float BossBarWidth = 600.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2948 | STextBlock | ImageSize | 28.f, 28.f | LootPromptIconBrush->ImageSize = FVector2D(28.f, 28.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2954 | STextBlock | ImageSize | 1.f, 1.f | PortraitBrush->ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2960 | STextBlock | ImageSize | GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize | UltimateBrush->ImageSize = FVector2D(GT66BottomLeftAbilityBoxSize, GT66BottomLeftAbilityBoxSize); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2976 | STextBlock | ImageSize | 120.f, 120.f | WheelTextureBrush.ImageSize = FVector2D(120.f, 120.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 2990 | STextBlock | ImageSize | 38.f, 38.f | HeartBrush->ImageSize = FVector2D(38.f, 38.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3006 | STextBlock | ImageSize | 34.f, 34.f | QuickReviveBrush->ImageSize = FVector2D(34.f, 34.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3023 | STextBlock | ImageSize | 38.f, 38.f | SkullBrush->ImageSize = FVector2D(38.f, 38.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3040 | STextBlock | ImageSize | 38.f, 38.f | ClownBrush->ImageSize = FVector2D(38.f, 38.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3056 | STextBlock | ImageSize | 1.f, 1.f | IdolSlotBrushes[i]->ImageSize = FVector2D(1.f, 1.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3063 | STextBlock | ImageSize | 36.f, 36.f | InventorySlotBrushes[i]->ImageSize = FVector2D(36.f, 36.f); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3067 | Constant | MinimapWidth | 228.f | static constexpr float MinimapWidth = 228.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3068 | Constant | DiffGap | 2.f | static constexpr float DiffGap = 2.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3069 | Constant | DiffSize | 44.f | static constexpr float DiffSize = 44.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3076 | SHorizontalBox | Padding | i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f | .Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3098 | SHorizontalBox | Padding | i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f | .Padding(i == 0 ? 0.f : DiffGap, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3114 | Constant | HeartSize | 48.f | static constexpr float HeartSize = 48.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3115 | Constant | HeartPad | 1.f | static constexpr float HeartPad = 1.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3116 | Constant | HeartsRowWidth | (HeartSize + (HeartPad * 2.f)) * static_cast<float>(UT66RunStateSubsystem::DefaultMaxHearts) | const float HeartsRowWidth = (HeartSize + (HeartPad * 2.f)) * static_cast<float>(UT66RunStateSubsystem::DefaultMaxHearts); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3123 | SHorizontalBox | Padding | HeartPad, 0.f | .Padding(HeartPad, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3146 | SBox | WidthOverride | 34.f | .WidthOverride(34.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3147 | SBox | HeightOverride | 34.f | .HeightOverride(34.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3164 | SHorizontalBox | Padding | 2.f, 0.f | .Padding(2.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3167 | SBox | WidthOverride | 10.f | .WidthOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3168 | SBox | HeightOverride | 10.f | .HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3180 | Constant | IdolColumns | 2 | static constexpr int32 IdolColumns = 2; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3181 | Constant | IdolSlotPad | GT66BottomLeftIdolSlotPad | static constexpr float IdolSlotPad = GT66BottomLeftIdolSlotPad; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3182 | Constant | IdolSlotSize | GT66BottomLeftIdolSlotSize | static constexpr float IdolSlotSize = GT66BottomLeftIdolSlotSize; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3183 | Constant | IdolRows | FMath::DivideAndRoundUp(UT66IdolManagerSubsystem::MaxEquippedIdolSlots, IdolColumns) | const int32 IdolRows = FMath::DivideAndRoundUp(UT66IdolManagerSubsystem::MaxEquippedIdolSlots, IdolColumns); |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3184 | Constant | IdolPanelMinWidth | GT66BottomLeftSidePanelWidth | const float IdolPanelMinWidth = GT66BottomLeftSidePanelWidth; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3185 | Constant | HUDSidePanelMinHeight | GT66BottomLeftSidePanelHeight | const float HUDSidePanelMinHeight = GT66BottomLeftSidePanelHeight; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3186 | Constant | StatsPanelMinWidth | GT66BottomLeftSidePanelWidth | const float StatsPanelMinWidth = GT66BottomLeftSidePanelWidth; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3187 | Constant | PortraitPanelSize | GT66BottomLeftPortraitPanelSize | const float PortraitPanelSize = GT66BottomLeftPortraitPanelSize; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3188 | Constant | AbilityColumnHeight | GT66BottomLeftAbilityColumnHeight | const float AbilityColumnHeight = GT66BottomLeftAbilityColumnHeight; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3189 | Constant | AbilityColumnWidth | GT66BottomLeftAbilityBoxSize | const float AbilityColumnWidth = GT66BottomLeftAbilityBoxSize; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3190 | Constant | AbilityIconSize | GT66BottomLeftAbilityBoxSize | const float AbilityIconSize = GT66BottomLeftAbilityBoxSize; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3203 | SBorder | Padding | 3.f | .Padding(3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3211 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 2.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3215 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3219 | STextBlock | Padding | 4.f, 0.f, 4.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 4.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3222 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3255 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3263 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3272 | SBorder | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3275 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3284 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3287 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3309 | Constant | InvCols | 10 | static constexpr int32 InvCols = 10; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3310 | Constant | InvRows | 2 | static constexpr int32 InvRows = 2; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3311 | Constant | InvSlotSize | 36.f | static constexpr float InvSlotSize = 36.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3312 | Constant | InvSlotPad | 2.f | static constexpr float InvSlotPad = 2.f; |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3346 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3354 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3363 | SBorder | Padding | 1.f, 1.f, 1.f, 0.f | .Padding(1.f, 1.f, 1.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3366 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3375 | SBorder | Padding | 1.f, 0.f, 1.f, 1.f | .Padding(1.f, 0.f, 1.f, 1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3378 | SBox | HeightOverride | 2.f | .HeightOverride(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3389 | SBorder | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3417 | SOverlay | Padding | 0.f, 12.f | .Padding(0.f, 12.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3421 | SBox | HeightOverride | 28.f | .HeightOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3452 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3461 | STextBlock | Padding | 0.f, 48.f, 0.f, 0.f | .Padding(0.f, 48.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3464 | SBox | WidthOverride | 760.f | .WidthOverride(760.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3465 | SBox | HeightOverride | 40.f | .HeightOverride(40.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3471 | SBorder | Padding | 10.f, 6.f | .Padding(10.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3474 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3477 | SBox | WidthOverride | 28.f | .WidthOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3478 | SBox | HeightOverride | 28.f | .HeightOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3490 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3504 | SBox | RenderTransform | 0.f, 0.f | .RenderTransform(FSlateRenderTransform(FVector2D(0.f, 0.f))) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3513 | SBorder | Padding | 10.f, 6.f | .Padding(10.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3517 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3521 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3526 | SBorder | Padding | 10.f, 6.f | .Padding(10.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3530 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3534 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3539 | SBorder | Padding | 10.f, 6.f | .Padding(10.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3543 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3548 | STextBlock | SetPadding | 12.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3556 | STextBlock | Padding | 16.f, 0.f, 0.f, GT66HeartsTopOffsetFromBottom | .Padding(16.f, 0.f, 0.f, GT66HeartsTopOffsetFromBottom) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3559 | SBox | WidthOverride | GT66MediaPanelW | .WidthOverride(GT66MediaPanelW) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3560 | SBox | HeightOverride | GT66MediaPanelH | .HeightOverride(GT66MediaPanelH) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3567 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 2.f | .Padding(0.f, 0.f, 0.f, 2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3572 | SHorizontalBox | Padding | 2.f, 0.f | .Padding(2.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3589 | SHorizontalBox | SetFontSize | 13 | ).SetFontSize(13).SetMinWidth(80.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3589 | SHorizontalBox | SetMinWidth | 80.f | ).SetFontSize(13).SetMinWidth(80.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3594 | SHorizontalBox | Padding | 2.f, 0.f | .Padding(2.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3611 | SHorizontalBox | SetFontSize | 13 | ).SetFontSize(13).SetMinWidth(80.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3611 | SHorizontalBox | SetMinWidth | 80.f | ).SetFontSize(13).SetMinWidth(80.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3622 | SBorder | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3636 | SBorder | SetPadding | 6.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3644 | SBorder | Padding | 24.f, 24.f | .Padding(24.f, 24.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3656 | STextBlock | FontSize | 17 | .Font(FT66Style::Tokens::FontBold(17)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3659 | STextBlock | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3663 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3666 | STextBlock | Padding | 10.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3670 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3674 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3678 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3681 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3685 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3689 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3693 | STextBlock | FontSize | 15 | .Font(FT66Style::Tokens::FontBold(15)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3708 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3715 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3718 | STextBlock | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3722 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3726 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3730 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3733 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3737 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3741 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3745 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3749 | STextBlock | SetPadding | 12.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3757 | STextBlock | Padding | 24.f, 0.f, 24.f, 0.f | .Padding(24.f, 0.f, 24.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3760 | SBox | WidthOverride | 160.f | .WidthOverride(160.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3761 | SBox | HeightOverride | 160.f | .HeightOverride(160.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3771 | SBox | WidthOverride | 120.f | .WidthOverride(120.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3772 | SBox | HeightOverride | 120.f | .HeightOverride(120.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3783 | SBox | WidthOverride | 140.f | .WidthOverride(140.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3787 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3794 | STextBlock | SetPadding | 10.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(10.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3802 | STextBlock | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3808 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 2.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3812 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3817 | STextBlock | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3821 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3826 | STextBlock | Padding | 0.f, 0.f, 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3832 | SBox | WidthOverride | IdolPanelMinWidth + 16.f | .WidthOverride(IdolPanelMinWidth + 16.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3840 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3844 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3869 | SBox | MinDesiredHeight | HUDSidePanelMinHeight - 34.f | .MinDesiredHeight(HUDSidePanelMinHeight - 34.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3879 | SBox | Padding | 16.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(16.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3893 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3898 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3919 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3930 | STextBlock | Padding | 0.f, 2.f, 2.f, 0.f | .Padding(0.f, 2.f, 2.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3933 | SBox | WidthOverride | 52.f | .WidthOverride(52.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3934 | SBox | HeightOverride | 52.f | .HeightOverride(52.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3947 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3974 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3985 | STextBlock | Padding | 0.f, 2.f, 2.f, 0.f | .Padding(0.f, 2.f, 2.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3988 | SBox | WidthOverride | 52.f | .WidthOverride(52.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 3989 | SBox | HeightOverride | 52.f | .HeightOverride(52.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4002 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4011 | STextBlock | Padding | 12.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(12.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4020 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 3.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4024 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4029 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4033 | STextBlock | Padding | 0.f, 0.f, 0.f, 3.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4037 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4042 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4046 | STextBlock | Padding | 0.f, 0.f, 0.f, 3.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4050 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4055 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4059 | STextBlock | Padding | 0.f, 0.f, 0.f, 3.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4063 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4068 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4072 | STextBlock | Padding | 0.f, 0.f, 0.f, 3.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4076 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4081 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4089 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4094 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4102 | STextBlock | Padding | 16.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(16.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4110 | SVerticalBox | Padding | 0.f, 0.f, 0.f, GT66BottomLeftAbilityGap | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, GT66BottomLeftAbilityGap) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4123 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4134 | SImage | Padding | 0.f, 0.f, 4.f, 4.f | .Padding(0.f, 0.f, 4.f, 4.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4137 | SBox | WidthOverride | 28.f | .WidthOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4138 | SBox | HeightOverride | 28.f | .HeightOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4146 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4152 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4171 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4193 | STextBlock | FontSize | 22 | .Font(FT66Style::Tokens::FontBold(22)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4204 | SBox | WidthOverride | 22.f | .WidthOverride(22.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4205 | SBox | HeightOverride | 22.f | .HeightOverride(22.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4215 | STextBlock | FontSize | 11 | .Font(FT66Style::Tokens::FontBold(11)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4232 | STextBlock | Padding | 24.f, 24.f | .Padding(24.f, 24.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4257 | ST66WorldMapWidget | SetPadding | 8.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(8.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4263 | ST66WorldMapWidget | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4267 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4271 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4283 | SVerticalBox | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4288 | SVerticalBox | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4294 | SButton | ContentPadding | FMargin(8.f, 4.f | .ContentPadding(FMargin(8.f, 4.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4298 | STextBlock | FontSize | 10 | .Font(FT66Style::Tokens::FontBold(10)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4303 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4309 | SButton | ContentPadding | FMargin(8.f, 4.f | .ContentPadding(FMargin(8.f, 4.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4313 | STextBlock | FontSize | 10 | .Font(FT66Style::Tokens::FontBold(10)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4325 | STextBlock | Padding | 24.f, 0.f, 24.f, 24.f | .Padding(24.f, 0.f, 24.f, 24.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4335 | SVerticalBox | Padding | 4.f, 0.f, 4.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(4.f, 0.f, 4.f, 8.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4342 | STextBlock | FontSize | 17 | .Font(FT66Style::Tokens::FontBold(17)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4345 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4349 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4352 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4356 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4373 | SBorder | Padding | 3.f | .Padding(3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4378 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4381 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4385 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4389 | STextBlock | Padding | 4.f, 0.f, 4.f, 6.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(4.f, 0.f, 4.f, 6.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4392 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4399 | SBorder | Padding | 4.f, 0.f, 4.f, 8.f | + SVerticalBox::Slot().AutoHeight().Padding(4.f, 0.f, 4.f, 8.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4406 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4409 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4413 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4416 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4420 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4433 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4433 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4442 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4442 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4451 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4451 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4460 | SBox | HeightOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4460 | SBox | WidthOverride | 10.f | SNew(SBox).WidthOverride(10.f).HeightOverride(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4474 | SBorder | Padding | 24.f, 0.f, 24.f, 280.f | .Padding(24.f, 0.f, 24.f, 280.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4478 | SBox | WidthOverride | 280.f | .WidthOverride(280.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4483 | SBorder | Padding | 3.f | .Padding(3.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4488 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4495 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4498 | STextBlock | WrapTextAt | 256.f | .WrapTextAt(256.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4500 | STextBlock | Padding | 0.f, 2.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4504 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4515 | STextBlock | Padding | 24.f, 0.f, 24.f, PickupCardBottomOffset | .Padding(24.f, 0.f, 24.f, PickupCardBottomOffset) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4528 | STextBlock | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4531 | STextBlock | WrapTextAt | PickupCardWidth - 24.f | .WrapTextAt(PickupCardWidth - 24.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4533 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4548 | SImage | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f), |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4552 | SImage | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4556 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontRegular(12)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4560 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4568 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4572 | STextBlock | SetPadding | FMargin(8.f, 6.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(8.f, 6.f))) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4574 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4), |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4582 | STextBlock | Padding | 0.f, -220.f, 0.f, 0.f | .Padding(0.f, -220.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4590 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4594 | STextBlock | Padding | 0.f, 4.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4599 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4604 | STextBlock | SetPadding | FMargin(12.f, 8.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 8.f)).SetVisibility(EVisibility::Collapsed), |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4612 | STextBlock | Padding | 0.f, -140.f, 0.f, 0.f | .Padding(0.f, -140.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4615 | SBox | WidthOverride | 28.f | .WidthOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4616 | SBox | HeightOverride | 28.f | .HeightOverride(28.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4637 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4649 | ST66ScopedSniperWidget | Padding | 0.f, 24.f, 0.f, 0.f | .Padding(0.f, 24.f, 0.f, 0.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4654 | STextBlock | FontSize | 20 | .Font(FT66Style::Tokens::FontBold(20)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4656 | STextBlock | SetPadding | FMargin(12.f, 8.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(12.f, 8.f))) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4661 | STextBlock | Padding | 0.f, 0.f, 0.f, 42.f | .Padding(0.f, 0.f, 0.f, 42.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4666 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4668 | STextBlock | SetPadding | FMargin(14.f, 8.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f, 8.f))) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4686 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4718 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4725 | STextBlock | FontSize | 18 | .Font(FT66Style::Tokens::FontBold(18)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4732 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4739 | SBox | WidthOverride | 1100.f | .WidthOverride(1100.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4740 | SBox | HeightOverride | 680.f | .HeightOverride(680.f) |
| Source/T66/UI/T66GameplayHUDWidget.cpp | 4746 | ST66WorldMapWidget | SetPadding | 10.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(10.f) |
| Source/T66/UI/T66GameplayHUDWidget.h | 168 | Constant | PickupCardWidth | 260.f | static constexpr float PickupCardWidth = 260.f; |
| Source/T66/UI/T66GameplayHUDWidget.h | 169 | Constant | PickupCardHeight | 460.f | static constexpr float PickupCardHeight = 460.f; |
| Source/T66/UI/T66GameplayHUDWidget.h | 170 | Constant | PickupCardBottomOffset | 310.f | static constexpr float PickupCardBottomOffset = 310.f; |
| Source/T66/UI/T66HeroCooldownBarWidget.cpp | 41 | SVerticalBox | Padding | 0.f, 0.f | .Padding(0.f, 0.f) |
| Source/T66/UI/T66HeroCooldownBarWidget.cpp | 48 | SBorder | Padding | FMargin(1.f | .Padding(FMargin(1.f)) |
| Source/T66/UI/T66HeroCooldownBarWidget.cpp | 73 | SBorder | Padding | 0.f, 2.f | .Padding(0.f, 2.f) |
| Source/T66/UI/T66HeroCooldownBarWidget.cpp | 78 | STextBlock | FontSize | 12 | .Font(FT66Style::Tokens::FontBold(12)) |
| Source/T66/UI/T66HeroCooldownBarWidget.h | 31 | Constant | BarWidth | 100.f | static constexpr float BarWidth = 100.f; |
| Source/T66/UI/T66HeroCooldownBarWidget.h | 32 | Constant | BarHeight | 8.f | static constexpr float BarHeight = 8.f; |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 99 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 102 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 106 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 116 | STextBlock | WrapTextAt | 280.f | .WrapTextAt(280.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 137 | SBorder | Padding | FMargin(16.f, 10.f | .Padding(FMargin(16.f, 10.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 315 | STextBlock | ImageSize | 220.f, 220.f | OfferIconBrushes[SlotIndex]->ImageSize = FVector2D(220.f, 220.f); |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 322 | STextBlock | ImageSize | 220.f, 220.f | TradeIconBrushes[SlotIndex]->ImageSize = FVector2D(220.f, 220.f); |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 339 | SHorizontalBox | Padding | SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f | .Padding(SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 342 | SBox | MinDesiredHeight | 470.f | .MinDesiredHeight(470.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 347 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 352 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 357 | SBorder | Padding | FMargin(16.f | .Padding(FMargin(16.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 369 | STextBlock | Padding | 0.f, 20.f, 0.f, 20.f | + SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 20.f, 0.f, 20.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 374 | SBorder | Padding | 3.f | .Padding(3.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 379 | SBorder | Padding | 8.f | .Padding(8.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 382 | SBox | WidthOverride | 220.f | .WidthOverride(220.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 383 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 415 | STextBlock | Padding | 0.f, 12.f, 0.f, 0.f | OffersContent->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 420 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | OffersContent->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 444 | SHorizontalBox | Padding | SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f | .Padding(SlotIndex > 0 ? FMargin(14.f, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 447 | SBox | MinDesiredHeight | 470.f | .MinDesiredHeight(470.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 452 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 457 | SBorder | Padding | 1.f | .Padding(1.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 462 | SBorder | Padding | FMargin(16.f | .Padding(FMargin(16.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 474 | STextBlock | Padding | 0.f, 20.f, 0.f, 20.f | + SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f, 20.f, 0.f, 20.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 479 | SBorder | Padding | 3.f | .Padding(3.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 484 | SBorder | Padding | 8.f | .Padding(8.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 487 | SBox | WidthOverride | 220.f | .WidthOverride(220.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 488 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 529 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 550 | SBorder | Padding | FMargin(24.f | .Padding(FMargin(24.f)) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 587 | SHorizontalBox | Padding | 8.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 599 | SHorizontalBox | Padding | 0.f, 12.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 606 | STextBlock | Padding | 0.f, 18.f, 0.f, 0.f | + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 18.f, 0.f, 0.f) |
| Source/T66/UI/T66IdolAltarOverlayWidget.cpp | 620 | SWidgetSwitcher | SetPadding | FMargin(28.f | .SetPadding(FMargin(28.f))) |
| Source/T66/UI/T66IdolAltarOverlayWidget.h | 31 | Constant | OfferSlotCount | 16 | static constexpr int32 OfferSlotCount = 16; |
| Source/T66/UI/T66IdolAltarOverlayWidget.h | 32 | Constant | OfferSlotsPerCategory | 4 | static constexpr int32 OfferSlotsPerCategory = 4; |
| Source/T66/UI/T66IdolAltarOverlayWidget.h | 34 | Constant | TradeSlotCount | 4 | static constexpr int32 TradeSlotCount = 4; |
| Source/T66/UI/T66LabOverlayWidget.cpp | 25 | Constant | LabPanelWidth | 480.f | static constexpr float LabPanelWidth = 480.f; |
| Source/T66/UI/T66LabOverlayWidget.cpp | 26 | Constant | LabPanelTopOffset | 340.f | static constexpr float LabPanelTopOffset = 340.f; |
| Source/T66/UI/T66LabOverlayWidget.cpp | 27 | Constant | LabPanelMaxHeight | 220.f | static constexpr float LabPanelMaxHeight = 220.f; |
| Source/T66/UI/T66LabOverlayWidget.cpp | 190 | SScrollBox | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 211 | SScrollBox | Padding | 2.f | .Padding(2.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 216 | STextBlock | FontSize | 11 | SNew(STextBlock).Text(EntryLabel).Font(FT66Style::Tokens::FontRegular(11)).ColorAndOpacity(FT66Style::Tokens::Text) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 243 | SHorizontalBox | Padding | 2.f | TabRow->AddSlot().AutoWidth().Padding(2.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 246 | SHorizontalBox | SetFontSize | 9 | .SetMinWidth(0.f).SetFontSize(9).SetPadding(FMargin(6.f, 2.f))) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 246 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f).SetFontSize(9).SetPadding(FMargin(6.f, 2.f))) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 246 | SHorizontalBox | SetPadding | FMargin(6.f, 2.f | .SetMinWidth(0.f).SetFontSize(9).SetPadding(FMargin(6.f, 2.f))) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 258 | SHorizontalBox | SetFontSize | 10 | .SetMinWidth(0.f).SetFontSize(10).SetPadding(FMargin(8.f, 4.f))); |
| Source/T66/UI/T66LabOverlayWidget.cpp | 258 | SHorizontalBox | SetMinWidth | 0.f | .SetMinWidth(0.f).SetFontSize(10).SetPadding(FMargin(8.f, 4.f))); |
| Source/T66/UI/T66LabOverlayWidget.cpp | 258 | SHorizontalBox | SetPadding | FMargin(8.f, 4.f | .SetMinWidth(0.f).SetFontSize(10).SetPadding(FMargin(8.f, 4.f))); |
| Source/T66/UI/T66LabOverlayWidget.cpp | 269 | SVerticalBox | Padding | 0.f, 6.f | LabPanel->AddSlot().AutoHeight().Padding(0.f, 6.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 279 | SBox | Padding | 0.f, 4.f | LabPanel->AddSlot().AutoHeight().Padding(0.f, 4.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 291 | SBox | Padding | 0.f, 4.f | LabPanel->AddSlot().AutoHeight().Padding(0.f, 4.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 306 | SOverlay | Padding | 24.f, LabPanelTopOffset, 24.f, 0.f | .Padding(24.f, LabPanelTopOffset, 24.f, 0.f) |
| Source/T66/UI/T66LabOverlayWidget.cpp | 314 | SBorder | Padding | 8.f | .Padding(8.f) |
| Source/T66/UI/T66LoadPreviewOverlayWidget.cpp | 22 | SOverlay | Padding | 20.f, 0.f, 0.f, 20.f | .Padding(20.f, 0.f, 0.f, 20.f) |
| Source/T66/UI/T66LoadPreviewOverlayWidget.cpp | 26 | SOverlay | SetMinWidth | 160.f | .SetMinWidth(160.f)) |
| Source/T66/UI/T66ScreenBase.cpp | 40 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Bg).SetPadding(FT66Style::Tokens::Space4)); |
| Source/T66/UI/T66ScreenBase.cpp | 104 | STextBlock | FontSize | 0 | .Font(FT66Style::Tokens::FontRegular(FontSize > 0 ? FontSize : 14)) |
| Source/T66/UI/T66ScreenBase.cpp | 112 | STextBlock | FontSize | 0 | .Font(FT66Style::Tokens::FontBold(FontSize > 0 ? FontSize : 24)) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 25 | SBorder | Padding | FMargin(10.f, 8.f | .Padding(FMargin(10.f, 8.f)) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 28 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 4.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 32 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 42 | STextBlock | WrapTextAt | 280.f | .WrapTextAt(280.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 392 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 396 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 412 | STextBlock | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 416 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 445 | STextBlock | Padding | 0.f, 8.f, 0.f, 8.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 448 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 459 | SBorder | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 496 | SBox | HeightOverride | FT66Style::Tokens::NPCStatsPanelContentHeight | .HeightOverride(FT66Style::Tokens::NPCStatsPanelContentHeight) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 508 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 524 | SBox | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 545 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 549 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 573 | STextBlock | Padding | 0.f, 8.f, 0.f, 8.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 576 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 587 | SBorder | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 608 | STextBlock | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 612 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 639 | STextBlock | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 643 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 670 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 699 | SBox | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 717 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 721 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 737 | STextBlock | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 741 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 770 | STextBlock | Padding | 0.f, 8.f, 0.f, 8.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 773 | SBox | HeightOverride | 1.f | .HeightOverride(1.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 784 | SBorder | Padding | 0.f, 0.f, 0.f, 6.f | StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 822 | SBox | HeightOverride | FT66Style::Tokens::NPCStatsPanelContentHeight | .HeightOverride(FT66Style::Tokens::NPCStatsPanelContentHeight) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 834 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66StatsPanelSlate.cpp | 850 | SBox | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66UIManager.h | 153 | Constant | MaxHistoryDepth | 10 | static constexpr int32 MaxHistoryDepth = 10; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 154 | Constant | AngerFaceSize | FT66Style::Tokens::NPCAngerCircleSize | const float AngerFaceSize = FT66Style::Tokens::NPCAngerCircleSize; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 185 | Constant | ShopSlotCount | UT66RunStateSubsystem::VendorDisplaySlotCount | static constexpr int32 ShopSlotCount = UT66RunStateSubsystem::VendorDisplaySlotCount; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 204 | Constant | ShopCardSize | FT66Style::Tokens::NPCShopCardWidth | const float ShopCardSize = FT66Style::Tokens::NPCShopCardWidth; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 205 | Constant | ShopCardHeight | FT66Style::Tokens::NPCShopCardHeight | const float ShopCardHeight = FT66Style::Tokens::NPCShopCardHeight; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 206 | Constant | ShopIconSize | ShopCardSize - FT66Style::Tokens::Space4 * 2.f | const float ShopIconSize = ShopCardSize - FT66Style::Tokens::Space4 * 2.f; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 218 | Unknown | ImageSize | 160.f, 160.f | InventorySlotIconBrushes[i]->ImageSize = FVector2D(160.f, 160.f); |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 230 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 231 | SHorizontalBox | SetPadding | FMargin(8.f, 6.f | .SetPadding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 235 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 246 | STextBlock | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 247 | STextBlock | SetPadding | FMargin(8.f, 6.f | .SetPadding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 248 | STextBlock | SetFontSize | 14 | .SetFontSize(14) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 254 | STextBlock | Padding | i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f | .Padding(i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 266 | SBox | HeightOverride | 60.f | .HeightOverride(60.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 273 | STextBlock | WrapTextAt | ShopCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(ShopCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 277 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 291 | SImage | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f), |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 296 | SImage | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 303 | STextBlock | WrapTextAt | ShopCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(ShopCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 306 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 311 | SHorizontalBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space2, 0.f | .Padding(0.f, 0.f, FT66Style::Tokens::Space2, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 318 | SHorizontalBox | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4), |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 325 | Constant | BuybackSlotCount | UT66RunStateSubsystem::BuybackDisplaySlotCount | static constexpr int32 BuybackSlotCount = UT66RunStateSubsystem::BuybackDisplaySlotCount; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 349 | SHorizontalBox | SetMinWidth | 100.f | .SetMinWidth(100.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 350 | SHorizontalBox | SetPadding | FMargin(8.f, 6.f | .SetPadding(FMargin(8.f, 6.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 354 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 361 | STextBlock | Padding | i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f | .Padding(i > 0 ? FMargin(FT66Style::Tokens::Space4, 0.f, 0.f, 0.f) : FMargin(0.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 372 | SBox | HeightOverride | 60.f | .HeightOverride(60.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 379 | STextBlock | WrapTextAt | ShopCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(ShopCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 382 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 396 | SImage | SetPadding | 0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f), |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 400 | SImage | Padding | 0.f, FT66Style::Tokens::Space2, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space2, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 407 | STextBlock | WrapTextAt | ShopCardSize - FT66Style::Tokens::Space4 * 2.f | .WrapTextAt(ShopCardSize - FT66Style::Tokens::Space4 * 2.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 409 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 414 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4), |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 423 | SBox | WidthOverride | 560.f | .WidthOverride(560.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 424 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 429 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 436 | STextBlock | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 439 | SBox | WidthOverride | 360.f | .WidthOverride(360.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 440 | SBox | HeightOverride | 28.f | .HeightOverride(28.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 455 | SBox | WidthOverride | 0.f | .WidthOverride(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 460 | SBox | WidthOverride | 10.f | .WidthOverride(10.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 470 | SBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 484 | STextBlock | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 485 | STextBlock | SetPadding | FMargin(22.f, 12.f | .SetPadding(FMargin(22.f, 12.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 489 | STextBlock | SetPadding | 18.f | FT66PanelParams(ET66PanelType::Panel).SetPadding(18.f).SetColor(FT66Style::Tokens::Panel2)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 506 | SVerticalBox | Padding | 0.f, 20.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 20.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 508 | SVerticalBox | Padding | 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 515 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 521 | STextBlock | SetMinWidth | 420.f | .SetMinWidth(420.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 522 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 526 | STextBlock | Padding | 0.f, 8.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 532 | STextBlock | SetMinWidth | 420.f | .SetMinWidth(420.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 533 | STextBlock | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 540 | SUniformGridPanel | SlotPadding | FMargin(FT66Style::Tokens::Space2, 0.f | .SlotPadding(FMargin(FT66Style::Tokens::Space2, 0.f)); |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 549 | SUniformGridPanel | SetHeight | FT66Style::Tokens::InventorySlotSize | .SetMinWidth(FT66Style::Tokens::InventorySlotSize).SetHeight(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 549 | SUniformGridPanel | SetMinWidth | FT66Style::Tokens::InventorySlotSize | .SetMinWidth(FT66Style::Tokens::InventorySlotSize).SetHeight(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 550 | SUniformGridPanel | SetPadding | FMargin(0.f | .SetPadding(FMargin(0.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 560 | SImage | Padding | 0.f, 6.f, 8.f, 0.f | + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 6.f, 8.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 564 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontBold(14)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 566 | STextBlock | ShadowOffset | 1.f, 1.f | .ShadowOffset(FVector2D(1.f, 1.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 578 | STextBlock | SetPadding | FMargin(0.f | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(0.f)), |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 586 | SBox | WidthOverride | FT66Style::Tokens::InventorySlotSize | .WidthOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 587 | SBox | HeightOverride | FT66Style::Tokens::InventorySlotSize | .HeightOverride(FT66Style::Tokens::InventorySlotSize) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 600 | SBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 601 | SBox | SetPadding | FMargin(18.f, 10.f | .SetPadding(FMargin(18.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 649 | SScrollBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 650 | SScrollBox | SetPadding | FMargin(12.f, 8.f | .SetPadding(FMargin(12.f, 8.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 663 | SScrollBox | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 664 | SScrollBox | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 682 | SHorizontalBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space6, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 685 | SBox | WidthOverride | FT66Style::Tokens::NPCVendorStatsPanelWidth | .WidthOverride(FT66Style::Tokens::NPCVendorStatsPanelWidth) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 686 | SBox | HeightOverride | FT66Style::Tokens::NPCMainRowHeight | .HeightOverride(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 691 | SBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space6, 0.f | + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space6, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 694 | SBox | MinDesiredHeight | FT66Style::Tokens::NPCMainRowHeight | .MinDesiredHeight(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 700 | SHorizontalBox | Padding | 0.f, 0.f, FT66Style::Tokens::Space4, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 709 | SHorizontalBox | Padding | 0.f, FT66Style::Tokens::Space4, 0.f, 0.f | + SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, FT66Style::Tokens::Space4, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 721 | SWidgetSwitcher | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 725 | SWidgetSwitcher | Padding | 0.f, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 728 | SBox | WidthOverride | FT66Style::Tokens::NPCRightPanelWidth | .WidthOverride(FT66Style::Tokens::NPCRightPanelWidth) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 729 | SBox | MinDesiredHeight | FT66Style::Tokens::NPCMainRowHeight | .MinDesiredHeight(FT66Style::Tokens::NPCMainRowHeight) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 733 | SVerticalBox | Padding | 0.f, 6.f, 0.f, 14.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 14.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 736 | SBox | WidthOverride | 260.f | .WidthOverride(260.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 737 | SBox | HeightOverride | 260.f | .HeightOverride(260.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 747 | SSpacer | Padding | 0.f, 0.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 751 | SVerticalBox | Padding | 0.f, 0.f, 0.f, FT66Style::Tokens::Space4 | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 758 | STextBlock | Padding | 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 761 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 764 | SBox | WidthOverride | FT66Style::Tokens::NPCBankSpinBoxWidth | .WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 765 | SBox | HeightOverride | FT66Style::Tokens::NPCBankSpinBoxHeight | .HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 769 | SSpinBox<int32> | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 789 | SSpinBox<int32> | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 790 | SSpinBox<int32> | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 794 | SSpinBox<int32> | Padding | 0.f, 6.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 797 | SHorizontalBox | Padding | 0.f, 0.f, 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 800 | SBox | WidthOverride | FT66Style::Tokens::NPCBankSpinBoxWidth | .WidthOverride(FT66Style::Tokens::NPCBankSpinBoxWidth) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 801 | SBox | HeightOverride | FT66Style::Tokens::NPCBankSpinBoxHeight | .HeightOverride(FT66Style::Tokens::NPCBankSpinBoxHeight) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 805 | SSpinBox<int32> | FontSize | 16 | .Font(FT66Style::Tokens::FontBold(16)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 817 | SSpinBox<int32> | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 818 | SSpinBox<int32> | SetPadding | FMargin(16.f, 10.f | .SetPadding(FMargin(16.f, 10.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 823 | SSpinBox<int32> | SetPadding | FT66Style::Tokens::Space5 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space5)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 826 | SSpinBox<int32> | SetPadding | FT66Style::Tokens::Space6 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space6).SetColor(FT66Style::Tokens::Panel)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 847 | STextBlock | Padding | 0.f, 12.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 854 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space6, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 858 | STextBlock | Padding | 0.f, FT66Style::Tokens::Space6, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space6, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 872 | STextBlock | Padding | 18.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 881 | STextBlock | Padding | 0.f, 0.f, 16.f, 0.f | + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 904 | SSpacer | Padding | 0.f, FT66Style::Tokens::Space3, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, FT66Style::Tokens::Space3, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 917 | SScrollBox | Padding | FT66Style::Tokens::Space6, 0.f, 0.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(FT66Style::Tokens::Space6, 0.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 921 | SBox | WidthOverride | 160.f | .WidthOverride(160.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 922 | SBox | HeightOverride | 160.f | .HeightOverride(160.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 934 | STextBlock | Padding | 0.f, 6.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 942 | STextBlock | Padding | 0.f, 8.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 949 | STextBlock | Padding | 0.f, 10.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 952 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66Style::Tokens::Space4)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 957 | STextBlock | SetPadding | FT66Style::Tokens::Space4 | FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4).SetColor(FT66Style::Tokens::Panel)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 983 | SWidgetSwitcher | SetPadding | FT66Style::Tokens::NPCOverlayPadding | FT66PanelParams(ET66PanelType::Bg).SetPadding(FT66Style::Tokens::NPCOverlayPadding).SetColor(FT66Style::Tokens::Bg)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 988 | SWidgetSwitcher | Padding | FT66Style::Tokens::NPCOverlayPadding, FT66Style::Tokens::NPCOverlayPadding, FT66Style::Tokens::NPCOverlayPadding, 0.f | .Padding(FT66Style::Tokens::NPCOverlayPadding, FT66Style::Tokens::NPCOverlayPadding, FT66Style::Tokens::NPCOverlayPadding, 0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 994 | SWidgetSwitcher | SetMinWidth | 0.f | .SetMinWidth(0.f) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 995 | SWidgetSwitcher | SetPadding | FMargin(20.f, 12.f | .SetPadding(FMargin(20.f, 12.f)) |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 1179 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 1272 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66VendorOverlayWidget.cpp | 1551 | Constant | ScaleMult | RunState ? RunState->GetHeroScaleMultiplier() : 1.f | const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f; |
| Source/T66/UI/T66VendorOverlayWidget.h | 89 | Constant | InventorySlotCount | 5 | static constexpr int32 InventorySlotCount = 5; |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 101 | SBorder | Padding | 26.f | .Padding(26.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 104 | SVerticalBox | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 108 | STextBlock | FontSize | 28 | .Font(FT66Style::Tokens::FontBold(28)) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 111 | STextBlock | Padding | 0.f, 0.f, 0.f, 10.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 115 | STextBlock | FontSize | 14 | .Font(FT66Style::Tokens::FontRegular(14)) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 118 | STextBlock | Padding | 0.f, 6.f, 0.f, 12.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 6.f, 0.f, 12.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 124 | SBox | WidthOverride | 220.f | .WidthOverride(220.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 125 | SBox | HeightOverride | 220.f | .HeightOverride(220.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 137 | SBox | WidthOverride | 170.f | .WidthOverride(170.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 138 | SBox | HeightOverride | 170.f | .HeightOverride(170.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 143 | SBorder | Padding | 0.f | .Padding(0.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 152 | SBox | WidthOverride | 150.f | .WidthOverride(150.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 153 | SBox | HeightOverride | 10.f | .HeightOverride(10.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 165 | SBox | WidthOverride | 10.f | .WidthOverride(10.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 166 | SBox | HeightOverride | 150.f | .HeightOverride(150.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 177 | SBorder | Padding | 0.f, 8.f, 0.f, 0.f | .Padding(0.f, 8.f, 0.f, 0.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 180 | SBox | WidthOverride | 18.f | .WidthOverride(18.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 181 | SBox | HeightOverride | 18.f | .HeightOverride(18.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 194 | SBox | WidthOverride | 22.f | .WidthOverride(22.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 195 | SBox | HeightOverride | 22.f | .HeightOverride(22.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 206 | SBorder | Padding | 0.f, 14.f, 0.f, 0.f | + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 14.f, 0.f, 0.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 209 | SHorizontalBox | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 211 | SBox | HeightOverride | 44.f | SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 211 | SBox | MinDesiredWidth | 180.f | SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 225 | STextBlock | Padding | 10.f, 0.f | + SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 227 | SBox | HeightOverride | 44.f | SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f) |
| Source/T66/UI/T66WheelOverlayWidget.cpp | 227 | SBox | MinDesiredWidth | 180.f | SNew(SBox).MinDesiredWidth(180.f).HeightOverride(44.f) |

