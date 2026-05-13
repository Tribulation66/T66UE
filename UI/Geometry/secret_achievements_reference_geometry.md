# Secret Achievements Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Secret.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Secret Achievements V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SecretAchievements.Root | `(0.024, 0.125, 0.951, 0.804)` | `±0.012` | All Secret-owned UI below the shared top bar. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SecretAchievements.SubTabs | `(0.242, 0.125, 0.487, 0.079)` | `±0.008` | Steam/Secret sub-tab row. |
| SecretAchievements.SubTabs.SteamButton | `(0.242, 0.125, 0.228, 0.079)` | `±0.006` | Default STEAM tab. |
| SecretAchievements.SubTabs.SteamInfoIcon | `(0.434, 0.150, 0.019, 0.033)` | `±0.006` | Default info icon. |
| SecretAchievements.SubTabs.SecretButton | `(0.487, 0.125, 0.242, 0.079)` | `±0.006` | Selected red SECRET tab. |
| SecretAchievements.SubTabs.SecretInfoIcon | `(0.691, 0.150, 0.019, 0.033)` | `±0.006` | Selected red info icon. |

## Summary

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SecretAchievements.SummaryPanel | `(0.024, 0.225, 0.951, 0.213)` | `±0.010` | Wide red summary container. |
| SecretAchievements.Summary.SecretLogo | `(0.060, 0.239, 0.114, 0.185)` | `±0.010` | Secret occult mark from reference crop. |
| SecretAchievements.Summary.Header | `(0.217, 0.274, 0.413, 0.053)` | `±0.008` | `SECRET ACHIEVEMENTS`. |
| SecretAchievements.Summary.Count | `(0.607, 0.274, 0.102, 0.053)` | `±0.008` | `0/???` count. |
| SecretAchievements.Summary.ProgressBar | `(0.217, 0.357, 0.719, 0.038)` | `±0.008` | Red progress bar. |

## Achievement List

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| SecretAchievements.ListPanel | `(0.024, 0.463, 0.951, 0.467)` | `±0.010` | Four visible masked rows and dividers. |
| SecretAchievements.Row01 | `(0.035, 0.497, 0.928, 0.082)` | `±0.010` | First masked row. |
| SecretAchievements.Row02 | `(0.035, 0.610, 0.928, 0.082)` | `±0.010` | Second masked row. |
| SecretAchievements.Row03 | `(0.035, 0.725, 0.928, 0.082)` | `±0.010` | Third masked row. |
| SecretAchievements.Row04 | `(0.035, 0.838, 0.928, 0.082)` | `±0.010` | Fourth masked row. |
| SecretAchievements.Row01.Number | `(0.059, 0.512, 0.024, 0.040)` | `±0.008` | `01`. |
| SecretAchievements.Row01.Name | `(0.123, 0.509, 0.147, 0.045)` | `±0.008` | Masked title. |
| SecretAchievements.Row01.Description | `(0.308, 0.509, 0.153, 0.045)` | `±0.010` | Masked description. |
| SecretAchievements.Row01.Progress | `(0.639, 0.509, 0.036, 0.045)` | `±0.008` | Masked progress. |
| SecretAchievements.Row01.RewardValue | `(0.722, 0.509, 0.014, 0.045)` | `±0.008` | `5`. |
| SecretAchievements.Row01.RewardIcon | `(0.745, 0.507, 0.027, 0.040)` | `±0.008` | Ticket icon. |
| SecretAchievements.Row01.ClaimButton | `(0.804, 0.498, 0.081, 0.056)` | `±0.008` | CLAIM button. |
| SecretAchievements.Row01.FavoriteButton | `(0.914, 0.498, 0.029, 0.056)` | `±0.008` | Star favorite toggle. |
| SecretAchievements.Row02.Number | `(0.059, 0.625, 0.024, 0.040)` | `±0.008` | `02`. |
| SecretAchievements.Row02.Name | `(0.123, 0.622, 0.147, 0.045)` | `±0.008` | Masked title. |
| SecretAchievements.Row02.Description | `(0.308, 0.622, 0.153, 0.045)` | `±0.010` | Masked description. |
| SecretAchievements.Row02.Progress | `(0.639, 0.622, 0.036, 0.045)` | `±0.008` | Masked progress. |
| SecretAchievements.Row02.RewardValue | `(0.722, 0.622, 0.014, 0.045)` | `±0.008` | `5`. |
| SecretAchievements.Row02.RewardIcon | `(0.745, 0.620, 0.027, 0.040)` | `±0.008` | Ticket icon. |
| SecretAchievements.Row02.ClaimButton | `(0.804, 0.610, 0.081, 0.056)` | `±0.008` | CLAIM button. |
| SecretAchievements.Row02.FavoriteButton | `(0.914, 0.610, 0.029, 0.056)` | `±0.008` | Star favorite toggle. |
| SecretAchievements.Row03.Number | `(0.059, 0.739, 0.024, 0.040)` | `±0.008` | `03`. |
| SecretAchievements.Row03.Name | `(0.123, 0.736, 0.147, 0.045)` | `±0.008` | Masked title. |
| SecretAchievements.Row03.Description | `(0.308, 0.736, 0.153, 0.045)` | `±0.010` | Masked description. |
| SecretAchievements.Row03.Progress | `(0.639, 0.736, 0.036, 0.045)` | `±0.008` | Masked progress. |
| SecretAchievements.Row03.RewardValue | `(0.722, 0.736, 0.014, 0.045)` | `±0.008` | `5`. |
| SecretAchievements.Row03.RewardIcon | `(0.745, 0.734, 0.027, 0.040)` | `±0.008` | Ticket icon. |
| SecretAchievements.Row03.ClaimButton | `(0.804, 0.725, 0.081, 0.056)` | `±0.008` | CLAIM button. |
| SecretAchievements.Row03.FavoriteButton | `(0.914, 0.725, 0.029, 0.056)` | `±0.008` | Star favorite toggle. |
| SecretAchievements.Row04.Number | `(0.059, 0.853, 0.024, 0.040)` | `±0.008` | `04`. |
| SecretAchievements.Row04.Name | `(0.123, 0.850, 0.147, 0.045)` | `±0.008` | Masked title. |
| SecretAchievements.Row04.Description | `(0.308, 0.850, 0.153, 0.045)` | `±0.010` | Masked description. |
| SecretAchievements.Row04.Progress | `(0.639, 0.850, 0.036, 0.045)` | `±0.008` | Masked progress. |
| SecretAchievements.Row04.RewardValue | `(0.722, 0.850, 0.014, 0.045)` | `±0.008` | `5`. |
| SecretAchievements.Row04.RewardIcon | `(0.745, 0.848, 0.027, 0.040)` | `±0.008` | Ticket icon. |
| SecretAchievements.Row04.ClaimButton | `(0.804, 0.838, 0.081, 0.056)` | `±0.008` | CLAIM button. |
| SecretAchievements.Row04.FavoriteButton | `(0.914, 0.838, 0.029, 0.056)` | `±0.008` | Star favorite toggle. |
