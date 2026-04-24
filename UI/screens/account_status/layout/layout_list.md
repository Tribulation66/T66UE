# Account Status Layout List

Reference gate target: generated screen-specific reference for the Account Status screen with the Overview tab visible.

Canvas: canonical `1920x1080`.

Current target screenshot:
- `C:\UE\T66\UI\screens\account_status\current\2026-04-24\current_runtime_1920x1080.png`
- The file decodes at the required `1920x1080`. Use it as the exact current runtime structure capture.

Required image-generation inputs:
- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current target screenshot listed above.
- This layout list.

Source audited:
- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp`
- `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.h`

## Structural Contract

- Preserve the current two-column dashboard hierarchy inside a full-width scrollable account frame.
- Current visible state is the `Overview` tab, not `History` or `Suspension`.
- The generated reference must restyle the flat/gray panels into the canonical main-menu fantasy material system while preserving the data-dashboard layout.
- Do not convert this into a marketing page, character screen, or single large modal.

## Canvas And Bounds

- `canvas`: full-screen reference canvas, `1920x1080`.
- `shared_topbar`: visible global frontend topbar at the top edge; current capture shows it across the full `1920x1080` canvas. The Account tab is active in the shared topbar.
- `topbar_clearance_band`: empty dark screen area between the shared topbar and account frame; current capture shows content beginning around `y 214`.
- `shared_back_button_region`: visible shared Back button on the left, current approximate bounds `x 27-226`, `y 239-301`. It floats over/near the top-left of the account frame and must remain a real navigation control.
- `account_outer_frame`: full-width frame below the topbar clearance, current approximate bounds `x 2-1461`, `y 214-913`. Preserve this as the main screen container.
- `tab_row`: centered row at the top of the account frame, current labels `OVERVIEW` and `HISTORY` visible; optional `SUSPENSION` tab appears when account restriction state exists.
- `account_scroll_viewport`: vertical scroll region inside the outer frame. Current capture shows a right-side scrollbar near `x 1436`, with the lower rows partially clipped by the capture bottom.
- `overview_left_column`: left half of active content; contains account standing and progress panels.
- `overview_right_column`: right half of active content; contains personal-best dropdowns and two leaderboard summary tables.

## Visible Regions In Current Overview State

- `global_topbar_chrome`: shared runtime/global UI chrome, not owned by this screen.
- `account_background_surface`: screen background under shared topbar. It should be dark and quiet, with fantasy texture only if it does not compete with the dense dashboard.
- `account_outer_shell`: generated full-width frame/backplate for the account content area.
- `tab_strip`: centered tabs above active content.
  - `overview_tab`: selected in current screenshot.
  - `history_tab`: normal/inactive in current screenshot.
  - `suspension_tab`: conditional; hidden when account has no restriction.
- `overview_content_shell`: active tab content area inside a vertical scrollbox.
- `status_panel`: upper-left panel with section heading `ACCOUNT STATUS`, standing value, help button, eligibility body, appeal status, optional reason, and optional reviewed-run button.
- `standing_help_popup`: collapsible info panel below the standing row, hidden in current screenshot.
- `progress_panel`: lower-left panel with section heading `ACCOUNT PROGRESS` and six progress rows.
- `personal_best_panel`: right-column panel containing two dropdown fields and two table blocks.
- `pb_mode_dropdown`: top-left dropdown in right column, current value `Personal Best`.
- `pb_party_size_dropdown`: top-right dropdown in right column, current value `Solo`.
- `highest_score_block`: first table block, title `Highest Score`, five difficulty rows.
- `best_speed_run_block`: second table block, title `Best Speed Run`, five difficulty rows; bottom is partly clipped in current capture because the account screen scrolls.
- `account_scrollbar`: vertical scrollbar at far right of the account frame.

## Progress Rows

Each progress row is a generated row shell plus live text/value/progress fill:

- `achievements_unlocked_row`: label `Achievements Unlocked`, current visible value `29/124`.
- `permanent_buffs_unlocked_row`: label `Permanent Buffs Unlocked`, current visible value `0/70`.
- `heroes_unlocked_row`: label `Heroes Unlocked`, current visible value `16/16`.
- `companions_unlocked_row`: label `Companions Unlocked`, current visible value `1/24`.
- `challenges_completed_row`: label `Challenges Completed`, current visible value `0/6`.
- `stages_beat_row`: label `Stages Beat`, current visible value `2/23`, partially near the bottom of the current capture.

Progress bars are runtime-owned fills inside generated trough/backplate art. The bar percentages and counts must remain live values.

## Personal Best Tables

Each table block has:

- `table_title`: runtime/localizable section title.
- `table_header_row`: generated header strip with live/localizable column labels.
- `difficulty_rows`: five row buttons in order `Easy`, `Medium`, `Hard`, `Very Hard`, `Impossible`.
- `difficulty_column`: runtime/localizable difficulty label.
- `hero_column`: runtime hero display name or empty.
- `date_column`: runtime date or empty.
- `rank_or_value_column`: runtime rank/value depending on view mode.
- `value_or_rank_column`: runtime score/time/rank depending on view mode.

Current screenshot shows no records, so value cells display `No Record` in the score/time column and empty hero/date/rank wells.

## History Tab Variant

The same outer frame and tab row remain. Active content changes to:

- `recent_runs_panel`: section heading `RECENT RUNS`.
- `history_subtitle`: runtime/localizable helper text `Click any run to open its saved summary.`
- `history_filter_bar`: four dropdown fields for Hero, Difficulty, Party Size, and Status.
- `history_column_header`: columns `HERO PLAYED`, `STATUS`, `DATE / TIME`, `DURATION`, `RUN`.
- `history_rows`: clickable row buttons with hero portrait well, hero name, companion/stage subline, completed/not-completed status, timestamp, duration, and run details.
- `empty_history_message`: runtime text for no runs or no matching filtered runs.

## Suspension Tab Variant

The `SUSPENSION` tab appears only when account restriction state exists. Active content changes to:

- `suspension_panel`: section heading `SUSPENSION`.
- `suspension_headline`: runtime/localizable `ACCOUNT SUSPENDED` or `ACCOUNT RESTRICTED`.
- `suspension_body`: runtime/localizable restriction explanation.
- `reason_panel`: generated row/panel shell with `REASON` label and runtime reason text.
- `appeal_status_text`: runtime/localizable appeal status.
- `appeal_status_message`: runtime success/error message after submit.
- `appeal_button`: opens the appeal editor.
- `appeal_editor_panel`: text-entry panel with multiline editable field, Submit Appeal button, and Cancel button.

## Controls And Required States

- `shared_back_button`: real shared navigation button.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `account_tab_button`: real tab buttons for Overview, History, and conditional Suspension.
  - Required states: `normal`, `hover`, `pressed`, `selected`, `disabled`.
- `standing_help_button`: small `?` button in status panel.
  - Required states: `normal`, `hover`, `pressed`, `disabled`, `selected/open`.
- `pb_mode_dropdown`: real combo button.
  - Required states: `normal`, `hover`, `pressed/open`, `disabled`, `focused`.
  - Menu entries need `normal`, `hover`, `pressed`, `selected`, `disabled`.
- `pb_party_size_dropdown`: real combo button with the same state requirements.
- `history_filter_dropdown`: real combo buttons for History tab filters.
  - Required states: `normal`, `hover`, `pressed/open`, `disabled`, `focused`.
  - Menu entries need `normal`, `hover`, `pressed`, `selected`, `disabled`.
- `personal_best_row_button`: clickable when a run summary slot exists; disabled/no-op when no record.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `history_row_button`: clickable when a saved run summary slot exists.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `view_reviewed_run_button`: conditional button in status panel.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `appeal_button`: conditional button in Suspension tab.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `submit_appeal_button`: enabled only when draft text is non-empty and submit is not in flight.
  - Required states: `normal`, `hover`, `pressed`, `disabled`, `loading`.
- `cancel_appeal_button`: neutral button in appeal editor.
  - Required states: `normal`, `hover`, `pressed`, `disabled`.
- `appeal_multiline_text_field`: editable text well.
  - Required states: `normal`, `focused`, `disabled`, `error`.
- `account_scrollbar`: vertical scrollbar.
  - Required states: `normal`, `hover`, `dragged`, `disabled`.

## Live Runtime Content And Ownership

- Shared topbar icons, labels, ticket count, and account portrait are shared runtime/global content.
- Back button label is runtime/localizable/shared.
- Tab labels are runtime/localizable.
- Account standing value is runtime state: `Good Standing`, `Suspended`, or `Restricted`.
- Status color is runtime state: success for eligible, danger for restricted.
- Eligibility body text, appeal status, restriction reason, and standing help copy are runtime/localizable.
- All progress labels, counts, bar percentages, and bar fill colors are runtime-owned.
- Dropdown selected values and dropdown menu entries are runtime/localizable.
- Difficulty labels, hero names, dates, ranks, scores, times, and `No Record` text are runtime-owned.
- Hero portraits and fallback hero initials in History rows are runtime-owned image/text wells.
- Recent run status, timestamps, durations, companion names, stage numbers, party size, and difficulty are runtime-owned.
- Appeal draft text, hint text, submit status, and loading text are runtime-owned.

## Reference Constraints

- Preserve the dense dashboard spacing and the two-column overview layout from the current screenshot.
- Restyle panels and controls to match the main-menu fantasy material language, but keep readable high-contrast interiors for data-heavy text.
- Do not bake runtime labels, values, names, dates, ranks, scores, progress fills, `No Record`, or appeal text into runtime-intended art.
- Do not turn the scrollbar, tabs, dropdowns, or table rows into fake visible art without real control states later.
- Keep row heights stable across normal, hover, pressed, disabled, and selected states.
- Keep dropdown menu shells sized so long localized values such as `All Difficulties` and longer hero names can fit.
- The current capture clips the lower portion of the scroll content; the generated reference should show the same top scroll position and acknowledge that additional content continues below.
- Full-screen reference output is an offline comparison target only. Later runtime work must use a UI-free background plate plus foreground component families.
