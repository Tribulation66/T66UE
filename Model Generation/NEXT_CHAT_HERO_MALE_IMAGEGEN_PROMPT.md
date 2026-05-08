# Next Chat Prompt: Male Hero Source Image Batch

Copy this into the next Codex chat.

```text
We are continuing the T66 Quad Retro hero art pipeline in C:\UE\T66.

First read these files, in this order:
1. C:\UE\T66\Model Generation\README.md
2. C:\UE\T66\Model Generation\TRELLIS_SOURCE_IMAGE_RULES.md
3. C:\UE\T66\Model Generation\RETRO_CHARACTER_PIPELINE.md
4. C:\UE\T66\Model Generation\HERO_CHAD_STACY_PROMPT_GUIDE.md
5. C:\UE\T66\Model Generation\QUAD_RETRO_DO_THIS_RUNBOOK.md

Task:
Generate source image candidates for the remaining 11 male Chad heroes only. Do not run TRELLIS yet, do not run Blender yet, and do not generate Stacy variants yet.

Use Boxer Chad as the silhouette/style reference:
C:\UE\T66\Model Generation\Runs\Heroes\QuadRetroSourceExploration01\Inputs\imagegen_candidates\BoxerChad_ExaggeratedV_04.png

The already-selected reference hero is:
- Hero_3: Boxer Chad

Generate 4 source-image variants each for:
- Hero_1 Royal Chad
- Hero_2 Chinese Chad
- Hero_4 Founding Chad
- Hero_5 Robo Chad
- Hero_6 Billy Chad
- Hero_7 Rabbit Chad
- Hero_8 CS Chad
- Hero_9 Goblino Chad
- Hero_10 Monotone Chad
- Hero_11 Bald Chad
- Hero_12 Roach Chad

Save all outputs under:
C:\UE\T66\Model Generation\Runs\Heroes\ChadStacySourceImageBatch01\Inputs\male_candidates

Rules:
- Use the built-in Codex image generation workflow if available. Do not switch to an API-key workflow unless I explicitly ask.
- Source images must be clean TRELLIS input, not final retro art.
- Full-body front view, neutral A-pose, arms angled 30-45 degrees down and separated from torso.
- Orthographic/flat camera, no perspective distortion, no action pose.
- Flat magenta #FF00FF background unless a character uses too much magenta, then use chroma green #00FF00.
- Flat even lighting, no floor, no shadow, no glow.
- Clean painted/cel-shaded concept art, not photoreal, not pixel art.
- Push the male body silhouette harder than realistic bodybuilding: extremely wide shoulders, oversized chest/traps, thick arms, very thin waist, dramatic V-shaped torso, hips much narrower than shoulders.
- Face is low-detail and not the identity carrier. Identity comes from costume, equipment, hair/headwear, and large color blocks.
- Use HERO_CHAD_STACY_PROMPT_GUIDE.md for each hero's identity block.

After generation:
- Make the outputs easy to review with direct links or a contact sheet.
- Tell me which 1-2 candidates per hero look most TRELLIS-safe and why.
- Do not proceed to TRELLIS until I approve the source images.
```
