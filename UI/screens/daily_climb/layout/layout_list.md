# Daily Climb Layout List

Canvas: 1920x1080

Source current screenshot: `C:\UE\T66\UI\screens\daily_climb\current\current_state_1920x1080.png`
Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\main_menu_reference_1920x1080.png`

## Fixed Structure

- Full-screen scenic dark red/black temple background with clean lighting, banners, torches, and a central golden Chad statue/stair focal point.
- Small `Back` button in the top-left corner.
- Large centered page title across the upper third.
- Subtitle centered below the title.
- Large left-side rules panel occupying the lower-left quadrant:
  - panel title `Today's Rules`
  - divider line
  - bordered description text box
  - four stat/info cards in a two-by-two grid:
    - hero
    - difficulty
    - seed quality
    - reward
  - `Rule Stack` heading
  - long empty rule-stack box near the panel bottom
- Large bottom-right `Start Challenge` CTA button in a heavy frame.

## Runtime-Owned Regions

- Back label, page title, subtitle, panel title, description, card labels, card values, rule-stack text, reward values, and CTA label are runtime-owned text.
- Hero/difficulty/seed/reward values are runtime-owned values.
- The scenic background and shell frames are generated art; text and values stay live later.

## Explicit Bans

- Do not add top navigation, sidebars, leaderboards, extra tabs, inventory slots, currency boxes, or additional CTA buttons.
- Do not move the left rules panel or bottom-right CTA into a new layout.
