# Hero Active-Ragdoll Stage 3 Authority Rebuild Proof Summary

Date: 2026-06-07

## Scope

Rebuild Stage 3 Hero 1 active ragdoll around the agreed authority model:

- capsule remains gameplay/input/navigation authority
- skeletal mesh component uses kinematic component-transform authority
- Chaos simulates pelvis and body chain below the component
- hip anchor constrains simulated pelvis to the capsule
- child-body PAC acts as pose muscle, not gameplay authority
- obstacle response applies simulated-body impulse plus bounded capsule shove

## Current Proof

Primary infrastructure video:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\heroactiveragdollproof_reaction_testroom_high_no_runtime_resync.mp4
```

Primary frames:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\frames_reaction_testroom_high_no_runtime_resync\
```

Primary log snapshot:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\T66_heroactiveragdollproof_high_no_runtime_resync.log
```

Supplemental movement telemetry:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\hero_movementqa_active_ragdoll_no_runtime_resync_60f.mp4
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\T66_hero_movementqa_active_ragdoll_no_runtime_resync_60f.log
```

The capture command used `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof`; this mode now injects:

```text
-T66AutomationTestRoom
-T66AutoCaptureHeroHPOverride=20000
```

## Log Gates Passed

From the high-angle proof log snapshot:

- TestRoom proof route active through `-T66AutomationTestRoom`
- Wipeout arm scheduled: `TestRoom wipeout arm trap scheduled at V(Z=178.00)`
- Active pelvis/body simulation initialized: `PelvisSimulating=1`
- Active-ragdoll reaction applied: `Reaction Applied=1 Source=TestRoomWipeoutArm`
- Active path used: `ActiveTried=1 ActiveApplied=1 LegacyApplied=0`
- Legacy TestRoom fallback PAC is explicitly labeled as legacy profile logging: `LegacyProfilePAC=0 LegacyDriveMode=0`
- Recovery path exercised: `State Balanced -> KnockedDown`, `KnockedDown -> Recovering`, and `Recovering -> Balanced`
- No `EmergencyPelvisResync`, fatal, or player-death markers were found in the final proof log scan

## Important Runtime Interpretation

During active body simulation, Unreal can report the skeletal mesh parent as `None`. That is acceptable in this model when:

- the component transform remains kinematic
- the capsule remains gameplay authority
- effective pelvis/capsule distance stays bounded
- shutdown restores the pre-active mesh relative transform

Raw body distance can look large because Chaos body readback can be component-relative. The proof and runtime debug now compare raw and component-transformed pelvis positions and use the effective distance nearest the capsule hip anchor.

The hip-anchor constraint is the steady-state pelvis positioner. The mesh/capsule sync keeps the component frame coherent with the gameplay capsule; it is not an actor-follows-pelvis loop and it must not replace the anchor constraint as the physics mechanism.

The proof camera is still a harness limitation. The high-angle and wide-angle captures are useful for confirming no old long-line flattening artifact, but the yellow wipeout arm can occlude Hero 1 during the best contact frames and the shoved hero can leave the most readable part of the view. Treat the current MP4 as infrastructure proof, not final subjective feel proof.

Normal-movement stretch/spazz is not visually accepted by this packet. The supplemental MovementQA log is telemetry evidence only: it shows mesh/capsule sync, no `EmergencyPelvisResync`/fatal markers, harness-teleport divergence warnings that explicitly avoid runtime body teleport, and later return to bounded effective pelvis/capsule distance.

## Remaining Risk

The infrastructure is stable enough for gameplay testing, but the Fall-Guys-like feel is not final. Next tuning should focus on PhysicsAsset body shapes/mass/inertia, hip-anchor strength/limits, pose-drive strength, rebound, recovery timing, and proof camera framing.
