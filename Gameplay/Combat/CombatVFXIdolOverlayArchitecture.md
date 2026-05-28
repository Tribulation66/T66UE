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
- gameplay capture proves base-only and base+idol states separately.
