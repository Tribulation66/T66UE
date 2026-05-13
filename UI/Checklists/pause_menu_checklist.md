# Pause Menu Structural Preservation Checklist

Source inventory: `C:\UE\T66\UI\Geometry\pause_menu_structural_inventory.md`

Baseline capture: `C:\UE\T66\Saved\Codex\UI\PauseMenu\baseline_capture.png`

Baseline dump: `C:\UE\T66\Saved\Codex\UI\PauseMenu\baseline_dump.json`

## Structure

- [ ] PauseMenu.Root | exists=true
- [ ] PauseMenu.Scrim | exists=true
- [ ] PauseMenu.ModalPanel | exists=true
- [ ] PauseMenu.Title | exists=true
- [ ] PauseMenu.ResumeButton | exists=true
- [ ] PauseMenu.SaveAndQuitButton | exists=true
- [ ] PauseMenu.RestartButton | exists=true
- [ ] PauseMenu.SettingsButton | exists=true
- [ ] PauseMenu.AchievementsButton | exists=true
- [ ] PauseMenu.LeaderboardButton | exists=true

## Geometry

- [ ] PauseMenu.Root | x=0.000 | 0.005
- [ ] PauseMenu.Root | y=0.000 | 0.005
- [ ] PauseMenu.Root | w=1.000 | 0.005
- [ ] PauseMenu.Root | h=1.000 | 0.005
- [ ] PauseMenu.ModalPanel | x=0.342 | 0.020
- [ ] PauseMenu.ModalPanel | y=0.112 | 0.020
- [ ] PauseMenu.ModalPanel | w=0.315 | 0.020
- [ ] PauseMenu.ModalPanel | h=0.776 | 0.020
- [ ] PauseMenu.Title | x=0.445 | 0.025
- [ ] PauseMenu.Title | y=0.157 | 0.025
- [ ] PauseMenu.ResumeButton | x=0.371 | 0.020
- [ ] PauseMenu.ResumeButton | y=0.273 | 0.020
- [ ] PauseMenu.SaveAndQuitButton | y=0.377 | 0.020
- [ ] PauseMenu.RestartButton | y=0.481 | 0.020
- [ ] PauseMenu.SettingsButton | y=0.585 | 0.020
- [ ] PauseMenu.AchievementsButton | y=0.689 | 0.020
- [ ] PauseMenu.LeaderboardButton | y=0.793 | 0.020

## Colors

- [ ] PauseMenu.ModalPanel | button_state=Default
- [ ] PauseMenu.ResumeButton | button_state=Selected
- [ ] PauseMenu.SaveAndQuitButton | button_state=Default
- [ ] PauseMenu.RestartButton | button_state=Default
- [ ] PauseMenu.SettingsButton | button_state=Default
- [ ] PauseMenu.AchievementsButton | button_state=Default
- [ ] PauseMenu.LeaderboardButton | button_state=Default
- [ ] PauseMenu.ModalPanel | border_color=DefaultBorder

## Content

- [ ] PauseMenu.Title | text=PAUSED
- [ ] PauseMenu.Title | is_label=true
- [ ] PauseMenu.ResumeButton | text=RESUME GAME
- [ ] PauseMenu.SaveAndQuitButton | text=SAVE AND QUIT
- [ ] PauseMenu.RestartButton | text=RESTART
- [ ] PauseMenu.SettingsButton | text=SETTINGS
- [ ] PauseMenu.AchievementsButton | text=ACHIEVEMENTS
- [ ] PauseMenu.LeaderboardButton | text=LEADERBOARD

## Interactivity

- [ ] PauseMenu.ResumeButton | has_click_handler=true
- [ ] PauseMenu.ResumeButton | hover_capable=true
- [ ] PauseMenu.SaveAndQuitButton | has_click_handler=true
- [ ] PauseMenu.SaveAndQuitButton | hover_capable=true
- [ ] PauseMenu.RestartButton | has_click_handler=true
- [ ] PauseMenu.RestartButton | hover_capable=true
- [ ] PauseMenu.SettingsButton | has_click_handler=true
- [ ] PauseMenu.SettingsButton | hover_capable=true
- [ ] PauseMenu.AchievementsButton | has_click_handler=true
- [ ] PauseMenu.AchievementsButton | hover_capable=true
- [ ] PauseMenu.LeaderboardButton | has_click_handler=true
- [ ] PauseMenu.LeaderboardButton | hover_capable=true
