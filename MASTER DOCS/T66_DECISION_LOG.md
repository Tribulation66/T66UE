# T66 Decision Log

This file records historical decisions, active exceptions, and known debt. The current policy lives in `T66_MASTER_GUIDELINES.md`.

## 2026-04-18 Anti-Cheat Docs Elevation And Ranked Integrity Policy

- The active anti-cheat docs were moved out of `MASTER DOCS/Anti Cheat/` into the new top-level `ANTI_CHEAT/` folder.
- Ranked mode policy is now explicitly "pristine build only."
- Challenges, internal mods, and future external mods are intentionally unranked rather than punitive by default.
- Local leaderboard state is being redefined as a cache of backend-accepted ranked runs only; recent-run history may still remain local.
- The anti-cheat roadmap now includes a compact integrity layer built around launch/run-start baseline capture plus run-end compare, and a compact provenance layer for inventory/stat/currency/progression validation.
- Steam trusted writes are not treated as a mod detector; the T66 backend remains the authority for ranked acceptance.

## 2026-04-17 Optimization Implementation Checkpoint

- The major `V5` optimization audit work was integrated into the live repo and rebuilt staged standalone on branch `codex/version-2.3`.
- The packaged Development standalone at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe` is the current local runtime reference for this checkpoint.
- Packaged startup smoke passed for:
  - frontend boot
  - direct `GameplayLevel` startup
  - direct `T66MiniBattleMap` startup
- A packaged-only combat-presentation issue was found during direct gameplay startup: `NS_PixelParticle` was falling back to `VFX_Attack1`.
- The local fix was to add a small startup combat-presentation warmup in `UT66GameInstance`, then rebuild/restage the standalone and revalidate the packaged path.
- Build-process rule learned from this pass:
  - `-SkipCook` is not sufficient after packaged asset-reference, preload/warmup, or cook/staging changes
  - those changes require a fresh cooked standalone before packaged results are trusted

## 2026-04-10 Master Docs Reorganization

- The active master handoff documents were moved out of `Guidelines/` and consolidated under the new top-level `MASTER DOCS/` folder.
- At that checkpoint anti-cheat moved to `MASTER DOCS/Anti Cheat/MASTER_ANTI_CHEAT.md`; that location was later superseded by `ANTI_CHEAT/MASTER_ANTI_CHEAT.md` on 2026-04-18.
- `Docs/README.md` and the master references treat `MASTER DOCS/` as the canonical home for most active master documentation, with anti-cheat later split into its own top-level `ANTI_CHEAT/` folder.

## 2026-04-02 Repo Cleanup

- Root documentation and planning files were moved under `Docs/` by category.
- Legacy guideline source files were archived under `Guidelines/Archive/`.
- Ambiguous root leftovers were moved into `Docs/Archive/Root_Quarantine/` instead of being deleted outright.
- `SourceAssets/` and `ThirdParty/` were explicitly kept at the root because build/runtime code still depends on those paths.

## 2026-04-02 UI Resolution Policy

- Minimum safe layout baseline is `1280x720`.
- Steam Deck `1280x800` is a primary validation target.
- Required validation also includes `1366x768`, `1920x1080`, `2560x1440`, `3440x1440`, and `3840x2160`.
- Critical UI uses a centered safe-frame approach so ultrawide displays keep important content in the core view region.
- UE DPI scaling uses `Shortest Side`, with player UI scale layered on top.

## 2026-04-02 UI Framework Direction

- Shared UI primitives are the intended path forward:
  - `ST66Button`
  - `ST66Panel`
- Shared style/tokens/layout policy is centralized under `FT66Style`.
- New UI should not reintroduce bespoke per-screen scale systems or fixed-size text-clipping shells.

## 2026-04-02 Optional Brush Cache Safety

- A crash during Standalone teardown exposed that optional texture brush caches were holding stale brush resource pointers.
- The fix was to keep optional imported/file textures alive through strong ownership in:
  - `Source/T66/UI/Style/T66Style.cpp`
  - `Source/T66/UI/Dota/T66DotaSlate.cpp`
- Future optional resource caches should follow the same rule: the cache must own the UObject strongly enough to prevent stale Slate brush resource pointers.

## Active Exceptions / Follow-Up

- Unreal-generated folders such as `Binaries/`, `Intermediate/`, `DerivedDataCache/`, and parts of `Saved/` may still appear at the root after normal editor/build usage.
- Legacy documentation may still contain historical path references to the old `Guidelines/` folder location until those references are normalized over time.
- `Items.csv`, `T66.png`, and `waveletNoiseTile.bin` were quarantined rather than deleted because they were not clearly part of the live build/runtime surface.
