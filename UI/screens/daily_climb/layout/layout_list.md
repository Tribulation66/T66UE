# Daily Climb Layout List

Reference gate target: generated screen-specific Daily Climb full-screen reference. Preserve the current full-screen Daily Challenge hierarchy and restyle it into the canonical main-menu fantasy material language. Do not redesign the screen into a different layout.

## Required Inputs

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot: `C:\UE\T66\UI\screens\daily_climb\current\2026-04-24\current_runtime_1920x1080.png`
- Source layout: `C:\UE\T66\Source\T66\UI\Screens\T66DailyClimbScreen.cpp`
- This layout list.

## Canvas And Capture Notes

- Canonical generated reference canvas: 1920x1080.
- Current screenshot decodes at the required 1920x1080. Use it as the exact current runtime structure capture.
- Source layout is explicitly built around a 1920x1080 reference panel.
- Current screenshot shows the left rules panel, bottom-center action stack, and right leaderboard region on the full canonical capture.

## Full-Screen Structure

- `daily_climb_canvas`
  - Bounds: x 0, y 0, w 1920, h 1080.
  - Ownership: reference canvas only.

- `scene_background`
  - Bounds: full canvas.
  - Current source uses the main-menu reference sky background.
  - Visual target: full-screen fantasy scene/background continuing the approved main-menu visual family.
  - Ownership: generated scene/background only. It must not contain baked top title, buttons, left rules panel, action stack, leaderboard shell, row content, labels, values, or avatars.

- `dark_scrim`
  - Bounds: full canvas.
  - Source opacity: black overlay around 0.28.
  - Purpose: keeps foreground fantasy panels readable.
  - Ownership: generated/reference treatment or runtime overlay, not part of foreground control art.

## Top Navigation And Title

- `back_button`
  - Bounds: x 42, y 42, w 132, h 50.
  - Text: `BACK`.
  - Family: compact neutral/back button treatment.
  - Required states: normal, hover, pressed, disabled.
  - Ownership: real visible button with generated state plates and runtime/localizable label.
  - Constraint: keep it in the upper-left corner, separate from the title group.

- `title_group`
  - Bounds: x 500, y 54, w 920, h 132.
  - Contents:
    - Main title `DAILY CHALLENGE`, centered, source font black 54.
    - Subtitle `One seed. One attempt. Same puzzle for everyone.`, centered, source font bold 18, 8 px below title.
  - Ownership: runtime/localizable text. Do not bake into runtime art unless explicitly approved as display art in a later intake.
  - Visual target: ornate fantasy title treatment matching the main-menu family while keeping the same centered top placement.

## Left Rules Panel

- `rules_panel_shell`
  - Bounds: x 42, y 246, w 487, h 726.
  - Source shell asset reference size: 487x726.
  - Content padding: left 32, top 34, right 32, bottom 30.
  - Visual target: large left fantasy panel with thick carved frame, dark interior, green/gold accents, and readable data-card wells.
  - Ownership: generated-shell. No labels, values, or rule text baked into runtime-intended shell art.

- `rules_panel_inner_content`
  - Nominal bounds: x 74, y 280, w 423, h 662.
  - Layout: vertical stack.
  - Ownership: runtime text, generated nested card shells, and runtime list content.

- `rules_panel_title`
  - Text: `TODAY'S RULES`.
  - Position: top of rules panel inner content.
  - Ownership: runtime/localizable text.

- `rules_panel_divider`
  - Position: below rules title with 8 px top gap and 10 px bottom gap.
  - Visual: thin divider strip.
  - Ownership: generated/static shell or runtime border; no text.

- `status_card`
  - Position: below divider.
  - Shell: row shell card with 16 px horizontal and 12 px vertical padding.
  - Text examples:
    - `Backend unavailable.`
    - `Loading Daily Challenge...`
    - `Your Daily Challenge run is in progress. Continue to resume it.`
    - `Today's Daily Challenge is already completed.`
    - `Your Daily attempt is already active. Resume from your saved Daily run.`
    - `Everyone gets the same seed, hero, and rules. One attempt only.`
  - Ownership: generated card shell plus runtime/live status text.
  - Constraint: allow wrapping without clipping.

- `info_card_grid`
  - Position: below status card.
  - Layout: two rows, two columns, 7 px horizontal gap, 10 to 12 px vertical gaps.
  - Cards:
    - Hero.
    - Difficulty.
    - Seed Quality.
    - Reward.
  - Shell: row shell card for each card, 14 px left/right, 10 px top, 12 px bottom padding.
  - Ownership: generated card shells; labels and values runtime/localizable/live.
  - Value examples:
    - Hero: `Forsen`, fallback `Unknown Hero`, or hero ID.
    - Difficulty: `Easy`, `Medium`, `Hard`, `Very Hard`, `Impossible`, or `--`.
    - Seed Quality: `54 / 100` or `--`.
    - Reward: `25 Chad Coupons` or `--`.

- `rule_stack_header`
  - Text: `Rule Stack`.
  - Position: below info cards with 14 px top gap and 8 px bottom gap.
  - Ownership: runtime/localizable text.

- `rule_stack_scroll_area`
  - Position: fills remaining lower portion of rules panel.
  - Behavior: vertical scroll box with collapsed scrollbar.
  - Ownership: runtime list content.
  - Empty state: one row shell containing `No special rules are published for this Daily yet.`

- `rule_row`
  - Repeats for each daily rule.
  - Shell: row shell card with 16 px horizontal and 12 px vertical padding.
  - Vertical gap: 10 px.
  - Content:
    - Rule label, source font bold 18.
    - Rule description, source font regular 15, 4 px below label, wraps.
  - Ownership: generated row shell plus runtime rule label and description.

## Center Action Stack

- `action_stack_shell`
  - Bounds: x 738, y 650, w 445, h 311.
  - Source shell asset reference size: 445x311.
  - Visual target: central ornate CTA stack frame matching main menu, anchored low center.
  - Ownership: generated-shell only.

- `start_challenge_button`
  - Bounds inside canvas: x 766, y 671, w 388, h 92.
  - Bounds inside action stack: x 28, y 21, w 388, h 92.
  - Text variants:
    - `START CHALLENGE`.
    - `STARTING...`.
    - `RUN IN PROGRESS`.
    - `ATTEMPT USED`.
  - Enabled when a valid challenge exists, no attempt has started or completed, and no start request is in flight.
  - Required states: normal, hover, pressed, disabled.
  - Ownership: real visible button with generated wide state plates and runtime/live label.

- `continue_challenge_button`
  - Bounds inside canvas: x 766, y 768, w 388, h 97.
  - Bounds inside action stack: x 28, y 118, w 388, h 97.
  - Text: `CONTINUE CHALLENGE`.
  - Enabled only when a compatible in-progress daily save exists.
  - Required states: normal, hover, pressed, disabled.
  - Ownership: real visible button with generated wide state plates and runtime/localizable label.

- `action_note_text_well`
  - Bounds inside canvas: x 784, y 880, w 353, h 58.
  - Bounds inside action stack: x 46, y 230, w 353, h 58.
  - Text: `Daily Climb is always Solo and scored on a separate global leaderboard.`
  - Ownership: runtime/localizable text. Do not bake into action stack shell.
  - Constraint: centered, wraps cleanly within the text well.

## Right Leaderboard Assembly

- `leaderboard_assembly`
  - Bounds: x 1420, y 228, w 458, h 787.
  - Contains an offset right panel shell and the embedded `ST66LeaderboardPanel` in daily reference mirror mode.
  - Visual target: right-side main-menu leaderboard family, preserving the source placement even though the current screenshot bitmap crops most of it.

- `leaderboard_shell`
  - Bounds: x 1420, y 306, w 458, h 709.
  - Source shell asset reference size: 458x709.
  - Ownership: generated-shell. It must not bake live entries, avatars, ranks, scores, or labels into runtime-intended shell art.

- `leaderboard_widget_well`
  - Bounds: x 1432, y 320, w 434, h 667.
  - Source padding within assembly: left/right 12, top 92, bottom 28.
  - Ownership: runtime leaderboard component content.

- `daily_leaderboard_title`
  - Text: `DAILY LEADERBOARD`.
  - Mode: visible because DailyChallengeMode is true.
  - Ownership: runtime/localizable text.

- `standard_leaderboard_filters`
  - Standard weekly/all-time tabs, party/difficulty dropdowns, score/speedrun toggles, and global/friends/streamers filter buttons exist in the shared leaderboard component, but are collapsed in DailyChallengeMode.
  - Reference generation should not show these collapsed standard filters on the Daily Climb screen.
  - Future non-daily variants would require separate references.

- `leaderboard_header_row`
  - Columns:
    - Left spacer/rank/avatar region.
    - `NAME`.
    - `SCORE` for DailyChallengeMode.
  - Ownership: runtime/localizable header labels.

- `leaderboard_entry_list`
  - Behavior: vertical scroll area with collapsed scrollbar.
  - Visible count target: up to 10 leaderboard entries.
  - Empty state: `No leaderboard data yet`.
  - Ownership: runtime/live rows.

- `leaderboard_entry_row`
  - Repeated row content:
    - Favorite star button, 22x22 in reference mirror mode.
    - Rank text like `#1`.
    - Avatar frame 56x56 with 40x40 portrait inset.
    - Display name, ellipsized if too long.
    - Score text right aligned, 82 px well.
  - Row interactions:
    - Favorite button can toggle favorite state when favoritable.
    - Row can open a run summary when local player or run summary data exists.
  - Required row states: normal, hover, selected/local-player highlight, disabled/non-clickable.
  - Required favorite states: normal, hover, pressed, favorited, disabled/hidden.
  - Ownership: row hover backing may be generated/runtime tint; rank, star state, avatar, display name, and score are runtime/live content.

## Button And Control State Requirements

- `back_button`: normal, hover, pressed, disabled.
- `start_challenge_button`: normal, hover, pressed, disabled, loading label variant.
- `continue_challenge_button`: normal, hover, pressed, disabled.
- `leaderboard_entry_row`: normal, hover, local-player/selected highlight, disabled.
- `leaderboard_favorite_button`: normal, hover, pressed, favorited, disabled/hidden.
- Collapsed standard leaderboard controls must not be shown in the Daily Climb canonical reference.

## Live Runtime Ownership

- Top title and subtitle are runtime/localizable text.
- Back, Start, Continue, and all leaderboard labels are runtime/localizable text.
- Daily status text is live backend state.
- Hero, difficulty, seed quality, reward, challenge attempt state, and continue availability are live values.
- Rule stack row count, labels, and descriptions are live backend data.
- Leaderboard entry count, ranks, avatars, names, scores, favorite state, local-player highlight, and clickable state are live data.
- Avatar and portrait images are runtime-owned image wells. The generated reference may show representative placeholders only.
- Scroll positions for rules and leaderboard are runtime-owned.
- Generated runtime art later must be limited to the scene plate, panel shells, card shells, row shells, button state plates, avatar frames, and decorative trim.

## Required Variants

- Challenge loading/no challenge: status card shows loading or backend message, info values show `--`, start disabled.
- Challenge ready and not started: Start button enabled with `START CHALLENGE`, Continue disabled unless a valid save exists.
- Start request in flight: Start label `STARTING...`, Start disabled.
- Attempt already active without compatible save: Start label `RUN IN PROGRESS`, Start disabled, Continue disabled.
- Attempt already active with compatible save: status text says run is in progress, Continue enabled.
- Attempt completed: Start label `ATTEMPT USED`, Start disabled.
- Rule stack with zero rules.
- Rule stack with several wrapped rule rows.
- Leaderboard empty.
- Leaderboard populated with up to 10 rows, including long names, avatar placeholders, score values, favorited and non-favorited rows, and local-player highlight if available.

## Reference Generation Constraints

- Preserve the full-screen hierarchy exactly: upper-left back button, centered title group, left rules panel, low-center action stack, right leaderboard assembly.
- Keep the left panel, action stack, and right leaderboard at the source positions and proportions listed above.
- Restyle the screen to match the canonical main-menu fantasy language: dramatic scene background, dark carved panels, bronze/gold trim, jewel-like CTA plates, and readable fantasy typography.
- Do not add a modal frame around the full screen.
- Do not move the action stack upward into the title area or merge it with the rules panel.
- Do not omit the right leaderboard assembly just because the inspected current screenshot bitmap crops it.
- Do not show collapsed standard leaderboard filters in the Daily Climb reference.
- The full generated reference may include representative readable text for visual planning, but runtime-derived art must be text-free except approved icon/display art.
- Do not bake live/localizable labels, values, rules, backend status, leaderboard rows, names, ranks, scores, favorites, avatars, portraits, or button labels into runtime-intended art.
- Leave enough room for long hero names, long rule descriptions, long status messages, long leaderboard names, and large score values without clipping.
- Later sprite work must keep scene/background plate, foreground shell rects, visible control rects, and live-content rects distinct.
- Bad generated structure or contaminated runtime-owned regions should be fixed by regenerating the screen-specific reference, not by manual pixel repair.
