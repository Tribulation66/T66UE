Operator Packet: COMPLETE

# Idol Category-Native Impact — Operator Packet

Task: Complete and prove full category-native idol behavior (Idol_Light Pierce, Idol_Electric Bounce, Idol_Poison DOT) triggered from the Hero 1 AOE weapon impact point (Hero_1_black_aoe), preserving the existing Idol_Water AOE path. Recovery continuation of a stalled FullOperator run.

Role: Claude = Operator. Codex = Validator. Approved scope artifact: Reports/AgentReviews/IdolCategoryNativeImpact/codex_operator_approval_recovery.md.

Result: All implementation requirements met; full focused recompile clean; all 5 Unreal-owned proof cases (Light/Electric/Poison + Water regression + Earth neutral control) pass every required log anchor and every forbidden-absence check.

---

## 1. Changed files

C++ implementation:
- Source/T66/Gameplay/T66CombatComponent.cpp — generalized idol impact dispatch from Water-only to category-native; weapon publishes WeaponPrimary impact context (SourceID=Hero_1_black_aoe); each impact-presentation idol publishes its own IdolPrimary context (SourceType=IdolModifier, SourceID=<idol>, ParentSourceID=Hero_1_black_aoe), owns its damage source, and suppresses the legacy projectile lane. Generalized CombatImpactChainDiagnostic (category-agnostic schema, ContextParity). Category switch: Pierce (line read + per-hit falloff), Bounce (chain to nearest + per-link falloff), DOT (ApplyDOT ticking), AOE (preserved Water radius path). Preserved Water-specific CombatIdolImpactDiagnostic.
- Source/T66/Gameplay/T66CombatComponent.h — declared SpawnIdolImpactPlaceholderVFX alongside the preserved SpawnWaterIdolImpactPlaceholderVFX.
- Source/T66/Gameplay/T66CombatVFX.cpp — added SpawnIdolImpactPlaceholderVFX(context, IdolID, Category, Linger): dispatches Pierce/Bounce/DOT/AOE placeholder primitives, colored via UT66IdolManagerSubsystem::GetIdolColor; emits CombatVFXIdolImpactPlaceholderSpawned. Preserved SpawnWaterIdolImpactPlaceholderVFX.
- Source/T66/Gameplay/T66PlayerController_Overlays.cpp — proof harness: all idol proofs route through the proven hero1axeaoewateridolimpact capture mode with -Hero1AxeProofIdol <id>; SupportedProofIdols {Water, Light, Electric, Poison}; per-category proof target specs and PASS/FAIL hitbox assertions; idol-state snapshot/restore.

Verification tooling:
- Scripts/CaptureT66GameplayVideo.ps1 — added -Hero1AxeProofIdol param, idol-mode CVar T66.Combat.ImpactSourceVerbose 1, WallOcclusion disable for proof.
- Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1 — NEW. 5-case runner (LightPierce/ElectricBounce/PoisonDOT/WaterAOERegression/EarthNeutral); non-fatal per-case required/forbidden pattern checks recorded to a summary so one anchor mismatch cannot abort the batch.

Note: Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1 is new/untracked; the five C++ and capture files are modified in the working tree. No git add/commit performed (out of Operator authority without explicit approval).

---

## 2. Compile (focused recompile of affected target)

Command:
  "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project=C:/UE/T66/T66.uproject -WaitMutex -NoHotReload

Logs:
- Reports/AgentReviews/IdolCategoryNativeImpact/compile_T66Editor.log — initial invocation ("Target is up to date"; binaries newer than source).
- Reports/AgentReviews/IdolCategoryNativeImpact/compile_T66Editor_forced.log — forced genuine recompile after touching the changed sources: compiled Module.T66.*.cpp unity files, linked UnrealEditor-T66.dll. Result: Succeeded (exit 0). One non-blocking warning C4996 (FNiagaraEmitterInstance::IsReadyToRun deprecation) in T66Hero1AxeAOEVFXLabActor.cpp — pre-existing, unrelated.
- Reports/AgentReviews/IdolCategoryNativeImpact/compile_T66Editor_pierce_fix.log — recompile after the Pierce proof-geometry fix (below): compiled Module.T66.29.cpp, relinked UnrealEditor-T66.dll. Result: Succeeded (exit 0).

---

## 3. Capture / proof

Command:
  pwsh -NoProfile -ExecutionPolicy Bypass -File Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1 -OutputRoot C:/UE/T66/Saved/VideoCaptures/Hero1AxeIdolCategoryNativeImpactProof

All cases launch the editor in -game via hero1axeaoewateridolimpact with FireDelay=7.6, FrameCount=72, log to Saved\Logs\T66.log (copied per case). Outcome: exit 0; all 5 cases Capture status: CaptureOK.

Summary artifact:
- Saved/VideoCaptures/Hero1AxeIdolCategoryNativeImpactProof/Hero1AxeIdolCategoryNativeImpactProofSummary.md (Generated 20260530_014507)

Per-case artifacts (each has <Name>.mp4, frames/, T66.log, proof_log_excerpt.md under Saved/VideoCaptures/Hero1AxeIdolCategoryNativeImpactProof/<Name>/):
- LightPierce (Idol_Light)
- ElectricBounce (Idol_Electric)
- PoisonDOT (Idol_Poison)
- WaterAOERegression (Idol_Water)
- EarthNeutral (Idol_Earth)

Status: every required pattern PASS and every forbidden pattern absent across all 5 cases.

---

## 4. Key log anchors (per idol)

LightPierce (Idol_Light → Pierce):
- CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase SourceID=Hero_1_black_aoe ... AttackCategory=AOE ImpactPoint=V(X=694.95) HitTargets=1
- CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Light ParentSourceID=Hero_1_black_aoe AttackCategory=Pierce LineLength=1030 TubeRadius=84 HitTargets=2
- CombatIdolCategoryImpactResolved SourceID=Idol_Light Category=Pierce ParentSourceID=Hero_1_black_aoe Targets=2
- CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Light
- CombatImpactChainDiagnostic SourceID=Idol_Light Category=Pierce ContextParity=PASS LegacyFallbacks=0 DamageByDownstreamSource=PASS
- DamageBySource SourceID=Idol_Light TotalDamage=15 (idol-owned, distinct from AutoAttack=28)

ElectricBounce (Idol_Electric → Bounce): WeaponPrimary SourceID=Hero_1_black_aoe; IdolPrimary SourceType=IdolModifier SourceID=Idol_Electric ParentSourceID=Hero_1_black_aoe; CombatIdolCategoryImpactResolved Category=Bounce; lane suppressed; ContextParity=PASS; DamageBySource SourceID=Idol_Electric.

PoisonDOT (Idol_Poison → DOT): WeaponPrimary SourceID=Hero_1_black_aoe; IdolPrimary SourceType=IdolModifier SourceID=Idol_Poison ParentSourceID=Hero_1_black_aoe; CombatIdolCategoryImpactResolved Category=DOT; lane suppressed; ContextParity=PASS; DamageBySource SourceID=Idol_Poison.

WaterAOERegression (Idol_Water → AOE, preserved): WeaponPrimary SourceID=Hero_1_black_aoe; IdolPrimary SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe; CombatIdolWaterImpactResolved ... RadiusSource=FIdolData.AoeRadius; CombatIdolImpactDiagnostic SourceID=Idol_Water WaterIdolContextParity=PASS; DamageBySource SourceID=Idol_Water.

EarthNeutral (Idol_Earth → neutral control): only WeaponPrimary SourceID=Hero_1_black_aoe + DamageBySource SourceID=Idol_Earth present; NO IdolPrimary IdolModifier context, NO CombatIdolCategoryImpactResolved, NO themed-idol DamageBySource (Water/Light/Electric/Poison) — confirms idol impact context is gated to impact-presentation idols and does not leak to neutral idols.

No-double-lane: every impact-presentation idol logs CombatVFXIdolProjectileLaneSuppressed and LegacyFallbacks=0; no unintended legacy projectile damage lane.

---

## 5. Caveat resolved during this run (Pierce proof geometry)

First proof pass showed LightPierce tripping the forbidden anchor Result=FAIL while all contract anchors passed. Root cause was a proof-harness geometry artifact (not an implementation defect): the Pierce ExpectedHit=0 control targets were placed inside the parent weapon AOE radius (435) — so the parent AOE legitimately damaged them, and total-HP-delta could not separate parent-AOE damage from idol-pierce damage — and OutsideBehind landed only 40u behind the pierce origin, leaking into the pierce capsule start and stealing a slot under the 2-target cap from the on-line PierceInLineSecond.

Fix (Source/T66/Gameplay/T66PlayerController_Overlays.cpp, bIdolPierceProof TargetSpecs): moved all ExpectedHit=0 controls far outside both the parent AOE radius and the pierce tube/length (Right*4200, Forward*4200 past LineLength, -Forward*4200), and moved PierceInLineSecond to Forward*600 (world X=960, beyond AOE outer radius 435 but within pierce LineLength 1030) so it is hit only by the pierce line — isolating and proving pierce reach. Recompiled (compile_T66Editor_pierce_fix.log, exit 0) and recaptured all 5 cases. Post-fix LightPierce: Primary PASS, PierceInLineSecond ActualHit=1 (pierce-only, Amount=7) PASS, all 3 controls ActualHit=0 PASS; parent AutoAttack HitTargets=1; idol DamageBySource SourceID=Idol_Light TotalDamage=15. No remaining FAIL anchors in any case.

This brought Pierce control placement in line with the Bounce/DOT cases (which already placed controls at Forward*4200 and were unaffected).

---

## 6. Scope / authority notes

- No material scope expansion beyond the approved recovery contract; presentation remains placeholder/proof primitives (no final Niagara art), as permitted.
- No git add/commit/push/tag/reset/clean, no LFS/binary-asset scans, no credential/billing changes performed.
- Working tree carries unrelated pre-existing modifications (per session snapshot); this packet's changes are limited to the files in section 1.

## 7. Token usage

Claude token usage is not exposed to the Operator at runtime in this environment; not reported.
