# Question-Only Prompt: FriendslopStyle Next Step

Working task:
Operator: Codex
Validator: Claude
Scope: answer what should happen next after the pass12 quality diagnosis, with no implementation.
Stop condition: provide a concrete next-step plan that avoids repeating the failed screenshot-inpaint approach.

Context:

- User rejected the pass12 visual quality and asked four root-cause questions.
- The answer concluded that OpenCV/skimage/Pillow are acceptable for measurement/verification but not viable as production UI asset generation.
- The core defect is screenshot-crop-and-inpaint production: it creates smudged masks, baked-content layering, icon-on-icon, and state/detail mismatches.
- User now asks: "Ok so what should be done next?"

Relevant current evidence:

- Pass12 capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_capture.png`
- Material crop sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_crop_sheet.png`
- Prior question-only validator answer: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T003139-IndependentAnswer-pass9\claude_review_pass9.md`

Answer only. No implementation requested.
