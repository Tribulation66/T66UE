Original user request:
Ok before I test I should also be able to jump above the cylinder and not get hit, but previously whenever I try that, and visually it looks like I cleared the obstocle I still get hit fix this.

Working task:
Operator: Codex
Validator: Claude
Scope: fix the TestRoom wipeout arm hit test so a visibly cleared jump over the cylinder does not ragdoll the hero.
Stop condition: root cause is corrected in trap collision logic, focused compile and TestRoom proof are run, staged standalone validation is attempted if runtime gameplay changes are made, and limitations are stated.

Relevant repo rules:
- Use live repo state.
- Gameplay runtime changes require compile/build verification and staged standalone validation when playable standalone is affected.
- Prefer data-authored tuning over hardcoded C++ defaults.
- This is implementation, not consultation.

Please provide an independent, repo-grounded answer: inspect the current TestRoom wipeout arm collision/hit code and identify the safest fix so jumping over the cylinder clears the trap instead of triggering a false hit.
