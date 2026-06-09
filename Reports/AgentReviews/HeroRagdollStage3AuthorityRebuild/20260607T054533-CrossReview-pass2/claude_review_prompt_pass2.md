You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\HeroRagdollStage3AuthorityRebuild\claude_independent_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\HeroRagdollStage3AuthorityRebuild\codex_cross_review_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\HeroRagdollStage3AuthorityRebuild\20260607T033509-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:

Okay, so go ahead and rebuild Stage 3 around the authority model you and Claude agree on, so that we have the correct system, and then update all the docs to remove the old outdated information on how we used to do it with the new process, and then we'll test out with this new infrastructure that the two of you built.

Working task:
Operator: Codex
Validator: Claude
Scope: Rebuild Stage 3 Hero 1 active-ragdoll around a single authority model, update stale physics docs to describe the new process, and verify the new infrastructure in the playable/TestRoom path.
Stop condition: Code/docs are changed, focused verification is run, and Codex reports what passed, what failed, and what still needs tuning.

Repo rules:
- Read C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Treat this as a read-only independent answer. Do not edit files.
- Current operator is Codex; validator is Claude.
- Physics owner docs are under Gameplay/Physics, especially PHYSICS_AGENTS.md, MASTER_PHYSICS.md, HeroPhysicsModel.md, PhysicsReactionProfiles.md, and PhysicsAssetPipeline.md.
- Movement/trap docs may be relevant: Gameplay/Movement/MASTER_MOVEMENT.md and Gameplay/Traps/MASTER_TRAPS.md.

Current reported failure to solve:
- Prior flattening was reduced, but normal movement now stretches/spazzes as if the mesh is being pulled.
- The wipeout arm barely moves the character and does not feel like active ragdoll physics.
- The user wants a comprehensive authority-model rebuild, not another narrow tuning patch.

Architecture to review:
- Capsule remains gameplay/input/navigation authority.
- Skeletal mesh has an always-on simulated pelvis/central body for active ragdoll.
- A kinematic hip anchor follows the capsule.
- A physics constraint connects hip anchor to simulated pelvis.
- Physical Animation Component acts as muscle/pose target drive, not the gameplay authority.
- Obstacles apply physical reaction profiles: body impulse at hit/contact location, temporary loosening of pose/anchor, bounded capsule shove for gameplay displacement.
- Actor-to-pelvis teleport/follow loops should not be the normal authority path.

Please provide:
1. Independent implementation plan for this Stage 3 rebuild, repo-grounded.
2. The top risks or code traps Codex should avoid.
3. Any doc updates that must happen in the same change.
4. Focused verification you expect before calling the pass done.

</original_prompt>

<validator_independent_answer>
I have enough grounding to produce the independent answer. This is an architecture/plan answer the models can resolve internally; no user-only decision is blocking.

Result: OK

## Independent Answer

The current `T66HeroPhysicsComponent` has the authority inverted from the agreed model, which directly explains both symptoms. Two competing authority paths fight each other:

1. **Pelvis is kinematic, not simulated.** `ConfigureMeshPhysics` sets `ComponentTransformIsKinematic` + `SkipAllBones` and calls `SetAllBodiesBelowSimulatePhysics(SimulationRootBodyName, true, /*bIncludeSelf=*/false)`. The `false` leaves the pelvis/root **kinematic** while only children simulate. But the agreed architecture requires an **always-on simulated pelvis** with a **separate kinematic hip anchor** following the capsule.
2. **Wipeout barely moves the hero** because `ApplyPhysicsReaction` does `AddImpulse` on `PelvisBodyName` (`T66HeroPhysicsComponent.cpp:200`) — a kinematic body ignores impulses. The reaction also overwrites `Movement->Velocity` on the capsule (`:209`), so the visible motion is whatever the capsule does, not ragdoll physics.
3. **Normal-movement stretch/spazz** comes from the kinematic root teleporting with the capsule each frame while PAC local drive pulls simulated children toward animation targets, plus the `ResyncPelvisBodyToActor` / `FollowActorToPelvisBody` teleport loops (`:696`, `:776`) that snap bodies and even the actor every tick. Those teleport loops are precisely the "actor-to-pelvis follow loops should not be the normal authority path" the prompt rules out.

### Rebuild plan (repo-grounded)
1. **Invert body ownership.** Make the pelvis a simulated body at blend weight 1 (`SetAllBodiesBelowSimulatePhysics(Pelvis, true, /*bIncludeSelf=*/true)` or simulate pelvis explicitly). Drop `ComponentTransformIsKinematic`/`SkipAllBones` in favor of a mesh that follows simulation (`SimulationUpatesComponentTransform` style) since the pelvis is now the simulated source.
2. **Add a real kinematic hip anchor.** Keep the existing `HipAnchorComponent` (sphere attached to capsule) but make it the only thing that tracks the capsule. The capsule stays gameplay/nav authority; the anchor is its kinematic proxy.
3. **Constraint anchor→simulated pelvis.** `ConfigureHipAnchorConstraint` already constrains `(Mesh, PelvisBodyName) ↔ HipAnchor`; this becomes meaningful only once the pelvis actually simulates. Tune linear/angular drive (the existing `AnchorLinear*`/`AnchorAngular*` profile fields) so the capsule "carries" the pelvis without hard-locking it.
4. **PAC as muscle only.** Keep `bIsLocalSimulation = true` child pose drive (`:445-463`) but ensure it never drives the pelvis as the gameplay authority — pose drive shapes the body, the anchor constraint positions it.
5. **Delete the teleport/follow loops** as the normal path. Remove `FollowActorToPelvisBody` from the per-tick state machine and demote `ResyncPelvisBodyToActor` to a bounded safety-only recovery (the `MaxPelvisCapsuleDistance` guard), not a steady-state mechanism.
6. **Reaction routes to simulated bodies.** `ApplyPhysicsReaction` should `AddImpulse` to the now-simulated pelvis and bodies below at the hit location, temporarily lower pose/anchor multipliers (the existing stagger/knockdown multipliers), and apply only a **bounded capsule shove** for gameplay displacement — not overwrite capsule velocity outright.
7. **Keep `UT66KnockbackComponent` as legacy fallback** (Hard Rule) until the rebuilt path passes proof.

### Top risks / traps for Codex
- **Half-inversion**: simulating the pelvis but leaving `ComponentTransformIsKinematic` or the teleport loops in place will reproduce the same fight. The two authority models must not coexist.
- **Impulse on kinematic body** silently no-ops — verify the pelvis is actually simulating before trusting reaction logs.
- **Constraint frame/limits**: a too-stiff anchor drive re-creates the rigid "pulled mesh" feel; too loose and the hero floats away from the capsule. This is the main tuning axis.
- **Mesh transform update mode** must match the new owner (simulation-driven), or you get double-application of capsule + simulation.
- **Hardcoded feel values**: keep everything in `FT66HeroPhysicsProfile`/data, per Hard Rule "prefer data-authored physics profiles."
- **Hero-1-Chad gating** (`ShouldEnableForHero` requires mesh name contains `Hero_1_Chad` + `PhysicsFirst`) — confirm the TestRoom hero actually matches or the rebuild silently never initializes.

### Doc updates required in the same change
- `Gameplay/Physics/HeroPhysicsModel.md` — rewrite the "Stage 3 Runtime Component Plan" to describe simulated-pelvis + kinematic-anchor authority and explicitly retire the kinematic-pelvis/actor-follow approach.
- `Gameplay/Physics/MASTER_PHYSICS.md` and `PhysicsReactionProfiles.md` — update reaction model (impulse to simulated bodies, pose/anchor loosening, bounded capsule shove) and remove stale teleport/follow-loop description.
- `PhysicsAssetPipeline.md` — confirm pelvis-simulated/CCD/mass-dominant gates still hold under always-on simulation.
- Check `Gameplay/Movement/MASTER_MOVEMENT.md` and `Gameplay/Traps/MASTER_TRAPS.md` for any references to the old "capsule velocity overwrite" reaction and align them.
- Record any out-of-scope findings in `Source/T66/Gameplay/Physics/pending_issues_Physics.md`.

### Focused verification before calling the pass done
- Focused compile of the T66 target.
- TestRoom direct-entry run with `t66.HeroPhysics.DebugLog 1`: confirm `Init OK` with **pelvis simulated** (not kinematic), and that `PelvisCapsuleDist` stays bounded during normal movement (no growing divergence / resync spam).
- Normal locomotion capture (Unreal-owned, multi-frame) showing **no stretch/spazz**.
- Wipeout/obstacle hit capture showing the body actually reacting (impulse visible on simulated bodies), then recovering through Staggered→Recovering→Balanced.
- Staged standalone refresh + shortcut verification since playable behavior changes (Hard Rule + AGENTS staged-build process).
- Report passed / failed / still-needs-tuning per the stop condition, with log + frame evidence, not desktop screenshots.

## Evidence Checked
- `AGENTS.md` (process router, staged-build and Operator/Validator rules).
- `Gameplay/Physics/PHYSICS_AGENTS.md`, `HeroPhysicsModel.md` (owner docs, Hard Rules, Stage 3 plan).
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h` / `.cpp` (full current implementation — authority config, reaction path, teleport/follow loops).
- Invocation sites: `T66HeroBase.cpp`, `T66GameMode_TestRoom.cpp`, `T66PlayerController_Overlays.cpp` (not deep-read; confirms wiring points exist).

## Questions Or Blockers
None that require the user. The authority model is already specified in the prompt and matches the owner docs; Codex can implement and verify internally. (Not deep-read, so Codex should confirm during implementation: the exact reaction call sites in `T66HeroBase.cpp` / TestRoom, and that the live TestRoom hero mesh name satisfies the `Hero_1_Chad`+`PhysicsFirst` gate.)

## Caveats
- Read-only pass; I did not compile or run. Symptom diagnosis is inferred from code reading, but the kinematic-pelvis-vs-simulated-children conflict and impulse-on-kinematic-body no-op are strongly supported by the source.
- I did not read every prior review report under `Reports/AgentReviews/HeroRagdoll*`; if the "agreed authority model" recorded there differs in any detail from the prompt's architecture block, the prompt and owner docs should win, but Codex should reconcile.
- Tuning values (anchor stiffness, impulse scale) are feel-dependent and will need iteration with capture proof; the plan fixes the architecture, not the final numbers.

</validator_independent_answer>

<codex_draft>
# Codex Draft For Cross-Review - Hero Active-Ragdoll Stage 3 Authority Rebuild

## Task Contract

Operator: Codex
Validator: Claude

Scope: Rebuild Stage 3 Hero 1 active-ragdoll around the agreed authority model, update stale physics docs/process notes, and verify the new infrastructure in the playable/TestRoom path.

Stop condition: Code/docs changed, focused verification run, and final report states what passed, what failed, and what remains tuning work.

## Implemented Authority Model

- `UT66HeroPhysicsComponent` is the Hero 1 Stage 3 owner.
- Capsule remains gameplay/input/navigation authority.
- Skeletal mesh component transform is kinematic.
- Chaos simulates pelvis/body chain below the component.
- Hip anchor constraint binds simulated pelvis to capsule authority.
- Physical Animation Component is local child-body pose muscle, not gameplay root authority.
- Obstacle reaction applies mass-scaled simulated-body impulse plus bounded capsule shove.
- Old normal actor-to-pelvis follow and `SimulationUpdatesComponentTransform` root authority are avoided.
- Runtime distance debug now resolves effective pelvis location by comparing raw body coordinates against component-transformed coordinates to avoid false origin-divergence/flattening diagnosis.

## Main Files Changed

- `C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.h`
- `C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.cpp`
- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_TestRoom.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66KnockbackComponent.h`
- `C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1`
- `C:\UE\T66\Gameplay\Physics\README.md`
- `C:\UE\T66\Gameplay\Physics\MASTER_PHYSICS.md`
- `C:\UE\T66\Gameplay\Physics\HeroPhysicsModel.md`
- `C:\UE\T66\Gameplay\Physics\PhysicsAssetPipeline.md`
- `C:\UE\T66\Gameplay\Traps\MASTER_TRAPS.md`
- `C:\UE\T66\Scripts\README.md`
- `C:\UE\T66\Source\T66\Gameplay\Physics\pending_issues_Physics.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\stage3_authority_rebuild_proof_summary.md`

## Verification

- Focused editor build passed:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- Staged standalone refreshed successfully:
  - `C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
  - Final staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Both `C:\UE\T66\T66 Standalone.lnk` and pinned taskbar shortcut target that exe.
- Unreal-owned proof video:
  - `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\heroactiveragdollproof_reaction_testroom_final_framed.mp4`
  - `ffprobe`: 1280x720, 16 fps, 6.0s, 96 frames.
- Final proof frames:
  - `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\frames_reaction_testroom_final_framed\`
- Final proof log gates in `C:\UE\T66\Saved\Logs\T66.log`:
  - `-T66AutomationTestRoom` and `-T66AutoCaptureHeroHPOverride=20000` were injected by `heroactiveragdollproof`.
  - TestRoom direct entry active.
  - Wipeout arm scheduled.
  - `Reaction Applied=1 Source=TestRoomWipeoutArm`.
  - `ActiveTried=1 ActiveApplied=1 LegacyApplied=0`.
  - TestRoom legacy PAC fields now log as `LegacyProfilePAC=0 LegacyDriveMode=0`.
  - State transitions include `Balanced -> KnockedDown`, `Recovering`, and return to `Balanced`.
  - Effective `PelvisCapsuleDist` remains bounded in samples despite raw body distance reading high due component-relative body readback.
  - No `PelvisCapsuleDistance exceeded`, active-ragdoll resync, `PlayerDied`, or fatal markers were found in the final proof scan.

## Known Remaining Risk

This completes the Stage 3 authority/infrastructure rebuild, not final Fall-Guys-like feel tuning. The remaining work is design tuning: PhysicsAsset body shapes/mass/inertia, hip-anchor stiffness/limits, pose-drive strength, rebound, get-up timing, and proof camera framing.

## Draft Final Position

The Stage 3 authority model is rebuilt and verified enough to test in the game. The old flattening/origin-divergence class is addressed by keeping capsule authority, kinematic component transform, simulated pelvis anchored to the capsule, local child-body PAC, bounded capsule shove, and effective pelvis-distance diagnostics. The docs now route future work through `Gameplay/Physics` and the `heroactiveragdollproof` proof path. The result is stable infrastructure, not a final feel pass.

</codex_draft>
