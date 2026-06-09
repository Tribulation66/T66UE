# FriendslopStyle Main Menu Reference - Round08 Current

Created: 2026-06-07

Purpose: update the active Main Menu reference before the next full
FriendslopStyle implementation iteration. This pass keeps the established top
bar, left social panel, center title/subtitle, center CTA stack, golden rubber
statue background, dark/red/green/yellow palette, and overall composition while
making one approved structural reference change:

- the right leaderboard column now matches the left social panel width;
- the three leaderboard filter toggles (`GLOBAL`, `SOCIAL`, `STREAMERS`) move
  from the vertical side rail into a separate small panel above the leaderboard;
- the leaderboard panel below that filter panel is shorter to make room.

This is not runtime UI chrome and must not be imported as production button,
panel, title, text, icon, or data art. Runtime implementation still uses
generated blank plates plus live Slate text/data/icons.

## Archive

- Previous authoritative `Current` reference archived to:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round08`

## Reference Inputs

- Previous authoritative reference:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round08\main_menu_reference_02_rubber_statue_title_fixed_cli.png`
- User-approved correction notes from 2026-06-07:
  - keep everything else the same;
  - make the right panel the same width as the left panel;
  - move the global/social/streamers toggles into their own panel above the
    leaderboard;
  - shorten the leaderboard panel below to fit the new toggle panel.

## Process

- One fresh local `codex exec` worker.
- Previous current reference was attached through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, browser screenshot,
  runtime UI edit, source code edit, Unreal import, old generated-image folder,
  cached candidate, or manual pixel repair.
- Worker request explicitly banned copying old generated images or using the
  input reference as a runtime source.
- Worker final message: `IMAGE_SAVED`.
- A first launcher wrapper failed before the Codex CLI process started due to a
  local PowerShell parser error; this is recorded in the worker folder and was
  not an image generation attempt.

## Final Image

| Direction | PNG | Prompt | Worker | SHA-256 |
| --- | --- | --- | --- | --- |
| Equal-width right panel with filter panel above leaderboard | `main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png` | `prompt.md` | `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass23_workers\reference_equal_width_right_panel` | `C6EB25734BEADE73FDD6CA740BD927DE5E5AEE809147752F89875AF70BFF1374` |

Final image dimensions:

- `1672x941`.

## Visual QA

Visual QA below is Codex's direct inspection of the generated raster reference;
final art-direction sign-off remains with the user.

- PASS: right leaderboard column is wider and visually matches the left panel
  width class.
- PASS: `GLOBAL`, `SOCIAL`, and `STREAMERS` are now in a separate top filter
  panel above the leaderboard.
- PASS: the old vertical filter rail is removed.
- PASS: leaderboard panel below is shorter to fit the new filter panel.
- PASS: top bar, left social panel, center title/CTA stack, and golden rubber
  background remain recognizable from the prior current reference.
- Caveat: small generated text remains reference-only and must stay live/
  localizable in runtime implementation.

## Token Notes

- Codex CLI image-worker token line from `last_message.txt`: 153,081.
- Claude token line inside worker final message: 306,890.
- Built-in imagegen token count was unavailable/not exposed.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus one separate local Codex CLI worker using account-backed built-in image generation.
Matches declared process: YES
Evidence: archived prior Current folder, final PNG, prompt file, worker request/last-message/log files, hash file, and direct visual inspection.
```
