Phase 3 complete. All seven tasks are done.

## Summary

**Demo Gating Visibility Phase 3** — demo-gated, non-deprecated, non-Mini UI entries are now **hidden** from visible lists/buttons instead of shown with a `COMING SOON` overlay. Backend/navigation guards are untouched.

**Code (8 approved files):**
- Heroes (carousel + grid): source lists swapped to `GetPlayableHeroIDs()`; inert overlay wraps removed.
- Companions (selection, grid, hero-selection wheel): swapped to `GetPlayableCompanionIDs()`; progression-locked-but-allowed companions still show as locked tiles.
- Difficulties (hero-select, companion-select, leaderboard): built from `GetPlayableDifficulties()` / skip non-playable — demo exposes only Easy.
- Lab button: slot omitted when not playable.
- Daily Descent CTA: slot omitted when unavailable; screen nav guard intact.
- Casino: gated cards return `SNullWidget`; "More Games" button collapses when its sub-view would be empty.

**Docs:** `DEMO_RELEASE_INSTRUCTIONS.md` and `DEMO_GATED_INVISIBLE_CONTENT.md` updated to the hidden-entry model.

**Verification:** focused compile passed — `Result: Succeeded` (55.47s). Remaining `WrapWithComingSoonOverlay` usages classified as available-now no-ops (Phase 1 unlocks / now-filtered lists), backend guards, or deprecated/Mini-excluded — none are still-overlaying demo gates on approved surfaces.

**Notes:** the casino build file and the invisible-content doc are untracked git files from prior phases (edits are on disk and compiled). Pre-existing `M` files (PowerUp, Achievements, HUD, etc.) were left untouched. No git operations or staged-standalone run performed — Phase 4 owns staging/capture proof.

Completion packet: `Reports/AgentReviews/DemoGatingVisibility/phase3_completion_packet.md`.

One judgment call worth flagging for validation: I edited the casino "More Games" button visibility (beyond pure card-hiding) to avoid introducing a dead-end empty sub-view in demo — documented in the packet's caveats.

