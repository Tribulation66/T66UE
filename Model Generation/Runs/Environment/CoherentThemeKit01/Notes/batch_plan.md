# CoherentThemeKit01 Batch Plan

## Goal

Generate a cohesive modular environment-kit batch for the tower themes by using source sheets instead of unrelated one-off images.

The old `DungeonKit01` pass generated individual wall and floor references. The meshes worked as isolated modules, but the wall set did not read like one shared world. This batch changes the source-image strategy:

1. Generate one coherent source sheet per theme and surface type.
2. Split that sheet into four equal module crops.
3. Feed the crops, not the full sheet, into TRELLIS.
4. Normalize every accepted mesh to the existing visual-overlay contract.

This keeps the runtime modular while making the art language come from a single shared plate.

## Theme Scope

This batch targets four wall modules and four floor modules for each theme:

| Runtime / Design Target | Kit Label | Notes |
|---|---|---|
| Easy / Dungeon | `DungeonKit02` | stone dungeon, chunky bones, attached chains, simple torches |
| Medium / Forest | `ForestKit01` | walls made from trunks, roots, bark slabs, vines, mushrooms |
| Hard / Ocean | `OceanKit01` | coral reef walls, shell limestone, kelp, reef stone, tide-worn floors |
| VeryHard / Martian | `MartianKit01` | red rock, Mars regolith, basalt, ruins, crystal veins |
| Impossible / Hell | `HellKit01` | brimstone, obsidian, skull relief, chains, lava cracks |

Implementation note: the current repo docs and `T66TowerThemeVisuals` map `Hard -> Ocean` and `VeryHard -> Martian`. This batch follows that Martian `VeryHard` mapping.

## Source Sheet Rules

Wall sheets:

- one 2x2 sheet per theme
- intended size: `3072 x 2048`
- four equal cells, each cell becomes one `1536 x 1024` wall reference
- straight-on orthographic front view
- rectangular wall silhouette, broad chunky shapes, shallow relief
- no floor slab, no ceiling, no room, no characters

Floor sheets:

- one 2x2 sheet per theme
- intended size: `2048 x 2048`
- four equal cells, each cell becomes one `1024 x 1024` floor reference
- top-down orthographic view
- flat readable walkable surface
- low-profile detail only
- no walls, no room, no tall center props

All sheets:

- flat solid `#00ff00` background
- consistent camera, lighting, palette, and material language across all four cells
- no text, labels, watermarks, UI, decorative borders, or cast shadows
- no fine filigree, tiny surface noise, hair-thin roots, wire-thin chains, or dense rubble
- details must be designed as broad low-poly forms that can survive 3D reconstruction
- props such as bones, torches, chains, roots, crystals, and skulls must be attached to the module surface

## Module Counts

Target output count:

- This batch uses `10` source sheets total:
  - five themes
  - two sheet types per theme: wall and floor
- `40` Trellis module inputs after splitting:
  - four wall crops per theme
  - four floor crops per theme

## Workflow

1. Generate the eight source sheets from the prompts in `Inputs/prompts`.
2. Save them under `Inputs/source_sheets` using the `source_sheet` filenames in `batch_manifest.json`.
3. Run:

   ```powershell
   python "C:\UE\T66\Model Generation\Scripts\split_theme_module_sheet.py" `
     --run-root "C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01" `
     --manifest "C:\UE\T66\Model Generation\Runs\Environment\CoherentThemeKit01\batch_manifest.json"
   ```

4. Upload `Inputs/approved_seed_images` to the RunPod workspace.
5. Run each crop through the TRELLIS server with:
   - seed: `1337`
   - texture size: `2048`
   - decimation: `80000`
   - `preprocess_image=True`
6. Run raw QA renders.
7. Reject background platforms, detached fragments, unreadable silhouettes, or over-detailed noise.
8. Normalize accepted walls/floors to the existing Unreal-ready visual-overlay dimensions.

## Acceptance Notes

Use the existing DungeonKit runtime contract unless a later import test proves a better convention:

- wall visual target: `1300 cm` length, `1200 cm` height, `120 cm` depth
- floor visual target: `1300 x 1300 cm`, `24 cm` visual thickness
- generated visuals remain separate from collision
- hidden simple proxies remain collision-authoritative
- do not promote complex generated mesh collision for traversal
