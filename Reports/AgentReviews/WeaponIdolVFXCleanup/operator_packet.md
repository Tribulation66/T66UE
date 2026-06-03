Operator Packet: COMPLETE

## Task

Behavior-preserving cleanup pass of the weapon/idol Combat VFX infrastructure, per
`Reports/AgentReviews/WeaponIdolVFXCleanup/codex_operator_approval.md` (APPROVE).

- Operator: Claude (`claude-opus-4-8`), tool profile FullOperator.
- Validator: Codex.

## Changed Files

| File | Change |
|---|---|
| `Source/T66/Gameplay/T66CombatShared.h` | Declared centralized proof-idol accessors `GetImpactPresentationProofIdols()` and `GetSupportedProofIdols()` with intent comments. |
| `Source/T66/Gameplay/T66CombatShared.cpp` | Implemented the two accessors as a base set (Water/Light/Electric/Poison) and a derived set (base + Earth) so the two memberships cannot drift. |
| `Source/T66/Gameplay/T66CombatComponent.cpp` | De-Watered internal diagnostic locals/lambda params; routed `UsesImpactPresentationForIdol` through the centralized set; renamed one suppression reason token. Emitted log text left byte-identical where proof runners parse it. |
| `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` | Replaced the inline `SupportedProofIdols` set in the axe proof harness with `T66CombatShared::GetSupportedProofIdols()`. |
| `Gameplay/Combat/VFX_PROCESS_INDEX.md` | Corrected the Current Baseline table to match live bindings. |
| `Gameplay/Combat/CombatVFXInfrastructureInventory.md` | Updated the binding-table-source row to list all active Hero 1 rows. |

Note: the impact-context idol infrastructure these edits touch is itself uncommitted
working-tree work (it does not exist in `HEAD`), so the cleanup was layered on top of
that in-session state, not a committed baseline. `git diff --stat` line counts for the
two `.cpp` files are dominated by that pre-existing uncommitted infrastructure, not by
this cleanup pass.

## Behavior-Preserving Claim

No runtime behavior changed.

- The de-Water renames are internal C++ identifiers (locals, lambda parameters) with zero
  observable effect.
- Every emitted `UE_LOG` string that a proof runner parses was kept byte-identical. In
  particular `CombatIdolImpactDiagnostic SourceID=Idol_Water WaterIdolContextParity=%s ...
  ExpectedWaterIdolImpactContexts=%d WaterIdolImpactContexts=%d WaterSkippedNoWeaponContext=%d
  WaterSkippedInvalidImpactPoint=%d WaterLegacyFallbacks=%d ...` is unchanged; its
  Water-named log fields are a deliberately preserved compatibility diagnostic that
  `RunHero1AxeAOEWaterIdolImpactProof.ps1` and the Water-regression case of
  `RunHero1AxeIdolCategoryNativeImpactProof.ps1` parse.
- Centralization is membership-identical: `GetImpactPresentationProofIdols()` returns
  exactly {Water, Light, Electric, Poison} and `GetSupportedProofIdols()` adds Earth —
  the same sets that were previously inlined at the two call sites.

### Intentionally NOT touched

- Damage, target selection, hitbox geometry, timing, falloff, `AoeDelay`,
  `AoeInnerRadiusRatio`, proof HP/staging, placeholder scale/lifespan.
- The preserved Water/AOE path log `CombatIdolWaterImpactResolved ...
  RadiusSource=FIdolData.AoeRadius`, the `Idol_Water` literal category branch, and the
  `SpawnWaterIdolImpactPlaceholderVFX` Water-placeholder spawn (these are genuinely the
  Water/AOE path, not generalized vocabulary).
- No new DOT production binding row, no production `IdolModifier` binding rows, no proof
  runner pattern changes (none were needed), no Mini/minigame code, no final Niagara art.

## Before/After Naming Map (this cleanup pass)

Internal identifiers in `T66CombatComponent.cpp` idol impact diagnostic block:

| Before | After |
|---|---|
| `WaterIdolImpactContextCount` (local) | `IdolImpactContextCount` |
| `WaterSkippedNoWeaponContext` (local) | `IdolSkippedNoWeaponContext` |
| `WaterSkippedInvalidImpactPoint` (local) | `IdolSkippedInvalidImpactPoint` |
| `WaterLegacyFallbackCount` (local) | `IdolLegacyFallbackCount` |
| `ExpectedWaterIdolImpactContexts` (local) | `ExpectedIdolImpactContexts` |
| `bTouchedWaterIdolDiagnostics` | `bTouchedIdolImpactDiagnostics` |
| `bWaterContextParity` | `bIdolContextParity` |
| `Reason=ImpactPresentationOwnsWaterPlaceholder` (suppression log token) | `Reason=ImpactPresentationOwnsIdolPlaceholder` |

The suppression token sits on the `CombatVFXIdolProjectileLaneSuppressed SourceID=%s ...`
line, which the proof runners gate on only by the `SourceID=` prefix, not the `Reason=`
value — so this rename is proof-safe (verified: all cases still PASS that pattern).

Centralization replacements (membership-preserving):

| Site | Before | After |
|---|---|---|
| `T66CombatComponent.cpp` `UsesImpactPresentationForIdol` | inline `TSet<FName>{Water,Light,Electric,Poison}` | `T66CombatShared::GetImpactPresentationProofIdols().Contains(...)` |
| `T66PlayerController_Overlays.cpp` axe proof harness | inline `static const TSet<FName> SupportedProofIdols{...Earth}` | `T66CombatShared::GetSupportedProofIdols().Contains(...)` |

## Docs Consistency Correction

Live data verified against `Content/Data/CombatVFXBindings.csv`:

- `Hero1Axe_AOE_Base` → `Hero_1_black_aoe` — active (Enabled True).
- `Hero1Axe_Pierce_Base` → `Hero_1_black_pierce` — active (Enabled True).
- `Hero1Axe_Bounce_Base` → `Hero_1_black_bounce` — active (Enabled True).
- No DOT row exists.

`VFX_PROCESS_INDEX.md` previously claimed Hero 1 Pierce and Bounce had "No active
production binding row" — corrected to active rows. DOT remains "Infrastructure packet
only / no active production binding row." Idol overlays row clarified: idol category
proofs are structural/proof placeholder paths, not production idol Niagara rows.
`CombatVFXInfrastructureInventory.md` binding-table-source row updated to list all three
active Hero 1 rows and to note no DOT production row and that idol category proofs are
placeholder/proof paths.

## Verification — Commands and Outcomes

1. Focused compile: `T66Editor Win64 Development`
   - Command: `Build.bat T66Editor Win64 Development -Project="C:/UE/T66/T66.uproject" -WaitMutex`
   - Result: **Succeeded** (exit 0, ~42s). Only warning is a pre-existing Niagara API
     deprecation (`FNiagaraEmitterInstance::IsReadyToRun`) in
     `T66Hero1AxeAOEVFXLabActor.cpp:353`, unrelated to this pass.

2. Idol category-native impact proof:
   `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1`
   - Result: all 5 cases CaptureOK, every required pattern PASS, every forbidden pattern
     absent.

| Case | Idol | Category | Result |
|---|---|---|---|
| LightPierce | Idol_Light | Pierce | PASS |
| ElectricBounce | Idol_Electric | Bounce | PASS |
| PoisonDOT | Idol_Poison | DOT | PASS |
| WaterAOERegression | Idol_Water | AOE | PASS (incl. `WaterIdolContextParity=PASS`, `CombatIdolImpactDiagnostic SourceID=Idol_Water`) |
| EarthNeutral | Idol_Earth | (neutral control) | PASS (no themed idol context/damage; all idol-impact forbidden patterns absent) |

The Water regression case passing with its byte-identical Water-named diagnostic confirms
the de-Water rename did not alter emitted log contracts. Earth passing confirms the
neutral control is still excluded from the idol impact-presentation lane.

## Proof Artifacts

- Summary: `Saved/VideoCaptures/Hero1AxeIdolCategoryNativeImpactProof_20260530_025904/Hero1AxeIdolCategoryNativeImpactProofSummary.md`
- Per-case logs/excerpts/videos under that same root, one folder per case
  (`LightPierce/`, `ElectricBounce/`, `PoisonDOT/`, `WaterAOERegression/`, `EarthNeutral/`),
  each with `T66.log`, `proof_log_excerpt.md`, frames, and `.mp4`.

## Caveats / Skipped Verification

- Visual fidelity / player-readability not assessed; this proof validates impact-context
  identity, parity, and damage-source diagnostics only (by design of the runner).
- The pre-existing Niagara deprecation warning was left as-is (out of scope).
- No git commit/push/tag performed (none requested; would require Codex approval).

## Claude Token Usage

Not exposed: this pass was run as an interactive Operator session, not through
`Scripts/Invoke-ClaudeDirectRead.ps1`, so no helper token meter was available to report.
