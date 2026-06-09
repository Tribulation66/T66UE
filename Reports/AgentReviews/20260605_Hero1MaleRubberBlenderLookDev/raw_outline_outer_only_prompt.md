Original user request:
No that tries to add black lines over every detail, I want it like kirby where its just around the outer of edges of the character.

Working task:
Operator: Codex
Validator: Claude
Scope: Blender-only correction to the outline pass: keep the raw Pixal3D look unchanged and replace the Freestyle detail-line treatment with a Kirby-like outer silhouette around the character only. No Unreal and no base material changes.
Stop condition: Blender is open on a corrected comparison scene, with proof render and recipe updated to document the outer-edge-only outline method.

Relevant repo rules:
- C:\UE\T66\AGENTS.md: derive task contract, use Codex operator / Claude validator, no native goal tools, run current verification, report token usage.
- Model Generation/MODEL_GENERATION_AGENTS.md owns Pixal3D/Blender QA.
- Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md says raw FriendSlop visual work must preserve the generated GLB texture/material unless the user explicitly asks for processing. The user explicitly asks for a Blender outline-only look-dev pass, not Unreal import.

Current problem:
- Existing proof `Hero_1_Chad_Male_raw_vs_black_outline.png` uses Blender Freestyle. It correctly leaves the left copy raw, but the right copy has black lines over many internal details. The user rejected this.

Planned correction:
- Replace Freestyle detail-line mechanism with an outer-only silhouette method.
- Keep the visible right-hand character raw.
- Add a separate black outline shell behind the right-hand character only, slightly expanded so it shows only around the outside edge.
- Remove Freestyle from the corrected scene.
- Render proof and update recipe to state that the outline is a separate shell / silhouette carrier and not interior line art.

Ask for Claude:
- Provide an independent repo-grounded answer/check: is this the right Blender-only correction? Surface any scope or verification issue before Codex edits the builder.
