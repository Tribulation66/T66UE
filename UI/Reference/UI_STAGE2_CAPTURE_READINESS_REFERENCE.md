# Stage 2 Capture Readiness

Date: 2026-05-11

Sources:
- `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md`, section 5.3
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Frontend.cpp`
- `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`

## Resolver Baseline

`TryResolveFrontendScreenName()` matches names with `ESearchCase::IgnoreCase`, but docs and pass logs should use the canonical spellings below.

Unknown `-T66FrontendScreen=<name>` values fail loudly: the frontend logs `Frontend automation: unknown screen override`, requests process exit status `66`, and `CaptureT66UIScreen.ps1` returns nonzero instead of accepting a fallback/Main Menu capture.

Stage 2 convention:

- Use the Stage 2 human/canonical screen name directly in `-Screen <Name>`.
- If that screen is a tab/category inside a parent screen, the resolver maps the name to the parent `ET66ScreenType`, and the parent screen reads `T66FrontendScreen` to activate the requested tab/category.
- Existing explicit tab flags remain supported for debugging: `-T66AccountTab=Overview|History`, `-T66PowerUpTab=Diplomas|Drugs`, `-T66AchievementsTab=Steam`, and `-T66SettingsTab=RetroFX`.

## Stage 2 Screen List

| Migration order | Stage 2 screen | Accepted `-Screen` name | Resolved parent screen | Tab/category activation | Status |
| ---: | --- | --- | --- | --- | --- |
| 2 | Overview | `Overview` | `AccountStatus` | Account screen activates Overview from `T66FrontendScreen=Overview` | Ready |
| 3 | History | `History` | `AccountStatus` | Account screen activates History from `T66FrontendScreen=History` | Ready |
| 4 | Diplomas | `Diplomas` | `PowerUp` | Power Up screen activates permanent/diploma tab from `T66FrontendScreen=Diplomas` | Ready |
| 5 | Drugs | `Drugs` | `PowerUp` | Power Up screen activates single-use/drugs tab from `T66FrontendScreen=Drugs` | Ready |
| 6 | Steam Achievements | `SteamAchievements` | `Achievements` | Achievements screen activates Steam/Achievements tab from `T66FrontendScreen=SteamAchievements` | Ready |
| 7 | Minigames | `Minigames` | `Minigames` | Direct screen | Ready |
| 8 | Settings Retro FX | `SettingsRetroFX` | `Settings` | Settings screen activates Retro FX from `T66FrontendScreen=SettingsRetroFX`; `Settings` also defaults to Retro FX currently | Ready |
| 9 | Daily Descent | `DailyDescent` | `DailyDescent` | Direct screen | Ready |
| 10 | Challenges | `Challenges` | `Challenges` | Direct screen/modal | Ready |
| 11 | Load Game | `LoadGame` | `SaveSlots` | Direct Save Slots screen through human-readable alias | Ready |
| 12 | Run Summary | `RunSummary` | `RunSummary` | Direct screen/modal | Ready |

## Accepted Alias Additions

Added to `T66PlayerController_Frontend.cpp`:

- `Overview` -> `ET66ScreenType::AccountStatus`
- `History` -> `ET66ScreenType::AccountStatus`
- `Diplomas` -> `ET66ScreenType::PowerUp`
- `Drugs` -> `ET66ScreenType::PowerUp`
- `SteamAchievements` / `Steam` -> `ET66ScreenType::Achievements`
- `SettingsRetroFX` / `RetroFX` -> `ET66ScreenType::Settings`
- `LoadGame` -> `ET66ScreenType::SaveSlots`

Parent tab/category activation implemented in:

- `T66AccountStatusScreen.cpp`: falls back from `T66AccountTab` to `T66FrontendScreen=Overview|History`.
- `T66PowerUpScreen.cpp`: falls back from `T66PowerUpTab` to `T66FrontendScreen=Diplomas|Drugs`.
- `T66AchievementsScreen.cpp`: falls back from `T66AchievementsTab` to `T66FrontendScreen=SteamAchievements|Steam`.
- `T66SettingsScreen.cpp`: supports `T66SettingsTab` and falls back from `T66FrontendScreen=SettingsRetroFX|RetroFX`.

## Audit Verdict

Zero Stage 2 capture-name gaps remain for the eleven remaining screens in master plan section 5.3.

Fresh-agent capture examples:

```powershell
.\Scripts\CaptureT66UIScreen.ps1 -Screen Overview -Output C:\UE\T66\Saved\Codex\UI\Overview\pass_01_capture.png -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\Overview\pass_01_dump.json")
.\Scripts\CaptureT66UIScreen.ps1 -Screen History -Output C:\UE\T66\Saved\Codex\UI\History\pass_01_capture.png -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\History\pass_01_dump.json")
.\Scripts\CaptureT66UIScreen.ps1 -Screen Diplomas -Output C:\UE\T66\Saved\Codex\UI\Diplomas\pass_01_capture.png -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\Diplomas\pass_01_dump.json")
.\Scripts\CaptureT66UIScreen.ps1 -Screen SteamAchievements -Output C:\UE\T66\Saved\Codex\UI\SteamAchievements\pass_01_capture.png -ExtraArgs @("-T66AutoDumpScreen=C:\UE\T66\Saved\Codex\UI\SteamAchievements\pass_01_dump.json")
```
