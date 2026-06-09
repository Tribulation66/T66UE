# Codex Draft For Cross-Review

## Proposed Answer

The user correction is right: the roadmap should put the asset standard before the active-ragdoll component. The game cannot get a good Fall Guys-like active ragdoll from a rig/clip set that was authored as a normal humanoid animation bridge. The active-ragdoll component will only be as good as the skeleton, weights, rest pose, pose-target clips, and PhysicsAsset affordances it drives.

## Roadmap Revision

1. Stage 1: Physics ownership and routing. Complete.
2. Stage 2: Physics-first hero rigging and animation standard, implemented on Hero 1 Chad and wired as the in-game Hero 1 model. This produces the production asset foundation only: rig, mesh, Idle, Walk, Jump, Leap, optional get-up/recovery clips, import, data wiring, and gameplay proof that Hero 1 uses the new asset.
3. Stage 3: Hero active-ragdoll MVP. Keep capsule, make mesh always simulatable, use pelvis/hip constraint, PhysicalAnimationComponent pose drive, profile-driven loosen/recover states.
4. Stage 4: Data-authored physics reaction profiles.
5. Stage 5: Obstacle/environment integration beyond the first proof obstacle.
6. Stage 6: performance proof and cheaper variants for mobs/other actors.

## New Rigging Standard

The standard should be called the physics-first hero rig, not raw humanoid rigging. The goal is not retargeting library animations or making a normal animated hero. The goal is a deform skeleton and clip set that can be physically driven.

Mechanism target:

- The visible hero should read like a chunky elastic body, not a realistic biped.
- The capsule remains the gameplay mover in Stage 3.
- Animation clips are pose targets for a physical animation drive, not the whole motion.
- The skeleton exists to give PhysicsAsset bodies stable roles: dominant pelvis/torso mass, light arms/legs, clear spine chain, no helper junk.
- The active physical body should be able to wobble, get hit, tumble, and recover without visible snapping.

Skeleton:

- Keep the useful part of the existing spike: one deform skeleton, root with pelvis child, three spine bones, head, clavicles/arms, thighs/calves/feet/ball bones.
- Keep UE-friendly naming where it reduces importer/runtime friction.
- Do not add fingers, twist bones, control bones, leaf bones, or mannequin-retarget complexity for MVP.
- Define physics roles separately from bone names: `pelvis` as follow/recovery body, spine bodies as the bean-like torso mass, arms/legs as secondary stabilizers, feet as contact/rebound helpers.
- The PhysicsAsset should approximate a simple two-lobed bean/snowman mass through large pelvis/torso bodies even though the visible skeleton is humanoid.

Rest pose:

- Replace the current relaxed low-arm spike stance with a production neutral pose.
- Preferred rest pose is a soft A-pose with arms separated enough for shoulder weights and body collision, but not a broad destructive T-pose.
- If the fused costume/coat mesh cannot survive a clean soft A-pose, do local cleanup/segmentation/retopo instead of broad-transforming the whole fused mesh.

Weights/topology:

- Current coordinate-region weights are acceptable evidence for a spike, not for the standard.
- Stage 2 should clean shoulder, hip, neck, and coat-panel weights enough for authored pose targets and active-ragdoll deformation.
- Max four influences, normalized weights, no unweighted vertices, no root-only or pelvis-only blobs remain hard gates.
- Retopo is not mandatory up front, but if bend proof shows topology-driven tearing at shoulder/hip/knee, Stage 2 should either retopo those regions or stop and report that the source mesh is not production-ready.

Animation:

- Clip set: `Idle`, `Walk`, `Jump`, `Leap`, and recommended `GetUp_Back`/`GetUp_Front` or a single `RecoverStand` if the team wants the smallest set.
- No `Run` clip for this MVP.
- `Roll` should be removed as a concept. A leap is not a roll animation with a new label.
- All clips should be in-place/no root motion for Stage 2 so capsule movement and later active-ragdoll anchoring own world displacement.
- Motion should be low-frequency and readable: big center-of-mass shifts, mild arm counter-swing, soft knee compression, fast hop/leap anticipation, short airborne tuck, quick recovery.
- Avoid high-frequency realistic limb motion because it will fight the Stage 3 pose drive.

## Why Current Hero 1 Outputs Are Not The New Standard

The current Hero 1 Chad rig report is useful but self-limits as a deterministic spike. It has clean hierarchy and QA, which should be preserved, but it also says:

- relaxed low-arm stance was kept because forcing A-pose would distort fused costume geometry;
- coordinate-region weights are a spike compromise;
- shoulder/hip polish should be reviewed before final authored animation;
- it is not a hand-polished production animation rig.

The current animation manifest uses Quaternius-derived roles and exports `Idle`, `Walk`, `Jump`, and `Roll`. That directly conflicts with the new standard.

So: reuse the source GLB, useful hierarchy, material preservation lessons, and QA checklist. Do not reuse the current rig/animation outputs as the accepted future-hero standard.

## Runtime/Data Wiring Recommendation

Because the user wants roll removed and leap to replace it, the clean standard is to rename the concept in code/data:

- `RollAnimation` becomes `LeapAnimation`.
- `RollForward()` / `TryRollForward()` become `Leap()` / `TryLeap()`, with deprecated wrappers only if needed for compatibility.
- `CharacterVisuals.csv` gains/uses `LeapAnimation` instead of `RollAnimation`.
- Hero movement QA and input labels should talk about leap, not roll.

The cheap MVP alternative is to put the Leap animation into the old `RollAnimation` column and keep calling `RollForward()`, but I do not recommend that. It bakes the old mental model into the new standard and will confuse future agents.

## Stage 2 Implementation Scope After Approval

In scope:

- Rewrite the current FriendSlop/raw humanoid rigging instructions into a physics-first hero rigging and animation standard.
- Update Rigging and Animation routing docs so this process is no longer treated as out-of-scope manual one-off work.
- Update Physics docs roadmap so active-ragdoll component moves to Stage 3.
- Build or rebuild Hero 1 Chad in Blender from the raw GLB or a clean derivative: production neutral pose, cleaned weights, clips, proof renders, rig report.
- Export skeletal FBX and animation FBXs.
- Import into Unreal and wire Hero 1 to the new skeletal mesh and `Idle/Walk/Jump/Leap` assets.
- Update data/code schema for Leap, replacing Roll cleanly.
- Run current proof: focused compile, DataTable import/reload, Hero Movement QA capture, and staged standalone if the playable runtime asset is changed.

Out of scope:

- Stage 3 active-ragdoll component/PAC/hip constraint implementation.
- Full obstacle reaction profile system.
- All heroes/companions migration.
- Monster physics variants.
- Deleting old assets before reference audit.

## PPF / Process Gate

PPF CHECK
Objective: Create the production Hero 1 Chad rigging/animation foundation for a Fall Guys-like physics-first hero.
Proven process: New T66 physics-first hero rigging standard to be written from `Gameplay/Physics` direction plus current Hero 1 Chad source evidence; current `13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` is source evidence but not sufficient as the owning process.
My planned implementation: After approval, replace/rewrite the rigging standard, rebuild Hero 1 Chad rig and clips, import and wire Hero 1 as a game asset, then defer active-ragdoll runtime mechanics to Stage 3.
Same method class: YES for the proposed new process; NO if someone simply uses the current spike rig/Quaternius clips or hides Leap inside Roll forever.
If NO, why: The old method treats animation as normal gameplay clips and the current rig as a PhysicsAsset spike. The new method treats clips as pose-drive targets for a physically simulated hero.
User approval required before proceeding: YES.
Verification evidence: Blender rig QA, exported hierarchy/weights, animation proof, Unreal import proof, DataTable/code proof for Leap, Hero movement capture, staged standalone when playable content changes.

ARTIFACT PARITY GATE
Reference artifact/category: Fall Guys-like playable hero feel, implemented as T66-owned physics-first rig/animation foundation.
Role: Primary.
Required: YES.
Planned artifact/path: Hero 1 Chad production rig and animation set under the FriendSlop Hero 1 run, imported to `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw` or a clearly versioned successor path.
Status: EQUIVALENT, not SAME.
Evidence: T66 cannot copy hidden Fall Guys implementation values; it should replicate the method class: simple chunky body, elastic physical mass, authored pose targets, fast jump/leap/recovery, physics-ready skeleton.

MECHANISM MANIFEST
Reference/source: Public Fall Guys mechanics descriptions and T66 Physics direction.
Required mechanisms:
  1. Mechanism: chunky elastic body read
     Required: YES
     Planned implementation: large stable pelvis/torso mass roles, soft A-pose, low-frequency pose targets
     Evidence needed: rig/animation proof and later PhysicsAsset body report
  2. Mechanism: pose-target animation, not animation-only motion
     Required: YES
     Planned implementation: in-place Idle/Walk/Jump/Leap clips authored to drive Stage 3 PAC
     Evidence needed: animation FBX proof and Stage 3 PAC capture
  3. Mechanism: no roll, leap as the ability concept
     Required: YES
     Planned implementation: Leap clip plus runtime/data rename from Roll to Leap
     Evidence needed: code/data grep and Hero Movement QA
  4. Mechanism: fast recovery target
     Required: YES
     Planned implementation: get-up/recover pose clips in Stage 2; recovery drive in Stage 3
     Evidence needed: clips now, recovery capture later
