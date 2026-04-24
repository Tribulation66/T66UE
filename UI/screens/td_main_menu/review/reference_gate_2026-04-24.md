# TD Main Menu Reference Gate Notes - 2026-04-24

Status: reference gate established.

## Inputs Used

- Style anchor: `C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png`
- Current runtime screenshot: `C:\UE\T66\UI\screens\td_main_menu\current\2026-04-24\current_runtime_1920x1080.png`
- Layout list: `C:\UE\T66\UI\screens\td_main_menu\layout\layout_list.md`
- Ownership audit: `C:\UE\T66\UI\screens\td_main_menu\assets\content_ownership.json`

## Outputs

- Canonical reference: `C:\UE\T66\UI\screens\td_main_menu\reference\canonical_reference_1920x1080.png`
- Raw generated source: `C:\UE\T66\UI\screens\td_main_menu\reference\canonical_reference_1920x1080.generated_source.png`
- Prompt: `C:\UE\T66\UI\screens\td_main_menu\prompts\reference_prompt.txt`

## Generation Notes

- Legacy note: this reference was created before the native-only policy was locked.
- Current policy requires Codex-native image generation and rejects `1672x941` or resampled production references.
- Treat this reference as blocked for production until regenerated natively at `1920x1080`.

## Visual Review

- The layout target should preserve the TD Main Menu hierarchy: top-left back button, top-center title, lower-left Regular Mode panel, lower-center two-button CTA stack, and lower-right Tier Preview panel.
- A first native attempt was rejected because it added icon sockets inside text-only cards and changed the second CTA to blue.
- The accepted pass removes those extra sockets and keeps both CTA buttons green.

## Remaining Risks

- This is a generated offline reference, not runtime art.
- This specific reference must be regenerated under the current native-only `1920x1080` rule before sprite/component work continues.
- The next stage must generate text-free component families from this reference and keep all labels, counts, and descriptions live.
- The current runtime still contains the pre-gate styling work and should not be polished until sprite/component assets are generated from this reference.
