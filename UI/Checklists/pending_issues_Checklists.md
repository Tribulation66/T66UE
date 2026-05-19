# Main Menu Checklist Baseline Needs Refresh

Severity tag: [Minor]

What's wrong: `UI/Checklists/main_menu_checklist.md` still expects the previous Main Menu structure, including `MainMenu.Right.Panel`, `MainMenu.Right.ScoreScopeButton`, `MainMenu.Right.SpeedrunScopeButton`, `MainMenu.Right.TypeDropdown`, and `MainMenu.Right.ModeDropdown`. The current native screen intentionally removed/replaced those controls during the Main Menu layout pass.

Why it's out of scope now: This pass implemented and verified the runtime Main Menu screen changes. Refreshing the whole checklist requires choosing a new accepted baseline capture/dump and updating all intentional content-delta expectations for live Steam friend data.

What fixing it would entail: Capture an accepted Main Menu baseline, update `main_menu_checklist.md` to the new tags and geometry (`TimeWeeklyButton`, `TimeAllTimeButton`, `PartySizeDropdown`, `DifficultyDropdown`, metric check buttons, friend rows, and top-bar Home label), then rerun `Scripts/VerifyUIFidelity.py` until only accepted content deltas remain.
