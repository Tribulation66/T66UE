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
