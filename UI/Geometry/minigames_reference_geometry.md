# Minigames Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Minigames.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Minigames V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Minigames.Root | `(0.031, 0.141, 0.936, 0.790)` | `±0.012` | All Minigames-owned UI below the shared top bar. |
| Minigames.Title | `(0.381, 0.141, 0.237, 0.071)` | `±0.012` | Floating `MINIGAMES` title. |
| Minigames.DescriptionBand | `(0.277, 0.253, 0.447, 0.032)` | `±0.012` | Locked description text line; reference has no visible panel chrome. |
| Minigames.MainOuterContainer | `(0.031, 0.307, 0.936, 0.624)` | `±0.012` | Card carousel body region. |
| Minigames.CardsRow | `(0.033, 0.307, 0.934, 0.624)` | `±0.012` | Four visible minigame cards. |

## Carousel

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Minigames.Carousel.LeftNavButton | `(0.010, 0.570, 0.018, 0.045)` | `±0.010` | Carousel paging affordance inferred from spec; visually subtle in reference. |
| Minigames.Carousel.RightNavButton | `(0.972, 0.570, 0.018, 0.045)` | `±0.010` | Carousel paging affordance inferred from spec; visually subtle in reference. |
| Minigames.Pagination | `(0.471, 0.948, 0.058, 0.018)` | `±0.014` | Bottom pagination area inferred from spec; reference leaves this area dark. |
| Minigames.Pagination.Dot01 | `(0.471, 0.948, 0.020, 0.018)` | `±0.012` | Page 1 dot, selected. |
| Minigames.Pagination.Dot02 | `(0.509, 0.948, 0.020, 0.018)` | `±0.012` | Page 2 dot, default. |

## Cards

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Minigames.Card01 | `(0.033, 0.307, 0.225, 0.624)` | `±0.010` | CHADPOCALYPSE MINI. |
| Minigames.Card01.Artwork | `(0.040, 0.321, 0.211, 0.304)` | `±0.012` | Mini gameplay screenshot. |
| Minigames.Card01.Title | `(0.068, 0.662, 0.155, 0.043)` | `±0.010` | Card title. |
| Minigames.Card01.Description | `(0.053, 0.713, 0.185, 0.094)` | `±0.012` | Card description. |
| Minigames.Card01.PlayButton | `(0.043, 0.839, 0.202, 0.069)` | `±0.008` | Selected red PLAY button. |
| Minigames.Card02 | `(0.271, 0.307, 0.224, 0.624)` | `±0.010` | CHADPOCALYPSE TOWER DEFENSE. |
| Minigames.Card02.Artwork | `(0.278, 0.321, 0.210, 0.304)` | `±0.012` | Tower defense gameplay screenshot. |
| Minigames.Card02.Title | `(0.288, 0.662, 0.190, 0.043)` | `±0.010` | Card title. |
| Minigames.Card02.Description | `(0.291, 0.713, 0.184, 0.094)` | `±0.012` | Card description. |
| Minigames.Card02.PlayButton | `(0.281, 0.839, 0.202, 0.069)` | `±0.008` | Selected red PLAY button. |
| Minigames.Card03 | `(0.506, 0.307, 0.224, 0.624)` | `±0.010` | CHADPOCALYPSE DECKBUILDER. |
| Minigames.Card03.Artwork | `(0.513, 0.321, 0.210, 0.304)` | `±0.012` | Deckbuilder gameplay screenshot. |
| Minigames.Card03.Title | `(0.522, 0.662, 0.190, 0.043)` | `±0.010` | Card title. |
| Minigames.Card03.Description | `(0.526, 0.713, 0.184, 0.094)` | `±0.012` | Card description. |
| Minigames.Card03.PlayButton | `(0.516, 0.839, 0.202, 0.069)` | `±0.008` | Selected red PLAY button. |
| Minigames.Card04 | `(0.742, 0.307, 0.225, 0.624)` | `±0.010` | CHADPOCALYPSE IDLE. |
| Minigames.Card04.Artwork | `(0.749, 0.321, 0.211, 0.304)` | `±0.012` | Idle gameplay screenshot. |
| Minigames.Card04.Title | `(0.774, 0.662, 0.160, 0.043)` | `±0.010` | Card title. |
| Minigames.Card04.Description | `(0.762, 0.713, 0.185, 0.094)` | `±0.012` | Card description. |
| Minigames.Card04.PlayButton | `(0.752, 0.839, 0.202, 0.069)` | `±0.008` | Selected red PLAY button. |
