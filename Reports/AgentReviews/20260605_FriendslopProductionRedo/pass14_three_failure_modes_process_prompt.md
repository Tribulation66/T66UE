User asks for recommendations, not implementation:

They disagree with Codex's prior interpretation and identify three process failures:

1. Center CTA button still appears to have masking/pillow/inpaint artifacts behind the words. If there is no masking, then the button element itself is too different from the reference. Either way, the current process is failing at reproducing the reference element.
2. The same issue applies to the title: the runtime title element is different from the reference, which means the extraction/reproduction process is not preserving the target element.
3. The topbar clearly shows both masking artifacts and weak text-fitting/placement inside buttons.

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only process recommendation. Do not edit files. Recommend how Codex and Claude should solve these three problems for FriendslopStyle Main Menu reference reproduction.
Stop condition: Provide a concrete extraction/gating workflow that avoids masked/smudged elements, preserves element shape/material from the reference, and adds stronger live text placement validation.

Relevant paths:
- Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- Produced pass14 capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass14_fixture_capture.png`
- Current direct-reference gate report: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass14_reference_component_gate\pass14_direct_reference_v4_component_gate_report.md`
- Contract addendum: `C:\UE\T66\UI\FriendslopStyle\Elements\main_menu_pass14_component_contract_addendum.md`

Repo rules:
- Follow `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- No API imagegen or `OPENAI_API_KEY`.
- Visual scorecard controls acceptance over structural pass counts.
- The goal is reusable Slate UI with live text/icons/data/state and reference-faithful blank chrome plates, not pasted full-screen reference images or baked labels.

Please answer:
- What process should replace the current masking/inpaint-like plate extraction for buttons/title/topbar?
- What new gates should reject "technically blank but visually different" element plates?
- How should text fitting/placement inside topbar and CTA buttons be validated?
- What should Codex do before any next implementation pass?

End with `RESULT: OK` or `RESULT: NEEDS_USER`.
