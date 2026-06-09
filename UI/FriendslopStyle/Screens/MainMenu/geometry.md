# FriendslopStyle Main Menu Reference Geometry

Reference image: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Native reference resolution: `1672x941`

Normalization basis: values below are measured from Round06 and normalized to the 1920x1080 verifier basis used by `T66.UI.DumpScreen`.

Measurement method: direct Round06 visual measurement plus red-surface component detection for the primary CTA, top-bar HOME/power controls, filter icon button, weekly button, and local ranking row. Values are target geometry for runtime Slate widgets; text remains live and is not baked into any plate.

## Overall Extents

| Tag / Region | Target BBox 1920 | Normalized BBox | Tolerance | Notes |
|---|---:|---:|---:|---|
| MainMenu.Root | `(0, 0, 1920, 1080)` | `(0.000, 0.000, 1.000, 1.000)` | `0.005` | Full screen. |
| FrontendTopBar.OuterContainer | `(0, 0, 1920, 127)` | `(0.000, 0.000, 1.000, 0.117)` | `0.002` | Top bar is dumped under `top_bar.widgets`; visible chrome must be flush to the top, left, and right reference-canvas edges. |
| MainMenu.Left.Panel | `(18, 145, 500, 892)` | `(0.009, 0.134, 0.260, 0.826)` | `0.010` | Tall social panel with full rubber shell. |
| MainMenu.Center.TitleRegion | `(520, 68, 880, 260)` | `(0.271, 0.063, 0.458, 0.241)` | `0.020` | Title branding target plus live subtitle region. |
| MainMenu.Center.CtaStack | `(640, 748, 680, 238)` | `(0.333, 0.693, 0.354, 0.220)` | `0.012` | Primary and secondary CTA stack. |
| MainMenu.Right.Root | `(1350, 130, 516, 894)` | `(0.703, 0.120, 0.269, 0.828)` | `0.012` | Horizontal icon-filter panel plus leaderboard panel. |
| MainMenu.Right.FilterPanel | `(1350, 130, 516, 90)` | `(0.703, 0.120, 0.269, 0.083)` | `0.012` | Icon-only filter panel above leaderboard body. |
| MainMenu.Right.LeaderboardPanel | `(1350, 234, 516, 790)` | `(0.703, 0.217, 0.269, 0.731)` | `0.012` | Right rubber leaderboard panel, same width class as the filter panel. |

## Top Bar

| Tag / Region | Target BBox 1920 | Normalized BBox | Tolerance | Notes |
|---|---:|---:|---:|---|
| FrontendTopBar.SettingsButton | `(24, 24, 96, 74)` | `(0.013, 0.022, 0.050, 0.069)` | `0.010` | Gear button. |
| FrontendTopBar.GlobeButton | `(136, 24, 96, 74)` | `(0.071, 0.022, 0.050, 0.069)` | `0.010` | Language button. |
| FrontendTopBar.AccountButton | `(252, 24, 292, 74)` | `(0.131, 0.022, 0.152, 0.069)` | `0.012` | ACCOUNT tab. |
| FrontendTopBar.ProfileButton | `(592, 24, 325, 80)` | `(0.308, 0.022, 0.169, 0.074)` | `0.012` | HOME selected tab. |
| FrontendTopBar.PowerUpButton | `(934, 24, 292, 74)` | `(0.486, 0.022, 0.152, 0.069)` | `0.012` | POWER UP tab. |
| FrontendTopBar.AchievementsButton | `(1242, 24, 300, 74)` | `(0.647, 0.022, 0.156, 0.069)` | `0.012` | ACHIEVEMENTS tab. |
| FrontendTopBar.TicketBadge | `(1572, 24, 172, 74)` | `(0.819, 0.022, 0.090, 0.069)` | `0.012` | Ticket pill. |
| FrontendTopBar.PowerButton | `(1776, 24, 96, 74)` | `(0.925, 0.022, 0.050, 0.069)` | `0.010` | Power button. |

## Left Panel

| Tag / Region | Target BBox 1920 | Normalized BBox | Tolerance | Notes |
|---|---:|---:|---:|---|
| MainMenu.Left.ProfileButton | `(40, 181, 460, 108)` | `(0.021, 0.168, 0.240, 0.100)` | `0.008` | Profile row. |
| MainMenu.Left.ProfileAvatar | `(52, 197, 76, 76)` | `(0.027, 0.182, 0.040, 0.070)` | `0.008` | Avatar remains live. |
| MainMenu.Left.SearchField | `(40, 301, 460, 60)` | `(0.021, 0.279, 0.240, 0.056)` | `0.008` | Search plate and live editable text. |
| MainMenu.Left.FriendsPanel | `(40, 372, 460, 466)` | `(0.021, 0.344, 0.240, 0.431)` | `0.008` | Online/offline group region. |
| MainMenu.Left.OnlineToggle | `(40, 372, 460, 42)` | `(0.021, 0.344, 0.240, 0.039)` | `0.008` | Online header. |
| MainMenu.Left.OnlineFriendRow01 | `(40, 420, 460, 58)` | `(0.021, 0.389, 0.240, 0.054)` | `0.008` | First online row. |
| MainMenu.Left.OfflineToggle | `(40, 500, 460, 42)` | `(0.021, 0.463, 0.240, 0.039)` | `0.012` | Offline header may shift with online count. |
| MainMenu.Left.OfflineFriendRow01 | `(40, 548, 460, 58)` | `(0.021, 0.507, 0.240, 0.054)` | `0.012` | First offline row. |
| MainMenu.Left.PartyPanel | `(40, 853, 460, 156)` | `(0.021, 0.790, 0.240, 0.144)` | `0.010` | Party label plus slots. |
| MainMenu.Left.PartySlot01 | `(52, 905, 94, 94)` | `(0.027, 0.838, 0.049, 0.087)` | `0.010` | Local party slot. |
| MainMenu.Left.PartySlot04 | `(382, 905, 94, 94)` | `(0.199, 0.838, 0.049, 0.087)` | `0.010` | Fourth slot. |

## Center

| Tag / Region | Target BBox 1920 | Normalized BBox | Tolerance | Notes |
|---|---:|---:|---:|---|
| MainMenu.Center.Title | `(590, 124, 730, 100)` | `(0.307, 0.115, 0.380, 0.093)` | `0.020` | Title branding target; generated title-only asset or live Slate, not full-reference crop. |
| MainMenu.Center.Subtitle | `(680, 214, 560, 58)` | `(0.354, 0.198, 0.292, 0.054)` | `0.018` | Live subtitle. |
| MainMenu.Center.EnterTribulationButton | `(640, 748, 680, 104)` | `(0.333, 0.693, 0.354, 0.096)` | `0.010` | Red primary CTA. |
| MainMenu.Center.EnterTribulationButton.Label | `(756, 784, 410, 50)` | `(0.394, 0.726, 0.214, 0.046)` | `0.020` | Live CTA label. |
| MainMenu.Center.LoadGameButton | `(650, 884, 660, 94)` | `(0.339, 0.819, 0.344, 0.087)` | `0.010` | Secondary CTA. |
| MainMenu.Center.LoadGameButton.Label | `(850, 910, 220, 48)` | `(0.443, 0.843, 0.115, 0.044)` | `0.020` | Live load label. |

## Right Panel

| Tag / Region | Target BBox 1920 | Normalized BBox | Tolerance | Notes |
|---|---:|---:|---:|---|
| MainMenu.Right.FilterWorldButton | `(1374, 143, 150, 64)` | `(0.716, 0.132, 0.078, 0.059)` | `0.010` | World filter icon button. |
| MainMenu.Right.FilterFriendsButton | `(1542, 143, 150, 64)` | `(0.803, 0.132, 0.078, 0.059)` | `0.010` | Friends filter icon button. |
| MainMenu.Right.FilterStreamersButton | `(1710, 143, 150, 64)` | `(0.891, 0.132, 0.078, 0.059)` | `0.010` | Streamers filter icon button. |
| MainMenu.Right.LeaderboardHeader | `(1472, 252, 272, 32)` | `(0.767, 0.233, 0.142, 0.030)` | `0.020` | Header text. |
| MainMenu.Right.TimeWeeklyButton | `(1370, 312, 232, 52)` | `(0.714, 0.289, 0.121, 0.048)` | `0.010` | Weekly button. |
| MainMenu.Right.TimeAllTimeButton | `(1614, 312, 232, 52)` | `(0.841, 0.289, 0.121, 0.048)` | `0.010` | All-time button. |
| MainMenu.Right.PartySizeDropdown | `(1370, 372, 232, 52)` | `(0.714, 0.344, 0.121, 0.048)` | `0.010` | Party dropdown. |
| MainMenu.Right.DifficultyDropdown | `(1614, 372, 232, 52)` | `(0.841, 0.344, 0.121, 0.048)` | `0.010` | Difficulty dropdown. |
| MainMenu.Right.HighScoreMetricButton | `(1370, 436, 232, 52)` | `(0.714, 0.404, 0.121, 0.048)` | `0.012` | High-score metric. |
| MainMenu.Right.SpeedRunMetricButton | `(1614, 436, 232, 52)` | `(0.841, 0.404, 0.121, 0.048)` | `0.012` | Speed-run metric. |
| MainMenu.Right.RankHeader | `(1382, 531, 47, 29)` | `(0.720, 0.492, 0.024, 0.027)` | `0.018` | Table rank header. |
| MainMenu.Right.NameHeader | `(1484, 531, 292, 29)` | `(0.773, 0.492, 0.152, 0.027)` | `0.018` | Table name header. |
| MainMenu.Right.ScoreHeader | `(1776, 531, 58, 29)` | `(0.925, 0.492, 0.030, 0.027)` | `0.018` | Table score header. |
| MainMenu.Right.LeaderboardRows | `(1370, 568, 476, 438)` | `(0.714, 0.526, 0.248, 0.406)` | `0.016` | Scroll rows body. |
| MainMenu.Right.RankingRowLocal | `(1370, 568, 476, 46)` | `(0.714, 0.526, 0.248, 0.043)` | `0.012` | Local row. |

