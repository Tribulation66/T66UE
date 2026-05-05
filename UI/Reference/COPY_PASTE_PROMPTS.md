# Reference UI Copy-Paste Prompts

This file contains the full prompt text for each screen/modal, numbered for easy copy/paste into separate Codex chats.

Each prompt starts with the target name so the chat title is easy to identify.

## Prompt 01 - MainMenu

```text
MainMenu

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only MainMenu match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\screens\main_menu\reference\Reference.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\MainMenu
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\MainMenu
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp
- C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp
- C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\MainMenu. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for MainMenu before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for MainMenu.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the MainMenu-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 02 - HeroSelection

```text
HeroSelection

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only HeroSelection match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroSelection.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\HeroSelection
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Private.h
- C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_RetroFX.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for HeroSelection before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for HeroSelection.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the HeroSelection-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 03 - CompanionSelection

```text
CompanionSelection

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only CompanionSelection match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\CompanionSelection.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\CompanionSelection
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66CompanionSelectionScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\CompanionSelection. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for CompanionSelection before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for CompanionSelection.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the CompanionSelection-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 04 - SaveSlots

```text
SaveSlots

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only SaveSlots match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\SaveSlots.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\SaveSlots
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66SaveSlotsScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\SaveSlots. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for SaveSlots before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for SaveSlots.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the SaveSlots-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 05 - Settings

```text
Settings

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only Settings match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Settings.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\Settings
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\Settings\T66SettingsScreen_Private.h

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\Settings. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for Settings before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for Settings.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the Settings-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 06 - LanguageSelect

```text
LanguageSelect

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only LanguageSelect match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\LanguageSelect.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\LanguageSelect
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\LanguageSelect
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66LanguageSelectScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\LanguageSelect. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for LanguageSelect before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for LanguageSelect.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the LanguageSelect-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 07 - PowerUp

```text
PowerUp

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only PowerUp match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\PowerUp
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for PowerUp before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for PowerUp.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the PowerUp-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 08 - Achievements

```text
Achievements

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only Achievements match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Achievements.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\Achievements
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66AchievementsScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\Achievements. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for Achievements before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for Achievements.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the Achievements-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 09 - Challenges

```text
Challenges

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only Challenges match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\Challenges.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\Challenges
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66ChallengesScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\Challenges. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for Challenges before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for Challenges.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the Challenges-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 10 - DailyClimb

```text
DailyClimb

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only DailyClimb match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\screens\daily_climb\reference\daily_climb_reference_1920x1080.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\DailyClimb
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\DailyClimb
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66DailyClimbScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\DailyClimb. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for DailyClimb before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for DailyClimb.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the DailyClimb-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 11 - AccountStatus

```text
AccountStatus

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only AccountStatus match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatus.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\AccountStatus
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for AccountStatus before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for AccountStatus.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the AccountStatus-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 12 - TemporaryBuffSelection

```text
TemporaryBuffSelection

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only TemporaryBuffSelection match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\TemporaryBuffSelection.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\TemporaryBuffSelection
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\TemporaryBuffSelection
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66TemporaryBuffSelectionScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\TemporaryBuffSelection. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for TemporaryBuffSelection before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for TemporaryBuffSelection.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the TemporaryBuffSelection-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 13 - TemporaryBuffShop

```text
TemporaryBuffShop

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only TemporaryBuffShop match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\TemporaryBuffShop.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\TemporaryBuffShop
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\TemporaryBuffShop
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66TemporaryBuffShopScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\TemporaryBuffShop. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for TemporaryBuffShop before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for TemporaryBuffShop.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the TemporaryBuffShop-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 14 - RunSummary

```text
RunSummary

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only RunSummary match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\RunSummary.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\RunSummary
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66RunSummaryScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\RunSummary. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for RunSummary before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for RunSummary.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the RunSummary-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 15 - Minigames

```text
Minigames

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only Minigames match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\screens\minigames\reference\minigames_reference_1920x1080.png
Workspace folder: C:\UE\T66\UI\Reference\Screens\Minigames
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Screens\Minigames
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66MinigamesScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Screens\Minigames. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for Minigames before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for Minigames.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the Minigames-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 16 - ReportBug

```text
ReportBug

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only ReportBug match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\ReportBug.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\ReportBug
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\ReportBug
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66ReportBugScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\ReportBug. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for ReportBug before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for ReportBug.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the ReportBug-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 17 - QuitConfirmation

```text
QuitConfirmation

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only QuitConfirmation match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\QuitConfirmation.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\QuitConfirmation
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\QuitConfirmation
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66QuitConfirmationModal.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\QuitConfirmation. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for QuitConfirmation before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for QuitConfirmation.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the QuitConfirmation-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 18 - PartyInvite

```text
PartyInvite

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only PartyInvite match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PartyInvite.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\PartyInvite
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\PartyInvite
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66PartyInviteModal.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\PartyInvite. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for PartyInvite before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for PartyInvite.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the PartyInvite-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 19 - PauseMenu

```text
PauseMenu

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only PauseMenu match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PauseMenu.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\PauseMenu
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\PauseMenu
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66PauseMenuScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\PauseMenu. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for PauseMenu before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for PauseMenu.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the PauseMenu-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 20 - PlayerSummaryPicker

```text
PlayerSummaryPicker

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only PlayerSummaryPicker match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PlayerSummaryPicker.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\PlayerSummaryPicker
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\PlayerSummaryPicker
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66PlayerSummaryPickerScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\PlayerSummaryPicker. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for PlayerSummaryPicker before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for PlayerSummaryPicker.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the PlayerSummaryPicker-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 21 - SavePreview

```text
SavePreview

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only SavePreview match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\SavePreview.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\SavePreview
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\SavePreview
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66SavePreviewScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\SavePreview. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for SavePreview before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for SavePreview.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the SavePreview-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 22 - HeroGrid

```text
HeroGrid

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only HeroGrid match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroGrid.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\HeroGrid
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\HeroGrid
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66HeroGridScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\HeroGrid. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for HeroGrid before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for HeroGrid.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the HeroGrid-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

## Prompt 23 - CompanionGrid

```text
CompanionGrid

Read C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\Docs\UI\UI_GENERATION.md and follow it as the global authority.

Goal: make only CompanionGrid match its reference image through the official imagegen-to-runtime process. Work screen-by-screen/modal-by-modal; do not broaden into other targets.

Reference image: C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\CompanionGrid.png
Workspace folder: C:\UE\T66\UI\Reference\Modals\CompanionGrid
Runtime asset folder: C:\UE\T66\SourceAssets\UI\Reference\Modals\CompanionGrid
Target source files:
- C:\UE\T66\Source\T66\UI\Screens\T66CompanionGridScreen.cpp

Hard rules for this chat:
- Do not embed generated imagegen outputs in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proofs as local links/paths.
- Keep all labels, names, stats, scores, dates, save data, owned counts, currency values, avatars, portraits, and selection state live in Slate/UMG. Generated art is chrome only.
- Generate text-free sprite sheets or direct transparent component PNGs for missing UI element families. Bad art routes back to imagegen, not local pixel repair.
- Store accepted runtime assets only under C:\UE\T66\SourceAssets\UI\Reference\Modals\CompanionGrid. If another screen/modal already has a useful asset, duplicate it into this folder and rename it for CompanionGrid before routing this target to it.
- Do not route this target to another screen/modal asset folder. SourceAssets\UI\Reference\Shared is only a bootstrap/compatibility source; copy from it into this target folder before claiming the target is individualized.
- Preserve the anti-squish button technique: non-square buttons must use the Reference sliced plate path, nearest filtering, zero brush margin, live text over the plate, and T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth or equivalent minimum width.
- Use explicit resize contracts: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill. Never stretch a whole ornate PNG unless it is authored for that.
- Stage and capture the packaged screen after each meaningful pass. Compile success is not visual proof.

First pass checklist:
1. Inventory the visible UI families in the reference and current runtime capture for CompanionGrid.
2. List missing or mismatched component families: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
3. Generate the missing text-free component art with imagegen and place accepted PNGs in this target runtime asset folder.
4. Update only the target source files needed to load the CompanionGrid-owned assets and apply the correct resize contract.
5. Run a packaged capture, compare against the reference, write the difference list, and repeat.
```

