# T66 HUD and In-Game UI Flat Migration Inventory

Generated during the 2026-05-12 audit session.

This inventory covers UI surfaces outside the frontend screens already completed in the Stage 2 progress log. It is source-backed: rows are based on C++/Slate/Canvas widget classes and the runtime creation paths that add them to the viewport or widget components.

The requested technical handoff path, `C:\UE\T66\UI\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`, was not present in this checkout. The audit used `UI_FIDELITY_LOOP_INSTRUCTIONS.md`, `UI_FLAT_REDESIGN_REFERENCE.md`, `Saved\Codex\UI\overnight_progress_log.md`, and source inspection instead.

Standard legacy chrome regex used:

```powershell
rg -n "SourceAssets/UI/Reference|RuntimeDependencies/T66/UI/Reference|MakeReference|Get.*ReferenceScrollBarStyle|ST66Reference|ST66RetroUIRetainedSurface|M_UI_Glow|M_UI_RetroRetainer|MakeRetroUIChromeSurface|MakeRetroUIChromeOverlay"
```

Important interpretation: `regex matches = 0` does not mean the widget is flat-migrated. Many runtime HUDs use custom Slate, Canvas drawing, `FT66Style`, or gameplay-specific colors without hitting the reference-chrome regex.

## Summary

- Total source-backed widget/surface rows identified: 38
- Total rows with standard legacy chrome regex matches greater than 0: 4
- Total rows with standard regex count 0: 34
- Rows already proven as Stage 2 frontend migrations were excluded from the main inventory unless they are shared components that still have independent legacy chrome paths.
- Binary Blueprint/UMG-only assets were not found as readable `*WBP*.uasset` matches under `Content` during this pass, so the inventory is C++/Slate/Canvas-backed.

### Breakdown by Category

| Category | Count |
|---|---:|
| HUD | 10 |
| Overlay | 6 |
| In-world | 2 |
| Shop | 9 |
| Modal | 1 |
| Minigame-HUD | 1 |
| Other | 9 |

### Top 5 Highest-Complexity Items

1. `UT66GameplayHUDWidget` - Complex because it is the primary active gameplay HUD and contains health/status, ability cooldowns, inventory/economy, minimap/full-map, interaction prompts, boss bars, pickup cards, crate/chest presentations, and legacy reference chrome matches in the same construction path.
2. `AT66MiniBattleHUD` - Complex because it is an `AHUD` Canvas renderer, not a Slate tree, and uses many gameplay-semantic colors for hearts, timer danger, XP, ultimate readiness, boss health, and loot crate presentation.
3. `UT66GamblerOverlayWidget` - Complex because it owns multiple casino pages and couples UI to run economy, inventory, wager state, boss anger, and minigame result state.
4. `UT66CasinoShopTabWidget` - Complex because it shares the in-run casino host, uses run-state economy/inventory delegates, and has purchase/steal/alchemy behavior that needs interaction verification beyond visual chrome.
5. `UT66LeaderboardPanel` - Complex because it is a large shared component with 22 standard legacy chrome matches, old hover behavior, state plates, dropdowns, pagination/rows, and likely cross-screen usage.

### Ambiguity and Audit Limits

- `AT66PlayerController` exposes DeletedTheme-theme override class slots for HUD and overlays (`DeletedThemeGameplayHUDClass`, `DeletedThemeCasinoOverlayClass`, `DeletedThemeCollectorOverlayClass`, `DeletedThemeCowardicePromptClass`, `DeletedThemeIdolAltarOverlayClass`). If assigned to Blueprint subclasses, those subclasses are not separately source-readable here. They inherit the same categories as their base widgets but should be checked in-editor before a HUD migration plan is locked.
- `T66HUDPresentationController`, `T66TemporaryBuffUIUtils`, and `T66ItemCardTextUtils` are not standalone rendered widgets. They are helper/controller paths feeding `UT66GameplayHUDWidget` presentations, so their UI impact is captured under that HUD row.
- `T66ScreenSlateHelpers`, `FT66Style`, `T66RuntimeUITextureAccess`, and `T66RuntimeUIBrushAccess` still contain global legacy helpers, but they are infrastructure rather than UI elements. They should be handled during Stage 3/global cleanup, or during a targeted migration only when the audited widget still reaches them.
- The completed Stage 2 frontend screens from `overnight_progress_log.md` are excluded: History, Diplomas, Drugs, Steam Achievements, Minigames, Settings tabs, Daily Descent, Challenges, Run Summary, Main Menu, Pause Menu, modals, selection grids, minigame menu screens, TD screens, Idle screens, and Deck screens.

## Inventory

| # | Widget name | File path | Visual description | Current chrome state | Regex matches | Category | Migration complexity | Gameplay-readability variant |
|---:|---|---|---|---|---:|---|---|---|
| 1 | `UT66GameplayHUDWidget` | `C:\UE\T66\Source\T66\UI\T66GameplayHUDWidget.h`; `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_*.cpp`; `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h` | Primary active-run HUD: hearts/status, ability and ultimate cooldowns, idol and inventory slots, gold/debt, boss bars, minimap/full map, pickup cards, crate/chest presentations, interaction prompts, and world dialogue. | Uses reference square panel/slot PNG paths and `MakeReferenceProgressBar`; also uses extensive custom Slate and old `FT66Style` helpers. | 4 | HUD | Complex - many independent gameplay states and presentations live in one widget path. | Yes - Needs gameplay-readability variant. |
| 2 | `ST66RingWidget` | `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h` | Custom leaf widget used by the gameplay HUD for circular/ring-style presentation such as cooldown or scope-style UI. | Regex-clean; custom Slate paint path inside the gameplay HUD private header, not `FT66FlatStyle`. | 0 | HUD | Medium - leaf paint behavior must be preserved while restyling the owning HUD. | Yes - overlays active gameplay. |
| 3 | `ST66DotWidget` | `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h` | Small dot/marker leaf widget used by HUD indicator clusters. | Regex-clean; custom Slate leaf widget, not flat-tagged. | 0 | HUD | Simple - isolated paint primitive, but state colors need review. | Yes - overlays active gameplay. |
| 4 | `ST66CrosshairWidget` | `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h` | Crosshair-style leaf widget displayed during combat/aiming contexts. | Regex-clean; custom Slate paint path, not `FT66FlatStyle`. | 0 | HUD | Medium - aiming visibility depends on contrast against world art. | Yes - Needs gameplay-readability variant. |
| 5 | `ST66ScopedSniperWidget` | `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h` | Scoped/sniper overlay presentation for the scoped ultimate or aimed mode. | Regex-clean; custom Slate paint path, not `FT66FlatStyle`. | 0 | HUD | Complex - full-screen aiming overlay with gameplay readability constraints. | Yes - Needs gameplay-readability variant. |
| 6 | `ST66WorldMapWidget` | `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Private.h`; `C:\UE\T66\Source\T66\UI\HUD\T66GameplayHUDWidget_Map.cpp` | Minimap and full-map renderer with player, enemy, POI, tower, reveal, and marker layers. | Regex-clean; custom Slate map rendering and `FT66Style` minimap colors, not flat chrome. | 0 | HUD | Complex - map colors encode gameplay meaning and need separate map-readability rules. | Yes - Needs gameplay-readability variant. |
| 7 | `UT66HeroCooldownBarWidget` | `C:\UE\T66\Source\T66\UI\T66HeroCooldownBarWidget.h`; `C:\UE\T66\Source\T66\UI\T66HeroCooldownBarWidget.cpp` | World/widget-component cooldown bar attached to the hero, shown over gameplay. | Regex-clean; simple UUserWidget/Slate construction with custom cyan/yellow colors. | 0 | HUD | Simple - small bar, but color semantics need preservation. | Yes - Needs gameplay-readability variant. |
| 8 | `UT66EnemyHealthBarWidget` | `C:\UE\T66\Source\T66\UI\T66EnemyHealthBarWidget.h`; `C:\UE\T66\Source\T66\UI\T66EnemyHealthBarWidget.cpp` | Enemy health and lock presentation above enemies. | Regex-clean; custom Slate colors with red health/fill and dark backing. | 0 | HUD | Medium - world-space health state and lock visibility must stay readable. | Yes - Needs gameplay-readability variant. |
| 9 | `UT66EnemyLockWidget` | `C:\UE\T66\Source\T66\UI\T66EnemyLockWidget.h`; `C:\UE\T66\Source\T66\UI\T66EnemyLockWidget.cpp` | Lock-on indicator attached to enemies. | Regex-clean; custom widget-component UI, not `FT66FlatStyle`. | 0 | HUD | Simple - focused indicator, but gameplay contrast matters. | Yes - Needs gameplay-readability variant. |
| 10 | `UT66FloatingCombatTextWidget` | `C:\UE\T66\Source\T66\UI\T66FloatingCombatTextWidget.h`; `C:\UE\T66\Source\T66\UI\T66FloatingCombatTextWidget.cpp` | Floating damage/combat numbers spawned from `AT66FloatingCombatTextActor`. | Regex-clean; custom floating text widget, not flat chrome. | 0 | HUD | Simple - mostly text animation/color, no chrome shell. | Yes - Needs gameplay-readability variant. |
| 11 | `UT66CrateOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66CrateOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66CrateOverlayWidget.cpp` | In-run crate/chest opening overlay and reward presentation host. | Regex-clean; custom Slate and old style usage through HUD presentation handoff. | 0 | Overlay | Medium - coupled to reward reveal timing and rarity visuals. | Yes - overlays gameplay. |
| 12 | `UT66IdolAltarOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66IdolAltarOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66IdolAltarOverlayWidget.cpp` | In-run idol/level-up choice overlay with selectable upgrade cards. | Regex-clean; custom Slate/old style path, not flat-tagged. | 0 | Overlay | Complex - choice cards are gameplay-affecting and data-driven. | Yes - modal over gameplay. |
| 13 | `UT66LabOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66LabOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66LabOverlayWidget.cpp` | In-run lab overlay for upgrade/lab interactions. | Regex-clean; custom Slate/old style construction, not `FT66FlatStyle`. | 0 | Overlay | Medium - transactional choices and run-state coupling. | Yes - modal over gameplay. |
| 14 | `UT66CollectorOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66CollectorOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66CollectorOverlayWidget.cpp` | Full-screen in-run Collector NPC interface. | Regex-clean; custom Slate/old style construction, not flat-tagged. | 0 | Overlay | Medium - NPC shop/collector data and selection state. | Yes - modal over gameplay. |
| 15 | `UT66CowardicePromptWidget` | `C:\UE\T66\Source\T66\UI\T66CowardicePromptWidget.h`; `C:\UE\T66\Source\T66\UI\T66CowardicePromptWidget.cpp` | Prompt/confirmation overlay for Cowardice Gate interaction. | Regex-clean; custom prompt styling, not `FT66FlatStyle`. | 0 | Overlay | Simple - focused prompt with a small interaction contract. | Yes - modal over gameplay. |
| 16 | `UT66LoadingScreenWidget` | `C:\UE\T66\Source\T66\UI\T66LoadingScreenWidget.h`; `C:\UE\T66\Source\T66\UI\T66LoadingScreenWidget.cpp` | Full-screen loading overlay added at high Z order. | Regex-clean; no reference chrome match and no flat tagging. | 0 | Overlay | Simple - static/low-interaction full-screen overlay. | No - blocks gameplay rather than overlaying active combat. |
| 17 | `AT66WorldInteractableBase` | `C:\UE\T66\Source\T66\Gameplay\T66WorldInteractableBase.h`; `C:\UE\T66\Source\T66\Gameplay\T66WorldInteractableBase.cpp` | Base in-world interactable that participates in targeting and interaction prompt flow. | Regex-clean; no direct Slate chrome, but drives `UT66GameplayHUDWidget` interaction prompts. | 0 | In-world | Medium - not itself a widget, but migration depends on prompt contract and target labels. | Yes - prompt must be readable over gameplay. |
| 18 | `AT66TutorialPromptActor` | `C:\UE\T66\Source\T66\Gameplay\T66TutorialPromptActor.h`; `C:\UE\T66\Source\T66\Gameplay\T66TutorialPromptActor.cpp` | Tutorial prompt actor/signage path used for in-world guidance. | Regex-clean; no reference chrome match found. | 0 | In-world | Medium - actor-driven prompt content likely needs runtime readability review. | Yes - in-world text or prompt over gameplay. |
| 19 | `UT66CasinoOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66CasinoOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66CasinoOverlayWidget.cpp`; `C:\UE\T66\Source\T66\UI\T66CasinoOverlayShared.h` | Host overlay for casino/shop/gambling tabs during a run. | Regex-clean; shared host uses custom Slate and creates `UT66GamblerOverlayWidget`/`UT66CasinoShopTabWidget`. | 0 | Shop | Complex - shared in-run shop host with tab routing and run-state delegates. | Yes - modal over gameplay. |
| 20 | `UT66GamblerOverlayWidget` | `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.h`; `C:\UE\T66\Source\T66\UI\T66GamblerOverlayWidget.cpp`; `C:\UE\T66\Source\T66\UI\Gambler\T66GamblerOverlayWidget_*.cpp` | In-run gambler interface with dialogue, casino selection, coin flip, RPS, blackjack, lottery, plinko, and box opening pages. | Regex-clean; extensive custom Slate/old style usage, no flat tagging. | 0 | Shop | Complex - multiple mini-interactions, economy and boss anger coupling. | Yes - modal over gameplay. |
| 21 | `UT66CasinoShopTabWidget` | `C:\UE\T66\Source\T66\UI\T66CasinoShopTabWidget.h`; `C:\UE\T66\Source\T66\UI\T66CasinoShopTabWidget.cpp` | In-run casino shop/alchemy tab with item cards and purchase/steal actions. | Regex-clean; custom Slate/old style usage, no flat tagging. | 0 | Shop | Complex - economy, inventory, debt, anger, steal permissions, and item data. | Yes - modal over gameplay. |
| 22 | `UT66ArcadePopupWidget` | `C:\UE\T66\Source\T66\UI\T66ArcadePopupWidget.h`; `C:\UE\T66\Source\T66\UI\T66ArcadePopupWidget.cpp` | Abstract/shared base for in-world arcade popup games launched from arcade interactables. | Regex-clean; base class, not a complete flat surface. | 0 | Shop | Medium - base lifecycle and completion/reward hooks need preservation. | Yes - modal over gameplay. |
| 23 | `UT66ArcadeSelectionWidget` | `C:\UE\T66\Source\T66\UI\T66ArcadeSelectionWidget.h`; `C:\UE\T66\Source\T66\UI\T66ArcadeSelectionWidget.cpp` | Arcade random-selection popup that lets the player choose from arcade game entries. | Regex-clean; custom Slate and old style usage. | 0 | Shop | Medium - data-driven entry list and selection behavior. | Yes - modal over gameplay. |
| 24 | `UT66WhackAMoleArcadeWidget` | `C:\UE\T66\Source\T66\UI\T66WhackAMoleArcadeWidget.h`; `C:\UE\T66\Source\T66\UI\T66WhackAMoleArcadeWidget.cpp` | Full Whack-a-Mole arcade popup with 3x3 board, score/time/combo HUD, lives, hover hammer, and mole states. | Regex-clean; bespoke Slate, sprite art, and per-cell hover state. | 0 | Shop | Complex - active minigame loop, hover gameplay affordance, timer/state machine. | Yes - active minigame UI. |
| 25 | `UT66TopwarArcadeWidget` | `C:\UE\T66\Source\T66\UI\T66TopwarArcadeWidget.h`; `C:\UE\T66\Source\T66\UI\T66TopwarArcadeWidget.cpp` | Topwar arcade popup with bespoke game UI. | Regex-clean; custom Slate/old style usage. | 0 | Shop | Medium - arcade gameplay state and scoring. | Yes - active minigame UI. |
| 26 | `UT66GoldMinerArcadeWidget` | `C:\UE\T66\Source\T66\UI\T66GoldMinerArcadeWidget.h`; `C:\UE\T66\Source\T66\UI\T66GoldMinerArcadeWidget.cpp` | Gold Miner arcade popup with bespoke game UI. | Regex-clean; custom Slate/old style usage. | 0 | Shop | Medium - arcade gameplay state and scoring. | Yes - active minigame UI. |
| 27 | `UT66QuickArcadeWidget` | `C:\UE\T66\Source\T66\UI\T66QuickArcadeWidget.h`; `C:\UE\T66\Source\T66\UI\T66QuickArcadeWidget.cpp` | Fallback quick arcade popup for arcade game types without a dedicated widget. | Regex-clean; custom Slate/old style usage. | 0 | Shop | Medium - covers many arcade variants through one generic presentation. | Yes - active minigame UI. |
| 28 | `UT66MiniPauseMenuWidget` | `C:\UE\T66\Source\T66Mini\Public\UI\T66MiniPauseMenuWidget.h`; `C:\UE\T66\Source\T66Mini\Private\UI\T66MiniPauseMenuWidget.cpp` | Runtime Mini pause/settings modal with resume, settings, save/quit, sliders, toggles, key rebinding, and settings tabs. | Regex-clean; custom Mini UI style and raw Slate controls, not `FT66FlatStyle`. | 0 | Modal | Complex - runtime pause modal with settings persistence and input capture. | Yes - modal over active minigame. |
| 29 | `AT66MiniBattleHUD` | `C:\UE\T66\Source\T66Mini\Public\UI\T66MiniBattleHUD.h`; `C:\UE\T66\Source\T66Mini\Private\UI\T66MiniBattleHUD.cpp` | Actual Mini battle HUD rendered during Mini gameplay: hearts, timer, score, XP, ultimate, boss bar, pause prompt, and loot crate presentation. | Regex-clean; Canvas `DrawHUD` rendering, not Slate and not `FT66FlatStyle`. | 0 | Minigame-HUD | Complex - Canvas renderer with many gameplay-semantic colors and timed overlays. | Yes - Needs gameplay-readability variant. |
| 30 | `FT66OverlayChromeStyle` | `C:\UE\T66\Source\T66\UI\Style\T66OverlayChromeStyle.h`; `C:\UE\T66\Source\T66\UI\Style\T66OverlayChromeStyle.cpp` | Shared overlay chrome helper used by several in-run overlays for panels and overlay shells. | Uses `SourceAssets/UI/Reference/Shared`, `MakeReferenceSharedAssetPath`, `MakeReferenceHorizontalSlicedImage`, and `MakeRetroUIChromeSurface`. | 4 | Other | Complex - shared helper can affect multiple runtime overlays at once. | Depends on consuming overlay; likely yes. |
| 31 | `UT66LeaderboardPanel` | `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.h`; `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.cpp` | Shared leaderboard component with filters, dropdowns, rows, pagination/state plates, avatars, and score/speedrun views. | Uses reference shared dirs, reference buttons, reference sliced images, reference dropdowns, and reference state plate buttons. | 22 | Other | Complex - legacy-heavy shared component with hover and multiple control types. | No for active gameplay; yes if embedded in in-run overlay later. |
| 32 | `UT66FrontendBackButtonWidget` | `C:\UE\T66\Source\T66\UI\T66FrontendBackButtonWidget.h`; `C:\UE\T66\Source\T66\UI\T66FrontendBackButtonWidget.cpp` | Shared standalone frontend back-button chrome widget managed by `UT66UIManager`. | Uses reference pill button brushes and reference horizontal sliced image. | 4 | Other | Medium - shared chrome; isolated, but likely appears outside screen-specific Stage 2 paths. | No - frontend/shared chrome. |
| 33 | `UT66Button` | `C:\UE\T66\Source\T66\UI\Components\T66Button.h`; `C:\UE\T66\Source\T66\UI\Components\T66Button.cpp` | UMG-style reusable button wrapper with hover delegates and visual state callbacks. | Regex-clean; old component-level hover path, not `FT66FlatStyle` metadata. | 0 | Other | Medium - generic reusable button could have broad callers. | Depends on caller. |
| 34 | `UT66LeaderboardFilterButton` | `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardFilterButton.h`; `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardFilterButton.cpp` | Reusable leaderboard filter button component. | Regex-clean; not flat-tagged. | 0 | Other | Simple - small component, but depends on leaderboard panel migration. | No for active gameplay. |
| 35 | `ST66AnimatedBackground` | `C:\UE\T66\Source\T66\UI\ST66AnimatedBackground.h`; `C:\UE\T66\Source\T66\UI\ST66AnimatedBackground.cpp` | Animated background Slate widget used as a non-flat ambience/backdrop element. | Regex-clean; custom Slate animation and old style hooks. | 0 | Other | Simple - content/background effect, not chrome. | No unless used behind active gameplay overlay. |
| 36 | `ST66PulsingIcon` | `C:\UE\T66\Source\T66\UI\ST66PulsingIcon.h`; `C:\UE\T66\Source\T66\UI\ST66PulsingIcon.cpp` | Pulsing icon Slate widget used for attention/status emphasis. | Regex-clean; custom Slate icon effect, not flat-tagged. | 0 | Other | Simple - isolated effect; decide whether it remains content art or becomes flat icon helper. | Depends on caller. |
| 37 | `T66StatsPanelSlate` | `C:\UE\T66\Source\T66\UI\T66StatsPanelSlate.h`; `C:\UE\T66\Source\T66\UI\T66StatsPanelSlate.cpp` | Reusable stats panel Slate component. | Regex-clean; custom/old style Slate component, not `FT66FlatStyle`. | 0 | Other | Medium - reusable panel with data rows and likely legacy-style assumptions. | Depends on caller. |
| 38 | `ST66DevConsole` | `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp` | Developer console overlay built as a local Slate class inside frontend controller code. | Regex-clean; custom Slate overlay, not `FT66FlatStyle` and not part of Stage 2 screen verification. | 0 | Other | Simple - debug-only surface unless exposed in runtime builds. | No, unless used during gameplay debugging. |

## Composite Coverage Notes

`UT66GameplayHUDWidget` is one widget class but contains many of the individual elements listed in the prompt. Its row covers:

- Health/status display and hearts.
- Ability, ultimate, and scoped-shot cooldown displays.
- Currency/economy display, including gold and debt.
- Run/presentation text, score-like status text, and boss presentation text.
- Idol, portrait, inventory, item/buff, and temporary presentation icon slots.
- Minimap and full-map display.
- Boss health and boss-part bars.
- Interaction prompts and world dialogue options.
- Pickup notification cards, crate opening UI, and chest reward presentations.
- Inventory inspect mode and interactive HUD mode.

These should probably become separate checklist regions in a future HUD migration even though they share one current source class.

## Analysis

### A. Gameplay readability versus menu palette

Yes, several active gameplay widgets would be harmed by blindly applying the menu palette and universal hover rules.

Examples found in source:

- Enemy health bars use red fill/danger colors and dark backing. That is gameplay state, not menu chrome.
- Mini battle HUD uses Canvas colors for hearts, low-time danger, ultimate readiness, XP, boss health, and loot crate presentation.
- Gameplay HUD map markers use different colors for enemies, POIs, miasma, floor art, reveal state, and player direction.
- Boss bars, boss-part bars, health displays, rarity accents, pickup card colors, and inventory item colors encode runtime meaning.

Implication: HUD and in-run UI need a dedicated gameplay-readability variant or a separate HUD palette contract. The menu palette can inform chrome shapes, but it should not replace health, danger, rarity, map marker, cooldown, or state colors.

### B. Menu UI versus HUD UI separation

There is a mostly clean architectural separation:

- Frontend/menu screens are primarily `UT66ScreenBase` classes under `Source\T66\UI\Screens` or minigame screen modules, routed through `UT66UIManager` and verified with the Stage 2 dump/checklist loop.
- Runtime HUD and in-run overlays are mostly `UUserWidget`, `AHUD`, widget-component, or plain Slate/Canvas classes under `Source\T66\UI`, `Source\T66\UI\HUD`, `Source\T66Mini\Private\UI`, and gameplay controller overlay paths.

There are still shared code paths:

- `FT66Style` and `T66ScreenSlateHelpers` are global legacy style/helper surfaces.
- `FT66OverlayChromeStyle` is a shared in-run overlay chrome helper with direct reference-chrome matches.
- Some runtime widgets use `FT66Style` for fonts, panels, buttons, or minimap colors.

Implication: systematic migration should not run the Stage 2 screen loop unchanged against all HUDs. Use a HUD-specific loop that keeps gameplay semantic colors and world readability, then retire shared legacy helpers only after every consumer is routed away.

### C. Existing hover states versus universal green hover

There are existing hover behaviors that could conflict with a universal green hover rule:

- `UT66WhackAMoleArcadeWidget` uses hover to show the hammer/cell highlight in a gameplay board. Replacing that with a generic green UI hover would obscure the minigame affordance.
- `UT66LeaderboardPanel` has old hover variants and row hover state.
- `UT66Button` wires `OnHovered` and `OnUnhovered` at the component level.
- `UT66GameplayHUDWidget` has inventory hover/tooltip behavior in inspect mode.

Most HUD elements are not standard hover-enabled menu controls. For runtime widgets, hover should be opt-in per interactive control and should preserve gameplay-specific hover affordances.

### D. Minigame HUD and shop framework separation

Minigame HUDs are not implemented through one shared HUD base:

- Mini gameplay uses bespoke `AT66MiniBattleHUD`, an `AHUD` Canvas renderer.
- TD, Idle, and Deck menu/gameplay screens listed in the progress log are Stage 2-migrated `UT66ScreenBase` style screens where applicable.
- No shared minigame HUD base class equivalent to `UT66ScreenBase` was found for actual in-run minigame HUD rendering.

Arcade and shop surfaces are partially shared but still bespoke:

- Arcade interactions share `UT66ArcadePopupWidget` as a base and route through `AT66PlayerController::SpawnArcadePopupWidget`.
- Dedicated arcade widgets exist for Whack-a-Mole, Topwar, and Gold Miner; other arcade game types use `UT66QuickArcadeWidget`; random selection uses `UT66ArcadeSelectionWidget`.
- Casino/gambler flows share `UT66CasinoOverlayWidget` and `T66CasinoOverlayShared.h`, but `UT66GamblerOverlayWidget` and `UT66CasinoShopTabWidget` implement their own complex UI surfaces.

Implication: a future migration should group arcade popup framework work separately from gambler/casino shop work, and Mini battle HUD separately from frontend minigame menu screens.

## Recommended Next Audit Boundaries

No implementation changes were made in this session. If Pablo turns this inventory into a migration plan, the next clean split is:

1. Gameplay HUD contract pass: define a HUD palette/readability variant and break `UT66GameplayHUDWidget` into checklist regions.
2. Shared overlay chrome pass: migrate `FT66OverlayChromeStyle` consumers or route them to a HUD-safe flat helper.
3. Casino/Gambler pass: migrate `UT66CasinoOverlayWidget`, `UT66GamblerOverlayWidget`, and `UT66CasinoShopTabWidget` together.
4. Arcade popup pass: migrate `UT66ArcadePopupWidget` plus the concrete arcade widgets together.
5. Mini runtime pass: migrate `AT66MiniBattleHUD` and `UT66MiniPauseMenuWidget` with a Canvas-aware verification method.
