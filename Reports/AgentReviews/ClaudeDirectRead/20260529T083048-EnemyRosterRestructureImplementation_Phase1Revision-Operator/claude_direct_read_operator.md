All five Codex validation gaps are closed. Summary of what this revision did:

**Finding 1 — UniqueDebuff projectile fully removed.** Deleted `T66UniqueDebuffProjectile.{h,cpp}`, the `ProfileUniqueDebuff` decl/def + color/shape table entries, the perf/lag active-projectile counter contributions, the Backrooms cleanup filter, and the Overlays debug-preview spawn. `grep UniqueDebuff|DebuffEnemy|DebuffProjectile|ProfileUniqueDebuff` → 0 hits in Source and Content/Data.

**Finding 2 — mob-floor rename completed.** Renamed the three previously-deferred fields (`GameplayFloorsPerStage`, `InitialEnemiesPerGameplayFloor`, `InitialTowerEnemiesPerGameplayFloor`) in source, `PlayerExperience.json` (all 5 blocks), and the roster validator. Added 3 `PropertyRedirects` to bridge the existing `DT_PlayerExperience.uasset` until Phase 2. `grep GameplayFloor` in Source → 0 hits.

**Finding 3 — VendorToken canonical.** Renamed all runtime symbols (functions, members, helpers); preserved save compat via marked legacy save fields, the deprecated `GamblerToken` enum, the `Item_GamblersToken` alias, and 2 `FunctionRedirects`. Also fixed a latent double-count (the inventory recompute now skips canonical `VendorToken`, not just legacy `GamblerToken`) — flagged in the packet for sign-off. Remaining `GamblersToken` hits are all explicitly legacy.

**Finding 4 — archetype docs closed.** Both `pending_issues` docs updated to record that `Exploder`/`Stutterer`/`Burrower` were dropped (live roster uses only `Melee`/`Rush`/`Flying`/`Ranged`).

**Finding 5 — packet rewritten** with section status, exact legacy exceptions, all verification greps + results, and the Phase 2 deferral list.

No editor build, no Git writes, no binary-asset mutation. This is an Operator artifact for Codex re-validation, not a greenlight.
