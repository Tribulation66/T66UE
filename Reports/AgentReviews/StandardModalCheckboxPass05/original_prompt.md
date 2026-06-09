# Original User Prompt And Task Contract

## Original Request

Pass 05 Standard Modal Checkbox Worker Request. Use account-backed built-in imagegen capability. Create textless `standard_modal_checkbox_unchecked.png` and `standard_modal_checkbox_checked.png`, each 44 x 44 PNG with alpha, transparent outside the control, centered, no labels/text/data/watermark, checked output contains only the required check mark. Also create `standard_modal_checkbox_contact_sheet.png`, `validation.json`, `record.md`, and `last_message.txt`. If generation succeeds and validation passes write `IMAGE_SAVED`; if generation fails write `IMAGE_FAILED`. Final response must be exactly one line: `IMAGE_SAVED` or `IMAGE_FAILED`.

## Task Contract

Working task: Generate and validate two textless runtime standard-modal checkbox state plates using one fresh built-in imagegen path plus allowed mechanical processing only.
Operator: Codex
Validator: Claude
Scope: Worker folder outputs under `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\SharedPrimitives\pass05_workers\standard_modal_checkbox`; no runtime code integration, no OpenAI API scripts, no web/browser/cached fallback, no manual pixel repair.
Stop condition: `last_message.txt` contains `IMAGE_SAVED` when validation passes, otherwise `IMAGE_FAILED`.

## Repo Rules Applied

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `UI/UI_AGENTS.md`
- `UI/FriendslopStyle/README.md`
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
