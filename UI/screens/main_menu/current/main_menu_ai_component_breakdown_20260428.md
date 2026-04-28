# Main Menu AI Component Breakdown

Runtime screenshot:
`C:\UE\T66\UI\screens\main_menu\current\main_menu_runtime_capture_20260428_1920x1080.png`

Use this as a visual inventory for style rendering. The descriptions below are intentionally generic and avoid saved asset names.

## Overall Composition

- Full 1920x1080 game main menu screen with a dark fantasy/sci-fi arcade presentation.
- Strong contrast between black UI panels, neon magenta outlines, cream pixel text, and a gold/orange central hero scene.
- Layout is divided into five major zones: top navigation bar, left social panel, center title/hero art, center action buttons, and right leaderboard panel.

## Background And Hero Scene

- Deep black starfield backdrop with many small white stars.
- Large fiery eclipse/corona ring behind the central figure, glowing orange and red.
- Monumental gold stylized male bust/statue in the center, lit like polished metal.
- Golden stepped pyramid or altar base below the statue, with bright reflective highlights.
- Foreground gold steps and architectural blocks occupy the lower center, partially behind the action buttons.

## Title Area

- Large blocky pixel-art game title wordmark centered near the top.
- Title letters are pale cream/purple with thick dark shadowing and beveled pixel edges.
- Short purple tagline under the title, rendered in chunky outlined pixel text.

## Top Navigation Bar

- Full-width black horizontal header strip with a bright magenta border and rounded corners.
- Large square/rectangular nav slots with dark interiors and purple outlines.
- Left utility icons: gray gear/settings icon and gray language/translation icon.
- Text navigation tabs: account, power-up, achievements, and minigames.
- Center home/profile badge: square portrait-style gold bust icon inside a purple-framed slot.
- Right currency area: colorful ticket/coupon icon with a white number counter.
- Far-right red power/quit symbol inside the top bar.

## Center Action Area

- Two large stacked primary buttons in the lower-middle of the screen.
- Buttons have dark rectangular bodies, thick magenta neon borders, subtle beveled edges, and large white pixel text.
- Button labels are centered and high contrast.
- The top action is for starting a new run; the second action is for loading a saved run.

## Left Social Panel

- Tall left-side dark panel with a bright magenta rounded border.
- Top profile block with square avatar, player name, level text, horizontal progress bar, and next-level label.
- Thin divider line below the profile block.
- Friend search field with dark fill, purple border, and gray placeholder text.
- Online friends section with small expand/collapse chevron, section label, and count.
- Friend rows contain square avatar frames, player name, level text, favorite star icon, and an invite button.
- Offline friends section mirrors the online section but uses muted text and disabled/offline buttons.
- Party section at the bottom with a section label and four square party slots.
- First party slot shows the local player avatar; remaining slots show centered purple plus icons for empty invite/add slots.

## Right Leaderboard Panel

- Tall right-side dark leaderboard panel with a bright magenta rounded border.
- Header label at the top in purple pixel text.
- Small vertical category selector icons along the left edge of the leaderboard area: globe, friends/group, and expressive face/emote-style icon.
- Two large top tabs for weekly and all-time leaderboard views; the active tab is filled purple.
- Two dropdown-style filter fields below the tabs, one for party/mode and one for difficulty.
- Metric selector row with two radio-style controls for high-score and speed-run modes.
- Table header with rank, player name, and score columns.
- Leaderboard rows with rank number, small square avatar frame, player name, score value, and thin separator lines.
- Bottom visible row highlights the local player lower in the ranking list.

## Reusable UI Primitives

- Neon magenta outer frame/border.
- Dark rounded panel shell.
- Dark rectangular button plate with magenta bevel.
- Purple active tab and dark inactive tab.
- Dropdown field with dark fill and small chevron.
- Radio selector ring with purple active dot.
- Square avatar frame with magenta border.
- Thin horizontal divider/separator line.
- Purple progress bar fill inside a dark progress track.
- Pixel-font labels, headers, and numeric counters.

## AI Rendering Guidance

- Treat text, icons, panels, and character/background art as separable layers.
- Preserve the layout hierarchy: top nav, side panels, central hero art, central action buttons.
- Preserve the readability of all UI text and do not merge text into painterly background details.
- Keep the center statue and fire-ring background visually dominant, while side panels remain readable functional UI.
- Maintain the black, gold, orange, cream, and electric-magenta value structure unless deliberately exploring a new palette.
