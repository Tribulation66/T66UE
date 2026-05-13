# Companion Selection Structural Inventory

No-reference Stage 2 structural baseline derived from:

- Baseline capture: `C:\UE\T66\Saved\Codex\UI\CompanionSelection\baseline_capture.png`
- Baseline dump: `C:\UE\T66\Saved\Codex\UI\CompanionSelection\baseline_dump.json`

Coordinate basis: normalized 1920x1080.

## Regions

| Element | x | y | w | h | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| CompanionSelection.Root | 0.000 | 0.000 | 1.000 | 1.000 | Full screen root |
| CompanionSelection.LeftPanel | 0.000 | 0.016 | 0.285 | 0.694 | Skins and account-currency column |
| CompanionSelection.BackButton | 0.008 | 0.029 | 0.058 | 0.029 | BACK action in upper-left |
| CompanionSelection.BalanceBadge | 0.190 | 0.029 | 0.086 | 0.029 | Companion currency badge |
| CompanionSelection.SkinsTitle | 0.107 | 0.070 | 0.071 | 0.035 | SKINS label |
| CompanionSelection.SkinsList | 0.010 | 0.117 | 0.266 | 0.556 | Scrollable skin rows |
| CompanionSelection.Skins.DefaultRow | 0.010 | 0.120 | 0.266 | 0.067 | Default skin row |
| CompanionSelection.Skins.BeachgoerRow | 0.010 | 0.194 | 0.266 | 0.067 | Beachgoer skin row |
| CompanionSelection.Carousel | 0.300 | 0.010 | 0.370 | 0.120 | Prev/portrait/next strip |
| CompanionSelection.PreviewPanel | 0.300 | 0.140 | 0.370 | 0.580 | Companion preview area |
| CompanionSelection.RightPanel | 0.690 | 0.010 | 0.310 | 0.700 | Companion details column |
| CompanionSelection.CompanionName | 0.725 | 0.047 | 0.241 | 0.043 | Selected companion name |
| CompanionSelection.PortraitPanel | 0.715 | 0.109 | 0.261 | 0.109 | Portrait/detail visual strip |
| CompanionSelection.RankRow | 0.715 | 0.242 | 0.261 | 0.059 | Rank metric row |
| CompanionSelection.UnityRow | 0.715 | 0.318 | 0.261 | 0.089 | Unity metric row |
| CompanionSelection.LoreHeader | 0.715 | 0.424 | 0.261 | 0.043 | LORE toggle/action |
| CompanionSelection.LorePanel | 0.715 | 0.476 | 0.261 | 0.122 | Lore body panel |
| CompanionSelection.PassivePanel | 0.715 | 0.615 | 0.261 | 0.065 | Passive/healing body panel |
| CompanionSelection.PartyPanel | 0.000 | 0.740 | 0.350 | 0.200 | Party member footer |
| CompanionSelection.ConfirmPanel | 0.370 | 0.740 | 0.260 | 0.200 | Confirm companion footer |
| CompanionSelection.ConfirmButton | 0.383 | 0.764 | 0.233 | 0.117 | CONFIRM COMPANION action |
| CompanionSelection.RunPanel | 0.650 | 0.740 | 0.340 | 0.200 | Difficulty/enter/community footer |
| CompanionSelection.DifficultyDropdown | 0.664 | 0.807 | 0.120 | 0.065 | Difficulty selector |
| CompanionSelection.EnterButton | 0.788 | 0.807 | 0.089 | 0.065 | ENTER/ready action |
| CompanionSelection.ChallengesButton | 0.881 | 0.807 | 0.058 | 0.065 | Community challenges action |
| CompanionSelection.ModsButton | 0.943 | 0.807 | 0.046 | 0.065 | Community mods action |

## Roles

- Buttons: Back, carousel prev/next, carousel slots, Beachgoer preview/buy-or-equip, Lore, Confirm Companion, Difficulty dropdown, Enter, Challenges, Mods.
- Toggle groups: carousel portrait slots use `CompanionSelectionCarousel`.
- Labels: SKINS, currency label/value, skin names, companion name, rank label/value, unity label/value, lore text, passive text, party slot labels.
- Content artwork is preserved as live companion/portrait data where available; chrome is flat only.
