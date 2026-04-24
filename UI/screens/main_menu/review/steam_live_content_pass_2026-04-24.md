# Main Menu Steam Live Content Pass - 2026-04-24

## Capture

- Packaged capture: `UI/screens/main_menu/outputs/2026-04-24/steam_leaderboard_identity_pass_final.png`
- Resolution: 1920x1080
- Staged executable: `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`

## Review Notes

- Leaderboard rank, player name, score, mode filters, and status text are runtime Slate text layered above generated shell art.
- Leaderboard avatar cells now use a regenerated transparent socket frame over the runtime Steam/API portrait image; the frame no longer covers the portrait center.
- Leaderboard identity resolution is live Steam data first: Steam persona name and Steam avatar texture by `steam_id`, then backend avatar URL if supplied, then an empty/default socket. Hero portraits are no longer used as leaderboard identity fallback.
- Friend/profile/party surfaces remain generated shells and wells, with Steam/API profile names, Steam status, friend lists, avatars, and invite state owned by runtime widgets.
- Packaged review should treat avatar/profile/friend/leaderboard text and image regions as live-content regions. Compare shells, spacing, clipping, and sockets strictly; fixture or mask dynamic Steam/API values.

## Validation

- `python C:\UE\T66\Scripts\ValidateMainMenuReferencePack.py` -> 0 failures, 0 warnings.
- `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development` -> succeeded.
- `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Screen MainMenu -DelaySeconds 10 -TimeoutSeconds 75 -Output C:\UE\T66\UI\screens\main_menu\outputs\2026-04-24\steam_leaderboard_identity_pass_final.png` -> succeeded.
- `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Tools\Steam\UploadToSteam.ps1 -BuildSource C:\UE\T66\Saved\StagedBuilds\Windows -Description "Steam leaderboard identity pass 2026-04-24 clean depot"` -> uploaded clean Steam build `22947092`.
