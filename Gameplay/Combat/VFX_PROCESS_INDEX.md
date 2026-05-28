# Combat VFX Process Index

**Status:** Router and quick-start only. This file does not replace `CombatVFXAuthoringProcedure.md`, per-effect packets, or runtime combat docs.

## Purpose

Use this file when starting any T66 combat VFX task: weapon base attacks, slash/aura/projectile/trail effects, production VFX binding, item-stat VFX scaling proof, or idol overlay planning.

This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.

## Read Order

1. `AGENTS.md`
2. `Gameplay/GAMEPLAY_AGENTS.md`
3. `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
4. `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
5. Per-effect packet, such as `Hero1AxeAOESlashMechanismPacket.md`
6. `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
7. Runtime reference in `Gameplay/Combat/MASTER_COMBAT.md`
8. Inventory/history in `Gameplay/Combat/CombatVFXInfrastructureInventory.md`

## Current Baseline

| Area | Current status |
|---|---|
| Hero 1 AOE | Production binding and crescent-band hitbox backend proof exist for `Hero_1_black_aoe`; final visual-polish approval remains separate. |
| Hero 1 DOT | Infrastructure packet only. No active production binding row. |
| Hero 1 Pierce | Infrastructure packet only. No active production binding row. |
| Hero 1 Bounce | Infrastructure packet only. No active production binding row. |
| Idol overlays | Architecture document only. No idol overlay assets or active rows. |
| Generated assets | Combat-VFX-local policy only; repo-wide generated asset policy is out of scope for this baseline. |

## Standard VFX Flow

1. Classify the effect and select the owning process packet.
2. Collect source evidence through Pablo-provided transcripts or written sources.
3. Complete PPF, artifact parity, mechanism manifest, and anti-lookalike gates.
4. Create or update the effect packet from `EffectPacketTemplate.md`.
5. Author lab assets only inside declared lab space until promotion is approved.
6. Validate visual mechanisms through editor-isolation and gameplay-capture evidence.
7. Promote to production only through a reviewed production binding row and runtime proof.
8. Prove damage/hitbox authority through combat logic, not Niagara collision or visual opacity.
9. Commit only after validator logs, evidence bundle, staged manifest, and staged-diff review pass.

## Durable Proof Artifacts

Use `Reports/Proof/CombatVFX/<TaskSlug>/` for proof summaries, logs, contact sheets, and validation output. Use `Reports/AgentReviews/<TaskSlug>/decision_block.md` for durable decision gates that should not be re-asked on continuation.
