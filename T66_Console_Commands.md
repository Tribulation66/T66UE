# T66 Console Commands

This file lists **all currently registered** T66 console commands / CVars found in code (C++).

Notes:
- Open the Unreal console in-game with `~` (tilde) or via the standard UE console keybind.
- Some commands are **editor-only** (live in `Source/T66Editor/**`) and won’t exist in a packaged build.

---

## Runtime (Game) — available in `T66` / `T66Editor`

### `clearlead`
- **Type**: command
- **What it does**: clears the **local leaderboard** save and all local best bounty run summary snapshot saves, then recreates an empty local leaderboard save.
- **Alias**: `clear.lead`
- **Defined in**: `Source/T66/Core/T66LeaderboardSubsystem.cpp`

### `clear.lead`
- **Type**: command (alias)
- **What it does**: alias for `clearlead`
- **Defined in**: `Source/T66/Core/T66LeaderboardSubsystem.cpp`

### `sus1`
- **Type**: command
- **What it does**: debug shortcut for `t66.AccountStatus.Force 1` (forces Account Status = Suspicion)
- **Defined in**: `Source/T66/Core/T66LeaderboardSubsystem.cpp`

### `sus2`
- **Type**: command
- **What it does**: debug shortcut for `t66.AccountStatus.Force 2` (forces Account Status = Cheating Certainty)
- **Defined in**: `Source/T66/Core/T66LeaderboardSubsystem.cpp`

### `t66.AccountStatus.Force`
- **Type**: console variable (CVar)
- **Values**:
  - `0`: normal (no forced Account Status)
  - `1`: Suspicion
  - `2`: Cheating Certainty
- **Defined in**: `Source/T66/Core/T66LeaderboardSubsystem.cpp`

### `Pixel1` … `Pixel10`
- **Type**: command (no arguments)
- **What it does**: Sets **retro pixelation** level by lowering internal render resolution and scaling up. **Pixel1** = very slight (100%), **Pixel10** = strong (25%). Affects the whole frame (scene + UI). Console only; not persisted (resets on restart).
- **Defined in**: `Source/T66/Core/T66Pixelation.cpp`

---

## Runtime (Game) — In-game Dev Console (GameplayLevel)

### `Enter` (keybind)
- **Type**: keybind (debug)
- **What it does**: opens the **in-game dev console overlay** where you can type commands (e.g. `clearlead`) while the game is running.
- **Close**: `Esc` (dev console has priority over Pause)
- **Defined in**: `Source/T66/Gameplay/T66PlayerController.cpp`

---

## Editor-only — available only in `T66Editor`

### `T66Setup`
- **Type**: command
- **What it does**: runs the T66 full auto-setup pipeline (configures Blueprints, levels, etc.)
- **Defined in**: `Source/T66Editor/T66UISetupSubsystem.cpp`

