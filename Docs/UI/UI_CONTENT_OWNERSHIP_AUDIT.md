# UI Content Ownership Audit

## Purpose

This document defines the required ownership audit that must happen before style-reference generation, family-board generation, or packaged diffing.

The core rule is simple:

- do not paint over runtime-owned content

Many T66 screens combine reconstructed shell art with live-injected content such as portraits, icons, media, 3D previews, leaderboard avatars, or localizable text. Those regions must be identified from code before image generation work begins.

For the current workflow, the main menu is the golden calibration screen. Its approved reference establishes the visual family, but the packaged target and ownership audit establish what may be baked. The title wordmark may be baked display art; the tagline and all menu/social/leaderboard labels, names, values, and states must remain live and localizable.

The full-screen reference and any buttonless/textless variants are offline comparison or prompting artifacts only. Runtime ownership must be expressed as a UI-free scene/background plate plus separate foreground component families and live-content wells.

## Required Artifact

Each screen reference pack should contain:

- `content_ownership.json`

Typical location:

- `C:\UE\T66\SourceAssets\UI\<ScreenToken>Reference\content_ownership.json`

This file is a companion to:

- `screen_master.png` as the offline comparison target
- `scene_plate.png` or the screen's equivalent UI-free background plate
- `reference_layout.json`
- `asset_manifest.json`

## Ownership Classes

Use one of these `content_type` values for each region:

- `generated-shell`
- `generated-scene-plate`
- `runtime-text`
- `runtime-value`
- `runtime-image`
- `runtime-icon`
- `runtime-avatar`
- `runtime-media`
- `runtime-3d-preview`
- `runtime-state`
- `baked-title-art`

The point is to say what owns the visible interior of the region at runtime.

Use `generated-scene-plate` only for background environment pixels that remain after all UI chrome, controls, panels, leaderboard rows, labels, values, avatars, icons, media, and preview content have been removed.

Treat every visible runtime-sourced string, number, image, and value as owned content. That includes subtitles, taglines, menu labels, friend names, leaderboard rows, scores, timers, prices, keybinds, selected values, toggle states, progress fills, avatars, badges, icons, preview thumbnails, media panels, and async-loaded images.

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
   - scene/background plate generation and acceptance

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
- visible control rect
- inner live-content rect

Do not use one crop box for shell art, interaction, and live content. A control may share a shell visually, but the manifest still needs a distinct control rect for hit testing and state anchoring.

## Review Rule

Packaged review must use ownership-aware validation:

- strict-diff only the generated shell region
- strict-diff the scene plate only in visible background areas where foreground UI does not cover it
- manually validate or mask the runtime-owned interior
- keep the baseline packaged capture at the canonical authoring target, normally `1920x1080`
- keep ownership masks in reference-coordinate space and transform them for supported aspect-bucket validation captures
- store or describe the mask set used for strict diffs

## Example Cases

### Hero Selection

- hero carousel portrait slots: `runtime-image` + `socket-frame`
- idol/media tiles: `runtime-image` or `runtime-media` + `socket-frame` / `empty-backplate`
- ult/passive icon wells: `runtime-icon` + `socket-frame`
- center preview stage: `runtime-3d-preview` or `runtime-media` + `open-aperture`

### Main Menu

- title wordmark: `baked-title-art` + `shell-only`
- tagline/subtitle: `runtime-text`
- center CTA labels: `runtime-text`
- profile avatar: `runtime-avatar` + `socket-frame`
- friend avatars: `runtime-avatar` + `socket-frame`
- leaderboard avatars: `runtime-avatar` + `socket-frame`
- friend names, statuses, leaderboard labels, ranks, names, scores, and tabs: `runtime-text` or `runtime-value`

#### Main Menu Steam/Social/Leaderboard Runtime Contract

- friends panel profile card: generated art owns the card shell and portrait well; Steam local display name, Steam ID/status text, action text, and local avatar remain runtime-owned `FText`/avatar brushes.
- friend rows: generated art owns only avatar frames, favorite/invite/offline button plates, and neutral row chrome; friend names, presence, online/offline grouping, invite pending state, favorite state, and friend avatars remain runtime data from Steam/session systems.
- invite/offline buttons: the visible plate must remain the real `SButton` content, not a hotspot over static art; generated assets are text-free plates, while `INVITE`, `INVITED`, `OFFLINE`, `In Party`, and `PARTY FULL` are runtime state text.
- leaderboard rows: generated art owns shell/control chrome and reusable row/socket frames; row count, rank, player/team names, score/time, local-player/favorite state, clickability, and run-summary availability remain runtime-owned.
- leaderboard avatars: generated art owns only the socket frame. Runtime first resolves the Steam avatar texture by leaderboard Steam ID, then falls back to a backend avatar URL if provided, then leaves the socket/default placeholder empty. Backend hero portraits are not a leaderboard identity fallback.
- player names, ranks, and scores: always runtime `FText`/values, never baked into row plates or reference-derived chrome.
- Steam connection/status text: runtime `FText` from `UT66SteamHelper`, `UT66PartySubsystem`, `UT66SessionSubsystem`, and backend/session state.

Packaged review must mask or fixture-freeze the profile, friend, party, and leaderboard live-content interiors before strict shell diffs. For the main menu leaderboard avatar socket, the current mask rect is `Right.leaderboard_avatar_live_rect` in `SourceAssets/UI/MainMenuReference/reference_layout.json`.

### Settings

- row labels and values: `runtime-text`
- dropdown values and option labels: `runtime-text`
- toggle/state labels: `runtime-text`

## Success Bar

The ownership audit is correct when:

- style references stop painting over live slots
- family boards produce sockets/frames instead of fake baked content
- manifests expose shell rects, visible control rects, and live-content rects separately
- packaged diffs stop flagging expected runtime images as fixed-art misses
