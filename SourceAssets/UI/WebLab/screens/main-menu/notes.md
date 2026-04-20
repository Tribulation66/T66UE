# Main Menu Hierarchy

This screen is built as a fixed `1920x1080` Unreal-style browser replica.
Current style pass: `Dungeon Sanctum`.

## Layer 1: Screen Root

- `main-menu-screen`
- `screen__background`
- `top-bar`
- `scene-title`
- `screen__columns`

## Layer 2: Background Stack

- `screen__bg-sanctum`
- `screen__bg-arch`
- `screen__bg-banner--left`
- `screen__bg-banner--right`
- `screen__torch--left`
- `screen__torch--right`
- `screen__bg-backlight`
- `screen__bg-floor`
- `screen__bg-layer--pyramid`
- `screen__bg-fog`
- `screen__bg-embers`
- `screen__scrim`

## Layer 2: Top Bar

- `top-bar__surface`
- `top-bar__left`
- `top-bar__center`
- `top-bar__right`

## Layer 3: Top Bar Left Utility Cluster

- `top-bar__utility-button` for settings
- `top-bar__utility-button` for language

## Layer 3: Top Bar Center Navigation

- `top-bar__nav-button` for account
- `top-bar__nav-button--home` for home
- `top-bar__home-crest`
- `top-bar__nav-button` for power up
- `top-bar__nav-button` for achievements
- `top-bar__nav-button` for minigames

## Layer 3: Top Bar Right Utility Cluster

- `top-bar__coupon-button`
- `top-bar__coupon-icon-wrap`
- `top-bar__coupon-balance`
- `top-bar__utility-button` for quit

## Layer 2: Scene Title

- `scene-title`
- `scene-title__ornament--left`
- `scene-title__glow`
- `scene-title__main`
- `scene-title__ornament--right`

## Layer 2: Bottom Column Layout

- `social-panel`
- `center-actions`
- `leaderboard-column`

## Layer 3: Left Social Panel

- `social-panel__scroll`
- `party-panel`

## Layer 4: Profile Card

- `profile-card`
- `profile-card__avatar`
- `profile-card__avatar-fill`
- `profile-card__meta`
- `profile-card__name`
- `profile-card__status`
- `profile-card__line`

## Layer 4: Friends Area

- `social-panel__divider`
- `friend-search`
- `friend-groups`
- `friend-group-header`
- `friend-group-header__arrow`
- `friend-group-header__label`
- `friend-group-header__count`

## Layer 4: Party Footer

- `party-panel__header`
- `party-panel__title`
- `party-panel__leave`
- `party-panel__slots`
- `party-slot`

## Layer 3: Center Action Column

- `center-actions__shell`
- `main-action--start`
- `main-action`

## Layer 3: Leaderboard Column

- `leaderboard-column__filters`
- `leaderboard-shell`

## Layer 4: Leaderboard Filters

- `leaderboard-filter` global
- `leaderboard-filter` friends
- `leaderboard-filter` streamers

## Layer 4: Leaderboard Shell

- `leaderboard-shell__time-row`
- `leaderboard-shell__dropdown-row`
- `leaderboard-shell__type-row`
- `leaderboard-shell__headers`
- `leaderboard-shell__empty`

## Layer 5: Leaderboard Controls

- `time-filter`
- `leaderboard-dropdown`
- `leaderboard-dropdown__arrow`
- `leaderboard-type`
- `leaderboard-type__dot`
- `leaderboard-shell__header`

## Asset Notes

- Background shrine environment, banners, torches, trim, and logo crest are built in HTML/CSS/SVG.
- The altar still uses the repo `pyramid_chad` layer as the centerpiece.
- The three leaderboard filter textures are not available as source PNGs in the repo. The browser replica uses SVG stand-ins for global and friends, plus `Forsen_male_low.png` as the streamer portrait placeholder so the component layout remains intact.
