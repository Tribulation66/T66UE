# Rigging Process

This file is the focused rigging guide for post-TRELLIS `T66` character work.
Read it after [Model Processing.md](C:/UE/T66/Model%20Generation/Model%20Processing.md)
when a head/body assembly is ready for a rigging prototype.

The immediate target is the Mike Chad Pass02 rig prototype:

- [Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_RigPrototype_A03_LiftedNeckBridge.glb)

Current Mike Pass02 implementation script:

- [rig_chad_mike_process_prototype.py](C:/UE/T66/Model%20Generation/Scripts/rig_chad_mike_process_prototype.py)

Current Mike Pass02 QA notes:

- [RIGGING_QA_NOTES.md](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Rigging/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/RIGGING_QA_NOTES.md)

This is a process candidate, not production art approval.

## Current Objective

Rig the Mike prototype first, before any roster batch, with:

- one generated deform armature
- `18` bones
- rigid or semi-rigid head weighting
- neck-bridge weighting that does not tear at the head/body join
- one saved idle animation
- one saved walk animation where the legs move and bend in a walking cycle
- no requirement for arm swing in the first walking proof
- a separate sword or weapon proxy attached to the right hand as a process test
- Blender QA renders for rest, idle, walk, neck closeup, and weapon grip
- FBX/GLB exports that preserve the armature, mesh skinning, and actions

## Source Rules Before Rigging

Do not rig straight from an unreviewed TRELLIS output. The model must first pass
the head/body assembly review in Blender.

Before rigging, verify:

- the mesh is not a source-image card or poster panel
- the body is grounded
- the full character height can normalize to `2.0` meters
- the head is positioned from front, side, back, and neck closeup views
- the neck/head/body join is visually acceptable enough to test deformation
- any separate repair mesh, such as Mike's rear neck bridge, is intentionally
  part of the rig test

If the head/body join fails visually while still static, do not use rigging as
the fix. Return to source generation, assembly, or retopo cleanup.

## Blender Setup

Normalize in Blender, not through Unreal import scale.

Required setup:

- full character height: `2.0` meters
- visible mesh bottom: ground level
- origin: centered at the feet
- exported FBX/GLB import scale: `1.0`
- placement empties baked into mesh data before weighting
- old placement empties removed after baking

Important Batch01 finding: assembled GLBs can store body/head placement on
parent empties. Bake each mesh object's world transform directly into mesh data,
detach the mesh from the empty, remove the empty, and only then generate the
armature and weights. Background-mode `transform_apply` was not reliable enough
as the only bake step.

## Armature Baseline

The proven automated baseline used these script paths:

- prototype: [rig_typea_mike_prototype.py](C:/UE/T66/Model%20Generation/Scripts/rig_typea_mike_prototype.py)
- batch path: [rig_typea_batch01.py](C:/UE/T66/Model%20Generation/Scripts/rig_typea_batch01.py)

The first prototype used Mike standard from Batch01, then the batch path rigged
all completed Batch01 Chad/Beachgoer assets. Do not redo that completed
Batch01 import/wiring pass unless explicitly asked.

Expected bones:

- `root`
- `pelvis`
- `spine_01`
- `spine_02`
- `neck_01`
- `head`
- `upperarm_l`, `lowerarm_l`, `hand_l`
- `upperarm_r`, `lowerarm_r`, `hand_r`
- `thigh_l`, `calf_l`, `foot_l`
- `thigh_r`, `calf_r`, `foot_r`

Deform bones are every bone except `root`.

## Weighting Rules

Use procedural weights as the first prototype path:

- body vertices are weighted by normalized height and side of body
- head vertices are rigid to `head`, with the lowest neck portion blended
  between `neck_01` and `head`
- compact disconnected TRELLIS fragments are stabilized as rigid pieces after
  initial weights are assigned
- separate repair geometry must be intentionally weighted and checked in motion

TRELLIS character bodies can contain thousands of disconnected fragments. If
small details keep independent smoothed weights, they can tear even when the
main body looks acceptable. Stabilize compact disconnected components as rigid
pieces before accepting the rig.

For Mike Pass02, the separate rear neck bridge is the key risk. It should blend
from `neck_01` at the lower portion to `head` at the upper portion, then be
checked in side, back, and closeup renders during rest, idle, and walk poses.

## Animation Rules

Every generated Blender action must set:

```python
action.use_fake_user = True
```

Without this, Blender can drop non-active actions when the `.blend` is saved.

The current Mike process pass must create two actions:

- idle: subtle rest movement, enough to prove the rig evaluates without
  destroying the neck/head/body join
- walk: in-place lower-body walk cycle with thigh, calf, and foot movement

The first walk proof does not need arm swing. Keep arms stable so weapon
attachment can be evaluated independently from locomotion.

Recommended walk proof frames:

- frame `1`: left stride
- frame `9`: left passing pose
- frame `17`: right stride
- frame `25`: right passing pose
- frame `33`: loop back to left stride

## Weapon Attachment Prototype

Do not generate weapons attached to body images. Use a separate weapon mesh and
attach it after the character mesh is acceptable.

For the Mike rigging process test, use a sword proxy even if Mike's final weapon
is not a sword. The goal is to prove the attachment procedure:

1. import a separate sword GLB
2. normalize and scale it relative to the `2.0` meter character
3. place the grip into the right hand volume
4. weight or parent the weapon rigidly to `hand_r`
5. export the weapon together with the rigged character for process review
6. save a report describing the source weapon, hand bone, transform, and QA
   renders

Important Mike finding: inspect the source weapon orientation before choosing
the grip point. The Arthur Excalibur proxy is modeled blade-down in its local
vertical: the blade tip is near mesh min Z and the handle is near mesh max Z.
For that source, attach the upper handle region to the hand and rotate the
source local `-Z` blade direction forward/up. Do not infer the grip from the
lower 15 percent of the mesh just because the object is vertical.

The visual target is an organic held sword, not a sword merely following the
hand bone. In the side view, the blade should project forward from the
hip/hand line at roughly `45` degrees upward. For the current Mike coordinate
space, character-front is negative Y, so a working reference vector is close to
`(0.035, -0.72, 0.693)`: mostly forward and upward, with only a small outward
X bias.

Sword scale and roll matter as much as the blade pitch. The current Mike
process pass uses a sword target length of `1.44` meters for a normalized `2.0`
meter hero. The hilt is rolled `-90` degrees around the blade axis so it sits
sideways with the wrist instead of presenting as a flat bar across the hand.

The sword placement is not approved from one screenshot. Save at least:

- front rest view
- side rest view
- oblique rest view
- close weapon-grip view
- walk view proving the weapon stays attached

Reject or iterate if:

- the grip is visibly outside the hand volume
- the crossguard is not at the fist/palm area
- the pommel does not sit below or behind the fist like a held sword
- the sword reads as a dagger because it is too small for the hero scale
- the hilt/guard rotation is not sideways with the wrist
- the blade reads as floating beside the hand
- the side view does not show the blade projecting forward/up from the
  hip/hand line at about `45` degrees
- the weapon penetrates the torso in the main front/side views
- the weapon does not follow the hand bone during the walk proof
- the hand/weapon setup hides a head/body join failure
- the generated hand remains visibly open around the handle and there is no
  separate hand pose, socket adjustment, or weapon shape that makes the grip
  read organically

## Verification Loop

For each rigging attempt:

1. run the rigging script in Blender
2. export FBX and GLB
3. render deterministic QA images
4. inspect the screenshots visually
5. record findings in the rig report or process notes
6. adjust bone placement, weights, weapon transform, or animation poses
7. rerun until the prototype looks acceptable enough for the next stage

Minimum Blender checks:

- rest front, side, back
- idle front and neck closeup
- walk front and side
- weapon grip closeup
- exported-file reimport check

Do not move to Unreal import until Blender visual QA and exported-file reimport
both pass.

## Unreal Boundary

After Blender rigging is accepted:

- import through the full editor wrapper path, not `UnrealEditor-Cmd.exe -run=pythonscript`
- use import scale `1.0`
- use fresh animation asset suffixes if the skeleton changed
- export or extract packed Blender texture images before Unreal import if the
  FBX does not carry them through
- repair materials so `M_Character_Unlit` remains masked and texture-backed;
  textureless imported material instances must not be accepted because they can
  render as a black silhouette in the staged executable
- reload `DT_CharacterVisuals`
- run the existing verification script
- if packaged or staged content changed, run a full stage and inspect the
  staged executable visually

Current Mike Pass02 Unreal handoff:

- import script: [ImportMikeRigPrototype.py](C:/UE/T66/Scripts/ImportMikeRigPrototype.py)
- material repair script: [RepairMikeRigPrototypeMaterials.py](C:/UE/T66/Scripts/RepairMikeRigPrototypeMaterials.py)
- logical row wired for testing: `Hero_3_Chad`
- Unreal asset root: `/Game/Characters/Heroes/Hero_3/Chad/RigPrototype`
- imported animations:
  - `AM_Hero_3_Mike_Chad_RigPrototype_Idle`
  - `AM_Hero_3_Mike_Chad_RigPrototype_WalkLegsOnly`
- staged visual proof:
  [MikeRigPrototype_StagedHeroSelection_Textured_QA.png](C:/UE/T66/Saved/MikeRigPrototypeUnrealCheck/MikeRigPrototype_StagedHeroSelection_Textured_QA.png)

The Mike import is a process prototype for testing rigging, animations, weapon
attachment, and material handoff in-game. It is not permission to batch-rig or
batch-import the roster from this candidate.
