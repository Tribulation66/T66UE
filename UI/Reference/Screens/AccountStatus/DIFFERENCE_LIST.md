# AccountStatus Reference Pass

Reference: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatus.png`

Final packaged proof: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Proof\AccountStatus_final2_1672x941_2026-05-04.png`

## Visible Families

- Top navigation bar: shared frontend chrome, kept live and outside this screen pass.
- Account shell: dark wood fullscreen content frame.
- Tabs: Overview/History reference sliced plates with live Slate labels.
- Profile panel: parchment frame, live avatar/name/level/XP text, live progress meter.
- Account status panel: parchment frame, live standing, help button, body/appeal status text.
- Account progress panel: parchment frame, live owned/unlocked counts and progress meters.
- Personal-best filters: two dark dropdown fields with live selected values.
- Leaderboard tables: parchment panels with live difficulty rows, hero/date/rank/value columns.
- Scrollbar: AccountStatus-owned reference control sheet region.

## First-Pass Mismatches

- Runtime was using dark V16 panels and row shells instead of parchment content panels.
- AccountStatus was still routing pill buttons through Shared instead of target-owned assets.
- The screen lacked the dark wood content shell from the locked reference.
- Title/tabs were too low in the first runtime proof and then needed a target-only offset after the shell was restored.
- Leaderboard rows were over-framed as repeated ornate panels instead of ruled parchment table rows.
- Account progress showed an extra `Stages Beat` row not present in the locked reference.
- Text ink was too faint after parchment routing and needed stronger AccountStatus-local colors.

## Accepted Runtime Assets

- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Panels\accountstatus_panels_reference_scroll_paper_frame.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Buttons\accountstatus_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Buttons\accountstatus_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Buttons\accountstatus_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Buttons\accountstatus_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Buttons\accountstatus_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Progress\accountstatus_progress_reference_progress_meter_sheet.png`

## Resize Contracts

- Content shell: fixed target-owned AccountStatus brush used as the fullscreen frame.
- Parchment panels: AccountStatus-owned 9-slice via `accountstatus_panels_reference_scroll_paper_frame.png`.
- Buttons/tabs: AccountStatus-owned horizontal sliced plates, nearest filtering, zero brush margin, live text over the plate.
- Dropdowns: AccountStatus-owned field shell with live Slate value text and chevrons.
- Leaderboard rows: Slate ruled rows over parchment table panels, not stretched whole ornate row art.

## Verification

- Target compile: `Build.bat T66 Win64 Development -SingleFile=T66AccountStatusScreen.cpp` succeeded.
- Full staged cook/package: `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipShortcutRefresh` succeeded.
- Packaged capture: `Scripts\CaptureT66UIScreen.ps1 -Screen AccountStatus -ResX 1672 -ResY 941` produced the final proof listed above.

## Remaining Visual Differences

- Shared top bar still shows live ticket count (`10`) rather than the locked reference placeholder (`--`); this is outside AccountStatus and was left live.
- Live profile/avatar/save data differ from the locked placeholder values by design.
- The content shell and panel stack are close, but not a pixel-perfect match to the locked screenshot because the shared frontend top bar height differs from the locked image.
- Progress meters still use the shared helper implementation even though the target-owned meter sheet was copied; routing that sheet cleanly would require a helper-level API change outside the target source file.
