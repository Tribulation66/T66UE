# FriendslopStyle Main Menu Element Manifest

Reference visual: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Fresh baseline capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\current_baseline_20260605_0953.png`

Fresh baseline dump: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\current_baseline_20260605_0953_dump.json`

Status note: this manifest records the Main Menu pilot mapping and is now the
required five-family visual ledger. After the 2026-06-06 process correction, a
future Main Menu implementation pass must first assess the five families below
as visual `PASS` or visual `FAIL`. Do not treat the asset names below as
accepted final plates unless the active visual pass marks the owning family and
element visual `PASS`.

Latest pass ledger: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_visual_family_element_assessment.md`

Latest pass review evidence: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_reference_vs_final_contact_sheet.png`

## Per-Pass Five-Family Ledger Contract

Every Main Menu implementation pass must classify exactly five visual families:

- `TopBar`
- `LeftSocialPanel`
- `RightLeaderboardPanel`
- `CenterButtonStack`
- `Background`

Visual vocabulary:

- Visual `PASS`: the family or element does not need image regeneration in this
  iteration.
- Visual `FAIL`: the family or element must be regenerated in this iteration.

Use visual `FAIL` for wrong rubber material, wrong silhouette, smears, masks,
pillow centers, baked text/icons/data, title/background style failure, wrong
blank plate, wrong row fill, or any visual mismatch with the reference. Be
strict: if there is doubt, the family or element is visual `FAIL`.

For every visual `FAIL` family, classify every element inside that family as
visual `PASS` or visual `FAIL`. Launch one local Codex CLI imagegen worker per
visual `FAIL` family, and that worker must generate the sheet/assets for all
visual `FAIL` elements in that family. The worker must use the cached textless
family crop for the approved reference and output both a contact sheet and
individualized backgroundless PNGs for runtime elements. Manual cropping from a
family sheet is not the runtime asset path. All generated assets must be
implemented onto the screen before the pass reports.

Worker prompt language rule:

- Use extraction-only prompt language for failed-family workers.
- Follow `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
  Section 2.2.1 for the full Allowed/Forbidden definition.
- Do not use descriptive/adjectival material, shape, color, vibe, polish, or
  game-comparison language.
- Treat the cached textless family crop as the only visual style authority.
- If a worker request cannot identify the needed element without descriptive
  language, stop and get a better crop, reference, or user decision.

User-only preliminary pass:

- Only the user can mark an element as preliminary pass.
- A preliminary-pass element is preserved from the exact prior iteration path
  named in the pass ledger and is not regenerated in that pass.
- The agent may not infer preliminary pass or skip regeneration because an
  element appears close enough.
- Current user-marked preliminary-pass carry-forward set for the next
  iteration: previous topbar buttons, previous invite button, previous party
  slots, Global Chad Ranking weekly/all-time/solo/easy/high score/speedrun
  controls, background, and title branding.

Pre-worker assessment output must show the element breakdown grouped under each
family so a reviewer can quickly confirm nothing was skipped. Use this format
for every family:

```markdown
**TopBar family - FAIL**

| Element | Visual PASS/FAIL |
|---|---|
| Topbar outer shell | FAIL |
| Topbar icon plates | FAIL |
| Topbar tabs | FAIL |
| Ticket badge | FAIL |
```

The family heading starts in bold, names the family, and includes the family
verdict. The table has exactly two columns: `Element` and `Visual PASS/FAIL`.
Do not put worker instructions, reasons, or notes in this table. Put those in a
separate worker queue or notes section after the element tables.

After generated assets are implemented, run a sizing/fitting correction pass
across the same five families and record what changed or blocked correction.
Then run wiring/functionality `PASS`/`FAIL` and correct wiring/functionality
failures. Do not summarize the pass as `FULL` or `PARTIAL`; report objective
coverage plus the wiring/functionality gate.

Every pass evidence packet must include both `reference_vs_current` and
`previous_vs_current` comparison sheets.

## Main Menu Sizing Contract

Side-panel sizing must use the framed-panel content-budget rule from
`UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`. Content width is the
primary sizing input, the inner content inset is explicit padding, and the
outer panel width is derived:

- left social panel: `ContentWidth=520`, `ContentInset=30`,
  `PanelWidth=ContentWidth + (ContentInset * 2) = 580`;
- right leaderboard panel: `ContentWidth=460`, `ContentInset=30`,
  `PanelWidth=ContentWidth + (ContentInset * 2) = 520`;
- two-column leaderboard controls derive each column from
  `(ContentWidth - ColumnGap) / 2`, currently `225` with a `10` pixel gap;
- runtime children inside framed side panels may not use the full panel width.

If a future iteration still reads as squished, do not shrink the side-panel
shell. First preserve or increase the outer shell width when the visual direction
needs breathing room, then reduce child control footprint, slot group width, or
named gaps inside the content area. Widening away from the reference target still
requires an explicit visual-direction decision from the user.

## Main Menu Anchor Contract

- Top bar outer chrome touches the top, left, and right edges of the 1920x1080
  reference canvas: `x=0`, `y=0`, `w=1920`.
- Left social panel touches the left edge and bottom edge of the canvas:
  `x=0`, `y+h=1080`.
- Right leaderboard panel touches the right edge and bottom edge of the canvas:
  `x+w=1920`, `y+h=1080`.
- Runtime verification should check these anchors from the merged screen dump
  where possible, and from screenshot/contact evidence when a top-level overlay
  is captured separately.

Implementation owner note: the live right leaderboard panel is
`ST66FlatLeaderboardPanel`, instantiated by `T66MainMenuScreen.cpp`. The legacy
local `MakeRightPanel` helper in `T66MainMenuScreen.cpp` is not the active
FriendslopStyle right-panel owner and must not be used for right-panel sizing
fixes.

## Required Visual Families

These five families are the only top-level Main Menu visual assessment units.
The element breakdown inside each family is the minimum inventory for failed
families; a pass may split an element when useful, but it may not omit any
listed element inside a failed family.

| Visual family | Reference target | Owned elements | Runtime owners | Worker rule |
|---|---|---|---|---|
| TopBar | Full-width dark rubber top strip and controls | Topbar outer shell; icon plates; tabs; ticket badge | `UT66FrontendTopBarWidget` | One worker generates all failed topbar plates/sheet. |
| LeftSocialPanel | Social/friends/party panel | Left panel frame; profile row; search pill; online/offline headers; friend rows; invite/offline action buttons; party panel and slots | `UT66MainMenuScreen` | One worker generates all failed left-panel plates/sheet. |
| RightLeaderboardPanel | Right filter rail and leaderboard panel | Right filter buttons; right leaderboard frame; leaderboard filters/dropdowns; leaderboard headers; leaderboard rows; scrollbars/dividers | `ST66FlatLeaderboardPanel` | One worker generates all failed right-panel plates/sheet. |
| CenterButtonStack | Center title/subtitle and CTA stack | Title branding; subtitle; primary CTA; secondary CTA | `UT66MainMenuScreen` | One worker generates all failed center/title/CTA assets/sheet. |
| Background | Star/fire/golden statue scene with rubbery Friendslop material | Background art | Main Menu background brush | One worker generates the failed no-text background art. |

## Element Breakdown For Failed Families

### TopBar

| Element | Plate/live ownership | Visual PASS/FAIL focus |
|---|---|---|
| Topbar outer shell | Blank chrome plus live controls | Silhouette, height, edge material. |
| Topbar icon plates | Blank plates plus live glyphs | No baked glyphs; icon-plate shape/material. |
| Topbar tabs | Blank tab plates plus live labels | Selected/default material, no baked text, tab shape. |
| Ticket badge | Blank pill plus live ticket icon/value | Plate shape and live count placement. |

### LeftSocialPanel

| Element | Plate/live ownership | Visual PASS/FAIL focus |
|---|---|---|
| Left panel frame | Blank panel chrome plus live rows | Reference rubber frame and inner dark body. |
| Profile row | Blank row chrome plus live avatar/name/progress | Material, progress state, content fit. |
| Search pill | Blank pill plus live input/icon | No flat/thin plate, no smear, live text fit. |
| Online/offline headers | Blank toggle chrome plus live label/count/dot | Header shape/material and dot placement. |
| Friend rows | Blank row chrome plus live avatar/name/state | Row shape, dark interior, no overflow. |
| Invite/offline action buttons | Blank action plate plus live action text | Green invite, dark offline, no masked center. |
| Party panel and slots | Blank slot plates plus live members/plus glyph | Slot shape, plus/live ownership, containment. |

### RightLeaderboardPanel

| Element | Plate/live ownership | Visual PASS/FAIL focus |
|---|---|---|
| Right filter buttons | Blank icon plates plus live icons | Side tab material and selected/default states. |
| Right leaderboard frame | Blank panel chrome plus live filters/rows | Reference rubber frame and internal density. |
| Leaderboard filters/dropdowns | Blank button/dropdown chrome plus live text/icons | Text fit, state fill/outline, containment. |
| Leaderboard headers | Live text | Alignment and clipping. |
| Leaderboard rows | Blank row plates plus live rank/name/score/icons | Local red outline/dark interior, row containment. |
| Scrollbars/dividers | Blank chrome only | Width, material, no smudged seams. |

### CenterButtonStack

| Element | Plate/live ownership | Visual PASS/FAIL focus |
|---|---|---|
| Title branding | Generated title asset or live Slate title | Not cropped, no subtitle/background fragments. |
| Subtitle | Live Slate text | Contained under title, no overlap. |
| Primary CTA | Blank CTA plate plus live label/icons | Reference silhouette/material; no pillow/mask. |
| Secondary CTA | Blank CTA plate plus live label | Dark material and label placement. |

### Background

| Element | Plate/live ownership | Visual PASS/FAIL focus |
|---|---|---|
| Background art | Generated background art only; no UI text/data | Star/fire/golden statue scene and current runtime path. |

## Implementation Mapping

| Screen Region / Tag Family | Runtime Chrome Asset | Live Content Source | Notes |
|---|---|---|---|
| `FrontendTopBar.OuterContainer` | `panel_large_dark.png` | `UT66FrontendTopBarWidget` | Shared top bar keeps existing route/click handlers. |
| `FrontendTopBar.*Button` | selected: `button_primary_red.png`, default: `button_long_dark.png` | `UT66FrontendTopBarWidget` | Icon/text content remains Slate. |
| `FrontendTopBar.TicketBadge` | `pill_dark.png` | `GetChadCouponsValueText()` | Coupon icon/value remain live. |
| `MainMenu.Left.Panel` | `panel_large_dark.png` | `UT66MainMenuScreen::BuildFlatMainMenuUI` | Large left friends/party panel. |
| `MainMenu.Left.ProfileButton` | `row_dark.png` | Steam helper, achievements, account navigation | Profile avatar/name/level/progress remain live. |
| `MainMenu.Left.SearchField` | `pill_dark.png` | `FriendSearchQuery` and editable text box | Search remains interactive. |
| `MainMenu.Left.OnlineToggle`, `OfflineToggle` | `button_long_dark.png` | Party subsystem friend counts | Expand/collapse remains live. |
| `MainMenu.Left.*FriendRow*` | `row_dark.png` | Party subsystem friend entries | Online/offline action state remains live. |
| `MainMenu.Left.*ActionPanel` | online invite: `button_action_green.png`, disabled/offline: `button_long_dark.png` | Party/session subsystem invite eligibility | Action text remains live. |
| `MainMenu.Left.PartySlot*` | `party_slot_dark.png` | Party member/avatar data | Preserve four-slot party layout. |
| `MainMenu.Center.Title` | Slate text only | Static localized title text | Must read `CHADPOCALYPSE`, not baked image text. |
| `MainMenu.Center.EnterTribulationButton` | `button_primary_red.png` | Main Menu new-game handler | Primary CTA. |
| `MainMenu.Center.LoadGameButton` | `button_long_dark.png` | Main Menu load-game handler | Secondary CTA. |
| `MainMenu.Right.Filter*Button` | selected: `button_primary_red.png`, default: `icon_button_dark.png` | `ST66FlatLeaderboardPanel` filter state | Keeps icon brushes and tooltips. |
| `MainMenu.Right.LeaderboardPanel` | `panel_large_dark.png` | Leaderboard subsystem/backend | Live leaderboard panel. |
| `MainMenu.Right.Time*Button`, dropdowns, metric buttons | selected: `button_primary_red.png`, default: `button_long_dark.png` / `pill_dark.png` | `ST66FlatLeaderboardPanel` | Dropdown menus remain runtime Slate. |
| `MainMenu.Right.RankingRow*` | local/selected: `row_selected_red.png`, default: `row_dark.png` | Leaderboard entries | Rows remain clickable/favoritable. |

## Scope Notes

The existing function names still include `Flat` in some places during the pilot because the Main Menu implementation is currently a monolithic Slate build path. The visual owner for the pilot is `FT66FriendslopStyle`; future cleanup can rename functions after the style is accepted.

