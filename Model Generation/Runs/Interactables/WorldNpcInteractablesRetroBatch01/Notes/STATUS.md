# WorldNpcInteractablesRetroBatch01 Status

- Manifest generated: 2026-05-07T12:06:28
- Output root: C:\UE\T66\Model Generation\Runs\Interactables\WorldNpcInteractablesRetroBatch01
- Total audited rows: 71
- Props live rows: 0 (retired_no_live_rows)
- Needs source images: 26
- Needs Trellis: 26
- Needs Quad Retro: 31
- Needs Unreal import: 31

Scope guard:
- Regular enemies excluded.
- Bosses excluded.
- Weapon projectile meshes excluded unless future validation finds a broken reference.
- Loot crate included as AT66CrateInteractable item/loot crate visual, not a random-weapon behavior change.
- Floors/walls audited from CoherentThemeKit01 without Trellis regeneration.

## Completion update - 2026-05-07

- Source/Trellis stage: 31/31 ready with front QA.
- Quad Retro stage: 31/31 ready with front QA.
- Unreal-ready FBX export: 31/31 ready, 31/31 textures copied.
- Unreal import: 31/31 imported with StaticMesh, material instance, and pixelated texture.
- Live data update: 15 arcade rows updated in ArcadeInteractables.json; 3 NPC rows updated in CharacterVisuals.csv.
- Archived pre-update data/reference state:
  - C:\UE\T66\Archive\DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01\PreBatchLiveVisualReferences.json
  - C:\UE\T66\Archive\DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01\ArcadeInteractables.pre_world_npc_retro_batch01.json
  - C:\UE\T66\Archive\DataReorg_2026-05-07_WorldNpcInteractablesRetroBatch01\CharacterVisuals.pre_world_npc_retro_batch01.csv
- Validation: UnrealValidationReport errors=0; stale in-scope references=0.
- CoherentThemeKit01 floor/wall validation: 40/40 meshes, 40/40 materials, 40/40 textures; material parent remains M_Environment_Unlit.
- C++ build: T66Editor Win64 Development succeeded.

Visual QA notes:
- LootBag_Black, LootBag_Red, and LootBag_White are dark in the neutral Quad Retro QA render; imported material instances apply rarity brightness/tint overrides to improve gameplay readability.
- Saint is valid/imported, but the front QA silhouette reads more like a rear/side robed figure than a strong front-facing character. Treat as a visual review note, not an asset-reference failure.
- GamblerBoss keeps its existing legacy CharacterVisuals row because bosses are explicitly out of scope for this batch; the validation report records it as a boss-excluded legacy reference.
