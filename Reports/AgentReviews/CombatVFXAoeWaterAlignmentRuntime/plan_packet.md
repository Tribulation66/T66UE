# Combat VFX AOE / Water Alignment Runtime Plan

## Working Goal

Align the Hero 1 AOE weapon visual and Water idol placeholder with their authoritative damage footprints, then verify the result with the repo Unreal gameplay video capture process.

## Roles

- Operator: Codex in the active workspace.
- Validator: Claude Code CLI through `Scripts/Invoke-ClaudePlanReview.ps1`.
- Requested model: `claude-opus-4-8`.
- Claude permissions: read-only plan review through the helper.

## Tier And Scope

- Tier: Tier 1.
- In scope: runtime combat/VFX alignment code for the Hero 1 AOE weapon production spawn and the temporary Water idol placeholder; focused compile; gameplay video capture using `Scripts/CaptureT66GameplayVideo.ps1`.
- Out of scope: final Water Niagara authoring, redesigning the Hero 1 AOE Niagara visual, Mini/minigame systems, normal item acquisition proof, and changing idol AOE delay semantics.

## Applicable Instructions

- `AGENTS.md`: working goal, Tier 1 review, PPF, Unreal-owned video capture, no Mini scope.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay runtime ownership and compile/build verification.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: Combat VFX process and visual/damage alignment gate.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: authoritative damage center, impact point, visual anchor, footprint mapping, marker versus area read, log/capture evidence.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: Hero 1 AOE crescent-band worked example.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`: Water idol is an additive idol-owned impact/damage source triggered from the weapon impact context.

## Current Repo Findings

- Hero 1 AOE damage authority is `BuildSlashTargets(...)` with a frontal sector centered at `SlashCenter`, outer radius `EffectiveSlashRadius`, inner radius `EffectiveSlashInnerRadius`, and forward vector `SlashForward`.
- The current weapon impact context for hollow Hero 1 AOE publishes `ImpactPoint = SlashCenter + SlashForward * ((Inner + Outer) * 0.5)`, so idols trigger at the visible crescent band instead of at the hollow center.
- The generated slash mesh in `T66Hero1AxeAOEVFXCommandlet.cpp` is built around the local origin with inner radii roughly `222..310` and outer radii roughly `338..382`; its support/impact emitters use forward offsets around `286..320`. Therefore the production Niagara pivot should stay at the damage center while the visual impact/support read is band-forward.
- The current Water placeholder uses `/Engine/BasicShapes/Sphere`, spawns at `IdolImpactContext.ImpactPoint + Z76`, and uses fixed scale `0.85`, so its visual radius is about `42.5` Unreal units while `Idol_Water` damage radius is `300`.

## PPF CHECK

Objective: align runtime visual placement/scale for the existing Hero 1 AOE production effect and temporary Water idol proof placeholder against authoritative combat hitboxes.

Proven process: `CombatVFXVisualDamageAlignmentContract.md` plus the existing Hero 1 AOE effect packet and `Scripts/CaptureT66GameplayVideo.ps1`.

My planned implementation:

- Add explicit `DamageCenter` and `bDamageCenterValid` fields to `FT66CombatImpactContext`.
- Populate weapon contexts with their damage center where the runtime knows it; for Hero 1 AOE, `DamageCenter = SlashCenter` and `ImpactPoint = band midpoint`.
- Keep Hero 1 AOE production Niagara pivot at `DamageCenter`, because the mesh is center-pivoted and its visible carrier/support geometry already extends forward into the annulus. Add spawn logs naming `DamageCenter`, `ImpactPoint`, `VisualPivot`, `VisualAnchorModel`, and `ImpactOffsetFromDamageCenter` so future weapons cannot hide pivot/impact mismatches.
- Populate Water idol impact contexts with `DamageCenter = ImpactPoint`, because Water is currently a spherical idol-owned damage source.
- Change the Water placeholder from compact marker to temporary area-read proof sphere: spawn at the idol damage center, scale the engine sphere by `Radius / 50.0`, and log `VisualRadius`, `DamageCenter`, `ImpactPoint`, `VisualScale`, and the placeholder type.
- Capture `hero1axeaoewateridolimpact` with debug damage volumes and impact-source verbose logs enabled.

Same method class: YES.

If NO, why: N/A.

User approval required before proceeding: NO, assuming validator approval. This does not substitute a new visual method; it aligns existing runtime presentation and a user-approved temporary blue sphere proof marker with the damage contract.

Verification evidence:

- Focused C++ compile for `T66Editor Win64 Development`.
- Gameplay capture with `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact -EvidenceBundle`.
- Evidence bundle with MP4, contact sheet, selected frames, ffprobe JSON, and visibility checklist.
- Runtime log proof containing weapon `CombatImpactContext`, weapon `CombatVFXProductionSpawned`, Water `CombatIdolWaterImpactResolved`, Water placeholder spawn, source damage, and target hit/miss results.

## ARTIFACT PARITY GATE

Reference artifact/category: Hero 1 AOE production Niagara slash carrier.
Role: Primary.
Required: YES.
Planned artifact/path: existing `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`; no new Niagara asset.
Status: SAME.
Evidence: runtime `CombatVFXProductionSpawned` log and capture frame range.

Reference artifact/category: Water idol temporary placeholder.
Role: Secondary proof artifact.
Required: YES for this structure proof only.
Planned artifact/path: runtime blue `/Engine/BasicShapes/Sphere` spawned by `SpawnWaterIdolImpactPlaceholderVFX`.
Status: EQUIVALENT for temporary area-read proof; not final idol Niagara.
Evidence: user explicitly requested a simple blue sphere for structure proof; runtime log and capture must show the sphere centered on the Water damage source and scaled to the Water radius.

Reference artifact/category: DamageVolume debug overlay.
Role: Primary evidence carrier for authority.
Required: YES.
Planned artifact/path: existing combat debug draw from `BuildSlashTargets`.
Status: SAME.
Evidence: capture with `T66.Combat.DebugView 2`.

## MECHANISM MANIFEST

Reference/source: `CombatVFXVisualDamageAlignmentContract.md`, Hero 1 AOE packet, current runtime code.

Required mechanisms:

1. Mechanism: damage-center/impact-point separation for irregular weapon shapes.
   Required: YES.
   Planned implementation: add/log `DamageCenter` separately from `ImpactPoint`; keep Hero 1 AOE damage center at `SlashCenter` and band impact at annulus midpoint.
   Evidence needed: verbose `CombatImpactContext` and `CombatVFXProductionSpawned` logs.

2. Mechanism: visual pivot remains tied to the authored carrier pivot.
   Required: YES.
   Planned implementation: spawn Hero 1 AOE Niagara at `DamageCenter`, because the mesh local origin is the sector center and the visible carrier/support is authored forward from that pivot.
   Evidence needed: log fields `VisualPivot`, `VisualAnchorModel`, `ImpactOffsetFromDamageCenter`, plus capture showing slash and sector together.

3. Mechanism: Water idol area-read placeholder footprint mapping.
   Required: YES for this proof placeholder.
   Planned implementation: scale engine sphere by `Radius / 50.0` and center it at `IdolImpactContext.DamageCenter`.
   Evidence needed: placeholder log `VisualRadius=Radius`, capture with Water sphere and DamageVolume.

4. Mechanism: idol damage source remains independent.
   Required: YES.
   Planned implementation: preserve Water source ID, parent source ID, own hit target list, own damage application, and source damage logs.
   Evidence needed: `CombatIdolWaterImpactResolved`, `DamageBySource SourceID=Idol_Water`, and target hit/miss proof.

5. Mechanism: temporal video proof.
   Required: YES.
   Planned implementation: use the Unreal-owned gameplay capture script with retained evidence bundle.
   Evidence needed: MP4, contact sheet, selected frames, ffprobe.

Cheap wrong result: a small blue sphere at a plausible contact point while the real Water damage sphere is much larger or offset.

Discriminator: the placeholder log reports `VisualRadius` equal to the Water `Radius`, and same-frame capture shows the blue sphere centered on the Water DamageVolume instead of reading as a compact marker or hero-centered effect.

## Risks And Rollback

- A fully radius-sized opaque blue sphere may obscure some enemies because it is a temporary proof sphere. That is acceptable for structure proof but not final Water art.
- If the capture makes the opaque area too unreadable, the follow-up should author a real Water Niagara area read or a translucent material through the normal VFX process, not shrink the damage footprint marker again.
- Rollback is limited to the touched C++ lines in `T66CombatComponent.h`, `T66CombatComponent.cpp`, and `T66CombatVFX.cpp`.

## Proposed Files

- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp`

## Review Request

Please review this plan for contradictions with the repo instructions, missed runtime seams, unsafe assumptions about the Hero 1 AOE mesh pivot versus impact point, insufficient verification, or any reason the planned Water placeholder should not be treated as a temporary area-read proof artifact.
