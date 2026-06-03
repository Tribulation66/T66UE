# Phase 1A Pixal3D Lineup Report

Endpoint used:

`ssh root@69.30.85.138 -p 22082 -i ~/.ssh/id_ed25519`

In-pod generate endpoint:

`http://127.0.0.1:18001/generate`

Locked base settings:

- `X-Resolution=1536`
- `X-Texture-Size=4096`
- `X-SS-Steps=12`
- `X-Shape-Steps=12`
- `X-Tex-Steps=12`
- Default guidance values: SS `7.5`, shape `7.5`, texture `1.0`

## Generation Results

| Asset | Source | Pixal3D result | Timing | Raw GLB |
|---|---|---:|---:|---|
| ARIA | `aria_front_v01.png` | Success, remesh on | 105.8s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/ARIA/Raw/aria.glb` |
| Gambler | `gambler_front_v01.png` | Success, remesh on | 99.0s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/Gambler/Raw/gambler.glb` |
| Slime | `slime_front_v01.png` | Initial remesh export failed; no-remesh retry succeeded | 274.7s retry | `SourceAssets/ToonStyle/Pixal3D/Phase1A/Slime/Raw/slime.glb` |
| TombSpider | `tombspider_front_v01.png` | Success, remesh on | 161.4s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/TombSpider/Raw/tombspider.glb` |
| CaveBat | `cavebat_front_v01.png` | Success, remesh on | 99.9s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/CaveBat/Raw/cavebat.glb` |
| Idol Altar | `idolaltar_front_v01.png` | Success, remesh on | 177.1s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/IdolAltar/Raw/idolaltar.glb` |
| Arcade Machine | `arcademachine_front_v01.png` | Success, remesh on | 174.3s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/ArcadeMachine/Raw/arcademachine.glb` |
| Loot Chest | `lootchest_front_v01.png` | Success, remesh on | 181.7s | `SourceAssets/ToonStyle/Pixal3D/Phase1A/LootChest/Raw/lootchest.glb` |
| Loot Bag Yellow | `lootbag_yellow_v01.png` | Failed: CuMesh CUDA OOM during `fill_holes()` | 154.3s final retry | No GLB |
| Loot Crate | `lootcrate_front_v01.png` | Initial run failed after prior OOM; isolated no-remesh retry succeeded | 276.7s retry | `SourceAssets/ToonStyle/Pixal3D/Phase1A/LootCrate/Raw/lootcrate.glb` |

Logs:

- Main batch: `SourceAssets/ToonStyle/Pixal3D/Phase1A/LineupBatch/Logs/pixal3d_generation_status.jsonl`
- No-remesh retry: `SourceAssets/ToonStyle/Pixal3D/Phase1A/LineupRetryNoRemesh/Logs/pixal3d_generation_status.jsonl`
- Loot Bag isolated retry: `SourceAssets/ToonStyle/Pixal3D/Phase1A/LootBagYellowRetrySingle/Logs/pixal3d_generation_status.jsonl`
- Loot Crate isolated retry: `SourceAssets/ToonStyle/Pixal3D/Phase1A/LootCrateRetrySingle/Logs/pixal3d_generation_status.jsonl`

## UE Import Results

All imported assets below used the new `RunPixal3DToUE.ps1` pipeline and verified at approximately 180 UU unscaled height. No runtime actor-scale workaround is required.

| Asset | Static Mesh | Verified height | Textures |
|---|---|---:|---:|
| Lu Bu validation | `/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation` | 180.000 | 2 |
| ARIA | `/Game/ToonStyle/TestAssets/Lineup/SM_aria` | 180.000 | 2 |
| Gambler | `/Game/ToonStyle/TestAssets/Lineup/SM_gambler` | 180.000 | 2 |
| Slime | `/Game/ToonStyle/TestAssets/Lineup/SM_slime` | 180.000 | 2 |
| TombSpider | `/Game/ToonStyle/TestAssets/Lineup/SM_tombspider` | 180.000 | 2 |
| CaveBat | `/Game/ToonStyle/TestAssets/Lineup/SM_cavebat` | 180.000 | 2 |
| Idol Altar | `/Game/ToonStyle/TestAssets/Lineup/SM_idolaltar` | 180.000 | 2 |
| Arcade Machine | `/Game/ToonStyle/TestAssets/Lineup/SM_arcademachine` | 180.000 | 2 |
| Loot Chest | `/Game/ToonStyle/TestAssets/Lineup/SM_lootchest` | 180.000 | 2 |
| Loot Crate | `/Game/ToonStyle/TestAssets/Lineup/SM_lootcrate` | 180.000 | 2 |

Loot Bag Yellow has no UE asset because Pixal3D did not produce a GLB.

## Anomalies

Slime required no-remesh because the remeshed export path failed with CuMesh CUDA OOM. Loot Crate also required a fresh-server no-remesh retry after previous OOMs left the server in a bad GPU-memory state.

Loot Bag Yellow failed three times:

1. Main batch with remesh on.
2. Retry batch with remesh off.
3. Isolated no-remesh retry on a freshly restarted server.

The final failure still occurred inside `decode_latent()` / `fill_holes()`, before GLB export. This is not a UE import problem and not a material-binding problem.

