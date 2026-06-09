Result: OK

## Summary
Codex and the independent validator agree on the root cause: the raw FriendSlop Hero 1 mesh uses the raw/static forward convention (like mobs, +90) but inherited the legacy processed-hero yaw (−90), producing the exact 180° back/front inversion. Codex chose the data-row path (option 2 in Claude's answer) but hardened it into a manifest-driven, validator-gated system rather than a bare per-row hack — which reasonably satisfies the "future heroes" stop condition. Codex also supplied the runtime verification the validator left open (DataTable reload, validator pass, visual capture, staged build, shortcut check). Evidence is concrete and internally consistent.

## Suggested Answer Patch
The proposed final answer is sound. Two tightening edits before sending to the user:
- State explicitly that the fix is a **runtime orientation correction in `CharacterVisuals.csv` (yaw −90→+90)**, made reusable via the manifest + validator, **not** a mesh re-bake — so the user understands the raw mesh is untouched.
- Add one line: future FriendSlop raw heroes are auto-corrected **only if they share the same raw forward axis**; the doc rule encodes that expected axis, and the validator will flag any hero that regresses from yaw 90.

## Issues To Fix
- **Generality of the +90 rule.** Confirm the Facing Preservation Rule in doc 11 documents the *expected raw-export forward axis*, not just "use yaw 90." An unconditional +90 over-rotates any future hero authored to the hero forward convention. The validator gate mitigates regressions but the doc should make the assumption explicit. (Minor — Codex can confirm the doc wording.)
- **Divergence from Claude's recommended path is acceptable but should be acknowledged.** Claude preferred import-time normalization; Codex's manifest+validator data path is defensible and arguably equally systematic. No change required, just note the trade-off (data-explicit yaw vs. baked mesh) so it's a conscious choice.

## Question For User
None. Scope was pre-approved; the fix-location choice and verification were Operator-executable.

## Evidence Or Verification Gaps
- Visual-proof interpretation checks out: a chase camera correctly shows the hero's **back to camera** while moving toward enemies, which is the intended forward orientation — consistent with the reported bug being resolved. Worth having the user eyeball the contact sheet/mp4 to confirm subjectively.
- All claimed verification artifacts (validator JSON `ok=true`, reload log lines, staged exe, shortcut targets) are cited with paths; not independently re-run here but specific and falsifiable.

## Notes
Codex's approach is stronger than a plain data-row hack because the validator now fails on yaw regression, directly defending the "future heroes" stop condition. No blocking issues.
