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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\main_menu_next_version_plan_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\codex_draft_main_menu_next_version_plan.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\20260606T005834-IndependentAnswer-pass13\claude_review_pass13.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:

Well hold on, I feel like you are answering these questions to conceptually and broadly and not really considering the context, every single ui element we have or at least 90% of them for the main menu screen were they not produced through imagegen? How else would we produce a button for example? I get what you mean about you take that image gen product and then create the actual slate umg buttons. I agrre on the next step, but lets not forget the other points because this was a clarification. This was your original assesment: Next step: stop iterating the crop/inpaint asset pipeline. Pass12 should be treated as diagnostic evidence, not a base for pass13.

The right next move is:

Pick the production method. My recommendation is native Slate/token-driven UI, since this screen is already live Slate and needs real states, sizing, text, icons, hover/click behavior, and dynamic data. Use Pillow/OpenCV/skimage only for measuring and verifying captures, not creating runtime button art.

Write a component contract before touching code. For each offender: topbar icon buttons, achievements tab, primary/secondary CTA, leaderboard row, checkbox, online header, invite button, friend rows. Define: owner of text/icon, exact size, font size, padding, colors, border/outline, fill, corner radius, hover/selected/disabled states.

Rebuild the worst elements natively first, not the whole screen. Start with:

topbar icon buttons: no baked icon under live icon
achievements tab: measured text-fit rule
CTA buttons: native glossy layered brush/material, no masked center
leaderboard row: red outline + dark interior
high score checkbox: uniform rounded square
online header green dot + green invite state
Replace the verifier gate. PASS=251 is not enough. Add a material/state gate with side-by-side crops for each component and explicit PASS/FAIL rows for color, shape, state, content ownership, and text fit.

Only then do a new capture. The next capture should prove a small native component slice first. If that slice still fails visually, we fix the method before touching the rest of the screen.

So the immediate next action, if we proceed, should be: freeze screenshot-inpaint runtime generation and create a native Slate component/token spec for those six failing UI families, then implement only that first component slice. So rework this into an actual implementation plan. Which you will do for the new version of the main menu screen

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: Produce a concrete implementation plan for the next FriendslopStyle Main Menu version. The plan must explicitly correct the prior over-broad wording: imagegen remains valid for clean blank runtime chrome plates/component families, but screenshot crop/inpaint is frozen, and Slate/UMG owns live layout, text, icons, states, and data. No code or asset implementation in this answer.
Stop condition: Deliver a specific plan with phases, files/systems, imagegen asset family rules, Slate integration targets, verification gates, risks, and acceptance criteria.

Relevant live repo rules and observations:

- Root AGENTS.md requires Operator/Validator review for every T66 prompt and routes UI reference fidelity work through UI instructions.
- .t66/operator-state.json selects Codex as Operator and Claude as Validator.
- UI/UI_AGENTS.md says FriendslopStyle may use generated raster chrome only through UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, decomposed into reusable sliced UI assets, not pasted full-screen mockups.
- UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md says runtime chrome generation must create blank standalone plates one component family at a time, with no labels/names/scores/fake text, clean front-on orthographic view, alpha-clean transparent PNG, matched states, local contact sheets, slice specs, and Slate-owned live content.
- UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md says structured PASS counts do not prove visual fidelity; FriendslopStyle needs a blocking visual scorecard/contact sheet.
- UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md requires parent containment and stable min widths/heights for controls; clipped controls cannot be accepted.
- Current code surfaces that need plan-level targeting:
  - Source/T66/UI/Style/T66FriendslopStyle.h/cpp: ET66FriendslopChrome, GetChromeBrush, MakeButton, MakeFixedImage, chrome descriptors and draw modes.
  - Source/T66/UI/T66FrontendTopBarWidget.cpp: MakeIconActionButton, AchievementsRect, CategoryTabFontSize, category tabs, ticket canvas, power/settings/language buttons.
  - Source/T66/UI/Screens/T66MainMenuScreen.cpp: MakeCtaButton, MakeSearchField, MakeFriendGroupToggle, MakeFriendRow, MakeLeftPanel, MakeCtaStack.
  - Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp/h: BuildFilterButton, BuildTimeButton, BuildMetricCheckButton, BuildLeaderboardRow.
- Known failures the plan must solve:
  - icon-on-icon on topbar buttons because generated plate/glyph ownership and live glyph ownership were not cleanly separated.
  - achievements label does not fit the button.
  - central CTA buttons look masked/smudged, as if text was painted out and new text written on top.
  - leaderboard local row uses red-filled interior but reference requires red outline with dark interior.
  - high score checkbox shape/color is not the uniform rounded square seen in reference.
  - friends panel lacks online green dot and green invite state in the visible result.
  - rows/content overflow inside panel cannot be accepted.

Please give an independent plan-focused answer. Do not edit files. Do not run mutating commands. Keep it practical and specific to this repo, and include any corrections to the plan if Codex should avoid a wrong framing.

</original_prompt>

<validator_independent_answer>
I have enough grounding. The chrome enum already encodes a token/descriptor-per-family system that the plan should build on.

Result: OK

## Independent Answer

This is a method-correction + first-slice plan, not a full-screen rebuild. The framing fix is the load-bearing part: **imagegen is not banned — screenshot crop/inpaint is.** Imagegen stays valid only for producing *blank, stateless, label-free chrome plates per component family*, which are then sliced into `ET66FriendslopChrome` brushes. Slate/UMG owns every live glyph, label, score, color-state, and data binding. The existing `T66FriendslopStyle` enum already has per-family, per-state descriptors (e.g. `TopbarIconDarkRound06`, `CtaPrimaryRound06`, `RankingRowRedRound06`, `CheckboxChecked/EmptyRound06`, `InviteButtonGreenRound06`), so the plan extends an existing token system rather than inventing one.

**Phase 0 — Freeze & framing correction**
- Hard-freeze screenshot-inpaint runtime generation. Pass12 = diagnostic evidence only; no pass13 on that pipeline.
- Document the corrected rule in `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (plan only): imagegen → blank plates per family; Slate → content/state.

**Phase 1 — Component contract (no code)**
For each of the six failing families, write a contract defining: owner of text vs icon vs plate, exact px size, min width/height (per `UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md` containment rule), font size, padding, colors, border vs fill, corner radius, and the hover/selected/disabled/empty states. Families:
1. Topbar icon buttons — plate owns frame only; live `SImage` glyph on top; **no baked glyph in the plate** (root cause of icon-on-icon).
2. Achievements tab — measured text-fit rule (auto-shrink/min-width) so label fits; ties to `CategoryTabFontSize`/`AchievementsRect`.
3. CTA primary/secondary — native layered glossy brush/material; **no masked center**, no painted-over text (root cause of smudged look).
4. Leaderboard row — red **outline** + dark interior, not red fill. Fix `RankingRowRedRound06`/`RowSelectedRed` descriptor.
5. High-score checkbox — uniform rounded square; reconcile `CheckboxCheckedRound06`/`CheckboxEmptyRound06`.
6. Friends panel — online green dot + green invite state (`InviteButtonGreenRound06` vs `OfflineButtonDarkRound06`).

**Phase 2 — Imagegen plate regen (only where a native brush is insufficient)**
Regen blank plates one family at a time: front-on orthographic, alpha-clean transparent PNG, no labels/scores/fake text, all states, local contact sheet + slice spec. Many of these (outline rows, rounded checkbox, gloss CTA) can be pure native Slate brush/material and may not need a plate at all — prefer native first.

**Phase 3 — Implement one slice first**
Rebuild the single worst offender natively before touching the rest. Recommend **topbar icon buttons** (`MakeIconActionButton` in `T66FrontendTopBarWidget.cpp`) since icon-on-icon is the clearest ownership failure and validates the plate-vs-glyph separation rule. Touch points by family:
- `T66FrontendTopBarWidget.cpp`: `MakeIconActionButton`, `AchievementsRect`, `CategoryTabFontSize`, tabs.
- `T66MainMenuScreen.cpp`: `MakeCtaButton`, `MakeFriendRow`, `MakeCtaStack`, `MakeLeftPanel`.
- `T66FlatLeaderboardPanel.cpp`: `BuildLeaderboardRow`, `BuildMetricCheckButton`.
- `T66FriendslopStyle.cpp`: `GetChromeBrush` descriptor + draw-mode edits.

**Phase 4 — Replace the verifier gate**
Retire "PASS=251 is enough." Add a blocking visual scorecard / contact sheet (per `UI_FIDELITY_LOOP_INSTRUCTIONS.md`) with explicit PASS/FAIL rows per component for: color, shape, state coverage, content ownership (plate vs live), and text-fit. Pillow/OpenCV/skimage used only to measure/verify captures, never to author runtime art.

**Phase 5 — Capture & decide**
New capture proves the single native slice only. If the slice still fails visually, fix the method before scaling to remaining families.

**Acceptance criteria:** per-component scorecard all-PASS; no icon-on-icon; achievements label fits at min width; CTA has clean native gloss with no masked center; leaderboard row = red outline/dark interior; checkbox = uniform rounded square; friends panel shows green dot + green invite; no clipped/overflowing controls inside parents.

**Risks:** scope creep back into full-screen mockups; treating a native brush gap as an excuse to re-enable inpaint; descriptor edits regressing other screens sharing the enum; material-based gloss cost/complexity vs layered brush.

## Evidence Checked
- Confirmed all six code surfaces exist: `T66FriendslopStyle.h/.cpp`, `T66FrontendTopBarWidget.cpp`, `T66MainMenuScreen.cpp`, `T66FlatLeaderboardPanel.cpp/.h`.
- Confirmed all four instruction docs exist.
- Read `ET66FriendslopChrome` enum (50+ descriptors incl. per-family Round06 + Red/Dark/Green/Checkbox state variants) — the token system the plan extends.

## Questions Or Blockers
None requiring the user. Direction (freeze inpaint, native-first, one-slice) is already chosen by the user in the original assessment; Codex can finalize internally.

## Caveats
- I did not open the function bodies (`MakeIconActionButton`, `BuildLeaderboardRow`, etc.) or the instruction docs' full text — plan targets are inferred from the contract and enum names; Codex should confirm exact signatures before writing the spec.
- Whether each offender needs a regen plate vs a pure-native brush is a per-family judgment; default to native first to avoid unnecessary imagegen cycles.
- "First slice" target (topbar) is a recommendation; CTA smudging is also a strong candidate if the user weights it as the most visible failure.

</validator_independent_answer>

<codex_draft>
You are right to call out the wording. For FriendslopStyle, "rebuild the button" does not mean "stop using imagegen for the button art." The actual production split is:

- imagegen or Photoshop-equivalent source-art work produces the blank, clean, reference-matched chrome plate family for the button/panel/row/etc.
- Slate/UMG turns that plate into the real runtime component: live text, live icons, live data, padding, fit rules, states, click behavior, tags, containment, and verification.

So the corrected plan for the next Main Menu version is this:

1. Freeze the bad asset method.
   - Do not use pass12 crop/inpaint output as a base for pass13.
   - Keep pass12 only as diagnostic evidence of what failed: cut-off middles, painted-over labels, baked glyphs, wrong shapes/colors, and containment misses.
   - Pillow/OpenCV/skimage are allowed for alpha cleanup inspection, crop sheets, fit measurements, and verifier/contact sheets. They are not the production tool for removing text from screenshots or inventing final button interiors.

2. Write the pass13 component contract before touching code or assets.
   - Artifact: `UI/FriendslopStyle/Elements/main_menu_pass13_component_contract.md`.
   - For each component family, define:
     - reference crop path and intended runtime tag
     - imagegen/source-art ownership: blank plate, static decorative marks only, no live labels/data/glyphs
     - Slate ownership: text, icons, live player/friend/score data, state, layout, hit rect
     - exact normal size, min size, max/wide behavior, content padding, font size, icon size, z-order
     - state family required: normal, hover, pressed, selected, disabled, online/offline where applicable
     - expected visual properties: fill, outline, gloss, bevel thickness, corner/cap shape, shadow, material density
     - verifier crop name and PASS/FAIL criteria
   - Families in scope for first implementation slice:
     - topbar icon buttons
     - achievements tab
     - CTA primary and secondary buttons
     - leaderboard local row
     - high-score checkbox
     - friends online header, invite/offline buttons, and friend rows

3. Regenerate or author clean chrome families, not screenshot-derived fixes.
   - Source path: `SourceAssets/UI/FriendslopStyle/MainMenu/Pass13/`.
   - Prompt path: `UI/FriendslopStyle/SourcePrompts/MainMenu/pass13_*.md`.
   - Runtime path after acceptance: `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/`.
   - Rule per generated family:
     - front-on orthographic chrome only
     - transparent alpha or removable matte only as intermediate
     - no text, fake text, labels, names, scores, icons, skulls, stars, search glyphs, or baked player data unless the contract explicitly says a decorative glyph is part of the plate
     - generate matched state variants together, not one-off mismatched pieces
     - create min/normal/wide contact sheet before Unreal import
     - reject any plate with visible text-smear, center masking, split/cut middle, baked glyph contamination, or mismatched material density
   - For this reference, the CTA buttons, topbar tabs, rounded icon plates, leaderboard row shell, checkbox, invite/offline buttons, and friend row shells should be treated as authored/generated chrome plates unless a native-only construction can pass the side-by-side crop gate. Slate-native composition is the runtime method, not a replacement for the premium plate art.

4. Update the Friendslop style registry and slice specs.
   - Code surfaces:
     - `Source/T66/UI/Style/T66FriendslopStyle.h`
     - `Source/T66/UI/Style/T66FriendslopStyle.cpp`
   - Add or replace descriptors for pass13 role-specific assets instead of reusing contaminated generic ones.
   - Use `DrawAs=Box` and real nonzero margins only for assets that pass slice tests.
   - Use size-specific/fixed plates where slicing destroys highlights or caps.
   - Create/refresh `UI/FriendslopStyle/SliceSpecs/main_menu_pass13_slice_specs.md`.
   - Specific cleanup:
     - topbar icon backgrounds must be blank plates if live glyphs are rendered on top
     - CTA plates must be clean blank button bodies if live skull icons/text are rendered on top
     - leaderboard local row must become red outline plus dark interior, not red-filled interior
     - checkbox checked/empty must be uniform rounded square states

5. Implement the runtime component slice in targeted code.
   - `Source/T66/UI/T66FrontendTopBarWidget.cpp`
     - `MakeIconActionButton`: one owner for glyphs. Use blank generated plate plus live glyph, or baked glyph plate with no live glyph. For this pass choose blank plate plus live glyph.
     - `AchievementsRect` and `CategoryTabFontSize`: replace fixed text assumptions with a measured fit rule for the `ACHIEVEMENTS` label, including min-width/compact fallback.
   - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
     - `MakeCtaButton`: use clean CTA plate, live text, live skull slots if needed, no plate with skull/text remnants under live content.
     - `MakeSearchField`: use a deliberate search glyph or no glyph, not a question-mark placeholder if the reference expects search.
     - `MakeFriendGroupToggle`: add online green status dot and keep header text/count live.
     - `MakeFriendRow`: lock row width/height inside left panel content bounds; use live avatar, name, level, star, and action button; green invite plate for online, dark offline plate for offline.
   - `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
     - `BuildLeaderboardRow`: make local row red outline/dark interior with live rank/name/score.
     - `BuildMetricCheckButton`: use uniform checked/empty rounded-square assets and stable box size.
     - `BuildFilterButton`/dropdowns: enforce icon ownership and text fit the same way as topbar.

6. Replace the verifier gate for this pass.
   - Artifact: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass13_material_state_scorecard.md`.
   - A numeric `VerifyUIFidelity.py` PASS count is not sufficient.
   - Required scorecard rows:
     - topbar icons: no icon-on-icon, correct plate shape, correct glyph owner
     - achievements tab: text fits and no clipped/overflowing label
     - CTA buttons: no masked center, no smudged remnants, correct red/dark rubber material
     - leaderboard row: red outline and dark interior
     - checkbox: uniform rounded square checked/empty states
     - friends header: online green dot present
     - invite/offline buttons: green invite and dark offline states visible, text fits
     - friend rows/leaderboard rows: contained inside panel body
   - Each row must include reference crop, current crop, verdict, and reason.

7. Capture only after the component slice is integrated.
   - Run focused compile first.
   - Capture through the Unreal-owned UI capture path, not desktop screenshots.
   - Produce:
     - fresh capture PNG
     - fresh dump JSON
     - verifier report
     - side-by-side crop/contact sheet
     - visual scorecard
   - The first pass13 capture is allowed to prove only this component slice. If the slice still has the same method failure, stop and fix the method before expanding to the rest of the screen.

8. Acceptance criteria for the next version slice.
   - No screenshot-inpaint production assets.
   - No baked live text/icons underneath live text/icons.
   - No manual bars, painted-over centers, or smudged masks.
   - No cut-in-half chrome or missing middle bands.
   - Text fits the component at the declared minimum size.
   - Rows and child controls fit inside their parent panels with containment checks.
   - The specific color/shape/detail discrepancies called out by the user are corrected, not averaged away.
   - Visual scorecard is `Result: PASS` for the component slice before scaling the approach.

9. What I will not do in the next implementation pass.
   - I will not generate another full-screen reference and paste/crop pieces out of it.
   - I will not inpaint text out of screenshots and call the result a clean component.
   - I will not cover bad baked labels with a manual bar or overlay.
   - I will not accept `PASS=251` or compile success as proof of visual fidelity.
   - I will not implement the whole screen before the first component slice proves the production method.

</codex_draft>
