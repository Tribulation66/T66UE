# FriendslopStyle Main Menu Failure Review Draft

## Status

The pass09 Main Menu result is rejected. The previous `DONE` and Claude pass4 `OK` were false accepts.

Posthoc invalidation evidence:

- Invalidated capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_capture.png`
- Reference: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
- Invalidated scorecard: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass09_visual_scorecard.md`
- Posthoc verifier: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFailureRetrospective\pass09_posthoc_verifier.md`
- Posthoc verifier result: `PASS=252 FAIL=1 UNSURE=0`, with the failure on the holistic visual gate.
- Note: the original structural run reported `PASS=253 FAIL=0`; the `FAIL=1` above comes from the posthoc holistic visual gate after the scorecard was correctly invalidated. The original tooling did not catch the visual mismatch by itself.
- Attribution note: pass09 was layered on broad pre-existing Friendslop/UI worktree changes, so the failed visual output is not cleanly attributable to one isolated code edit. The failure is the approach and acceptance pipeline, not just one constant change.

## What Failed

The user is correct: the quality gap is not a small-adjustment problem.
The user's mandate is an approach change, not another round of tweaks.

- The top bar shares red/dark colors with the reference, but its scale, material weight, text legibility, ticket badge, and button proportions do not match Round06.
- The left social rail is compressed. Rows, labels, avatars, action buttons, and section headers do not read like the clean contained reference rows.
- The title is not the same visual artifact. A layered Slate text approximation cannot currently reproduce the raster reference's thick cream/red logo weight.
- The right leaderboard passes coordinates but not visual identity: faint headers, sparse body, weaker controls, and a different density from the reference.
- The generic pilot plates do not match the reference at runtime sizes. They are rubber-like atoms, not production component plates.

## Root Causes

1. **The pass measured layout bands instead of visual identity.** `PASS=253 FAIL=0` proved tagged widgets were near expected coordinates. It did not prove the screen looked like Round06.

2. **The visual scorecard was self-authored acceptance.** The Operator wrote the scorecard while also trying to get the implementation to pass. That let broad visual mismatch be reframed as residual deltas.

3. **Claude was anchored by the packet framing.** Claude pass4 read a packet that already argued the screen was materially complete, then reduced visible failures to "soft deltas." The validator did not perform a truly blind first-glance gate.

4. **The asset strategy was wrong for this quality bar.** `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`, the asset registry, and the slice specs all warn that generic blank atoms are acceptable only if they match the approved reference at runtime size. Pass09 reused the generic pilot sheet without proving that.

5. **No deterministic reference fixture existed.** The capture used current live friend/leaderboard data. Fidelity against a static reference needs a deterministic visual fixture with reference-like friend counts, row topology, leaderboard row, and ticket value, plus a separate current-data overflow sanity capture.

6. **Containment checks were too shallow.** The report checked many parent boxes, but it did not veto the way live row content, labels, and buttons visually crowded or escaped the intended row/panel interiors.

## Replacement Approach

### 1. Freeze The Current Result As Failed

Do not continue from pass09 as a near-pass. Treat it as a failed prototype that proved the old approach cannot meet the bar.

Before any new implementation pass:

- `friendslop_pass09_visual_scorecard.md` remains `Result: FAIL`.
- Any final report citing pass09 must say it is invalidated.
- Claude pass4 is kept as evidence of reviewer anchoring, not as acceptance evidence.

### 2. Use A Blind Visual Gate

The validator must inspect the reference and capture before reading the Operator scorecard or narrative.

Required reviewer output order:

1. First-glance verdict: `SAME_SCREEN_YES` or `SAME_SCREEN_NO`.
2. Top 5 visible mismatches, with no reference to Operator claims.
3. Only after that, read the scorecard and cross-review the Operator packet.

If the first-glance verdict is `SAME_SCREEN_NO`, the result is `FAIL` regardless of structural PASS count.

### 3. Make The Next Pass Asset-First

Do not start by adjusting Slate constants. Start by replacing the visual carrier set.

For each load-bearing region, create a reference-crop-backed blank plate or size-specific family:

- topbar strip
- settings/language icon button
- account/home/power-up/achievements tabs at their actual reference sizes
- ticket badge and power button
- left panel shell
- profile row
- search field
- online/offline section headers
- online/offline friend rows
- green invite and dark offline pills
- party slot
- title treatment decision
- Enter CTA and Load CTA
- right filter icon buttons
- right leaderboard panel shell
- weekly/all-time tabs
- dropdown shells
- metric checkboxes/labels
- table header band
- selected ranking row

Each plate must be compared against the matching Round06 crop at its target runtime size before Unreal integration. If it does not match silhouette, bevel thickness, shadow padding, highlight shape, material density, and edge treatment, it fails.

### 4. Stop Stretching Generic Atoms Into Production UI

9-slice and 3-slice are allowed only after a min/normal/wide contact sheet proves the specific plate survives scaling. If a component is high-visibility or size-sensitive, use a size-specific plate.

Practical consequence: the current `panel_large_dark.png`, `button_primary_red.png`, `row_dark.png`, and `pill_dark.png` can remain as prototypes, but they cannot be acceptance assets unless they pass the new reference-crop gate. Most likely they need replacement.

### 5. Add A Deterministic Fidelity Fixture

A reference-fidelity capture cannot depend on current local Steam/backend data.

Add or use a deterministic automation fixture for Main Menu visual capture:

- one online friend row in the same topology as Round06
- four offline rows or a matching reference-equivalent count
- local profile/ticket value matching the reference capture where possible
- one visible leaderboard row with rank/name/score topology
- stable avatars or approved content stubs

Then run two captures:

- **Reference fixture capture:** must match Round06 visually.
- **Current live-data capture:** must prove no overflow, clipping, or unreadable row content under current data.

Only the fixture capture is allowed to decide reference visual parity. The current-data capture decides robustness.

### 6. Replace The Scorecard With A Blocking Matrix

The scorecard cannot be narrative-only. It needs one row per element with these required columns:

- reference crop path
- runtime crop path
- chrome asset path
- target runtime size
- size verdict
- position verdict
- material verdict
- containment verdict
- legibility verdict
- final verdict

Rules:

- Any element row `FAIL` means `Result: FAIL`.
- `USER_ACCEPTED_DELTA` is allowed only with a linked user decision.
- No `PASS with residual delta` phrasing.
- The Operator cannot self-finalize `Result: PASS`; validator must provide the blind first-glance verdict first.

### 7. Tighten Containment And Legibility Gates

Add structural checks that can catch the obvious failures:

- child content contained inside row/header/panel insets, not just parent box existence
- no sibling overlap inside row content groups
- min text pixel height for topbar, ticket, row labels, leaderboard headers, CTA labels
- min contrast for live text over plate surfaces
- action buttons fully inside row bounds
- friend row body count and row heights match the fixture reference topology
- leaderboard row/header visible and inside the panel content area

### 8. Decide The Title Mechanism Up Front

If the target is exact Round06 visual parity, the `CHADPOCALYPSE` title probably needs a dedicated title-logo asset or a much stronger custom material/text renderer. Plain Slate text layers are not enough.

This is the only art-policy decision before implementation:

- Option A: allow a static title-logo PNG for `CHADPOCALYPSE` because it is branding, not live data.
- Option B: keep it live text, but then the title cannot be judged against the raster reference as exact parity.

Do not hide this as a residual delta.

## Revised PPF Check For Continuing The Old Method

```text
PPF CHECK
Objective: Continue from pass09 by tuning layout/type constants over the existing generic pilot plate sheet.
Proven process: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md
My planned implementation: Keep generic pilot atoms and make more Slate adjustments.
Same method class: NO
If NO, why: The approved process requires reference-matched transparent PNG plates or plate families at runtime size. Pass09 proved generic reusable atoms and coordinate tuning cannot reproduce the Round06 material quality or row/control hierarchy.
User approval required before proceeding: YES
Verification evidence: User visual rejection, Claude independent failure review, invalidated scorecard, and posthoc verifier FAIL on the holistic visual gate.
```

## Revised PPF Check For The Replacement Method

```text
PPF CHECK
Objective: Rebuild FriendslopStyle Main Menu from reference-matched per-element/per-size plates, deterministic fidelity fixture data, and a blocking blind visual gate.
Proven process: UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md plus UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, with the generic pilot-plate loophole closed.
My planned implementation: Author plate families from Round06 crops before Unreal integration, prove each plate in a source/runtime contact sheet, use deterministic fixture data for reference capture, verify structural containment and legibility, then require blind Codex/Claude visual PASS before reporting done.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: YES, because this supersedes the previous implementation strategy and may require new imagegen workers plus a title-logo policy decision.
Verification evidence: plate contact sheets, fixture capture/dump, live-data robustness capture, VerifyUIFidelity report, blocking scorecard matrix, and blind Claude cross-review.
```

## Immediate Next Step If Approved

Do not edit the Main Menu UI again first. The first implementation step should be a new planning artifact:

`UI/FriendslopStyle/Elements/main_menu_round06_production_plate_plan.md`

It should enumerate every required production plate, source crop, intended runtime size, whether it is size-specific or sliceable, and whether it needs imagegen cleanup. Only after that plate plan is reviewed should image generation or Slate implementation resume.
