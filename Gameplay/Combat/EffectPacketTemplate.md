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

## 8. Hitbox / Damage Authority And Visual Alignment

See `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md` for definitions. VFX remains presentation; damage authority stays in the combat query, damage primitive, or impact context.

- Authoritative damage center:
- Impact point:
- Damage shape type:
- Damage extents:
  - radius:
  - inner radius:
  - half-angle:
  - length / width / height / tube radius:
- Visual anchor model: `CenterAnchored / ImpactAnchored / BandAnchored / PathAnchored`
- Visual pivot:
- Visual offsets:
  - planar offset:
  - Z lift:
  - socket/path offset:
- Visual footprint:
- Footprint mapping:
  - `BaseVisualRadius`:
  - `VisualScaleMultiplier`:
  - formula or alternate mapping:
- Intentional mismatch: `YES/NO`
- If `YES`, user approval artifact and alternate telegraph/area read:
- Alignment tolerance:
- Proof route:
  - debug DamageVolume overlay:
  - gameplay capture:
  - editor-isolation capture:
  - log fields proving impact point, visual location, extents, `BaseVisualRadius`, and `VisualScale`:

## 9. Impact Context Contract

See `Gameplay/Combat/CombatVFXImpactContextContract.md` when the effect publishes, consumes, or chains a combat impact context.

- Role: `WeaponPublisher / IdolConsumer / DownstreamPublisher / None`
- Weapon context publication policy:
- Eligible context rule:
- Expected downstream context count:
- SourceType:
- SourceID:
- ParentSourceID rule:
- Impact point rule:
- Damage/status source proof:
- Neutral control:
- Diagnostic schema:
  - generalized fields:
  - effect-specific field mapping, if any:
- Legacy fallback allowed: `YES/NO`
- Video-only proof accepted: `NO`
- Intentional exceptions:

## 10. Proof Plan

- editor-isolation capture:
- gameplay capture:
- evidence bundle:
- item/stat proof:
- validator:
- staged manifest:

## 11. Close

Use the close templates from `CombatVFXAuthoringProcedure.md`.
