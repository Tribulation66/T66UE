# FriendslopStyle Main Menu Reference - Round05

Created: 2026-06-05

Purpose: focused internal visual-direction target after Round04 became too complicated. This pass returns to simple bouncy/Fall-Guys-like UI element shapes, while using the original main-menu statue/fire-eclipse/starfield background and the current T66 palette.

This is not runtime UI chrome and should not be imported as production button/panel art. Text remains expected to be live/localizable in implementation.

Reference inputs:

- Layout screenshot: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- Original background art: `C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens\MainMenu\ScreenArt\mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_1920.png`
- Structural inventory: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`
- Palette source: `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md`

Process:

- One fresh local `codex exec` worker.
- Both the layout screenshot and original background image were attached through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, Unreal import, runtime UI edit, or source code edit.
- Worker request explicitly banned copying images created before worker start time.
- Worker PID was verified no longer running before this manifest was written.
- Claude independent answer before generation: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopSimpleBouncyRound05Generate\20260605T085202-IndependentAnswer-pass1\claude_review_pass1.md`

## Final Image

| Direction | PNG | Prompt | Worker | SHA-256 |
| --- | --- | --- | --- | --- |
| Simple bouncy UI, current T66 palette | `main_menu_reference_01_simple_fallguys_t66_palette_cli.png` | `prompts/01_simple_fallguys_t66_palette.prompt.md` | `workers/01_simple_fallguys_t66_palette` | `D53BB3514D15F71387421DE322DB997C977DC72CFCF042E0D6EDA4AC5D73C6E3` |

Final image dimensions:

- `1672x941`, the closest saved 16:9-ish output produced by the built-in image tool in the CLI worker.

## Visual QA

Background gate:

- PASS. The generated reference keeps the starfield, golden statue head, fiery eclipse/ring, and dark reflective mood from the original background.

Duplicate-title gate:

- PASS. The result uses a single visible `CHADPOCALYPSE` title/subtitle lockup from the background/title area; it does not add a second competing title.

UI simplicity gate:

- PASS. The main controls use simple rounded/pill shapes, circular icon buttons, glossy soft borders, and clean inflated panels rather than the more complicated Round04 themed chrome.

Palette gate:

- PASS. The dominant UI palette stays close to current T66: dark fills, red selected/action states, green status accents, soft off-white text, with cyan/yellow as small accents only.

Layout/content gate:

- PASS for vision-board fidelity. The recognizable current main-menu structure remains: top bar, left social/account panel, center CTA stack, and right leaderboard.

Fall Guys IP guardrail:

- PASS. No Fall Guys characters, bean mascots, logos, or rainbow candy palette.

Text caveat:

- Major labels are readable in this mockup. Runtime implementation must still keep all title/labels as live Slate text.

## Token Notes

Codex CLI image-worker token line:

- `01_simple_fallguys_t66_palette.stderr.log`: 75,538.

Claude validation tokens:

- Independent answer: 113,782.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus one separate local Codex CLI worker using account-backed built-in image generation.
Matches declared process: YES
Evidence: final PNG, prompt file, worker request/last-message/log files, worker PID no longer running, direct visual inspection, and this manifest.
```

```text
MECHANISM CLOSE
Mechanism: original background preservation
Status: PRESENT
Evidence: final image keeps starfield, fire eclipse ring, golden statue head, and reflective dark water mood.
Discriminator test: not a generic generated background or Round04 theme background.
Reported status: FULL

Mechanism: simple bouncy UI
Status: PRESENT
Evidence: final image uses simple pill buttons, rounded panels, circular icon buttons, glossy soft borders, and inflated tactile control shapes.
Discriminator test: not complex dungeon/post-apoc/guild/bloody chrome.
Reported status: FULL for static visual direction; motion remains unverified.

Mechanism: current T66 palette
Status: PRESENT
Evidence: dark fills, red selected/action accents, green status accents, off-white text, limited cyan/yellow accents.
Discriminator test: not Fall Guys rainbow/candy palette.
Reported status: FULL
```
