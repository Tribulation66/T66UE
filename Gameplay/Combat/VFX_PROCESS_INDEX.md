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
5. `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
6. `Gameplay/Combat/CombatVFXImpactContextContract.md`
7. Per-effect packet, such as `Hero1AxeAOESlashMechanismPacket.md`
8. `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
9. Runtime reference in `Gameplay/Combat/MASTER_COMBAT.md`
10. Inventory/history in `Gameplay/Combat/CombatVFXInfrastructureInventory.md`

## Current Baseline

| Area | Current status |
|---|---|
| Hero 1 AOE | Active production binding row (`Hero1Axe_AOE_Base` → `Hero_1_black_aoe`) plus crescent-band hitbox backend proof; final visual-polish approval remains separate. |
| Hero 1 RetiredLine | Active production binding row (`Hero1Axe_RetiredLine_Base` → `Hero_1_black_retired-line`, PathAnchored lane); final visual-polish approval remains separate. |
| Hero 1 Bounce | Active production binding row (`Hero1Axe_Bounce_Base` → `Hero_1_black_bounce`, ImpactAnchored per-link carrier); final visual-polish approval remains separate. |
| Hero 1 DOT | Active production binding row (`Hero1Axe_DOT_Base` → `Hero_1_black_dot`, moving aura-ring carrier transported by the single hero->target DOT shot); final visual-polish approval remains separate. |
| Idol overlays | Architecture plus impact-context contract. Idol category proofs are structural/proof placeholder paths only; no production idol Niagara assets or active production idol rows are approved by this baseline. |
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
9. Prove visual/damage alignment: declare anchor model, footprint mapping, offsets, tolerance, and evidence size/position agreement with the authoritative hitbox, or record an approved intentional mismatch.
10. Prove impact-context identity and parity: weapon contexts publish at official impact points, idol/downstream contexts preserve `ParentSourceID`, own their `SourceID`, emit parity and skip/fallback counters, and pass a neutral control.
11. Commit only after validator logs, evidence bundle, staged manifest, and staged-diff review pass.

## Durable Proof Artifacts

Use `Reports/Proof/CombatVFX/<TaskSlug>/` for proof summaries, logs, contact sheets, and validation output. Use `Reports/AgentReviews/<TaskSlug>/decision_block.md` for durable decision gates that should not be re-asked on continuation.
