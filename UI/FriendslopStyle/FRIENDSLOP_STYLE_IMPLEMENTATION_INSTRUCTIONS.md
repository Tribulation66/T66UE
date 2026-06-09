# FriendslopStyle Implementation Instructions

Status: Approved for FriendslopStyle screen implementation. Main Menu was the
pilot approved on 2026-06-05.

Global chrome flip (2026-06-09): all `FT66FlatStyle` chrome entry points
(`MakeFlatPanelSurface` choke point, `MakeHudPanel`, legacy `MakeButton`) render
FriendslopStyle plates behind `T66.UI.FriendslopGlobal` (default on;
`-T66FlatLegacy` escape). Every screen not yet hand-migrated renders Friendslop
chrome through this adapter using the shared MainMenu plate family. Hand-migrated
screens (direct `FT66FriendslopStyle` calls) are unaffected. Per-screen
high-fidelity passes under this document remain the path for screens that need
dedicated reference-driven plates; the flip sets the global baseline, not the
per-screen ceiling. Implementation: `Source/T66/UI/Style/T66FlatStyle.cpp`,
commit d4123783a on branch `friendslop-migration`.
Authority: this file is the single source of truth for FriendslopStyle UI
process. `UI/FriendslopStyle/README.md` is the folder router, but this file owns
method class, visual asset provenance, runtime ownership, and acceptance gates.

This file defines the production pipeline for turning an approved FriendslopStyle
reference image into real T66 Unreal Slate UI. The first pilot target is Main
Menu Round06:

`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

The Round06 reference is a visual direction target, not a runtime UI texture.
Runtime UI must be built from tagged Slate widgets, live text/data, and reusable
sliced raster chrome assets.

## 1. Approval Boundary

Do not begin FriendslopStyle implementation until the user approves this file.
Approval means:

- FriendslopStyle becomes a separate style lane from FlatStyle.
- Generated raster chrome is allowed only inside this FriendslopStyle lane.
- FlatStyle's no-raster-chrome visual rule remains valid for FlatStyle work.
- The user also approves a follow-up `UI/UI_AGENTS.md` router amendment that
  scopes the current global no-raster-chrome hard rule to FlatStyle and routes
  FriendslopStyle work through this file.
- FriendslopStyle still inherits tagged dumps, responsive layout rules, capture
  requirements, contact-sheet evidence, worker provenance, and the live-text
  prohibition. It does not use `VerifyUIFidelity.py`, visual scorecard
  `Result`, or `FULL/PARTIAL` as the final Friendslop acceptance language.
- `UI/UI_AGENTS.md` should be updated in the implementation pass to route
  FriendslopStyle work here and to scope the existing raster-chrome ban to
  FlatStyle unless otherwise stated.

No implementation pass may treat approval of the Round06 mockup as approval to
paste the mockup into the UI or to bake text, player data, scores, online state,
or localization into raster art.

## 2. Method Class

FriendslopStyle uses this method class:

1. ImageGen creates full-screen visual references for art direction.
2. The approved reference is decomposed into reusable UI surface categories.
3. The load-bearing surfaces are authored as final transparent PNG plates or
   plate families through account-backed built-in imagegen executed in a
   separate local Codex CLI worker, or through a separately documented
   user-approved exception. Manual/Pillow/OpenCV/skimage tools may perform
   mechanical packaging and QA only; they may not author, patch, inpaint, clone,
   smooth, recolor, synthesize, or salvage production UI pixels.
4. Plates are sliced only when a min/normal/wide test proves their bevels,
   highlights, shadows, and material read survive scaling. If a plate cannot be
   sliced cleanly, create a size-specific plate instead of stretching it.
5. Unreal renders the final authored plates as Slate `Box`/9-slice,
   horizontal-sliced, or fixed-image brushes.
6. Slate owns layout, live labels, live player data, icons, click handlers,
   hover/pressed/disabled state, and responsive behavior.
7. Every implementation pass evaluates the full screen element inventory. No
   pass may focus only on the most recently discussed component while leaving
   other non-matching elements unclassified.
8. The screen is captured and documented against the reference using geometry,
   dump, contact sheet, family/element coverage, worker records, sizing/fitting
   notes, and a wiring/functionality gate. The user owns final visual judgment
   for element fidelity and layout match.

The cheapest wrong result is a full-screen paintover or a few rubber-looking
buttons placed approximately on the screen. The discriminator is that every
load-bearing region from the reference has a matching tagged runtime widget,
correct normalized geometry, correct live content, correct reusable chrome
asset, and a full-screen contact sheet that still reads like the approved
reference.

This is an explicit amendment to the first Main Menu pilot approach: generic
blank rubber atoms are not enough for high-fidelity FriendslopStyle work. Unreal
owns placement, live content, interactivity, and state wiring; the PNG plate owns
the premium material quality.

## 2.1 Visual Asset Authorship Authority

Production FriendslopStyle visual pixels are authored only by account-backed
built-in imagegen run from a separate local Codex CLI worker, or by a specific
user-approved exception documented in the active screen contract. Do not use
OpenAI API scripts, `OPENAI_API_KEY`, web image URLs, browser screenshots, or
the main Codex app chat image tool for Friendslop iteration generation unless
the user explicitly approves a different process in that turn.

Reference crops are measurement and comparison targets only. They are not
runtime asset sources. A title is never cropped from the full reference image.
Title art must be either a clean title-only generated asset or a live Slate title
treatment with measured containment.

Textless reference breakdowns are a required art-direction artifact, not a
runtime asset source. For each approved full-screen reference, run one fresh
local Codex CLI worker to produce a clean no-text/no-data version of the
reference that preserves the family silhouettes, panel/button shapes, material
read, shadows, spacing, and background framing. The textless reference is then
mechanically cropped into one context per user-declared visual family and cached
for future iterations against that same approved reference. Because removing
text is visual authorship, the textless reference must be generated by the same
account-backed CLI-worker process as other Friendslop visual assets; it may not
be created by manual inpaint, clone, blur, crop-patch, or API fallback. It is
never imported into runtime UI.

Cropping, alpha extraction, resizing, slicing, matte/chroma removal, and contact
sheets are mechanical operations. They may trim, package, or verify an
already-approved generated candidate, but they may not create, repair, blur,
inpaint, smear, recolor, clone, or synthesize production pixels.

Contact sheets are evidence only. Nothing on a contact sheet is imported. If a
contact sheet shows wrong bounds, text fragments, baked glyphs, title fragments,
local discontinuities, pillow centers, masks, or smudged content zones, that is
a failed gate and the candidate must be regenerated or replaced.

### 2.1.1 Composite Primitive Completeness

A FriendslopStyle composite primitive, such as a modal, dialog, confirmation,
tooltip, popover, drawer, or overlay, is not complete just because its largest
shell plate was generated. Every visible chrome subcomponent rendered by that
primitive is part of the primitive contract.

For a composite primitive, the active pass must list and classify every visible
chrome subcomponent, including:

- outer shell or panel plate;
- title plate or header plate;
- body/content well plate;
- each button chrome family used by the primitive, including red, green, dark,
  disabled, selected, hover, and pressed states when those states can render;
- tooltip pointer/notch, row separators, value chips, icon wells, or other
  chrome sub-surfaces visible in the primitive.

Each listed chrome subcomponent must have one of these statuses:

- `GENERATED`: authored by a current account-backed local Codex CLI imagegen
  worker for this primitive pass, with request, logs, output path, hash, and
  final status recorded.
- `APPROVED_REUSE`: reused from another FriendslopStyle pass through an
  explicit user-approved exception documented in the active screen or primitive
  contract. The record must name the exact reused source asset, source pass,
  runtime path, and reason reuse is acceptable.
- `MISSING`: no valid generated asset or approved reuse exists.

`MISSING` is a blocking failure. Reusing chrome generated for another screen,
borrowing a generic button, using a fallback Slate brush, recoloring a plate,
or procedurally composing a button from non-generated shapes is process-invalid
unless the active contract records an explicit user-approved `APPROVED_REUSE`
exception. A pass may not report a composite primitive as `FULL`, complete, or
ready for visual review while any visible chrome subcomponent is `MISSING`.

The anti-lookalike discriminator for this rule is: a generated textless shell
plus borrowed or manually/procedurally assembled buttons is not one generated
modal primitive. The shell and every rendered button chrome/state must each be
generated for the primitive or explicitly approved for reuse.

Color and state variants are visual chrome, not layout metadata. A red, green,
dark, disabled, selected, hover, or pressed button cannot be produced by local
drawing, tinting, recoloring, CSS/Slate-style fills, vector construction,
Pillow/OpenCV/skimage generation, or a fallback brush just because the shape is
simple or already known. If that variant is visible in the primitive, the pass
must either use a generated runtime PNG for that exact variant family or record a
user-approved `APPROVED_REUSE` exception before implementation. Slate may layer
live text, icons, hit targets, and enabled/disabled behavior over that chrome,
but it may not author the chrome pixels.

## 2.2 Codex CLI Imagegen Worker Contract

Every FriendslopStyle generation iteration that creates or replaces visual
assets must use a fresh local Codex CLI worker such as `codex exec`, using the
account-backed built-in imagegen capability exposed to that worker. This is the
approved non-API route. It is not an OpenAI API fallback and must not require
`OPENAI_API_KEY`.

Do not call imagegen directly in the main Codex app chat for screen iteration
assets. The main chat may coordinate, inspect, package, compare, and integrate
outputs, but generation work belongs in isolated CLI workers so prompts, start
times, logs, outputs, and token usage are auditable.

Each worker needs a durable record:

- worker request/prompt markdown with the exact input contract;
- worker start time or run id;
- attached reference images or source paths used as visual context;
- stdout/stderr logs or equivalent transcript;
- `last_message.txt` or equivalent final status;
- output PNG path copied from that worker's own newly generated image;
- SHA-256 or comparable identity hash when practical;
- token line or worker token count when exposed;
- process result such as `IMAGE_SAVED` or `IMAGE_FAILED`.

### 2.2.1 Runtime Worker Prompt Language Rule

Runtime chrome and family-element worker prompts must be extraction-first and
must not use descriptive/adjectival style language. The supplied approved
reference, generated textless reference, or textless family crop is the only
visual style authority.

Allowed prompt content:

- input image paths and their role;
- family and element names from the manifest;
- output filenames, canvas/alpha requirements, runtime dimensions, and
  contact-sheet requirements;
- live-content prohibitions such as no baked text, labels, icons, names,
  scores, row data, or localized strings;
- mechanical verbs such as extract, separate, individualize, reproduce from the
  input image, and save.

Forbidden prompt content:

- positive visual adjectives or style descriptors for material, shape, color,
  theme, mood, vibe, polish, or game comparisons;
- verbal attempts to correct appearance, such as telling the worker to make an
  element more rubbery, glossy, shiny, inflated, pill-like, rounded, squared,
  beveled, 3D, dark, chrome, soft, thick, thin, chunky, sleek, cute, polished,
  Fall Guys-like, or similar;
- describing desired colors or material qualities unless the words are already
  part of a required element id or output filename.

If an element cannot be identified from the supplied crop without descriptive
language, stop and get a better crop, reference, or user decision. Do not
improvise with words. Before launching a worker, scan `request.md` for
forbidden descriptive language. Outputs produced from a noncompliant prompt are
process-invalid unless the user explicitly accepts that exception, and must not
be implemented as normal FriendslopStyle assets.

Textless-reference workers may name removal targets and output constraints, but
must not add verbal restyling instructions. The image carries the look.

### 2.2.2 Working Codex CLI Invocation Order

Use this PowerShell-safe command shape for account-backed built-in imagegen
workers. The global approval flag belongs before the `exec` subcommand; putting
it after `exec` can fail before generation with an argument-order error such as
`unexpected argument '-a'`.

```powershell
$WorkerDir = "C:\UE\T66\Saved\Codex\UI\FriendslopStyle\<Screen>\<pass>_workers\<asset_name>"
$RequestPath = "$WorkerDir\request.md"
$ReferencePath = "C:\UE\T66\UI\FriendslopStyle\Reference\<Screen>\<crop_or_context>.png"
$LastMessagePath = "$WorkerDir\last_message.txt"
$StdoutPath = "$WorkerDir\stdout.jsonl"
$StderrPath = "$WorkerDir\stderr.txt"

New-Item -ItemType Directory -Force -Path $WorkerDir | Out-Null
Get-Date -Format o | Set-Content -LiteralPath "$WorkerDir\start_time.txt" -Encoding UTF8

Get-Content -Raw -LiteralPath $RequestPath |
  codex --ask-for-approval never exec `
    -C "C:\UE\T66" `
    --sandbox danger-full-access `
    --json `
    --output-last-message $LastMessagePath `
    --image $ReferencePath `
    - > $StdoutPath 2> $StderrPath

$LASTEXITCODE | Set-Content -LiteralPath "$WorkerDir\exit_code.txt" -Encoding UTF8
```

Required ordering:

- `--ask-for-approval never` is a global Codex CLI option and must appear
  before `exec`.
- `-C`, `--sandbox`, `--json`, `--output-last-message`, and `--image` are part
  of the `exec` invocation.
- The final `-` is the prompt source and means "read prompt from stdin"; keep it
  after all options.
- Use `Get-Content -Raw ... | codex ... -` rather than shell input
  redirection, so the command works in PowerShell.
- Capture stdout/stderr and the exit code even for failed launches. A syntax
  failure is still a worker record, but it is not a generation attempt.

Worker prompts must ban copying old generated images, searching old
`C:\Users\DoPra\.codex\generated_images` folders as the answer, web image
downloads, browser screenshots, OpenAI API scripts, and any manual pixel repair.
The worker may copy only the PNG produced by its own generation call after the
worker started.

If the built-in imagegen call returns `TooManyRequests`, session/auth-like bad
requests, or another transient worker-session failure, treat it as transient.
Restart or fork into a fresh Codex CLI worker. Do not continue by approximating
the asset manually, using cached candidates, using API scripts, or salvaging a
failed asset with pixel editing.

## 3. What To Reuse From FlatStyle

Reuse only the process spine:

- Reference geometry extraction from the approved image.
- Geometry overlay generated by `Scripts\GenerateUIGeometryOverlay.py`.
- Widget tagging and structural dumps through `T66.UI.DumpScreen` or
  `T66.UI.DumpWidget`.
- Optional structural/wiring checks through `Scripts\VerifyUIFidelity.py`.
  FriendslopStyle does not use that script as the final visual gate.
- The checklist section model: Structure, Geometry, Colors, Content,
  Interactivity.
- Pass logs, stuck-failure escalation, accepted-delta tracking, and contact
  sheets.
- Responsive sizing rules from `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.

Do not reuse these as the FriendslopStyle target:

- Flat rectangular Slate-native chrome.
- `FT66FlatStyle` as the visual style target.
- The FlatStyle Step 0 goal of deleting every raster chrome path.
- Any old reference/chrome image-generation pipeline that produced baked,
  distorted, full-screen UI art.

FriendslopStyle may reuse proven low-level Slate concepts, but it should own a
parallel namespace such as `FT66FriendslopStyle` and Friendslop-specific asset
registries instead of reviving legacy `MakeReference*` chrome as the public
style API.

## 4. Required Per-Screen Artifacts

Each FriendslopStyle screen needs these artifacts before it can be called
implemented:

- Visual reference: `UI/FriendslopStyle/Reference/<Screen>/Current/...png`
- Reference manifest: `UI/FriendslopStyle/Reference/<Screen>/Current/manifest.md`
- Screen folder: `UI/FriendslopStyle/Screens/<Screen>/`. If it does not exist,
  create it before any generation or implementation work. Use
  `Screens/MainMenu/` as the folder-shape example only.
- User-provided visual family breakdown recorded in the screen folder. Family
  count is screen-specific and drives the worker queue: one local Codex CLI
  worker per visual `FAIL` family that requires generation.
- Textless reference breakdown for the approved reference, generated by a
  fresh local Codex CLI worker and cached with one crop per user-declared
  visual family. Main Menu's pilot crops are `TopBar`, `LeftSocialPanel`,
  `RightLeaderboardPanel`, `CenterButtonStack`, and `Background`.
- Fresh current capture: `Saved/Codex/UI/FriendslopStyle/<Screen>/...png`
- Fresh current dump: `Saved/Codex/UI/FriendslopStyle/<Screen>/...json`
- Screen router: `UI/FriendslopStyle/Screens/<Screen>/README.md`
- Geometry table: `UI/FriendslopStyle/Screens/<Screen>/geometry.md`
- Geometry overlay: `UI/FriendslopStyle/Screens/<Screen>/geometry_overlay.png`
- Element manifest: `UI/FriendslopStyle/Screens/<Screen>/element_manifest.md`
- Per-pass visual family ledger inside the element manifest or linked from it:
  each screen is first evaluated as its user-declared set of visual families.
  Main Menu is the worked example with five families: `TopBar`,
  `LeftSocialPanel`, `RightLeaderboardPanel`, `CenterButtonStack`, and
  `Background`.
- Per-failed-family element ledger: every element inside a visually failed
  family is then evaluated as visual `PASS` or visual `FAIL`.
- Slice specification: `UI/FriendslopStyle/Screens/<Screen>/slice_specs.md`
- Verification checklist: `UI/FriendslopStyle/Screens/<Screen>/checklist.md`
- Current component contract: `UI/FriendslopStyle/Screens/<Screen>/component_contract_current.md`
- Asset registry: `UI/FriendslopStyle/friendslop_asset_registry.md`
- Imagegen worker records for every generated or regenerated visual asset:
  prompt/request, logs, final message, output path, and token/hash data when
  exposed
- Pass log: `Saved/Codex/UI/FriendslopStyle/<Screen>/pass_log.md`
- Final capture, dump, contact sheet or side-by-side sheet, worker records,
  changed file list, sizing/fitting notes, and wiring/functionality gate report.
  A `VerifyUIFidelity.py` report may be included as optional technical evidence
  when useful, but it is not required for Friendslop completion.

If an existing FlatStyle checklist is stale, do not patch around it. Author a
FriendslopStyle checklist from the new geometry table and fresh live dump.

## 5. PPF Gate Before Implementation

Before coding or importing assets for any screen, write this gate in the pass
log:

```text
PPF CHECK
Objective:
Proven process: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md plus UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md Step 0.5 through verification loop.
My planned implementation:
Same method class: YES/NO
If NO, why:
User approval required before proceeding: YES/NO
Verification evidence:
```

If `Same method class` is `NO`, stop and ask the user. Do not prototype another
method in parallel.

## 6. Artifact Parity Gate

Each screen must list required artifacts before implementation:

```text
ARTIFACT PARITY GATE
Reference artifact/category:
Role: Primary/Secondary
Required: YES/NO
Planned artifact/path:
Status: SAME/EQUIVALENT/MISSING/DEFERRED
Evidence:
```

Required primary artifacts for a normal FriendslopStyle screen are:

- Approved full-screen visual reference.
- Fresh live capture and dump for current content authority.
- Geometry table and overlay.
- Authored transparent PNG plates or plate families for panels, buttons, tabs,
  dropdowns, rows, counters, progress tracks, and other visual surfaces used by
  the reference. A generic reusable atom is acceptable only when it passes the
  reference-specific visual gate at the runtime size where it is used.
- Live Slate text/icon/data layers over those surfaces.
- Wiring/functionality checklist plus visual evidence packet for user review.

The approved full-screen reference is not equivalent to reusable runtime chrome.
It is a visual target only.

## 7. Mechanism Manifest

Before implementation, list the visual and runtime mechanisms that make the
reference work:

```text
MECHANISM MANIFEST
Reference/source:
Required mechanisms:
  1. Mechanism:
     Required: YES/NO
     Planned implementation:
     Evidence needed:
```

For Main Menu Round06, required mechanisms include:

- Layout preservation: top bar, left social/friends panel, center CTA stack,
  right leaderboard, ticket/power controls, and current online/offline content.
- Rubber material read: inflated rounded forms, soft bevels, glossy highlights,
  dark body fill, red/green/yellow accent usage, and soft shadow separation.
- Slice preservation: corners and edge caps do not stretch or smear when button
  widths differ.
- Live content: text, friend names, counts, scores, party state, dropdowns, and
  leaderboard rows are Slate content, not baked into the texture.
- State behavior: normal, hover, pressed, selected, disabled, online, offline,
  and focus states keep the same material family.
- Responsive behavior: 1920x1080 drives composition, but runtime layout adapts
  to required target resolutions.

If a required mechanism has no evidence path, the pass is not ready to start.

## 8. ImageGen Rules

ImageGen has two different jobs.

All FriendslopStyle image generation for references, runtime chrome, title art,
background art, content stubs, and corrected component assets must be performed
by a separate local Codex CLI worker using account-backed built-in imagegen,
following Section 2.2. The main Codex app chat coordinates and reviews the work;
it does not perform generation for an iteration pass. API scripts,
`OPENAI_API_KEY`, old generated-image folders, web image search, and browser
screenshots are not valid substitutes.

Full-screen reference generation:

- Used for vision, art direction, and user approval.
- May include full layout and approximate text for communicating the idea.
- Must be based on fresh current capture/dump when preserving current content.
- Must be archived with prompt, worker record, manifest, and visual QA.
- Must not be imported as runtime UI chrome.

Runtime chrome generation:

- Create blank standalone plates, not full screens.
- For each approved reference, first create the required textless reference
  breakdown. Future iterations for the same reference reuse that cached
  textless breakdown instead of re-describing the visual style in words.
- Runtime family worker prompts must follow Section 2.2.1: no descriptive or
  adjectival style language. The prompt may identify the family, list elements,
  attach the textless crop, and request contact-sheet plus individualized PNG
  outputs, but it may not describe the desired material, shape, color, or vibe
  in words.
- Generate one component family at a time from the textless family crop:
  panel shell, primary button, secondary button, tab, dropdown shell, row strip,
  counter pill, progress track, party slot, icon button, scrollbar, divider.
- Each family worker must output both a family contact sheet and individualized
  backgroundless PNG files for each requested element. Runtime implementation
  uses the individualized files, not manual crops from the family sheet.
- For high-visibility or size-sensitive regions, create per-element or
  per-size-family plates that match the approved reference directly instead of
  stretching a generic atom.
- Do not include labels, names, scores, fake friend rows, fake currencies, or
  decorative text.
- Use a clean front-on orthographic view.
- Runtime input must be an alpha-clean transparent PNG. If the generator cannot
  produce reliable alpha, use a flat removable matte only as an intermediate
  cleanup step and remove it before import.
- Before copying an individualized PNG into runtime, run an alpha-bounds sanity
  check against the expected visible extent for that asset. A correct canvas
  size with the actual plate sitting in a narrow transparent strip is a failed
  worker output and must be regenerated by a bounded retry worker; do not
  manually crop it to make it fit.
- Keep corners and edge caps visually detailed. Keep the center stretch band
  low-detail and seam-safe.
- Keep lighting direction, bevel thickness, shadow style, and highlight shape
  consistent across states.
- Produce required states as a matched family: normal, hover, pressed, selected,
  disabled, and any semantic states needed by the screen.
- Save prompts and outputs under `UI/FriendslopStyle/SourcePrompts/<Screen>/`
  and source PNGs under `SourceAssets/UI/FriendslopStyle/<Screen>/`.
- Record the CLI worker run that created each accepted candidate. A runtime
  asset without an auditable worker record is not production-proven unless the
  active screen contract documents a user-approved exception.

Every runtime chrome set needs a local contact sheet before Unreal import. The
contact sheet must show all individualized outputs, all states at source size and
at expected min/normal/wide runtime sizes, and the corresponding textless family
crop or reference measurement crop. A plate that merely shares the same semantic
category, such as "red rubber button," fails this gate if it does not match the
approved reference's silhouette, scale, material density, and edge treatment.

## 9. Slice Rules

For 9-slice assets, Unreal should render with an `FSlateBrush` using
`DrawAs = ESlateBrushDrawType::Box` and nonzero `FMargin` values. The margins are
normalized fractions of the source texture width and height. Corners stay fixed,
edges stretch in one axis, and the center stretches in both axes.

For 3-slice or horizontal-slice assets, the renderer preserves the left and
right caps and stretches the center band. Use this only for button-like surfaces
whose height is stable and whose width changes.

Slice specs must record:

- Asset path.
- Source image dimensions.
- Intended runtime min, normal, and max size.
- Slice mode: `9-slice` or `horizontal-3-slice`.
- Margins as normalized fractions.
- Protected cap pixel sizes.
- Center stretch region.
- Content padding.
- Supported states.
- Seam/stretch test result.

Do not trust a slice margin because it "looks plausible." Test the asset at
minimum, normal, and wide sizes. If highlights, outlines, dents, stitches, or
shadows smear across the center because the visual pixels are wrong, regenerate
or replace the asset. If the visual pixels are correct but packaging is wrong,
recrop the generated candidate, alpha-clean, adjust slice margins, or switch to
a size-specific plate.

Sizing and positioning success does not prove slice integrity. Stable-height
button plates must pass a center-seam and pillow check at the declared minimum,
normal, and wide sizes. Fail the slice when protected caps collide, the center
band collapses, highlights or shadows form a visible center line, or the center
balloons like a pillow even if live labels and layout fit correctly. Record the
source cap pixels and the runtime cap budget in the slice spec. If the protected
cap pixels consume too much of the target width or height, use a
horizontal-3-slice setup, lower margins, or a size-specific generated plate
through the imagegen worker process. Do not dismiss the artifact as an
acceptable 9-slice/aspect mismatch, and do not repair it by manual repainting,
masking, or cropping.

## 10. Unreal Implementation Rules

The implementation pass should add a Friendslop-specific runtime style layer,
not expand the old reference chrome API as the screen-facing contract.

Expected code direction:

- Add `FT66FriendslopStyle` or equivalent under `Source/T66/UI/Style/`.
- Add helpers for Friendslop panels, buttons, tabs, rows, dropdowns, counters,
  and progress/track surfaces.
- Load raster chrome through the existing runtime brush access pattern or an
  explicit Friendslop asset registry.
- Use `FSlateBrush` `Box` draw type for 9-slice surfaces.
- Use a Friendslop horizontal-slice widget for stable-height variable-width
  buttons where 3-slice is the correct asset class.
- Use fixed-image or size-specific plates when slicing would degrade the
  authored material. Do not force all high-quality chrome through one generic
  resizable brush.
- Keep child content live with `STextBlock`, existing icon widgets, and live data
  bindings.
- Attach dump metadata/tags to every named region and control.
- Preserve click handlers, hover capability, selected state, disabled state,
  dropdown behavior, and current screen routing.

Text must never be part of a button or panel texture. If generated chrome
contains accidental text or text-like marks, reject it or clean/regenerate it
before import.

## 11. Per-Screen Pipeline

### Step A: Fresh Current Baseline

Capture the current live screen and dump:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen <ScreenName> -Output <capture.png> -DelaySeconds 6 -ExtraArgs @("-T66AutoDumpScreen=<dump.json>")
```

Use this to confirm current content, removed/stale elements, and required tags.

### Step B: Reference Reconciliation

Compare the approved Friendslop reference against the fresh capture/dump.
Record:

- Current content that must remain.
- Deliberate visual deltas from the live screen.
- Deliberate naming/title deltas approved by the user.
- Generated-text errors in the reference that must not be implemented.
- Any user-only art-direction decisions.

If the approved reference does not yet have a current textless breakdown, create
it now through one fresh Codex CLI worker. The worker receives the approved
reference and outputs a clean no-text/no-data full reference that preserves the
family silhouettes and material. Mechanically crop that generated textless
reference into the user-declared family contexts and record the paths. If the
approved reference already has a textless breakdown, verify the manifest points
to the current reference hash and reuse it.

### Step C: Geometry Extraction

Measure the approved reference at native resolution and normalize every element
to the 1920x1080 verifier basis. Save:

`UI/FriendslopStyle/Screens/<Screen>/geometry.md`

Then create an overlay:

```powershell
python Scripts\GenerateUIGeometryOverlay.py --geometry C:\UE\T66\UI\FriendslopStyle\Screens\<Screen>\geometry.md --output C:\UE\T66\UI\FriendslopStyle\Screens\<Screen>\geometry_overlay.png
```

Visually inspect the overlay before code. Boxes must include intended breathing
room and perceived region boundaries, not only tight visible pixels.

### Step D: Element Decomposition

Create:

`UI/FriendslopStyle/Screens/<Screen>/element_manifest.md`

The screen manifest is the complete inventory of visual and interactive
elements. It is not a one-time planning note. Each implementation pass must
update every row or a linked per-pass ledger for every row.

Each visual family maps:

- Reference region/tag.
- Owned elements.
- Runtime widget owners.
- Live content sources.
- Chrome/background/title asset families.
- Reference measurement crop or comparison target.
- Visual verdict: `PASS` or `FAIL`.
- Evidence path.
- Next action.

This is where the full-screen reference becomes implementable parts.

For generated-raster FriendslopStyle, decomposition uses the approved reference
plus its textless family crops. The family crop is the visual input to the
family worker. Prompts must be reference-first and extraction-oriented:
identify the family and requested elements, require a contact sheet plus
individual transparent PNGs, and forbid baked text/data/icons unless the
element is itself an approved icon. Do not use descriptive/adjectival style
language in the worker prompt. Let the supplied image carry the style.

Visual vocabulary:

- `PASS`: the visual family or element looks close enough to the approved
  reference that it does not need imagegen regeneration in this iteration.
- `FAIL`: the visual family or element does not match the approved reference and
  must be regenerated in this iteration.

Use `FAIL` for wrong rubber material, wrong silhouette, wrong bevel/gloss,
wrong shadow, wrong title/background style, wrong blank plate, baked
text/icons/data, smears, masks, pillow centers, wrong row fill, or any other
pixel-authorship mismatch. Be strict. If there is doubt about visual fidelity,
mark `FAIL`.

Worked example, Main Menu visual family order:

1. `TopBar`: topbar strip, settings/language/power icon buttons, account/home/
   power-up/achievements tabs, and ticket badge.
2. `LeftSocialPanel`: left panel shell, profile row, search field,
   online/offline headers, friend rows, invite/offline buttons, party panel, and
   party slots.
3. `RightLeaderboardPanel`: side filter rail, leaderboard shell, filter tabs,
   dropdowns, metric controls, headers, rows, and dividers.
4. `CenterButtonStack`: title, subtitle treatment, primary CTA, secondary CTA,
   and CTA icons/plates.
5. `Background`: star/fire/golden statue scene and any non-UI background art.

No implementation pass may omit a visual family, leave a family unclassified, or
work only on the most recently discussed component. If a family is `FAIL`, the
same iteration must classify the elements inside that family and launch one
Codex CLI worker for that family unless a hard blocker is documented.

### Step E: Runtime Chrome Asset Authoring

Generate blank component surfaces after the visual assessment. For each failed
visual family:

- Launch exactly one fresh Codex CLI worker for that family and record the
  worker artifacts required by Section 2.2.
- The worker prompt must include the failed family name, all failed elements in
  that family, textless family crop paths, no-baked-text rules, and required
  output sheet/individual asset paths.
- The worker prompt must pass the Section 2.2.1 prompt-language rule: no
  descriptive/adjectival style language, no verbal shape/material/color fixes,
  and no game/vibe comparisons. If the request cannot be written without those
  words, stop and improve the reference crop or ask the user.
- Generate one contact sheet containing all requested failed elements for that
  family matching only the supplied textless family crop, plus one
  individualized backgroundless PNG per requested runtime element.
- `Background` is the only Main Menu exception to individualized transparent
  PNG output: it produces a full-scene background plate, plus contact evidence.
- Reject candidates with baked labels/data.
- Remove matte/chroma and confirm alpha only on a blank generated candidate when
  the worker output uses a removable matte. Do not manually crop contact sheets
  to create runtime assets.
- Use reference crops only as measurement and comparison targets.
- Compare the generated plate against the matching reference measurement crop.
- Determine slice margins.
- Build a contact sheet at min/normal/wide sizes.
- Record each accepted generated asset in the registry and slice spec.

Do not use crop, alpha extraction, inpaint, blur, clone, local painting, or
procedural image synthesis to remove text/icons/data from a full-screen
reference or from a bad candidate. If the asset pixels are wrong, regenerate or
replace the asset.

Before scaling to a full screen asset set, import or load one representative
Friendslop 9-slice brush and prove it renders correctly at runtime in a tiny
smoke surface. This catches texture import, fallback, alpha, filtering, and
`DrawAs=Box` issues before the pass spends time authoring every family.

Do not import the full family until the slice/contact-sheet gate passes and the
first runtime brush smoke test has current evidence. Once a family generation is
accepted, implement every generated element from that family onto the screen
before moving to layout correction. Do not finish an iteration with generated
family assets sitting only in worker folders.

### Step F: Generated Asset Implementation

Implement every newly generated family asset onto the screen using
FriendslopStyle helpers and the geometry table. Tag all named regions. Keep all
content live. Preserve current control behavior.

### Step F.1: Sizing And Fitting Correction

After all failed visual families have been regenerated once and their generated
elements are implemented, run a sizing/fitting pass across the same declared
families.

This pass corrects obvious runtime assembly issues: sizing, placement,
containment, padding, slicing, density, and live text/icon placement. Record
what changed and any hard blocker. This pass uses a technical
`SIZING/POSITIONING PASS` or `SIZING/POSITIONING FAIL`; it does not declare the
final user-owned visual layout match. The user still reviews the produced
capture/contact sheet for final visual approval.

Sizing/fitting problems are corrected directly in Slate/layout/packaging and
recaptured when needed. Do not regenerate pixels for a sizing/fitting problem in
the same iteration.

For modals, dialogs, tooltips, popovers, drawers, and other composite
primitives, the sizing/fitting pass must include a primitive fit gate. The gate
uses the current widget dump, current capture PNG, and the primitive's slice
spec. The gate must write a short report with the checked values and a final
binary result:

```text
PRIMITIVE FIT GATE
Primitive:
Capture:
Dump:
Slice spec:
Result: SIZING/POSITIONING PASS | SIZING/POSITIONING FAIL
Failures:
```

Minimum required checks:

- `Chrome coverage`: every visible chrome subcomponent from Section 2.1.1 is
  `GENERATED` or `APPROVED_REUSE`; no `MISSING` subcomponent renders.
- `Texture load and tint`: the generated plate is loaded, not the fallback
  brush, and the loaded brush is not multiplied by fallback tint. Fail if the
  shell appears materially darker than the source/runtime asset or if the
  loaded brush keeps a non-white tint unless explicitly specified. For textured
  chrome, record at least one opaque `TintSampleRect` from the source asset and
  the matching capture area; fail the gate when the captured mean or median
  luminance is darker than the source by more than the pass tolerance, or by
  more than 10% when no tighter tolerance is declared.
- `Slice integrity`: render each sliced chrome surface at the actual runtime
  size plus the min, normal, and wide sizes from the slice spec. Fail on visible
  center seams, protected-cap collision, collapsed center bands, pillow bulges,
  stretched highlights or shadows, or any generated plate that looks cut in
  half. This is separate from content fit; a layout pass cannot override a
  slice-integrity failure.
- `Containment`: every live title, body, row, value, icon, and button rect is
  inside the primitive content rect. Use explicit insets from the slice spec.
- `Minimum padding`: live content keeps the declared minimum distance from
  chrome edges, pointer notches, button edges, and sibling controls.
- `No overlap`: interactive controls and text/value rows do not intersect
  unless the overlap is a declared visual stack in the slice spec.
- `Centering/alignment`: title, body block, tooltip row group, and button row
  centroids are within declared tolerances of their intended axes.
- `Text fit`: live text is readable, intentionally wrapped, scaled down, or
  intentionally ellipsized. Fail if important text clips, touches chrome, runs
  under the notch, or relies on baked image text.
- `State fit`: hover, pressed, selected, disabled, ready, and default states
  keep the same containment and padding guarantees when those states exist.

The primitive fit gate must be rerun after every sizing fix. A failed gate must
produce concrete correction knobs, not generic visual commentary. Use this order:

1. Recalculate content rects from `OuterSize`, `ChromeInsets`, and named gaps.
2. Validate slice mode, margins, source cap pixels, and runtime cap budget
   before changing live text. For stable-height buttons, switch to
   horizontal-3-slice, reduce margins, or use a size-specific generated plate
   before accepting a `Box` seam or pillow center.
3. Reduce child width, gap, or font within the minimum readable bounds declared
   in the slice spec.
4. Switch the content group to a compact layout or scroll area when the
   primitive contract allows it.
5. Increase or create a size-specific generated plate only when the content
   cannot fit within declared minimums. A larger or alternate plate is a new
   visual asset pass and must follow the imagegen worker process.
6. Recapture, redump, and rerun the gate.

If two consecutive attempts produce the same `SIZING/POSITIONING FAIL` set,
stop the loop and create a user review packet with the unchanged fail list,
capture, dump, and proposed next knob. Do not keep making subjective tweaks.

### Step F.2: Wiring Correction

After the sizing/fitting pass, run a wiring/functionality-only check across the
same declared families. For wiring/functionality, use only `PASS` or `FAIL`:

- Wiring/functionality `PASS`: imports, live ownership, handlers,
  hover/pressed/selected/disabled states, dropdowns/toggles, text/data bindings,
  and dump metadata are correct.
- Wiring/functionality `FAIL`: any of those technical properties are wrong.

Wiring/functionality failures are corrected directly and recaptured/dumped until
they pass or a hard blocker is documented. Wiring/functionality success never
substitutes for visual approval by the user.

### Step G: Compile

Run focused compile for the affected UI code. Do not proceed to capture with a
known compile failure.

### Step H: Capture, Dump, And Evidence

Capture the screen through Unreal-owned capture tooling and produce a current
dump. Create the side-by-side/contact evidence needed for user visual review.
When useful for technical checks, `VerifyUIFidelity.py` may be run as an
optional structural/wiring helper:

```powershell
python Scripts\VerifyUIFidelity.py --checklist <friendslop_checklist.md> --dump <dump.json> --reference <reference.png> --capture <capture.png> --output <report.md> --contact-sheet <contact_sheet.png>
```

The exact arguments may follow the current script contract. Use the current
script help if it changes. Do not report the script's numeric PASS/FAIL count as
Friendslop visual success.

The pass report must summarize the visual family and element ledgers:

- all declared visual families evaluated;
- visual family `PASS` count and visual family `FAIL` count;
- a family-by-family element breakdown where each family starts with a bold
  heading such as `**TopBar family - FAIL**`, followed by a two-column table
  with headers `Element` and `Visual PASS/FAIL`;
- for every failed family, per-element visual `PASS` and `FAIL` counts;
- one Codex CLI worker record for every failed visual family;
- generated family assets and their implementation paths;
- sizing/fitting work performed after implementation;
- wiring/functionality `PASS`/`FAIL` result for all declared families after
  sizing/fitting.
- a `reference_vs_current` comparison sheet and a `previous_vs_current`
  comparison sheet, both saved with the pass artifacts and shown in the final
  user-facing response.

Do not put worker instructions, reasons, or notes inside the two-column element
tables. Put those details in the worker queue, notes, or implementation
sections so the element coverage table remains easy to audit.

### Step I: Ordered Correction Loop

Run the correction loop in this order:

1. **Visual assessment.** Check the screen's declared visual families. Mark
   each family visual `PASS` or visual `FAIL`.
2. **Element breakdown.** For every visual `FAIL` family, check all elements
   inside that family. Mark each element visual `PASS` or visual `FAIL`.
3. **Generation.** Launch one local Codex CLI worker per visual `FAIL` family.
   Each worker receives the cached textless family crop and generates a contact
   sheet plus individualized backgroundless PNGs for all visual `FAIL` elements
   in that family. Do not launch only one representative worker when several
   families failed. Do not use manual sheet cropping as the runtime asset path.
   Do not use descriptive/adjectival style language in the worker prompt; the
   textless crop is the only visual style instruction.
4. **Implementation.** Put every regenerated family element onto the screen.
   Update source/runtime/staged runtime paths and records.
5. **Sizing/fitting pass.** Correct sizing, placement, containment, slicing,
   density, and live text/icon placement issues that are objectively actionable
   in Slate/layout/packaging. Record what changed and any hard blocker.
6. **Wiring/functionality pass.** Check wiring/functionality `PASS`/`FAIL` for
   the declared families. Correct wiring/functionality failures until they pass
   or a hard blocker is documented.
7. **Capture/evidence/report.** Capture, dump, create the contact/side-by-side
   evidence, report family/element coverage, report implemented assets, report
   sizing/fitting work, and report the wiring/functionality gate. Do not report
   a Codex-owned visual `FULL`, `PARTIAL`, or visual-scorecard `Result`.

Visual generation is one batch per failed family per iteration. Do not re-check
and regenerate visual images repeatedly in the same iteration. If the generated
family sheet is imperfect, implement the generated assets, report the remaining
visual result honestly, and let the next user-directed iteration decide the next
visual batch.

Stop and make a user review packet when:

- A required primary artifact is missing or must be substituted.
- A failed visual family cannot be sent to a CLI worker.
- A worker fails repeatedly with the same non-transient issue.
- Generated assets cannot be implemented onto the screen.
- A sizing/fitting blocker or wiring/functionality failure cannot be corrected
  internally.
- A visual decision depends on user taste rather than measurable repo evidence.

### Step J: User Visual Review Evidence

This step produces the visual evidence the user reviews. It is not a Codex-owned
PASS/FAIL or FULL/PARTIAL gate.

Create a full-screen side-by-side/contact sheet with:

- Approved reference.
- Current capture.
- Previous iteration capture.
- Geometry overlay.
- Notes for accepted deltas.

At minimum, create two separate comparison sheets: `reference_vs_current` and
`previous_vs_current`. The first shows distance from the approved target. The
second shows whether the newest pass regressed any user-preferred prior assets.

Then create a visual review markdown file with category rows for:

- declared-family visual ledger completeness
- declared visual-family assessment completeness
- failed-family worker coverage
- regenerated asset implementation completeness
- sizing/fitting work performed
- wiring/functionality gate result
- first-glance match
- overall silhouette and region weight
- panel/button scale hierarchy
- rubber material fidelity
- authored plate quality
- spacing rhythm and density
- overflow/clipping
- live content preservation
- reference-specific accepted deltas

Codex and Claude may include prose observations about:

- Overall silhouette and region weight.
- Rubber inflation and material consistency.
- Spacing rhythm and breathing room.
- Visual hierarchy.
- Button/panel scale relationships.
- Whether the screen still reads like the approved reference at a glance.

Do not mark those visual observations as final PASS/FAIL unless the user
explicitly asks for that judgment in the turn. The user decides whether the
visual element fidelity and layout match are acceptable for the next iteration.

### Step K: Responsive Gate

Use the layout instructions. At minimum, verify the screen at the required
desktop target set when practical:

- 1920x1080
- 1600x900
- 1366x768
- 1280x720
- 2560x1440
- 3440x1440

Important text must remain readable or intentionally ellipsized. Controls must
not overlap, collapse, or resize unpredictably.

### Step L: Manual Interaction Gate

As part of the wiring/functionality gate, run the manual or automated
interaction checks that are available:

- Buttons click and route correctly.
- Hover/pressed/selected states change and return correctly.
- Dropdowns open and select correctly.
- Party/friend/leaderboard rows keep live data behavior.
- Disabled controls do not claim hover/click behavior.

Any failed interaction makes the wiring/functionality gate `FAIL` until it is
corrected or the blocker is documented.

## 12. Main Menu Pilot Notes

The Main Menu pilot must start from the current Round06 reference and the fresh
current baseline named in the current manifest.

Round06 source resolution is `1672x941`. Geometry extraction must measure that
native image and then normalize boxes to the 1920x1080 verifier basis. Do not
pretend the source PNG is already 1920x1080.

Known current Main Menu checklist problem:

- `UI/Checklists/main_menu_checklist.md` is stale.
- `UI/Checklists/pending_issues_Checklists.md` says it still expects removed
  controls and must be refreshed to current controls.
- The active Friendslop checklist is
  `UI/FriendslopStyle/Screens/MainMenu/checklist.md`; do not treat the stale
  FlatStyle checklist as valid evidence.

Main Menu implementation is ready to start only after:

- User approves this file.
- The UI router conflict around Friendslop raster chrome is resolved.
- Round06 is confirmed as the pilot reference.
- PPF, artifact parity, and mechanism manifests are written for Main Menu.

## 13. Completion Close

At completion of a FriendslopStyle implementation pass, report:

```text
PPF CLOSE
Process used:
Matches declared process: YES/NO
Evidence:
```

For each required mechanism:

```text
MECHANISM CLOSE
Mechanism:
Status: PRESENT/ABSENT/DEFERRED
Evidence:
Discriminator test:
Notes:
```

Do not report a FriendslopStyle implementation pass as `FULL` or `PARTIAL`.
Report objective process coverage instead:

- declared-family assessment completed: YES/NO
- failed-family element breakdown completed: YES/NO
- one worker launched per failed visual family: YES/NO
- generated assets implemented: YES/NO
- sizing/fitting work performed and blockers, if any
- wiring/functionality gate: PASS/FAIL
- responsive evidence: path or skipped reason
- manual/interaction evidence: path or skipped reason

The final user-facing status should not claim visual acceptance. Show the
reference and produced capture/contact sheet, summarize what changed, and let
the user decide the next visual direction.

