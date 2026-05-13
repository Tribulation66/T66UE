# Language Select Structural Inventory

Source baseline capture: `C:\UE\T66\Saved\Codex\UI\LanguageSelect\baseline_capture.png`

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\LanguageSelect\baseline_dump.json`

Reference mode: no external V3 reference image exists for Language Select. This inventory is the structural source for the Stage 2 no-reference migration.

Normalized basis: 1920x1080.

## Baseline Summary

- Screen: `LanguageSelect`
- Baseline widgets: `305`
- Baseline tagged widgets: `0`
- Shared top bar: present as `top_bar`; treated as a previously verified shared component.
- Visible layout: shared top bar above a centered language-selection panel.
- Current baseline state: `ENGLISH` is selected, rows 02-07 are visible defaults, and `CONFIRM` is the primary action.
- Baseline dump encoding note: dump is UTF-16 because localized display names include non-ASCII entries.

## Regions And Elements

| Element | Tag | x | y | w | h | Text / role |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Root | `LanguageSelect.Root` | 0.000 | 0.000 | 1.000 | 1.000 | Screen root with shared top bar. |
| Panel | `LanguageSelect.Panel` | 0.118 | 0.157 | 0.764 | 0.805 | Centered language panel. |
| Title | `LanguageSelect.Title` | 0.398 | 0.221 | 0.205 | 0.068 | `SELECT LANGUAGE`. |
| Language list | `LanguageSelect.LanguageList` | 0.187 | 0.313 | 0.627 | 0.512 | Scrollable language list. |
| Row 01 | `LanguageSelect.Row01` | 0.187 | 0.318 | 0.585 | 0.072 | Selected language button, `ENGLISH`. |
| Row 02 | `LanguageSelect.Row02` | 0.187 | 0.401 | 0.585 | 0.072 | Language button, `SIMPLIFIED CHINESE`. |
| Row 03 | `LanguageSelect.Row03` | 0.187 | 0.483 | 0.585 | 0.072 | Language button, `TRADITIONAL CHINESE`. |
| Row 04 | `LanguageSelect.Row04` | 0.187 | 0.566 | 0.585 | 0.072 | Language button, `JAPANESE`. |
| Row 05 | `LanguageSelect.Row05` | 0.187 | 0.649 | 0.585 | 0.072 | Language button, `KOREAN`. |
| Row 06 | `LanguageSelect.Row06` | 0.187 | 0.731 | 0.585 | 0.072 | Language button, `RUSSIAN`. |
| Row 07 | `LanguageSelect.Row07` | 0.187 | 0.814 | 0.585 | 0.072 | Language button, `POLSKI`. |
| Confirm button | `LanguageSelect.ConfirmButton` | 0.405 | 0.854 | 0.190 | 0.077 | Primary action button, `CONFIRM`. |

## Structural Notes

- The shared top bar should continue to appear in the dump as `top_bar`, but its internal fidelity is owned by the completed Frontend Top Bar migration.
- Language rows are a mutually exclusive toggle group. The selected marker is part of the row content and should move when `SelectLanguage` changes `PreviewedLanguage`.
- Hidden/offscreen language rows may exist below row 07; the structural checklist preserves the visible baseline rows only.
