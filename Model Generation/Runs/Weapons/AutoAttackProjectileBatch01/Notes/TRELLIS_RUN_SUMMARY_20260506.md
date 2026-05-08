# Auto-Attack Projectile Batch 01

Status: Finished.

The 12 hero auto-attack projectile source images were generated with built-in ImageGen, mirrored to the RunPod workspace, processed through the locked TRELLIS.2 pod setup, and downloaded locally as raw GLB outputs.

## Roots

- Local root: `C:\UE\T66\Model Generation\Runs\Weapons\AutoAttackProjectileBatch01`
- Pod root: `/workspace/T66/ModelGeneration/Runs/Weapons/AutoAttackProjectileBatch01`
- Manifest: `batch_manifest.json`
- Source images: `Inputs/source_images`
- Raw GLBs: `Raw/Trellis`

## Trellis Settings

- Seed: `1337`
- Texture size: `2048`
- Decimation target: `80000`
- GPU: `NVIDIA A40`

## Verification

- Manifest entries: 12
- Local GLBs: 12
- Missing outputs: 0
- Undersized outputs below 100 KB: 0
- Blender GLB import/render QA: 12 / 12 passed

## Review

- Source image sheet: `Notes/source_image_contact_sheet.png`
- Raw GLB default-view sheet: `Review/raw_glb_qa_contact_sheet.png`
- Raw GLB side-view sheet: `Review/raw_glb_qa_contact_sheet_yaw90.png`
- Per-mesh QA metadata: `Review/RawGLB_QA/*.json`

## Outputs

| Projectile | Size |
| --- | ---: |
| `RoyalChad_Sword_S1337_D80000_Trellis2.glb` | 3,528,664 bytes |
| `ChineseChad_Guandao_S1337_D80000_Trellis2.glb` | 4,225,228 bytes |
| `BoxerChad_Glove_S1337_D80000_Trellis2.glb` | 4,607,212 bytes |
| `FoundingChad_Rapier_S1337_D80000_Trellis2.glb` | 3,385,584 bytes |
| `RoboChad_GearBlade_S1337_D80000_Trellis2.glb` | 4,040,584 bytes |
| `BillyChad_Bullet_S1337_D80000_Trellis2.glb` | 3,159,048 bytes |
| `RabbitChad_Carrot_S1337_D80000_Trellis2.glb` | 3,014,428 bytes |
| `CSChad_TacticalKnife_S1337_D80000_Trellis2.glb` | 3,516,040 bytes |
| `GoblinoChad_Cleaver_S1337_D80000_Trellis2.glb` | 3,897,432 bytes |
| `MonotoneChad_InkShard_S1337_D80000_Trellis2.glb` | 3,404,548 bytes |
| `BaldChad_Hatchet_S1337_D80000_Trellis2.glb` | 4,070,232 bytes |
| `RoachChad_RustyCrown_S1337_D80000_Trellis2.glb` | 4,209,160 bytes |

## Mesh Metadata

| Projectile | Triangles | Bounds Size |
| --- | ---: | --- |
| `RoyalChad_Sword` | 77,668 | `(0.125, 0.800, 0.997)` |
| `ChineseChad_Guandao` | 79,014 | `(0.640, 0.193, 1.002)` |
| `BoxerChad_Glove` | 79,331 | `(0.741, 1.001, 0.709)` |
| `FoundingChad_Rapier` | 75,528 | `(1.001, 0.193, 1.000)` |
| `RoboChad_GearBlade` | 79,748 | `(1.001, 0.213, 0.996)` |
| `BillyChad_Bullet` | 77,558 | `(0.359, 1.002, 0.359)` |
| `RabbitChad_Carrot` | 75,848 | `(0.485, 1.001, 0.469)` |
| `CSChad_TacticalKnife` | 75,202 | `(0.082, 0.851, 0.999)` |
| `GoblinoChad_Cleaver` | 78,384 | `(0.798, 0.159, 1.001)` |
| `MonotoneChad_InkShard` | 77,572 | `(0.832, 0.110, 0.999)` |
| `BaldChad_Hatchet` | 79,758 | `(0.842, 0.164, 1.001)` |
| `RoachChad_RustyCrown` | 74,690 | `(1.001, 0.879, 0.483)` |

## Logs

- Batch log: `Notes/trellis_weapon_projectiles_batch01.log`
- Server log: `Notes/trellis_server_20260506.log`

## Import Target

The manifest records the intended future Unreal asset paths under `/Game/Weapons/Projectiles/SM_*`. These GLBs are raw Trellis outputs and still need the Unreal import/setup pass before `Content/Data/Heroes.csv` can be filled with `AutoAttackProjectileMesh` references.
