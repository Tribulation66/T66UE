Codex Approval: APPROVE

## Approved Task

Phase 2 runtime wiring only: make Hero 1 Bounce publish official per-link weapon impact contexts and call the production VFX dispatcher for each Bounce chain link when a `Hero1Axe_Bounce_Base` binding exists.

## Approved Scope

Claude FullOperator may edit:

- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp` only if needed for a small helper/log change
- `Gameplay/Combat/MASTER_COMBAT.md` only if a concise current-state note is needed for the runtime change

Expected runtime direction:

- Keep Bounce damage authority in `PerformBounce`.
- Replace or supplement the current one aggregated Bounce context with `PerChainLink` contexts.
- Each resolved Bounce hit should have its own `FT66CombatImpactContext` with `AttackCategory=Bounce`, `SourceType=WeaponBase`, `SourceID=Hero_1_black_bounce`, `ChainIndex`, `ImpactPoint`, `bImpactPointValid`, target handle, and effective damage for that link.
- Add a Bounce branch to bound weapon VFX spawning as needed. The visual anchor model should be `ImpactAnchored`, with a small fixed footprint/scale suitable for point impacts.
- Call the bound VFX dispatcher for each Bounce link. If no binding exists yet, the call should fail gracefully and not suppress current temporary visual behavior.
- Preserve AOE and Pierce behavior.
- Preserve idol downstream processing semantics as much as possible, with the expected future direction that each Bounce link can be an eligible weapon impact context.

## Approved Tool Surface

Claude may edit approved source/doc files and run focused read/grep/diff/build commands.

Claude may run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE
```

## Required Process Rules

- Follow `AGENTS.md`.
- Follow `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Follow `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.
- Follow `CombatVFXVisualDamageAlignmentContract.md`.
- Follow `CombatVFXImpactContextContract.md`.
- Do not use Niagara visuals as damage authority.

## Explicitly Excluded Actions

- No Unreal asset generation.
- No commandlet asset generation.
- No CSV/DataTable edits.
- No script edits unless Claude stops and requests Codex approval.
- No capture runs.
- No imagegen.
- No Mini/minigame work.
- No Git mutation.
- No broad Git/LFS scans.
- No credential or billing changes.

## Verification Required After Operator Run

Claude should report:

- changed files,
- exact compile command and result if run,
- log/source anchors showing per-link contexts,
- source anchors showing Bounce `ImpactAnchored` VFX spawn path,
- any verification skipped.

Codex will validate the actual diff and compile/log evidence.

## Approval Rationale

This phase is bounded to runtime wiring and compile proof. It does not depend on production Bounce assets yet; the VFX dispatcher must simply be ready to spawn them once Phase 3/4 adds assets and binding.
