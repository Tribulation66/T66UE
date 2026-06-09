# FriendslopStyle Main Menu Reference - Round09 Current

Created: 2026-06-07

Purpose: update the active Main Menu reference before the next full FriendslopStyle implementation iteration. This pass preserves the Round08 equal-width side panel direction and applies the user's three requested corrections:

- right-side UI foreground layering: the statue/background must sit behind the leaderboard and icon filter panels;
- the top leaderboard filter strip uses icons only instead of `GLOBAL`, `SOCIAL`, and `STREAMERS` text labels;
- the topbar coupon badge reads as a classic fair/carnival ticket/coupon icon.

This is not runtime UI chrome and must not be imported as production button, panel, title, text, icon, or data art. Runtime implementation still uses generated blank plates plus live Slate text/data/icons.

## Archive

- Previous authoritative `Current` reference archived to:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round09`

## Reference Inputs

- Previous authoritative reference:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round09\main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png`
- User correction notes from 2026-06-07:
  - bring right leaderboard/toggle panels in front of the statue/background;
  - replace Global/Social/Streamers text with icon controls;
  - make the coupon badge a classic fair-ticket/coupon icon.

## Process

- One fresh local `codex exec` worker.
- Previous current reference was attached through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, browser screenshot, runtime UI edit, source code edit, Unreal import, old generated-image folder, cached candidate, or manual pixel repair.
- Worker request explicitly banned copying old generated images or using the input reference as a runtime source.
- Worker final message: `IMAGE_SAVED`.

## Final Image

| Direction | PNG | Prompt | Worker | SHA-256 |
| --- | --- | --- | --- | --- |
| Right panels foreground, icon filters, fair-ticket coupon badge | `main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png` | `prompt.md` | `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_workers\reference_zorder_icons_coupon` | `D3EBD71D110E3E2862D4BC08934F13E2DAF4B8C91B3CF73B0A9FBC4F6CF8D16A` |

Final image dimensions:

- `1672x941`.

## Visual QA

Visual QA below is Codex's direct inspection of the generated raster reference; final art-direction sign-off remains with the user.

- PASS: right leaderboard and icon-filter panels are foreground UI over the statue/background.
- PASS: the top right filter controls are icon-only: globe, friends/people, and broadcast/streamer.
- PASS: the topbar coupon badge reads as a notched ticket/coupon shape.
- PASS: top bar, left social panel, center title/CTA stack, golden rubber background, and equal-width right panel direction remain recognizable from Round08.
- Caveat: generated labels elsewhere remain reference-only and must stay live/localizable in runtime implementation.

## Token Notes

- Built-in imagegen token count was unavailable/not exposed.
- Worker `last_message.txt` reports Codex/Claude token lines; use the worker record for exact exposed values.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus one separate local Codex CLI worker using account-backed built-in image generation.
Matches declared process: YES
Evidence: archived prior Current folder, final PNG, prompt file, worker request/last-message/log files, hash file, and direct visual inspection.
```
