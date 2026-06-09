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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\HeroActiveRagdollStage3Plan\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\HeroActiveRagdollStage3Plan\codex_operator_draft_for_cross_review.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\HeroActiveRagdollStage3Plan\Claude\20260606T145004-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Okay, so what would be the next step now? This was originally stage 2, which now is stage 3, which is the hero active raglan MVP. So what does this entail? Come up with an implementation plan for this, and wait for my green light.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: Plan the next implementation stage only: Hero Active Ragdoll MVP for Hero 1 Chad on top of the Stage 2 PhysicsFirst rig/Leap foundation. No code/content edits yet.
Stop condition: Provide a repo-grounded implementation plan, include risks/proof gates, incorporate Claude's independent plan-review input, and wait for the user's green light.

Relevant repo rules:

- This is T66 work in C:\UE\T66.
- Codex is Operator, Claude is Validator.
- Use current live docs and assets, not stale pure-Chaos/PAC-off prototype assumptions.
- No implementation until user approval.
- Stage 3 must follow Gameplay/Physics ownership docs.
- The active target is capsule-backed always-on active ragdoll for Hero 1 Chad first.
- Keep UT66KnockbackComponent as legacy/prototype support until the new path proves itself.
- Runtime physics implementation will require focused compile, staged standalone validation, and Unreal-owned multi-frame capture proof.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

Stage 3 is the **Hero 1 Chad active-ragdoll MVP** built on the now-landed Stage 2 foundation (fresh PhysicsFirst rig, Idle/Walk/Jump/Leap clips, GetUp/RecoverStand clips, and the `PA_Hero_1_Chad_PhysicsFirst_Stage2Seed` 18-body/17-constraint asset). The plan should *implement the existing "Stage 3 Runtime Component Plan" in `HeroPhysicsModel.md`*, not invent a new direction. Recommended plan to hand the user for green-light:

**Goal:** Capsule-backed, always-on simulated Hero 1 Chad that wobbles in idle/walk, gets perturbed by hits, loosens (not mode-switches) on knockdown, and recovers through a ramp — proven by Unreal-owned multi-frame capture.

**Workstream A — new runtime component (additive, do not mutate KnockbackComponent):**
- `UT66HeroPhysicsComponent` + `FT66HeroPhysicsProfile` + `FT66HeroPhysicsRuntimeState` (names from the doc).
- Init always-on sim on required bodies (pelvis/spine_01-03/head/thighs/calves/feet; arms/hands by stability test) at blend weight 1.
- `UPhysicalAnimationComponent` local pose drive toward Idle/Walk/Jump/Leap.
- Kinematic hip anchor riding above the capsule + physics constraint anchor↔pelvis; mesh **not** rigidly attached to capsule during sim.
- Locomotion playrate from capsule speed; debug/log hooks for proof.

**Workstream B — states & reactions:** `Balanced / Staggered / KnockedDown / Recovering`; hits loosen anchor + pose-drive strength and apply impulse at contact location; recovery ramps stiffness back using RecoverStand/GetUp clips. Keep feel values profile-data-driven, not hardcoded (per Hard Rules).

**Workstream C — PhysicsAsset hardening:** validate the Stage2Seed under *continuous* sim (not just hit-spike): spine non-collapse, limited angular ranges, CCD on pelvis, self-collision off where it jitters, sleep disabled for the active hero.

**Proof gates (MASTER_PHYSICS Verification Policy):** focused compile; staged standalone refresh + shortcut verification when playable behavior changes; Unreal-owned video/log (no desktop screenshots); explicit FULL/PARTIAL per mechanism. **Anti-lookalike discriminator:** must show wobble *before* impact, perturbation *during* contact, tumble/rebound when loosened, and a recovery ramp — a bigger LaunchCharacter or post-impact passive ragdoll fails.

**Scope guard:** Hero 1 Chad only; no all-hero/mob migration, no Stage 4 data-profile build-out beyond the minimum profile struct, KnockbackComponent retained as legacy fallback.

**Risks to call out to the user:**
1. Stage 2 QA reported `max_influences_per_vertex=1` (rigid single-bone skinning) — joints may deform poorly once bodies simulate; confirm whether reskin is needed before/within Stage 3.
2. Stage2Seed PhysicsAsset is a *seed*, validated only for hit-ragdoll, not always-on sim — expect tuning iterations.
3. Capsule↔mesh decoupling + camera-follow regressions (prior floor/follow/camera guard lessons).
4. Naming debt (`FriendSlopRaw/PhysicsFirst` production path) is pre-existing, not a Stage 3 blocker.

Stop at the plan and wait for green light, as instructed.

## Evidence Checked
- `Gameplay/Physics/MASTER_PHYSICS.md` (target model, roadmap, verification policy, anti-lookalike).
- `Gameplay/Physics/HeroPhysicsModel.md` (has explicit Stage 3 Runtime Component Plan + names).
- `Gameplay/Physics/PHYSICS_AGENTS.md`, `PhysicsReactionProfiles.md` (states, hard rules, legacy KnockbackComponent policy).
- Stage 2 cross-review verdict `Result: OK` (`...FallGuysHeroRiggingStage2Implementation/.../claude_review_pass3.md`).
- Landed deliverables on disk: PhysicsFirst rig/clips/manifest and `Content/.../FriendSlopRaw/PhysicsFirst/` (incl. GetUp/RecoverStand + `PA_..._Stage2Seed`).
- `T66HeroBase.h/.cpp` — no PAC/HeroPhysics component yet (Stage 3 is greenfield).

## Questions Or Blockers
None require the user to *decide* before the plan can be written — the green-light is the normal stop condition, not a blocker. Codex should fold the four risks above (especially the single-influence-skinning question) into the plan it presents.

## Caveats
- Stage 2 PASS claims (compile, import/physics-asset JSON, Leap MP4) are taken on the prior reviewer's word; not re-opened here.
- The Stage2Seed body/constraint counts and skinning figures come from reports, not independent re-measurement.
- Plan assumes Stage 2 content is committed/stable; working tree shows large content deletions/mods — confirm the foundation isn't mid-flux before coding begins.

</validator_independent_answer>

<codex_draft>
# Codex Operator Draft: Hero Active Ragdoll Stage 3 Plan

## Task

Plan the next implementation stage only: Hero Active Ragdoll MVP for Hero 1 Chad, now Stage 3 after the PhysicsFirst rigging/animation stage. Wait for user green light before implementation.

## Current Repo-Grounded Basis

- `Gameplay/Physics/README.md` makes Hero 1 Chad the first active-ragdoll target and identifies Stage 3 as unimplemented.
- `Gameplay/Physics/MASTER_PHYSICS.md` defines the target as capsule-backed always-on active ragdoll with local PAC pose drive, kinematic hip/pelvis anchor, impact loosen, impulse/rebound, and recovery ramp.
- `Gameplay/Physics/HeroPhysicsModel.md` already names the intended runtime component shape: `UT66HeroPhysicsComponent`, `FT66HeroPhysicsProfile`, `FT66HeroPhysicsRuntimeState`, and states `Balanced`, `Staggered`, `KnockedDown`, `Recovering`.
- `Gameplay/Physics/PhysicsAssetPipeline.md` says the Stage 2 PhysicsAsset is a seed and requires runtime proof before it is accepted for always-on active-ragdoll play.
- `Gameplay/Physics/PHYSICS_AGENTS.md` says old pure-Chaos/PAC-off work is historical and `UT66KnockbackComponent` must remain a legacy fallback until the new path proves itself.

## Proposed Stage 3 Goal

Create the first playable Hero 1 Chad active-ragdoll path in the real game, using TestRoom's wipeout arm as the first proof obstacle. The MVP is not a standalone prototype: it should be a runtime physics layer that can become the standard hero physics model.

Stage 3 is complete only if Hero 1 Chad:

1. Runs with an always-simulated skeletal body under capsule-authoritative locomotion.
2. Uses Physical Animation Component pose drive in local simulation, not a hit-only detached ragdoll.
3. Has a kinematic hip/pelvis anchor constrained to the simulated pelvis so the body wobbles, rebounds, and stays recoverable.
4. Reacts to the existing TestRoom wipeout arm through the new active-ragdoll reaction path, with legacy knockback still available as fallback.
5. Recovers from a stagger/knockdown through a stiffness/anchor ramp rather than snap-back.
6. Produces Unreal-owned video/log proof in a staged standalone build.

## Implementation Plan

### 0. Preflight And Asset Verification

- Confirm the Stage 2 Hero 1 Chad PhysicsFirst skeletal mesh, animation clips, and seed PhysicsAsset are the current runtime targets.
- Inspect body names, pelvis/root mapping, collision bodies, constraints, mass distribution, and whether the Stage 2 single-influence weighting makes active simulation unreadable.
- Treat the current PhysicsAsset as a seed. Tune it if necessary, but do not reroute back to ToonStyle, AccuRig, or the older FriendSlop raw rigging approach.

### 1. Add The Runtime Component Boundary

- Add a dedicated source boundary for the new owned runtime physics layer, preferably `Source/T66/Gameplay/Physics/`.
- Implement `UT66HeroPhysicsComponent` with a small `FT66HeroPhysicsProfile` and `ET66HeroPhysicsRuntimeState`.
- Attach it to `AT66HeroBase`, expose a getter, and keep `UT66KnockbackComponent` intact as fallback.
- Keep the first profile in C++ defaults or narrow config only; full DataTable-driven reaction profiles belong to the later reaction-profile stage.

### 2. Bring Up Always-On Simulated Hero Body

- On Hero 1 Chad initialization, bind the component to the active skeletal mesh and PhysicsAsset.
- Simulate the required bodies continuously, with physics blend weight at 1 for the active body set.
- Configure collision, damping, sleep behavior, and substep-friendly settings so the body remains active and readable during movement.
- Bind `UPhysicalAnimationComponent` to the skeletal mesh and apply local pose-drive settings below the pelvis/root body set.

### 3. Add Hip/Pelvis Anchor Ownership

- Create a hidden kinematic primitive anchor that rides with the capsule/root at the intended hip location.
- Add a physics constraint between the anchor and the skeletal mesh pelvis body.
- Tune linear/angular limits and drive strength so the capsule remains authoritative while the mesh can lag, wobble, lean, rebound, and recover.
- Add divergence guards and logging for pelvis-vs-capsule distance so failures are visible instead of silent.

### 4. Build The First Runtime State Machine

- Implement `Balanced`, `Staggered`, `KnockedDown`, and `Recovering` in the new component.
- Balanced: normal locomotion with always-on wobble.
- Staggered: short loosened pose drive/anchor after moderate impacts, partial control retained.
- KnockedDown: stronger loosened drive plus larger impulse response, temporary control/combat suppression.
- Recovering: ramp pose drive and anchor stiffness back to balanced, ideally using the Stage 2 get-up/recover animation poses as the authored recovery target.

### 5. Bridge TestRoom Wipeout Arm To The New Path

- In the TestRoom obstacle hit path, prefer `UT66HeroPhysicsComponent::ApplyPhysicsReaction(...)` for Hero 1 Chad when the active-ragdoll component is enabled and initialized.
- Fall back to `UT66KnockbackComponent::ApplyKnockbackLaunch(...)` if the new component is disabled, not ready, or the actor is not the Stage 3 target.
- Use a narrow CVar or profile flag for rollback during tuning.

### 6. Movement, Input, Camera, And Leap Integration

- Keep the capsule as the movement authority; the active ragdoll affects readability and reaction, not navigation ownership in this MVP.
- Feed state into existing movement/input suppression only where required for `KnockedDown` and recovery.
- Keep walk/jump/leap animation playback driven by capsule state, while PAC and the hip anchor supply the physical wobble and impact response.
- Check camera follow against capsule/mesh divergence so the view does not snap to a thrown mesh.

### 7. Proof Harness And Verification

- Add or extend an Unreal-owned proof mode for active-ragdoll capture instead of relying on desktop screenshots.
- Capture at least three proof beats: balanced walk/idle wobble before impact, wipeout-arm contact with loosen/impulse/rebound, and recovery back to controllable state.
- Log state, drive strength, anchor strength, pelvis/capsule distance, body simulation status, and recovery timers.
- Run focused compile, staged standalone refresh, shortcut target verification, and standalone smoke/video proof.

## Acceptance Gates

The MVP should be reported as FULL only if all required mechanisms are present with multi-frame evidence:

- Always-on body simulation before impact.
- Local PAC pose drive.
- Hip/pelvis anchor constraint.
- Impact loosen plus impulse/rebound.
- Recovery/get-up stiffness ramp.
- Capsule-authoritative locomotion preserved.
- Unreal-owned staged standalone video/log proof.

Anything missing should be reported as PARTIAL, not complete.

## Out Of Scope For Stage 3

- Migrating all heroes, mobs, bosses, or all obstacles.
- Building the full reaction profile/data-table system.
- Removing `UT66KnockbackComponent`.
- Renaming transitional `FriendSlopRaw` or `PhysicsFirst` paths unless a path blocks runtime binding.
- Final skinning polish for all animations unless Hero 1 deformation makes the proof unreadable.
- Broad Leap/Dash/Roll compatibility cleanup beyond what is necessary for the active-ragdoll state checks.

## Main Risks

- The Stage 2 single-influence skinning may be acceptable for proof but may look too rigid under continuous simulation.
- The seed PhysicsAsset may need meaningful constraint/body tuning before it can support the anchor without collapse.
- Capsule/mesh divergence can break camera readability or floor contact if not guarded.
- Reusing the old knockback component too aggressively would recreate the old hit-only ragdoll path instead of the new always-on active-ragdoll method.

## Green-Light Boundary

No implementation should start until the user approves this Stage 3 plan.

</codex_draft>
