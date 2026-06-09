# Pending Issues - Gameplay Physics

## [Major] Hero 1 ragdoll feel and PhysicsAsset tuning are not production-final

The current implemented Hero 1 path is hit-triggered full ragdoll in `UT66HeroPhysicsComponent`: normal capsule/CharacterMovement play, full skeletal simulation on qualifying hit, actor XY follow toward pelvis during ragdoll, floor-aware capsule placement for get-up, get-up animation, and return to walking.

The current path is stable enough for PIE/standalone testing after the latest build and Hero 1 facing fixes, but it is still not final Fall Guys-like feel. The amount of launch, tumble, rebound, body readability, get-up timing, arm/limb visibility, PhysicsAsset mass/inertia balance, and obstacle displacement still need design tuning against multi-frame video proof.

This is out of scope for the docs cleanup because this pass only reorganizes current physics documentation and history. Fixing the remaining issue entails iterating the Hero 1 PhysicsFirst PhysicsAsset/body settings, `FT66HeroPhysicsProfile` values, wipeout-arm reaction settings, obstacle speed/shape, and proof camera framing, then rerunning `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof` until review confirms the intended feel.
