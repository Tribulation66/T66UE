# History Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\History.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image has the same 16:9 aspect, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the History V3 reference image at native resolution, followed by a visual sanity overlay at `C:\UE\T66\UI\Geometry\history_reference_geometry_overlay.png`. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table. Use `±0.005` for tight buttons/icons/table controls, `±0.010` for normal panels, and `±0.012` for full extents.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| History.Reference.TopmostOwnedUI | `(0.153, 0.165, 0.691, 0.084)` | `±0.008` | Top of the History-owned sub-tab row; top bar is excluded. |
| History.Root | `(0.011, 0.165, 0.977, 0.812)` | `±0.012` | All History-owned UI below the top bar. |
| History.MainBody | `(0.011, 0.271, 0.977, 0.706)` | `±0.012` | Filter container plus history table panel. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| History.SubTabs | `(0.153, 0.165, 0.691, 0.084)` | `±0.008` | Two account sub-tabs only; no outer container. |
| History.SubTabs.OverviewButton | `(0.153, 0.165, 0.340, 0.084)` | `±0.005` | Default purple `OVERVIEW` tab. |
| History.SubTabs.OverviewInfoIcon | `(0.457, 0.193, 0.018, 0.033)` | `±0.005` | Purple info icon inside default tab. |
| History.SubTabs.HistoryButton | `(0.506, 0.165, 0.338, 0.084)` | `±0.005` | Selected red `HISTORY` tab. |
| History.SubTabs.HistoryInfoIcon | `(0.808, 0.193, 0.018, 0.033)` | `±0.005` | Red info icon inside selected tab. |

## Filters

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| History.FilterPanel | `(0.011, 0.271, 0.977, 0.168)` | `±0.010` | Outer purple filter container. |
| History.FilterPanel.HeroLabel | `(0.031, 0.303, 0.056, 0.030)` | `±0.006` | `HERO`. |
| History.FilterPanel.HeroDropdown | `(0.031, 0.343, 0.200, 0.072)` | `±0.005` | Forced Selected red dropdown. |
| History.FilterPanel.DifficultyLabel | `(0.251, 0.303, 0.107, 0.030)` | `±0.006` | `DIFFICULTY`. |
| History.FilterPanel.DifficultyDropdown | `(0.251, 0.343, 0.193, 0.072)` | `±0.005` | Forced Selected red dropdown. |
| History.FilterPanel.PartySizeLabel | `(0.467, 0.303, 0.099, 0.030)` | `±0.006` | `PARTY SIZE`. |
| History.FilterPanel.PartySizeDropdown | `(0.467, 0.343, 0.184, 0.072)` | `±0.005` | Forced Selected red dropdown. |
| History.FilterPanel.StatusLabel | `(0.672, 0.303, 0.070, 0.030)` | `±0.006` | `STATUS`. |
| History.FilterPanel.StatusDropdown | `(0.672, 0.343, 0.178, 0.072)` | `±0.005` | Forced Selected red dropdown. |
| History.FilterPanel.DailyDescentLabel | `(0.876, 0.303, 0.097, 0.030)` | `±0.006` | `DAILY DESCENT`. |
| History.FilterPanel.DailyDescentCheckbox | `(0.882, 0.356, 0.024, 0.044)` | `±0.006` | Default purple checkbox. |

## History Table

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| History.RunHistoryPanel | `(0.011, 0.463, 0.977, 0.514)` | `±0.010` | Outer run-history table container. |
| History.TableHeader | `(0.027, 0.503, 0.945, 0.057)` | `±0.008` | Header row including sortable controls and rank selector. |
| History.TableDivider | `(0.027, 0.559, 0.945, 0.004)` | `±0.006` | Purple divider under the header row. |
| History.TableHeader.HeroPlayedButton | `(0.033, 0.504, 0.110, 0.037)` | `±0.006` | Sortable `HERO PLAYED` header plus arrows. |
| History.TableHeader.DateButton | `(0.245, 0.504, 0.072, 0.037)` | `±0.006` | Sortable `DATE` header plus arrows. |
| History.TableHeader.StatusButton | `(0.407, 0.504, 0.090, 0.037)` | `±0.006` | Sortable `STATUS` header plus arrows. |
| History.TableHeader.ScoreButton | `(0.566, 0.504, 0.085, 0.037)` | `±0.006` | Sortable `SCORE` header plus arrows. |
| History.TableHeader.DurationButton | `(0.715, 0.504, 0.112, 0.037)` | `±0.006` | Sortable `DURATION` header plus arrows. |
| History.TableHeader.RankDropdown | `(0.876, 0.504, 0.080, 0.037)` | `±0.006` | Rank selector dropdown. |
| History.EmptyState | `(0.036, 0.591, 0.220, 0.033)` | `±0.010` | `No runs have been recorded yet.` |
