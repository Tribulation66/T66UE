# Phase 1C ImageGen Batch 3 Report

Date: 2026-05-17

## Scope

Phase 1C Track A and Track B generated corrected flat-matte source images, isolated them to transparent PNGs with `rembg`, and ran the amended QA gate before Pixal3D submission.

No Pixal3D, Blender, UE import, material, or runtime test-room work is covered here.

## Tools

- Image generation wrapper: `ToonStyle/Tools/RunCodexImageGen.ps1`
- Isolation script: `ToonStyle/Tools/rembg_isolate.py`
- QA script: `ToonStyle/Tools/image_qa.py`
- rembg model: `u2net`
- QA summary: `Saved/Codex/ToonStyle/Phase1C/image_qa_summary.json`

`RunCodexImageGen.ps1` was patched so Codex CLI plugin-sync warnings written to stderr do not automatically fail an otherwise successful image-generation run.

## Prompt Amendments Applied

- The continuous black-outline anchor sentence was appended to all eleven prompts.
- Gambler used the amended demon dealer plus casino table prompt.
- ARIA and CaveBat required prompt retries before their outputs passed QA; the final accepted prompts preserved the Phase 1C flat-matte and black-outline constraints.

## Outputs And QA

| Asset | Source PNG | Isolated PNG | QA Result | Source Coverage | Alpha Coverage | Largest Component |
| --- | --- | --- | --- | ---: | ---: | ---: |
| Lu Bu | `SourceAssets/ToonStyle/ImageGen/Phase1C/LuBu/lubu_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/LuBu/lubu_v01_isolated.png` | Pass | 0.2833 | 0.4065 | 0.9944 |
| ARIA | `SourceAssets/ToonStyle/ImageGen/Phase1C/ARIA/aria_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/ARIA/aria_v01_isolated.png` | Pass | 0.1725 | 0.2090 | 0.9882 |
| Gambler | `SourceAssets/ToonStyle/ImageGen/Phase1C/Gambler/gambler_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/Gambler/gambler_v01_isolated.png` | Pass | 0.4609 | 0.5339 | 0.9971 |
| Slime | `SourceAssets/ToonStyle/ImageGen/Phase1C/Slime/slime_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/Slime/slime_v01_isolated.png` | Pass | 0.4892 | 0.5232 | 0.9961 |
| TombSpider | `SourceAssets/ToonStyle/ImageGen/Phase1C/TombSpider/tombspider_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/TombSpider/tombspider_v01_isolated.png` | Pass | 0.3180 | 0.4252 | 0.9923 |
| CaveBat | `SourceAssets/ToonStyle/ImageGen/Phase1C/CaveBat/cavebat_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/CaveBat/cavebat_v01_isolated.png` | Pass | 0.2976 | 0.3264 | 0.9933 |
| Idol Altar | `SourceAssets/ToonStyle/ImageGen/Phase1C/IdolAltar/idolaltar_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/IdolAltar/idolaltar_v01_isolated.png` | Pass | 0.4918 | 0.5174 | 0.9949 |
| Arcade Machine | `SourceAssets/ToonStyle/ImageGen/Phase1C/ArcadeMachine/arcademachine_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/ArcadeMachine/arcademachine_v01_isolated.png` | Pass | 0.4062 | 0.5114 | 0.9956 |
| Loot Chest | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootChest/lootchest_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootChest/lootchest_v01_isolated.png` | Pass | 0.4880 | 0.5047 | 0.9908 |
| Loot Bag Yellow | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootBagYellow/lootbag_yellow_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootBagYellow/lootbag_yellow_v01_isolated.png` | Pass | 0.4616 | 0.5333 | 0.9970 |
| Loot Crate | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootCrate/lootcrate_v01.png` | `SourceAssets/ToonStyle/ImageGen/Phase1C/LootCrate/lootcrate_v01_isolated.png` | Pass | 0.6308 | 0.6479 | 0.9960 |

## Gate Results

- G1: Passed. Lu Bu rembg output had a real alpha channel, transparent corners, and preserved subject coverage.
- G2.5: Passed for all eleven image/isolation pairs.

## Notes

The QA checks caught the intended failure classes before Pixal3D: non-white background, missing alpha, oversized/undersized subject, off-center subject, and detached fragments. No asset was skipped at the QA stage.

