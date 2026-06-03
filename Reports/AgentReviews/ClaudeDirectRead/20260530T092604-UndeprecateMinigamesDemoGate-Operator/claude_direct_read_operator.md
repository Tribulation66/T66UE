Done. Here's where things stand.

## Summary

Minigames are now **demo-gated invisible** instead of **deprecated**. The `bDisableMinigames` flag and `AreMinigamesDisabled()` accessor are fully removed; screen visibility now flows through the release-variant demo gate. Arcade remains deprecated and untouched.

**Result by build:**
- **Demo build** — Minigames top-bar tab absent, all minigame screens blocked (via `CanShowScreenForReleaseVariant` → `T66IsDemoGatedMinigameScreenType`, the conditional top-bar tab, and the `FrontendMinigameLocked` registry gate). The shipped demo top-bar layout is byte-for-byte unchanged.
- **Full build** — Minigames tab visible, minigame screens reachable.

**Verification done:**
- `T66Editor` Win64 Development compiled clean (exit 0, full T66 module relink, UHT `-WarningsAsErrors` passed).
- Symbol sweep confirms no dangling references; all remaining `T66DeprecatedFeatures::` calls are arcade-only; config has just the two arcade flags.

**Two things to note:**
1. **Scope disclosure** — I edited one file outside the approval's list, `T66VersusArcadeScreen.cpp`, because its back-handler referenced the removed accessor (hard compile blocker). Minimal fix, no arcade-availability change. Flagged in the packet for Codex ratification.
2. **Pending verification** — the staged standalone build + runtime capture proofs are **not run**. You dismissed the prompt, so I didn't kick off the 20-40 min cook. The packet documents them as recommended-before-sign-off.

Completion packet written to `Reports/AgentReviews/UndeprecateMinigamesDemoGate/completion_packet.md`. Just say the word if you want me to run the staged build and capture proofs.

