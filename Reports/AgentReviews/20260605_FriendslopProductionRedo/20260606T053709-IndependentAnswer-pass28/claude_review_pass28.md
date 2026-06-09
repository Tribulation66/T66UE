All key facts verify: 48 markdown files, the permissive "regenerate, recrop, alpha-clean, or adjust slice margins" line at line 414, the AGENTS.md imagegen rule, and the user-approved pass14 exception dated 2026-06-06.

Result: OK

## Independent Answer

The user's analysis is substantially correct and Codex should adopt it with one important nuance. The proposed rule — imagegen is the only authoring path for premium runtime UI pixels; manual/Pillow/OpenCV/skimage are confined to measurement, extraction, alpha/crop/contact-sheet QA, and validation — is the right governing principle and matches the existing direction in `AGENTS.md` and the pass14 prompt warning. The doc set is currently too permissive in exactly one place and should be tightened.

**The one nuance that must be preserved:** "ban manual/Pillow/patched edits to production asset pixels" is correct as a *content-authoring* ban, but it must NOT ban mechanical, non-painting operations that are part of legitimate packaging: alpha thresholding/cleanup, cropping, slicing, transparent-channel flattening, resizing, and assembling contact sheets. The distinction is **authoring vs. mechanical processing**, not "tool X is banned." A clean phrasing: *no tool may invent, paint, retouch, infill, denoise, upscale-hallucinate, or otherwise synthesize visual content outside of approved imagegen.* Pillow/OpenCV/skimage remain allowed for lossless geometric/alpha operations and all QA/validation.

**Second nuance — the pass14 carve-out must be respected.** `main_menu_pass14_component_contract_addendum.md` contains a user-approved exception (2026-06-06) allowing crop-derived runtime plates for Main Menu pass14 only, explicitly "not a global permission." Any new global rule must be written so it does not retroactively void that approved exception — reference it as the single sanctioned deviation, gated and scoped.

**Exact doc changes to propose to Codex:**

1. `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` line 414 — replace `regenerate, recrop, alpha-clean, or adjust slice margins` with language that separates remediation classes: (a) **content failure → regenerate via imagegen only** (re-prompt, re-roll, re-mask the generation request); (b) **packaging failure → re-crop, re-slice, re-alpha-clean** are allowed because they touch only geometry/channels, not painted content. Make explicit that "recrop/alpha-clean" is never license to patch or paint pixels.

2. `AGENTS.md` / `UI/UI_AGENTS.md` — add a short canonical "Visual Asset Authoring Authority" rule stating imagegen is the sole synthesis path, listing the allowed mechanical/QA uses of Pillow/OpenCV/skimage, and pointing to the pass14 addendum as the one approved exception. Put the authoritative wording in ONE place and have the others link to it to avoid drift.

3. Optionally add the same one-paragraph rule to `friendslop_asset_registry.md` header so registry consumers see it.

**Document count/classification:** 48 total `.md` under `UI/FriendslopStyle` is confirmed. Classifying 16 as active governing/spec/checklist docs is reasonable, but note the list spans repo-root (`AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`) and `UI/Instructions/` files that live *outside* `UI/FriendslopStyle`, so "48 under FriendslopStyle" and "16 governing" are two different universes — Codex should state that distinction plainly so the user isn't told 16 of 48 when several of the 16 aren't in that folder.

## Evidence Checked
- `find UI/FriendslopStyle -name '*.md' | wc -l` → 48 (confirmed).
- Line 414 of `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — permissive remediation phrasing confirmed verbatim.
- `AGENTS.md` imagegen-authority language present (line ~203).
- `main_menu_pass14_component_contract_addendum.md` — user-approved 2026-06-06 narrow exception, "not global," confirmed.

## Questions Or Blockers
None that require the user. The pass14 exception was already user-approved; the new global rule is a process-doc edit Codex can draft and apply. The user's earlier "is the analysis correct?" is answerable from evidence.

## Caveats
- 7 of the 16 "governing" docs live outside `UI/FriendslopStyle`; don't conflate the 48-file folder count with the 16-doc governance set.
- The rule's value depends entirely on the authoring-vs-mechanical wording landing precisely — a blunt "no Pillow" ban would break legitimate alpha/crop/contact-sheet QA and should be rejected.
- Centralize the canonical rule in one file; duplicated copies across 16 docs will drift.
