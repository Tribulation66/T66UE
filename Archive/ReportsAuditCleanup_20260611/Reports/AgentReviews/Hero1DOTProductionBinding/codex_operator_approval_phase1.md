Codex Approval: APPROVE

## Approved Task

Phase 1: make the Hero 1 DOT weapon production Combat VFX binding real and active, matching the AOE/Pierce/Bounce production binding method class.

## Roles

- Operator: Claude (`claude-opus-4-8`) through `Scripts\Invoke-ClaudeDirectRead.ps1`.
- Tool profile: `FullOperator`.
- Validator: Codex.

## Scope

Approved in scope:

- Create or update a production DOT weapon Niagara carrier asset under `Content/VFX/Hero1/Axe/DOT/`.
- Add the active `Hero1Axe_DOT_Base` `WeaponBase` row for `Hero_1_black_dot` / `DOT` to `Content/Data/CombatVFXBindings.csv`.
- Update `Scripts/SetupCombatVFXBindingsDataTable.py` so it enforces the DOT row with AOE/Pierce/Bounce.
- Update `Scripts/ValidateCombatVFXProductionBindings.py` so it validates the DOT row and required production DOT asset(s).
- Wire runtime DOT weapon presentation so `PerformDOT` resolves and uses the DOT production binding for its single hero-to-target shot carrier, while keeping DOT damage and tick timing behavior unchanged.
- Update DOT/VFX docs that explicitly say DOT has no active production row.
- Run focused compile, DOT asset commandlet/generation, Combat VFX DataTable reload, and production binding validation if physically possible.
- Write `Reports/AgentReviews/Hero1DOTProductionBinding/operator_packet_phase1.md`.

Explicitly out of scope:

- Idols or `IdolModifier` production rows.
- Final DOT visual-polish sign-off, imagegen, or Pablo visual approval.
- Gameplay damage/timing changes: preserve initial contact damage, one `ApplyDOT(... HeroPrimaryDot)` payload, `TickInterval`, DOT marker reveal cadence, marker visible duration, and frostbite/SFX behavior.
- Mini/minigame systems.
- Git commit/push/tag/stage operations.
- Destructive cleanup, broad unrelated refactors, or reverting user/peer changes.

## Process Gates

PPF CHECK
Objective: make `Hero_1_black_dot` an active weapon-base production Combat VFX binding like AOE/Pierce/Bounce.
Proven process: `Gameplay/Combat/CombatVFXAuthoringProcedure.md` plus active AOE/Pierce/Bounce production binding method: Niagara carrier asset under `Content/VFX`, active CSV row, DataTable reload, production validator, runtime proof.
My planned implementation: add a real DOT Niagara carrier and bind the DOT shot to it. Do not create a fake CSV-only row or point DOT at an unrelated slash asset.
Same method class: YES, if and only if the primary DOT shot silhouette is authored by Niagara/material/renderer assets and runtime only transports it.
If NO, why: stop and report the blocker.
User approval required before proceeding: NO for the same-method implementation; YES if substituting actor-side placeholder geometry or an unrelated existing VFX asset as the active production row.
Verification evidence: compile, generated asset path, DataTable reload, production validator.

ARTIFACT PARITY GATE
Reference artifact/category: active Hero 1 weapon-base production rows for AOE/Pierce/Bounce.
Role: Primary.
Required: YES.
Planned artifact/path: `Content/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_*` plus `Hero1Axe_DOT_Base` row for `Hero_1_black_dot`.
Status: SAME method class required.
Evidence: asset exists, row active, DataTable reload, validator required asset check.

MECHANISM MANIFEST
Reference/source: `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md` and active weapon production binding pattern.
Required mechanisms:
1. Mechanism: single moving hero-to-target DOT shot.
   Required: YES.
   Planned implementation: DOT shot uses the bound Niagara carrier when `Hero1Axe_DOT_Base` resolves; `AT66HeroProjectile`/movement only transports it.
   Evidence needed: runtime/code proof that DOT binding is resolved and the temporary core mesh is hidden when a carrier is attached.
2. Mechanism: single authoritative DOT payload.
   Required: YES.
   Planned implementation: preserve exactly one `ApplyDOT(... HeroPrimaryDot)` payload.
   Evidence needed: code proof and later runtime proof.
3. Mechanism: DOT marker cadence.
   Required: YES.
   Planned implementation: do not change `AT66DotMarkerVFX` reveal/hide cadence or visible pulse duration.
   Evidence needed: code proof and later runtime proof.
4. Mechanism: active production binding contract.
   Required: YES.
   Planned implementation: active CSV/DataTable row, production asset path, no `/Game/VFXLab` dependency, validator coverage.
   Evidence needed: setup/validator logs.

## Required Context

Read and follow:

- `AGENTS.md`
- `.t66/operator-state.json`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/Hero1AxeDOTMechanismPacket.md`
- `Gameplay/Combat/pending_issues_Combat.md`

Useful live findings from Codex:

- `Content/VFX/Hero1/Axe/DOT` currently has no DOT Niagara asset by filename.
- `PerformDOT` currently publishes a weapon impact context, then spawns a temporary-profile visual-only `SpawnVisualTravelProjectile` and defers the single DOT payload/markers to its arrival.
- `ShouldSuppressWeaponBaseProjectileVisual()` would suppress the pre-fire base placeholder if a DOT row resolves, but `PerformDOT` must also consume the row for the actual moving DOT shot carrier.
- `Scripts/ValidateCombatVFXProductionBindings.py` currently has a DOT scaffold self-test row but no active DOT validator.

## Required Output

Write:

`Reports/AgentReviews/Hero1DOTProductionBinding/operator_packet_phase1.md`

The first non-empty line must be exactly:

`Operator Packet: COMPLETE`

Packet must include:

- changed files/assets;
- generated asset paths and byte sizes if available;
- behavior-preserving claim for DOT damage/timing;
- active DOT row details;
- setup/reload/validator/compile commands and outcomes;
- caveats/skipped verification;
- next required Phase 2 runtime proof command;
- Claude token usage if exposed by the helper.
