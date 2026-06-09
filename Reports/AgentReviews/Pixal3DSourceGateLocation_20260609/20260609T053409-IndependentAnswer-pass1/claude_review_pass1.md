Codex's finding is fully verified against the live repo.

Result: OK

## Independent Answer

The Pixal3D source gate lives at:

**`Model Generation/Instructions/02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`**

This is the shared source-image approval gate for all Pixal3D/TRELLIS model-generation inputs — it defines required image qualities (single subject, T-pose, front-readable silhouette, clean white background) and the default color discipline for non-humanoid characters, creatures, props, and interactables.

The companion file for humanoid hero/companion inputs is:

**`Model Generation/Instructions/10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md`**

It explicitly states (lines 5–6) that it sits "below the shared source-image gate in `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md`," and instruction #1 (line 18) tells the reader to read `02` first. The `02` file reciprocally points humanoid prompts to `10` for the family-specific color section (lines 21–24). Use `02` first, then `10` for humanoid prompts.

## Evidence Checked
- `Model Generation/Instructions/` directory listing — both files present.
- Read header of `02_SOURCE_IMAGE_RULES_INSTRUCTIONS.md` — confirms it is the source/color gate and cross-references `10`.
- Grep of `10_HUMANOID_IMAGE_GENERATION_GUIDELINES.md` — confirms the "below the shared source-image gate in `02_...`" language and the "read `02` first" step.

## Questions Or Blockers
None. Files are live and Codex's path references match exactly.

## Caveats
- Codex's draft is accurate and complete; no corrections needed.
- Path separators in the original prompt use `/`; on this Windows repo the actual separator is `\`, but the relative paths are otherwise identical and correct.
