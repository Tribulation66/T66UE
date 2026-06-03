# Chat 2 Combat Content Plan Review Decision Gate

## Status

[Blocker] Stream B depends on a Foundation traveler API that is not present in the live repo.

## What's Wrong

The implementation plan requires Combat Content to consume:

- `FireOutgoingTraveler(const FT66OutgoingTravelerFireParams&)`
- `CancelOutgoingTraveler(...)`
- `FT66OutgoingTravelerFireParams`

Live repo search found none of these symbols in `Source/T66`, `Gameplay`, `Content`, or `Reports`. The only live traveler-related match is unrelated backend sample data, while current hero/idol projectile presentation still uses `T66TemporaryProjectileSystem`, `AT66HeroProjectile`, and existing combat VFX paths.

Because the prompt explicitly says Foundation owns the traveler API and Combat Content must consume it rather than build its own renderer, Stream B cannot be implemented or proven as written until that dependency is resolved.

## Decision Needed

Choose one before implementation proceeds:

1. Wait for the Foundation traveler API to land, then run this plan review again against the merged API.
2. Proceed only with non-traveler-dependent work, such as Stream A and the layout-only parts of Stream C, while explicitly deferring Stream B and any final "idol travelers routing through Foundation API" proof.
3. Explicitly approve a temporary compatibility wrapper over the existing projectile/VFX system. This is a method substitution and should be treated as a scoped exception to the Foundation-consumption rule.

## Current Recommendation

Use option 1 if the Foundation chat is actively landing the API soon. Use option 2 only if you want parallel progress and accept that the integrated idol traveler gate remains blocked. Avoid option 3 unless speed matters more than preserving the planned architecture.

## Evidence

- `rg -n -F -e "FT66OutgoingTravelerFireParams" -e "FireOutgoingTraveler" -e "CancelOutgoingTraveler" -e "Traveler" Source Gameplay Content Reports`
- `Source/T66/Gameplay/T66CombatComponent.cpp` includes and uses `T66TemporaryProjectileSystem`.
- `Source/T66/Gameplay/T66HeroProjectile.*` still owns current hero projectile presentation.
- `Source/T66/Gameplay/T66ProjectileManagerSubsystem.*` owns current enemy/boss projectile manager paths, not the requested Foundation outgoing traveler API.
