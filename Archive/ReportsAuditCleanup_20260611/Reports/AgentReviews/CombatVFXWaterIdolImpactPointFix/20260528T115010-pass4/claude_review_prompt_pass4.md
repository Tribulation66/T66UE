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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXWaterIdolImpactPointFix\supplemental_marker_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Water Idol Impact Marker Scale - Supplemental Review Packet

## Working Goal

Fix the Water idol blue-sphere placeholder so it spawns at the actual Hero 1 AOE weapon impact point instead of visually sitting on top of the hero, then verify with reviewed code and Unreal-owned capture evidence.

## Current Verified State

The first reviewed patch changed `Source/T66/Gameplay/T66CombatComponent.cpp` so frontal-sector hollow crescent AOE weapon contexts publish an impact point at the band midpoint instead of the raw damage-query center.

Runtime capture proof after that patch:

- Old captured point before this fix: `ImpactPoint=V(X=360.00, Z=64.00)`.
- New captured `WeaponPrimary`: `ImpactPoint=V(X=696.89, Z=64.00)`.
- New captured `IdolPrimary`: `ImpactPoint=V(X=696.89, Z=64.00)`.
- `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base` still spawns the weapon slash at `Location=V(X=360.00, Z=134.00)`, preserving the Niagara origin.
- `CombatVFXIdolImpactPlaceholderSpawned` still uses the corrected impact point, but logs `Radius=300.00` and `VisualScale=1.500`.

Problem after the first fix:

- The code source point is now correct, but the temporary placeholder sphere is scaled from Water's damage radius via `FMath::Clamp(Radius / 200.f, 0.80f, 1.75f)`.
- At `Radius=300`, this creates a large sphere (`VisualScale=1.5`) that can still visually overlap the hero from the proof camera even though its center is now at the corrected weapon impact point.
- The user complaint is visual and code-level: the blue sphere should visibly be at the weapon impact point, not on top of the hero. A giant opaque damage-volume-sized sphere is the cheapest wrong result that can hide a corrected center point.

## PPF CHECK

Objective: Make the temporary Water blue sphere read as an impact-point marker while keeping the corrected weapon-driven idol impact context and Water damage source intact.

Proven process: Combat VFX runtime binding/impact-context process in `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` and `Gameplay/Combat/MASTER_COMBAT.md`.

My planned implementation: Keep the Water idol damage radius, damage query, source ID, and impact context unchanged. In the temporary fallback placeholder only, decouple the mesh visual scale from the gameplay damage radius so the blue sphere is a compact marker for the impact point. Continue logging the real `Radius=300.00` for damage proof.

Same method class: YES

If NO, why: N/A

User approval required before proceeding: NO, because this preserves the existing temporary placeholder artifact and makes it visually prove the same code-level impact source the user asked to fix.

Verification evidence: focused compile plus Unreal-owned `hero1axeaoewateridolimpact` capture/log proof showing `WeaponPrimary` and `IdolPrimary` at `X=696.89`, `DamageBySource SourceID=Idol_Water`, and placeholder `VisualScale` reduced from `1.500` to about `0.850`.

## ARTIFACT PARITY GATE

Reference artifact/category: Temporary Water idol blue sphere placeholder.

Role: Primary for this temporary proof only.

Required: YES

Planned artifact/path: Existing blue sphere placeholder in `SpawnWaterIdolImpactPlaceholderVFX`, still spawned from `IdolImpactContext.ImpactPoint`.

Status: SAME

Evidence: Unreal-owned capture plus `CombatVFXIdolImpactPlaceholderSpawned ... ImpactPoint=... Radius=300.00 Placeholder=BlueSphere VisualScale=0.850`.

## MECHANISM MANIFEST

Reference/source: `CombatVFXIdolOverlayArchitecture.md`

Required mechanisms:

1. Mechanism: Weapon impact context drives idol impact context.
   Required: YES
   Planned implementation: No change from first patch; Water still inherits `PrimaryWeaponImpactContext.ImpactPoint`.
   Evidence needed: `WeaponPrimary SourceID=Hero_1_black_aoe` and `IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe` with matching `ImpactPoint`.

2. Mechanism: Idol owns independent damage/query source.
   Required: YES
   Planned implementation: No change to query radius or damage application. Keep `IdolImpactContext.Radius = IdolData.AoeRadius` and `DamageBySource SourceID=Idol_Water`.
   Evidence needed: `DamageBySource SourceID=Idol_Water` and Water proof target logs.

3. Mechanism: Placeholder visually marks the impact point.
   Required: YES for this temporary proof.
   Planned implementation: Change only the fallback placeholder mesh scale to a fixed `0.85f`. A trial capture with `0.5f` proved the marker could become too small to see clearly behind the target stack, so `0.85f` is the compact-marker candidate for the final capture.
   Evidence needed: capture frame/contact sheet where the blue marker is readable at the corrected impact point without covering the hero silhouette and log `VisualScale=0.850`.

## Planned Code And Doc Change

Edit `Source/T66/Gameplay/T66CombatVFX.cpp`:

1. In `SpawnWaterIdolImpactPlaceholderVFX`, change the temporary placeholder visual scale from `FMath::Clamp(Radius / 200.f, 0.80f, 1.75f)` to a fixed `0.85f`.
2. Add one concise comment making clear the mesh is a temporary impact marker and `Radius` remains the damage/query radius.
3. Leave spawn location, Z offset, life span, color, `Radius` log, and damage path unchanged.

Edit `Gameplay/Combat/pending_issues_Combat.md`:

1. Update the resolved Water item to note that the placeholder visual scale is decoupled from damage radius so the blue sphere is a point marker, not a final Water area effect.

## Risks

- The placeholder no longer visually communicates the full Water AOE radius. That is acceptable for this structure proof because damage radius is logged and damage is verified by `DamageBySource`; final Water Niagara will own the real area read.
- If the user wants the temporary placeholder to show both impact point and AOE size, that should be a separate proof visualization such as a translucent ring, not the opaque marker sphere in this fix.

## Verification Plan

1. `git diff --check -- Source/T66/Gameplay/T66CombatComponent.cpp Source/T66/Gameplay/T66CombatVFX.cpp Gameplay/Combat/pending_issues_Combat.md`
2. Focused compile:
   `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex`
3. Unreal-owned capture:
   `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact ... -EvidenceBundle`
4. Check log lines:
   - `CombatImpactContext Phase=WeaponPrimary ... ImpactPoint=V(X=696.89, Z=64.00)`
   - `CombatImpactContext Phase=IdolPrimary ... ImpactPoint=V(X=696.89, Z=64.00)`
   - `CombatVFXIdolImpactPlaceholderSpawned ... Radius=300.00 ... VisualScale=0.850`
   - `DamageBySource SourceID=Idol_Water`
   - `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`
5. Inspect contact sheet/video for the blue sphere as a compact marker at the corrected impact point, not an opaque ball covering the hero.

## Reviewer Request

Please review whether this supplemental patch is the right next step after the impact-context center fix. Focus on whether decoupling temporary placeholder visual scale from Water damage radius preserves the idol damage-source architecture while addressing the user's visual complaint.

Return first non-empty line exactly `Verdict: APPROVE`, `Verdict: REVISE`, `Verdict: NEEDS_HUMAN_DECISION`, or `Verdict: BLOCK`.

</review_packet>
