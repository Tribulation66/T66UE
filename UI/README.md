# T66 UI Workspace

The active frontend UI pipeline is the flat Slate pipeline. Chrome is built in code through `FT66FlatStyle`; it is not generated as raster art.

Read these in order for frontend screen or modal work:

```text
C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md
C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md
C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md
C:\UE\T66\UI\Reference\UI_STAGE2_CAPTURE_READINESS_REFERENCE.md
C:\UE\T66\Audit\Reference\T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md
```

Special processes are not default frontend screen guidance. Use `UI/Processes/LootUIAnimationAuthoringProcedure.md` only for loot crate/chest/bag/wheel post-interaction 2D/UI animation work, and `UI/Processes/MainMenuVideoBackgroundProcedure.md` only for main-menu background video work.

## Active Rules

- UI chrome is Slate-native: panels, borders, button plates, tab plates, dropdown shells, frames, dividers, tracks, and scroll-like controls must be constructed with `FT66FlatStyle` or equivalent flat Slate helpers.
- Do not generate raster chrome, sprite sheets, button plates, panel plates, or modal shells for frontend migration work.
- Content artwork remains PNG-driven. When production content is missing and the flat fidelity loop calls for a temporary stub, generate a content stub and record it in `UI/content_stubs_registry.md`.
- Missing flat icon glyphs may be generated from exact reference-region crops and recorded in `UI/icon_manifest.md`.
- Live labels, player data, scores, dates, balances, save metadata, selection state, and localized text stay live in Slate/UMG.
- Compile success is not visual proof. Use the fidelity loop, capture/dump evidence, `Scripts/VerifyUIFidelity.py`, and resolution checks as the task requires.

## Working Outputs

Reference screenshots, captures, dumps, compare reports, and pass logs are temporary review outputs:

```text
C:\UE\T66\Saved\Codex\UI\<ScreenName>\
```

Accepted runtime UI assets belong under `RuntimeDependencies`:

```text
C:\UE\T66\RuntimeDependencies\T66\UI\Icons\Flat\
C:\UE\T66\RuntimeDependencies\T66\UI\<TargetOwnedFolder>\
```

Source crops and source-only stub ledgers belong under `UI/` or `SourceAssets/` as named by the fidelity loop:

```text
C:\UE\T66\UI\IconSourceCrops\<ScreenName>\
C:\UE\T66\UI\icon_manifest.md
C:\UE\T66\UI\content_stubs_registry.md
C:\UE\T66\SourceAssets\UI\ContentStubs\<ScreenName>\
```

The retired imagegen-chrome prompt and sprite-sheet workflow were removed from this folder. Recover them from Git history only for historical audit work, not for active UI implementation.

Individual screen agents should use working captures from the local development executable and loose `RuntimeDependencies` while iterating. They should not run full UAT/stage/cook/package for each target. Final packaged verification belongs to the coordinating pass after target work is finished.

