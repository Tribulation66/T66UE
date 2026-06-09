# Main Menu Pass13 Component Contract

Status: Pass13 pre-implementation contract.

Reference:
`UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Current rejected evidence:

- `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass12_fixture_capture.png`
- `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass12_material_crop_sheet.png`
- `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass12_material_verdict.md`

Pass13 rule: imagegen or equivalent source-art authoring may produce blank
chrome plates, but live labels, player data, glyphs, state, sizing, layout, and
interaction stay in Slate. Do not use screenshot crop/inpaint output as the
production source for this pass.

## Ownership Rules

| Family | Plate/source-art owns | Slate owns | Explicitly forbidden |
|---|---|---|---|
| Topbar icon buttons | Blank dark/red rounded rubber square frame, bevel, gloss, shadow | Gear/globe/power glyph widgets, click handlers, selected state, tooltip, tag | Baked glyph under live glyph, fake icon marks, painted-over icon area |
| Topbar text tabs | Blank dark/red long rubber tab body, bevel, gloss, shadow | ACCOUNT/HOME/POWER UP/ACHIEVEMENTS labels, font fit, selected state, click handler | Baked text, masked text remnants, clipped text |
| CTA primary/secondary | Blank red/dark long rubber button body, bevel, gloss, shadow | ENTER TRIBULATION/LOAD GAME labels, skull slots if used, click handler, enabled state | Baked skulls under live skulls, baked or painted-over text, manual center mask |
| Leaderboard local row | Red outline/rim with dark interior, rounded row shell | Rank, avatar, player name, score, click/favorite behavior | Red-filled interior, baked rank/name/score |
| Metric checkbox | Uniform rounded square checked and empty states | High Score/Speed Run label, selected state, click handler | Nonuniform shape, partial fill, checkmark baked into wrong-sized shell |
| Friends section headers | Dark rubber header strip, optional room for status dot | Expand arrow, green/gray status dot, ONLINE/OFFLINE label, count, click behavior | Missing online dot, baked labels/counts |
| Friend rows | Dark rubber row shell, bevel, shadow | Avatar, name, level, star, invite/offline action, favorite/invite handlers | Child content baked into row plate, row or action button escaping parent |
| Friend action buttons | Green invite and dark disabled/offline mini button shells | INVITE/OFFLINE label and enabled state | Green state hidden by wrong z-order, baked labels, text overflow |

## Runtime Component Targets

| Family | Code target | Existing chrome token | Pass13 target |
|---|---|---|---|
| Topbar icon buttons | `Source/T66/UI/T66FrontendTopBarWidget.cpp::MakeIconActionButton` | `TopbarIconDarkRound06`, `TopbarPowerRedRound06` | Blank icon plate plus live glyph only. Selected state may use red power plate only if it is blank behind live glyph. |
| Achievements tab | `Source/T66/UI/T66FrontendTopBarWidget.cpp::MakeCategoryItem` and category button loop | `TopbarTabDarkRound06`, `TopbarTabRedRound06` | Use fitted label sizing for long labels. `ACHIEVEMENTS` must fit at its rect width without clipping. |
| CTA buttons | `Source/T66/UI/Screens/T66MainMenuScreen.cpp::MakeCtaButton` | `CtaPrimaryRound06`, `CtaSecondaryRound06` | Clean blank body plate with live label and live skull slots only. |
| Search field | `Source/T66/UI/Screens/T66MainMenuScreen.cpp::MakeSearchField` | `SearchFieldRound06` | Replace the temporary `?` text glyph with a deliberate search glyph or hide the glyph if no accepted glyph exists. |
| Friends headers | `Source/T66/UI/Screens/T66MainMenuScreen.cpp::MakeFriendGroupToggle` | `SectionHeaderRound06` | Add live status dot before label: green for online, gray for offline. |
| Friend rows/actions | `Source/T66/UI/Screens/T66MainMenuScreen.cpp::MakeFriendRow` | `FriendRowRound06`, `InviteButtonGreenRound06`, `OfflineButtonDarkRound06` | Stable row/action sizes, row containment, visible green online invite state. |
| Leaderboard local row | `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp::BuildLeaderboardRow` | `RankingRowRedRound06` | Red outline with dark interior, not a red-filled row. |
| Metric checkbox | `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp::BuildMetricCheckButton` | `CheckboxCheckedRound06`, `CheckboxEmptyRound06` | Uniform rounded square states at stable 28x28 or revised stable contract size. |

## Size And Fit Contract

All sizes are runtime reference-space sizes before viewport scaling.

| Family | Normal size | Min size | Content insets | Text/icon rule |
|---|---:|---:|---|---|
| Topbar icon button | 86x68 to 96x74 | 74x62 | 8 px all sides | Live glyph max 42 px, centered. No glyph may touch bevel or plate edge. |
| Topbar text tab | Existing normalized rect width/height | Label must fit at current rect width | 12 px horizontal, 8 px vertical | Label font clamps by measured text width. `ACHIEVEMENTS` is the longest gate label. |
| CTA primary | 680x104 | 560x92 | 26 px horizontal, 10 px vertical | Main label centered. Skull slots are reserved fixed boxes, not baked into plate. |
| CTA secondary | 660x94 | 540x82 | 26 px horizontal, 10 px vertical | Label centered, no center paint/mask behind text. |
| Leaderboard local row | 406x46 current token | 360x42 | 7 px horizontal, 4 px vertical | Rank/name/score ellipsize within row. Score stays right aligned. |
| Metric checkbox | 28x28 current token | 28x28 | N/A | Label is separate and does not change box geometry. |
| Friends section header | 460x42 | 420x38 | 10 px horizontal, 5 px vertical | Dot, arrow, label, and count stay in a single contained row. |
| Friend row | 460x58 | 420x54 | 8 px horizontal, 5 px vertical | Name ellipsizes before star/action. Action button never leaves row. |
| Friend action button | 80x44 invite, 80x42 offline | 72x38 | 8 px horizontal, 4 px vertical | Label font clamps if needed; online state must visibly use green plate. |

## Verification Rows

Pass13 material/state scorecard must include these rows:

| Row | Required PASS condition |
|---|---|
| topbar_icon_ownership | Reference crop and capture crop show one icon per button. No doubled glyph or icon-on-icon effect. |
| achievements_text_fit | `ACHIEVEMENTS` fits inside its button at 1920x1080 and compact width rule; no clipping or overlap. |
| cta_clean_plate | CTA bodies have clean rubber centers with no painted-over text or skull remnants. |
| leaderboard_local_row_style | Local row has red outline/rim and dark interior, not red-filled interior. |
| metric_checkbox_shape | Checked and empty boxes are uniform rounded squares with stable dimensions. |
| online_header_dot | Online header includes a live green status dot before the label. |
| invite_green_state | Online friend action uses a visible green invite button and fitted label. |
| row_containment | Friend rows, action buttons, and leaderboard rows remain inside owning panel body. |

## Anti-Lookalike Gate

Cheapest wrong result: a crop/inpaint plate with live labels layered over
painted-out text, plus generic PASS counts from structural checks.

Discriminator: every checked component has one clear source of truth for plate,
text, icon, and state; crop sheets show no masking artifacts; verifier checks
containment; visual scorecard rows explain PASS/FAIL against the reference crop.
