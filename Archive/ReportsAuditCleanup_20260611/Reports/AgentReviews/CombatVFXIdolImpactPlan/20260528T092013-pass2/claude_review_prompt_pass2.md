You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXIdolImpactPlan\codex_review_packet_pass2.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Review Packet Pass 2: Combat VFX Idol Impact Plan

## Working Goal

Analyze the live T66 combat VFX/idol architecture from commit `27e148ae`, run Claude cross-review, and explain what is missing plus next steps for impact-point-driven idol Niagara effects without implementing changes.

## Output Scope To Review

Read-only architectural answer to Pablo. No implementation. The answer should state whether Codex and Claude understand the idol-overlay vision, identify missing runtime/process pieces, and name the design decisions needed before implementation.

## Reconciliation From Pass 1

Claude pass 1 returned `Verdict: REVISE` with three Major issues:

1. Whiff/no-primary-target behavior was undefined.
2. Single-impact versus per-hit overlay semantics were not explicit.
3. `Idol_Lava` as a fire explosion was not sharply separated from its current `DOT` data category.

Accepted. The revised answer below explicitly says:

- Current auto-attacks do not fire without a primary target, so the immediate AOE implementation path has no normal whiff overlay unless design later adds whiff firing. If future whiff firing exists, suppressing the overlay is the safest default until Pablo approves an attack-origin/maximum-range fallback.
- First pass should be one idol overlay at the weapon attack's primary impact context, not N overlays for every actor in `WeaponHitActors`. Per-hit overlays for bounce/pierce/secondary splash should be a later explicit rule.
- `Idol_Lava` is currently `DOT`; changing it to an AOE fire explosion is gameplay/data work and needs separate approval/proof. For first implementation without changing live data semantics, use an existing AOE idol such as `Idol_Earth`, `Idol_Water`, or `Idol_Storm`, or make Lava a visual-only overlay while retaining its existing DOT behavior.

## Current Evidence Summary

- `Gameplay/Combat/VFX_PROCESS_INDEX.md:24-30`: Hero 1 AOE has production binding; idol overlays are architecture-only with no active rows/assets.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md:11-15`: weapon VFX owns primary silhouette; idol overlay owns additive secondary layer; no temporary projectile placeholder resurrection; overlay rows need their own effect packet/proof.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md:21-29`: future binding wants weapon binding ID, idol ID/modifier ID, overlay Niagara path, material parameter set, compatible categories, stacking/priority, authority note, fallback flag, packet ID.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md:17-22`: gameplay capture, temporal evidence, combat hitbox authority, production binding, item/stat proof, and idol overlay gates apply.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md:16-25`: CSV owns binding intent, DataTable must be refreshed, production assets under `Content/VFX`, setup and validator required.
- `Content/Data/CombatVFXBindings.csv:2`: only active row is `Hero1Axe_AOE_Base`, with note `idol overlays deferred`.
- `Source/T66/Data/T66DataTypes.h:31-83`: binding schema already has `WeaponBase` and `IdolModifier`, plus single-valued `AttackCategory`, Niagara path, packet/profile metadata, suppress/fallback, scale/timing fields.
- `Source/T66/Core/T66GameInstance.cpp:916-973`: lookup matches `SourceType`, `SourceID`, and `AttackCategory`, so multiple categories currently mean multiple rows, not a list field.
- `Source/T66/Gameplay/T66CombatComponent.cpp:1244-1245`: auto-attack origin is computed locally.
- `Source/T66/Gameplay/T66CombatComponent.cpp:1365-1460`: AOE target gathering owns sphere/sector query and debug draw; this remains damage authority.
- `Source/T66/Gameplay/T66CombatComponent.cpp:1587-1607`: AOE computes `PrimaryHandle`, `SlashCenter = GetTargetAimPoint(PrimaryHandle)`, radius, forward, inner radius, target list, and weapon-base Niagara spawn. This is the closest live impact-point seam.
- `Source/T66/Gameplay/T66CombatComponent.cpp:1028-1192`: bound production VFX spawn exists only for weapon-base slash.
- `Source/T66/Gameplay/T66CombatComponent.cpp:2263-2291`: idol placeholder projectile lanes are spawned in `TryFire`; this is the gating site to suppress when a bound overlay exists.
- `Source/T66/Gameplay/T66CombatComponent.cpp:1486-1534`: `SpawnWeaponProjectileVisual` uses temporary projectile profiles and sets target actor.
- `Source/T66/Gameplay/T66CombatComponent.cpp:2366-2418`: idol payload damage/status loops actors in `WeaponHitActors`; it does not consume an impact context.
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.cpp:74-80,157-164`: current idol overlay placeholder profile is a temporary cube overlay.
- `Content/Data/Idols.csv:3,8-10`: `Idol_Lava` is `DOT`; `Idol_Earth`, `Idol_Water`, and `Idol_Storm` are `AOE`.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md:182-190,351-358`: carrier archetypes include `SupportImpact`; it is support/secondary by design and cannot replace a primary attack slash/trail/aura/path.

## Revised Proposed Answer

Codex should answer:

Yes, I understand the vision, and it lines up with the existing architecture: the weapon attack produces the authoritative combat result and primary VFX silhouette; the idol adds an additive Niagara overlay at the weapon impact point, such as an explosion burst on the impact center. That overlay should not be the damage authority unless a separate idol combat rule says so.

What's missing:

1. **A named attack impact context.** Hero 1 AOE already computes the right ingredients locally (`AttackOrigin`, `PrimaryHandle`, `SlashCenter`, `SlashForward`, radius, inner radius, target list). They need to become a small struct/event instead of local variables only. That context should carry weapon ID, hero ID, attack category, source binding ID if available, primary target handle, impact point, forward/orientation, logical radii/shape, effective scale/playback, and hit target handles/actors.

2. **A first-pass semantic rule: one overlay at the primary impact point.** For Hero 1 AOE, the first idol overlay should be one burst at the weapon attack's primary impact context, not one burst for every actor in `WeaponHitActors`. Per-hit/per-bounce/per-secondary-splash overlays are valid later, but they must be an explicit rule because they can quickly multiply visual noise and gameplay ambiguity.

3. **A whiff/no-target rule.** The current auto-attack does not fire without a primary target, so the immediate implementation path does not need a whiff overlay. If future attacks can whiff, the safe default should be to suppress idol overlays until Pablo approves a fallback point such as attack-origin plus max range.

4. **A bound idol overlay spawn path.** The binding schema already supports `SourceType=IdolModifier`, but no caller resolves idol rows. Add a `TrySpawnBoundIdolOverlayVFX` or generalized spawn helper that resolves idol binding rows, spawns at the impact context, passes scale/timing/orientation/user parameters, and logs `CombatVFXIdolOverlaySpawned`.

5. **Placeholder suppression at the current gate.** The current idol visual is the temporary projectile lane in `TryFire`/`SpawnWeaponProjectileVisual`. Once a bound idol overlay succeeds, that specific idol placeholder should be suppressed so the production Niagara overlay does not coexist with the old blue cube path.

6. **First idol selection and Lava mismatch.** The live AOE idols are `Idol_Earth`, `Idol_Water`, and `Idol_Storm`. `Idol_Lava` is currently `DOT`, so making it a fire explosion AOE is not just a VFX swap; it changes live idol combat/data semantics and needs explicit approval plus new proof. If Pablo wants fire first without gameplay changes, make Lava visual-only over its current DOT behavior or pick an existing AOE idol for the first structural pass.

7. **Binding row policy.** For a first pass, the current single-valued `AttackCategory` row is enough: create one `IdolModifier + IdolID + AOE` row. Multi-category support can be represented as multiple rows for now. Later, if many idols need compatibility/priority/material parameter sets, extend the schema with the future fields listed in `CombatVFXIdolOverlayArchitecture.md`.

8. **Process/tooling.** The first idol needs an effect packet, PPF/artifact parity/mechanism manifest, source evidence, lab asset path, production promotion path, setup-script row preservation, validator coverage for the idol row/assets/no lab dependency, and a proof harness that captures base-only and base+idol separately.

Recommended next steps:

1. Confirm the first idol target and semantics: `Idol_Earth`/`Idol_Water`/`Idol_Storm` as existing AOE, or a visual-only Lava fire overlay that does not change Lava's DOT behavior.
2. Write the first idol effect packet. Treat the overlay as a secondary/support impact effect relative to the weapon primary slash; do not let it replace the weapon silhouette.
3. Refactor Hero 1 AOE to emit/fill an attack impact context while preserving current behavior.
4. Add `IdolModifier` binding resolution and a bound overlay spawn helper.
5. Suppress the current idol placeholder lane only when a bound idol overlay actually spawns.
6. Add one active idol row, regenerate `DT_CombatVFXBindings`, extend the validator, and run base-only/base+idol capture proof.

## Specific Review Questions

1. Is this revised answer safe to present to Pablo under the T66 instructions?
2. Are there any remaining Blocker/Major objections?
3. Does the answer properly distinguish discussion from implementation, and visual overlay from damage authority?

## Verification For This Read-Only Pass

- Verified HEAD with `git rev-parse --short HEAD`: `27e148ae`.
- Read requested docs and additional required routers/docs.
- Inspected live code/data paths listed above.
- Verified `ANTHROPIC_API_KEY` is unset in Process/User/Machine before Claude review.
- No build/test/capture is required because this pass changes no runtime code/assets.

</review_packet>
