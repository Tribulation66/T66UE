# Original Prompt

User asks: "Can you handle it from here and do what you said above?"

The prior recommendation was to proceed to Unreal: import the FriendSlop Hero 1 male skeletal FBX as an isolated experimental asset, create/tune a PhysicsAsset with meaningful pelvis/spine/limb bodies, point only the TestRoom skeletal override at it, re-enable PAC if stable, and capture wipeout-arm impact/recovery proof while preserving the normal static `Hero_1_Chad` runtime rows.

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: implement the Unreal-side FriendSlop Hero 1 skeletal import, PhysicsAsset creation/tuning, TestRoom-only override swap, PAC validation, and capture proof, without changing normal Hero_1_Chad runtime rows until proven.
Stop condition: imported experimental skeletal asset exists, TestRoom uses it only for the ragdoll test path, verification is run, and remaining caveats are reported.

## Repo Rules / PPF

- Do not use native goal tools.
- Codex is Operator, Claude is Validator.
- Relevant docs:
  - `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
  - `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`
  - `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`
  - `Gameplay/GAMEPLAY_AGENTS.md`
  - `Gameplay/README.md`
- FriendSlop raw visual identity must be preserved. No ToonStyle/QuadRetro/tint/outline processing.
- Normal `Content/Data/CharacterVisuals.csv` rows should remain static unless/until proof is accepted.
- If playable content changes, staged standalone verification is required.

PPF:
- Objective: take the Blender FriendSlop male rig into Unreal and validate it in the TestRoom ragdoll/PAC spike.
- Proven process: T66 Unreal import/validation docs plus FriendSlop raw import rules and new raw humanoid rigging process; TestRoom ragdoll pending issue defines the PAC proof gate.
- Planned implementation: import skeletal FBX as isolated experimental asset, create/tune PhysicsAsset from new pelvis/spine/limb hierarchy, point only TestRoom override at it, capture wipeout-arm proof.
- Same method class: YES.

## Current Inputs

- Skeletal FBX:
  `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging/Hero_1_Chad_Male_FriendSlop_Skeletal.fbx`
- Rig QA/report:
  `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging/Hero_1_Chad_Male_FriendSlop_Rig_QA.json`
  `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/Rigging/Hero_1_Chad_Male_FriendSlop_Rig_Report.md`
- Texture folder:
  `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/RawTexturedFBX/Hero_1_Chad_Male/Textures/`
- Rig facts:
  - final visual front +X
  - 23 deform bones
  - real pelvis plus three-spine chain
  - 0 unweighted, 0 over four influences, max influences 3
  - relaxed arms-down rest pose, physics-grade not animation-grade

## Current Ragdoll/PAC Context

- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp` currently uses legacy AnimatedToonStyle `SK_Hero_1_Chad` for TestRoom skeletal override.
- `Source/T66/Gameplay/pending_issues_Gameplay.md` says PAC is off because current generated PhysicsAsset is unstable: only 6 bodies/2 constraints, no central pelvis chain, PAC crashed/hung.

## Ask For Validator

Independently inspect the repo read-only as needed and provide implementation guidance/risk review before Codex edits: where to import, how to create/verify PhysicsAsset, what code should change, what should not change, and what proof gates matter.
