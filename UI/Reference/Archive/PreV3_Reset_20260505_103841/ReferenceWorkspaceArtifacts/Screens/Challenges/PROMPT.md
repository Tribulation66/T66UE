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
