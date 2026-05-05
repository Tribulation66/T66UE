# Model Processing

This is the required post-TRELLIS character setup guide for `T66`.

Read this after [MASTER_WORKFLOW.md](C:/UE/T66/Model%20Generation/MASTER_WORKFLOW.md) before doing any character assembly, Blender cleanup, rigging, Unreal import, DataTable wiring, or staged visual verification.

The Trellis generation workflow explains how to get raw `.glb` outputs. This document explains what happens after a usable raw model exists.

Detailed Unreal import instructions now live in [Model Importing.md](C:/UE/T66/Model%20Generation/Model%20Importing.md). Use that file for import commands, DataTable reloads, material repair scripts, and staged executable verification.

## Current Naming

Use the front-end names for current and future work:

- `Chad` for the masculine/default body pipeline.
- `Stacy` for the feminine body pipeline.
- `Beachgoer` for the alternate outfit variant.

Some historical scripts, folders, reports, and notes still use `TypeA` or `TypeB`. Treat those names as legacy implementation names unless the current DataTable, asset path, or user-facing UI says otherwise. Do not rename old script paths casually during a quality pass; migrate names only when the affected import, redirector, DataTable, and staged-build checks are part of the same task.

## Required Source Split

Do not use one all-in character image as the production path when identity or outfit quality matters.

Use separate inputs:

- body-only image
- head-only image
- weapon-only image, when weapons are needed

Reject or reroll source images that contain:

- floor cards
- source-image panels
- background poster geometry
- duplicated body silhouettes
- off-axis heads
- visible green-screen contamination that Trellis turns into mesh
- handheld weapons already baked into the body image

For the next Chad quality pass, generate source images on opaque flat white
instead of green when testing new prompts. The white background must be a true
flat source background: no alpha, floor, cast shadow, contact shadow, gradient,
reflection, gray patch, white poster card, or border panel. If white causes
silhouette loss, reroll the prompt or garment colors rather than adding shadows.

Chad body-only images must be headless bodies with an open neck socket, not
full bodies with the head painted out. Chad head-only images must be head plus
a narrow insertable neck plug only, with no shoulders, clothing, collar, or
torso. Do not accept a head source where the neck is a full-width rectangular
column under the ears; the Mike Pass02 test showed TRELLIS turns that shape into
a rear head/neck slab. Use portrait body canvases such as `1024x1536` unless a
square source is manually accepted after side/depth review.

For Beachgoer variants, reuse the accepted standard head and generate a new Beachgoer body per hero. Keep outfits varied by hero; do not make every Beachgoer use the same trunks, vest, colors, or prop family.

## Head And Body Assembly

Use [assemble_typea_head_body.py](C:/UE/T66/Model%20Generation/Scripts/assemble_typea_head_body.py) as the current proven assembly path until it is renamed or replaced.

Validated constants from the first Chad batch:

- head height ratio: `0.245` of body height
- neck overlap: `0.045` of body height
- forward/back offset: `-0.006` of body height toward the front

Mike Pass02 used [assemble_chad_head_body_pair.py](C:/UE/T66/Model%20Generation/Scripts/assemble_chad_head_body_pair.py)
as a generic process-pass assembler. That test found the original Batch01
placement constants can leave the new plug-style heads floating above the body
socket. For the best current Mike A03 head/body pair, the better visual
placement was:

- head height ratio: `0.275`
- neck overlap: `0.155`
- forward/back offset: `-0.006`

The previous Mike A02/B01 comparison used the same head height and forward/back
offset with `0.125` neck overlap. A03/B02 won that comparison because the deeper
insertion hid the body neck tube without creating a visible rear slab.

Mike Pass02 then paused model generation on a rigging-process candidate:

- [Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb)

The Mike rig prototype keeps the A03 face/front read, raises the head by `0.034m`, and adds a
small separate rear-biased body-owned neck bridge. Treat this as a Mike-only
rigging prototype input, not a universal assembly recipe and not final art
approval. The bridge is intentionally separate process geometry; rigging should
prove whether that kind of neck fix survives weighting and placeholder motion.

The first Mike Pass02 rigging proof is documented in
[RIGGING_QA_NOTES.md](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/RIGGING_QA_NOTES.md).
It exports a Mike-only rig with idle, legs-only walk, and a right-hand sword
proxy attachment. Treat it as a process proof, not production art approval.

Do not blindly promote the A03/B02 or Mike rig prototype values to every hero. Use them as
Mike evidence for Pass02 plug-style heads, then verify front, side, back, and
close neck renders per character.

Assembly must be checked from more than one angle:

- front
- side
- oblique
- close neck view when the join is questionable

A correct assembly transform does not make a bad source mesh production-ready. If the source body has panels, cards, fused props, duplicated forms, or bad silhouette, reroll or clean the source instead of pushing the assembled model forward.

## Canonical Blender Setup

Before rigging and Unreal import, the character should be normalized in Blender, not in Unreal import settings.

Required character setup:

- full character height is `2.0` meters in Blender
- mesh origin is centered at the feet
- bottom of the visible mesh sits on ground level
- exported FBX/GLB imports to Unreal with uniform scale `1.0`
- do not use an Unreal import scale map as the production fix for size or grounding

When assembled GLBs import with placement stored on parent empties, bake the world transform into mesh data, detach the mesh from the placement empty, and remove the empty before generating weights or armature modifiers. Background-mode `transform_apply` is not reliable enough by itself for this step.

## Rigging Baseline

The current proven rigging baseline comes from:

- prototype: [rig_typea_mike_prototype.py](C:/UE/T66/Model%20Generation/Scripts/rig_typea_mike_prototype.py)
- batch path: [rig_typea_batch01.py](C:/UE/T66/Model%20Generation/Scripts/rig_typea_batch01.py)

Expected rig traits:

- one generated deform armature
- `18` bones
- rigid or semi-rigid head weighting
- procedural body weights
- disconnected-fragment stabilization for compact Trellis mesh fragments
- saved idle/action data
- exported FBX/GLB verification renders

For the next pass, rig the Mike prototype first as a prototype before any roster batch.
Because the Mike rig prototype includes a separate neck bridge, the prototype must specifically
verify head rigidity, neck bridge weighting, and no tearing at the head/body
join during the placeholder idle/action.

Set generated Blender actions to `use_fake_user = True` so non-active actions survive when the `.blend` is saved.

TRELLIS character bodies can contain thousands of disconnected fragments. Stabilize compact fragments as rigid pieces before accepting an action or walk-cycle proof. Otherwise small details can tear even when the main body seems weighted correctly.

Detailed rigging instructions now live in [Rigging Process.md](C:/UE/T66/Model%20Generation/Rigging%20Process.md). Use that file for the Mike Pass02 rigging prototype, idle/walk action requirements, neck-bridge weighting checks, and Blender screenshot iteration loop.

## Weapon Attachment Baseline

Do not generate weapons attached to the body image. Treat weapons as separate
source inputs, separate TRELLIS GLBs, and separate Blender attachment work after
the body/head mesh is acceptable.

For the Mike Pass02 process test, use a sword proxy even though Mike's final
weapon is not locked yet. This is a procedure test for hero weapon attachment:

- import the accepted character assembly
- import a separate weapon GLB
- scale the weapon relative to the normalized `2.0` meter hero
- place the grip inside the right hand volume
- rotate the blade and hilt as a real held weapon, not just as an attached prop
- attach the weapon rigidly to `hand_r`, either through bone parenting or a
  rigid `hand_r` vertex group plus the same armature modifier
- save front, side, oblique, close grip, and walk-proof renders
- export FBX/GLB and reimport at least one exported file for verification

Reject or iterate the attachment if the grip floats outside the hand, the blade
reads as beside the character instead of held, the weapon penetrates the torso
in the main views, the sword scale reads like a dagger, the hilt is not rolled
sideways with the wrist, or it fails to follow the hand during the walk action.

Current Mike process values from the accepted Blender proof:

- sword target length: `1.44` meters for a normalized `2.0` meter hero
- blade direction: `(0.035, -0.72, 0.693)` where negative Y is character-front
- hilt roll: `-90` degrees around the blade axis

## Materials And Textures

The current character material path depends on packed base-color PNGs exported by the rigging script.

After import, run the targeted material repair so:

- `M_Character_Unlit` remains the character master material
- material slots are texture-backed
- `DiffuseColorMap` is assigned
- `DiffuseColorMap.A` drives `Opacity Mask`
- masked alpha-card source geometry does not render as opaque poster panels

Texture presence alone is not enough. Verify that the material actually uses the texture and that alpha masking is still wired.

Mike Pass02 finding: the FBX import created four material instances but no
texture assets, which made the staged executable render Mike as a black
silhouette even though the mesh, skeleton, animations, and DataTable row were
correct. The fix was to extract packed Blender images from the rig scene, import
them into `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype`, create unlit
material instances, and reassign the skeletal mesh material slots.

Current repair entry points:

- [inspect_mike_rig_materials_blender.py](C:/UE/T66/Model%20Generation/Scripts/inspect_mike_rig_materials_blender.py)
- [RepairMikeRigPrototypeMaterials.py](C:/UE/T66/Scripts/RepairMikeRigPrototypeMaterials.py)

## Unreal Import And Wiring

Detailed import runbooks now live in [Model Importing.md](C:/UE/T66/Model%20Generation/Model%20Importing.md). The notes below remain the character-specific baseline.

Use the full editor wrapper for the skeletal mesh plus animation batch:

- [RunImportTypeABatch01RiggedHeroesAndExit.py](C:/UE/T66/Scripts/RunImportTypeABatch01RiggedHeroesAndExit.py)

For the Mike-only Pass02 process prototype, use:

- [RunImportMikeRigPrototypeAndExit.py](C:/UE/T66/Scripts/RunImportMikeRigPrototypeAndExit.py)
- [ImportMikeRigPrototype.py](C:/UE/T66/Scripts/ImportMikeRigPrototype.py)

This intentionally rewires only `Hero_3_Chad` for in-game testing and does not
redo the completed Batch01 roster import/wiring pass.

Known import rule:

- Do not use `UnrealEditor-Cmd.exe -run=pythonscript` for this skeletal mesh plus animation batch. UE 5.7 hit a Slate assertion in that path.
- Do not use `-ExecCmds="py path"` for the import wrapper because Unreal can treat the script path as Python source text.
- If a mesh skeleton is replaced after earlier animation imports, use a fresh animation asset suffix such as `RigIdleV2`.

After import:

- reload `DT_CharacterVisuals` from [CharacterVisuals.csv](C:/UE/T66/Content/Data/CharacterVisuals.csv)
- make sure current rows use `Chad`, `Stacy`, and `Beachgoer` naming for front-end concepts
- keep historical `TypeA` script names only where they are still the actual script paths
- do not create a separate map for each body type; the hero-selection map is shared

## Verification Gate

Do not call a character model setup done from Blender or editor checks alone.

Run [VerifyTypeABatch01HeroVisuals.py](C:/UE/T66/Scripts/VerifyTypeABatch01HeroVisuals.py) until the report proves:

- every expected default and Beachgoer row loads
- skeletal meshes load
- idle animations load
- mesh and animation skeletons match
- material slots are texture-backed
- character material alpha path is masked correctly
- mesh height is near `200` cm
- bottom Z is near `0` cm

If DataTables, imported assets, materials, maps, or cooked content changed, run a full stage with [StageStandaloneBuild.ps1](C:/UE/T66/Scripts/StageStandaloneBuild.ps1). Do not rely on `-SkipCook` for a packaging-facing verification.

For front-end hero-selection work, launch the staged executable and capture the real game screen. Confirm visually that the model is:

- visible
- textured
- not half underground
- not scaled by Unreal import workaround
- grounded at the feet
- using the expected default or Beachgoer variant

The current staged executable path is:

```text
C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Useful Mike Pass02 staged capture command pattern:

```powershell
& 'C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe' `
  -T66FrontendScreen=HeroSelection `
  -T66HeroSelectionPreviewHero=Hero_3 `
  -T66HeroSelectionBody=Chad `
  -T66AutoScreenshot='C:\UE\T66\Saved\MikeRigPrototypeUnrealCheck\MikeRigPrototype_StagedHeroSelection_Textured_QA.png' `
  -T66AutoScreenshotDelay=7 `
  -T66AutomationResX=1600 `
  -T66AutomationResY=900 `
  -T66AutomationWindowed `
  -windowed -ResX=1600 -ResY=900 -forcelogflush
```

## Art Quality Pass

The first Chad batch validated that the pipeline can produce, rig, import, wire, and package default plus Beachgoer variants. It did not make every model art-approved.

For tomorrow's quality work, judge each model on:

- silhouette readability in the hero-selection camera
- face identity
- giga-chad/nephilim body mass, with huge shoulders, chest, arms, and legs plus a thin waist
- shared Chad skull/head scale, with identity kept in hair, beard, ethnicity cues, and facial details
- head/body join
- outfit clarity
- material contrast
- unwanted card or panel geometry
- fragments that tear during the placeholder animation
- ground contact
- whether Beachgoer looks intentionally different from default

If a model fails these checks, prefer rerolling or cleaning the source and rerunning the proven process over patching the symptom in Unreal.

`model_ready` means the raw GLB exists. It is not art approval. For Chad source
rerolls, review the source PNG contact sheet first, then raw TRELLIS front/side
renders and bounds/depth metadata, then head/body assembly closeups. A near-flat
body side view or body depth below roughly `0.15m` is a source/TRELLIS failure,
not an assembly problem.

