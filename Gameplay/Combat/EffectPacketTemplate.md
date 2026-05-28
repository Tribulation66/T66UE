# Combat VFX Effect Packet Template

**Status:** Fillable template for future combat VFX packets. Copy this into a new per-effect packet and replace bracketed fields.

This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.

## 1. Packet Header

- Effect name: `[Hero/Weapon/Category]`
- Status: `PLANNED / PARTIAL / FULL / DEFERRED`
- Owner: `Gameplay/Combat`
- Parent process: `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- Related runtime doc: `Gameplay/Combat/MASTER_COMBAT.md`
- Production binding status: `NONE / PLANNED / ACTIVE`

## 2. User Direction

Record the exact user visual/mechanical direction and any explicit scope limits.

## 3. Source Evidence

- Primary reference:
- Transcript/source paths:
- Values:
  - `observed`:
  - `inferred`:
  - `tuned`:
- Missing source detail:

## 4. Carrier And Mechanisms

- Primary carrier archetype:
- Secondary archetypes:
- Required mechanisms:
  - motion:
  - timing:
  - reveal:
  - material animation:
  - erosion/dissipation:
  - support/impact:

## 5. Artifact Parity

List primary and secondary artifacts. Required primary artifacts cannot be dropped or replaced without approval.

## 6. Mask / Material Manifest

Use the row schema from `CombatVFXAuthoringProcedure.md`. Do not copy generic process text here.

## 7. Binding Contract

- Binding row ID:
- Source type:
- Source ID:
- Attack category:
- Niagara path:
- Effect packet ID:
- Development fallback allowed:
- Active production row: `YES/NO`
- If `NO`, explain why this packet is infrastructure-only or deferred.

## 8. Hitbox / Damage Authority

State the logical query shape and proof route. VFX remains presentation.

## 9. Proof Plan

- editor-isolation capture:
- gameplay capture:
- evidence bundle:
- item/stat proof:
- validator:
- staged manifest:

## 10. Close

Use the close templates from `CombatVFXAuthoringProcedure.md`.
