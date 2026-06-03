# Operator Completion: B.13 No-Land Closeout

- Operator: Claude (claude-opus-4-8), FullOperator
- Validator/Finisher: Codex
- Codex approval artifact: `Reports/AgentReviews/20260529_B13_NoLand_Closeout/codex_operator_approval.md`
- Date: 2026-05-29
- Scope: documentation and verification only. No live runtime renderer changes,
  no B.13R work, no asset deletion, no worktree deletion, no Git commit/push/reset/clean.

## Outcome

B.13 is formally closed as a no-land. The live repo now contains a single
authoritative B.13 audit, standing docs point to it and mark B.13 closed, live
source/content was checked for B.13 renderer leftovers (none found), current
build/run evidence for the per-mob renderer is recorded, and the isolated
worktree disposition is reported without deleting the worktree.

## Files Changed

| File | Change |
| --- | --- |
| `PerformanceSystem/B13_MobInstancedRendering_Audit.md` | Created. Authoritative consolidation: goal, staged de-risk approach, before baseline, correctness proofs, full candidate table (14 candidates), root cause, UE 5.7 engine-source audit, final no-land decision + GPU-crowd escape hatch, live-repo state. |
| `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md` | Added CLOSED - NO-LAND banner to the B.13 section pointing at the audit; updated the B.13 pass-table row to closed/no-land; updated the "remaining large performance pass" line to closed. |
| `PerformanceSystem/pending_issues_PerformanceSystem.md` | Marked the "Dynamic Mob HISM Rendering..." item Status: CLOSED - NO-LAND (no longer pending), pointed it at the audit, reframed the out-of-scope/future-work text around the recorded decision. |
| `Reports/AgentReviews/20260529_B13_NoLand_Closeout/operator_completion.md` | Created (this file). |
| `Reports/AgentReviews/20260529_B13_NoLand_Closeout/preserved_worktree_evidence/` | Created. Copied 16 `*_capture_results.json` + 19 `*Proof*.log` (1.9 MB) from the worktree so the load-bearing raw evidence survives eventual worktree deletion. |

Historical evidence intentionally NOT rewritten:
`Reports/AgentReviews/20260529_B13_MobHISM_VAT/combined_packet.md` and its
sub-run artifacts are left as-is (and are outside the approved change list). The
audit references them.

## Verification

### Live source clean of B.13 renderer work

- Instanced material search (Glob `Content/**/*Instanced*.uasset`): **no files**.
  The B.13-created `M_EasyMobVAT_Unlit_UV2_Instanced` does not exist in live
  Content. Glob `Content/Materials/M_EasyMobVAT*.uasset` returns only the base
  `M_EasyMobVAT_Unlit_UV2.uasset`. Result: **no Pass D orphan**.
- Mob-path instanced-renderer search (Grep `HISM|HierarchicalInstanced|InstancedStaticMesh|_Instanced|CellBucket|SpatialCell|CustomDataFrame` in `Source/T66/Gameplay/*Mob*`):
  only `CustomDataFrame` hits in `T66MobManagerSubsystem.cpp/.h`. These are the
  B.11/B.12 manager VAT flat-state fields, not an instanced renderer.
- Engine instanced-API usage (Grep `HierarchicalInstancedStaticMeshComponent|UInstancedStaticMeshComponent|AddInstance|BatchUpdateInstancesTransforms|UpdateInstanceTransform|SetCustomDataValue` in `Source/T66`):
  all hits are pre-existing systems - projectile manager (HISM), hero range
  rings (ISM), miasma boundary/manager, main-map/tower terrain, floor-spike
  trap, character-visual outline. None is the basic-mob renderer.
- Mob renders via per-mob static mesh: `Source/T66/Gameplay/T66MobBase.h:52`
  declares `TObjectPtr<UStaticMeshComponent> VisualMesh`.
- Only B.13 reference in live source: `Source/T66/Gameplay/T66MobManagerSubsystem.h:168`
  comment "B.13 consumes this flat layout as HISM per-instance custom data" - a
  forward-looking note on the B.11/B.12 layout, not a landed renderer.
- Mob renderer files clean in working tree (narrow `git status --porcelain` on
  `T66MobBase.cpp/.h`, `T66MobManagerSubsystem.cpp/.h`,
  `T66CharacterVisualSubsystem.cpp/.h`): **empty output = no uncommitted
  changes**. No half-applied renderer change in live source.

### Camera-angle invisibility bug

Absent from live by construction: the camera-angle invisibility behavior was a
HISM-experiment artifact in the worktree, and no HISM renderer landed in live
source (verified above). There is no live HISM mob renderer that could exhibit
it.

### Build / run proof

- Fresh current-tree compile: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReload`
  completed with `Result: Succeeded` in `6.30` seconds. The only warning was the
  known `T66Mini.Build.cs` missing referenced directory warning for
  `Source\T66Mini\Public\UI\Components`.
- Fresh current-tree boot/quit: `C:\UE\T66\Binaries\Win64\T66.exe -nullrhi -nosound -unattended -NoSplash -ExecCmds=Quit -abslog=C:\UE\T66\Saved\StandaloneLogs\T66_B13_NoLand_Closeout_BootQuit2.log -forcelogflush`
  returned `ExitCode=0`. The requested abslog was not emitted in this headless
  run, but the process exited cleanly without leaving a live `T66.exe` process.
- Existing run (per-mob renderer at saturation): `Saved/StandaloneLogs/T66_RetroFXOffByDefault_FullResEnemyWavePerf.log`
  (2026-05-28 22:01), result `Saved/Codex/Performance/RetroFXOffByDefaultFix/full_res_enemywaveperf_result.json`:
  90 saturated live regular mobs rendering full-resolution (1920x1080,
  `r.ScreenPercentage=100`) at 146.30 avg FPS - the per-mob static-mesh renderer
  driving the saturation scene.
- Existing boot of current staged binary: newest staged PerformanceSystem session
  `20260529T025953Z_U8tNlE86orbvuaiB3Xvw9A` (2026-05-28 23:59) - FrontendLevel,
  clean `SubsystemDeinitialize`, balanced write queue. Confirms the staged
  binary launches.

## Isolated Worktree Disposition

- Path: `C:\UE\T66_B13_Worktree`. Size: **51 GB**.
- Not a git worktree: `git rev-parse` inside it returns "not a git repository",
  and there is no `.git` link file. It is a plain robocopy of the live repo made
  2026-05-28 22:27 (`Reports/AgentReviews/20260529_B13_MobHISM_VAT/isolated_copy_robocopy.log`),
  excluding `.git`, `Saved`, `Intermediate`, `Binaries`, `DerivedDataCache`,
  `.vs`. Deletion needs no Git operation - it is an ordinary directory removal.
- B.13 raw evidence lives only under
  `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\` (capture
  `*_capture_results.json`, `*_capture_rows.jsonl`, `*_capture_progress.jsonl`,
  `*Proof*.log`, and `screenshots/*.png`).
- Evidence preservation status: **safe**.
  - All numeric findings, SHA256 hashes, and correctness-proof markers are
    already in the live repo: `Reports/AgentReviews/20260529_B13_MobHISM_VAT/combined_packet.md`,
    the Lightweight Actor plan, and the new audit.
  - As an additional safeguard, the load-bearing raw result JSONs (16) and
    correctness-proof logs (19) were copied into
    `Reports/AgentReviews/20260529_B13_NoLand_Closeout/preserved_worktree_evidence/`
    (1.9 MB).
  - Not copied: large `*_capture_rows.jsonl` / `*_capture_progress.jsonl` and the
    screenshots. Their content is summarized/described in the combined packet and
    audit; the visual sanity screenshots are referenced by path. If the user
    wants pixel-level visual proof retained, copy `screenshots/*.png` before
    deletion.
- Recommendation: the worktree (51 GB) is safe to delete to reclaim disk - all
  decision-relevant evidence is preserved in the live repo. **Per scope, it was
  NOT deleted.** Deletion is left to the user/Validator. If a future renderer
  pass (B.13R) is likely soon, the user may prefer to keep it as a warm sandbox.

## Review / Validation

This is an Operator work artifact, not a greenlight. Codex validates the actual
changed files, the verification evidence, scope adherence, and writes the final
user-facing report.

## Token Routing

```text
TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: Unavailable (not self-observable mid-run; see ClaudeDirectRead manifest)
OperatorRunDir: Reports/AgentReviews/ClaudeDirectRead/<this run>
OperatorManifest: Reports/AgentReviews/ClaudeDirectRead/<this run>/manifest.json
CodexApprovalPath: Reports/AgentReviews/20260529_B13_NoLand_Closeout/codex_operator_approval.md
ExpectedValidatorDepth: targeted
ValidatorBudgetHint: confirm (1) no Content/**/*Instanced*.uasset, (2) mob source has no HISM/ISM component, (3) audit candidate table matches combined_packet.md, (4) plan + pending_issues point at audit and read closed, (5) worktree not deleted.
```

```text
## Token Ledger
AuthoringTokens: Claude:Unavailable
ReviewTokens: Codex:105141 (latest completed Codex turn before final answer)
FinishTokens: Codex:Unavailable (final report turn not yet flushed)
OperatorIsAuthoring: YES
PerModel: Claude=Unavailable, Codex=105141
TargetMet: Unavailable
Notes: Operator (Claude) did the authoring; Claude self-token count is not observable from inside the run and should be read from the ClaudeDirectRead manifest by the Finisher.
```

## Caveats

1. The fresh headless boot/quit returned `ExitCode=0`, but did not produce the
   requested abslog file. Saturated per-mob renderer evidence therefore still
   comes from the existing full-res `enemywaveperf` run listed above.
2. `combined_packet.md` and worktree-only raw artifacts (rows/progress JSONL,
   screenshots) were left in place; only result JSONs and proof logs were copied
   into the durable closeout folder.
3. Worktree intentionally not deleted (out of scope). Disposition: safe to
   delete, recommendation recorded.
4. Line 1935 of the plan ("...B.13 remains mob VAT/ISM work") sits inside a
   completed-pass historical verification block and was left unedited to avoid
   rewriting historical evidence; the authoritative status is set by the B.13
   section banner, the pass-table row, and the summary line.
