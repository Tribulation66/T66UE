# Combat VFX Water Alignment Delta Plan

## Working Goal

Align the Hero 1 AOE weapon visual and Water idol placeholder with their authoritative damage footprints, then verify the result with the repo Unreal gameplay video capture process.

## Reason For Delta

The first capture after the approved runtime patch produced the intended Water runtime behavior:

- weapon impact context logged `DamageCenter=V(X=360.00, Z=64.00)`,
- weapon impact point logged `ImpactPoint=V(X=696.89, Z=64.00)`,
- Water idol context logged `DamageCenter=ImpactPoint=V(X=696.89, Z=64.00)`,
- Water placeholder logged `Radius=300.00`, `VisualRadius=300.00`, and `VisualScale=6.000`.

However, the Water-specific proof harness still expected the target labeled `OutsideRadius` at `Primary + Forward*520` to remain unhit. That target is outside the weapon AOE but inside the correctly centered Water radius, so the proof harness now marks a valid Water hit as `Result=FAIL`.

## Delta Scope

Add one capture-harness-only source file to the affected scope:

- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`

Do not change gameplay damage logic for this delta.

## Planned Delta

In the `hero1axeaoewateridolimpact` target list for `Idol_Water` only:

- rename the `Forward*520` target to `WaterOnlyOuterRadius` and set `ExpectedHit=true`, because it is outside the weapon outer radius but inside the Water idol sphere,
- add a new `OutsideAllRadius` target farther forward at `Forward*760` with `ExpectedHit=false`, proving the Water sphere still has a finite boundary,
- keep existing false targets outside angle and behind the Water sphere.

## Verification

- Recompile `T66Editor Win64 Development`.
- Recapture `hero1axeaoewateridolimpact` with `Scripts/CaptureT66GameplayVideo.ps1 -EvidenceBundle`.
- Confirm all target result logs pass, including `WaterOnlyOuterRadius ActualHit=1 Result=PASS` and `OutsideAllRadius ActualHit=0 Result=PASS`.
- Confirm Water placeholder still logs `VisualRadius=300.00`, `VisualScale=6.000`, and `DamageCenter=ImpactPoint`.

## Review Request

Please review whether this capture-harness update is the correct response to the new, intended Water radius behavior, or whether the failing `OutsideRadius` target should instead be treated as evidence that the runtime Water radius/center is wrong.
