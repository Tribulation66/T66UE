# UI Reimagine — Overnight Run Log

Contract: `run_contract.md` (binding). Sentinel: scheduled task `ui-reimagine-sentinel`
(every 30 min; no-ops while heartbeats here are <35 min old; takes over if stale).

## Run start — 2026-06-10

- [03:43] HEARTBEAT. Run start (delayed from 06-09 ~21:05 — session paused mid-setup;
  sentinel never took over; zero reimagine work had happened. Starting fresh.)
- [03:43] State: HEAD = 8b6c08aea (motion-rig lane on top of 1.2). 630 foreign dirty
  files in tree (material-migration pass, finished-but-uncommitted, NOT mine — never
  git add them). UI battlefield: only 5 dirty UI files (stats cluster) ->
  DEFER LIST: T66HeroSelectionScreen_Stats.cpp, T66PowerUpScreen.cpp,
  T66RunSummaryScreen.cpp, T66StatsPanelSlate.cpp/.h. Screens touching ONLY those files
  wait; everything else proceeds.
- [03:43] Usage: Claude 5h 96% left / weekly 82% left. Codex 5h 100% / weekly 7% left
  (resets tonight 00:24). BUDGET PLAN: spend Codex down to 98% used max tonight on
  highest-leverage art (bible -> 2 north stars -> kit families in priority order);
  everything else Claude-side; remaining imagegen resumes after the 00:24 reset
  (sentinel carries the run across).
- [03:43] North star: SELF-APPROVED per user waiver ("too tired to stay up... just go
  ahead and proceed"). Anchor = candy minimap assets (ring F, 10 icons, in-game crop).
- [03:46] NIGHT MANIFEST (exposure order; D = full/partial defer due to foreign-dirty files):
  1 HUD (hudreview + enemywaveperf) GO · 2 MainMenu GO · 3 HeroSelection PARTIAL-D
  (avoid _Stats.cpp) · 4 PauseMenu GO · 5 Casino vendor+gambler GO · 6 IdolAltar GO ·
  7 Settings GO · 8 LoadGame GO · 9 Lab/Crate/Collector GO · 10 RunSummary D ·
  11 PowerUp D · 12 tail (GameOver, modals, Challenges, grids, LanguageSelect,
  CompanionSelection, loading) = kit-inherit + bespoke as budget allows.
  KIT FAMILIES (Codex priority): F1 panels+buttons · F2 controls (tab/dropdown/checkbox/
  slider/toggle/scrollbar) · F3 rows/pills/progress/wells · F4 modal+tooltip · F5 HUD-calm
  variants. Expect F1-F3 tonight within budget; F4-F5 + per-screen refs after 00:24 reset.
  KIT WIRING: second Candy descriptor table in FT66FriendslopStyle (same enum), selected
  by CVar `T66.UI.CandyKit` (default 1) — per-asset margins tuned from generated plates;
  fallback to current table when a candy asset is missing. Zero overwrites.
- [03:46] In flight: restage (baselines) + style-bible worker, parallel.
- [03:43] REVERT DESIGN (user requirement "easy to revert"): (1) all new art in NEW
  folders under RuntimeDependencies/T66/UI/FriendslopStyle/Candy/ — zero overwrites;
  (2) single switch CVar `T66.UI.CandyKit` (default 1) routes FriendslopStyle
  descriptors to Candy assets, 0 = today's look instantly; (3) per-screen commits
  prefixed "UI Reimagine:" from start hash 8b6c08aea. Revert = CVar flip, or
  git revert of prefixed commits, or both.
