# T66 UI Reconstruction Workspace

This is the canonical workspace for UI reconstruction artifacts.

The required process for every screen or modal is:

1. Capture the current runtime screenshot for that exact screen.
2. Write the layout list for that exact screen.
3. Feed all three inputs to image generation:
   - `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
   - the current runtime screenshot of the target screen
   - the target screen layout list
4. Save the generated target reference in the target screen folder.
5. Only then create sprite/component families, place them in Unreal, capture packaged runtime, compare, and iterate.

The current runtime screenshot is the authority for layout, content count, and arrangement. The main-menu reference is only the style anchor. Future reference generations must not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, explanatory labels, or any other structure not present in the current screenshot and layout list.

Current style direction is sleek, modern, minimalist, clean planar surfaces, crisp borders, flat/satin metallic accents, restrained gold, and a clean red/black/charcoal scheme while preserving the same font, layout, and content. Do not prompt toward grain, cracked stone, gemstone/crystal/beveled fantasy surfaces, noisy distressed panels, rubble texture, or micro-detail borders.

Deprecated or stale targets are not valid generation tasks. Do not create active screen-specific references for `wheel_overlay`, `party_size_picker`, `casino_overlay`, `gambler_overlay`, `vendor_overlay`, standalone `leaderboard`, legacy `Lobby`, `LobbyReadyCheck`, `LobbyBackConfirm`, `HeroLore`, or `CompanionLore` screens. Use canonical active names: `powerup` instead of `shop`, and `minigames` instead of `unlocks`.

Skipping step 3 is a blocking process failure. A screen that only borrows the general main-menu style without a generated screen-specific reference is not production work.

Stopping after step 4 is also a process failure. Reference generation is not completion. A chat or agent may report a screen complete only after sprite/component generation, runtime implementation, packaged capture, and review are done, or after it reports a concrete blocker with the missing artifact or failing command.

Use Codex-native `image_gen` only for UI image generation. Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling.

`1920x1080` is the canonical authoring and review baseline, not the only supported runtime resolution. Imagegen outputs that are already landscape-safe may be normalized into that baseline with `Scripts\InvokeDeterministicResample.py --target-width 1920 --target-height 1080`, which center-crops to 16:9 and resamples with Lanczos while preserving the raw source beside it. If normalization crops important UI/title/content, reject the output and regenerate it. Square, portrait, or badly framed generations are not repair candidates.

Runtime UI must be scalable. Build from anchors, safe zones, DPI scaling, nine-slice/stretch rules, and live text/content wells, then validate packaged captures across aspect buckets instead of authoring separate bespoke art for every resolution.

## Folder Contract

Each screen or modal lives under:

```text
C:\UE\T66\UI\screens\<screen_slug>\
  reference\
  current\
  layout\
  assets\
  outputs\
    YYYY-MM-DD\
  review\
```

Use `reference` for the generated canonical target image. Use `current` for the pre-style runtime capture. Use `layout` for the layout list and measurement notes. Use `assets` for source image-generation boards and slice notes. Use `outputs/YYYY-MM-DD` for packaged captures, diffs, and pass artifacts.

During style discovery and before the final reference pass, keep each screen folder lean: one current-state PNG where available, one approved canonical reference PNG when it exists, and no stale generated boards, masks, diffs, temporary prompts, or packaged captures in the active folder. Move old artifacts to `UI\archive\...` with their relative paths preserved.

Runtime-imported final assets may still live under `SourceAssets` or `RuntimeDependencies` if the code expects them there, but their source, reference, and review artifacts belong in this UI workspace.

## Packaged Capture Launcher

Use this launcher for packaged screenshots:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenKey> -Output C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\packaged_capture.png
```

It resolves Windows display 1 and passes the correct `-WinX` and `-WinY` flags so Unreal opens on display 1 by default.

## Agent Setup

When a UI pass contains more than one independent screen, modal, asset family, or review task, deploy subagents immediately instead of doing the whole pass serially.

Good splits:

- one agent audits/captures current runtime screenshots and writes layout lists
- one agent generates or reviews screen-specific reference requirements
- one agent works on isolated asset/component families
- one agent performs packaged-review classification while the main thread patches runtime placement

Keep write ownership explicit. Do not assign two agents to edit the same source file or asset folder at the same time. Every agent must follow the reference gate in `SCREEN_WORKFLOW.md`.

Every agent must also follow the completion gate: do not final-answer after reference generation alone. Continue through assets, runtime, packaged capture, and review unless blocked.

## Anchor

The single canonical main-menu anchor is:

```text
C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png
```

Do not use older main-menu screenshots from `SourceAssets` or the archived root `output` folder as style anchors.

