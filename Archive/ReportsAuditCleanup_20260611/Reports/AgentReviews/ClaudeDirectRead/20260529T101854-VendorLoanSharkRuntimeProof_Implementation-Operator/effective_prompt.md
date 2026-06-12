You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\codex_operator_approval_implementation.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
Working task:
Implement and run non-shipping runtime proof routes for Vendor failed-steal and Loan Shark debt-spawn on a staged standalone build.

Operator:
Claude (`claude-opus-4-8`, FullOperator)

Validator:
Codex

Scope:
Implement test-only hooks, build/stage, run staged captures, and write a completion packet. No production behavior changes.

Source plan:
Use `C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\operator_plan_packet.md` as the implementation basis, with the Codex corrections below.

Codex corrections to the plan packet:
1. Final acceptance captures MUST run on the staged standalone binary:
   `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
   You may use editor `-game` for debugging if needed, but final proof logs must come from staged `T66.exe`.
2. Build editor, stage standalone, record SHA256 of staged `T66.exe`, then run both proof modes on that exact staged binary.
3. Vendor terminal summary must include all requested markers:
   - `StealAttempted=1`
   - `StealFailed=1`
   - `VendorBossSpawned=1`
   - `VendorBossIdentity=1` (BossID/class is VendorBoss, not GamblerBoss)
   - `VendorBossDefeated=1`
   - `VendorTokenDropped=1`
   - `HiddenBossCount=1`
   - `CasinoAngerSpawnedBoss=0`
   - `Pass=1`
4. Loan Shark terminal summary must include:
   - `DebtSet=1`
   - `PendingSet=1`
   - `LoanSharkSpawned=1`
   - `ChasingHero=1`
   - low/high debt speed values
   - low/high debt damage values
   - `SpeedScaledWithDebt=1`
   - `DamageScaledWithDebt=1`
   - `TouchDamageApplied=1`
   - `LoanSharkDespawned=1`
   - `Pass=1`
5. Keep hooks non-shipping and production-neutral. Small `#if !UE_BUILD_SHIPPING` `Automation*` accessors on `AT66LoanShark` are approved if needed for deterministic proof.

Implementation expectations:
- Add `-T66GameplayAutoCapture=vendorfailedstealproof`.
- Add `-T66GameplayAutoCapture=loansharkdebtproof`.
- Logs go under `C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\logs\`.
- Completion packet goes to `C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\implementation_completion.md`.
- If a foundation differs from the plan, report it and either adapt safely within the approved test-only scope or stop if adaptation would change production behavior.

Verification required:
- `T66Editor Win64 Development` build succeeds.
- Staged standalone BuildCookRun succeeds.
- Record staged `T66.exe` SHA256.
- Run both staged proof modes and capture logs.
- Confirm both terminal summaries show all markers and `Pass=1`.
- Confirm clean process exit for both staged proof runs.

Do not:
- Do not change production behavior.
- Do not edit data/config/content except unavoidable build outputs under staged/saved locations.
- Do not touch Mini/minigame systems.
- Do not run broad Git/LFS scans.
- Do not stage/commit/revert/clean.

