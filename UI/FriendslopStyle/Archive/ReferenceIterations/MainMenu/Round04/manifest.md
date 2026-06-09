# FriendslopStyle Main Menu References - Round04

Created: 2026-06-05

Purpose: internal visual-direction board for a bouncy/rubber `FriendslopStyle` direction. These are not runtime UI chrome assets and should not be imported as production button/panel art. Text remains expected to be live/localizable in implementation.

Baseline layout reference:

- `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`

Process:

- One fresh local `codex exec` worker per image.
- Baseline screenshot attached to each worker through `--image`.
- Account-backed built-in Codex image generation only.
- No `OPENAI_API_KEY`, OpenAI API script, web image URL, Unreal import, runtime UI edit, or source code edit.
- Worker requests explicitly banned copying images created before worker start time.
- All five workers exited before this manifest was written.
- Claude independent answer before generation: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopBouncyRound04Generate\20260605T083017-IndependentAnswer-pass1\claude_review_pass1.md`

## Final Images

| # | Direction | PNG | Prompt | Worker | SHA-256 | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 01 | Midnight cosmic rubber | `main_menu_reference_01_midnight_cosmic_rubber_cli.png` | `prompts/01_midnight_cosmic_rubber.prompt.md` | `workers/01_midnight_cosmic_rubber` | `AAF9DD6EED7D844EDDF26C5BD7F8F530331C7CE534E6284B13B59F11BC2582A4` | Cool star/statue atmosphere with glossy midnight rubber, gel capsules, and luminous rounded chrome. |
| 02 | Post-apocalyptic inflatable | `main_menu_reference_02_post_apocalyptic_inflatable_cli.png` | `prompts/02_post_apocalyptic_inflatable.prompt.md` | `workers/02_post_apocalyptic_inflatable` | `A9DC97A2BCEEF615AD8B054180CA2ABCF1091A143F914210E757D6777D79694F` | Survival/hazard mood over clean inflatable rubber UI; dust/hazard marks read as decals rather than base grit. |
| 03 | Bloody horror-comedy rubber | `main_menu_reference_03_bloody_horror_comedy_rubber_cli.png` | `prompts/03_bloody_horror_comedy_rubber.prompt.md` | `workers/03_bloody_horror_comedy_rubber` | `07A9EFCA0AFBA27FF80D12728068E414A7C97A1F34A83983E979D25C68525A43` | Glossy black/red rubber, stylized splat decals, horror-comedy mood without realistic gore. |
| 04 | Dungeon slime rubber | `main_menu_reference_04_dungeon_slime_rubber_cli.png` | `prompts/04_dungeon_slime_rubber.prompt.md` | `workers/04_dungeon_slime_rubber` | `1F88153497BB453A3DF36F1CC2727243376E80D55D4BA3F6B0012CB41AC352E0` | Dungeon atmosphere as polished slime/rubber, not stone/wood/metal fantasy bevel. |
| 05 | Isekai guild rubber | `main_menu_reference_05_isekai_guild_rubber_cli.png` | `prompts/05_isekai_guild_rubber.prompt.md` | `workers/05_isekai_guild_rubber` | `1AB1462E9201AA0CD7D2A50AE893EC487FB5A037226DADBD9C923A24C998D78C` | Guild/quest motifs as inflated rubber badges, seals, and toy-like heraldry instead of parchment/wood. |

Contact sheet:

- `main_menu_reference_round04_contact_sheet.png`

Final image dimensions:

- All five final PNGs are `1672x941`, the closest saved 16:9-ish output produced by the built-in image tool in these CLI workers.

## Visual QA

Layout/content gate:

- PASS for vision-board fidelity. All final images preserve the recognizable current main-menu structure: top navigation, left social/account panel, center `Chadpocalypse` title plus subtitle and three CTA buttons, and right leaderboard panel.
- This is not pixel fidelity. The acceptance bar for Round04 is recognizable region/content preservation plus style exploration, not exact Slate layout reproduction.

Bouncy identity gate:

- PASS. All five images keep rounded/circular/pill geometry, glossy rubber/plastic surfaces, and soft inflated UI forms.
- Static images cannot prove click bounce, hover wobble, or jiggle. Motion remains a future implementation note, not verified here.

Unique theme gate:

- PASS. The contact sheet shows five distinct theme treatments rather than one reused chrome recolored five ways.
- Final SHA-256 hashes are all unique.

Grit/material guardrail:

- PASS with caveat. The gritty themes remain mostly glossy/HD, but post-apocalyptic and dungeon variants should be watched in future passes so they do not drift into real rust/stone/wood as implementation targets.

Title/text gate:

- PASS for visual-direction use. Final images use the `Chadpocalypse` title region and the major current main-menu labels.
- Caveat: imagegen text is not production text. Runtime implementation must keep all labels as live Slate text.

## Token Notes

Codex CLI image-worker token lines from stderr logs:

| Worker log | Tokens |
| --- | ---: |
| `01_midnight_cosmic_rubber.stderr.log` | 65,248 |
| `02_post_apocalyptic_inflatable.stderr.log` | 60,739 |
| `03_bloody_horror_comedy_rubber.stderr.log` | 65,137 |
| `04_dungeon_slime_rubber.stderr.log` | 63,059 |
| `05_isekai_guild_rubber.stderr.log` | 64,728 |

Total across the five CLI workers: 318,911.

Claude validation tokens:

- Independent answer: 151,438.

## PPF Close

```text
PPF CLOSE
Process used: AGENTS.md Image generation row plus separate local Codex CLI workers using account-backed built-in image generation.
Matches declared process: YES
Evidence: final PNGs, prompt files, worker request/last-message logs, unique hashes, contact sheet, all workers exited, and this manifest.
```

```text
MECHANISM CLOSE
Mechanism: bouncy identity
Status: PRESENT
Evidence: contact sheet shows glossy rubber/plastic, rounded/pill/circular forms, and inflated tactile UI surfaces in all five images.
Discriminator test: not paper, wood, leather, rust, stone, or flat-panel UI as the base material.
Reported status: FULL for static visual direction; motion remains unverified.

Mechanism: unique variation
Status: PRESENT
Evidence: five prompt files and contact sheet show midnight, post-apocalyptic, bloody, dungeon, and isekai guild as distinct reinterpretations of the rubber identity.
Discriminator test: not the Round03 Fall Guys chrome recolored five times.
Reported status: FULL

Mechanism: layout preservation
Status: PRESENT
Evidence: contact sheet shows the top bar, left social panel, center title/CTA stack, and right leaderboard in all five images.
Discriminator test: not arbitrary new menu layouts.
Reported status: FULL

Mechanism: worker isolation
Status: PRESENT
Evidence: five separate `codex exec` worker entries under `workers/`; process check shows all PIDs exited; each last message reports `IMAGE_SAVED`.
Discriminator test: not one shared chat context generating sequential variants.
Reported status: FULL
```
