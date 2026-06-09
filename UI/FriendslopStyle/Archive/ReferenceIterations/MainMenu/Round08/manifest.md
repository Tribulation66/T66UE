# FriendslopStyle Main Menu Reference - Round07 Current

Created: 2026-06-06

Purpose: update the active Main Menu reference before the next full
FriendslopStyle implementation iteration. This pass preserves the established
top bar, left social panel, center CTA stack, right leaderboard panel, dark/red/
green/yellow palette, and overall composition while correcting two approved
reference problems:

- the background statue now reads as soft golden rubber/vinyl rather than stone;
- the title clearly reads `CHADPOCALYPSE` without the malformed mark between
  `A` and `D`.

This is not runtime UI chrome and must not be imported as production button,
panel, title, text, icon, or data art. Runtime implementation still uses
generated blank plates plus live Slate text/data/icons.

## Archive

- Previous authoritative `Current` reference archived to:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round07`

## Reference Inputs

- Previous authoritative reference:
  `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round07\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- User-approved correction notes from 2026-06-06:
  - static rubber statue background;
  - corrected `CHADPOCALYPSE` spelling/artifact;
  - keep layout and UI composition otherwise unchanged.

## Process

- One fresh local `codex exec` worker.
- Previous current reference was attached through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, browser screenshot,
  runtime UI edit, source code edit, Unreal import, old generated-image folder,
  or manual pixel repair.
- Worker request explicitly banned copying images created before worker start
  time.
- Worker final message: `IMAGE_SAVED`.

## Final Image

| Direction | PNG | Prompt | Worker | SHA-256 |
| --- | --- | --- | --- | --- |
| Static rubber statue background and fixed title | `main_menu_reference_02_rubber_statue_title_fixed_cli.png` | `prompt.md` | `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass22_workers\reference_update` | `CFFC37254DAF4E62EB286439625B1E7FE369A3E75A06357090536EB4B7256C99` |

Final image dimensions:

- `1672x941`.

## Visual QA

Visual QA below is Codex's direct inspection of the generated raster reference;
final art-direction sign-off remains with the user.

- PASS: title reads `CHADPOCALYPSE`.
- PASS: statue/background now reads as golden rubber/vinyl with a sun/fire halo.
- PASS: same broad Main Menu composition remains recognizable.
- PASS: secondary CTA reads dark/black rather than purple.
- PASS: invite action accents read green.
- PASS: ticket badge does not include the previously called-out odd yellow
  bracket lines.
- Caveat: small generated text remains reference-only and must stay live/
  localizable in runtime implementation.

## Token Notes

- Codex CLI image-worker token line from `last_message.txt`: 202,512.
- Claude token line inside worker final message: 637,361.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus one separate local Codex CLI worker using account-backed built-in image generation.
Matches declared process: YES
Evidence: archived prior Current folder, final PNG, prompt file, worker request/last-message/log files, hash file, and direct visual inspection.
```
