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
