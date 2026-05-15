# Screen Workflow

The active prompt is `C:\UE\T66\UI\MASTER_REFERENCE_UI_GENERATION_PROMPT.md`. Treat this file as a short checklist only; the master prompt carries the full execution contract.

1. Copy the full master prompt into the target chat and fill in every target field.
2. Confirm the exact reference image, target source files, capture script, and staged standalone executable exist before imagegen or edits.
3. Work only on the named target and state. Keep shared top bars and sibling states out of scope unless the prompt explicitly includes them.
4. Archive active generated runtime assets for the target before starting fresh. Do not keep generated reference/proof screenshots in the repo after the pass; keep only accepted runtime assets.
5. Generate a reference-derived text-free sprite/component sheet with built-in imagegen only. API keys, SDK fallback scripts, and inline-preview excuses are not part of this workflow.
6. Run the sprite sheet quality gate before slicing or assembly. If the sheet does not match the reference art family, reject it and generate another sheet.
7. Build a 1920x1080 hierarchy and containment map from the reference before assembly. Every card, panel, row, button, slot, and child control must fit inside its intended parent. If a child overflows, fix the hierarchy size or placement instead of accepting the overflow.
8. Apply `C:\UE\T66\UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`: top-bar screens fill the viewport below the shared top bar, parent panels grow or switch layout when children do not fit, and fixed 1920x1080 shells are not acceptable runtime roots.
9. Store accepted runtime assets under the target folder in `C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens` or `C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Modals`.
10. Implement each component with an explicit resize contract: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill.
11. Preserve the anti-squish button path: sliced plate renderer, nearest filtering, live text, and minimum width clamp.
12. Keep labels, names, values, scores, avatars, selections, and runtime state live. If the reference shows a button, row, skin, slot, or content area that does not have backend/gameplay infrastructure yet, create a visually accurate stub and mark it for later wiring instead of omitting it.
13. Use a normal Unreal build only when C++ changes require it. Do not run UAT/full stage/cook/package for individual target iteration.
14. Capture the working screen from the staged standalone executable, compare against the reference with `C:\UE\T66\Scripts\CompareUIScreen.ps1`, write the difference list, fix the highest-impact difference, and repeat until the owned UI matches.

Working capture command pattern:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -Screen <ScreenKey> -ResX 1920 -ResY 1080 -Output C:\UE\T66\Saved\Codex\UI\<ScreenName>\Proof\<capture_name>.png
```

If the capture script times out, it now checks the cooked sandbox mirror for the requested output path and copies the screenshot back when Unreal wrote it there first.

Do not manually pixel-edit generated UI art with Pillow/PIL or equivalent tools. Bad generated art routes back to imagegen. Bad runtime proportions route to the resize contract and containment map.

