# FriendslopStyle Main Menu References - Round03

Created: 2026-06-05

Purpose: internal visual-direction board for an alternative `FriendslopStyle` UI direction. These are not runtime UI chrome assets and should not be imported as production button/panel art. Text remains expected to be live/localizable in implementation.

Baseline layout reference:

- `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`

Process:

- One fresh local `codex exec` worker per final image.
- Baseline screenshot attached to each worker through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, Unreal import, runtime UI edit, or source code edit.
- Claude independent answer before generation: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleRound03Generate\20260605T065630-IndependentAnswer-pass1\claude_review_pass1.md`

## Final Images

| # | Direction | PNG | Prompt | Final worker | SHA-256 | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 01 | PEAK cozy chunky | `main_menu_reference_01_peak_cozy_chunky_cli.png` | `prompts/01_peak_cozy_chunky.prompt.md` | `workers/01_peak_cozy_chunky_regen1` | `209951900E4AB6482379ECDECD5A9E9EDCADF19626C1BE2BD67698B6A680B7DA` | Regenerated once after the first version drifted into casino props. Final version uses cozy outdoor chunky panels and keeps the three-region layout. |
| 02 | Schedule I scrappy utility | `main_menu_reference_02_schedule_scrappy_utility_cli.png` | `prompts/02_schedule_scrappy_utility.prompt.md` | `workers/02_schedule_scrappy_utility` | `D11AE84AFF1CF038FAFE71B8DAE1FC731C53A7B6D0AC177B88D0605DFA4FCF91` | Receipt-paper, dark-green, taped-label, small-business utility direction. |
| 03 | Fall Guys bouncy party | `main_menu_reference_03_fallguys_bouncy_party_cli.png` | `prompts/03_fallguys_bouncy_party.prompt.md` | `workers/03_fallguys_bouncy_party` | `9F8FB0E861A1C9873431F9BB4462B412508AEB9525B78008B644C3F7CC8E01BD` | Bright candy party-game direction with soft 3D pills and bubble typography. |
| 04 | Gamble With Your Friends tabletop social | `main_menu_reference_04_gamble_friends_tabletop_cli.png` | `prompts/04_gamble_friends_tabletop.prompt.md` | `workers/04_gamble_friends_tabletop_regen1` | `C6AE04E61FD0C7DA54972065759639B4423835523673B85A5149BE309F4C3548` | Regenerated once after the first worker copied candidate 01's generated image. Final version uses casino-night table/card/chip language and is unique. |
| 05 | R.E.P.O. scrap utility | `main_menu_reference_05_repo_scrap_utility_cli.png` | `prompts/05_repo_scrap_utility.prompt.md` | `workers/05_repo_scrap_utility` | `935734CEEF845041C988150F6A54F744D48C64C0BF118B315A177E8A861AD823` | Dark toy-industrial, acid-yellow/cyan scrap-utility direction. |

Contact sheet:

- `main_menu_reference_round03_contact_sheet.png`

Final image dimensions:

- All five final PNGs are `1672x941`, the closest saved 16:9-ish output produced by the built-in image tool in these CLI workers.

## Rejected Attempts

- `workers/04_gamble_friends_tabletop`: rejected because it copied candidate 01's generated image from the shared generated-images directory. Evidence: initial candidate 04 hash matched candidate 01 exactly.
- Initial `workers/01_peak_cozy_chunky`: rejected by visual QA because it drifted into casino/tabletop props rather than PEAK/cozy-adventure UI.

The final set uses replacement workers for both rejected candidates.

## Visual QA

Layout/content gate:

- PASS for vision-board fidelity. All final images preserve the recognizable current main-menu structure: top navigation, left social/account panel, center `Chadpocalypse` title plus subtitle and three CTA buttons, and right leaderboard panel.
- This is not pixel fidelity. The acceptance bar for Round03 is recognizable region/content preservation plus style exploration, not exact Slate layout reproduction.

Title gate:

- PASS for visual-direction usage. The final set uses `Chadpocalypse` as the title region text.
- Caveat: imagegen text may contain minor rendering imperfections. Runtime implementation must keep title and labels as live Slate text.

Uniqueness gate:

- PASS after two regenerations. All five final PNGs have unique SHA-256 hashes and visibly different material systems, palettes, and title treatments.

Style-transfer gate:

- 01 PEAK: PASS. Cozy outdoor, chunky patch/buttons, soft black plates, friendly cooperative-adventure feel.
- 02 Schedule I: PASS. Paper/receipt panels, stamped green headers, taped-label utility feel.
- 03 Fall Guys: PASS. Candy-bright, rounded, soft 3D party-game UI language.
- 04 Gamble With Your Friends: PASS. Tabletop casino-night/social scorecard direction.
- 05 R.E.P.O.: PASS. Dark scrap-utility, acid-yellow/cyan status, toy-industrial direction.

## Token Notes

Codex CLI image-worker token lines from stderr logs:

| Worker log | Tokens |
| --- | ---: |
| `01_peak_cozy_chunky.stderr.log` | 66,311 |
| `01_peak_cozy_chunky_regen1.stderr.log` | 69,432 |
| `02_schedule_scrappy_utility.stderr.log` | 72,232 |
| `03_fallguys_bouncy_party.stderr.log` | 73,868 |
| `04_gamble_friends_tabletop.stderr.log` | 64,272 |
| `04_gamble_friends_tabletop_regen1.stderr.log` | 66,187 |
| `05_repo_scrap_utility.stderr.log` | 125,328 |

Total across all seven CLI workers, including rejected attempts: 537,630.

Total across the five final-image workers only: 407,047.

Claude validation tokens:

- Independent answer: 231,632.
- Cross-review: 174,779.
- Total: 406,411.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus separate local Codex CLI workers using account-backed built-in image generation.
Matches declared process: YES
Evidence: final PNGs, prompt files, worker request/last-message logs, unique hashes, contact sheet, and this manifest.
```

```text
MECHANISM CLOSE
Mechanism: layout preservation
Status: PRESENT
Evidence: contact sheet shows the top bar, left social panel, center title/CTA stack, and right leaderboard in all five images.
Discriminator test: not arbitrary new menu layouts; the same T66 main-menu regions remain recognizable.
Reported status: FULL

Mechanism: source-style transfer
Status: PRESENT
Evidence: five prompt files plus contact sheet show distinct PEAK, Schedule I, Fall Guys, Gamble With Your Friends, and R.E.P.O. visual-language directions.
Discriminator test: not one generic imagegen UI theme repeated five times.
Reported status: FULL

Mechanism: worker isolation
Status: PRESENT
Evidence: final images came from separate `codex exec` workers under `workers/`; rejected candidates were regenerated with fresh replacement workers.
Discriminator test: not one shared chat context generating sequential variants.
Reported status: FULL

Mechanism: uniqueness discrimination
Status: PRESENT
Evidence: duplicate candidate 04 was detected by hash and regenerated; PEAK drift was detected visually and regenerated; final five hashes are unique.
Discriminator test: repeated title arcs/generic plate reuse did not pass the final gate.
Reported status: FULL
```
