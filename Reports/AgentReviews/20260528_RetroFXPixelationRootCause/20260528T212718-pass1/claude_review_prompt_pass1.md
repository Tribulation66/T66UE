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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_RetroFXPixelationRootCause\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# RetroFX / Pixelation Unexpected Enablement Diagnostic Plan

## Working Goal

Diagnose why T66 retro/pixelation post-FX turns on unexpectedly, identify the side-effect trigger and source-of-truth problem, and produce a root-cause finding plus permanent fix proposal without making unrelated edits.

## User Constraints

- Correct current state is pixelation / retro post-FX off.
- Diagnostic-first: identify the mechanism and trigger before changing anything.
- Do not simply toggle it off.
- Do not implement a fix unless the root cause is a clear, single-point, low-risk fix and this packet explicitly proposes it.
- Deliverable is a consolidated diagnostic with source-of-truth map, writers, trigger, committed-vs-dirty determination, root cause, and permanent patch proposal.
- Out of scope: B.13 HISM rendering, retro art direction, enabling/designing RetroFX mode, unrelated edits.

## Applicable Repo Instructions

- Root `AGENTS.md`: establish working goal, inspect live repo, use Claude review by default, report verification or skipped verification.
- `Gameplay/GAMEPLAY_AGENTS.md`: gameplay/runtime work must use live code and current pending issues; no Mini/minigame scope unless explicitly named.
- `Reports/AGENTS.md`: reports and review artifacts belong under `Reports/AgentReviews/<TaskSlug>/`.
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`: relevant because this blocks a performance pass, but this pass is diagnostic/report-only and does not run captures.

PPF is not required for this pass because this is not a visual/VFX authoring task and does not create or alter visual artifacts. If a later patch changes the visual runtime path, that implementation pass should include the appropriate visual verification evidence.

## Preliminary Evidence From Live Repo

The likely failure shape is a committed source-of-truth/default bug, not a dirty source diff:

- `Source/T66/Core/T66RetroFXSettings.h` defaults gameplay RetroFX master on: `bEnableRetroFXMaster = true`, real low resolution on: `bUseRealLowResolution = true`, target height percent `40.0f`.
- `Source/T66/Core/T66PlayerSettingsSaveGame.h` duplicates the master flag as `bRetroFXMasterEnabled = true`.
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp` creates/migrates/resets settings through `FT66RetroFXSettings()` and copies `RetroFXSettings.bEnableRetroFXMaster` into the duplicate save flag.
- `Source/T66/Core/T66RetroFXSubsystem.cpp` applies saved settings through `ApplyCurrentSettings() -> ApplySettings() -> ApplyResolutionRuntime()`, and when `bUseRealLowResolution` is true it writes runtime resolution CVars including `r.ScreenPercentage`.
- `Source/T66/Gameplay/T66GameMode.cpp::HandleSettingsChanged()` applies current RetroFX settings on normal gameplay worlds. It only forces RetroFX off for test-room runs.
- `Source/T66/UI/Screens/Settings/T66SettingsScreen_RetroFX.cpp` and `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_RetroFX.cpp` both reset pending RetroFX settings to `FT66RetroFXSettings()`, which currently means "on" by default.
- Narrow Git status/diff for the RetroFX source/config/material paths is clean. The dirty nearby `T66GameMode.cpp` diff is unrelated companion/beacon work and does not touch the RetroFX block. This points to a committed-code issue rather than uncommitted-local source state.
- `Saved/SaveGames/T66_PlayerSettings.sav` and staged equivalent contain serialized RetroFX fields, so persisted local state can preserve/reapply a prior on state. That is a trigger vector, but the source defaults also make new/migrated/reset saves on-by-default.

## Diagnostic Plan

1. Complete source-of-truth map:
   - `FT66RetroFXSettings`
   - `UT66PlayerSettingsSaveGame::RetroFXSettings`
   - duplicate `UT66PlayerSettingsSaveGame::bRetroFXMasterEnabled`
   - `UT66PlayerSettingsSubsystem` getter/setter/migrations/reset/safe-mode
   - `UT66RetroFXSubsystem` effective settings, blendable weights, resolution runtime CVars, pixelation subsystem calls
   - `UT66PixelationSubsystem` independent post-process pixelation blendable
   - config CVars in `Config/DefaultEngine.ini` / `DefaultDeviceProfiles.ini`
   - UI writers and resets
   - persisted SaveGame state

2. Confirm intended-off state:
   - gameplay master false
   - real low resolution false
   - screen percentage restored/native
   - PS1/N64/chromatic/post-process blend weights zero
   - world/character pixelation levels zero
   - UI-only CRT handled separately and not conflated with gameplay pixelation

3. Enumerate all writers and triggers:
   - settings creation/migration/reset/safe-mode
   - Settings RetroFX tab apply/reset/live preview
   - Hero Selection inline RetroFX panel apply/reset/close
   - gameplay settings changed path
   - async asset preload callback
   - direct pixelation console helper
   - config/default CVars
   - persisted SaveGame restore

4. Determine committed-code vs dirty-state:
   - use narrow `git status`/`git diff` for source/config/targeted content paths
   - inspect saved/staged SaveGame presence and serialized RetroFX field names
   - avoid broad LFS/content scans

5. Root-cause decision:
   - If the evidence holds, report the root cause as multiple competing/on-by-default sources where `FT66RetroFXSettings()` represents enabled RetroFX, and common unrelated triggers such as save migration/reset/settings application reapply that default.
   - Distinguish apparent "pixelation" from actual low-resolution runtime scaling: default scalar pixelation percentages are zero, but `bUseRealLowResolution=true` can lower `r.ScreenPercentage` to a visibly pixelated result.

6. Permanent patch proposal:
   - Establish one explicit OFF-by-default source of truth for gameplay RetroFX.
   - Change defaults so `FT66RetroFXSettings()` is disabled for gameplay: `bEnableRetroFXMaster=false`, `bUseRealLowResolution=false`, target resolution neutral/native.
   - Remove or neutralize the duplicate saved master source by making `RetroFXSettings.bEnableRetroFXMaster` authoritative and treating `bRetroFXMasterEnabled` as legacy migration-only, or keep it only if writes are centralized through a helper that cannot diverge.
   - Add a migration for existing saved settings so old default-on saves created by this bug are converted off unless the user has explicit non-zero RetroFX effect settings that indicate intentional opt-in.
   - Make reset/safe-mode paths reset to the new off defaults.
   - Make `ApplyCurrentSettings()` unable to re-enable gameplay RetroFX from an implicit default; only explicit user opt-in should enable it.

## Proposed Implementation Decision For This Pass

Do not implement the fix in this pass unless review finds this is safe enough as a single-point default correction. My current recommendation is to stop at the diagnostic and fix proposal because the proper fix likely touches defaults, save migration semantics, UI reset semantics, and possibly the duplicate master field. That is small but cross-cutting enough to deserve a follow-up reviewed patch.

## Verification / Evidence To Produce

- Consolidated Markdown report under `Reports/AgentReviews/20260528_RetroFXPixelationRootCause/`.
- File:line references for every state holder and writer.
- Narrow dirty-state classification proving source/config/content paths for RetroFX are clean or naming any dirty exceptions.
- Clear root-cause statement and permanent patch proposal.
- No code/content/config edits in this diagnostic pass unless the reviewed packet explicitly greenlights a trivial fix.

## Known Risks / Review Questions

- Is the default-on `bUseRealLowResolution` enough to explain the user-visible "pixelation" even when world pixelation post-process percentages are zero?
- Is there any PPV/content default that can independently force the pixelation material on and should be included before finalizing?
- Is the duplicate `bRetroFXMasterEnabled` field the best permanent-fix target, or should it remain for compatibility while centralizing writes?
- Is stopping at proposal, rather than applying the default fix now, the right risk boundary?

</review_packet>
