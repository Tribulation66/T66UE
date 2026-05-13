# Steam Achievements Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Steam.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Steam Achievements V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SteamAchievements.Root | `(0.025, 0.181, 0.949, 0.776)` | `±0.012` | All Steam-owned UI below the shared top bar. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SteamAchievements.SubTabs | `(0.259, 0.181, 0.466, 0.080)` | `±0.008` | Steam/Secret sub-tab row. |
| SteamAchievements.SubTabs.SteamButton | `(0.259, 0.181, 0.221, 0.080)` | `±0.006` | Selected red STEAM tab. |
| SteamAchievements.SubTabs.SteamInfoIcon | `(0.444, 0.204, 0.020, 0.037)` | `±0.006` | Red info icon. |
| SteamAchievements.SubTabs.SecretButton | `(0.496, 0.181, 0.229, 0.080)` | `±0.006` | Default purple SECRET tab. |
| SteamAchievements.SubTabs.SecretInfoIcon | `(0.683, 0.204, 0.020, 0.037)` | `±0.006` | Purple info icon. |

## Summary

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SteamAchievements.SummaryPanel | `(0.025, 0.289, 0.949, 0.192)` | `±0.010` | Wide red summary container. |
| SteamAchievements.Summary.SteamLogo | `(0.073, 0.319, 0.074, 0.134)` | `±0.010` | Steam brand/logo placeholder. |
| SteamAchievements.Summary.Header | `(0.191, 0.335, 0.284, 0.052)` | `±0.008` | `STEAM ACHIEVEMENTS`. |
| SteamAchievements.Summary.Count | `(0.499, 0.335, 0.068, 0.052)` | `±0.008` | `0/100` count. |
| SteamAchievements.Summary.ProgressBar | `(0.191, 0.419, 0.746, 0.031)` | `±0.008` | Red progress bar. |

## Achievement List

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SteamAchievements.ListPanel | `(0.025, 0.507, 0.949, 0.450)` | `±0.010` | Four visible rows and dividers. |
| SteamAchievements.Row01 | `(0.038, 0.535, 0.925, 0.086)` | `±0.010` | Collector 1. |
| SteamAchievements.Row02 | `(0.038, 0.643, 0.925, 0.086)` | `±0.010` | Field Notes 1. |
| SteamAchievements.Row03 | `(0.038, 0.752, 0.925, 0.086)` | `±0.010` | Token Rank 1. |
| SteamAchievements.Row04 | `(0.038, 0.861, 0.925, 0.086)` | `±0.010` | First Win 1; reference-observed fourth row. |
| SteamAchievements.Row01.Number | `(0.059, 0.556, 0.025, 0.038)` | `±0.008` | `01`. |
| SteamAchievements.Row01.Name | `(0.120, 0.555, 0.091, 0.040)` | `±0.008` | `Collector 1`. |
| SteamAchievements.Row01.Description | `(0.239, 0.556, 0.184, 0.038)` | `±0.010` | `[Discover 1 items]`. |
| SteamAchievements.Row01.Progress | `(0.647, 0.556, 0.037, 0.038)` | `±0.008` | `0/1`. |
| SteamAchievements.Row01.RewardValue | `(0.726, 0.556, 0.016, 0.038)` | `±0.008` | `5`. |
| SteamAchievements.Row01.RewardIcon | `(0.747, 0.552, 0.026, 0.041)` | `±0.008` | Ticket icon. |
| SteamAchievements.Row01.ClaimButton | `(0.807, 0.541, 0.081, 0.058)` | `±0.008` | CLAIM button. |
| SteamAchievements.Row01.FavoriteButton | `(0.913, 0.541, 0.033, 0.058)` | `±0.008` | Star favorite toggle. |
| SteamAchievements.Row02.Number | `(0.059, 0.664, 0.025, 0.038)` | `±0.008` | `02`. |
| SteamAchievements.Row02.Name | `(0.120, 0.663, 0.127, 0.040)` | `±0.008` | `Field Notes 1`. |
| SteamAchievements.Row02.Description | `(0.249, 0.664, 0.205, 0.038)` | `±0.010` | `[Discover 1 enemies]`. |
| SteamAchievements.Row02.Progress | `(0.647, 0.664, 0.037, 0.038)` | `±0.008` | `0/1`. |
| SteamAchievements.Row02.RewardValue | `(0.726, 0.664, 0.016, 0.038)` | `±0.008` | `5`. |
| SteamAchievements.Row02.RewardIcon | `(0.747, 0.660, 0.026, 0.041)` | `±0.008` | Ticket icon. |
| SteamAchievements.Row02.ClaimButton | `(0.807, 0.649, 0.081, 0.058)` | `±0.008` | CLAIM button. |
| SteamAchievements.Row02.FavoriteButton | `(0.913, 0.649, 0.033, 0.058)` | `±0.008` | Star favorite toggle. |
| SteamAchievements.Row03.Number | `(0.059, 0.773, 0.025, 0.038)` | `±0.008` | `03`. |
| SteamAchievements.Row03.Name | `(0.120, 0.772, 0.116, 0.040)` | `±0.008` | `Token Rank 1`. |
| SteamAchievements.Row03.Description | `(0.255, 0.773, 0.261, 0.038)` | `±0.010` | `[Unlock Gambler's Token level 1]`. |
| SteamAchievements.Row03.Progress | `(0.647, 0.773, 0.037, 0.038)` | `±0.008` | `0/1`. |
| SteamAchievements.Row03.RewardValue | `(0.726, 0.773, 0.016, 0.038)` | `±0.008` | `5`. |
| SteamAchievements.Row03.RewardIcon | `(0.747, 0.769, 0.026, 0.041)` | `±0.008` | Ticket icon. |
| SteamAchievements.Row03.ClaimButton | `(0.807, 0.758, 0.081, 0.058)` | `±0.008` | CLAIM button. |
| SteamAchievements.Row03.FavoriteButton | `(0.913, 0.758, 0.033, 0.058)` | `±0.008` | Star favorite toggle. |
| SteamAchievements.Row04.Number | `(0.059, 0.882, 0.025, 0.038)` | `±0.008` | `04`. |
| SteamAchievements.Row04.Name | `(0.120, 0.881, 0.104, 0.040)` | `±0.008` | `First Win 1`. |
| SteamAchievements.Row04.Description | `(0.239, 0.882, 0.230, 0.038)` | `±0.010` | `[Win 1 match in any mode]`. |
| SteamAchievements.Row04.Progress | `(0.647, 0.882, 0.037, 0.038)` | `±0.008` | `0/1`. |
| SteamAchievements.Row04.RewardValue | `(0.726, 0.882, 0.016, 0.038)` | `±0.008` | `5`. |
| SteamAchievements.Row04.RewardIcon | `(0.747, 0.878, 0.026, 0.041)` | `±0.008` | Ticket icon. |
| SteamAchievements.Row04.ClaimButton | `(0.807, 0.867, 0.081, 0.058)` | `±0.008` | CLAIM button. |
| SteamAchievements.Row04.FavoriteButton | `(0.913, 0.867, 0.033, 0.058)` | `±0.008` | Star favorite toggle. |
