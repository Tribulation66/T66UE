# T66 Direct Entry Automation

Direct entry exists so automation and manual debugging can reach screens, maps, and run modes without mouse clicks.

## Command Line

Preferred entry point:

```powershell
T66.exe -T66Entry=Screen:Settings
T66.exe -T66Entry=Screen:HeroSelection
T66.exe -T66Entry=Run:TestRoom -T66Hero=Hero_2
T66.exe -T66Entry=Run:Lab -T66Hero=Hero_2
T66.exe -T66Entry=Run:Tutorial -T66Hero=Hero_2
T66.exe -T66Entry=Run:Tower -T66Hero=Hero_2
```

Short aliases are accepted:

```powershell
T66.exe -T66Entry=Settings
T66.exe -T66Entry=TestRoom
T66.exe -T66Entry=Test
```

Optional arguments:

- `-T66Hero=<HeroID>` defaults to `Hero_2`, the Lu Bu / Chinese Chad row.
- `-T66Companion=<CompanionID|None>` defaults to no companion.
- `-T66Difficulty=<Easy|Medium|Normal|Hard|VeryHard|Expert|Impossible>` defaults to Easy and is release-filtered by `UT66GameInstance`.
- `-T66Modal=<ScreenAlias>` opens a modal after a frontend screen when the modal is registered for that frontend context.

Legacy compatibility:

```powershell
T66.exe -T66AutomationTestRoom
```

This is retained as an alias for `-T66Entry=Run:TestRoom`.

## Runtime Console

The same resolver is available in-game:

```text
T66.Entry Screen:Settings
T66.Screen HeroSelection
T66.Run TestRoom Hero=Hero_2
T66.Run Lab Hero=Hero_2 Difficulty=Medium
```

`T66.Screen` opens directly when already in the frontend. From gameplay it queues the screen and opens `FrontendLevel`.

`T66.Run` configures the run state and transitions through `UT66GameInstance::TransitionToGameplayLevel()`. Direct-entry runs are leaderboard-ineligible by default because they are automation/debug starts.

## Automation Notes

- This layer uses existing game paths: `PendingFrontendScreen`, `UT66UIManager::ShowScreen`, `SelectedRunCategory`, and `TransitionToGameplayLevel()`.
- It is intentionally not a mouse automation surface.
- For Steam Deck validation, this should be paired with controller focus checks per screen. Direct entry gets the build to the target screen; controller navigation should be validated there using focus and gamepad input rather than pointer position.

## First Useful Smoke Commands

Frontend screen without clicking:

```powershell
Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -T66Entry=Screen:Settings -windowed -ExecCmds="quit"
```

TestRoom without clicking through Hero Selection:

```powershell
Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe -T66Entry=Run:TestRoom -T66Hero=Hero_2 -windowed
```
