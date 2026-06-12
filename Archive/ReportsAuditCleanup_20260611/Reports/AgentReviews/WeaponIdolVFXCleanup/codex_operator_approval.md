Codex Approval: APPROVE

## Approved Task

Implement the best behavior-preserving cleanup pass for the current weapon/idol Combat VFX infrastructure.

## Roles

- Operator: Claude (`claude-opus-4-8`) through `Scripts\Invoke-ClaudeDirectRead.ps1`.
- Tool profile: `FullOperator`.
- Validator: Codex.

## Scope

Approved in scope:

- Rename/de-Water generalized idol impact infrastructure identifiers and log text.
- Centralize proof-idol metadata for runtime C++ and overlay proof harness where practical.
- Keep existing proof runner behavior in sync with any renamed diagnostics.
- Refresh stale Combat VFX docs so they match live binding data:
  - AOE, Pierce, and Bounce have active Hero 1 weapon production binding rows.
  - DOT does not currently have an active Hero 1 weapon production binding row.
  - Idol category proofs are structural/proof placeholder paths, not production idol Niagara rows.
- Add/adjust report artifacts under `Reports/AgentReviews/WeaponIdolVFXCleanup`.
- Run focused compile and current proof commands needed to prove behavior did not change.

Explicitly out of scope:

- Mini/minigame systems.
- Final Niagara art or generated media.
- Adding DOT production binding.
- Adding production idol `IdolModifier` binding rows.
- Damage behavior, target selection, hitbox geometry, timing, falloff values, `AoeDelay`, `AoeInnerRadiusRatio`, proof target HP/staging, placeholder visual scale/lifespan.
- Broad refactors of unrelated gameplay automation, boss/vendor systems, movement, or non-VFX code.
- Destructive git operations or reverting unrelated/user/peer changes.

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
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Reports/AgentReviews/WeaponIdolVFXInfrastructurePass/codex_validator_report.md`

Important live-state correction from Codex validation:

- `Content/Data/CombatVFXBindings.csv` has active rows for `Hero1Axe_AOE_Base`, `Hero1Axe_Pierce_Base`, and `Hero1Axe_Bounce_Base`.
- No active Hero 1 DOT production row exists.
- `VFX_PROCESS_INDEX.md` is stale if it says Pierce/Bounce have no active production row.

## Required Verification

At minimum, attempt:

- Focused C++ compile for `T66Editor Win64 Development`.
- `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1` after any log/proof-pattern rename.
- A targeted check of `Content/Data/CombatVFXBindings.csv`/docs consistency.

If any proof cannot be run, state exactly what was attempted and why it failed.

## Required Output

Write:

`Reports/AgentReviews/WeaponIdolVFXCleanup/operator_packet.md`

The first non-empty line must be exactly:

`Operator Packet: COMPLETE`

Packet must include:

- changed files;
- behavior-preserving claim and what was intentionally not touched;
- compile/proof commands and outcomes;
- proof artifact/log paths;
- before/after naming or diagnostic mapping;
- docs consistency correction;
- caveats/skipped verification;
- Claude token usage if exposed by the helper.
