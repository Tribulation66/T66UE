# Chad Pass02 Process Build

This folder is for the next Chad source-image and TRELLIS quality pass. It is a
process-building workspace, not a replacement for the already validated
Batch01 rigging, Unreal import, or DataTable wiring.

## Goal

Build a repeatable source-to-TRELLIS gate that produces better Chad bodies and
heads before any new import/wiring pass happens.

The artistic target is not a normal athletic man. Every Chad body should read as
a giant giga-chad/nephilim: human, but beyond realistic human proportion, with
huge shoulders, huge traps, massive chest, thick arms and legs, and a very thin
waist. Heads should share a standardized giga-chad skull block and scale while
keeping hero identity through ethnicity, hair color, hair silhouette, beard
pattern, scars, makeup, and expression.

## Active Hero Numbering

Use contiguous active hero IDs for new authoring:

| Active ID | Name | Batch01 source/import ID currently reused |
| --- | --- | --- |
| `Hero_1` | Arthur | `Hero_1` |
| `Hero_2` | Lu Bu | `Hero_3` |
| `Hero_3` | Mike | `Hero_4` |
| `Hero_4` | George | `Hero_5` |
| `Hero_5` | Robo | `Hero_7` |
| `Hero_6` | Billy | `Hero_8` |
| `Hero_7` | Rabbit | `Hero_9` |
| `Hero_8` | Shroud | `Hero_11` |
| `Hero_9` | xQc | `Hero_12` |
| `Hero_10` | Moist | `Hero_13` |
| `Hero_11` | North | `Hero_14` |
| `Hero_12` | Asmon | `Hero_15` |

The live `Content/Data/Heroes.csv` and `CharacterVisuals.csv` rows already use
`Hero_1` through `Hero_12`. Some current asset paths intentionally still point
to Batch01 imported assets under the older source/import IDs. Do not rename or
move those imported assets during this process pass.

## Local Evidence

- Mike Pass02 TRELLIS process test:
  [MIKE_TRELLIS_TEST_20260504.md](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/MIKE_TRELLIS_TEST_20260504.md)
- Current Mike rigging handoff candidate:
  [Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb)
- Rejected Mike neck/join experiments archive:
  [ARCHIVE_MANIFEST.md](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/ARCHIVE_MANIFEST.md)
- Batch01 four-angle renders:
  [INDEX.md](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ChadFourAngleRenders/INDEX.md)
- Batch01 source diagnostic contact sheet:
  [approved_seed_diagnostic_contact_sheet.png](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ProcessBuild_20260504/approved_seed_diagnostic_contact_sheet.png)
- Batch01 source background stats:
  [approved_seed_background_stats.json](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ProcessBuild_20260504/approved_seed_background_stats.json)

## Working Rule

Do not send a source PNG to TRELLIS just because it exists. A source image must
pass visual review plus the source-audit gate first. A TRELLIS GLB must pass
side/depth review before assembly approval.

Mike Pass02 added one more source rule: head-only sources need a narrow
insertable neck plug. A broad rectangular neck column under the ears must be
rejected or preprocessed before TRELLIS because it generates a visible rear
head/neck slab.

Mike Pass02 handoff state: model generation is paused on the Mike rig prototype. It is acceptable
as a rigging process test candidate only. It is not final art approval, because
the neck bridge is still a procedural review aid and the rear-hair lesson needs
to be solved in future generated sources.

## RunPod Hugging Face Auth Rule

Do not write raw Hugging Face tokens into this repo, prompt files, process docs,
or shell scripts. Authenticate the active pod so the token is stored in the
pod's Hugging Face cache, then verify gated DINOv3 access before starting
TRELLIS.

Use this pattern with a token supplied out-of-band:

```bash
source /opt/conda/etc/profile.d/conda.sh
conda activate trellis2
python -c "from huggingface_hub import login; login(token='<HF_TOKEN>', add_to_git_credential=False)"
python -c "from huggingface_hub import hf_hub_download; hf_hub_download('facebook/dinov3-vitl16-pretrain-lvd1689m', 'config.json'); print('DINOv3 auth ok')"
```

After auth succeeds, start the server and verify `/health`:

```bash
cd /workspace/TRELLIS.2
export NVCC_PREPEND_FLAGS=
export CUDA_HOME=/usr/local/cuda
export PATH=/usr/local/cuda/bin:/usr/bin:/bin:/opt/conda/envs/trellis2/bin
export PYTHONPATH=/workspace/TRELLIS.2
nohup python -u /workspace/TRELLIS.2/trellis_server.py > /workspace/TRELLIS.2/trellis_server.log 2>&1 < /dev/null &
curl -s http://127.0.0.1:8000/health
```

