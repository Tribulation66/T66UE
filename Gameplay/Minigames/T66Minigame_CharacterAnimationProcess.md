# T66 Minigame Character Animation Process

This is the shared T66 process for turning a character, enemy, tower, NPC, or combatant concept into a game-ready 2D animation atlas for any isolated minigame mode.

Use it for:

- `Mini Chadpocalypse`
- `Chadpocalypse TD`
- `T66Deck`
- `T66Idle`

This document is based on an inspection of `tachikomared/character-animation-creator-skill`, but it is not an installed Codex skill. Treat that repo as a reference pattern only; T66 keeps the actual workflow, outputs, QA, and future helper scripts inside this project.

External reference snapshot:

- repo: https://github.com/tachikomared/character-animation-creator-skill
- inspected commit: `1214b3bc1b087eefc9748a72a8c8a2c7786a55e8`
- useful upstream files: `SKILL.md`, `references/sheet-spec.md`, `scripts/pixel_snap.py`, `scripts/validate_64_sheet.py`, `scripts/export_animation_previews.py`

## Purpose

Use this process when a minigame actor needs more life than a static sprite.

The target output is a fixed-cell transparent PNG atlas plus QA artifacts that prove the sheet is usable before runtime, data-table, or Unreal import work starts.

The process supports simple animation first. A tower-defense enemy can start with a small bob loop like BTD instead of a full 8-direction walk cycle. Full 8-direction idle, walk, and attack sheets are available when a mode actually needs directional readability.

## Ownership Rule

The process is shared, but assets stay mode-owned.

Do not put TD, Deck, or Idle source art under `SourceAssets/Mini`, and do not import accepted sheets into the wrong cooked content tree.

Mode roots:

- Mini: `SourceAssets/Mini`, `Content/Mini`, `Gameplay/Minigames/Mini`
- TD: `SourceAssets/TD`, `Content/TD`, `Gameplay/Minigames/TD`
- Deck: `SourceAssets/Deck`, `Content/Deck`, `Gameplay/Minigames/Deck`
- Idle: `SourceAssets/Idle`, `Content/Idle`, `Gameplay/Minigames/Idle`

Shared helper scripts should live under `Tools/ArtPipeline/Minigamesgames` when they become mode-neutral. Keep mode-specific import or runtime wiring in the owning mode.

## Scope Options

Use the smallest useful scope for the actor.

- `bob-only`: 1 row, 4 or 6 frames, single facing
- `idle-only`: 1 row, 4 or 6 frames, single facing
- `idle-hit`: idle row plus hit-react row
- `idle-attack`: idle row plus attack row
- `walk-cardinal`: 4 direction rows, 6 frames per row
- `walk-8way`: 8 direction rows, 6 frames per row
- `idle-walk-8way`: 16 rows, 6 frames per row
- `combat-8way`: 24 rows, 6 frames per row

Default cell size is `64x64`.

Default TD enemy baseline:

- `bob-only`
- 4 or 6 frames
- one readable front or three-quarter facing
- small vertical bob, squash, cloak flap, weapon twitch, wing flap, or body pulse
- no need for per-direction walk rows unless path orientation matters in-game

Default full direction order:

```text
south
south-east
east
north-east
north
north-west
west
south-west
```

Default `combat-8way` row order:

```text
idle-south
idle-south-east
idle-east
idle-north-east
idle-north
idle-north-west
idle-west
idle-south-west
walk-south
walk-south-east
walk-east
walk-north-east
walk-north
walk-north-west
walk-west
walk-south-west
attack-south
attack-south-east
attack-east
attack-north-east
attack-north
attack-north-west
attack-west
attack-south-west
```

Idle rows use 4 unique frames padded to 6 cells when the atlas standard is 6 columns. Walk and attack rows use 6 frames.

Default `combat-8way` atlas size is `384x1536`.

## T66 Folder Layout

Use a per-actor run folder so raw generation, final sheets, and QA stay together.

```text
SourceAssets/<Mode>/<Family>/AnimationSets/<Actor>/<RunId>/
  prompts/
  references/
  generated/
  final/
    <Actor>_<Scope>_Atlas.png
    <Actor>_<Scope>_Atlas_Clean.png
    metadata.json
  qa/
    validation.json
    contact_sheet.png
    previews/
```

Examples:

```text
SourceAssets/TD/Enemies/AnimationSets/SkeletonCreep/Run01/
SourceAssets/Mini/Heroes/AnimationSets/Arthur/Run01/
SourceAssets/Deck/Enemies/AnimationSets/Cultist/Run01/
SourceAssets/Idle/Bosses/AnimationSets/StageBoss_001/Run01/
```

`<Family>` should match the owning mode's source groups. Common options:

- `Heroes`
- `Enemies`
- `Bosses`
- `NPCs`
- `Towers`
- `Companions`
- `VFX`

Do not place raw generated animation sheets directly into `Content/<Mode>`. `SourceAssets/<Mode>` remains the source-art workspace; Unreal imports belong to a later explicit import step.

## Inputs

Start with references that already belong to T66 whenever possible:

- accepted static sprite from `SourceAssets/<Mode>/<Family>/Singles/`
- portrait or source art from `SourceAssets/FinalPortraits/`
- approved enemy, boss, tower, or hero notes from the owning mode docs or data
- optional color, equipment, projectile, tower, or VFX references

Record the chosen references in the run folder before generating animation.

## Workflow

1. Define the actor identity.
2. Pick the smallest useful animation scope.
3. Generate or select a canonical base sprite.
4. Generate animation rows.
5. Assemble the fixed-cell atlas.
6. Run deterministic cleanup.
7. Validate the atlas.
8. Export previews.
9. Visually review the contact sheet and previews.
10. Only then import or wire runtime data.

## Actor Identity Lock

Before generation, write a small identity card in `prompts/identity.md`.

Required fields:

- actor name
- mode: Mini, TD, Deck, or Idle
- family: hero, enemy, boss, tower, NPC, companion, or VFX
- body shape and silhouette
- face, head, prop, or tower features that must not drift
- outfit, material, and palette
- weapon, attack, tower shot, or idle motion language
- scope and row list
- rejected traits, if any

The identity card is the contract for every generated row. If rows drift into a different outfit, weapon, species, head shape, tower shape, or body scale, regenerate only the failing row group.

## Generation Rules

Use Codex-native image generation for visual creation when requested. Do not require API keys for this workflow.

Generate row strips, not one frame at a time, unless a row repeatedly fails.

Prompt template:

```text
64x64 pixel-art game sprite animation strip.
One row, <N> separated frames, <action> action, <facing or direction>.
Flat chroma-key background, no shadows, no floor, no UI, no text, no frame numbers.
Preserve the canonical actor identity exactly:
<identity card summary>.
Hard pixel-art edges, limited palette, readable silhouette.
Keep the actor fully inside each 64x64 frame.
```

TD bob-loop prompt addition:

```text
Animation goal: simple tower-defense enemy idle/bob loop, like a small readable BTD-style creep motion.
Keep movement subtle: body rises and settles, arms or cloak move slightly, no walking across the cell.
The actor remains centered and fully inside each frame.
```

Use a chroma-key background that does not exist in the actor. Prefer green `#00ff00`; use magenta `#ff00ff` for green actors, poison effects, cyber glow, or other green-heavy designs.

Do not hand-draw missing animation frames with code. Scripts can crop, normalize, quantize, remove background, compose an atlas, validate frames, and export previews. If animation quality fails, regenerate the failing row group.

## Chroma Key Rules

Background removal must be conservative.

Preferred native implementation:

1. Remove only chroma-key pixels connected to the image border.
2. Fit each sprite into the target cell.
3. Remove near-exact chroma residue only after fitting.

Do not globally delete all green-ish pixels. The existing Mini split tooling already has a useful model in `Tools/ArtPipeline/Minigames/T66MiniSplitSheets.py` through `remove_edge_connected_background`; a shared helper should move that idea into `Tools/ArtPipeline/Minigamesgames`.

Suggested thresholds:

- edge flood threshold: `100-110` distance from key
- final residue threshold: `60-75` distance from key
- alpha threshold: `12` for attack or VFX-heavy sheets
- alpha threshold: `24` for simple idle, bob, or walk sheets
- palette: `96-128` for attack or VFX-heavy sheets
- palette: `32-64` for simple idle, bob, or walk sheets

## Cleanup

Cleanup should be deterministic and repeatable.

Future native helper target:

```powershell
python .\Tools\ArtPipeline\Minigamesgames\T66MinigamePixelSnap.py `
  --input .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\final\SkeletonCreep_BobOnly_Atlas.png `
  --output .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\final\SkeletonCreep_BobOnly_Atlas_Clean.png `
  --cell 64 `
  --palette 64 `
  --alpha-threshold 24 `
  --pixelate-scale 1
```

Use `--pixelate-scale 2` only when the image is too smooth. If cleanup damages face, hands, tower tips, weapon tips, or attack read, rerun with a larger palette or no pixelate scale.

## Validation

Future native helper target:

```powershell
python .\Tools\ArtPipeline\Minigamesgames\T66MinigameValidateSpriteAtlas.py `
  --input .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\final\SkeletonCreep_BobOnly_Atlas_Clean.png `
  --rows 1 `
  --columns 6 `
  --cell 64 `
  --json-out .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\qa\validation.json `
  --contact-sheet .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\qa\contact_sheet.png
```

Validation must block acceptance when:

- atlas dimensions are wrong
- required frames are empty or nearly empty
- non-transparent pixels touch cell edges unexpectedly
- chroma-key residue remains as opaque pixels
- row count or column count does not match the declared scope

Validation scripts catch geometry and pixel risks only. Visual review is still required for animation quality, character identity, silhouette stability, and readable attacks.

## Preview Export

Future native helper target:

```powershell
python .\Tools\ArtPipeline\Minigamesgames\T66MinigameExportAnimationPreviews.py `
  --atlas .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\final\SkeletonCreep_BobOnly_Atlas_Clean.png `
  --rows 1 `
  --columns 6 `
  --cell 64 `
  --row-names bob `
  --prefix idle `
  --out-dir .\SourceAssets\TD\Enemies\AnimationSets\SkeletonCreep\Run01\qa\previews `
  --scale 4
```

Prefer transparent animated WebP for review. GIF previews are compatibility outputs only and need stable palette handling, transparency index reservation, no dithering, and `disposal=2`.

## Visual QA

Review both the contact sheet and animation previews before import.

Reject the run when:

- identity changes between rows
- direction read is unclear when direction rows are required
- feet slide or body scale jitters heavily
- bob motion moves across the cell instead of staying centered
- idle, walk, hit, and attack use conflicting proportions
- attack effects detach from the weapon, tower, or body unintentionally
- effects cross into neighboring cells
- shadows, floor marks, labels, frame borders, or UI text are present

For TD enemies, the minimum bar is readability in motion: the actor should feel alive while the path movement supplies traversal. A subtle bob, pulse, wing flap, cape flap, or weapon twitch is enough for many creeps.

For melee attacks, require anticipation, contact, and recovery. Keep arcs attached to the weapon and inside the cell.

For ranged attacks, animate the firing or casting motion. Do not bake a projectile traveling across the sheet unless that is explicitly the intended VFX sheet.

For magic attacks, keep effects hard-edged and close to the body, tower, or weapon. Avoid soft bloom and transparent particles for small sprites.

## Metadata

Write `final/metadata.json` next to the final sheet.

Minimum fields:

```json
{
  "mode": "TD",
  "actor": "SkeletonCreep",
  "family": "Enemies",
  "scope": "bob-only",
  "cell_size": 64,
  "columns": 6,
  "rows": 1,
  "directions": [],
  "actions": {
    "bob": 6
  },
  "source_references": [],
  "qa": {
    "validation_json": "qa/validation.json",
    "contact_sheet": "qa/contact_sheet.png",
    "previews": "qa/previews"
  }
}
```

For a full 8-direction combat sheet, include direction order and frame counts:

```json
{
  "mode": "Mini",
  "actor": "Arthur",
  "family": "Heroes",
  "scope": "combat-8way",
  "cell_size": 64,
  "columns": 6,
  "rows": 24,
  "directions": [
    "south",
    "south-east",
    "east",
    "north-east",
    "north",
    "north-west",
    "west",
    "south-west"
  ],
  "actions": {
    "idle": 4,
    "walk": 6,
    "attack": 6
  },
  "idle_padded_to": 6
}
```

## Integration Notes

Do not change minigame runtime code until a sheet passes visual QA.

The first integration step should be source-art import, not gameplay behavior changes. Existing Mini import behavior is static-sprite oriented, and TD, Deck, and Idle import paths are still mode-specific. Accepted animation atlases need an explicit import and wiring pass for the owning mode.

Likely native follow-up work:

- add `Tools/ArtPipeline/Minigamesgames/T66MinigamePixelSnap.py`
- add `Tools/ArtPipeline/Minigamesgames/T66MinigameValidateSpriteAtlas.py`
- add `Tools/ArtPipeline/Minigamesgames/T66MinigameExportAnimationPreviews.py`
- add a shared manifest shape for minigame animation runs
- add mode-owned Unreal import wrappers for accepted animation atlases
- add runtime metadata consumption only after one accepted sheet proves the format

## Relationship To Mini Walksheet Pipeline

The current [T66Mini_WalksheetPipeline.md](C:/UE/T66/Gameplay/Minigames/Mini/T66Mini_WalksheetPipeline.md) remains the lightweight Mini-specific process for one 2x2 walk candidate.

Use this shared document when the target is a reusable animation atlas process for any minigame mode, including TD bob loops, Deck combat idles, Idle stage enemies, or full 8-direction Mini sheets.
