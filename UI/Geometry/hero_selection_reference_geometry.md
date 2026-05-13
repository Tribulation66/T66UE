# Hero Selection Reference Geometry

Reference image: `C:\UE\T66\UI\Screen References\Hero Selection.png`

Native reference resolution: `1672x941`

Normalization basis: values below are `(x, y, w, h)` normalized to the same 1920x1080 basis used by `T66.UI.DumpScreen`; because the source image has the same 16:9 aspect, this is equivalent to `(x/1672, y/941, w/1672, h/941)`.

Measurement method: visual inspection of the reference image at native resolution, followed by a visual sanity overlay at `C:\UE\T66\UI\Geometry\hero_selection_reference_geometry_overlay.png`. Use `±0.005` for tight fixed controls, `±0.010` for normal panels/rows, and `±0.015` for artwork/content bounds unless a row notes otherwise. The column and bottom-row extents include intentional breathing room from the overlay sanity pass rather than only tight content pixels.

## Overall Extents

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.Reference.TopmostUI | `(0.010, 0.016, 0.981, 0.000)` | `±0.005` | Top edge of carousel/right-column outer frame; back button begins at `y=0.018`. |
| HeroSelection.Reference.BottomRowExtent | `(0.009, 0.755, 0.982, 0.202)` | `±0.012` | Corrected bottom row top after overlay sanity pass; lower edge is `y≈0.957`, leaving a bottom band of about `0.043`. |
| HeroSelection.TopRow | `(0.010, 0.016, 0.655, 0.070)` | `±0.010` | Back button plus hero carousel only; ARTHUR/LAB/subtitle are part of the right-column header. |

## Top Row

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.TopRow.BackButton | `(0.010, 0.018, 0.078, 0.050)` | `±0.005` | Red selected back button. |
| HeroSelection.TopRow.HeroCarousel | `(0.307, 0.016, 0.358, 0.068)` | `±0.010` | Left arrow, seven portraits, right arrow. |
| HeroSelection.TopRow.HeroCarousel.LeftArrow | `(0.307, 0.016, 0.024, 0.065)` | `±0.005` | Red selected arrow tile. |
| HeroSelection.TopRow.HeroCarousel.Portrait01 | `(0.341, 0.017, 0.035, 0.067)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.Portrait02 | `(0.384, 0.016, 0.035, 0.067)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.Portrait03 | `(0.427, 0.017, 0.035, 0.067)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.Portrait04 | `(0.470, 0.017, 0.035, 0.067)` | `±0.005` | Selected/current hero portrait. |
| HeroSelection.TopRow.HeroCarousel.Portrait05 | `(0.513, 0.017, 0.035, 0.067)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.Portrait06 | `(0.556, 0.018, 0.035, 0.066)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.Portrait07 | `(0.598, 0.018, 0.035, 0.066)` | `±0.005` | Hero portrait slot. |
| HeroSelection.TopRow.HeroCarousel.RightArrow | `(0.642, 0.016, 0.023, 0.065)` | `±0.005` | Red selected arrow tile. |

## Left Column

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.LeftColumn | `(0.009, 0.085, 0.278, 0.658)` | `±0.012` | From top of Skins panel to bottom of Drugs panel, expanded to preserve vertical breathing room. |
| HeroSelection.LeftColumn.SkinsPanel | `(0.009, 0.085, 0.278, 0.531)` | `±0.012` | Purple outer panel, expanded after overlay sanity pass. |
| HeroSelection.LeftColumn.SkinsPanel.Header | `(0.021, 0.101, 0.057, 0.027)` | `±0.006` | `SKINS`. |
| HeroSelection.LeftColumn.SkinsPanel.TicketBadge | `(0.224, 0.102, 0.025, 0.024)` | `±0.006` | Yellow ticket icon. |
| HeroSelection.LeftColumn.SkinsPanel.TicketValue | `(0.263, 0.103, 0.019, 0.026)` | `±0.006` | `10`; layout box includes right-side breathing room next to the ticket icon. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default | `(0.019, 0.140, 0.258, 0.104)` | `±0.008` | Selected red row. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Portrait | `(0.019, 0.140, 0.066, 0.104)` | `±0.006` | Thumbnail fills row height. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Name | `(0.097, 0.162, 0.096, 0.032)` | `±0.006` | `Default`; row text lane includes breathing room, not tight glyph pixels. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.EquippedBadge | `(0.205, 0.190, 0.063, 0.040)` | `±0.006` | `EQUIPPED`. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer | `(0.019, 0.256, 0.258, 0.103)` | `±0.008` | Default purple row. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Portrait | `(0.019, 0.257, 0.066, 0.102)` | `±0.006` | Thumbnail fills row height. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Name | `(0.097, 0.278, 0.096, 0.032)` | `±0.006` | `Beachgoer`; row text lane includes breathing room, not tight glyph pixels. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.PreviewButton | `(0.096, 0.308, 0.070, 0.039)` | `±0.006` | Preview button under name. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Cost | `(0.205, 0.308, 0.063, 0.039)` | `±0.006` | `50` cost button. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Crusader | `(0.019, 0.372, 0.258, 0.103)` | `±0.008` | Default purple row. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Crusader.Portrait | `(0.019, 0.372, 0.066, 0.103)` | `±0.006` | Thumbnail fills row height. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Crusader.Name | `(0.097, 0.394, 0.096, 0.032)` | `±0.006` | `Crusader`; row text lane includes breathing room, not tight glyph pixels. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.Crusader.Cost | `(0.205, 0.424, 0.063, 0.039)` | `±0.006` | `100` cost button. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.GoldenPaladin | `(0.019, 0.488, 0.258, 0.102)` | `±0.008` | Default purple row. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.GoldenPaladin.Portrait | `(0.019, 0.488, 0.066, 0.102)` | `±0.006` | Thumbnail fills row height. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.GoldenPaladin.Name | `(0.096, 0.507, 0.093, 0.031)` | `±0.006` | `Golden Paladin`. |
| HeroSelection.LeftColumn.SkinsPanel.SkinRow.GoldenPaladin.Cost | `(0.205, 0.538, 0.063, 0.039)` | `±0.006` | `150` cost button. |
| HeroSelection.LeftColumn.DrugsPanel | `(0.009, 0.632, 0.278, 0.111)` | `±0.012` | Six controls are one horizontal row; panel moved down after column expansion. |
| HeroSelection.LeftColumn.DrugsPanel.Header | `(0.022, 0.644, 0.063, 0.030)` | `±0.006` | `DRUGS`; corrected after overlay sanity pass. |
| HeroSelection.LeftColumn.DrugsPanel.EquipSlot01 | `(0.020, 0.676, 0.028, 0.045)` | `±0.005` | Empty plus slot. |
| HeroSelection.LeftColumn.DrugsPanel.EquipSlot02 | `(0.058, 0.676, 0.029, 0.045)` | `±0.005` | Empty plus slot. |
| HeroSelection.LeftColumn.DrugsPanel.EquipSlot03 | `(0.097, 0.676, 0.028, 0.045)` | `±0.005` | Empty plus slot. |
| HeroSelection.LeftColumn.DrugsPanel.EquipSlot04 | `(0.134, 0.676, 0.028, 0.045)` | `±0.005` | Empty plus slot. |
| HeroSelection.LeftColumn.DrugsPanel.BuyButton | `(0.180, 0.675, 0.040, 0.046)` | `±0.005` | Same row as slots. |
| HeroSelection.LeftColumn.DrugsPanel.ClearButton | `(0.228, 0.675, 0.048, 0.046)` | `±0.005` | Same row as slots. |

## Middle Column

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.MiddleColumn | `(0.300, 0.094, 0.371, 0.630)` | `±0.015` | Center preview frame, expanded after overlay sanity pass. |
| HeroSelection.MiddleColumn.CharacterPreviewPanel | `(0.300, 0.094, 0.371, 0.630)` | `±0.015` | Transparent frame/window, expanded after overlay sanity pass. |
| HeroSelection.MiddleColumn.CharacterRender | `(0.305, 0.104, 0.360, 0.611)` | `±0.020` | Dump-bound render slot. The visible 3D character silhouette is reviewed visually because Slate cannot dump mesh bounds. |

## Right Column

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.RightColumn | `(0.683, 0.016, 0.308, 0.715)` | `±0.012` | Top of ARTHUR to bottom of Weapon/Ultimate panel, expanded for breathing room. |
| HeroSelection.RightColumn.OuterPanel | `(0.683, 0.016, 0.308, 0.715)` | `±0.012` | Reference treats ARTHUR/LAB/subtitle as column top; expanded after overlay sanity pass. |
| HeroSelection.RightColumn.HeaderRow.HeroName | `(0.700, 0.047, 0.097, 0.046)` | `±0.006` | Plain ARTHUR title label. |
| HeroSelection.RightColumn.HeaderRow.LabButton | `(0.921, 0.040, 0.062, 0.046)` | `±0.006` | Red selected LAB button. |
| HeroSelection.RightColumn.Subtitle | `(0.700, 0.107, 0.189, 0.022)` | `±0.006` | `A KING. A CRUSADE. AN APOCALYPSE.` |
| HeroSelection.RightColumn.RankPanel | `(0.692, 0.158, 0.290, 0.052)` | `±0.006` | Begins at `y≈0.158`. |
| HeroSelection.RightColumn.RankPanel.InfoIcon | `(0.699, 0.172, 0.013, 0.023)` | `±0.005` | Question icon. |
| HeroSelection.RightColumn.RankPanel.Label | `(0.719, 0.174, 0.035, 0.027)` | `±0.006` | `RANK`. |
| HeroSelection.RightColumn.RankPanel.LockIcon | `(0.757, 0.173, 0.011, 0.021)` | `±0.005` | Yellow lock. |
| HeroSelection.RightColumn.RankPanel.Value | `(0.963, 0.177, 0.013, 0.028)` | `±0.006` | `--`; layout box includes label line height. |
| HeroSelection.RightColumn.MasteryPanel | `(0.692, 0.224, 0.290, 0.056)` | `±0.006` | Mastery row. |
| HeroSelection.RightColumn.MasteryPanel.InfoIcon | `(0.699, 0.239, 0.013, 0.023)` | `±0.005` | Question icon. |
| HeroSelection.RightColumn.MasteryPanel.Label | `(0.719, 0.239, 0.051, 0.027)` | `±0.006` | `MASTERY`. |
| HeroSelection.RightColumn.MasteryPanel.ProgressBar | `(0.786, 0.248, 0.092, 0.018)` | `±0.008` | Small status fill bar. |
| HeroSelection.RightColumn.MasteryPanel.Level | `(0.908, 0.243, 0.030, 0.028)` | `±0.006` | `LV 1`. |
| HeroSelection.RightColumn.MasteryPanel.XP | `(0.944, 0.243, 0.048, 0.028)` | `±0.006` | `0 / 100 XP`. |
| HeroSelection.RightColumn.StatsPanel | `(0.691, 0.310, 0.291, 0.203)` | `±0.010` | Stats panel with decorative header. |
| HeroSelection.RightColumn.StatsPanel.Header | `(0.807, 0.309, 0.060, 0.030)` | `±0.008` | `STATS`. |
| HeroSelection.RightColumn.StatsPanel.Damage | `(0.705, 0.351, 0.103, 0.026)` | `±0.008` | `Damage 4/99`. |
| HeroSelection.RightColumn.StatsPanel.AttSpeed | `(0.705, 0.390, 0.103, 0.026)` | `±0.008` | `ATT Speed 2/99`. |
| HeroSelection.RightColumn.StatsPanel.AttScale | `(0.705, 0.429, 0.103, 0.026)` | `±0.008` | `ATT Scale 2/99`. |
| HeroSelection.RightColumn.StatsPanel.Accuracy | `(0.705, 0.466, 0.103, 0.026)` | `±0.008` | `Accuracy 2/99`. |
| HeroSelection.RightColumn.StatsPanel.Armor | `(0.845, 0.351, 0.126, 0.026)` | `±0.008` | `Armor 7/99`. |
| HeroSelection.RightColumn.StatsPanel.Evasion | `(0.845, 0.390, 0.126, 0.026)` | `±0.008` | `Evasion 1/99`. |
| HeroSelection.RightColumn.StatsPanel.Luck | `(0.845, 0.429, 0.126, 0.026)` | `±0.008` | `Luck 2/99`. |
| HeroSelection.RightColumn.StatsPanel.Speed | `(0.845, 0.466, 0.126, 0.026)` | `±0.008` | `Speed 2/99`. |
| HeroSelection.RightColumn.WeaponUltimatePanel | `(0.691, 0.528, 0.291, 0.153)` | `±0.010` | Weapon/Ultimate split panel. |
| HeroSelection.RightColumn.WeaponUltimatePanel.WeaponColumn | `(0.693, 0.529, 0.143, 0.151)` | `±0.010` | Left half. |
| HeroSelection.RightColumn.WeaponUltimatePanel.WeaponColumn.Label | `(0.785, 0.545, 0.055, 0.026)` | `±0.008` | `WEAPON`. |
| HeroSelection.RightColumn.WeaponUltimatePanel.WeaponIcon | `(0.742, 0.568, 0.061, 0.068)` | `±0.015` | Content icon. |
| HeroSelection.RightColumn.WeaponUltimatePanel.UltimateColumn | `(0.836, 0.529, 0.145, 0.151)` | `±0.010` | Right half. |
| HeroSelection.RightColumn.WeaponUltimatePanel.UltimateColumn.Label | `(0.873, 0.545, 0.071, 0.026)` | `±0.008` | `ULTIMATE`. |
| HeroSelection.RightColumn.WeaponUltimatePanel.UltimateIcon | `(0.883, 0.556, 0.076, 0.075)` | `±0.015` | Content icon. |

## Bottom Row

| Tag / Region | Ref BBox | Tolerance | Notes |
|---|---:|---:|---|
| HeroSelection.BottomRow | `(0.000, 0.755, 1.000, 0.202)` | `±0.012` | Full-width bottom band containing the three panel groups, moved down after overlay sanity pass. |
| HeroSelection.BottomRow.SteamPartyPanel | `(0.009, 0.755, 0.344, 0.202)` | `±0.012` | Four Steam party slots. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot01.ReadyBadge | `(0.040, 0.780, 0.063, 0.028)` | `±0.010` | READY badge above local player slot. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot01 | `(0.028, 0.816, 0.076, 0.135)` | `±0.012` | Ready green square slot. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot01.SteamAvatar | `(0.033, 0.820, 0.045, 0.080)` | `±0.015` | Placeholder Steam profile image area. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot01.HeroPortrait | `(0.074, 0.897, 0.026, 0.046)` | `±0.012` | Nested selected-hero portrait. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot02 | `(0.110, 0.816, 0.076, 0.135)` | `±0.012` | Steam slot. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot02.SteamAvatar | `(0.120, 0.820, 0.045, 0.080)` | `±0.015` | Placeholder Steam profile image area. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot02.HeroPortrait | `(0.156, 0.897, 0.026, 0.046)` | `±0.012` | Nested hero portrait. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot03 | `(0.191, 0.816, 0.076, 0.135)` | `±0.012` | Steam slot. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot03.SteamAvatar | `(0.202, 0.820, 0.045, 0.080)` | `±0.015` | Placeholder Steam profile image area. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot03.HeroPortrait | `(0.237, 0.897, 0.026, 0.046)` | `±0.012` | Nested hero portrait. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot04 | `(0.273, 0.816, 0.076, 0.135)` | `±0.012` | Steam slot. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot04.SteamAvatar | `(0.284, 0.820, 0.045, 0.080)` | `±0.015` | Placeholder Steam profile image area. |
| HeroSelection.BottomRow.SteamPartyPanel.Slot04.HeroPortrait | `(0.319, 0.897, 0.026, 0.046)` | `±0.012` | Nested hero portrait. |
| HeroSelection.BottomRow.CompanionPanel | `(0.370, 0.755, 0.264, 0.202)` | `±0.012` | Chad/Stacy row plus Choose Companion. |
| HeroSelection.BottomRow.CompanionPanel.ChadButton | `(0.386, 0.776, 0.109, 0.054)` | `±0.010` | Selected red. |
| HeroSelection.BottomRow.CompanionPanel.StacyButton | `(0.510, 0.778, 0.105, 0.054)` | `±0.010` | Default purple. |
| HeroSelection.BottomRow.CompanionPanel.ChooseCompanionButton | `(0.386, 0.861, 0.195, 0.067)` | `±0.010` | Wide lower button. |
| HeroSelection.BottomRow.RightCluster | `(0.651, 0.755, 0.341, 0.201)` | `±0.012` | Difficulty, Enter, Challenges, Mods cluster. |
| HeroSelection.BottomRow.DifficultyPanel | `(0.662, 0.787, 0.081, 0.101)` | `±0.015` | Difficulty label plus dropdown frame. |
| HeroSelection.BottomRow.DifficultyPanel.Label | `(0.674, 0.796, 0.065, 0.030)` | `±0.012` | `DIFFICULTY`. |
| HeroSelection.BottomRow.DifficultyPanel.Dropdown | `(0.662, 0.833, 0.081, 0.052)` | `±0.015` | `Easy`. |
| HeroSelection.BottomRow.DifficultyPanel.EnterButton | `(0.759, 0.776, 0.106, 0.148)` | `±0.012` | Prominent red CTA. |
| HeroSelection.BottomRow.ChallengesButton | `(0.880, 0.776, 0.100, 0.065)` | `±0.010` | `CHALLENGES`; independent button. |
| HeroSelection.BottomRow.ModsButton | `(0.880, 0.861, 0.100, 0.065)` | `±0.010` | `MODS`; independent button. |

