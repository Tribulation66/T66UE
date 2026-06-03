Phase 2 complete. Three Markdown files written, all docs-only, anchored to current source/config.

**Docs created:**
- `Demo/DEMO_GATED_INVISIBLE_CONTENT.md` — central seam table plus per-item entries (extra heroes, extra companions, non-Easy difficulties, Daily Descent, Lab, Collector, extra casino games, the COMING SOON string), each with controlling seam + restore step. Separate section marks Phase-1 items (drugs, diploma upgrades, Steam/Secret achievements) as no longer demo-gated.
- `Demo/DEPRECATED_CONTENT.md` — central `UT66DeprecatedFeatureSettings` feature gates (arcade games, arcade interactables, minigames) as documentation-only entries, a distinct compatibility-retained section for deprecated-but-kept data fields, and the arcade allow-list cross-reference clarifying it isn't a re-enable.
- `Reports/AgentReviews/DemoGatingVisibility/phase2_completion_packet.md` — outcome, docs changed, full anchor list, no-change confirmation, token ledger, caveats.

**Key anchors:** `Config/DefaultDemoMode.ini`, `Config/DefaultGame.ini` deprecated block, `T66ReleaseVariantSubsystem.{h,cpp}`, `T66GameInstance.cpp` wrappers, `T66UIManagerReleaseVariant.cpp` screen gate, the `T66DemoModeUIUtils` overlay helper, and the Hero Selection / Main Menu / PowerUp / Achievements UI consumers.

No code, config, or asset files were touched; no Git/LFS scans or binary-asset inspection were performed. Two things I flagged as Phase-3/judgment items rather than acting on them: `GetVisibleDifficulties()` still lists non-Easy difficulties (overlay, not hidden), and Daily Descent is hard-gated to demo mode in code with no config flag — restoring it needs a code change.

This is an Operator work artifact, not a greenlight — it's ready for Codex to validate the actual files and write the final user-facing report.

**Codex Token Spent:**
Unavailable (helper not run; running `Scripts\Get-CodexTokenUsage.ps1` is Codex's step as Validator/Finisher)

**Claude Tokens Spent:**
Unavailable
