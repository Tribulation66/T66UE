# TD Difficulty Select Reference Gate Notes - 2026-04-24

Status: reference gate established.

## Inputs Used

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current runtime screenshot: `C:\UE\T66\UI\screens\td_difficulty_select\current\2026-04-24\current_runtime_1920x1080.png`
- Layout list: `C:\UE\T66\UI\screens\td_difficulty_select\layout\layout_list.md`
- Ownership audit: `C:\UE\T66\UI\screens\td_difficulty_select\assets\content_ownership.json`

## Outputs

- Canonical reference: `C:\UE\T66\UI\screens\td_difficulty_select\reference\canonical_reference_1920x1080.png`
- Raw generated source: `C:\UE\T66\UI\screens\td_difficulty_select\reference\canonical_reference_1920x1080.generated_source.png`
- Prompt: `C:\UE\T66\UI\screens\td_difficulty_select\prompts\reference_prompt.txt`

## Generation Notes

- Legacy note: this reference was created before the native-only policy was locked.
- Current policy requires Codex-native image generation and rejects `1672x941` or resampled production references.
- Treat this reference as blocked for production until regenerated natively at `1920x1080`.

## Visual Review

- The layout target should preserve the three-column flow: difficulty list, 2x2 map-card grid, and selected-map summary panel.
- The title/back-button crowding is improved relative to the current runtime capture.
- Map cards retain a gameplay-route preview concept and have clearer card chrome.
- The right panel keeps one primary Start Match button.

## Remaining Risks

- The map preview art is representative reference art; runtime map images remain live-owned.
- This specific reference must be regenerated under the current native-only `1920x1080` rule before sprite/component work continues.
- Next stage must generate text-free panel, card, selected-state, scrollbar, and button component families.
- Runtime implementation should not bake map names, descriptions, stats, featured heroes, or map images into art.
