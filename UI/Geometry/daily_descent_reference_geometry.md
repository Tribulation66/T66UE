# Daily Descent Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Daily.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Daily Descent V3 reference image at native resolution. The slim top bar is included here because the verified shared top-bar widget needed a Daily-only flat slim branch.

## Slim Top Bar

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| FrontendTopBar.OuterContainer | `(0.010, 0.013, 0.981, 0.104)` | `+/-0.010` | Slim top-bar outer container. |
| FrontendTopBar.SettingsButton | `(0.023, 0.028, 0.044, 0.073)` | `+/-0.008` | Settings cog single-action button. |
| FrontendTopBar.GlobeButton | `(0.083, 0.028, 0.045, 0.074)` | `+/-0.008` | Language/globe single-action button. |
| FrontendTopBar.BackToMainMenuButton | `(0.322, 0.028, 0.347, 0.076)` | `+/-0.008` | Center BACK TO MAIN MENU button. |
| FrontendTopBar.PowerButton | `(0.919, 0.028, 0.050, 0.076)` | `+/-0.008` | Power/quit single-action button. |

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| DailyDescent.Root | `(0.013, 0.137, 0.968, 0.803)` | `+/-0.012` | Daily-owned body below the slim top bar. |
| DailyDescent.LeftPanel | `(0.013, 0.153, 0.279, 0.774)` | `+/-0.010` | Rules of the day container. |
| DailyDescent.CenterArt | `(0.305, 0.130, 0.365, 0.465)` | `+/-0.014` | Preserved gold idol/halo artwork crop. |
| DailyDescent.Title | `(0.345, 0.594, 0.306, 0.066)` | `+/-0.012` | DAILY DESCENT title. |
| DailyDescent.Subtitle | `(0.359, 0.675, 0.270, 0.028)` | `+/-0.012` | Purple subtitle line. |
| DailyDescent.StartButton | `(0.321, 0.722, 0.344, 0.102)` | `+/-0.010` | Selected red primary CTA. |
| DailyDescent.ContinueButton | `(0.321, 0.847, 0.345, 0.095)` | `+/-0.010` | Default purple secondary CTA. |
| DailyDescent.RightLeaderboardPanel | `(0.690, 0.233, 0.291, 0.707)` | `+/-0.010` | Daily Global Chad Rankings panel. |

## Rules Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| DailyDescent.LeftPanel.Header | `(0.075, 0.183, 0.144, 0.043)` | `+/-0.012` | RULES OF THE DAY header. |
| DailyDescent.LeftPanel.IntroPanel | `(0.022, 0.235, 0.261, 0.091)` | `+/-0.010` | Info row sub-panel. |
| DailyDescent.LeftPanel.IntroIcon | `(0.032, 0.254, 0.027, 0.054)` | `+/-0.012` | Info icon. |
| DailyDescent.LeftPanel.IntroText | `(0.073, 0.252, 0.172, 0.056)` | `+/-0.014` | Intro copy. |
| DailyDescent.LeftPanel.HeroRow | `(0.023, 0.356, 0.257, 0.073)` | `+/-0.012` | Hero Selected stat row. |
| DailyDescent.LeftPanel.DifficultyRow | `(0.023, 0.450, 0.257, 0.070)` | `+/-0.012` | Difficulty stat row. |
| DailyDescent.LeftPanel.ModifiersHeader | `(0.029, 0.551, 0.126, 0.036)` | `+/-0.012` | MODIFIERS sub-header. |
| DailyDescent.LeftPanel.PocketDraftRow | `(0.023, 0.594, 0.257, 0.096)` | `+/-0.012` | Pocket Draft modifier row. |
| DailyDescent.LeftPanel.IronParadeRow | `(0.023, 0.690, 0.257, 0.101)` | `+/-0.012` | Iron Parade modifier row. |
| DailyDescent.LeftPanel.DoubleDropRow | `(0.023, 0.791, 0.257, 0.118)` | `+/-0.012` | Double Drop modifier row. |

## Leaderboard

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| DailyDescent.LeaderboardTabs.GlobalButton | `(0.696, 0.137, 0.080, 0.077)` | `+/-0.010` | Global scope tab, selected. |
| DailyDescent.LeaderboardTabs.FriendsButton | `(0.792, 0.137, 0.088, 0.077)` | `+/-0.010` | Friends scope tab. |
| DailyDescent.LeaderboardTabs.BroadcastButton | `(0.895, 0.137, 0.080, 0.077)` | `+/-0.010` | Broadcast/streamers scope tab. |
| DailyDescent.RightLeaderboardPanel.Header | `(0.731, 0.258, 0.208, 0.038)` | `+/-0.012` | DAILY GLOBAL CHAD RANKINGS header. |
| DailyDescent.RightLeaderboardPanel.Row01 | `(0.704, 0.334, 0.264, 0.048)` | `+/-0.010` | #1 CROWNED CHAD. |
| DailyDescent.RightLeaderboardPanel.Row05 | `(0.704, 0.525, 0.264, 0.048)` | `+/-0.010` | #5 NO HIT NATE. |
| DailyDescent.RightLeaderboardPanel.Row10 | `(0.704, 0.764, 0.264, 0.048)` | `+/-0.010` | #10 SKULL FARMER. |
| DailyDescent.RightLeaderboardPanel.PlayerSeparator | `(0.701, 0.837, 0.268, 0.004)` | `+/-0.012` | Red separator before player row. |
| DailyDescent.RightLeaderboardPanel.PlayerRow | `(0.704, 0.864, 0.264, 0.062)` | `+/-0.010` | #42 DOPRA 118700. |
