# Mike TRELLIS Test - 2026-05-04

This is a process-build test for `Hero_3` Mike Chad Standard. It does not
replace the already wired Batch01 Chad model in Unreal.

## Inputs

- Accepted body source:
  [Hero_3_Mike_Chad_Standard_BodyOnly_White.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/approved_seed_images/Hero_3_Mike_Chad_Standard_BodyOnly_White.png)
- Original accepted head source:
  [Hero_3_Mike_Chad_Standard_HeadOnly_White.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/approved_seed_images/Hero_3_Mike_Chad_Standard_HeadOnly_White.png)
- Deterministic head preprocess that produced the first better model:
  [Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A02.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/preprocessed/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A02.png)
- Deterministic head preprocess that produced the best current assembly:
  [Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A03.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/preprocessed/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A03.png)
- Source audit:
  [Mike_SourceAudit_Preprocessed.json](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/Mike_SourceAudit_Preprocessed.json)

## TRELLIS Settings

- Seed: `1337`
- Texture size: `2048`
- Body decimation target: `80000`
- Head decimation target: `120000`
- Server endpoint: `POST /generate` with raw PNG bytes and headers
  `X-Seed`, `X-Texture-Size`, and `X-Decimation`

Run log:
[trellis_mike_pass02.log](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Notes/trellis_mike_pass02.log)

## Raw Body Result

Output:
[Hero_3_Mike_Chad_Standard_BodyOnly_White_S1337_D80000_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Raw/Trellis/Hero_3_Mike_Chad_Standard_BodyOnly_White_S1337_D80000_Trellis2.glb)

QA renders:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Body_Raw_Front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Body_Raw_Right.png)
- [metadata](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Body_Raw.json)

Status: body passes the raw TRELLIS gate for this process test. It has usable
side depth (`0.287m`), no visible card/panel/floor fragment, strong chest and
shoulders, and a usable neck socket.

## Raw Head Iterations

Original accepted head source output:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_S1337_D120000_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Raw/Trellis/Hero_3_Mike_Chad_Standard_HeadOnly_White_S1337_D120000_Trellis2.glb)

Original raw QA:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_Raw_Front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_Raw_Right.png)
- [metadata](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_Raw.json)

Status: rejected for production. The broad source neck produced a dark rear
head/neck slab that remained visible after assembly.

Neck-narrow A02 output:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A02_S1337_D120000_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Raw/Trellis/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A02_S1337_D120000_Trellis2.glb)

A02 raw QA:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A02_Raw_Front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A02_Raw_Right.png)
- [metadata](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A02_Raw.json)

Status: first accepted plug-style head. The neck plug is much better and the
GLB size dropped from `7.05 MB` to `4.96 MB`. The side view still has a chunky
rear head/hair mass, so future head prompts should reduce rectangular side/back
hair and avoid full-width neck columns.

Neck-narrow A03 output:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A03_S1337_D120000_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Raw/Trellis/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckNarrow_A03_S1337_D120000_Trellis2.glb)

A03 raw QA:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A03_Raw_Front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A03_Raw_Right.png)
- [metadata](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckNarrow_A03_Raw.json)

Status: best current head/body connection source. A03 improves the side neck
panels and rear plug shape versus A02. It also over-tapers the lower face and
neck slightly, so the process lesson is not "make the neck more triangular";
the lesson is "make a narrow insertable neck plug natively in the source image."

## Assembly Iterations

Original-head assembly:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly/Hero_3_Mike_Chad_Standard_White_Assembled_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly/Hero_3_Mike_Chad_Standard_White_Assembled_right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly/Hero_3_Mike_Chad_Standard_White_Assembled_back.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly/Hero_3_Mike_Chad_Standard_White_Assembled_neck_closeup.png)

Status: rejected. The source neck failure remains visible as a black slab and
the head/body join reads as disconnected.

A02 default placement:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_right.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_neck_closeup.png)

Status: improved head geometry, but the default Batch01 placement leaves the
head floating above the body neck tube.

A02 B01 placement:

- Output GLB:
  [Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01.glb)
- Assembly report:
  [head_body_assembly_report_A02_B01.json](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/head_body_assembly_report_A02_B01.json)
- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01_right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01_back.png)
- [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01_left.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A02_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A02_Assembled_B01_neck_closeup.png)

Placement:

- Head height ratio: `0.275`
- Neck overlap ratio: `0.125`
- Head Y offset ratio: `-0.006`

Status: good comparison candidate, no longer the best connection. The front
silhouette, body mass, and head/body placement are strong, but the side/back
head mass is chunkier than A03 and the neck transition reads less cleanly.

A03 B01 placement:

- Output GLB:
  [Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01.glb)
- Assembly report:
  [head_body_assembly_report_A03_B01.json](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/head_body_assembly_report_A03_B01.json)
- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01_right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01_back.png)
- [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01_left.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B01/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B01_neck_closeup.png)

Placement:

- Head height ratio: `0.275`
- Neck overlap ratio: `0.125`
- Head Y offset ratio: `-0.006`

Status: useful intermediate. A03 improves the side and rear plug compared with
A02, but this overlap still leaves the join slightly high.

A03 B02 placement:

- Output GLB:
  [Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02.glb)
- Assembly report:
  [head_body_assembly_report_A03_B02.json](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/head_body_assembly_report_A03_B02.json)
- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02_right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02_back.png)
- [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02_left.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B02/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B02_neck_closeup.png)

Placement:

- Head height ratio: `0.275`
- Neck overlap ratio: `0.155`
- Head Y offset ratio: `-0.006`

Status: previous best Mike head/body baseline before the lifted-neck tests. The neck is inserted deeply
enough to hide the body tube, the front silhouette remains readable, and the
side/back views do not show the severe rear slab that failed the original head.
This is the candidate to preserve as the A03 comparison baseline.
Residual art issues remain: the jaw/neck surface is still very planar, the rear
hair has a visible scalp patch, and the back of the vest has noisy texture/mesh
artifacts. Those are not connection blockers, but they are not final art
approval.

A03 B03 placement:

- Output GLB:
  [Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/HeadBody/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03.glb)
- Assembly report:
  [head_body_assembly_report_A03_B03.json](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Assembly/head_body_assembly_report_A03_B03.json)
- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B03/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03_front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B03/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03_right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B03/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03_back.png)
- [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B03/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03_left.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_B03/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_B03_neck_closeup.png)

Placement:

- Head height ratio: `0.285`
- Neck overlap ratio: `0.155`
- Head Y offset ratio: `-0.006`

Status: rejected for now. The larger head scale makes the face/neck read less
natural than B02 without improving the join.

## A03 Blender-Side Repair Attempts

After visual review, the A03 B01 placement was kept as the better visible-neck
base, but the rear scalp/bald artifact remained. Several Blender-side repairs
were tested before generating a new head source:

- C12 texture-only scalp repair:
  [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C12_VisibleNeck_SoftTextureScalpRepair/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C12_VisibleNeck_SoftTextureScalpRepair_neck_closeup.png),
  [hair back closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C12_VisibleNeck_SoftTextureScalpRepair/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C12_VisibleNeck_SoftTextureScalpRepair_hair_back_closeup.png)
- C13 body-owned neck plus rear hair cards:
  [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C13_BodyNeck_RearHairCards/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C13_BodyNeck_RearHairCards_neck_closeup.png),
  [hair back closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C13_BodyNeck_RearHairCards/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C13_BodyNeck_RearHairCards_hair_back_closeup.png)
- C15 rear hair material assignment:
  [hair back closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C15_VisibleNeck_RearHairMaterial/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C15_VisibleNeck_RearHairMaterial_hair_back_closeup.png)
- C16 duplicated top hair geometry:
  [hair back closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/HeadBodyAssembly_A03_C16_VisibleNeck_DuplicateTopHairBack/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C16_VisibleNeck_DuplicateTopHairBack_hair_back_closeup.png)

Status: rejected as a final direction. These tests either left the bald spot
visible, created a fake painted patch, produced a flat helmet back, or added
obvious floating hair geometry. The useful process finding is that the rear
hair/crown issue should be solved at the source image/TRELLIS pass, not as a
late Blender patch.

## A04 Full-Hair Head Source

Attempt04 source:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_Attempt04_Source.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/approved_seed_images/Hero_3_Mike_Chad_Standard_HeadOnly_White_Attempt04_Source.png)

Preprocessed source:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckFullHair_A04.png](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Inputs/preprocessed/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckFullHair_A04.png)

Raw TRELLIS output:
[Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckFullHair_A04_S1337_D120000_Trellis2.glb](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Raw/Trellis/Hero_3_Mike_Chad_Standard_HeadOnly_White_NeckFullHair_A04_S1337_D120000_Trellis2.glb)

Raw QA:

- [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckFullHair_A04_Raw_Front.png)
- [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckFullHair_A04_Raw_Right.png)
- [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckFullHair_A04_Raw_Back.png)
- [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckFullHair_A04_Raw_Left.png)
- [metadata](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Review/RawGLB_QA/Hero_3_Mike_Chad_Head_NeckFullHair_A04_Raw.json)

Status: useful but not perfect. The full-hair prompt removes the obvious rear
bald/crown hole that A03 inferred. The generated neck is too long and the face
is less sharp than A03, so this head requires a larger scale plus deeper neck
burial in assembly.

Rejected full-hair buried-neck placement:

- Output GLB:
  [Hero_3_Mike_Chad_Rejected_FullHair_BuriedNeck.glb](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Assembly/HeadBody/Hero_3_Mike_Chad_Rejected_FullHair_BuriedNeck.glb)
- Assembly report:
  [head_body_assembly_report_Mike_Chad_Rejected_FullHair_BuriedNeck.json](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Assembly/head_body_assembly_report_Mike_Chad_Rejected_FullHair_BuriedNeck.json)
- [front](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/Mike_Chad_Rejected_FullHair_BuriedNeck/Mike_Chad_Rejected_FullHair_BuriedNeck_front.png)
- [right](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/Mike_Chad_Rejected_FullHair_BuriedNeck/Mike_Chad_Rejected_FullHair_BuriedNeck_right.png)
- [back](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/Mike_Chad_Rejected_FullHair_BuriedNeck/Mike_Chad_Rejected_FullHair_BuriedNeck_back.png)
- [left](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/Mike_Chad_Rejected_FullHair_BuriedNeck/Mike_Chad_Rejected_FullHair_BuriedNeck_left.png)
- [neck closeup](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/Mike_Chad_Rejected_FullHair_BuriedNeck/Mike_Chad_Rejected_FullHair_BuriedNeck_neck_closeup.png)

Placement:

- Head height ratio: `0.34`
- Neck overlap ratio: `0.19`
- Head Y offset ratio: `-0.006`

Status: rejected after visual review. It is still useful evidence because the
full-hair source removed the obvious rear bald/crown hole, but the face regressed
from A03 and the neck required too much burial. Do not use this rejected full-hair buried-neck attempt as the next
rigging source.

## A03 Neck Join Iteration After A04 Rejection

The follow-up goal was to preserve the sharper A03 face/front read while solving
the side-view no-neck issue. These tests used A03 B02 as the base.

A04 cut-neck graft tests:

- C25 A04 side/back cut graft:
  [front](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C25_A04CutNeckGraft/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C25_A04CutNeckGraft_front.png),
  [right](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C25_A04CutNeckGraft/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C25_A04CutNeckGraft_right.png),
  [neck closeup](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C25_A04CutNeckGraft/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C25_A04CutNeckGraft_neck_closeup.png)

Status: rejected as a primary solution. Aggressive cutting avoids the A04
mouth/chin contamination, but the remaining graft is too rear-biased and reads
as a small dark chunk rather than a real neck. Broader A04 grafts brought in
A04 cheek/chin texture and damaged the A03 face read.

Procedural lifted-head neck tests:

- Mike rig prototype: A03 lifted-neck bridge:
  [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_front.png),
  [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_right.png),
  [back](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_back.png),
  [left](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_left.png),
  [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_RigPrototype_A03_LiftedNeckBridge/Mike_Chad_RigPrototype_A03_LiftedNeckBridge_neck_closeup.png)
- Wider-neck comparison: A03 lifted-neck bridge:
  [front](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge_front.png),
  [right](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge_right.png),
  [neck closeup](C:/UE/T66/Model%20Generation/Runs/Heroes/Chad_Pass02_ProcessBuild/Renders/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge/Mike_Chad_Comparison_A03_WiderLiftedNeckBridge_neck_closeup.png)

Status: current best process direction, not production approval. The Mike rig prototype keeps the
A03 face/front read cleanest and improves the side profile by lifting the head
slightly and adding a small rear-biased body-owned neck. The wider-neck comparison is a useful
comparison with a little more neck mass. Both still expose a process issue: a
flat procedural neck material is only a review aid. The production source
should generate a real textured neck on the head or body, not depend on this
separate flat bridge.

Body neck-socket carve tests:

- C25 lifted neck socket:
  [front](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C25_LiftedNeckSocket/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C25_LiftedNeckSocket_front.png),
  [right](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C25_LiftedNeckSocket/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C25_LiftedNeckSocket_right.png)
- C26 broader lifted neck socket:
  [front](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C26_LiftedNeckSocket/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C26_LiftedNeckSocket_front.png),
  [right](C:/UE/T66/Model%20Generation/Archive/Mike_NeckJoin_Rejected_20260504/Renders/HeadBodyAssembly_A03_C26_LiftedNeckSocket/Hero_3_Mike_Chad_Standard_White_NeckNarrow_A03_Assembled_C26_LiftedNeckSocket_right.png)

Status: rejected. Cutting the existing body collar/vest geometry reveals jagged
mesh and texture artifacts from the source body. The body socket must be correct
in the source/TRELLIS body or repaired with deliberate retopo/cleanup, not by a
blind bounding-box delete after assembly.

## Process Findings

- Flat white source images worked for Mike body generation and avoided the green
  card/panel artifacts seen in Batch01.
- The body prompt direction is viable: huge upper body, thin waist, forward
  boots, and no floor mesh.
- Head sources need an additional neck-width gate. "Head plus neck" is not
  specific enough; the neck must be a narrow insertion plug.
- TRELLIS head failures can improve without changing seed/model settings by
  preprocessing the source silhouette before rerunning TRELLIS.
- Do not try to solve rear-head bald/crown artifacts with late Blender patches
  unless the patch is intended as a temporary review marker. The cleaner fix is
  a full-hair source image and a fresh TRELLIS head.
- New plug-style heads may need a small upward head placement plus a real neck
  volume rather than only deeper burial. A03 B02 remains the sharper-face
  baseline; The Mike rig prototype and wider-neck comparison show the current best side-profile direction, but their
  procedural neck bridge is only process evidence.
- The "generate a thick neck block, then cut it in Blender" idea is only partly
  useful. A04 proved a fuller hair/neck source can remove the rear bald patch,
  but cutting its neck onto A03 imported texture/shape problems unless it was
  cut so aggressively that it stopped reading as a neck.
- Blindly carving the body collar/vest after assembly is risky. It exposed
  jagged mesh and texture artifacts on Mike. Future bodies should either be
  generated with a clean neck socket/open collar, or get a deliberate retopo
  pass before rigging.
- Do not call Mike production-ready yet. This test produced a better process and
  a better candidate, but it has not gone through rigging, Unreal import, or
  staged executable verification.

