Plan packet written to `Reports\AgentReviews\20260529_VendorLoanSharkRuntimeProof\operator_plan_packet.md`. Summary:

**Real APIs confirmed (source-read, not assumed):**
- **Vendor** — deterministic fail via `ResolveShopStealAttempt(0, false, false)` → `Miss` (zero RNG; `T66RunStateSubsystem_EconomyInventory.cpp:247`); kill via public `TakeDamageFromHeroHit(BigValue)` since both `AT66BossBase::Die()` and `AT66VendorBoss::Die()` are protected; token drop read-back via **already-public** `AT66LootBagPickup::GetItemID()` (`T66LootBagPickup.h:44`) — so the Vendor proof needs **no production-source change**, only the GameMode hook.
- **Loan Shark** — host must be `AT66GameMode` (private `LoanShark`, public `TrySpawnLoanSharkIfNeeded()`); debt driven by real `BorrowGold`/`PayDebt`; despawn reproduces the inline `T66GameMode.cpp:1213–1218` check.

**AutoQA pattern anchored** to the existing `#if !UE_BUILD_SHIPPING` `ScheduleVerificationProofIfRequested` lambda in `T66GameMode_Tower.cpp` (CLI `-T66GameplayAutoCapture=`, `[…Summary] Terminal=1 … Pass=%d`, `RequestExitWithStatus`). Proposed modes: `vendorfailedstealproof`, `loansharkdebtproof`, with full terminal markers, files-to-touch, build+capture commands, log paths, rollback, and 6 risks.

**Decision status:** No Pablo decision needed — implementation can proceed after Codex approval. The one item for **Codex** to confirm is accepting a minimal `#if !UE_BUILD_SHIPPING` `Automation*` accessor on `AT66LoanShark` (for deterministic touch-damage / damage-hearts read-back), which matches the existing `Automation*` precedent and is recommended over the less-reliable teleport-overlap fallback.

This packet is the only file written; no source/data/git changes were made.
