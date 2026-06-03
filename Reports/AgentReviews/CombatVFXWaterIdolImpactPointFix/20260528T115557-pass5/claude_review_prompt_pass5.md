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

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactPointFix\supplemental_placeholder_transform_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Water Idol Placeholder Transform - Supplemental Review Packet

## Working Goal

Fix the Water idol blue-sphere placeholder so it spawns at the actual Hero 1 AOE weapon impact point instead of visually sitting on top of the hero, then verify with reviewed code and Unreal-owned capture evidence.

## Current Verified State

Two reviewed patches are in place:

1. `Source/T66/Gameplay/T66CombatComponent.cpp` publishes the Hero 1 hollow crescent AOE impact context at the band midpoint:
   - `WeaponPrimary ImpactPoint=V(X=696.89, Z=64.00)`
   - `IdolPrimary ImpactPoint=V(X=696.89, Z=64.00)`
2. `Source/T66/Gameplay/T66CombatVFX.cpp` decouples the temporary blue marker scale from Water's damage radius:
   - `CombatVFXIdolImpactPlaceholderSpawned ... Radius=300.00 ... VisualScale=0.850`

Problem still visible in the final capture:

- The log point is correct, but the blue mesh still reads near/on the hero in the selected frame.
- `SpawnWaterIdolImpactPlaceholderVFX` spawns a bare `AActor::StaticClass()` at the desired location, then creates a `UStaticMeshComponent`, sets it as the root, registers it, and scales it.
- The new root component is never explicitly anchored back to the calculated spawn location after root assignment. For a runtime-created root component on a bare actor, relying on the actor spawn transform to propagate through later root assignment is too implicit for this proof.

## PPF CHECK

Objective: Ensure the temporary Water placeholder mesh's actual actor/root transform is explicitly placed at `IdolImpactContext.ImpactPoint + ZOffset`.

Proven process: Combat VFX runtime binding/impact-context process in `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` and `Gameplay/Combat/MASTER_COMBAT.md`.

My planned implementation: Keep the corrected impact context, Water damage query, Water source ID, Water radius, and compact marker scale unchanged. In `SpawnWaterIdolImpactPlaceholderVFX`, compute `SpawnLocation` once, use it for actor spawn, and call `PlaceholderActor->SetActorLocation(SpawnLocation)` after the mesh component is set as root and registered. Add `VisualLocation` to the verbose placeholder log so the capture proves the mesh actor location, not only the context point.

Same method class: YES

If NO, why: N/A

User approval required before proceeding: NO, because this is the missing placement guarantee for the exact placeholder artifact already approved.

Verification evidence: focused compile plus Unreal-owned `hero1axeaoewateridolimpact` capture/log proof showing context impact `V(X=696.89, Z=64.00)`, placeholder `VisualLocation=V(X=696.89, Z=140.00)`, `VisualScale=0.850`, and contact-sheet evidence that the blue marker is no longer on top of the hero.

## ARTIFACT PARITY GATE

Reference artifact/category: Temporary Water idol blue sphere placeholder.

Role: Primary for this temporary proof only.

Required: YES

Planned artifact/path: Existing blue sphere placeholder in `SpawnWaterIdolImpactPlaceholderVFX`, still driven from `IdolImpactContext.ImpactPoint`.

Status: SAME

Evidence: Unreal-owned capture plus `CombatVFXIdolImpactPlaceholderSpawned ... ImpactPoint=... VisualLocation=... Placeholder=BlueSphere VisualScale=0.850`.

## MECHANISM MANIFEST

Reference/source: `CombatVFXIdolOverlayArchitecture.md`

Required mechanisms:

1. Mechanism: Weapon impact context drives idol impact context.
   Required: YES
   Planned implementation: No change from first patch; Water still inherits `PrimaryWeaponImpactContext.ImpactPoint`.
   Evidence needed: `WeaponPrimary` and `IdolPrimary` with matching `ImpactPoint=V(X=696.89, Z=64.00)`.

2. Mechanism: Idol owns independent damage/query source.
   Required: YES
   Planned implementation: No change to query radius or damage application.
   Evidence needed: `DamageBySource SourceID=Idol_Water`.

3. Mechanism: Placeholder mesh transform follows idol impact point.
   Required: YES for this temporary proof.
   Planned implementation: Explicitly reapply the computed `SpawnLocation` after root component assignment and registration, and log `VisualLocation`.
   Evidence needed: log `VisualLocation=V(X=696.89, Z=140.00)` and contact-sheet/video marker location.

## Planned Code Change

Edit `Source/T66/Gameplay/T66CombatVFX.cpp` only:

1. Add `const FVector SpawnLocation = IdolImpactContext.ImpactPoint + FVector(0.f, 0.f, 76.f);`.
2. Pass `SpawnLocation` to `World->SpawnActor`.
3. After root assignment/register, call `PlaceholderActor->SetActorLocation(SpawnLocation);`.
4. Extend `CombatVFXIdolImpactPlaceholderSpawned` to log `VisualLocation=%s`.

## Risks

- If the current visual was caused purely by camera projection rather than root-transform ambiguity, this should still be safe because it makes the runtime placement explicit and inspectable.
- No gameplay behavior changes: damage radius/query/source stay untouched.

## Verification Plan

1. `git diff --check -- Source/T66/Gameplay/T66CombatComponent.cpp Source/T66/Gameplay/T66CombatVFX.cpp Gameplay/Combat/pending_issues_Combat.md`
2. Focused compile:
   `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
3. Unreal-owned capture:
   `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact ... -EvidenceBundle`
4. Check log lines:
   - `CombatImpactContext Phase=WeaponPrimary ... ImpactPoint=V(X=696.89, Z=64.00)`
   - `CombatImpactContext Phase=IdolPrimary ... ImpactPoint=V(X=696.89, Z=64.00)`
   - `CombatVFXIdolImpactPlaceholderSpawned ... ImpactPoint=V(X=696.89, Z=64.00) ... VisualLocation=V(X=696.89, Z=140.00) ... VisualScale=0.850`
   - `DamageBySource SourceID=Idol_Water`
   - `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`
5. Inspect contact sheet/video for the blue marker positioned forward at the corrected impact point rather than visually sitting on the hero.

## Reviewer Request

Please review whether this transform anchoring patch is the right missing fix after the context and marker-scale changes. Focus on whether the root-component placement hypothesis is plausible, whether the explicit actor-location assignment is safe, and whether logging `VisualLocation` is the right proof hook.

Return first non-empty line exactly `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.

</review_packet>
