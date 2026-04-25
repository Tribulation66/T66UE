# UI Reconstruction Handoff

## Purpose

This handoff is for continuing the T66 screen-reconstruction process from a reference image, starting with the main menu.

The immediate goal is:

- make packaged runtime screens match approved references as closely as possible
- do it with real widgets, real labels, and production-safe ownership

## First Target

The first active target is the main menu.

Treat the main menu as the golden calibration screen for the stricter workflow. It must establish the reusable acceptance pattern before future screens are considered process-complete:

- canonical packaged comparison target: `1920x1080`
- `1920x1080` is the authoring and baseline review canvas, not the only supported runtime resolution
- acceptable landscape imagegen outputs may be archived and deterministically normalized into the baseline canvas
- badly framed or structurally cropped generated assets are rebuild candidates, not recovery candidates
- approved reference pack owns the offline visual target, not a runtime background
- runtime composition uses a UI-free scene/background plate plus separate foreground component families
- title wordmark may be baked display art
- tagline, CTA labels, top-bar labels, social rows, leaderboard text, ranks, names, scores, and states remain live/localizable
- manifest separates shell rects, visible control rects, and live-content rects
- packaged review uses strict shell diffs plus masks for runtime-owned interiors

## Installed Skill Split

The repo-managed copies now live under `C:\UE\T66\CodexSkills\UI`.

Install or refresh the live local copies with:

`powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Scripts\InstallUIReconstructionSkills.ps1" -Validate`

After bootstrap, the live stage skills exist under `C:\Users\DoPra\.codex\skills`:

- `ui-reconstruction-orchestrator`
- `ui-reference-prep`
- `ui-style-reference`
- `ui-layout-manifest`
- `ui-sprite-families`
- `ui-runtime-reconstruction`
- `ui-packaged-review`

Use `ui-reconstruction-orchestrator` to route work to the correct stage. Use the specialist skills directly when the stage is already known.
Use `ui-reference-prep` only when the active blocker is reference usability, such as deterministic `2x/4x` exports or helper-only AI upscales.
Run the content ownership audit before style-reference generation, sprite-family generation, manifest refinement, or packaged diffing.
Use native Codex `image_gen` only. Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling.

### Primary reference files

- `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`
- per-screen generated references: `C:\UE\T66\UI\screens\<screen_slug>\reference\<screen_slug>_reference_1920x1080.png`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\content_ownership.json`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_layout.json`

### Runtime files

- `C:\UE\T66\Source\T66\UI\Screens\T66MainMenuScreen.cpp`
- `C:\UE\T66\Source\T66\UI\T66FrontendTopBarWidget.cpp`
- `C:\UE\T66\Source\T66\UI\Components\T66LeaderboardPanel.cpp`
- `C:\UE\T66\Source\T66\UI\Style\T66MainMenuReferenceLayout.generated.h`

### Build and asset scripts

- `C:\UE\T66\Scripts\BuildReferenceMainMenuAssets.py`
- `C:\UE\T66\Scripts\SliceMainMenuGeneratedSpriteSheets.py`
- `C:\UE\T66\Scripts\BuildMainMenuCTAButtonAssets.py`

## What We Are Trying To Establish

The real deliverable is not just one fixed main menu. The real deliverable is a repeatable process for reconstructing screens from a reference image.

That process should now be kept deliberately simple:

1. make the assets
2. place the assets

Everything else is a support activity. Asset work owns image generation, slices, state families, alpha, and dimensions. Placement work owns anchors, runtime widgets, real buttons, live text, and dynamic content wells. If a pass starts aligning widgets over a full reference screenshot, stop and simplify the placement contract.

Generated assets must be correct at generation time. Manual pixel editing, cleanup, masks, erase/fill, cover patches, clone/repaint fixes, or screenshot repair are not part of the handoff process. Bad generated assets are rejected and regenerated; accepted boards may only be deterministically sliced/cropped before runtime placement and live text/content overlay.

Reference generation is not completion. A chat or agent must continue through sprite-family generation, runtime implementation, packaged capture, and packaged review unless a concrete blocker stops progress. If blocked, report `blocked` with the exact missing artifact or failing command.

## Current Lessons Learned

### 1. Do not use one flattened image as both layout and art source

The approved reference is good as an offline layout and comparison target. It is not a runtime background, and buttonless/textless full-screen masters are helper references only unless they have been explicitly regenerated as a UI-free scene plate.

### 2. Mixed ownership causes churn

The main menu currently suffers from overlapping ownership:

- top bar shell versus live top-bar buttons
- CTA stack shell versus live CTA buttons

Any future pass should fix ownership first before touching polish.

### 3. Plate proportions matter as much as placement

The recurring stretched-button problem was not only placement. It was also caused by feeding the runtime the wrong aspect-ratio art and then forcing it into measured slots.

This now extends to runtime scaling. The implementation should transform the `1920x1080` reference coordinate system into supported viewports with anchors, safe zones, DPI scaling, and nine-slice/stretch rules; it should not rely on bespoke per-resolution art.

### 4. Packaged validation is mandatory

Several passes that looked plausible in theory failed immediately in packaged runtime because of brush behavior, background contamination, or duplicated baked regions.

### 5. Upscale outputs are helper-first by default

Prepared or AI-upscaled references help inspection and prompting, but they do not fix wrong proportions or ownership by themselves and should default to `helper-only` unless deliberately reviewed.

## Current Main Menu Status

Blunt status: `close with blockers`.

Latest packaged capture:

- new captures should be stored under `C:\UE\T66\UI\screens\main_menu\outputs\YYYY-MM-DD\`
- legacy captures were archived under `C:\UE\T66\UI\archive\output_pre_reference_gate_20260424`
- stale screen-pack artifacts, including raw imagegen sources, old references, prompts, manifests, masks, review notes, diff metrics, and packaged captures, should be archived under `C:\UE\T66\UI\archive\...` before the final style pass

Current implementation state:

- main menu runtime now uses one fixed reference canvas for the screen body
- major body regions are placed from semantic center/left/right calculations
- the old top-level `SConstraintCanvas` alignment experiment was removed
- dead legacy main-menu layer brushes were removed from `T66MainMenuScreen`

The main remaining blockers are:

- asset exactness is still not final; the screen is improved but not an exact reference match
- top bar ownership/alignment should be reviewed as its own foreground component
- leaderboard live-content wells need a tighter measured content contract instead of broad padding
- dynamic Steam/leaderboard portraits, names, scores, and values must stay masked/manual-review regions

## Known Useful Paths

### Main menu reference assets

- `C:\UE\T66\SourceAssets\UI\MainMenuReference\TopBar`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\Center`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\LeftPanel`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\RightPanel`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\SpriteSheets`
- `C:\UE\T66\SourceAssets\UI\MainMenuReference\SheetSlices`

### Recent packaged screenshots

- Use `C:\UE\T66\UI\screens\main_menu\outputs\YYYY-MM-DD\` for new captures.
- Legacy root-output captures were archived under `C:\UE\T66\UI\archive\output_pre_reference_gate_20260424`.

Use them only as diagnostics. They are not acceptance captures.

## Recommended Next-Step Order

1. Asset phase: improve only the weakest foreground families or scene plate that fail the reference comparison.
2. Placement phase: keep the single reference canvas and semantic anchors; do not reintroduce whole-reference alignment overlays.
3. Review phase: package, capture the `1920x1080` baseline, validate the supported aspect buckets, and classify remaining deltas as asset, layout, text-fit, live-data mismatch, or ownership.

## Hard Rules For The Next Model

- do not redesign the screen
- do not use freehand placement nudges when a manifest box exists
- do not rely on pixel cleanup after the fact; it is not an allowed production method
- do not ship the full reference, buttonless master, or textless master as a runtime background
- do not let two layers own the same visible shell region
- do not keep iterating multiple regions in one packaged pass if one of them is already regressing
- do not bake the main menu tagline or any localizable/value text into runtime art
- do not use legacy browser-automation generation or request manifests for image generation
- do not report completion after reference generation alone
- do not accept a sliced asset until dimensions, alpha, nine-slice margins, and state anchors have been checked
- do not run strict diffs across runtime-owned avatars, images, text, values, or media without masks

