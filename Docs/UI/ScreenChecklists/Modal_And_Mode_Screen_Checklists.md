# Modal And Mode Screen Asset/State Checklists

Use these as preflight checklists before reference generation, sprite-family slicing, or packaged capture review. They are intentionally concise and do not replace a filled screen intake.

Global rules:

- Runtime art must be text-free except intentional title/logo display art.
- Scene/background plates must not contain panels, buttons, rows, cards, avatars, icons, values, or localizable labels.
- Visible controls must be real runtime controls with separate state art.
- Live regions must be masked or fixture-frozen before strict image diffing.

## Blocking Modals

Screens: `QuitConfirmation`, `Leaderboard` modal, `PauseMenu`, `ReportBug`, `PartyInvite`.

Scene/background needs:

- Dim/scrim overlay that reads over gameplay and frontend backgrounds.
- Modal-safe ambient background or blurred/dimmed scene source; no baked modal panel in the background.
- Optional narrow confirmation vignette for `QuitConfirmation` and `PartyInvite`; keep it UI-free.

Panel/control families and states:

- Modal shell: normal, focused/topmost, inactive-behind-modal.
- Header strip and close/back affordance: normal, hover, pressed, disabled.
- Primary/secondary/destructive buttons: normal, hover, pressed, disabled, keyboard/gamepad focused.
- `Leaderboard`: filter tabs/buttons, dropdown chips, entry rows, rank badges, avatar frames, local-player highlight, loading/empty/error rows, selected/clickable run-summary row.
- `PauseMenu`: menu option buttons, resume/retry/settings/quit variants, achievement/progress row chrome.
- `ReportBug`: category selector, text-entry shell, checkbox/toggle, submit/cancel buttons, pending/success/failure banner.
- `PartyInvite`: invite row, avatar frame, accept/decline buttons, pending/expired/blocked states.

Runtime-owned live regions:

- Modal titles, body copy, warnings, keybind hints, localized CTA labels.
- Leaderboard rows: rank, name, score/time, party size, difficulty, hero portrait/avatar, friend/global filter state, async loading/error text.
- Pause achievement names/progress values and current input prompts.
- Report text field contents, validation messages, submit state.
- Party member names, presence, avatars, invite status, platform/network errors.

Capture blockers:

- Scrim opacity differs between capture runs or leaks gameplay motion into strict-diff regions.
- Any row text, rank, avatar, or filter label baked into chrome art.
- Keyboard/gamepad focus state missing from button and row assets.
- Leaderboard async data not fixture-frozen, producing unstable row counts or avatar timing.
- Text-entry caret/blink not disabled or masked for `ReportBug`.

## Roster And Picker Screens

Screens: `HeroGrid`, `CompanionGrid`, `PlayerSummaryPicker`.

Scene/background needs:

- Full-screen UI-free selection backdrop aligned with the approved frontend shell.
- Optional left/right decorative gutters for ultrawide; no baked cards, stats, portraits, labels, or CTA stack.
- Preview area plate only if it is empty of runtime hero/companion/player content.

Panel/control families and states:

- Grid card shell: normal, hover, pressed, selected, disabled/locked, owned, equipped/current, affordable/unaffordable where relevant.
- Portrait/frame family: empty, loaded, loading placeholder, locked silhouette.
- Stat/detail panel shell: normal, active selection, comparison mode.
- Filter/search/sort controls: normal, hover, pressed, selected, disabled.
- CTA buttons: select/equip/buy/view/back with normal, hover, pressed, disabled, focused.
- `PlayerSummaryPicker`: summary row/card, selected row, unavailable row, loading/empty/error rows.

Runtime-owned live regions:

- Hero, companion, and player names; localized ability/stat descriptions; prices; owned/equipped labels.
- Dynamic stat numbers, modifiers, rarity, unlock conditions, and preview thumbnails.
- Player summary metadata: score/time, date, difficulty, party size, hero/companion icons, run eligibility.

Capture blockers:

- Portraits or names baked into card art instead of runtime apertures.
- Selected/locked/equipped states visually jump between anchors.
- Long localized names clip inside cards or CTAs at `1280x720`.
- Async thumbnail loading changes comparison captures.
- Grid scroll offset or selected item is not fixed for review.

## Challenge, Buff, And Summary Screens

Screens: `Challenges`, `TemporaryBuffSelection`, `TemporaryBuffShop`, `RunSummary`.

Scene/background needs:

- UI-free secondary-screen backdrop with enough contrast for dense list/card surfaces.
- Buff/shop surfaces need an empty shop-counter or altar-like plate only if foreground shelves/cards remain separate.
- `RunSummary` may use a celebratory background plate, but stats, medals, rewards, leaderboard status, and CTAs remain live.

Panel/control families and states:

- Challenge row/card: incomplete, complete, claimed, locked, pinned/tracked, hover, selected.
- Buff card: normal, hover, selected, disabled, max-selected, affordable/unaffordable, owned/active.
- Shop offer card: normal, hover, pressed, locked, sold, reroll-pending, insufficient-currency, disabled.
- Buff icon frame family for primary/secondary stat groups and empty slots.
- Summary stat panels, reward rows, item/buff grids, rating badges, leaderboard eligibility banner.
- CTA stack: continue, retry, submit leaderboard, anonymous submit, inspect details, back with all button states.

Runtime-owned live regions:

- Challenge names/descriptions/progress/reward values and timer/reset labels.
- Buff names, icons, stat deltas, duration, selection count, currency, reroll cost, lock state.
- Run summary values: score, time, kills, damage, stages, difficulty, party size, hero/companion/loadout, rewards, leaderboard submission state.

Capture blockers:

- Stat icons or buff interiors baked into card shells when they should be runtime-swappable.
- Selection count, currency, or leaderboard eligibility not fixture-frozen.
- Shop reroll/lock state missing from art states.
- `RunSummary` has live result values baked into the reference or background.
- Dense row/card text lacks wrap/scroll policy for localized strings.

## Mini Chadpocalypse Screens

Screens: `MiniMainMenu`, `MiniSaveSlots`, `MiniCharacterSelect`, `MiniCompanionSelect`, `MiniDifficultySelect`, `MiniIdolSelect`, `MiniShop`, `MiniRunSummary`.

Scene/background needs:

- Mini-owned UI-free frontend backdrop; do not reuse main or TD plates in place unless explicitly approved as a shared neutral source.
- Main shell supports left friends panel, center CTA/menu, and right leaderboard panel without baking those panels into the background.
- Selection/shop screens need empty mini-themed scene plates with separate foreground card/shelf/detail families.
- Run summary needs a UI-free celebration/results backdrop with all results runtime-owned.

Panel/control families and states:

- Mini top/header bar, friends panel chrome, leaderboard panel chrome, CTA stack, save slot card, character/companion/difficulty/idol cards, shop offer cards, item rows, summary stat panels.
- Common states: normal, hover, pressed, disabled, focused, selected, locked, owned, equipped, affordable/unaffordable, loading, empty, error.
- `MiniSaveSlots`: empty slot, occupied slot, autosave/resume, delete-confirm, corrupted/unavailable.
- `MiniShop`: lock, sold, reroll, continue, insufficient materials.
- `MiniRunSummary`: score/rank badge, reward row, leaderboard submit/sync state.

Runtime-owned live regions:

- Friends/presence names and avatars, mini leaderboard ranks/scores, save metadata, hero/companion/idol names and stats, difficulty descriptions, shop prices/materials, locked offers, run result values.
- Any generated sprites, portraits, icons, or data-table-driven thumbnails remain runtime-swappable.

Capture blockers:

- Mini UI accidentally uses regular-game or TD asset ownership paths.
- Friends/leaderboard async data changes between captures.
- Save slots not fixture-seeded, causing unstable occupied/empty states.
- Shop lock/reroll state not represented in both art and fixture data.
- Card grids lack locked/selected/focused variants for controller validation.

## Tower Defense Screens

Screens: `TDMainMenu`, `TDDifficultySelect`, `TDBattle`.

Scene/background needs:

- TD-owned UI-free backgrounds for menu and difficulty/map browser; do not bake Mini Chadpocalypse panels into TD plates.
- Difficulty/map browser needs empty map-preview frames and themed background variants for Dungeon, Forest, Ocean, Martian, and Hell.
- `TDBattle` needs map background plates per selected map, with the board/path environment separate from HUD chrome unless intentionally part of the map art.

Panel/control families and states:

- `TDMainMenu`: center CTA buttons, title/logo art, back button, side-panel shells if mirroring Mini shell.
- `TDDifficultySelect`: difficulty cards, map cards, preview frame, locked/unlocked/current-selected states, mode chip for Regular.
- `TDBattle`: top resource bar, wave/status panel, tower palette buttons, selected-tower info panel, upgrade buttons, sell button, speed/play-pause button, back/quit confirm affordance.
- Common states: normal, hover, pressed, disabled, focused, selected, locked, affordable/unaffordable, buildable/blocked, active wave, paused.

Runtime-owned live regions:

- TD menu labels, difficulty/map names, map thumbnails, descriptions, unlock status, selected hero/loadout if surfaced.
- Battle values: materials, hearts, wave, threat/status, selected tower name/body, upgrade costs, range/damage/tempo values, speed state, enemy/tower/projectile positions, countdowns.

Capture blockers:

- TD screen plates reuse Mini art or contain baked Mini shell panels.
- Battle board is captured with nondeterministic enemy/projectile/tower motion.
- Map thumbnails/previews not fixture-frozen.
- Upgrade affordability and selected tower state missing from visual state set.
- HUD values baked into battle chrome instead of runtime text.
