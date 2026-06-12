Packet written to the required path. Completion summary:

**Wrote:** `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\operator_plan_packet.md` (the only file in scope; no source/data/config/content edits, no deletes, no git/build/stage).

**All nine required sections present:** Operator Packet, Task Contract, Live Anchor Findings, CoreRedirect Verification Plan, Implementation Plan, Files/Paths To Touch, Verification Plan, Risks/Decisions, Codex Approval Request.

**Anchors re-verified live this pass** (not carried blindly from the prior draft), and two things materially differ from the earlier read-only summary:

- **Correction R1 (projectile scope):** `AT66EnemyProjectileBase` is **not** dead code — it's the live base of `AT66SpitProjectile`, which is still spawned at `T66PlayerController_Overlays.cpp:4278` and referenced by a live `TSubclassOf` property in `T66MobBase.h:154`. Deleting it is not behavior-neutral. The packet recommends descoping it and deleting only `AT66BossProjectile` (clean per the 2026-05-28 boss-manager migration), and flags the scope decision to Codex.
- **5 inert CVars, not 4:** found `T66.Mob.Diagnostics.RouteRushLightweight` (`T66MobManagerSubsystem.cpp:2233`) alongside the four named in the brief.

**Codex correction applied:** GamblerToken save fields (`ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`) are treated as delete-outright per Pablo's no-migration decision — not a blocker — with the effect stated plainly (one-time save reset; Vendor Token persistence becomes canonical-only). CoreRedirect removal stays conditional on a clean text + binary `.uasset` scan, retain-on-inconclusive.

**B.13 worktree** confirmed to still exist at `C:\UE\T66_B13_Worktree`; deletion procedure is path-asserted and evidence-gated.

This is a planning artifact only — implementation awaits Codex validation and a separate FullOperator mandate.

