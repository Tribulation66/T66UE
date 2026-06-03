Codex Approval: APPROVE

## Approved Task

Phase 1 only: activate `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` for the Hero 1 Bounce small horizontal red/blue slash structural VFX pass.

## Approved Scope

- Rewrite `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` from infrastructure scaffold into an active packet.
- Include PPF, artifact parity, mechanism manifest, visual/damage alignment, impact context, anti-lookalike discriminator, verification plan, and caveats.
- Lock primary structural carrier as `ImpactAnchored` ArcSlash per chain link.
- Lock intended Bounce weapon impact-context policy as `PerChainLink`.
- Explicitly defer final visual approval and imagegen/mockup gate unless Pablo asks for it.

## Approved Tool Surface

Claude FullOperator may edit the approved Markdown packet only.

No shell commands are required beyond read/write operations for this Phase 1 packet edit.

## Required Process Rules

- Follow `AGENTS.md`.
- Follow `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Follow `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- Follow `CombatVFXDefinitionOfDone.md`.
- Follow `CombatVFXVisualDamageAlignmentContract.md`.
- Follow `CombatVFXImpactContextContract.md`.

## Explicitly Excluded Actions

- No C++ edits.
- No script edits.
- No CSV/DataTable edits.
- No Unreal asset generation or modification.
- No commandlets.
- No capture runs.
- No Mini/minigame work.
- No Git mutation.
- No broad Git/LFS scans.
- No credential or billing changes.

## Verification Required After Operator Run

Codex must inspect `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` and confirm that the packet contains the approved carrier/context decisions and required process sections.

## Approval Rationale

The phase is small, reversible, and necessary before runtime/assets. It resolves the scaffold conflict and creates the source-of-truth packet needed for later bounded implementation phases.
