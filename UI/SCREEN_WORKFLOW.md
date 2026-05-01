# Screen Workflow

This is the blocking workflow for every T66 screen, modal, HUD overlay, tab, and mini-game UI.

## Image Generation Path

Use Codex-native `image_gen` only. Do not use legacy browser-automation generation, request JSONs, token-driven local services, or removed external image-generation tooling for UI reconstruction.

If native generation cannot produce the required reference, scene plate, component board, icon, or state family, mark the screen blocked and name that artifact. Do not fall back to removed external generation tooling.

For master UI asset-library elements, Codex-native image generation is the only allowed source for new or replacement visual chrome. The source image must come from imagegen using the approved reference; bad source art is regenerated with imagegen, not locally repaired.

Hard ban: do not use Pillow/PIL anywhere in the master UI asset-library chrome pipeline. This includes scripts, Python helpers, alpha cleanup, matte removal, resizing, cropping, slicing, proofing, or any "minor" repair.

Do not use procedural scripts, Slate drawing, fill rectangles, crop-patch repainting, or scripted wood/metal/paper synthesis to create or repaint buttons, panels, top bars, slots, paper, bars, tabs, dropdowns, select controls, or other reusable chrome. Local scripts may only run after an approved imagegen board exists, and only for rectangular slicing, deterministic resizing that preserves pixels, copying, naming, manifests, alpha validation, and replacing already-background pixels outside the component silhouette with transparent black.

Do not apply despill, edge cleanup, color mutation, repainting, sharpening, blurring, corner repair, or local pixel repair to master-library UI chrome. Alpha work is allowed only outside the intended component silhouette. Any removed background pixel must be alpha 0 with RGB 0,0,0; do not leave green, magenta, checkerboard gray, white, paper-colored, or any other visible RGB in transparent pixels because Unreal filtering and mips can bleed that color around the edges.

Rounded, oval, chamfered, or otherwise non-rectangular controls must ship as alpha-bearing PNG slices with transparent corners; opaque rectangular matte/corner pixels are not acceptable runtime assets. A rounded or organic button inside a visible square backing is a failed asset, not a layout issue.

Button text must render directly on the generated button surface. Do not add a separate rectangular text plate, label backing, or brown square under text. For the main-menu wood family, basic buttons are rectangular dark-mahogany planks with thin antique-gold bevels and black pixel outlines, not gold capsules, pointy chevrons, or arrow-ended controls. Invite and offline use the small rounded pill style from the reference with centered text. Basic panels include a dark mahogany fill plus border; paper background is only an inner content material and starts where the reference paper starts.

Imagegen does not have to emit `1920x1080` directly. For landscape-safe outputs, keep the raw generation and normalize a copy with:

```powershell
python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png --target-width 1920 --target-height 1080
```

This conversion is only an authoring-canvas normalization step: center-crop to 16:9, Lanczos resize, then inspect. If the crop loses important UI, title, character, or screen structure, reject the output and regenerate instead of repairing pixels.

## Blocking Reference Gate

Before asset generation or Unreal placement begins, the target screen must have a generated screen-specific reference image.

Required inputs to image generation:

- canonical style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- current screenshot: the exact current runtime screenshot of the target screen or modal
- layout list: the exact list of regions, controls, live-data wells, states, and modal/tab variants for the target screen

The current target screenshot is authoritative for layout, content count, and arrangement. Use the main-menu reference only for visual style. Do not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, explanatory labels, or any other structure not present in the current screenshot and layout list.

Style-positive direction: sleek, modern, minimalist, clean planar surfaces, crisp borders, flat or satin metallic accents, restrained gold, and a clean red/black/charcoal scheme while keeping the same font, layout, and content.

Style-negative direction: no grain, no cracked stone, no gemstone/crystal/beveled fantasy surface, no noisy distressed panels, no rubble texture, and no micro-detail borders.

Required output:

```text
C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png
```

Do not proceed to sprites, Slate/C++ styling, or packaged comparison without this file. If the generated reference does not preserve the target layout or does not match the canonical main-menu style, regenerate it.

Reference generation is not a completed UI pass. It only unlocks the next step.

## Screen Pack Checklist

For each screen folder:

1. Put the current runtime screenshot in `current/YYYY-MM-DD/`.
2. Put the layout list in `layout/layout_list.md`.
3. Generate the screen-specific target reference from the three required inputs.
4. Fill the element/state checklist before sprite generation.
5. Generate or regenerate component boards until the checklist is complete.
6. Slice only accepted generated boards. Do not manually repair pixels.
7. Place assets in Unreal using semantic anchors and real controls.
8. Capture packaged runtime at the authoring baseline, normally `1920x1080`.
9. Save captures, diffs, and review notes under `outputs/YYYY-MM-DD/` and `review/`.
10. Iterate by comparing packaged capture against the screen-specific reference, not against a vague style description.
11. Before calling the screen production-ready, validate scalable runtime behavior across the standard aspect buckets: primary `16:9`, `16:10`, ultrawide `21:9`, and one smaller/windowed size if supported.

Do not send a final/completed answer after steps 1-3 alone. A screen is complete only after steps 1-10 have run, or after a concrete blocker is reported with the exact missing artifact, failing command, and next required step.

## Required Agent Setup For Multi-Screen Work

If the assignment includes multiple screens, modals, HUD overlays, tabs, or asset families, deploy agents at the start. Do not wait until the main thread is overloaded.

Use agents for independent work only:

- capture/audit agent: current screenshots, source-file inventory, live-content notes, layout lists
- reference agent: image-generation prompt/input preparation and reference-gate validation
- asset agent: component checklist, sprite family requirements, generated-board validation
- review agent: packaged screenshot comparison, blocker classification, diff/mask notes

Each agent must receive:

- the canonical anchor path
- the target `UI\screens\<screen_slug>` folder
- the exact files it owns
- the rule that no runtime styling starts without `reference\<screen_slug>_reference_1920x1080.png`
- the rule that reference generation alone is not completion; agents must continue through sprite generation, runtime implementation, packaged capture, and packaged review unless blocked

Do not let two agents write the same runtime file or asset folder. If file ownership cannot be separated, keep that part local in the main thread.

## Display-1 Capture Rule

Use the shared launcher for packaged UI captures:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenKey> -Output C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\packaged_capture.png
```

This launcher resolves Windows display 1 and passes `-WinX` and `-WinY` so the game opens on display 1 instead of the primary/right-hand monitor. Do not use ad hoc `Start-Process` screenshot commands unless you also include the same display-1 `-WinX` and `-WinY` flags.

## Button Text Fit Rule

Button labels must use the Main Menu CTA text-fit approach instead of tiny fixed labels. The caller's explicit `FontSize` is the intended maximum size; do not force short labels larger just because the button is tall. If no font size is provided, use a height-derived fallback around `0.42 * button height`, cap the resolved size around `0.52 * button height`, use `LetterSpacing = 0..1`, center the text inside the plate with minimal content padding, and protect long localized labels with ellipsis plus a down-only `SScaleBox`. Do not solve fit by dropping button labels to 10-12px on 34-58px plates; that makes the UI look broken even when it technically avoids clipping.

## Hard Rules

- No screen may be styled from "general main-menu vibes" alone.
- Deprecated or stale targets are not valid generation tasks. Do not generate active references for `wheel_overlay`, `party_size_picker`, `casino_overlay`, `gambler_overlay`, `vendor_overlay`, standalone `leaderboard`, legacy `Lobby`, `LobbyReadyCheck`, `LobbyBackConfirm`, `HeroLore`, or `CompanionLore` screens.
- Use canonical active naming: `powerup` instead of `shop`, and `minigames` instead of `unlocks`.
- The current target screenshot owns layout, content count, and arrangement; the main-menu anchor owns style only.
- Do not add ability slots, idol slots, extra buttons, invented panels, icons, meters, currencies, menu entries, tabs, or explanatory labels.
- Positive style is sleek, modern, minimalist, clean planar surfaces, crisp borders, flat/satin metallic accents, restrained gold, and a clean red/black/charcoal scheme with the same font/layout/content.
- Negative style guardrails: no grain, no cracked stone, no gemstone/crystal/beveled fantasy surface, no noisy distressed panels, no rubble texture, and no micro-detail borders.
- No raw non-canonical generated image may become a production reference, sprite sheet, scene plate, slice, or runtime asset. Normalize acceptable landscape generations into the canonical `1920x1080` authoring baseline with `Scripts\InvokeDeterministicResample.py --target-width 1920 --target-height 1080`; reject square, portrait, badly framed, or structurally cropped outputs.
- No full-screen reference image may ship as the runtime background.
- No generated asset may be manually pixel-edited, masked, cover-patched, clone-painted, or repaired.
- No UI pass may use legacy browser-automation image generation or request manifests.
- No Pillow/PIL may be used anywhere in the master asset-library chrome pipeline, including cleanup, slicing, resizing, alpha work, proofing, or small repairs.
- No master asset-library UI chrome may be invented or repainted with procedural generation; bad chrome routes back to Codex-native image generation.
- No master asset-library UI chrome may be despilled, edge-cleaned, color-mutated, repainted, sharpened, blurred, corner-repaired, or locally pixel-repaired after generation. Alpha work is allowed only outside the intended component silhouette and must write transparent pixels as alpha 0 with RGB 0,0,0.
- No rounded, chamfered, oval, or organic control may ship with a visible square/rectangular backing layer or tinted transparent RGB that can bleed through runtime filtering.
- No button label may sit on a separate rectangular text plate; labels render directly on the button surface.
- No agent may report completion immediately after reference generation.
- Bad generated pixels route back to image generation.
- Layout must come from the target screen, not from a different screen.
- Localizable labels, names, values, scores, prices, timers, and dynamic data stay live.
- Visible controls must be real controls, not invisible hotspots.
- Buttons, custom clickable areas, panels, tabs, rows, slots, and top-bar controls must route through the shared UI helpers (`FT66Style`, `T66OverlayChromeStyle`, `T66MiniUI`, `T66MiniGeneratedChrome`, or an approved screen-family helper). If a production screen needs a new visual/control variant, extend a helper first instead of adding raw `SButton` or production chrome `SBorder` directly in the screen.
- Outputs go under `UI\screens\<screen_slug>\outputs\YYYY-MM-DD\`, not the repo-root `output` folder.
- Active screen folders should stay clean between passes: keep only the current-state PNG and the approved canonical reference PNG in active `current`/`reference` folders; archive stale boards, masks, prompts, review outputs, and packaged captures under `UI\archive`.

## Iteration Loop

For every pass:

```text
reference -> component checklist -> generated assets -> runtime placement -> packaged screenshot -> review deltas -> next pass
```

The review deltas must name missing or wrong items as `reference`, `asset`, `layout`, `text-fit`, `ownership`, or `live-data` problems.

