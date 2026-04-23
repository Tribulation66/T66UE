# UI Content Ownership Audit

## Purpose

This document defines the required ownership audit that must happen before style-reference generation, family-board generation, or packaged diffing.

The core rule is simple:

- do not paint over runtime-owned content

Many T66 screens combine reconstructed shell art with live-injected content such as portraits, icons, media, 3D previews, leaderboard avatars, or localizable text. Those regions must be identified from code before image generation work begins.

## Required Artifact

Each screen reference pack should contain:

- `content_ownership.json`

Typical location:

- `C:\UE\T66\SourceAssets\UI\<ScreenToken>Reference\content_ownership.json`

This file is a companion to:

- `screen_master.png`
- `reference_layout.json`
- `asset_manifest.json`

## Ownership Classes

Use one of these `content_type` values for each region:

- `generated-shell`
- `runtime-text`
- `runtime-image`
- `runtime-icon`
- `runtime-avatar`
- `runtime-media`
- `runtime-3d-preview`

The point is to say what owns the visible interior of the region at runtime.

## Render Contracts

Every mixed-ownership region should declare a `render_contract`:

- `shell-only`
  - the entire region is reconstructed art
- `socket-frame`
  - the frame/shell is reconstructed art, but the interior icon/image/avatar is injected live
- `empty-backplate`
  - the shell and a neutral interior backplate are reconstructed, but the final live content sits on top
- `open-aperture`
  - the frame is reconstructed, but the interior should stay open, dark, or neutral because runtime owns the view

## Audit Workflow

1. Read the screen code first.
2. Identify which visible areas are live-owned.
3. Record those regions in `content_ownership.json`.
4. Use that file to constrain:
   - style-reference prompting
   - sprite-family prompting
   - manifest rect separation
   - packaged review masks

## Code-First Clues

Treat these patterns as ownership signals:

- `SImage` using runtime or async-loaded brushes
- `BindSharedBrushAsync(...)`
- `MediaPlayer`, `OpenFile(...)`, or preview-video logic
- runtime-resolved portraits or avatars
- tooltips or buttons that wrap injected icons
- localizable `FText` labels that sit on generated shells

## Prompting Rule

If a region is runtime-owned, the generation prompt must say so directly.

Examples:

- `leave these portrait slots open; generate the frame only`
- `preserve the idol card shell but do not paint the looping media tile`
- `generate the ult/passive sockets only; runtime icons will be injected`
- `preserve the leaderboard avatar frame but do not bake profile pictures`

## Manifest Rule

For mixed-ownership regions, `reference_layout.json` should separate:

- outer shell rect
- inner live-content rect

Do not use one crop box for both.

## Review Rule

Packaged review must use ownership-aware validation:

- strict-diff only the generated shell region
- manually validate or mask the runtime-owned interior

## Example Cases

### Hero Selection

- hero carousel portrait slots: `runtime-image` + `socket-frame`
- idol/media tiles: `runtime-image` or `runtime-media` + `socket-frame` / `empty-backplate`
- ult/passive icon wells: `runtime-icon` + `socket-frame`
- center preview stage: `runtime-3d-preview` or `runtime-media` + `open-aperture`

### Main Menu

- profile avatar: `runtime-avatar` + `socket-frame`
- friend avatars: `runtime-avatar` + `socket-frame`
- leaderboard avatars: `runtime-avatar` + `socket-frame`

### Settings

- row labels and values: `runtime-text`
- dropdown values and option labels: `runtime-text`
- toggle/state labels: `runtime-text`

## Success Bar

The ownership audit is correct when:

- style references stop painting over live slots
- family boards produce sockets/frames instead of fake baked content
- manifests expose shell rects and live-content rects separately
- packaged diffs stop flagging expected runtime images as fixed-art misses
