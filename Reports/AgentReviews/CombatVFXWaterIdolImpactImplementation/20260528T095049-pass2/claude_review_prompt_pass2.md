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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactImplementation\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Review Packet: Water Idol Impact-Source Infrastructure

## Working Goal

Plan and, after review, implement the all-weapons/all-idols impact-source infrastructure in T66 so idols have their own damage source and impact point, with Water idol tested via a simple blue-sphere placeholder triggered from the Hero 1 AOE slash impact, without final Niagara authoring yet.

## Output Scope To Review

Implementation plan before edits. Review whether the proposed runtime structure, proof route, and temporary-placeholder boundary are safe under the T66 process docs. If approved, Codex will proceed to runtime/source edits, focused build, and Unreal-owned proof capture.

## User Constraints And Current Clarifications

- Pablo clarified that each idol should be its own damage source and should have its own impact point.
- Idol impact points do not need to drive downstream chaining yet, but the structure should preserve that future option.
- Use `Idol_Water`, not fire/Lava, for the first test.
- Use a simple blue sphere as a temporary Water idol proof visual; the final Water idol Niagara effect is a later pass.
- The infrastructure should be all-encompassing enough for all weapon categories and all idol categories, while this pass only visually proves Water AOE.
- Do not include Mini/minigame scope.

## Applicable Instructions And Process Docs

- `AGENTS.md`: working goal, folder routing, Claude review, PPF/artifact parity/mechanism manifest, Unreal-owned capture, no process substitution.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat VFX work routes through `Gameplay/Combat/CombatVFXAuthoringProcedure.md`; runtime-facing gameplay changes need compile/build verification and staged standalone validation when affecting playable standalone.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`: Hero 1 AOE production binding exists; idol overlays are architecture-only and need proof before active rows/assets.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: combat proof must separate VFX presentation from hitbox/damage authority and use Unreal-owned capture.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`: weapon VFX owns the primary silhouette; idol overlay is additive secondary presentation; current active overlay rows/assets are absent.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`: production binding rows must come from CSV/DataTable refresh and production assets under `Content/VFX`; this pass is not adding a production Water Niagara row.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: final Niagara authoring still requires real artifact/mechanism gates; the blue sphere cannot be treated as a production idol effect.
- `Gameplay/Combat/MASTER_COMBAT.md`: maintain after material combat/targeting/damage/VFX changes.
- `Reports/AGENTS.md`: review packets under `Reports/AgentReviews`; proof artifacts under `Reports/Proof`.

## Current Live Evidence

- `Source/T66/Gameplay/T66CombatComponent.cpp` computes `AttackOrigin`, target handles, AOE `SlashCenter`, `SlashForward`, effective slash radius/inner radius, and `SlashTargets` inside `TryFire`.
- `Source/T66/Gameplay/T66CombatComponent.cpp` still stores weapon hits as `TArray<AActor*> WeaponHitActors`; idol payloads later loop these actors and use `Hit->GetActorLocation()` for idol specials.
- `ApplyDamageToTargetHandle` already accepts `SourceID`; existing idol damage passes `IdolID`, so provenance exists but is not tied to an idol impact context.
- `Source/T66/Data/T66DataTypes.h` already has `ET66CombatVFXBindingSourceType::IdolModifier` and `FIdolData` says each idol is an independent attack source.
- `Content/Data/Idols.csv` has `Idol_Water` as `Category=AOE`, `BaseDamage=8`, `BaseProperty=200`, `AoeDelay=0.15`, and `AoeRadius=300`.
- Current live `Category=AOE` idol rows are `Idol_Earth`, `Idol_Water`, and `Idol_Storm`. Only `Idol_Water` is in scope for the target-query behavior change in this pass.
- `Content/Data/CombatVFXBindings.csv` has only `Hero1Axe_AOE_Base`; its note says idol overlays are deferred.
- `Source/T66/Gameplay/T66TemporaryProjectileSystem.*` provides the existing temporary blue projectile shapes; current idol overlay projectile is a blue cube attached to the projectile lane.
- `Source/T66/Gameplay/T66CombatVFX.cpp` has legacy/imported idol VFX helpers, including a Water pack path, but these are not the reviewed Water Niagara workflow and should not be promoted as the answer to this structural pass.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` has existing `hero1axeaoehitbox` and `hero1axeaoevfxbinding` proof modes that equip Hero 1 black AOE, stage fixed proof targets, fire real combat, and log HP deltas.
- `Scripts/CaptureT66GameplayVideo.ps1` already supports those proof modes, evidence bundles, debug view, and ffmpeg/ffprobe output.

## PPF CHECK

Objective:
Create the shared impact-source structure and prove Water idol follows the Hero 1 AOE slash impact point with separate idol-source damage.

Proven process:
`AGENTS.md` Section 2 plus the Combat VFX process docs listed above. For this pass, the method class is runtime combat/VFX infrastructure proof, not final Niagara visual authoring.

My planned implementation:
Refactor combat firing to produce named weapon impact contexts and derive named idol impact contexts from them. Keep damage authority in combat queries and `ApplyDamageToTargetHandle`. Use a temporary blue sphere as a structural Water-idol impact marker only, and log it as a placeholder. Do not add a production Water Niagara row or claim final Water visual completion.

Same method class: YES for structural infrastructure and proof; PARTIAL for final idol VFX visuals because final Niagara authoring is explicitly deferred by Pablo.

If NO, why:
Not NO for the requested structure. The deferred final Water Niagara is outside this pass and will still require the full Niagara artifact/mechanism process.

User approval required before proceeding: NO, because Pablo explicitly said to go ahead with the structure and temporary Water proof.

Verification evidence:
Focused C++ build, updated proof mode, Unreal-owned capture/evidence bundle, log excerpts showing weapon impact context, Water idol impact context, placeholder sphere spawn, `SourceID=Idol_Water` damage, and expected target HP deltas.

## ARTIFACT PARITY GATE

Reference artifact/category:
Water idol impact/overlay, temporary structure proof.

Role: Secondary

Required: YES for this structural proof.

Planned artifact/path:
Runtime-spawned blue static-mesh sphere at the Water idol impact point, spawned only as a placeholder/proof marker from the combat component.

Status: EQUIVALENT for structure proof, DEFERRED for final Niagara.

Evidence:
Proof capture and logs will show the sphere at the slash impact point. The logs must name it as a placeholder and must not create or activate a production binding row.

Reference artifact/category:
Final Water idol Niagara burst.

Role: Primary for the later Water VFX pass.

Required: NO for this pass because Pablo deferred final Niagara.

Planned artifact/path:
Deferred. Future production path should live under `Content/VFX/...` and be bound through `CombatVFXBindings.csv` / `DT_CombatVFXBindings`.

Status: DEFERRED

Evidence:
This plan explicitly avoids claiming final Water idol visual parity.

## MECHANISM MANIFEST

Reference/source:
Runtime impact-source architecture from live `T66CombatComponent` plus `CombatVFXIdolOverlayArchitecture.md`.

Required mechanisms:

1. Mechanism: weapon impact context creation
   Required: YES
   Planned implementation: Add a small `FT66CombatImpactContext` in `T66CombatComponent.h` carrying source type, source ID, hero ID, attack category, attack origin, impact point, forward vector, primary target handle, hit target handles, effective damage, and optional parent/chain data. Shape fields are category-tagged: AOE populates radius/inner radius/sector metadata; Pierce populates line length/tube radius; Bounce populates hop/chain index; DOT populates anchor/follow target only. Fill one or more contexts from Pierce, AOE, Bounce, and DOT paths without changing weapon damage behavior.
   Evidence needed: Verbose proof log lines such as `CombatImpactContext SourceType=WeaponBase SourceID=Hero_1_black_aoe AttackCategory=AOE ImpactPoint=... HitTargets=...`.

2. Mechanism: idol impact context derivation
   Required: YES
   Planned implementation: For every equipped valid idol, derive an `IdolModifier` impact context from the selected weapon impact context. For the first rule, use the primary weapon impact context as the idol trigger point; store parent weapon source ID and keep room for future per-hit/per-chain contexts.
   Evidence needed: Proof log line such as `CombatImpactContext SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ImpactPoint=...`.

3. Mechanism: idol-owned damage source
   Required: YES
   Planned implementation: Keep `SourceID=IdolID` when applying idol damage. For `Idol_Water` only, replace the old `WeaponHitActors` target source with target handles gathered by a Water-owned AOE sphere centered at the idol impact point. For `Idol_Earth`, `Idol_Storm`, and all non-AOE idols, this pass must be gameplay-observable neutral: keep the current target/damage behavior and add only context/logging plumbing unless a later reviewed proof expands their behavior.
   Evidence needed: proof log/damage totals showing `Idol_Water` separately from `AutoAttack`, and HP deltas on expected Water-hit targets.

4. Mechanism: temporary Water placeholder at impact
   Required: YES for structural proof
   Planned implementation: Add a helper that spawns a short-lived, collisionless, blue static-mesh sphere at the Water idol impact point when no production `IdolModifier` binding exists. The mesh source is the engine primitive sphere already exposed through `FT66VisualUtil::GetBasicShapeSphere`; no new asset is created. It must log `CombatVFXIdolImpactPlaceholderSpawned` and not write production assets.
   Evidence needed: Unreal-owned video/contact sheet and log line with `SourceID=Idol_Water`.

5. Mechanism: production binding future seam
   Required: YES structurally
   Planned implementation: Add `TrySpawnBoundIdolImpactVFX` or equivalent helper using existing `ResolveCombatVFXBinding(IdolModifier, IdolID, IdolCategory, ...)`. If a real row exists later, it should spawn the production Niagara at the idol impact context and suppress the temporary placeholder for that idol.
   Evidence needed: Source inspection/build for the seam. No active Water row is expected in this pass.

6. Mechanism: proof-mode equipment and capture
   Required: YES
   Planned implementation: Add a non-shipping proof mode such as `hero1axeaoewateridolimpact` that snapshots current equipped idols/tier values, equips Hero 1 black AOE, temporarily equips `Idol_Water`, stages fixed targets around the slash impact point, fires one real combat attack, then logs weapon damage, Water damage, placeholder spawn, and target pass/fail results. After proof logging, restore the previous idol state via the idol manager so an editor session is clean even though the normal capture process exits. Extend `CaptureT66GameplayVideo.ps1` and likely add a wrapper script for this mode.
   Evidence needed: Unreal-owned capture, evidence bundle, log excerpt, ffprobe metadata.

## Anti-Lookalike Rule

Cheapest wrong result:
A blue sphere that appears near the target while idol damage is still just a loop over `WeaponHitActors`, with no separate idol impact context and no proof that `Idol_Water` damage came from its own source.

Discriminator:
The proof must show the sequence: weapon impact context created at the slash impact point, derived Water idol impact context created at the same point, Water AOE target query/damage applied from that context with `SourceID=Idol_Water`, and the placeholder sphere spawned at that context. The sphere alone is not acceptance.

## Planned Edit Scope

- `Source/T66/Gameplay/T66CombatComponent.h`
  - Add the private impact context struct and helper declarations.
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - Fill weapon impact contexts in Pierce/AOE/Bounce/DOT paths.
  - Replace the AOE idol `WeaponHitActors` target source with context-driven AOE target query/damage for `Idol_Water` only.
  - Preserve existing gameplay-observable behavior for `Idol_Earth`, `Idol_Storm`, Pierce, Bounce, and DOT idols; this pass may add context/logging for them, but not change their target selection, damage timing, damage amount, or status application.
  - Suppress the old temporary idol projectile lane only for `Idol_Water` when the same attack will use Water impact presentation, either a future bound `IdolModifier` Niagara or the temporary Water sphere placeholder. Other idols keep the existing projectile-lane presentation unless they get their own reviewed impact presentation.
  - Add impact logging behind a CVar to avoid normal hot-path log spam.
- `Source/T66/Gameplay/T66CombatVFX.cpp`
  - Add bound idol impact VFX and temporary Water sphere placeholder helper implementation.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Add proof mode that equips Water idol and logs source-separated proof data.
- `Scripts/CaptureT66GameplayVideo.ps1`
  - Recognize the Water proof mode for debug view and proof args.
- Optional `Scripts/RunHero1AxeAOEWaterIdolImpactProof.ps1`
  - Wrapper for capture/evidence/log excerpt.
- `Gameplay/Combat/MASTER_COMBAT.md`
  - Document the new impact context structure, Water placeholder proof boundary, and final Niagara deferral.
- `Gameplay/Combat/pending_issues_Combat.md`
  - Only if implementation exposes a concrete out-of-scope gap not already documented, add a bounded pending issue.
- `Reports/Proof/CombatVFX/WaterIdolImpactStructure_<timestamp>/`
  - Store proof output or durable summary after capture.

## Out Of Scope

- Final Water idol Niagara authoring, masks, materials, texture generation, or production binding row.
- Changing `Idol_Lava` from DOT to AOE/fire explosion.
- Changing `Idol_Earth`, `Idol_Storm`, or any Pierce/Bounce/DOT idol's gameplay-observable target selection, damage amount, status application, or timing.
- Per-hit/per-bounce/per-secondary splash idol chaining behavior beyond storing contexts for future use.
- Mini/minigame code, data, docs, or generated outputs.
- Broad DataTable schema expansion beyond what the first structural pass needs.

## Risks And Mitigations

- Risk: changing idol damage selection could alter normal gameplay unexpectedly.
  - Mitigation: use context-driven target selection only for `Idol_Water`; all other idols are behavior-neutral in this pass. The proof mode includes Water-hit and Water-miss targets plus `DamageBySource` separation.
- Risk: placeholder sphere could be mistaken for final production VFX.
  - Mitigation: log and document it as `Placeholder`; do not add a production binding row or final PPF close for Water Niagara. `MASTER_COMBAT.md` must explicitly say the sphere is temporary structure proof, not a shipped Water effect.
- Risk: both the old blue cube projectile lane and the new blue sphere appear for Water.
  - Mitigation: mutual exclusion rule: if an equipped idol is `Idol_Water` and its impact presentation path is active for this attack, exclude it from `VisualPayloadCount`, skip `SpawnWeaponProjectileVisual(..., Idol_Water, ...)`, and log `CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Water Reason=ImpactPresentationActive`. Other idols are unaffected.
- Risk: normal combat log spam.
  - Mitigation: add `T66.Combat.ImpactSourceVerbose` defaulting to `0`; proof mode/capture enables it through `ExecCmds`.
- Risk: proof mode mutates normal inventory/selection state.
  - Mitigation: keep Water equip/grant inside non-shipping automation proof mode, snapshot/restore prior idol state after proof logging, and do not change normal idol acquisition flow.
- Risk: staged standalone verification is expensive.
  - Mitigation: run focused build first; if compile/capture passes, refresh staged standalone per repo rule because runtime gameplay code changed.

## Rollback / Revert Considerations

Edits are expected to be limited to the files above. Runtime behavior can be reverted by removing the impact-context struct/helper wiring, the Water proof mode, and the placeholder helper. No production Water assets/DataTable rows should be created, so rollback should not require asset deletion beyond report/proof outputs.

## Verification Plan

1. Run a focused C++ build for `T66Editor` Development Win64.
2. Run any existing lightweight validators that remain applicable, including `Scripts/ValidateCombatVFXProductionBindings.py` if the production binding code path or scripts are touched.
3. Run `Scripts/CaptureT66GameplayVideo.ps1` or the new wrapper with `-T66GameplayAutoCapture=hero1axeaoewateridolimpact`, evidence bundle enabled, and impact-source logging CVar enabled.
4. Inspect/copy proof log excerpt showing:
   - Hero 1 AOE weapon impact context.
   - Water idol impact context.
   - `CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Water Reason=ImpactPresentationActive`.
   - `CombatVFXIdolImpactPlaceholderSpawned SourceID=Idol_Water`.
   - `DamageBySource` or equivalent log with `Idol_Water` > 0.
   - Target HP pass/fail results for Water-hit and Water-miss targets.
5. Confirm `Content/Data/Idols.csv`, `Content/Data/CombatVFXBindings.csv`, and `Content/Data/DT_CombatVFXBindings.uasset` are unchanged by this structure pass.
6. Refresh staged standalone with `Scripts/StageStandaloneBuild.ps1` and verify the taskbar shortcut target if the runtime gameplay change is accepted into the playable build.

## Codex First-Pass Opinion

I agree with Pablo's clarification. The current missing piece is not final VFX art; it is a first-class combat event/impact contract. The safest implementation is to make weapons publish impact contexts, make idols derive their own impact contexts and apply damage as their own source, and prove the first concrete AOE idol with Water at the existing Hero 1 AOE slash center. I do not think we should add a Water production binding row or reuse the old imported `P_Splash_02` path as the answer to this request, because Pablo explicitly wants a blue sphere to verify structure before final Niagara.

The only caveat is that "all encompassing" should mean shared structure now, not full final category-specific behavior for every idol in this pass. The plan gives every weapon category and every idol a context seam, while only changing/proving the Water AOE target query and placeholder visual as the concrete first consumer. `Idol_Earth` and `Idol_Storm` are explicitly excluded from the query-swap behavior until their own reviewed proof.

## Reconciliation From Claude Pass 1

Claude pass 1 returned `Verdict: REVISE` with three Major issues:

1. The plan could unintentionally change all current AOE idols.
2. The plan's non-AOE preservation language was too soft.
3. Water's old blue cube projectile lane and the new blue sphere placeholder needed a precise mutual-exclusion rule.

Accepted. This revision states:

- Current AOE idol rows are `Idol_Earth`, `Idol_Water`, and `Idol_Storm`; only `Idol_Water` receives the context-driven AOE target-query change in this pass.
- `Idol_Earth`, `Idol_Storm`, and all Pierce/Bounce/DOT idols must be gameplay-observable neutral: no target-selection, damage, status, or timing changes.
- The old Water projectile lane is skipped only when Water impact presentation is active for that attack, and the skip is logged.
- The placeholder sphere uses `FT66VisualUtil::GetBasicShapeSphere`, not a new asset.
- The proof mode snapshots and restores prior equipped idol state after proof logging.
- The impact logging CVar is named `T66.Combat.ImpactSourceVerbose`, defaults off, and proof enables it.

## Review Questions

1. Is this implementation plan safe under the T66 process and current live code?
2. Does the plan satisfy Pablo's clarified requirement that idols have their own damage source and impact point?
3. Is limiting the first concrete target-query change to AOE/Water acceptable while structurally preparing all weapon/idol categories?
4. Are there any missing files, verification gates, or process contradictions before implementation?

## Claude Reviewer Instructions

Review this plan before edits. Identify unsafe scope, missing repo instructions, inadequate verification, damage-authority mistakes, placeholder/final-VFX confusion, or places where the plan silently narrows Pablo's all-weapons/all-idols requirement.

First non-empty line must be exactly `Verdict: APPROVE`, `Verdict: REVISE`, or `Verdict: BLOCK`.

</review_packet>
