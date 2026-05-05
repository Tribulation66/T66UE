# Type A Masculine Hero Batch 01

Historical note: this Batch01 folder preserves the original source/import IDs
used when the assets were generated. Current authored gameplay IDs are now
contiguous `Hero_1` through `Hero_12`; the older IDs in this run are legacy
source/import aliases and should not be used as the naming model for new Chad
passes.

## Scope

This batch is for the reduced 12-hero Type A masculine roster. The same 12-hero set is now the active gameplay/model-generation roster, with the removed heroes deleted from the live hero CSV sources and supporting model-generation prompts.

Removed from this generation batch:

- `Hero_2` Merlin
- `Hero_6` Zeus
- `Hero_10` Dog
- `Hero_16` Forsen

Active roster:

| Hero ID | Name | Standard generation | Beach goer generation |
| --- | --- | --- | --- |
| `Hero_1` | Arthur | reuse existing head/body/weapon baseline | generate beach body and beach weapon |
| `Hero_3` | Lu Bu | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_4` | Mike | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_5` | George | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_7` | Robo | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_8` | Billy | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_9` | Rabbit | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_11` | Shroud | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_12` | xQc | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_13` | Moist | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_14` | North | generate head, body, weapon | generate beach body and beach weapon |
| `Hero_15` | Asmon | generate head, body, weapon | generate beach body and beach weapon |

## Image Rules

- Use humanoid Type A masculine proportions for every hero.
- Keep the body silhouette compatible across the batch: broad shoulders, readable torso mass, strong arms and legs, centered upright front view.
- Vary clothing, colors, materials, hair, head shape, facial structure, and expression so the heroes do not look like Arthur clones.
- Avoid long robes, long coats, capes, skirts, or garments that hang below the knees because they can create rigging and leg-readability problems.
- Do not generate weapons attached to bodies. Generate every weapon as its own clean isolated image.
- Do not generate combined body-plus-head production images. Generate body-only and head-only standard images separately.
- Head-only images must be completely straight-on like the matching A-pose body: no three-quarter turn, no side tilt, no looking left or right, and a centered symmetrical face axis.
- For public-name heroes, make original stylized game characters. Do not target real-person likenesses.

## Standard Outfit Outputs

Each non-Arthur active hero needs:

- `Inputs/<HeroID>_<Name>_TypeA_Standard_BodyOnly_Green.png`
- `Inputs/<HeroID>_<Name>_TypeA_Standard_HeadOnly_Green.png`
- `Inputs/<HeroID>_<Name>_TypeA_Standard_WeaponOnly_Green.png`

Arthur already has usable standard split inputs and GLBs in `Runs/Heroes/Hero_1_Arthur`.

The generated prompt manifest is `batch_manifest.json`. It currently defines 57 new ImageGen inputs:

- 33 standard prompts for the 11 non-Arthur active heroes.
- 24 beach goer prompts for all 12 active heroes.

Per-asset prompt files are in `Inputs/prompts`. Approved generated PNGs should be saved to `Inputs/approved_seed_images` using the exact `target_png` names in the manifest before the TRELLIS script is run.

Preferred image source is the interactive ImageGen workflow. For long completion runs where the interactive image tool cannot practically finish dozens of remaining assets, use `Model Generation/Scripts/generate_typea_batch01_missing_seed_images.py` as a fallback-only seed generator, then review every generated contact sheet before TRELLIS. Do not treat fallback images as accepted just because the file exists.

Important finding from this batch: fallback seed images can unblock the manifest count but may still generate flat source-image panels, background cards, or poster-like GLBs. If a fallback image has any scene block, white/gray panel, green hole, or loose crop, reject it before TRELLIS for production work. The 2026-05-02 completion run generated all GLBs, but fallback-derived models need visual approval or reroll before they should be treated as final character assets.

## Beach Goer Outfit Outputs

Each active hero, including Arthur, needs:

- `Inputs/<HeroID>_<Name>_TypeA_BeachGoer_BodyOnly_Green.png`
- `Inputs/<HeroID>_<Name>_TypeA_BeachGoer_WeaponOnly_Green.png`

Beach goer heads reuse the standard head. Do not generate separate beach heads unless the standard head is rejected.

## Beach Variety Targets

- Arthur: heroic swim armor / board shorts, beach umbrella spear or foam sword.
- Lu Bu: red-black athletic beach armor, pool-noodle halberd or lifeguard polearm.
- Mike: street brawler beachwear, beach volleyball hammer or weighted cooler club.
- George: clean nautical duelist beachwear, fencing-oar or compact beach saber.
- Robo: waterproof humanoid plating, water cannon or pressure sprayer.
- Billy: outlaw beachwear, stylized water pistol.
- Rabbit: agile trickster beachwear, inflatable ring shield or beach paddle weapon.
- Shroud: dark tactical beachwear, compact harpoon or stealthy water blaster.
- xQc: high-energy neon beachwear, oversized water blaster.
- Moist: understated beachwear, deadpan novelty squirt weapon.
- North: cold-water beachwear / short wetsuit, ice-cooler shield or surfboard blade.
- Asmon: scavenger beachwear, beach shovel or treasure-hunter hook.

## TRELLIS Settings

Use the locked TRELLIS baseline unless a generated input fails QA:

- seed: `1337`
- texture: `2048`
- body decimation: `80000`
- head decimation: `120000`
- weapon decimation: `200000`

RunPod TRELLIS batch script: `Notes/run_trellis_typea_batch01.sh`.

The batch script skips an existing GLB by default so interrupted or partial runs can resume without wasting pod time. Set `FORCE_REGEN=1` only when an approved input has intentionally changed and the existing GLB must be overwritten.

## QA Rules

- Review inputs before TRELLIS; reject any image with attached weapons, full-body face loss, below-knee cloth, wrong silhouette, copied Arthur face, or an off-axis / three-quarter head.
- Review GLBs in Blender side by side by category: standard heads, standard bodies, standard weapons, beach bodies, beach weapons.
- Keep raw generated models separate from any assembled Blender review or rigging output.
- A `model_ready` manifest status only means the GLB exists. It does not mean the model passed art QA.
