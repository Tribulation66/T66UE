# Screen Review Gate

Use this gate before calling a reconstruction pass successful.

## Why This Exists

A screen can look closer in one region while regressing badly in another. This gate exists to stop churn and stop low-signal "improvements" from being accepted.

## Mechanical Checks

Every pass must clear all of these:

- screen-specific generated reference exists at `C:\UE\T66\UI\screens\<screen_slug>\reference\canonical_reference_1920x1080.png`
- that reference was generated from the canonical main-menu anchor, the current target screenshot, and the target layout list
- packaged captures and review artifacts are saved under `C:\UE\T66\UI\screens\<screen_slug>\outputs\YYYY-MM-DD\` or `review\`
- packaged captures are launched through `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1` or equivalent display-1 `-WinX`/`-WinY` flags
- packaged capture is at the canonical target size, normally `1920x1080`
- no `1672x941` or other non-canonical generated output has been promoted as a production reference, sprite sheet, scene plate, slice, or runtime asset
- all generated reference, scene, sprite-family, icon, and state assets came from Codex-native image generation
- no cropped text
- no cropped component edges
- no duplicated shell ownership
- no shell art extending outside its measured frame
- no dark placeholder blocks where a plate or environment strip should be
- no full reference screenshot, buttonless master, or textless master used as a runtime background
- scene/background plate contains no baked top bar, side panels, CTA stack, leaderboard rows, labels, values, buttons, avatars, icons, or live content
- no stretched plate art caused by forcing the wrong aspect ratio into a manifest box
- no top-bar, panel, or CTA region that is simultaneously owned by both the background and live widgets
- no obvious live-control dead zones or fake visible buttons
- shell rects, visible control rects, and live-content rects stay distinct when ownership is mixed
- sliced assets match manifest dimensions or declare a valid scale strategy
- alpha outside plate silhouettes is clean
- nine-slice margins exist for every stretchable plate
- state anchors do not jump between normal, hover, pressed, disabled, or selected variants

## Reconstruction Checks

For a reference-faithful screen:

- reference layout manifest is the placement source of truth
- runtime composition uses a UI-free scene/background plate plus clean foreground component families, not screenshot cleanup hacks
- generated assets have not been manually pixel-edited, cleaned up, masked, erased/filled, cover-patched, cloned, repainted, or screenshot-repaired after generation
- any incorrect generated asset was rejected for regeneration; only deterministic slicing/cropping and runtime overlays were used after generation
- localizable text is not baked into runtime art
- packaged runtime is used for the actual judgment
- static regions are diffed strictly when possible
- live regions are called out explicitly when they were only manually validated
- runtime-owned interiors are masked before strict diffing
- baked display art is intentional; localizable text and runtime values are not baked into runtime plates

## Review Process

1. Confirm the screen-specific generated reference exists and was created from the required three inputs.
2. Capture the packaged screen.
3. Confirm capture resolution matches the locked target.
4. Compare against the approved screen-specific reference at the same composition.
5. Confirm the runtime background layer is a UI-free scene plate.
6. Confirm topbar, side panels, leaderboard chrome, and center CTA stack are separate foreground components.
7. Apply ownership masks before strict diffing.
8. List ownership/layout failures before discussing polish.
9. Fix one region at a time.
10. Roll back immediately if a pass improves one region by breaking another.

## Status Language

Use blunt labels:

- `blocked`
- `close with blockers`
- `exact enough`

Do not call a pass final if it still fails any mechanical check.

Do not call a pass complete after reference generation alone. Completion requires runtime implementation, packaged capture, and packaged review.
