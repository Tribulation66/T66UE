# Combat VFX Idol Overlay Architecture

**Status:** Architecture only. No idol VFX assets, active idol binding rows, or gameplay behavior are implemented by this document.

## Goal

Define the future seam for idol-driven visual layers that add elemental or thematic overlays to weapon-base VFX without replacing the weapon effect or changing damage authority.

## Rules

- Weapon VFX owns the primary attack silhouette.
- Idol overlay VFX owns an additive secondary layer such as electricity, fire, poison, frost, holy light, or gravity.
- Idol overlays must not resurrect the temporary projectile placeholder path as the production system.
- Idol visuals are presentation unless a separate combat rule gives the idol damage or status authority.
- Overlay assets and active rows require their own effect packet, source evidence, visual proof, and validator coverage.
- Idol-owned damage uses a combat impact context, not the overlay layer, as its authority seam. Weapon impact contexts may trigger an idol impact context at the weapon impact point, and that idol context owns its own source ID, query radius, damage/status application, and future chaining impact point. The visual layer remains additive presentation even when it is spawned from that idol-owned context.
- Idol impact-context wiring must follow `CombatVFXImpactContextContract.md`: weapon contexts publish at official impact points, idol contexts preserve `ParentSourceID`, idol damage/status uses the idol `SourceID`, context parity is logged, skip/fallback counters are explicit, and a neutral control proves the themed idol did not trigger when inactive.
- Idol contexts should preserve `ParentSourceID=<weapon source>` when spawned from a weapon impact so future chaining, logs, and damage attribution can connect the idol event back to the driving attack.
- The first Water proof validates the compiled and reachable idol binding seam plus fallback placeholder branch; it does not validate a real idol Niagara asset spawn until a production `IdolModifier` binding row and effect packet are authored.
- Idol impact visuals must follow `CombatVFXVisualDamageAlignmentContract.md`. An idol with AOE damage must declare whether the visual is an impact marker, an area/damage read, or an approved split between the two.
- A compact marker over a larger idol AOE radius is temporary proof only unless the effect packet records user approval and provides a separate readable area telegraph.

## Future Binding Shape

Expected future fields or concepts:

- weapon binding ID,
- idol ID or idol modifier ID,
- overlay Niagara system path,
- overlay material parameter set,
- compatible attack categories,
- stacking/priority rule,
- damage/status authority note,
- development fallback flag,
- effect packet ID.

## Future Proof Requirements

- base weapon VFX still visible with overlay active,
- overlay follows the weapon carrier/hit timing,
- item scale/speed/damage changes remain driven by combat stats,
- overlay does not change hitbox geometry unless the combat packet explicitly says it should,
- AOE idol impact VFX declares and proves visual/damage alignment, including whether the effect shows the full area footprint or an approved marker/area split,
- weapon-plus-idol proof uses the generalized impact-chain diagnostic schema from `CombatVFXImpactContextContract.md`, not Water-only field names as permanent requirements,
- idol-owned damage/status proof includes `DamageBySource` or equivalent runtime evidence for the idol `SourceID`,
- neutral-control proof shows the target idol source does not emit diagnostics or damage when the idol is absent, replaced, disabled, or otherwise ineligible,
- gameplay capture proves base-only and base+idol states separately.
