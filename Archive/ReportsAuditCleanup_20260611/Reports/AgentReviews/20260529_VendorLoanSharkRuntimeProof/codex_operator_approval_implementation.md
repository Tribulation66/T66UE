Codex Approval: APPROVE

Approved scope:
- Implement non-shipping AutoQA proof routes for:
  - `-T66GameplayAutoCapture=vendorfailedstealproof`
  - `-T66GameplayAutoCapture=loansharkdebtproof`
- Touch only the source files needed for those proof hooks and minimal `#if !UE_BUILD_SHIPPING` automation accessors.
- Build `T66Editor Win64 Development`.
- Stage standalone using the repo's standard staged build path.
- Record SHA256 of `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Run both proof modes on the staged `T66.exe`.
- Write logs under `C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\logs\`.
- Write completion packet to `C:\UE\T66\Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\implementation_completion.md`.

Required correction to the plan:
- Final proof must be from the staged standalone binary, not editor-only `-game`.
- Vendor summary must include `StealAttempted`, `StealFailed`, `VendorBossSpawned`, `VendorBossIdentity`, `VendorBossDefeated`, `VendorTokenDropped`, `HiddenBossCount`, `CasinoAngerSpawnedBoss`, and `Pass`.
- Loan Shark summary must include debt setup, pending, spawn, chasing, low/high speed and damage values, scaling booleans, touch damage, despawn, and `Pass`.

Not approved:
- No production behavior changes.
- No data/config/content edits except normal generated build/stage outputs.
- No Mini/minigame work.
- No broad Git/LFS scans.
- No git stage/commit/revert/clean.
