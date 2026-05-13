# Diplomas Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Diplomas.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Diplomas V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Diplomas.Root | `(0.031, 0.158, 0.936, 0.749)` | `±0.012` | All Diplomas-owned UI below the shared top bar. |
| Diplomas.MainOuterContainer | `(0.031, 0.260, 0.936, 0.647)` | `±0.012` | Carousel region including arrows, cards, and pagination. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Diplomas.SubTabs | `(0.181, 0.158, 0.611, 0.068)` | `±0.008` | Power Up sub-tabs only; no description band. |
| Diplomas.SubTabs.DiplomasButton | `(0.181, 0.158, 0.297, 0.068)` | `±0.005` | Selected red `DIPLOMAS (PERMANENT)` tab. |
| Diplomas.SubTabs.DiplomasInfoIcon | `(0.418, 0.176, 0.018, 0.033)` | `±0.005` | Red info icon inside selected tab. |
| Diplomas.SubTabs.DrugsButton | `(0.496, 0.158, 0.296, 0.068)` | `±0.005` | Default purple `DRUGS (ONE TIME USE)` tab. |
| Diplomas.SubTabs.DrugsInfoIcon | `(0.719, 0.176, 0.017, 0.033)` | `±0.005` | Purple info icon inside default tab. |

## Carousel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Diplomas.Carousel.LeftNavButton | `(0.032, 0.532, 0.017, 0.046)` | `±0.006` | Left pagination arrow. |
| Diplomas.Carousel.RightNavButton | `(0.950, 0.532, 0.017, 0.046)` | `±0.006` | Right pagination arrow. |
| Diplomas.CardsRow | `(0.072, 0.260, 0.851, 0.610)` | `±0.010` | Four visible diploma cards. |
| Diplomas.Pagination | `(0.455, 0.891, 0.091, 0.021)` | `±0.012` | Pagination dots beneath the card row per spec; reference has very subtle/low-contrast pagination space. |

## Cards

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Diplomas.Card01 | `(0.072, 0.260, 0.205, 0.610)` | `±0.010` | Damage card outer panel. |
| Diplomas.Card01.Artwork | `(0.086, 0.278, 0.178, 0.446)` | `±0.012` | Dropout parchment art. |
| Diplomas.Card01.StatText | `(0.135, 0.740, 0.083, 0.028)` | `±0.008` | `+0 DAMAGE`. |
| Diplomas.Card01.GraduateButton | `(0.081, 0.781, 0.185, 0.072)` | `±0.006` | Red GRADUATE button. |
| Diplomas.Card01.GraduateButton.Label | `(0.096, 0.808, 0.084, 0.030)` | `±0.008` | Button label. |
| Diplomas.Card01.GraduateButton.Cost | `(0.214, 0.807, 0.036, 0.030)` | `±0.008` | Ticket cost. |
| Diplomas.Card02 | `(0.292, 0.261, 0.199, 0.608)` | `±0.010` | Attack Speed card outer panel. |
| Diplomas.Card02.Artwork | `(0.303, 0.279, 0.178, 0.445)` | `±0.012` | Bachelor's parchment art. |
| Diplomas.Card02.StatText | `(0.340, 0.740, 0.108, 0.028)` | `±0.008` | `+1 ATTACK SPEED`. |
| Diplomas.Card02.GraduateButton | `(0.300, 0.781, 0.181, 0.072)` | `±0.006` | Red GRADUATE button. |
| Diplomas.Card02.GraduateButton.Label | `(0.315, 0.808, 0.084, 0.030)` | `±0.008` | Button label. |
| Diplomas.Card02.GraduateButton.Cost | `(0.430, 0.807, 0.037, 0.030)` | `±0.008` | Ticket cost. |
| Diplomas.Card03 | `(0.508, 0.261, 0.201, 0.608)` | `±0.010` | Attack Scale card outer panel. |
| Diplomas.Card03.Artwork | `(0.519, 0.279, 0.178, 0.445)` | `±0.012` | Master's parchment art. |
| Diplomas.Card03.StatText | `(0.550, 0.740, 0.127, 0.028)` | `±0.008` | `+2 ATTACK SCALE`. |
| Diplomas.Card03.GraduateButton | `(0.517, 0.781, 0.181, 0.072)` | `±0.006` | Red GRADUATE button. |
| Diplomas.Card03.GraduateButton.Label | `(0.532, 0.808, 0.083, 0.030)` | `±0.008` | Button label. |
| Diplomas.Card03.GraduateButton.Cost | `(0.646, 0.807, 0.037, 0.030)` | `±0.008` | Ticket cost. |
| Diplomas.Card04 | `(0.722, 0.261, 0.201, 0.608)` | `±0.010` | Accuracy card outer panel. |
| Diplomas.Card04.Artwork | `(0.733, 0.278, 0.178, 0.446)` | `±0.012` | Ph.D. parchment art. |
| Diplomas.Card04.StatText | `(0.778, 0.740, 0.093, 0.028)` | `±0.008` | `+3 ACCURACY`. |
| Diplomas.Card04.GraduateButton | `(0.731, 0.781, 0.182, 0.072)` | `±0.006` | Red GRADUATE button. |
| Diplomas.Card04.GraduateButton.Label | `(0.746, 0.808, 0.083, 0.030)` | `±0.008` | Button label. |
| Diplomas.Card04.GraduateButton.Cost | `(0.862, 0.807, 0.036, 0.030)` | `±0.008` | Ticket cost. |
