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
- Packet path: C:\UE\T66\Reports\AgentReviews\CombatVFXAoeWaterAlignmentRuntime\delta_plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
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

</review_packet>
