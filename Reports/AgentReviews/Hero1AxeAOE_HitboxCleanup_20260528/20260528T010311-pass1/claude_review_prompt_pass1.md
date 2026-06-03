You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\Hero1AxeAOE_HitboxCleanup_20260528\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Hero 1 Axe AOE Hitbox Alignment And VFX Cleanup Plan Packet

## Working Goal

Align the Hero 1 axe AOE VFX hitbox/debug outline with the visual, clean and organize the VFX process/assets/docs for future agents, and prepare a new-agent handoff for item-stat confirmation and future idol overlay VFX work.

## User Constraints

- Current visual design is accepted enough for now; do not continue visual-design polish in this pass.
- Focus first on the hitbox matching the visual with the debug line around it.
- Then clean loose ends and organize the VFX tree/docs so future agents can follow the system.
- Prepare a new-agent prompt for later item/stat confirmation, scale confirmation, and idol overlay VFX design.
- Do not implement idol overlays or final item/stat proof in this pass.
- Default Mini/minigame scope remains excluded.

## Applicable Instructions Read

- `AGENTS.md`: active goal, live repo first, no process substitution, PPF/artifact/mechanism gates, Claude review gate, Unreal-owned capture, pending issues.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay folder routing, combat VFX procedure, data-authored tuning, runtime verification expectations.
- `Gameplay/README.md`: combat docs are under `Gameplay/Combat`.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: combat VFX source/process authority, editor/gameplay evidence, hitbox proof separation.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: current VFX binding/capture/inventory state.
- `Gameplay/Combat/pending_issues_Combat.md`: existing known VFX proof/capture limitations.
- `Reports/AGENTS.md`: review/handoff reports go under `Reports/AgentReviews`, proof/handoff summaries under `Reports/Proof` or durable report paths.

## Current Live Findings

- `UT66CombatComponent::PerformSlash` computes `EffectiveSlashRadius` from combat/weapon/item scale and calls `BuildSlashTargets(..., InnerRadius=0.f)`.
- `BuildSlashTargets` already supports an `InnerRadius` and passes it to both `T66IsPointInsidePlanarSector` and `T66CombatDebugDraw::DrawDamageSector`.
- The current production AOE VFX mesh is a crescent band, not a filled sector. Its C++ commandlet profile has approximate base inner/outer radii:
  - inner: about `222 cm` minimum before renderer layer scale,
  - outer: about `382 cm` maximum before renderer layer scale,
  - largest slash renderer scale: `1.08`, so primary outer carrier is about `413 cm`.
- `Content/Data/CombatVFXBindings.csv` currently has `BaseVisualRadius=435.0`, so production VFX scale is close but not tied to the actual carrier extent.
- Existing target proof positions assume a filled sector. If the hitbox becomes a visual crescent band, proof targets must be repositioned to include inside-band and inside-hole cases.

## PPF Check

```text
PPF CHECK
Objective: Make the Hero 1 axe AOE logical damage hitbox/debug line match the accepted crescent slash visual, then clean docs/handoff for future VFX work.
Proven process: AGENTS.md Section 2 + Gameplay/Combat/CombatVFXAuthoringProcedure.md + Hero1AxeAOESlashMechanismPacket.md hitbox proof and production-binding close.
My planned implementation: Keep Niagara as presentation and combat as authority. Add combat-owned crescent-band geometry metadata to weapon data for the Hero 1 AOE AOE branch, use it in BuildSlashTargets for the logical hitbox and debug outline, tune the production VFX base radius to the actual carrier extent, rerun data import/validator/build, capture a hitbox proof, then update docs and handoff.
Same method class: YES
If NO, why:
User approval required before proceeding: YES
Verification evidence: source diff, data table reload, production binding validator, focused T66Editor build, Unreal-owned hitbox/debug gameplay capture, proof log showing expected inside-band hits and outside/inner-hole misses, updated docs/handoff.
```

## Artifact Parity Gate

```text
ARTIFACT PARITY GATE
Reference artifact/category: Hero 1 AOE production Niagara crescent slash and combat DamageVolume debug sector.
Role: Primary
Required: YES
Planned artifact/path: /Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash plus combat-owned crescent-band sector metadata in weapon data and debug draw from BuildSlashTargets.
Status: SAME
Evidence: production binding row, commandlet mesh profile, capture frame with debug outline around the visual, HP-delta proof log.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: Future-agent VFX process tree and handoff.
Role: Secondary
Required: YES
Planned artifact/path: Gameplay/Combat docs updates plus a durable handoff prompt under Reports.
Status: SAME
Evidence: edited docs/index pointers and saved handoff file.
```

## Mechanism Manifest

```text
MECHANISM MANIFEST
Reference/source: CombatVFXAuthoringProcedure.md, Hero1AxeAOESlashMechanismPacket.md, live T66CombatComponent.cpp, T66Hero1AxeAOEVFXCommandlet.cpp.
Required mechanisms:
  1. Mechanism: Shared visual/logical geometry contract
     Required: YES
     Planned implementation: Add combat-owned AOE inner-radius ratio data to weapon rows, use it when calling BuildSlashTargets, and keep BaseVisualRadius calibrated to the Niagara carrier outer extent.
     Evidence needed: source/data diff, log line, and debug frame showing the outer and inner sector lines around the crescent.
  2. Mechanism: Damage authority remains combat-owned
     Required: YES
     Planned implementation: Keep damage selection in UT66CombatComponent and do not use Niagara collision or VFX actor overlap.
     Evidence needed: HP-before/after proof log with expected hits/misses.
  3. Mechanism: Item/stat scale still flows through EffectiveSlashRadius
     Required: YES
     Planned implementation: Keep existing EffectiveSlashRadius computation and derive inner radius from it, so later item scale changes scale both visual and hitbox proportionally.
     Evidence needed: code inspection and handoff prompt for next agent to run scale proof.
  4. Mechanism: Repeatable proof capture
     Required: YES
     Planned implementation: Update the existing hero1axeaoehitbox/hero1axeaoevfxbinding capture proof target positions for crescent-band geometry, then capture through Scripts/CaptureT66GameplayVideo.ps1 or RunHero1AxeAOEVFXBindingProof.ps1.
     Evidence needed: MP4/evidence bundle/log excerpt.
  5. Mechanism: Clean future-agent process routing
     Required: YES
     Planned implementation: Update the infrastructure inventory and mechanism packet with the geometry contract, current tree, cleanup status, and next-agent prompt.
     Evidence needed: docs diff and report path.
```

## Anti-Lookalike

- Cheapest wrong result: leave the filled 180-degree sector hitbox and only change the visual scale until the outer line looks less wrong.
- Discriminator: proof must show both outer and inner debug boundaries for the crescent-band shape, plus at least one target inside the visible band hit and at least one target inside the old filled-sector hollow/behind area not hit.

## Planned Edit Scope

Likely files:

- `Source/T66/Data/T66DataTypes.h`: add combat-owned AOE inner-radius ratio field to `FWeaponData`.
- `Content/Data/Weapons.csv`: set the Hero 1 black AOE row's inner-radius ratio to the accepted crescent-band ratio; default other weapons to `0`.
- `Scripts/SetupWeaponsDataTable.py`: include the new CSV column and generated defaults.
- `Source/T66/Gameplay/T66CombatComponent.cpp`: derive `EffectiveSlashInnerRadius` from weapon data and pass it to `BuildSlashTargets`; log geometry for proof.
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`: adjust automation target positions to test inside-band, inner-hole, outside-angle, and outside-radius behavior.
- `Content/Data/CombatVFXBindings.csv`: update `BaseVisualRadius` to the measured outer carrier extent if needed.
- `Scripts/ValidateCombatVFXProductionBindings.py`: add guards for the geometry contract fields/values and source fragments.
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`: record the hitbox-visual alignment close.
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`: update the infrastructure inventory and future-work state.
- `Gameplay/Combat/MASTER_COMBAT.md`: update combat geometry/debug/hitbox contract if runtime behavior changes.
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/`: durable handoff/cleanup report and new-agent prompt.

Out of scope:

- Idol overlay system implementation.
- Final visual polish of the accepted AOE slash.
- New VFX for DOT/Pierce/Bounce.
- Staged standalone packaging unless source/build verification shows it is required by the final scope.
- Mini/minigame systems.

## Risks And Rollback

- Changing from filled sector to crescent-band hitbox is a real gameplay semantics change. It matches the user's stated visual/hitbox alignment goal, but must be documented clearly.
- If the proof targets show the band is too strict for gameplay, rollback is straightforward: restore the Hero 1 AOE row's inner-radius ratio to `0` while keeping the docs/validation structure.
- DataTable reload may touch generated `.uasset` files; verify narrow paths only and avoid broad Git/LFS scans.

## Verification Plan

1. Reload `DT_Weapons` after CSV/schema change with the owning setup script.
2. Reload `DT_CombatVFXBindings` if `BaseVisualRadius` changes.
3. Run `Scripts/ValidateCombatVFXProductionBindings.py` through Unreal.
4. Build focused `T66Editor Win64 Development`.
5. Run an Unreal-owned hitbox/debug proof capture for Hero 1 AOE production binding with enemies staged in inside-band, inner-hole, outside-angle, and outside-radius cases.
6. Confirm the proof log has expected PASS lines and `CombatVFXProductionSpawned` geometry/scaling logs.
7. Inspect evidence contact sheet or selected frame for debug line around the current crescent visual.
8. Update docs/handoff with exact evidence paths and remaining next-agent tasks.

## Codex Position Before Claude Review

The right fix is not another VFX-only scale tweak. The current code already has a dormant inner-radius mechanism; using it turns the hitbox/debug outline into a crescent-band contract while preserving combat as the authority. The only material design choice is whether the user really wants gameplay semantics to become a crescent band instead of a filled 180-degree sector. Given the user's explicit "hit box matches the visual completely" wording, I interpret that as approved direction, but I want Claude to check whether this should be user-confirmed before implementation.

</review_packet>
