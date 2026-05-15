# Reference Screen And Modal Individualization Task

This file is the task-specific handoff for the current T66 Reference UI pass. It exists so each new Codex chat can work on one screen or modal with enough context, without needing the original conversation.

## Source Of Authority

Read these in order:

1. `C:\UE\T66\UI\Instructions\UI_SCREEN_MODAL_INSTRUCTIONS.md`
2. `C:\UE\T66\UI\Instructions\UI_GENERATION_INSTRUCTIONS.md`
3. `C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md`

`UI_GENERATION_INSTRUCTIONS.md` is the global UI generation process. The master prompt is the active copy/paste handoff for each target chat.

## Why This Pass Exists

The previous shared Reference UI asset library helped prove the process, but it is too coupled for the remaining screens. Many screens need UI elements that do not exist in the current shared asset set. The goal now is to bridge those gaps per screen or modal by generating missing UI chrome and routing the target to its own assets.

This is intentionally not a DRY asset-library approach. Each screen/modal gets its own asset folder so different chats can work independently and avoid stepping on each other's source art.

## Folder Model

Screen-owned runtime assets:

```text
C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens\<ScreenName>\
```

Modal-owned runtime assets:

```text
C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Modals\<ModalName>\
```

Compatibility/bootstrap assets:

```text
C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Shared\
```

Active prompt:

```text
C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md
```

## Asset Ownership Rule

Accepted runtime art for a target must live in that target's folder. If a useful asset already exists in `Shared` or in another target folder, duplicate it into the current target folder, rename it for the current target, and route the current target to that copy.

Do not route a completed screen/modal directly to another screen/modal folder. `Shared` can be used as a temporary bootstrap source, but the target is not individualized until touched assets are copied into that target's folder.

## What The Images Are For

Generate and use images only for text-free UI chrome:

- panels
- rows
- modal shells
- buttons and button states
- tabs
- dropdowns and fields
- slots and frames
- scrollbars
- icons
- dividers
- meters and progress tracks
- decorative screen-specific UI elements

Keep live in Slate/UMG:

- labels
- localized text
- names
- stats
- scores
- dates
- save metadata
- balances and costs
- portraits and avatars
- owned counts
- selection state
- runtime state

## Required Imagegen Process

For each target:

1. Compare the exact reference image to the current working capture when one exists.
2. Inventory missing or mismatched UI families.
3. Use imagegen to create text-free sprite sheets or direct transparent component PNGs.
4. Compare the generated text-free sprite/component sheet against the reference before slicing or assembly. Reject the sheet if it does not match the reference UI family.
5. Build a hierarchy and containment map from the reference so each child control fits inside its intended parent panel, card, row, or shell.
6. Save accepted runtime PNGs under the target runtime asset folder.
7. Implement the assets with explicit resize contracts.
8. Stub any visible reference element that does not have backend/gameplay infrastructure yet, and mark it for later wiring.
9. Run a normal Unreal build only if source changed.
10. Capture the working screen from `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
11. Compare against the reference with `C:\UE\T66\Scripts\CompareUIScreen.ps1` and repeat.

Do not use Pillow/PIL or local pixel repair for generated UI art. Bad generated art goes back to imagegen. Bad runtime proportions go to the resize contract.

## Anti-Squish Button Rule

The previous successful fix for cramped/squished buttons must be preserved.

Non-square buttons need:

- horizontal sliced plate rendering
- nearest filtering
- zero brush margin
- live text above the plate
- a minimum width clamp, using `T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth` or an equivalent target-local wrapper

Do not draw ornate button PNGs as ordinary stretched `SImage`, `SBorder`, or default `FButtonStyle` box art.

## Resize Contracts

Every accepted component needs one explicit contract:

- fixed image
- horizontal 3-slice
- vertical 3-slice
- 9-slice
- intentional tiled/fill interior

Never stretch an ornate full PNG unless the asset was authored for that.

## Chat Output Rule

Do not embed generated imagegen outputs directly in chat unless the user explicitly asks for inline previews. Report generated candidates, accepted assets, and proof captures as local links or paths.

## Verification Requirement

Compile success is not enough. Each implementation pass must produce an existing working screenshot or a true blocker status, then compare the working screenshot to the reference with `C:\UE\T66\Scripts\CompareUIScreen.ps1`.

Individual target chats should not run UAT, BuildCookRun, cook, stage, pak, or package. Final packaged verification belongs to the coordinating pass after target work is complete.

## Current Target Set

Screens:

- MainMenu
- HeroSelection
- CompanionSelection
- SaveSlots
- Settings
- LanguageSelect
- PowerUp
- Achievements
- Challenges
- DailyDescent
- AccountStatus
- TemporaryBuffSelection
- TemporaryBuffShop
- RunSummary
- Minigames

Modals:

- ReportBug
- QuitConfirmation
- PartyInvite
- PauseMenu
- PlayerSummaryPicker
- SavePreview
- HeroGrid
- CompanionGrid

Start from `C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md`, fill in the target fields, and paste the full prompt into a new chat.

