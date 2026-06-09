You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleImplementationProcedure\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleImplementationProcedure\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopStyleImplementationProcedure\ClaudeIndependent\20260605T093800-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original User Request

User wants Codex and Claude to figure out the full implementation pipeline for turning the approved FriendslopStyle Round06 Main Menu visual reference into real Unreal UI.

The user specifically asked:

- Explain next steps and the full process pipeline before implementation.
- Include how corrections and iterations are done.
- Use the available tools, primarily account-backed ImageGen plus Codex/Claude/Unreal work.
- Look into FlatStyle specifically for the part that made screens match reference images by location, size, and positioning.
- Reuse only the FlatStyle reference-fidelity/layout breakdown process, not the FlatStyle visual/chrome approach.
- Figure out the new component: making real UI elements and new 9-slice/N-slice assets.
- Author the central instruction file under FriendslopStyle/FrenchFlop for every screen about this approach.
- Send the implementation plan/instruction file for user approval before any implementation, asset generation, or UI runtime edits.
- If approved later, then generate assets and implement/iterate until it genuinely looks like the reference.
- Prevent lazy implementation where criteria are individually checked but the holistic screen still does not look like the reference.

# Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Produce a repo-grounded implementation/process plan for turning the Round06 FriendslopStyle reference into real Unreal UI, specifically reusing only the FlatStyle reference-fidelity/layout breakdown process where applicable, then author the central FriendslopStyle instruction file. No UI implementation, asset generation, or runtime edits beyond docs in this pass.
Stop condition: plan and instruction doc are authored, Claude has independently reviewed and cross-reviewed, and the result is ready for user approval before implementation begins.

# Relevant Repo Rules

- Root AGENTS.md requires T66 task contracts, live repo checks, Claude validator when available, and no native goal tools.
- UI/UI_AGENTS.md routes UI work through `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` and `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`.
- `UI/UI_AGENTS.md` currently bans generated raster chrome for the existing FlatStyle path. The user explicitly wants a separate FriendslopStyle alternative, so the new doc must define a new method class rather than silently treating FlatStyle rules as the target visual approach.
- Process-governed visual/UI work requires PPF, artifact parity, mechanism/discriminator gates before implementation.
- This pass stops at user approval. It must not generate production assets, edit Slate code, import textures, or run implementation.

# Repo Findings To Ground The Answer

Useful FlatStyle/fidelity process:

- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` Step 0.5 requires reference geometry extraction before implementation.
- Geometry tables record the reference image path, native resolution, normalized 1920x1080 bounding boxes, tolerances, and named UI elements.
- `Scripts/GenerateUIGeometryOverlay.py` renders the geometry table over the reference image for sanity checking.
- `T66.UI.DumpScreen` / `T66.UI.DumpWidget` emit tagged widget JSON used by `Scripts/VerifyUIFidelity.py`.
- `Scripts/VerifyUIFidelity.py` consumes a checklist, reference screenshot, capture, and dump, then writes a report/contact sheet.
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` says 1920x1080 is the composition source, not a fixed runtime viewport, and requires multi-resolution responsive checks.
- `UI/Checklists/pending_issues_Checklists.md` says the current Main Menu checklist is stale and must be refreshed to the current controls (`TimeWeeklyButton`, `TimeAllTimeButton`, `PartySizeDropdown`, `DifficultyDropdown`, metric check buttons, friend rows, top-bar Home).

Round06 reference:

- `UI/FriendslopStyle/Reference/MainMenu/Round06/manifest.md`
- Final mockup: `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- The manifest says the mockup is not runtime UI chrome, labels must remain live/localizable, and it was based on a fresh current capture/dump.
- The accepted direction is inflated/rubber/bouncy surfaces using the current dark/red/green/yellow palette and current Main Menu structure.

Existing Slate slice/brush mechanics:

- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp` creates a brush with `DrawAs = ESlateBrushDrawType::Box` when margins are nonzero and stores the brush margin.
- `Source/T66/UI/Style/T66Style.cpp` has an explicit comment for generated panel textures: `9-slice: corners preserved, center stretches`.
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp` has an older horizontal-sliced button widget that draws left cap, center stretch, right cap and overlays live content.
- `Source/T66/UI/Style/T66FlatStyle.cpp` wraps those reference sliced helpers for FlatStyle, but FriendslopStyle should not reuse the flat visual language as the style target.

# Validator Ask

Please provide an independent, repo-grounded answer:

1. Is the proposed method class viable with this repo's current tools?
2. What exact pipeline should the FriendslopStyle central instruction file require?
3. What should be reused from FlatStyle and what should be forbidden?
4. How should ImageGen-generated mockups become real Unreal UI elements without baking text or causing distortion?
5. What correction/iteration gates prevent a superficially checked but holistically wrong result?
6. Any missing risks or process gaps before Codex authors the central instruction file?

Return concrete corrections and must-have requirements. Do not implement or edit files.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

The task is authorable by Codex/Claude internally — it stops at a doc for user approval, and the user has already made the only user-level decisions (create a separate style; approve the doc later). Below are the concrete corrections and must-haves the central FriendslopStyle/FrenchFlop instruction file needs.

**1. Method class viability — YES, with a critical reframing.**
The repo already has the exact mechanism this style needs: `T66RuntimeUIBrushAccess.cpp` makes `ESlateBrushDrawType::Box` brushes with stored margins, `T66Style.cpp` documents the `9-slice: corners preserved, center stretches` behavior, and `T66ScreenSlateHelpers.cpp` has the left-cap/center-stretch/right-cap sliced button that overlays live content. So generated rubber raster chrome → N-slice brush → live Slate text overlay is mechanically supported today. **But** those same helpers (`ST66ReferenceHorizontalSlicedImage`, `MakeReferenceSlicedPlateButton`, etc.) are classified as *legacy chrome to be deleted* by the FlatStyle Step 0 cleanup regex (UI_FIDELITY_LOOP §5.2). FriendslopStyle must therefore own a **parallel namespace** (e.g. `FT66FriendslopStyle` + its own sliced-brush helpers/asset registry), and the doc must explicitly state that FriendslopStyle screens do **not** run FlatStyle's Step 0 legacy-chrome-removal as their cleanup target — that regex would destroy the very mechanism this style depends on.

**2. Pipeline the central file should require (reused process spine):**
- Step 0.5 reference geometry extraction → `UI/Geometry/<screen>_reference_geometry.md` + overlay via `Scripts/GenerateUIGeometryOverlay.py`, visually sanity-checked (breathing-room rule).
- Widget tagging convention + `T66.UI.DumpScreen`/`DumpWidget` structural dump.
- `Scripts/VerifyUIFidelity.py` against a 5-section checklist (Structure/Geometry/Colors/Content/Interactivity), with geometry copied from the corrected table.
- Pass log, iteration cap (5), stuck-FAIL-set escalation, ESCALATE → user-review packet.
- Multi-resolution responsive checks (1920×1080 is composition source, not runtime viewport).
- **New, style-specific steps:** (a) author each rubber chrome plate as a *fresh, clean, tileable/seamless* raster asset — never cropped from the mockup; (b) record per-asset N-slice margins; (c) wire via runtime texture/brush import as a `Box` brush; (d) keep all text/data as live Slate overlay.

**3. Reuse vs forbid:**
- *Reuse (process only):* geometry extraction, tagging, dump, VerifyUIFidelity, checklist format, pass log, termination/escalation, holistic contact-sheet review.
- *Forbid:* the FlatStyle *visual language* (pure Slate-native flat chrome, `bUseGlow=false` flat plates) as the target look; treating the FlatStyle "remove all raster chrome" rule as Friendslop's goal; importing the mockup itself as chrome; baking any label/score/player-data/localized text into raster (UI_AGENTS hard rule + Round06 manifest §7).

**4. Mockup → real UI without baked text or distortion:**
Decompose the mockup into chrome *elements* (button plate, panel plate, dropdown shell, divider), regenerate each as a standalone seamless raster with transparent padding, import as a `Box`/9-slice brush so corners stay fixed and only the center stretches, then place live `MakeFlatLabel`-equivalent Slate text/icons on top. Distortion is prevented by correct slice margins + sufficient source resolution; baked text is prevented by never generating text into the chrome asset.

**5. Anti-"lazy holistic" gate (the user's explicit concern):**
Add a mandatory **holistic gestalt gate** that runs after the per-item verifier passes and can independently FAIL even with zero per-item FAILs: a full-screen side-by-side (reference vs capture) judged as a whole for silhouette, weight/inflation, spacing rhythm, and overall resemblance. A screen is not DONE on checklist PASS alone — it must also pass this whole-screen sign-off (user-held for art direction, per the manifest's "final art-direction sign-off remains with the user").

## Evidence Checked
- `UI/FriendslopStyle/.../Round06/manifest.md` — mockup is **not** runtime chrome, text must stay live/localizable, output is 1672×941, art-direction sign-off is the user's.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` — full loop, Step 0/0.5, dump/verify infra, checklist/pass-log/termination, §5.2 legacy-chrome list (includes the slice helpers).
- `UI/UI_AGENTS.md` — hard rule banning generated raster chrome and routing through the two instruction docs.

## Questions Or Blockers
None requiring the user *before* the doc is authored. The doc itself is the approval artifact.

## Caveats
- **Governance conflict to surface in the doc, not silently ignore:** `UI/UI_AGENTS.md` hard rule "Do not use generated raster art for chrome" currently applies globally. The Friendslop doc must explicitly scope that ban to the FlatStyle path and declare a sanctioned carve-out for FriendslopStyle, otherwise the new doc contradicts repo law. Codex should flag this amendment for user approval as part of the approval package.
- New raster chrome assets need their own registry (analogous to `content_stubs_registry.md`/`icon_manifest.md`); define its path in the doc.
- Memory notes a prior **Slate brush texture import blocker** on the leaderboard port — runtime raster-brush import is a known risk area; the doc should call it out as a validation step before scaling to all screens.
- 1672×941 vs 1920×1080 normalization must be honored in geometry extraction and VerifyUIFidelity, per loop §10.2.

</validator_independent_answer>

<codex_draft>
# Codex Draft For Cross-Review

Codex authored the central FriendslopStyle process file:

`C:\UE\T66\UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`

This is intentionally a draft approval artifact. It does not implement UI, generate runtime assets, import textures, or modify Slate code.

## Main Decisions In The Draft

- FriendslopStyle is a separate style lane from FlatStyle.
- Approval of the file is required before implementation.
- Generated raster chrome is allowed only for FriendslopStyle after user approval and must not be treated as a FlatStyle workflow.
- The full-screen Round06 reference is visual direction only, not runtime chrome.
- Runtime UI must be built from live Slate widgets, tagged layout, live text/data, and reusable sliced raster chrome assets.
- The reusable FlatStyle part is only the fidelity/process spine: geometry extraction, overlay, tagged dumps, checklist, `VerifyUIFidelity.py`, pass logs, contact sheets, responsive checks.
- The forbidden FlatStyle carryover is its flat visual target and global no-raster-chrome implementation method.
- The implementation pass should create a Friendslop-specific runtime style layer such as `FT66FriendslopStyle`, not expose old `MakeReference*` APIs as the screen-facing contract.
- The Main Menu pilot must use Round06 and refresh the stale Main Menu verification checklist into a FriendslopStyle checklist.
- The draft adds PPF, artifact parity, mechanism manifest, slice/spec, ImageGen runtime chrome rules, correction loop, holistic gestalt gate, responsive gate, and manual interaction gate.

## Evidence Used

- `UI/UI_AGENTS.md`
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`
- `UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`
- `UI/Checklists/pending_issues_Checklists.md`
- `UI/FriendslopStyle/Reference/MainMenu/Round06/manifest.md`
- `Scripts/GenerateUIGeometryOverlay.py --help`
- `Scripts/VerifyUIFidelity.py --help`
- `Source/T66/UI/Style/T66RuntimeUIBrushAccess.cpp`
- `Source/T66/UI/Style/T66Style.cpp`
- `Source/T66/UI/Screens/T66ScreenSlateHelpers.cpp`
- `Source/T66/UI/Style/T66FlatStyle.cpp`

## Cross-Review Request

Please review the authored file directly and compare it to your independent answer. Flag:

1. Any contradiction with root/UI process rules that needs to be fixed before user approval.
2. Any missing must-have artifact or gate for preventing a lazy/holistically wrong implementation.
3. Any misleading technical claim about 9-slice/3-slice, Unreal Slate brush usage, ImageGen, or the fidelity loop.
4. Any paths or script commands that are wrong in the current repo.
5. Any wording that could let a later implementation skip user approval, bake text/data, use the full-screen mockup as runtime UI, or call a partial result complete.

Do not edit files. Return concrete corrections only.

</codex_draft>
