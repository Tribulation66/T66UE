# Reference UI Implementation Purpose

The images in this workspace exist to make each T66 frontend screen or modal visually match its approved reference while keeping the game UI live and data-driven.

## What These Images Are For

- Text-free UI chrome: panels, rows, buttons, tabs, dropdowns, fields, slots, scrollbars, icons, dividers, meters, and modal shells.
- Screen-owned runtime assets under `C:\UE\T66\SourceAssets\UI\Reference\Screens\<ScreenName>\`.
- Modal-owned runtime assets under `C:\UE\T66\SourceAssets\UI\Reference\Modals\<ModalName>\`.
- Iterative proof work where each target can be handled in a separate Codex chat without changing another target's asset folder.

## What These Images Are Not For

- They are not full-screen runtime screenshots.
- They are not replacements for live Slate/UMG controls.
- They must not bake player names, stats, balances, dates, scores, save metadata, labels, selection state, portraits, or localized text.
- They must not be shared as mutable source art across screens. If a target needs an asset from another target or from `Shared`, duplicate it into the target folder and rename it for that target.

## Required Runtime Contract

Every accepted PNG needs an explicit runtime contract:

- Fixed image for icons, badges, ornaments, portraits, and small square slots.
- Horizontal 3-slice for long buttons, pills, tabs, and compact selectors.
- Vertical 3-slice for scroll rails or vertical thumbs.
- 9-slice for panels, dropdown shells, frames, and fields.
- Tiled or fill interiors only when the source was generated for that use.

Non-square buttons must preserve the anti-squish technique: sliced plate renderer, nearest filtering, zero brush margin, live text over the image, and a minimum width clamp.

## Handoff Flow

1. Read `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`.
2. Read `C:\UE\T66\Docs\UI\UI_GENERATION.md`.
3. Open `C:\UE\T66\UI\Reference\PROMPT_INDEX.md`.
4. Start one new Codex chat per screen or modal.
5. Paste the target prompt. It begins with the target name for chat-title visibility.
6. Generate missing component art with imagegen, save accepted assets into that target's runtime folder, implement the resize contract, stage, capture, compare, and repeat.

Generated image outputs should be reported as local file links/paths, not embedded in chat, unless the user explicitly asks for inline previews.
