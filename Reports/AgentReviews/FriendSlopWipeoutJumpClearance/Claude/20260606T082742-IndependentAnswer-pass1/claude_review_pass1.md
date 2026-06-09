Result: OK

## Independent Answer

**Root cause.** The wipeout arm has no physics/overlap hit path — overlap events are disabled (`SetGenerateOverlapEvents(false)`, T66GameMode_TestRoom.cpp:567) and hits are decided entirely by the analytic test inside the trap timer lambda (lines 660–679). The false hit comes from the vertical gate:

```cpp
const bool bWithinVerticalBand = HeroLocation.Z <= (WipeoutArmCenterZ + WipeoutArmRadiusUU + 260.f); // line 669
```

Two problems make a visibly-cleared jump still register:
1. **Single-sided test with a large fudge.** It only checks an *upper* bound and adds `+260` uu. With `WipeoutArmCenterZ = 178` and `WipeoutArmRadiusUU = 92` (lines 278–280), the cylinder's top surface is at `178 + 92 = 270`, but the hit band extends up to `Z ≤ 530`. So the hero can be ~260 uu above the cylinder and still be "hit."
2. **Uses capsule center, not the hero's footprint.** `HeroLocation.Z` is the capsule center. The hero's feet are `Z − HalfHeight` (half-height ≈ 88–96 here). The test never subtracts half-height, so it compares the wrong reference against the arm.

The horizontal gates (`bWithinArmLength`, `bOutsideHub`, `bWithinArmRadius`) are fine; only the vertical gate is wrong.

**Safest fix.** Replace the loose upper-bound check with a true vertical-overlap test between the hero capsule's vertical extent and the arm's vertical extent:

- Arm vertical span: `[WipeoutArmCenterZ − WipeoutArmRadiusUU, WipeoutArmCenterZ + WipeoutArmRadiusUU]` (the cylinder is horizontal, so its radius is its vertical half-thickness).
- Hero vertical span: `[HeroLocation.Z − HalfHeight, HeroLocation.Z + HalfHeight]`, using `HeroCapsule->GetScaledCapsuleHalfHeight()` (already have `HeroCapsule` at line 660).
- Register a hit only if the spans overlap (with a small tolerance), i.e. cleared when `HeroBottom > ArmTop + Tolerance`.

Sketch:
```cpp
const float HeroHalfHeight = HeroCapsule ? HeroCapsule->GetScaledCapsuleHalfHeight() : 96.f;
const float HeroBottom = HeroLocation.Z - HeroHalfHeight;
const float HeroTop    = HeroLocation.Z + HeroHalfHeight;
const float ArmTop     = WipeoutArmCenterZ + WipeoutArmRadiusUU;
const float ArmBottom  = WipeoutArmCenterZ - WipeoutArmRadiusUU;
const bool bWithinVerticalBand =
    HeroBottom <= (ArmTop + VerticalTolerance) &&
    HeroTop    >= (ArmBottom - VerticalTolerance);
```

Keep `VerticalTolerance` small (e.g. ~10–20 uu) for jump-feel forgiveness, not the current 260. Per the repo's "prefer data-authored tuning over hardcoded C++ defaults" rule, expose it as a `t66.TestRoom.WipeoutArm*` CVar like the surrounding tuning knobs rather than a bare literal.

This corrects the trap collision logic at the root (geometry of the vertical test) rather than masking it.

## Evidence Checked
- T66GameMode_TestRoom.cpp:660–679 — analytic hit test; vertical gate at 669 is the defect.
- Lines 278–284 — `WipeoutArmCenterZ = 178`, `WipeoutArmRadiusUU = 92`, etc. (cylinder top ≈ 270).
- Lines 560–567, 595–602 — arm/hub collision is Block-only with overlaps disabled; no second hit path.
- Capsule half-height usage across codebase (~88–96 uu) confirms center-vs-feet mismatch.

## Questions Or Blockers
None requiring the user. The fix is a self-contained correction Codex can implement.

## Caveats
- Per repo rules, this is a gameplay runtime change: it needs a focused compile and a TestRoom proof, plus staged standalone validation (playable standalone is affected).
- Pick `VerticalTolerance` from in-game feel; if exposed as a CVar, keep the default conservative so cleared jumps read as clean but grazes still connect.
- I did not run the game; verdict is from static inspection of the live source. Confirm via TestRoom that a clearing jump no longer ragdolls and that a non-clearing pass still triggers.
