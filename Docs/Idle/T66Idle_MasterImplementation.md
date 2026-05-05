# T66Idle Master Implementation

## Scope

T66Idle is the isolated runtime module skeleton for an offline-progress, Tap-Titans-style Idle Chadpocalypse mode. It follows the existing `T66Mini` and `T66TD` split: runtime code lives in `Source/T66Idle`, loose content belongs under `Content/Idle`, source art and pipeline inputs belong under `SourceAssets/Idle`, and implementation notes live in `Docs/Idle`.

This first pass is infrastructure-only. It does not add central routing, project module registration, production data tables, backend calls, generated images, or finished gameplay loops.

## Runtime Shape

- `T66Idle.Build.cs` mirrors the lean TD dependency set and stages `Content/Idle/...` plus `SourceAssets/Idle/...` as loose runtime dependencies.
- `UT66IdleDataSubsystem` owns seed definitions for heroes and zones until authored data is introduced.
- `UT66IdleFrontendStateSubsystem` owns transient menu/session selections and the current profile snapshot for UI handoff.
- `UT66IdleProfileSaveGame` stores offline-capable profile state, including highest stage, gold, damage numbers, and the last saved UTC timestamp.
- `UT66IdleSaveSubsystem` loads, caches, saves, and applies a capped offline-progress pass from the local save profile.
- `UT66IdleMainMenuScreen` is a minimal Slate screen for compile-time UI integration testing once the parent module wires routing.

## Deferred Integration

- Add `T66Idle` to `T66.uproject` and target module lists in the parent integration pass.
- Add shared screen route IDs and navigation entries in the base `T66` module.
- Replace seed data with CSV/DataTable or asset-authored definitions under `Content/Idle/Data`.
- Add runtime battle, stage, hero upgrade, prestige, and economy subsystems after the module is registered.
- Backend sync, anti-cheat validation, leaderboards, seasons, and monetization are deferred and should not be assumed by this skeleton.

## Offline Progress Contract

Offline progress should be deterministic and local-first for the initial mode:

- Save UTC time on profile save.
- On load, parse the last saved UTC timestamp and clamp elapsed time to a configured maximum.
- Convert elapsed time through authored passive DPS and economy formulas.
- Surface the applied rewards to frontend state so the eventual UI can present an offline recap.
- Persist the post-claim profile immediately after rewards are accepted.

Backend authority can be evaluated later if ranked or competitive Idle state is added.
