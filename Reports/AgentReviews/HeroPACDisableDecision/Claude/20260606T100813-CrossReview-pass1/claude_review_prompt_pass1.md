You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\HeroPACDisableDecision\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\HeroPACDisableDecision\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\HeroPACDisableDecision\Claude\20260606T100100-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Okay, so you got at my question here, which is, will we eventually have heroes using pack? It's a yes or no. If the answer is no, then yeah, disable it in the test room. Disable it, not even just in the test room, disable it for heroes. But if it's yes, we need to figure out how to make it work with pack. But I agree, if pack is dragging you to a certain location, then we shouldn't use it, because the idea is to let the chaos do its thing. But that's basically for everything. I mean, even the bosses, I don't want them to come back to a specific place. I want them to, you know, just be chaotic and go where they need to be.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: make the yes/no architecture call on whether heroes should use Physical Animation Component for ragdoll, and if the answer is no, disable hero PAC rather than only disabling it in the TestRoom profile.
Stop condition: clear decision, scoped implementation if no, compile verification, and honest caveats about PAC infrastructure that remains.

Relevant repo/process rules:
- C:\UE\T66\AGENTS.md requires Codex operator / Claude validator loop for T66 prompts.
- Current operator state is Codex operator, Claude validator.
- Runtime gameplay changes need focused compile verification.
- Existing code has `FT66KnockbackProfile` defaults with PAC enabled, but the active detached path force-disables PAC at runtime.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

**The architecture call is NO — heroes should not use the Physical Animation Component for ragdoll.**

The user's own criterion settles it: PAC exists to drive simulated physics bodies *back toward an animated/target pose* (it's a corrective, "pull-toward-intended-pose" mechanism). That is precisely the "drag you to a certain location / come back to a specific place" behavior the user wants to eliminate. The user explicitly extended the chaos principle to "everything... even the bosses." So the answer is a clean NO, and the work is to disable hero PAC at the source, not just in the TestRoom profile.

Reality on the ground actually makes this almost a no-op functionally: the default profile ships with `bDetachMeshDuringRagdoll = true` (T66KnockbackComponent.h:87), and the runtime path at T66KnockbackComponent.cpp:147-159 *force-disables* `bEnablePhysicalAnimation` and sets `PhysicalAnimationDriveMode = Disabled` whenever the mesh is detached. So on the active hero ragdoll path, PAC already never runs — the profile defaults (`bEnablePhysicalAnimation = true`, `PhysicalAnimationDriveMode = CoreChain`) are misleading dead intent.

Recommended scoped implementation for Codex:
1. Flip the `FT66KnockbackProfile` defaults to make the decision explicit and self-documenting: `bEnablePhysicalAnimation = false` (line 144) and `PhysicalAnimationDriveMode = ET66KnockbackPhysicalAnimationDriveMode::Disabled` (line 147). This makes "heroes don't use PAC" the authored default everywhere, not a runtime override that fights the profile.
2. Leave the runtime detached-path force-disable in place (defense-in-depth; harmless once defaults agree).
3. Focused compile verification of the T66 module after the edit.

Do **not** rip out the PAC infrastructure (component pointer, `GetOrCreatePhysicalAnimationComponent`, `ApplyPhysicalAnimationDrive`, the enum, the profile fields). Disabling via defaults is the minimal, reversible change and keeps the door open if a non-detached, PAC-driven use ever appears for another actor class.

## Evidence Checked
- `T66KnockbackComponent.h:47-166` — `FT66KnockbackProfile`, PAC fields default enabled (`bEnablePhysicalAnimation=true`, `DriveMode=CoreChain`), `bDetachMeshDuringRagdoll=true`.
- `T66KnockbackComponent.cpp:147-159` — runtime force-disables PAC on the detached path.
- `T66KnockbackComponent.cpp:296-300` — detach occurs whenever `bDetachMeshDuringRagdoll` (the default).
- `cpp:1199-1272` / `1141-1199` — PAC creation/drive gated on `bEnablePhysicalAnimation` and non-`Disabled` drive mode.
- Original prompt + task contract.

## Questions Or Blockers
None that require the user. The user already supplied the deciding principle ("let the chaos do its thing," no return-to-place, applies to everything including bosses), so this is an internal architecture call Codex can execute.

## Caveats
- PAC infrastructure (component, drive functions, enum, profile fields) remains in the codebase after this change — be honest about that; this is a disable, not a removal.
- The change is partly cosmetic on the active path (detach already kills PAC at runtime), but it removes the contradiction between authored defaults and actual behavior, which matters if anyone later sets `bDetachMeshDuringRagdoll=false`.
- This decision is scoped to heroes' ragdoll/knockback. The user's "even bosses" comment about not returning to a fixed place is a related but separate behavior (anchoring/follow targeting), not strictly PAC — don't silently fold boss-positioning changes into this PAC task without confirming scope.
- I did not run a compile; verification is Codex's stop-condition step.

</validator_independent_answer>

<codex_draft>
Draft completion:

Decision: NO. Heroes should not use PAC for ragdoll/knockback. The hero direction is pure Chaos ragdoll: launch the body, let physics carry it, keep the actor/camera following the simulated body, and recover afterward. This matches the user's stated rule that the character should not be dragged back toward a target pose or intended location.

Implemented:
- `Source/T66/Gameplay/T66KnockbackComponent.h`: default `FT66KnockbackProfile` now has `bEnablePhysicalAnimation=false` and `PhysicalAnimationDriveMode=Disabled`.
- `Source/T66/Gameplay/T66KnockbackComponent.cpp`: runtime guard now disables PAC for hero ragdoll profiles even if a profile tries to enable it. The detached-ragdoll guard remains for non-hero defense-in-depth.
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`: TestRoom wipeout-arm PAC CVars now default to off/disabled.
- `Source/T66/Gameplay/pending_issues_Gameplay.md`: updated stale note that previously described PAC as the active hero direction.

PAC infrastructure intentionally remains in code. This is a disable/policy change, not a removal. It keeps old code reachable only if a future explicit non-hero/non-detached experiment is approved.

Verification:
- Focused `T66Editor Win64 Development` build passed.
- `Scripts/StageStandaloneBuild.ps1` passed, producing `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`.
- Shortcut target verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Existing unrelated compile warning remains: `T66Hero1AxeAOEVFXLabActor.cpp(704)` uses deprecated `FNiagaraEmitterInstance::IsReadyToRun`.

Caveat:
- I did not remove all PAC code from the project. Removing the subsystem entirely would be broader cleanup and is not needed to answer the hero decision.

</codex_draft>
