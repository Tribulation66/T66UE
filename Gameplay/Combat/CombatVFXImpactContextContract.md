# Combat VFX Impact Context Contract

**Status:** Required context and attribution contract for weapon and idol VFX packets that publish, consume, or chain combat impact events. This document does not make Niagara visuals authoritative for damage.

## Purpose

Use this contract whenever a combat VFX can create, consume, or trigger a damage/status event through a combat impact context. It applies to weapon base attacks, idol modifiers, projectile impacts, slash contacts, trails, auras, and future chained effects.

`CombatVFXVisualDamageAlignmentContract.md` owns where the visual and hitbox line up. This contract owns who published the context, who consumed it, whether downstream effects fired for every eligible impact, and whether damage attribution stayed on the correct source.

## Core Rule

Every weapon or idol damage source must have an official impact context with explicit source identity. A downstream idol effect must be driven by the weapon's impact context, must preserve the weapon as `ParentSourceID`, and must publish its own idol-owned impact context before applying idol-owned damage or status.

Video proof is never sufficient for this gate by itself. A capture can prove readability, but context wiring and damage ownership require runtime logs or validator output that show source identity, parent identity, impact points, parity counters, and damage-by-source records.

## Required Terms

Every applicable effect packet must define these terms:

- `Weapon impact context`: the context published by the weapon or weapon projectile when it reaches its authoritative impact or damage event.
- `Idol impact context`: the context published by the idol modifier after it consumes an eligible weapon context.
- `SourceType`: the source class, such as `WeaponBase`, `IdolModifier`, `Projectile`, or another declared combat source type.
- `SourceID`: the stable binding or data ID that owns the source, such as a weapon binding ID or idol ID.
- `ParentSourceID`: the upstream source that caused this context. Weapon base contexts normally use `None`; idol contexts triggered by weapons must use the weapon `SourceID`.
- `Impact point`: the contact or trigger point carried by this context. See `CombatVFXVisualDamageAlignmentContract.md` for alignment and footprint rules.
- `Damage source proof`: runtime proof that actual damage/status application used the source that owns the effect, not a visual placeholder or parent source.
- `Eligible weapon context`: a weapon impact context that should trigger the idol under the active equipment, element, proc, target, cooldown, and category rules.
- `Context parity`: expected downstream contexts versus actual downstream contexts for a captured attack sequence.
- `Skip/fallback counters`: explicit counts for missing weapon context, invalid impact point, disabled idol, neutral control, legacy fallback, or any other declared non-trigger path.

## Weapon Publication Requirements

Every weapon effect that can drive idol overlays or chained combat effects must publish a weapon impact context at the authoritative impact event. The packet must declare whether one attack produces:

- `OnePrimary`: one primary impact context per attack activation,
- `PerTarget`: one impact context per target hit,
- `PerProjectile`: one impact context per projectile impact,
- `PerPathSegment`: one impact context per path or beam segment,
- `PerChainLink`: one impact context per bounce/chain link,
- or another named policy with a clear expected-count rule.

Minimum proof fields:

```text
CombatImpactContext
  Phase=<WeaponPrimary/WeaponSecondary/...>
  SourceType=<WeaponBase/Projectile/...>
  SourceID=<weapon binding ID>
  ParentSourceID=<None or upstream source>
  AttackCategory=<AOE/DOT/Pierce/Bounce/...>
  ImpactPoint=<world xyz>
  ImpactPointValid=<0/1>
  DamageCenter=<world xyz>
  DamageShape=<sphere/sector/crescent/tube/path/...>
  DamageExtents=<radius/inner radius/half angle/length/...>
  HitTargetCount=<count>
```

If the weapon visual is centered somewhere other than the impact point, the context still uses the weapon's official impact point. Visual placement differences are handled by the visual/damage alignment contract, not by moving the context back to a convenient actor or visual origin.

## Idol Consumption Requirements

Every idol modifier that owns damage or status must consume an eligible weapon context and then publish its own idol context. The idol context must not be a renamed weapon context.

Minimum proof fields:

```text
CombatImpactContext
  Phase=<IdolPrimary/IdolSecondary/...>
  SourceType=IdolModifier
  SourceID=<idol ID>
  ParentSourceID=<weapon SourceID>
  TriggeredByImpactPoint=<weapon impact point>
  ImpactPoint=<idol-owned world xyz>
  ImpactPointValid=<0/1>
  DamageCenter=<idol-owned world xyz>
  DamageShape=<sphere/sector/crescent/tube/path/...>
  DamageExtents=<idol radius/extents>
  HitTargetCount=<count>
```

Idol damage/status proof must include the idol's own source ID, for example:

```text
DamageBySource SourceID=<idol ID>
```

An idol impact point is required even when no later chain currently consumes it. Future chaining must be able to follow the idol context without inferring from the weapon context or visual spawn.

## Generalized Chain Diagnostic Schema

Effect-specific proof may add fields, but every weapon-plus-idol proof must map to this reusable schema:

```text
CombatImpactChainDiagnostic
  SourceID=<idol ID or downstream source ID>
  ParentSourceID=<weapon SourceID>
  ContextParity=<PASS/FAIL>
  WeaponImpactContexts=<count>
  EligibleWeaponImpactContexts=<count>
  ExpectedDownstreamImpactContexts=<count>
  DownstreamImpactContexts=<count>
  SkippedNoWeaponContext=<count>
  SkippedInvalidImpactPoint=<count>
  SkippedDisabledOrNeutral=<count>
  LegacyFallbacks=<count>
  DamageByDownstreamSource=<count or PASS/FAIL>
```

`ContextParity=PASS` means:

- every eligible weapon context produced the expected downstream idol context,
- no required downstream context was missing,
- skip/fallback counters match the packet's declared rules,
- no legacy fallback path supplied the proof,
- and damage/status attribution was recorded against the downstream source when the downstream source owns damage/status.

Water-specific fields such as `ExpectedWaterIdolImpactContexts` are allowed only as compatibility or worked-example fields. Future proof must either use the generalized field names or explicitly map effect-specific names back to this schema.

## Neutral Control

Every idol proof needs a neutral or negative control unless the packet records why one is impossible and the user approves that exception.

The neutral control must prove the themed idol did not trigger when it should not have triggered. Valid controls include:

- same weapon with no idol equipped,
- same weapon with a different idol that should not produce the tested source ID,
- same weapon and idol with the proc gate disabled,
- or another declared inactive condition.

Minimum neutral-control proof:

- target idol diagnostic is absent or reports zero eligible downstream contexts,
- target idol `DamageBySource` is absent or zero,
- weapon context still publishes when the weapon attack lands,
- any skipped/neutral counter matches the packet's declared expectation.

## Effect Packet Block

Every applicable effect packet must include:

```text
IMPACT CONTEXT CONTRACT
Role: WeaponPublisher / IdolConsumer / DownstreamPublisher / None
Weapon context publication policy:
Eligible context rule:
Expected downstream context count:
SourceType:
SourceID:
ParentSourceID rule:
Impact point rule:
Damage source proof:
Neutral control:
Diagnostic schema:
Legacy fallback allowed: YES/NO
Video-only proof accepted: NO
Intentional exceptions:
```

If `Role` is `None`, the packet must explain why the effect has no damage, status, trigger, or chain context.

## Close Template

Use this close whenever the effect publishes or consumes an impact context:

```text
IMPACT CONTEXT CLOSE
Weapon context publication: PRESENT/ABSENT/DEFERRED/N/A
Idol context consumption: PRESENT/ABSENT/DEFERRED/N/A
Downstream context parity: PASS/FAIL/N/A
Damage source identity: PASS/FAIL/N/A
Neutral control: PASS/FAIL/N/A
Legacy fallback count:
Evidence:
Discriminator test:
Reported status: FULL/PARTIAL
```

`FULL` requires every required context, parity, damage-source, and neutral-control item to pass with evidence. Any missing required item, unmapped effect-specific diagnostic, legacy fallback proof, or video-only proof makes the context gate `PARTIAL`.

## Worked Reference

The current Hero 1 AOE plus Water idol proof is a worked reference only:

- the weapon context is `SourceType=WeaponBase` and `SourceID=Hero_1_black_aoe`,
- the idol context is `SourceType=IdolModifier`, `SourceID=Idol_Water`, and `ParentSourceID=Hero_1_black_aoe`,
- `DamageBySource SourceID=Idol_Water` proves idol-owned damage attribution,
- the Water diagnostic proves one eligible weapon context produced one Water idol context,
- the Earth neutral run proves the Water diagnostic is absent when Water is not the equipped idol.

Future weapon or idol work must not require Water-specific names. Use the Water proof as an example of the schema and the neutral-control pattern, not as a permanent field vocabulary.

## Anti-Lookalike Rule

The cheapest wrong result is a video where an idol visual appears after an attack, but the visual was spawned from the hero, from a placeholder, from a legacy fallback, or from the weapon damage source instead of an idol-owned context.

The discriminator is runtime proof that shows:

- the weapon published the expected impact context,
- the idol consumed that context and preserved `ParentSourceID`,
- the idol published its own context and impact point,
- the expected and actual context counts match,
- skip/fallback counters are zero or explicitly explained,
- and damage/status attribution uses the idol `SourceID` when the idol owns damage/status.
