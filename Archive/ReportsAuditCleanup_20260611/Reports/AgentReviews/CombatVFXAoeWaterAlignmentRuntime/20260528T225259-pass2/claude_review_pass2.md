Verdict: APPROVE

## Blockers
None.

## Major Issues
None. The delta is correctly scoped to the capture harness only (`Source/T66/Gameplay/T66PlayerController_Overlays.cpp`) and explicitly excludes gameplay damage logic, which is the right call given the runtime evidence.

## Minor Issues
- The packet asserts the geometry qualitatively ("outside weapon outer radius but inside the Water sphere") but does not show the arithmetic. From the logged values it is self-consistent: weapon center `X=360`, Water center `ImpactPoint X=696.89`, Water `Radius=300`. A target at `Primary + Forward*520` (≈`X=880`) sits ~183 from the Water center (inside 300) and ~520 from the weapon center (outside the ~337 weapon footprint), so `ExpectedHit=true` is correct. The new `Forward*760` target (≈`X=1120`) sits ~423 from the Water center (outside 300), so `ExpectedHit=false` is correct. Recommend Codex confirm the actual `Primary` origin and `Forward` axis in code rather than inferring from X-only logs, since the assertion depends on Forward being aligned with +X.
- "keep existing false targets outside angle and behind the Water sphere" — ensure the behind-sphere target is still beyond `Radius=300` from the new center after re-centering, not just behind the old center.

## Clarifying Questions
- Is `Primary` the weapon `DamageCenter (X=360)` or the idol `ImpactPoint (X=696.89)`? The geometry conclusion holds only if `Forward*520/760` is measured from the weapon-side primary. Confirm before renaming.

## Required Verification
- Recompile `T66Editor Win64 Development`.
- Recapture `hero1axeaoewateridolimpact` via `Scripts/CaptureT66GameplayVideo.ps1 -EvidenceBundle`.
- Confirm `WaterOnlyOuterRadius ActualHit=1 Result=PASS` and `OutsideAllRadius ActualHit=0 Result=PASS`.
- Confirm Water placeholder still logs `VisualRadius=300.00`, `VisualScale=6.000`, `DamageCenter=ImpactPoint`.
- Confirm the retained "outside angle" and "behind sphere" false targets still resolve `ActualHit=0`.

## Rationale
The stated goal is to align the visual/idol overlay with the authoritative damage footprint. The runtime capture already demonstrates that alignment: `DamageCenter=ImpactPoint`, `VisualRadius=Radius=300`, `VisualScale=6.000`. The failing `OutsideRadius` target is a stale harness expectation written before the center was correctly aligned — it now correctly falls inside the intended Water sphere, so flipping it to `ExpectedHit=true` and adding a true out-of-bounds target restores a valid boundary proof rather than masking a runtime defect. The change touches only the capture harness, carries concrete pass/fail verification that will surface any geometry error, and does not alter gameplay damage logic — safe for Codex to proceed.

