# TD Battle Reference Gate Notes - 2026-04-24

Status: reference gate established.

## Inputs Used

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current runtime screenshot: `C:\UE\T66\UI\screens\td_battle\current\2026-04-24\current_runtime_1920x1080.png`
- Layout list: `C:\UE\T66\UI\screens\td_battle\layout\layout_list.md`
- Ownership audit: `C:\UE\T66\UI\screens\td_battle\assets\content_ownership.json`

## Outputs

- Canonical reference: `C:\UE\T66\UI\screens\td_battle\reference\canonical_reference_1920x1080.png`
- Raw generated source: `C:\UE\T66\UI\screens\td_battle\reference\canonical_reference_1920x1080.generated_source.png`
- Prompt: `C:\UE\T66\UI\screens\td_battle\prompts\reference_prompt.txt`

## Generation Notes

- Legacy note: this reference was created before the native-only policy was locked.
- Current policy requires Codex-native image generation and rejects `1672x941` or resampled production references.
- Treat this reference as blocked for production until regenerated natively at `1920x1080`.

## Visual Review

- A first battle reference was rejected because it added an extra Score stat and showed active upgrade costs in the no-selection state.
- The accepted reference preserves three stat wells only: Materials, Hearts, Wave.
- The accepted reference keeps the default `No Hero Selected` state and visually disables the upgrade/sell buttons.
- The central board is clearer and reads as a live gameplay aperture with pad sockets.
- Roster rows keep portrait sockets and live text wells.

## Remaining Risks

- The board art is a reference target only; runtime board paths, pads, heroes, enemies, effects, selection, and drag/drop state remain live-owned.
- This specific reference must be regenerated under the current native-only `1920x1080` rule before sprite/component work continues.
- Next stage must generate text-free component families and an ownership-aware board frame/scene strategy.
- Upgrade, speed, start-wave, roster-row, scrollbar, and pad states still need full component-state coverage before Unreal runtime reconstruction continues.
