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
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_RetroFXOffByDefaultFix\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# RetroFX Off-By-Default Permanent Fix Plan

## Working Goal

Make T66 retro/CRT post-FX off by default, remove the duplicate master source of truth, migrate all saves off, seal all re-enable paths, verify `r.ScreenPercentage` stays `100`, and record the corrected full-resolution lightweight FPS sanity baseline.

## User Constraints

- Gameplay RetroFX defaults off.
- Real low-resolution defaults off.
- Frontend `UIFullScreenCRTEnabled` defaults off.
- Save migration forces all existing saves off. No opt-in preservation.
- The fix must address the default/recreation trigger, not simply toggle the current runtime value.
- When RetroFX/real-low-resolution is off, full-resolution rendering is restored via `r.ScreenPercentage=100`.
- Exercise each recurrence path individually and emit terminal summaries with `EnabledAfter=0` and `ScreenPercentage=100`.
- Run a saturated full-resolution lightweight FPS sanity capture after the patch.
- Out of scope: B.13, art-direction changes, enabling/designing RetroFX, unrelated visual changes.

## Applicable Instructions

- Root `AGENTS.md`: working goal, live-repo inspection, implementation plan, Claude review before implementation, no redundant Pablo approval after valid Claude approval, verification evidence required.
- `Gameplay/GAMEPLAY_AGENTS.md`: runtime-facing gameplay changes need build verification and staged standalone validation when playable standalone is affected.
- `UI/UI_AGENTS.md`: UI changes require verification as task needs; this pass touches UI state defaults, not UI layout/fidelity.
- `Reports/AGENTS.md`: place review packets and reviewer outputs under `Reports/AgentReviews`.
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`: performance sanity capture must preserve schema/capture hygiene and standalone behavior.

PPF: Not applicable. This pass is a runtime/settings/source-of-truth fix for an unintended visual mode, not an authored visual artifact, VFX, UI fidelity, import, animation, or generated-media task.

## Evidence From Live Repo

- `Source/T66/Core/T66RetroFXSettings.h`: `bEnableRetroFXMaster=true`, `bUseRealLowResolution=true`, `TargetResolutionHeightPercent=40.0f`, and `UIFullScreenCRTEnabled=true`.
- `Source/T66/Core/T66PlayerSettingsSaveGame.h`: schema `23` and duplicate saved master `bRetroFXMasterEnabled=true` coexist with full `RetroFXSettings`.
- `Source/T66/Core/T66PlayerSettingsSubsystem.cpp`: migrations 21-23 reset or mirror defaults and `GetRetroFXSettings()` overwrites the struct master from the duplicate mirror; resets and safe mode also recreate default settings.
- `Source/T66/Core/T66RetroFXSubsystem.cpp`: `ApplyResolutionRuntime()` lowers `r.ScreenPercentage` when real low-res is enabled; the off path only restores a captured original value and only when the runtime override was marked active.
- `Config/DefaultDeviceProfiles.ini`: the D3D11 Windows profile sets `r.ScreenPercentage=85`, which conflicts with the locked "off means 100" decision.
- `Source/T66/Gameplay/T66GameMode.cpp`: `HandleSettingsChanged()` applies current player RetroFX on gameplay-world settings changes/startup.
- UI reset/apply paths use `FT66RetroFXSettings()` defaults: `T66SettingsScreen_RetroFX.cpp`, `T66SettingsScreen_Build.cpp`, and `T66HeroSelectionScreen_RetroFX.cpp`.

## Planned Edits

### Source Of Truth And Defaults

- Edit `Source/T66/Core/T66RetroFXSettings.h`:
  - `bEnableRetroFXMaster=false`
  - `bUseRealLowResolution=false`
  - `TargetResolutionHeightPercent=100.0f`
  - `UIFullScreenCRTEnabled=false`
- Edit `Source/T66/Core/T66PlayerSettingsSaveGame.h`:
  - bump schema to `24`
  - remove the duplicate `bRetroFXMasterEnabled` UPROPERTY so `RetroFXSettings.bEnableRetroFXMaster` is the only persisted master flag.

### Save Migration And Recurrence Paths

- Edit `Source/T66/Core/T66PlayerSettingsSubsystem.h/.cpp`:
  - add a small internal helper that forces RetroFX settings to the new disabled defaults.
  - migration `<24` forces all existing saves off by replacing `RetroFXSettings` with disabled defaults and saving.
  - remove all reads/writes of `bRetroFXMasterEnabled`.
  - `SetRetroFXSettings()`, `GetRetroFXSettings()`, `ResetRetroFXSettingsToDefaults()`, and `ApplySafeModeSettings()` all use only the struct master and inherit disabled defaults.
  - add a non-shipping/automation-safe command-line recurrence-seal verification hook, invoked from gameplay startup, that exercises fresh launch, settings reset, safe mode, UI reset-equivalent default application, legacy previously-on save migration, and gameplay/map apply paths. Each path emits one `RetroFXSealSummary Path=... EnabledAfter=0 RealLowResAfter=0 UIFullScreenCRTAfter=0 ScreenPercentage=100.00` line.
- Edit `Source/T66/Gameplay/T66GameMode.cpp` minimally:
  - after the normal RetroFX apply path, run the recurrence-seal verification once when `-T66RetroFXSealVerify` is present.

### Full-Resolution Off Path

- Edit `Source/T66/Core/T66RetroFXSubsystem.cpp`:
  - make the off path call the restore function every time real low-res is disabled, so legacy lowered values are cleared even if the current subsystem instance did not mark the override active.
  - make restoration set `r.ScreenPercentage=100.0f` by policy. Restore `r.ScreenPercentage.MinResolutionFraction` to the captured value when available, but do not restore a lowered screen percentage.
- Edit `Config/DefaultDeviceProfiles.ini`:
  - change the D3D11 profile `r.ScreenPercentage` fallback from `85` to `100` to match the locked full-res-off policy.

### Documentation

- Append a resolution section to `Reports/AgentReviews/20260528_RetroFXPixelationRootCause/root_cause_report.md` or create a linked completion packet under `Reports/AgentReviews/20260529_RetroFXOffByDefaultFix/`.
- Update `Source/T66/Core/pending_issues_Core.md` only if a tracked issue entry is warranted; otherwise document that no pending Core issue remains because the root cause is fixed in this pass.
- Record the full-res lightweight FPS sanity result and note that B.13 should use this full-resolution baseline.

## Out Of Scope

- B.13 HISM rendering.
- Retuning or designing RetroFX/CRT mode.
- Enabling RetroFX as a shipped feature.
- UI layout/fidelity redesign.
- Changing unrelated dirty worktree files.

## Risks And Mitigations

- Removing the duplicate SaveGame UPROPERTY can leave old serialized data in existing saves; Unreal should ignore unknown serialized properties, and schema 24 forces the remaining authoritative struct off. Verification includes a previously-on migration path.
- Forcing all existing saves off is intentionally destructive to RetroFX state, but explicitly locked by Pablo.
- Setting `r.ScreenPercentage=100` may supersede old device-profile performance fallbacks; this is required by the locked full-resolution-off decision. The full-res FPS sanity read records the new honest baseline.
- The worktree is dirty with many unrelated runtime/content changes. Edits will be restricted to the listed files and verified with `git diff` against those paths.

## Verification Plan

1. Run a focused C++ build for the `T66` target after edits.
2. Refresh/stage the standalone build as required for playable runtime behavior.
3. Launch staged standalone with `-T66RetroFXSealVerify`; capture log and verify summaries for:
   - `FreshLaunch`
   - `SettingsReset`
   - `SafeMode`
   - `UIReset`
   - `LegacySaveLoadMigration`
   - `GameplaySettingsApply`
   - `MapWorldLoad`
4. Confirm every summary reports `EnabledAfter=0`, `RealLowResAfter=0`, `UIFullScreenCRTAfter=0`, and `ScreenPercentage=100.00`.
5. Run one saturated lightweight `enemywaveperf` capture with `HeroHPOverride=20000`, no rich row, full-resolution off policy active. Record FPS, `PerformanceSystemOverheadMaxUs`, hero survival, and staged binary SHA.
6. Update documentation with the code fix, recurrence-seal proof, and corrected full-res FPS sanity reference.


</review_packet>
