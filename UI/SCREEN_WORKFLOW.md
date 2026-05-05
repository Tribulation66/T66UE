# Screen Workflow

The authoritative process is `C:\UE\T66\Docs\UI\UI_GENERATION.md`.

Use this file as a quick checklist:

1. Open the target prompt from `C:\UE\T66\UI\Reference\PROMPT_INDEX.md`.
2. Work only on that screen or modal unless the user explicitly expands scope.
3. Use image generation to create enough text-free sprite sheets and tight transparent component PNGs for missing UI chrome.
4. Store accepted runtime assets under the target folder in `C:\UE\T66\SourceAssets\UI\Reference\Screens` or `C:\UE\T66\SourceAssets\UI\Reference\Modals`.
5. Duplicate and rename reused assets into the current target folder before routing code to them.
6. Implement each component with an explicit resize contract: fixed image, horizontal 3-slice, vertical 3-slice, 9-slice, or intentional tiled fill.
7. Preserve the anti-squish button path: sliced plate renderer, nearest filtering, live text, and minimum width clamp.
8. Keep labels, names, values, scores, avatars, selections, and runtime state live.
9. Stage new runtime assets and capture the packaged screen.
10. Compare packaged capture against the reference, write the difference list, fix the highest-impact difference, and repeat.

Capture command pattern:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenKey> -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\<ScreenName>\Proof\<capture_name>.png
```

Do not manually pixel-edit generated UI art with Pillow/PIL or equivalent tools. Bad generated art routes back to image generation. Bad runtime proportions route to the resize contract.
