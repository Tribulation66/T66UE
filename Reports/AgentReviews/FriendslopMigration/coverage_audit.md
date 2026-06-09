# Friendslop Coverage Audit — post-merge verification (2026-06-10)

Question: beyond "FlatStyle is legacy," is there frontend/HUD chrome that was NOT FlatStyle
before, and therefore escaped the flip?

Method: 5 parallel finder agents (legacy reference chrome, direct FT66Style bypasses,
hand-rolled SBorder chrome, retro-retainer reachability, UMG/WBP surfaces) + manual
verification of the high-impact claims against live smoke-run captures
(Saved/PreReleaseSmokeSuite/20260609_175117).

## Verdict

Every player-facing surface renders the Friendslop direction, via one of three routes:

1. **The global flip** (was FlatStyle): all in-run overlays, pause, settings, save slots,
   modals, grids, HUD panels, prompts. Verified by round-1 triage sweep.
2. **Hand-migrated plate pipelines** (the 2026-06-08 per-screen passes): MainMenu, TopBar,
   AccountStatus Overview/History, Achievements tabs, PowerUp Relics/Steroids, RunSummary,
   HeroSelection (partial), tooltips, shared modal primitives. IMPORTANT NUANCE: these
   screens deliver plates through legacy-NAMED plumbing (`BuildFlatSlicedPlateButton`,
   `SourceAssets/UI/Reference/Screens/...` paths remapped at runtime, reference scrollbar
   styles, retro-retainer wrapper) — the finder agents flagged them as escapes, but the
   live captures (Account Overview, PowerUp) show regenerated dark/red rounded plates
   consistent with the direction. The art was regenerated; only the code naming is legacy.
3. **Exempt gameplay viz** (correctly not chrome): crosshair, enemy bullseye, floating
   combat text, loot wheel dial, video player, minimap canvas (candy-styled separately).

UMG/WBP: no player-facing surfaces outside C++ Slate found; suspect widgets
(combat text, enemy lock, cowardice prompt, leaderboard panel, loading screen) verified
properly routed.

## True stragglers (minor, deliberately not fixed in this pass)

- Loading screen: plain dark fill + text, no plate (lowest priority).
- Custom flat scrollbar thumbs on AccountStatus/Achievements/PowerUp/Challenges/RunSummary.
- A handful of raw SBorder accent fills inside already-plated overlays (crate glow strip,
  loot wheel result fill, idol button backing, casino scrim).
- Casino gambler tab: magenta frame + empty content — PRE-EXISTING broken state (identical
  with flip off), needs its own fix pass.
- Retro-retainer material still loads as the FX wrapper on the sliced-plate path
  (intentional subtle pixelation infra per the fidelity doc; not a style escape, but it is
  legacy plumbing a future cleanup pass could retire together with the Reference naming).

## Suggested follow-up passes (future, not blocking)

1. Gambler tab content fix (pre-existing bug).
2. Plumbing cleanup: rename/retire Reference-named loaders, fold sliced-plate delivery into
   FT66FriendslopStyle proper. STATUS 2026-06-10: DEFERRED — an active uncommitted UI pass
   (Friendslop standard modals + RetroFX retainer removal) is mid-flight in the same files
   (ScreenSlateHelpers +529, the 4 screens +1790). Execute after it commits. Scoped cut:
   Friendslop-named entry points (MakeSlicedPlateButton/MakeSlicedProgressBar/
   MakeHorizontalSlicedImage), update 6 call sites (AccountStatus:864,970; Achievements:969,
   1036; PowerUp:1151; RunSummary:958), delete the 4 FlatStyle BuildFlat* wrappers.
3. Scrollbar + loading-screen + accent-fill polish (one small pass).
4. **Retro eradication** — full spec with verified classifications and phased plan:
   `retro_removal_spec.md` (same folder). Execute together with pass 2 (same files,
   same precondition).
