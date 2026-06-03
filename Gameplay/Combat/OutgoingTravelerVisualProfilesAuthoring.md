# Outgoing Traveler Visual Profiles

This document defines the authoring contract for the actorless outgoing traveler renderer.

## Runtime selector

`FT66OutgoingTravelerFireParams::TravelerVisualProfileID` selects a visual slot in the single persistent `NS_OutgoingTravelerPool` Niagara system. The runtime pool uploads only compact per-live arrays:

- `User.TravelerPositions`
- `User.TravelerRotations`
- `User.TravelerScales`
- `User.TravelerColors`
- `User.TravelerMeshIndices`

The selector resolves to `Particles.MeshIndex`; no material or texture data is uploaded per frame.

## Slot model

Slots `0-3` are temporary projectile fallback profiles. Slots `4-19` are the element/delivery traveler profiles:

- `4-7`: Fire AOE, Pierce, Bounce, DOT
- `8-11`: Ice AOE, Pierce, Bounce, DOT
- `12-15`: Electricity AOE, Pierce, Bounce, DOT
- `16-19`: Nature AOE, Pierce, Bounce, DOT

Fire and Electricity use the additive parent family. Ice and Nature use the translucent parent family. Each slot owns its own mesh, material instance, and texture.

## What is hand-authored

For a final content pass, the authored source for one traveler profile is:

- profile ID, for example `TravelerVisual.Fire.AOE`
- delivery slot, for example AOE
- element family, for example Fire/additive
- source texture or generated source image for that specific element/delivery
- source mesh shape if the commandlet-generated mesh is not sufficient

The current commandlet can generate placeholder slot meshes and textures for all slots so runtime and performance work can proceed before final art.

## What the commandlet generates

Run the outgoing traveler commandlet with `-ProductionPool` to regenerate the production pool:

```powershell
UnrealEditor-Cmd.exe C:\UE\T66\T66.uproject -run=T66OutgoingTravelerSwarmVFX -ProductionPool -unattended -nop4
```

The commandlet generates or updates:

- `/Game/VFX/Foundation/OutgoingTravelers/Profiles/Meshes/SM_TravelerVisual_*`
- `/Game/VFX/Foundation/OutgoingTravelers/Profiles/Materials/MI_TravelerVisual_*`
- `/Game/VFX/Foundation/OutgoingTravelers/Profiles/Textures/T_TravelerVisual_*`
- `/Game/VFX/Foundation/OutgoingTravelers/NS_OutgoingTravelerPool`

The Niagara mesh renderers keep one array-driven system and select per-instance visual slots with `Particles.MeshIndex`. Renderers use the material assigned to each slot mesh, so every profile can carry its own material instance and texture without adding per-frame upload work.

## Official visual proof standard

Phase 2 is not accepted by a broad gameplay screenshot or by manifest counters alone. The proof must show the subject: the 16 element/delivery traveler profiles must be individually visible and distinguishable in a framed capture.

A valid Phase 2 visual proof has two parts:

- A 16-profile distinctness capture: run the outgoing-traveler visual gate with `Count=16`, `GridColumns=4`, proof camera enabled, hero hidden, and the fixed proof lane enabled. The fixed proof lane places the 4 by 4 grid on a deterministic off-map overhead proof board instead of deriving it from the gameplay camera. The current default proof lane center is `X=20000, Y=0, Z=9000` with `Pitch=-90`, which frames the grid from above on a clean background and keeps it clear of spawn-room walls/ceilings. The frame must show a readable 4 by 4 traveler grid, not just the spawn room, wall, or hero.
- A scale-cost capture: run the same gate at the requested stress count, normally 5,000 travelers, to report FPS, game-thread ms, GPU ms, draw calls, pool upload ms, pack ms, Niagara array upload ms, and pool simulation ms.

The manifest assertions are necessary but not sufficient:

- `failed_spawn_count == 0`
- `pool_diagnostics.dropped_total == 0`
- `pool_diagnostics.peak_live_count >= requested_live_count`
- `visual_profiles_used_count == 16` for the mixed-profile case
- `uses_single_niagara_system_visual_selector == true` for the mixed-profile case

The image assertion is also required: the selected proof frame must contain the 16 traveler profiles as the dominant subject. A frame that only shows the hero/spawn room, even with valid manifests, is a failed visual proof because it does not prove slot distinctness.

Recommended distinctness command:

```powershell
.\Scripts\RunOutgoingTravelerVisualProfilesGate.ps1 `
  -Runs 1 `
  -Count 16 `
  -GridColumns 4 `
  -Spacing 180 `
  -SpawnDistance 700 `
  -VisualScaleMultiplier 3 `
  -ProofCamera `
  -HideHeroForProof `
  -ProofCameraDistance 1800 `
  -ProofCameraFOV 45 `
  -ProofLaneCenterX 20000 `
  -ProofLaneCenterZ 9000 `
  -ProofLanePitch -90 `
  -ScreenshotDelaySeconds 38 `
  -PostCaptureScreenshotDelaySeconds 2 `
  -SampleSeconds 8
```

Recommended scale command:

```powershell
.\Scripts\RunOutgoingTravelerVisualProfilesGate.ps1 `
  -Runs 3 `
  -Count 5000 `
  -ProofCamera `
  -HideHeroForProof `
  -ProofCameraDistance 1800 `
  -ProofCameraFOV 45 `
  -ProofLaneCenterX 20000 `
  -ProofLaneCenterZ 9000 `
  -ProofLanePitch -90
```

If the manifest says the travelers are live but the frame does not show them, do not proceed to later phases. First verify that the fixed proof-lane camera is framing open space, then reduce the proof to one oversized visible traveler, then 16 separated profiles, and only then repeat the 5,000-traveler scale run.
