# Drugs Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Drugs.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image is 16:9, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the Drugs V3 reference image at native resolution. Top bar chrome is intentionally excluded because `UT66FrontendTopBarWidget` has its own Stage 2 geometry table.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Drugs.Root | `(0.041, 0.135, 0.909, 0.823)` | `±0.012` | All Drugs-owned UI below the shared top bar. |
| Drugs.MainOuterContainer | `(0.041, 0.236, 0.909, 0.722)` | `±0.012` | Two category rows and eight cards. |

## Sub Tabs

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Drugs.SubTabs | `(0.188, 0.135, 0.590, 0.071)` | `±0.008` | Power Up sub-tabs only; no description band. |
| Drugs.SubTabs.DiplomasButton | `(0.188, 0.135, 0.285, 0.071)` | `±0.005` | Default purple `DIPLOMAS (PERMANENT)` tab. |
| Drugs.SubTabs.DiplomasInfoIcon | `(0.431, 0.154, 0.018, 0.033)` | `±0.005` | Purple info icon. |
| Drugs.SubTabs.DrugsButton | `(0.493, 0.135, 0.284, 0.071)` | `±0.005` | Selected red `DRUGS (ONE RUN USE)` tab. |
| Drugs.SubTabs.DrugsInfoIcon | `(0.739, 0.154, 0.017, 0.033)` | `±0.005` | Red info icon. |

## Category Rows

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Drugs.Category.DamagePanel | `(0.041, 0.236, 0.108, 0.346)` | `±0.010` | Left category label panel. |
| Drugs.Category.DamageIcon | `(0.075, 0.291, 0.038, 0.065)` | `±0.008` | Target icon. |
| Drugs.Category.DamageLabel | `(0.054, 0.390, 0.080, 0.085)` | `±0.008` | `DAMAGE DRUGS`. |
| Drugs.Category.AttackSpeedPanel | `(0.041, 0.613, 0.108, 0.343)` | `±0.010` | Left category label panel. |
| Drugs.Category.AttackSpeedIcon | `(0.075, 0.661, 0.040, 0.065)` | `±0.008` | Speedometer icon. |
| Drugs.Category.AttackSpeedLabel | `(0.054, 0.748, 0.083, 0.131)` | `±0.008` | `ATTACK SPEED DRUGS`. |
| Drugs.Row.Damage | `(0.170, 0.236, 0.781, 0.346)` | `±0.010` | Four damage drug cards. |
| Drugs.Row.AttackSpeed | `(0.170, 0.613, 0.781, 0.343)` | `±0.010` | Four attack speed drug cards. |

## Cards

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| Drugs.Card01 | `(0.170, 0.236, 0.175, 0.346)` | `±0.010` | OXYMETHOLONE. |
| Drugs.Card01.Name | `(0.213, 0.261, 0.089, 0.030)` | `±0.008` | Name. |
| Drugs.Card01.Artwork | `(0.238, 0.299, 0.048, 0.159)` | `±0.012` | Drug art. |
| Drugs.Card01.Effect | `(0.202, 0.480, 0.110, 0.024)` | `±0.008` | Effect. |
| Drugs.Card01.BuyButton | `(0.178, 0.513, 0.160, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card01.BuyButton.Cost | `(0.279, 0.529, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card02 | `(0.365, 0.236, 0.181, 0.346)` | `±0.010` | METHANDROSTENOLONE. |
| Drugs.Card02.Name | `(0.388, 0.261, 0.138, 0.030)` | `±0.008` | Name. |
| Drugs.Card02.Artwork | `(0.419, 0.313, 0.065, 0.134)` | `±0.012` | Drug art. |
| Drugs.Card02.Effect | `(0.392, 0.480, 0.127, 0.024)` | `±0.008` | Effect. |
| Drugs.Card02.BuyButton | `(0.374, 0.513, 0.165, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card02.BuyButton.Cost | `(0.472, 0.529, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card03 | `(0.566, 0.236, 0.182, 0.346)` | `±0.010` | FLUOXYMESTERONE. |
| Drugs.Card03.Name | `(0.601, 0.261, 0.114, 0.030)` | `±0.008` | Name. |
| Drugs.Card03.Artwork | `(0.639, 0.302, 0.056, 0.152)` | `±0.012` | Drug art. |
| Drugs.Card03.Effect | `(0.596, 0.480, 0.122, 0.024)` | `±0.008` | Effect. |
| Drugs.Card03.BuyButton | `(0.575, 0.513, 0.165, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card03.BuyButton.Cost | `(0.675, 0.529, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card04 | `(0.769, 0.236, 0.181, 0.346)` | `±0.010` | NANDROLONE DECANOATE. |
| Drugs.Card04.Name | `(0.783, 0.261, 0.148, 0.030)` | `±0.008` | Name. |
| Drugs.Card04.Artwork | `(0.839, 0.296, 0.049, 0.163)` | `±0.012` | Drug art. |
| Drugs.Card04.Effect | `(0.803, 0.480, 0.118, 0.024)` | `±0.008` | Effect. |
| Drugs.Card04.BuyButton | `(0.777, 0.513, 0.164, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card04.BuyButton.Cost | `(0.876, 0.529, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card05 | `(0.170, 0.613, 0.175, 0.343)` | `±0.010` | CAFFEINE CITRATE. |
| Drugs.Card05.Name | `(0.205, 0.640, 0.105, 0.030)` | `±0.008` | Name. |
| Drugs.Card05.Artwork | `(0.239, 0.675, 0.050, 0.160)` | `±0.012` | Drug art. |
| Drugs.Card05.Effect | `(0.204, 0.859, 0.108, 0.024)` | `±0.008` | Effect. |
| Drugs.Card05.BuyButton | `(0.178, 0.891, 0.160, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card05.BuyButton.Cost | `(0.279, 0.906, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card06 | `(0.365, 0.613, 0.181, 0.343)` | `±0.010` | MODAFINIL. |
| Drugs.Card06.Name | `(0.423, 0.640, 0.064, 0.030)` | `±0.008` | Name. |
| Drugs.Card06.Artwork | `(0.421, 0.682, 0.066, 0.137)` | `±0.012` | Drug art. |
| Drugs.Card06.Effect | `(0.392, 0.859, 0.127, 0.024)` | `±0.008` | Effect. |
| Drugs.Card06.BuyButton | `(0.374, 0.891, 0.165, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card06.BuyButton.Cost | `(0.472, 0.906, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card07 | `(0.566, 0.613, 0.182, 0.343)` | `±0.010` | EPHEDRINE HCL. |
| Drugs.Card07.Name | `(0.609, 0.640, 0.106, 0.030)` | `±0.008` | Name. |
| Drugs.Card07.Artwork | `(0.636, 0.675, 0.063, 0.160)` | `±0.012` | Drug art. |
| Drugs.Card07.Effect | `(0.597, 0.859, 0.121, 0.024)` | `±0.008` | Effect. |
| Drugs.Card07.BuyButton | `(0.575, 0.891, 0.165, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card07.BuyButton.Cost | `(0.675, 0.906, 0.039, 0.029)` | `±0.008` | Cost. |
| Drugs.Card08 | `(0.769, 0.613, 0.181, 0.343)` | `±0.010` | SALBUTAMOL SULFATE. |
| Drugs.Card08.Name | `(0.792, 0.640, 0.129, 0.030)` | `±0.008` | Name. |
| Drugs.Card08.Artwork | `(0.822, 0.678, 0.078, 0.148)` | `±0.012` | Drug art. |
| Drugs.Card08.Effect | `(0.804, 0.859, 0.117, 0.024)` | `±0.008` | Effect. |
| Drugs.Card08.BuyButton | `(0.777, 0.891, 0.164, 0.054)` | `±0.006` | Buy button. |
| Drugs.Card08.BuyButton.Cost | `(0.876, 0.906, 0.039, 0.029)` | `±0.008` | Cost. |
