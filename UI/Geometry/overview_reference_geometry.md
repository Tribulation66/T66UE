# Overview Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Overview.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image has the same 16:9 aspect, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Overview V3 reference image at native resolution, followed by a visual sanity overlay at `C:\UE\T66\UI\Geometry\overview_reference_geometry_overlay.png`. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table. Use `±0.005` for tight buttons/icons/table rows, `±0.010` for normal panels, and `±0.012` for full-column/body extents.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.Reference.TopmostOwnedUI | `(0.148, 0.123, 0.690, 0.060)` | `±0.008` | Top of the Overview-owned sub-tab row; top bar is excluded. |
| Overview.Root | `(0.013, 0.123, 0.974, 0.838)` | `±0.012` | All Overview-owned UI below the top bar, including sub-tabs and body panels. |
| Overview.MainBody | `(0.013, 0.201, 0.974, 0.760)` | `±0.012` | Body columns only; excludes top bar and sub-tab row. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.SubTabs | `(0.148, 0.123, 0.690, 0.060)` | `±0.008` | Two account sub-tabs only; no outer container. |
| Overview.SubTabs.OverviewButton | `(0.148, 0.123, 0.320, 0.060)` | `±0.005` | Selected red `OVERVIEW` tab. |
| Overview.SubTabs.OverviewButton.Label | `(0.269, 0.141, 0.068, 0.028)` | `±0.006` | `OVERVIEW` text. |
| Overview.SubTabs.OverviewInfoIcon | `(0.438, 0.140, 0.016, 0.030)` | `±0.005` | Red info icon inside selected tab. |
| Overview.SubTabs.HistoryButton | `(0.498, 0.123, 0.340, 0.060)` | `±0.005` | Default purple `HISTORY` tab. |
| Overview.SubTabs.HistoryButton.Label | `(0.634, 0.141, 0.063, 0.028)` | `±0.006` | `HISTORY` text. |
| Overview.SubTabs.HistoryInfoIcon | `(0.806, 0.140, 0.016, 0.030)` | `±0.005` | Purple info icon inside default tab. |

## Left Column

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.LeftColumn | `(0.013, 0.207, 0.393, 0.754)` | `±0.012` | Player block, Account Status, and Account Progress stack. |
| Overview.PlayerBlock | `(0.013, 0.207, 0.393, 0.221)` | `±0.010` | Purple-bordered profile panel. |
| Overview.PlayerBlock.Avatar | `(0.026, 0.230, 0.102, 0.173)` | `±0.010` | Player/avatar content area. |
| Overview.PlayerBlock.Name | `(0.145, 0.238, 0.106, 0.033)` | `±0.006` | Player name, placeholder `RandomChad` when live name is unavailable. |
| Overview.PlayerBlock.Level | `(0.145, 0.291, 0.077, 0.027)` | `±0.006` | `LEVEL 1/100`, red. |
| Overview.PlayerBlock.ExperienceLabel | `(0.145, 0.346, 0.065, 0.026)` | `±0.006` | `EXPERIENCE`. |
| Overview.PlayerBlock.ExperienceValue | `(0.298, 0.346, 0.095, 0.026)` | `±0.006` | XP value aligned to the right of the player block. |
| Overview.PlayerBlock.ExperienceProgress | `(0.145, 0.379, 0.247, 0.026)` | `±0.006` | Red progress bar track. |
| Overview.PlayerBlock.ExperienceProgress.Fill | `(0.145, 0.379, 0.069, 0.026)` | `±0.008` | Visible red fill length in the reference. |
| Overview.AccountStatusPanel | `(0.013, 0.454, 0.393, 0.182)` | `±0.010` | Purple-bordered status panel. |
| Overview.AccountStatusPanel.Icon | `(0.029, 0.488, 0.053, 0.084)` | `±0.008` | Shield icon. |
| Overview.AccountStatusPanel.Header | `(0.090, 0.483, 0.131, 0.030)` | `±0.006` | `ACCOUNT STATUS`. |
| Overview.AccountStatusPanel.Status | `(0.228, 0.483, 0.135, 0.030)` | `±0.006` | `GOOD STANDING`, green. |
| Overview.AccountStatusPanel.Warning | `(0.090, 0.525, 0.299, 0.080)` | `±0.010` | Locked warning paragraph; no branding strip. |
| Overview.AccountProgressPanel | `(0.013, 0.654, 0.393, 0.307)` | `±0.010` | Purple-bordered progress panel. |
| Overview.AccountProgressPanel.Icon | `(0.029, 0.678, 0.042, 0.061)` | `±0.008` | Bar chart icon. |
| Overview.AccountProgressPanel.Header | `(0.090, 0.683, 0.144, 0.032)` | `±0.006` | `ACCOUNT PROGRESS`. |
| Overview.AccountProgressPanel.Achievements | `(0.090, 0.729, 0.294, 0.029)` | `±0.008` | Full row extent. |
| Overview.AccountProgressPanel.Achievements.Label | `(0.090, 0.729, 0.135, 0.026)` | `±0.006` | `ACHIEVEMENTS UNLOCKED`. |
| Overview.AccountProgressPanel.Achievements.ProgressBar | `(0.231, 0.729, 0.115, 0.025)` | `±0.006` | Row progress bar. |
| Overview.AccountProgressPanel.Achievements.Value | `(0.359, 0.729, 0.026, 0.026)` | `±0.006` | Count value. |
| Overview.AccountProgressPanel.PowerUps | `(0.090, 0.775, 0.294, 0.029)` | `±0.008` | Full row extent. |
| Overview.AccountProgressPanel.PowerUps.Label | `(0.090, 0.775, 0.151, 0.026)` | `±0.006` | `PERMANENT BUFFS UNLOCKED`. |
| Overview.AccountProgressPanel.PowerUps.ProgressBar | `(0.231, 0.775, 0.115, 0.025)` | `±0.006` | Row progress bar. |
| Overview.AccountProgressPanel.PowerUps.Value | `(0.359, 0.775, 0.026, 0.026)` | `±0.006` | Count value. |
| Overview.AccountProgressPanel.Heroes | `(0.090, 0.819, 0.294, 0.029)` | `±0.008` | Full row extent. |
| Overview.AccountProgressPanel.Heroes.Label | `(0.090, 0.819, 0.108, 0.026)` | `±0.006` | `HEROES UNLOCKED`. |
| Overview.AccountProgressPanel.Heroes.ProgressBar | `(0.231, 0.819, 0.115, 0.025)` | `±0.006` | Row progress bar. |
| Overview.AccountProgressPanel.Heroes.Value | `(0.358, 0.819, 0.028, 0.026)` | `±0.006` | Count value. |
| Overview.AccountProgressPanel.Companions | `(0.090, 0.864, 0.294, 0.029)` | `±0.008` | Full row extent. |
| Overview.AccountProgressPanel.Companions.Label | `(0.090, 0.864, 0.139, 0.026)` | `±0.006` | `COMPANIONS UNLOCKED`. |
| Overview.AccountProgressPanel.Companions.ProgressBar | `(0.231, 0.864, 0.115, 0.025)` | `±0.006` | Row progress bar. |
| Overview.AccountProgressPanel.Companions.Value | `(0.359, 0.864, 0.026, 0.026)` | `±0.006` | Count value. |
| Overview.AccountProgressPanel.Challenges | `(0.090, 0.912, 0.294, 0.029)` | `±0.008` | Full row extent. |
| Overview.AccountProgressPanel.Challenges.Label | `(0.090, 0.912, 0.136, 0.026)` | `±0.006` | `CHALLENGES COMPLETED`. |
| Overview.AccountProgressPanel.Challenges.ProgressBar | `(0.231, 0.912, 0.115, 0.025)` | `±0.006` | Row progress bar. |
| Overview.AccountProgressPanel.Challenges.Value | `(0.359, 0.912, 0.026, 0.026)` | `±0.006` | Count value. |

## Right Column

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.RightColumn | `(0.420, 0.201, 0.566, 0.760)` | `±0.012` | Outer purple container around filters and both PB panels. |
| Overview.RightColumn.OuterPanel | `(0.420, 0.201, 0.566, 0.760)` | `±0.012` | Same as right column; included for explicit outer-container assertions. |
| Overview.FilterRow | `(0.441, 0.230, 0.526, 0.056)` | `±0.010` | Red selected filter dropdown row. |
| Overview.FilterRow.PersonalBestDropdown | `(0.441, 0.230, 0.252, 0.056)` | `±0.005` | Forced Selected red dropdown. |
| Overview.FilterRow.PersonalBestDropdown.Label | `(0.453, 0.248, 0.134, 0.028)` | `±0.006` | `PERSONAL BEST`. |
| Overview.FilterRow.PersonalBestDropdown.Arrow | `(0.666, 0.250, 0.012, 0.018)` | `±0.005` | Red dropdown arrow. |
| Overview.FilterRow.SoloDropdown | `(0.713, 0.230, 0.254, 0.056)` | `±0.005` | Forced Selected red dropdown. |
| Overview.FilterRow.SoloDropdown.Label | `(0.725, 0.248, 0.041, 0.028)` | `±0.006` | `SOLO`. |
| Overview.FilterRow.SoloDropdown.Arrow | `(0.947, 0.250, 0.012, 0.018)` | `±0.005` | Red dropdown arrow. |

## Highest Score Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.HighestScorePanel | `(0.440, 0.308, 0.527, 0.278)` | `±0.010` | Score table sub-panel. |
| Overview.HighestScorePanel.Icon | `(0.451, 0.329, 0.029, 0.047)` | `±0.008` | Trophy/laurel icon. |
| Overview.HighestScorePanel.Header | `(0.492, 0.332, 0.145, 0.034)` | `±0.006` | `HIGHEST SCORE`. |
| Overview.HighestScorePanel.Table | `(0.440, 0.373, 0.527, 0.214)` | `±0.010` | Table area below header. |
| Overview.HighestScorePanel.TableHeader | `(0.440, 0.373, 0.527, 0.030)` | `±0.006` | Column header row. |
| Overview.HighestScorePanel.Column.Difficulty | `(0.440, 0.373, 0.139, 0.214)` | `±0.006` | Difficulty column. |
| Overview.HighestScorePanel.Column.Hero | `(0.579, 0.373, 0.096, 0.214)` | `±0.006` | Hero column. |
| Overview.HighestScorePanel.Column.Date | `(0.675, 0.373, 0.097, 0.214)` | `±0.006` | Date column. |
| Overview.HighestScorePanel.Column.GlobalRank | `(0.772, 0.373, 0.115, 0.214)` | `±0.006` | Global rank column. |
| Overview.HighestScorePanel.Column.Score | `(0.886, 0.373, 0.081, 0.214)` | `±0.006` | Score column. |
| Overview.HighestScorePanel.Row01 | `(0.440, 0.403, 0.527, 0.036)` | `±0.006` | Easy row. |
| Overview.HighestScorePanel.Row02 | `(0.440, 0.440, 0.527, 0.036)` | `±0.006` | Medium row. |
| Overview.HighestScorePanel.Row03 | `(0.440, 0.476, 0.527, 0.036)` | `±0.006` | Hard row. |
| Overview.HighestScorePanel.Row04 | `(0.440, 0.513, 0.527, 0.036)` | `±0.006` | Very Hard row. |
| Overview.HighestScorePanel.Row05 | `(0.440, 0.550, 0.527, 0.037)` | `±0.006` | Impossible row. |

## Best Speed Run Panel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Overview.BestSpeedRunPanel | `(0.440, 0.616, 0.527, 0.296)` | `±0.010` | Speedrun table sub-panel. |
| Overview.BestSpeedRunPanel.Icon | `(0.451, 0.637, 0.029, 0.052)` | `±0.008` | Stopwatch icon. |
| Overview.BestSpeedRunPanel.Header | `(0.492, 0.642, 0.141, 0.034)` | `±0.006` | `BEST SPEED RUN`. |
| Overview.BestSpeedRunPanel.Table | `(0.440, 0.681, 0.527, 0.231)` | `±0.010` | Table area below header. |
| Overview.BestSpeedRunPanel.TableHeader | `(0.440, 0.681, 0.527, 0.033)` | `±0.006` | Column header row. |
| Overview.BestSpeedRunPanel.Column.Difficulty | `(0.440, 0.681, 0.139, 0.231)` | `±0.006` | Difficulty column. |
| Overview.BestSpeedRunPanel.Column.Hero | `(0.579, 0.681, 0.096, 0.231)` | `±0.006` | Hero column. |
| Overview.BestSpeedRunPanel.Column.Date | `(0.675, 0.681, 0.097, 0.231)` | `±0.006` | Date column. |
| Overview.BestSpeedRunPanel.Column.GlobalRank | `(0.772, 0.681, 0.115, 0.231)` | `±0.006` | Global rank column. |
| Overview.BestSpeedRunPanel.Column.Time | `(0.886, 0.681, 0.081, 0.231)` | `±0.006` | Time column. |
| Overview.BestSpeedRunPanel.Row01 | `(0.440, 0.714, 0.527, 0.037)` | `±0.006` | Easy row. |
| Overview.BestSpeedRunPanel.Row02 | `(0.440, 0.751, 0.527, 0.037)` | `±0.006` | Medium row. |
| Overview.BestSpeedRunPanel.Row03 | `(0.440, 0.789, 0.527, 0.037)` | `±0.006` | Hard row. |
| Overview.BestSpeedRunPanel.Row04 | `(0.440, 0.832, 0.527, 0.037)` | `±0.006` | Very Hard row. |
| Overview.BestSpeedRunPanel.Row05 | `(0.440, 0.874, 0.527, 0.038)` | `±0.006` | Impossible row. |
