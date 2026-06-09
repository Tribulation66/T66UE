# Original Prompt

User wants to do the rigging for the FriendSlop male model in a different chat that already has Blender open. Codex should read the current T66 rigging documents and provide a copy-ready prompt for that rigging agent, focused on making the rig work well with the current ragdoll/PhysicalAnimation direction.

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: read the live T66 rigging/animation process docs, identify the current rigging method and constraints, then produce a copy-ready prompt for a separate Blender rigging agent to rig the FriendSlop male model for the ragdoll system.
Stop condition: no code changes; deliver a grounded rigging-agent prompt plus the key constraints it is based on.

## Relevant Repo Context Already Identified

- Root process: `AGENTS.md`
- Operator/Validator protocol: `OPERATOR_VALIDATOR_PROTOCOL.md`
- Folder routers:
  - `Model Generation/MODEL_GENERATION_AGENTS.md`
  - `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md`
- Rigging/model docs:
  - `Model Generation/Rigging and Animation/README.md`
  - `Model Generation/Rigging and Animation/00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
  - `Model Generation/Rigging and Animation/01_TOOL_SETUP_INSTRUCTIONS.md`
  - `Model Generation/Rigging and Animation/03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`
  - `Model Generation/Rigging and Animation/06_MOB_ANIMATION_GUIDELINES.md`
  - `Model Generation/Rigging and Animation/07_ANIMATED_TOONSTYLE_HERO_PIPELINE_INSTRUCTIONS.md`
  - `Model Generation/Rigging and Animation/pending_issues_rigging_and_animation.md`
  - `Model Generation/Instructions/README.md`
  - `Model Generation/Instructions/00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`
  - `Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md`
  - `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
  - `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`
- Current FriendSlop male source:
  - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`
  - Textured FBX bundle: `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/RawTexturedFBX/Hero_1_Chad_Male/Hero_1_Chad_Male_Textured.fbx`
  - Source manifest says asset `Hero_1_Chad_Male`, rows `Hero_1_Chad` and `Hero_1_Chad_DemoSkin`, target dir `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw`, target height `180.0`, yaw `90.0`.
  - Export report says bounds are approx 90.2 x 52.4 x 180 cm and source vertex count 163,496.
- Current runtime row:
  - `Content/Data/CharacterVisuals.csv` maps `Hero_1_Chad` and `Hero_1_Chad_DemoSkin` to static mesh `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.SM_Hero_1_Chad_Male`, empty skeletal/animation columns, yaw 90.
- Current ragdoll prototype:
  - `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
  - TestRoom skeletal override currently uses legacy AnimatedToonStyle `SK_Hero_1_Chad` plus `PA_Hero_1_Chad_TestRoom`.
  - Ragdoll code follows a simulated spine body, disables movement/combat, restores after settle or max timeout, and has `t66.TestRoom.WipeoutArmEnablePhysicalAnimation` default off.
  - `Source/T66/Gameplay/pending_issues_Gameplay.md` says PAC is unstable because current proof has only 6 bodies / 2 constraints, no clean central pelvis chain, and enabling PAC crashed/hung.

## Ask For Validator

Independently inspect the repo read-only as needed and provide the answer/prompt you would give: what should the Blender rigging agent do so the FriendSlop male model works well with the ragdoll/PhysicalAnimation system? Call out current-doc constraints, source paths, skeleton/PhysicsAsset requirements, acceptance criteria, and common traps to avoid.
