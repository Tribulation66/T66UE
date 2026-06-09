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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\structural_solutions_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\operator_draft_structural_solutions.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260605T230940-IndependentAnswer-pass4\claude_review_pass4.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# FriendslopStyle Structural Solution Gate

Read-only validator analysis. Do not edit files, generate assets, run Unreal, or propose another blind production pass.

## User prompt

The user believes the clean alpha sheet is inadequate and explicitly does not want Codex or Claude to simply produce a new one and try again. They want solutions to three structural problems first:

1. Several buttons/elements look cut in half: there is a clear top part and bottom part with a line between, as if the middle section was removed. This cannot be accepted.
2. The team needs a real solution to content inside a panel not fitting inside that panel.
3. The produced UI elements were too different from the reference image. The left/right side panels and the two central buttons are categorically different from the reference, not merely slightly off.

Answer what the actual solutions to these three problems are before any further implementation.

## Task contract

Working task:
Operator: Codex
Validator: Claude
Scope: answer the three structural failure questions only, with no implementation or new asset attempt.
Stop condition: concrete solutions for the split/missing-middle artifact, panel content fitting, and reference-mismatch source problem, plus any user decision needed before work continues.

## Relevant project rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `UI/UI_AGENTS.md`, `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md`, and `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`.
- FriendslopStyle allows generated raster chrome, but it must be decomposed into reusable transparent PNG plates or plate families, with live Slate text/data/icons.
- A zero-FAIL structured report is not enough. FriendslopStyle acceptance requires a visual scorecard/contact-sheet gate.
- Generic blank rubber atoms are not enough for high-fidelity FriendslopStyle work.
- Plates are sliced only when min/normal/wide tests prove bevels, highlights, shadows, and material read survive scaling. If a plate cannot be sliced cleanly, a size-specific plate is required.
- Existing pass10 evidence:
  - Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
  - Pass10 capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass10_fixture_capture.png`
  - Pass10 report: `PASS=249 FAIL=4 UNSURE=0`
  - Pass10 visual scorecard: `Result: FAIL`
  - Pass10 scorecard findings: row containment passed, but top bar material, left panel material/scale, right leaderboard material/scale, CTA/button family, and whole-screen glance all failed.
- Existing pass log posthoc invalidation says the prior false accept came from generic pilot FriendslopStyle plates and structural coordinate checks, and future work must re-author reference-matched per-element/per-size plates rather than self-pass broad style mismatch.

## Required answer shape

Give a direct, practical answer with three sections:

1. Split / missing middle line problem: cause, actual solution, acceptance gate.
2. Panel fitting problem: cause, actual solution, acceptance gate.
3. Reference mismatch problem: cause, actual solution, acceptance gate.

End with the process change required before the next production pass. If any choice must be made by the user before implementation can resume, name it clearly.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

I inspected both images directly. The diagnosis below is grounded in what the pass10 capture actually shows versus the Round06 reference, and the solutions map to mechanisms the implementation instructions already require but that pass10 skipped.

### 1. Split / missing-middle line artifact

**Cause.** This is a 9-slice/3-slice authoring-and-margin failure, not a layout failure. The `ENTER TRIBULATION` and `LOAD GAME` plates are being drawn with `DrawAs=Box` where the source plate's vertical center band does not actually contain continuous body material, and/or the `FMargin` top+bottom caps sum to nearly the full source height. When the brush is stretched to runtime height, the preserved top cap and bottom cap pull apart and the empty/short center band renders as a seam — the "cut in half, middle removed" look. The reference buttons are single inflated rubber pills with an unbroken glossy body; the capture's are two stacked caps with a dead line between.

**Actual solution.**
- Treat these stable-height/variable-width buttons as **horizontal-3-slice**, not vertical/full 9-slice. Height should be fixed to the source plate height (or use a size-specific fixed-image plate). Only the width stretches; the vertical body is never stretched, so no horizontal seam can appear.
- If the button must change height, the center band must be re-authored as a clean, seam-safe, full-height fill with low detail, and margins set as normalized fractions that leave a real stretchable middle. Do not trust "looks plausible" margins (Sec. 9).
- Re-crop with enough transparent padding for the soft shadow so the cap isn't clipped.

**Acceptance gate.** The mandatory **min/normal/wide stretch test** at the runtime height plus the per-family **contact sheet at min/normal/wide** (Sec. 8–9). The plate fails the gate if any highlight/bevel/shadow smears or any seam appears at the center band. No import until that contact sheet passes and the single-brush runtime smoke test (Step E) renders clean.

### 2. Panel content not fitting inside the panel

**Cause.** Containment failure: the left social panel and right leaderboard were authored so child content (rows, search field, dropdowns, scroll body) is sized/positioned independently of the parent plate's interior content rect, so rows escape or crowd the panel walls. Absolute screen position being "close" hid it from the structural PASS count.

**Actual solution.**
- Define each panel plate with an explicit **content inset / interior content rect** (padding inside the 9-slice border) and constrain the live child list to that rect with proper clipping. The panel chrome owns the frame; Slate owns a bounded body that lists/scrolls inside the inset.
- Fix at the parent: panel size, content insets, row width/height, and list/table body constraints and clip region — per the Step I containment-failure recipe. Do not patch by nudging absolute row positions.
- Where the row count exceeds the visible body, the body must scroll/clip within the inset, never paint over the frame.

**Acceptance gate.** Containment is verified in the dump: every row/control's bounds must be inside its intended parent's content rect, plus a visual check that no content touches or crosses the plate border at min/normal/wide and across the responsive resolution set (Step K). "Row containment passed" in pass10's scorecard only covered the center stack — the left/right panels must pass the same containment check explicitly.

### 3. Reference mismatch (side panels + central buttons categorically wrong)

**Cause.** This is the already-recorded false-accept root cause: generic pilot/"blank rubber atom" plates were reused and self-passed on structural coordinates, instead of authoring reference-matched per-element/per-size plates. A plate that is merely the right semantic category ("red rubber button," "dark panel") but wrong silhouette, scale, material density, and edge treatment is explicitly a fail (Sec. 8). The capture's panels and buttons read as flat/generic against the reference's inflated, glossy, beveled rubber.

**Actual solution.**
- Re-author **per-element / per-size-family plates** that match the cropped reference region directly — left panel shell, right leaderboard shell, primary CTA, secondary CTA each as their own authored plate family with matched states — rather than stretching one generic atom across all surfaces (Sec. 8, Sec. 10 "do not force all high-quality chrome through one generic resizable brush").
- Compositing/Photoshop-equivalent cleanup is allowed and expected to hit the material bar (Sec. 2.3).
- Each plate is compared against its **cropped reference region** before slicing, so silhouette/scale/material/edge are matched, not approximated.

**Acceptance gate.** The **visual scorecard / holistic gestalt gate** (Step J) must return `Result: PASS` with the per-category rows (first-glance match, silhouette/region weight, panel/button scale hierarchy, rubber material fidelity, authored plate quality) all passing — and each plate's contact sheet must show it beside the cropped reference region. A zero-FAIL structured report is explicitly not acceptance.

### Process change required before the next production pass

No more blind full-screen production passes. Before any code/asset work, the next pass must, per the instructions:
1. Write the **PPF gate, Artifact Parity gate, and Mechanism Manifest** for Main Menu (Sec. 5–7), with the left panel, right leaderboard, and both CTAs each listed as **Required: YES, Primary** with their own planned plate paths.
2. Author and pass the **per-family contact sheet + min/normal/wide slice test + single-brush runtime smoke** before importing any full set (Step E).
3. Gate acceptance on the **visual scorecard PASS**, not the PASS/FAIL count — and both Codex and Claude must inspect the reference/capture before citing the structured count (Step J).
4. Sequence the three fixes as: re-author reference-matched per-element/per-size plates → wire panel content insets/clipping → only then re-capture and run the holistic gate.

**User decision to name before implementation resumes:** the slicing-vs-fixed-plate policy for the CTAs and panels is a taste/quality call the user owns — i.e., approve that height-variable rubber surfaces are allowed to ship as **size-specific fixed-image plates** (no vertical stretch) rather than forcing a single resizable 9-slice. If the user wants one resizable brush per surface for asset economy, the split-seam risk returns and the center band must be re-authored to a higher bar. This is the one art-direction/quality tradeoff that should be confirmed before the plate-authoring pass starts.

## Evidence Checked

- Pass10 capture `Saved/Codex/UI/FriendslopStyle/MainMenu/friendslop_pass10_fixture_capture.png` — viewed: CTA buttons show clear top/bottom cap separation with a dead center line; panels read flat/generic; rows crowd panel interiors.
- Reference `UI/FriendslopStyle/Reference/MainMenu/Round06/main_menu_reference_01_current_capture_stronger_rubber_cli.png` — viewed: single inflated rubber pills, consistent gloss/bevel, content seated inside panel insets.
- `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — Sec. 8 (per-element/per-size plates, contact-sheet gate), Sec. 9 (slice rules, min/normal/wide test), Step E/I/J/K gates.
- Confirmed all five named rule files exist; both evidence images exist.

## Questions Or Blockers

- None that block *answering* the three questions. The single user-only decision is the slicing-vs-size-specific-plate policy for CTAs/panels, named above; it is a prerequisite for the plate-authoring pass, not for this analysis.

## Caveats

- I did not measure pixel margins or open the pass10 dump/slice specs; the 9-slice-margin diagnosis is from visual evidence plus the instructions' known failure modes. Codex should confirm the actual `FMargin` values and source plate center band when authoring the fix.
- The "stale FlatStyle checklist" issue (Sec. 12) is orthogonal to these three problems but must not be used as evidence for the next pass — author a fresh Friendslop checklist.
- I did not re-verify pass10's PASS=249/FAIL=4 numbers against the report file; I took them from the prompt's stated context.

</validator_independent_answer>

<codex_draft>
# Operator Draft: FriendslopStyle Structural Solutions

Short answer: the clean alpha sheet is inadequate as a source for Round06. The solution is not another blind sheet attempt; the next production pass has to be blocked until the plate-authoring method, panel safe areas, and reference-category gates are changed.

## 1. Split / missing-middle line problem

Cause:
The split line is a source-and-slicing failure. Several plates are being treated like resizable 9-slice/box brushes even though their source art does not contain a continuous neutral center band. The preserved top and bottom cap detail survive, but the middle reads as a groove or missing strip. That is why the button looks like two halves, not one inflated rubber surface.

Actual solution:
Do not vertically 9-slice those controls. For stable-height buttons and fields, use either fixed-size plates or horizontal 3-slice plates: fixed height, stretch only width. If any element must resize vertically, the source plate must be re-authored with a real continuous center fill before it can be sliced. A plate with a built-in central groove is not valid for this target, regardless of margin tuning.

Acceptance gate:
Before Unreal import, every plate family gets a min/normal/wide contact sheet at target runtime height. It fails if a horizontal seam, separated top/bottom caps, clipped glow/shadow, smeared bevel, or missing center band appears. In the next PPF/artifact gate, these controls should be marked fixed-size or horizontal-slice unless a clean vertical stretch band is proven.

## 2. Content fitting inside panels

Cause:
The rows and controls were allowed to be "close" in absolute screen coordinates without a strict parent content rectangle. That lets rows, labels, buttons, and table content crowd or escape the actual usable body inside the decorative panel.

Actual solution:
Each panel must have two boxes: the outer chrome bounds and an explicit safe content rect/inset. All children are laid out relative to that safe rect, clipped by it, and verified against it. Friend rows, search fields, headers, party slots, leaderboard dropdowns, table headers, and ranking rows need fixed row heights, fixed gaps, max widths, and overflow behavior. Lists scroll or clip inside the body; they do not resize the panel and do not paint into the border.

Acceptance gate:
The checklist/dump must include `contained_in=<parent> inset=<l,t,r,b>` for every row/control that lives inside a panel. Add stress fixtures, not just the reference fixture: longest friend names, max online/offline counts, long leaderboard names, empty/loading states, and localized strings. A pass is only valid when both fixture and stress captures show no escape, overlap, or border collision.

## 3. Reference mismatch problem

Cause:
The source family is categorically wrong. The pass10 plates are generic pipe/groove rubber atoms. Round06 uses smoother inflated panels and pills with subtler bevels, different region weight, and different button faces. Matching coordinates and sharing red/black/green colors cannot fix a wrong element class.

Actual solution:
Re-author the runtime chrome by reference category, not by generic atom reuse. The left panel shell, right leaderboard shell, search/header/friend row surfaces, CTA primary, CTA secondary, tabs, dropdowns, small icon buttons, and leaderboard rows each need their own reference-matched plate or plate family. Imagegen can create candidates, but Photoshop-equivalent cleanup/compositing is expected. Live Slate still owns text, names, icons, counts, scores, handlers, and states.

Acceptance gate:
Every new plate is shown beside its Round06 crop at target runtime size before it is accepted. The gate must ask "same element category?" first, before polish. If the side panel is still a pipe frame, or the central button is still a split rail/groove button, it fails even if geometry passes. Whole-screen acceptance requires the visual scorecard to pass first-glance match, silhouette/region weight, panel/button hierarchy, rubber material fidelity, authored plate quality, overflow/clipping, and live content preservation.

## Required process change before the next production pass

The next pass should start with a plate taxonomy and safe-rect specification, not generation:

1. List every required Round06 plate family and classify it as fixed-size, horizontal-slice, or proven 9-slice.
2. For panels, define outer chrome bounds and inner content safe rects.
3. Produce per-family reference-crop contact sheets before runtime integration.
4. Run seam/stretch gates before import, containment gates after layout, and holistic visual scorecard after capture.

The user decision needed before implementation resumes is whether we are allowed to prefer size-specific fixed plates for the CTAs and large panels when that is the only way to preserve the Round06 look. My recommendation is yes: for this screen, visual fidelity matters more than forcing a single resizable brush across elements that should not be vertically stretched.

</codex_draft>
