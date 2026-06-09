Result: OK

## Independent Answer

This turn is planning/analysis only, and the models can produce a complete, repo-grounded answer without user intervention. The user has explicitly reserved the go/no-go decision for *after* reviewing the proposed solution — that is the normal next step in this contract, not a blocker. Below is the answer I would give.

**1. What the new T66 hero rigging/animation standard should be (Fall Guys-like, physics-first, obstacle-driven)**

The standard must invert the old ToonStyle assumption that animation *is* the motion. Here the authored clips are only a **pose target for a physical animation drive** — `MASTER_PHYSICS.md` (lines 27-41) already states the runtime model: a PAC pose drive keeps the simulated mesh near an authored locomotion pose while a kinematic hip/pelvis world-anchor constraint keeps it playable. So the rigging/animation standard should be defined by what the PhysicsAsset and PAC need, not by hand-keyed expressiveness. Concretely:

- **Skeleton:** the deform-only, PhysicsAsset-ready hierarchy already proven in the rig report (pelvis under root, three nonzero spine bones, distinct head/arm/leg chains, no helper/leaf bones). That part of the existing work is sound and should be promoted as the skeletal contract.
- **Pose authoring:** simple, readable, slightly exaggerated, low-frequency poses (a Fall Guys body reads as a wobbling blob — limbs are secondary). Clips are pose anchors, so they should be stable loops with clear centers of mass, authored against a **clean A/T-pose rig**, not the relaxed low-arm spike stance.
- **Clip set:** Idle, Walk, Jump, Leap, plus get-up/recovery poses if they help the PAC recovery ramp. **Run is removed.** **Roll is replaced by Leap.** Forward-leap is explicitly out per the user.
- **Recovery poses** matter more here than in a normal rig because `MASTER_PHYSICS.md` calls for a "physical recovery/get-up ramp" — get-up poses feed that, so they are worth doing.

**2. What is wrong with adopting the current FriendSlop doc + existing Hero 1 outputs as-is**

- `13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` is genuinely a *raw-import / PhysicsAsset-structure* process doc. It is not contaminated with ToonStyle/outline/tint baking (it explicitly forbids that). Its real gap is that it is silent on the **physics-first, pose-drive purpose** and on the **A/T-pose requirement for animation** — it even permits exporting the relaxed low-arm stance and dense topology as a spike. That is the philosophy mismatch: it documents "get a riggable mesh," not "build the production animation/pose-drive standard."
- The Hero 1 Chad rig report **self-identifies as a deterministic spike, not a production rig** (line 95): coordinate-region weights, no retopo, dense topology, relaxed non-A-pose, shoulder/hip polish explicitly deferred. Promoting it verbatim to "the standard for all future heroes" would canonize the spike compromises.
- The animation manifest ships Idle/Walk/Jump/**Roll** (Quaternius-derived). It has no Leap and includes Run-adjacent/Roll content the user is cutting.

So: the *skeleton + PhysicsAsset gate* are reusable; the *weights, rest pose, topology, and clip set* are not production-grade and must not be frozen as the standard without a cleanup pass.

**3. Roadmap revision**

Matches the user's instruction. Proposed for `MASTER_PHYSICS.md` Stage Roadmap (currently lines 90-96, where Stage 2 is the active-ragdoll MVP):

1. Stage 1: ownership layer (unchanged).
2. **Stage 2 (new):** define the Fall Guys-like rigging/animation standard and implement it on Hero 1 Chad — clean rig, Idle/Walk/Jump/Leap (+get-up if useful), wired and set as Hero 1's model. No physics components yet.
3. **Stage 3:** Hero active-ragdoll MVP (physics components: keep capsule, mesh always simulatable, pelvis body, hip/pelvis constraint, PAC). *(was Stage 2)*
4. Stage 4: data-authored physics reaction profiles. *(was 3)*
5. Stage 5: obstacle/environment integration. *(was 4)*
6. Stage 6: performance proof + cheaper mob variants. *(was 5)*

**4. Documents/files to replace or update (after approval only)**

- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` — rewrite to the physics-first, A/T-pose, pose-drive standard (this is the "Francois folder" instruction file the user means; confirm naming — see blockers).
- `Gameplay/Physics/MASTER_PHYSICS.md` — Stage Roadmap section.
- `.../AnimationSources/friendslop_raw_humanoid_animation_sources_manifest.json` — new clip set (drop Roll, add Leap, optional get-up).
- The Hero 1 Chad rig report / rig outputs — regenerate as production rather than spike, OR keep the spike as historical and add a new production run.
- `Content/Data/CharacterVisuals.csv` — the schema has **no Leap field**; the columns are `…JumpAnimation, RollAnimation, …` shared by every hero/companion row. Adding Leap is a **cross-cutting schema change** (struct `T66DataTypes.h` + every consuming row), not a single edit.
- Runtime: `T66HeroBase`, `T66HeroMovementComponent`, `T66PlayerController_Movement`, `T66DataTypes.h`, `T66CharacterVisualSubsystem` all reference Roll. Replacing Roll→Leap is a later runtime/data pass, confirmed by grep.

**5. In/out of scope for the next implementation pass**

- **In:** the rewritten standard doc; a clean production Hero 1 Chad rig (A/T-pose, weight cleanup, retopo if bend proofs demand it); Idle/Walk/Jump/Leap clips (+get-up if useful); wiring Hero 1 to the new mesh/clips.
- **Out / deferred to Stage 3+:** physics components (pelvis body, hip constraint, PAC, always-simulatable mesh); reaction profiles; obstacle integration.
- **Decision the user must make before/at implementation:** whether Leap replaces the Roll column (rename + runtime rename of `RollForward`/`TryRollForward`) or is added as a new field. This determines the size of the runtime/data pass.

## Evidence Checked

- `Gameplay/Physics/MASTER_PHYSICS.md` — target feel, two-support runtime model, anti-lookalike rules, current Stage Roadmap (Stage 2 = active-ragdoll).
- `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md` — full read; it is raw-import/PhysicsAsset focused and not ToonStyle-baked, but silent on pose-drive purpose and permits spike compromises.
- Hero 1 Chad rig report — self-labeled deterministic spike (line 95), coordinate weights, relaxed pose, no retopo.
- Animation manifest — Idle/Walk/Jump/Roll, Quaternius-derived, no Leap.
- `Content/Data/CharacterVisuals.csv` — Hero_1_Chad uses FriendSlop SK + Walk/Idle/Jump/Roll; column header has no Leap; Roll is a shared column across all rows.
- Grep of `Source/T66` — Roll/RollForward/TryRollForward present in 10 files; no Leap symbols exist yet.

## Questions Or Blockers

None that block *this planning turn*. Two items the user decides at go-ahead (not now):
- The "Francois folder" the user referenced does not literally appear; the in-repo instruction file is `13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`. Codex should confirm that is the file the user means before rewriting it.
- Leap as Roll-rename vs new field — sizes the later runtime pass.

## Caveats

- I did not open Blender or inspect the actual FBX/weights; rig quality claims rely on the rig report's own admissions, which are explicit enough to trust for planning.
- The CSV was read partially (rows 1-52 of 138); the Hero_1_Chad row and column header — the only rows that matter here — were fully seen.
- "Get-up poses if useful" remains a judgment call best confirmed against the PAC recovery-ramp design once Stage 3 is detailed; recommending them in Stage 2 is reasonable but not mandatory.
