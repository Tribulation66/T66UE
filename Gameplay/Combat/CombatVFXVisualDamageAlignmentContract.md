# Combat VFX Visual Damage Alignment Contract

**Status:** Required alignment contract for combat VFX packets and production binding reviews. This document does not make visual meshes or Niagara collision authoritative for damage.

## Purpose

Use this contract whenever a weapon, idol, projectile, slash, aura, trail, or impact VFX has a combat hitbox, damage query, or impact context. The goal is to prevent a visual from reading smaller, larger, behind, or offset from the damage it represents unless that mismatch is intentional, documented, and approved.

## Core Rule

Damage geometry remains authoritative and lives in combat logic, such as `FT66CombatImpactContext`, combat queries, projectile damage primitives, or explicit `DamageVolume` debug draws. Niagara collision, render mesh geometry, material opacity, particle size, and temporary visual mesh scale are presentation only.

This contract governs visual and damage geometry alignment. Source identity, parent identity, weapon-to-idol trigger parity, neutral controls, and damage-by-source proof are governed by `CombatVFXImpactContextContract.md`.

Production combat VFX must do one of two things:

- match the authoritative damage location and footprint within the effect packet's declared tolerance, or
- declare an intentional mismatch, explain why it improves readability or gameplay, provide the alternate visible telegraph or marker policy, and record user approval before the result can be called `FULL`.

If neither condition is true, report the result as `PARTIAL`.

## Required Terms

Every effect packet with damage or impact behavior must define these terms for the effect:

- `Authoritative damage center`: the world point used by the actual damage query or damage primitive.
- `Impact point`: the contact or trigger point carried by the impact context. It may differ from the damage center for irregular shapes, such as a crescent-band AOE whose idol trigger should originate at the band instead of at the query center. When the impact point drives an idol or downstream source, the owning packet must also satisfy `CombatVFXImpactContextContract.md`.
- `Damage shape type`: sphere, frontal sector, crescent band, tube, line, box, capsule, path, chain, or another explicitly described query shape.
- `Damage extents`: radius, inner radius, half-angle, length, tube radius, box extent, capsule half height/radius, path width, or chain-link radius.
- `Visual anchor`: the world point the visible carrier is authored around at runtime.
- `Visual pivot`: the mesh, ribbon, sprite, beam, Niagara system, or actor/component pivot used to place the carrier.
- `Visual offsets`: fixed planar offset, fixed Z lift, socket offset, path offset, or any parent-relative transform applied after the authoritative point is chosen.
- `Visual footprint`: the visible radius, width, length, arc band, tube width, trail width, or readable area implied by the carrier at gameplay camera distance.
- `BaseVisualRadius`: the calibration baseline used to map an authoritative damage radius to a runtime visual scale. It is a calibration value, not a cosmetic guess.
- `VisualScaleMultiplier`: an additional approved multiplier applied after the damage-to-visual calibration.
- `Alignment tolerance`: the allowed distance or percentage difference between the visual footprint and authoritative damage footprint, and between the visual anchor and intended impact point.

## Anchor Taxonomy

Each effect packet must choose exactly one primary anchor model for each primary VFX carrier:

| Anchor model | Meaning | Common use |
|---|---|---|
| `CenterAnchored` | Visual is built around the authoritative damage center. | Filled AOE sphere, pulse, aura, ground field. |
| `ImpactAnchored` | Visual is built around the impact point or contact point. | Impact burst, explosion, hit flare, idol impact trigger. |
| `BandAnchored` | Visual is built around a point on or near a band/ring/annulus instead of the geometric center. | Crescent-band slash, hollow AOE, ring-edge burst. |
| `PathAnchored` | Visual is built along a start/end path, beam, ribbon, hop, or chain. | Pierce beam, bounce hop, projectile trail, slash sweep path. |

If a secondary carrier uses a different anchor model, record it separately in the packet. Do not silently mix anchor models inside one effect.

## Footprint Mapping

Every production binding or effect packet must declare how the visual footprint maps to the authoritative extents.

For radius-based VFX, the normal calibration rule is:

```text
VisualScale = (EffectiveDamageRadius / BaseVisualRadius) * VisualScaleMultiplier
```

If the effect does not use that formula, the packet must state the alternate mapping and why it is correct for the carrier. A fallback scale based only on rarity, `ProjectileScaleMultiplier`, or a hand-tuned constant is not enough for final AOE or projectile footprint acceptance unless the packet explains why no direct damage-footprint mapping applies.

For irregular shapes, the packet must map the readable visual edge to the actual query edge:

- sectors must declare outer radius, inner radius when present, and half-angle,
- crescent bands must declare inner/outer band read and where the impact point sits inside the band,
- beams/tubes must declare length and readable width against tube radius,
- boxes/capsules must declare visible length/width/height against the damage primitive,
- chain and bounce effects must declare each link's start/end, link radius, and impact points.

## Marker And Area Read Comparison

An impact marker is not the same thing as an area/damage read.

If an idol or weapon source applies AOE damage, the final production visual must provide one of these:

- a readable area footprint that matches the authoritative AOE extents within tolerance, or
- an explicitly approved split where a compact impact marker is paired with a separate telegraph or area read that communicates the full damage footprint.

A compact sphere, flare, splash, or burst over a large damage radius is acceptable as a temporary proof marker only when the packet calls it temporary and does not claim final visual/damage alignment. The marker must not be used to imply the final AOE footprint is approved.

## Tolerance

The effect packet must set alignment tolerance before acceptance capture. Use the smallest practical tolerance for the effect family. If there is no effect-specific reason to widen it, start from:

- visual anchor within `50` Unreal units of the declared impact point for impact markers,
- visible AOE outer edge within `10%` of authoritative outer radius at gameplay camera distance,
- visible inner hollow edge within `10%` of authoritative inner radius for crescent-band or ring effects,
- visual path endpoint within `50` Unreal units of path start/end or target impact points,
- no visible carrier center drift that makes the effect read as player-centered when the damage or impact is target/band-centered.

Any mismatch outside the declared tolerance must be recorded as intentional, approved, and supported by a readable alternate telegraph; otherwise the gate is `PARTIAL`.

## Required Evidence

Alignment proof must include both structural and visual evidence:

- the effect packet's alignment block with the terms above filled in,
- same-frame or same-frame-range gameplay capture showing the VFX and combat debug `DamageVolume` overlay together,
- log proof of source type, source ID, parent source ID when applicable, impact point, visual spawn location, damage radius/extents, `BaseVisualRadius`, `VisualScale`, and hit/miss or damage results,
- editor-isolation evidence when the effect packet requires pure shape comparison without gameplay clutter,
- explicit note when a temporary marker is intentionally not showing the full damage footprint.

The capture must make the target size, viewing angle, occlusion, and timing adequate for judging size and position. A still frame can support footprint placement, but temporal effects still need a frame range.

## Worked Reference

Hero 1 AOE is the current worked example for an irregular weapon VFX:

- the damage authority is a query-only frontal sector with an optional inner hollow,
- `Hero_1_black_aoe` uses `AoeInnerRadiusRatio=0.54`,
- the production binding uses `BaseVisualRadius=411.4` to calibrate visual scale against the effective outer radius,
- the crescent-band proof captures the slash VFX and debug DamageVolume together,
- the Water idol structure proof demonstrated why an idol impact point can be band-anchored even when the base weapon visual still spawns from its own slash center.

Use `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md` as a reference example, not as generic boilerplate. Future weapons and idols must declare their own anchor model, footprint mapping, tolerance, and evidence. The Water idol proof is only a worked example for impact-context wiring; future idols must use the generalized schema in `CombatVFXImpactContextContract.md` rather than hard-coding Water-specific diagnostic names as requirements.
