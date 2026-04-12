# T66 Decision Log

This file records historical decisions, active exceptions, and known debt. The current policy lives in `T66_MASTER_GUIDELINES.md`.

## 2026-04-10 Master Docs Reorganization

- The active master handoff documents were moved out of `Guidelines/` and consolidated under the new top-level `MASTER DOCS/` folder.
- Anti-cheat now has its own master reference at `MASTER DOCS/Anti Cheat/MASTER_ANTI_CHEAT.md`.
- `Docs/README.md` and the master references now treat `MASTER DOCS/` as the canonical home for active master documentation.

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
