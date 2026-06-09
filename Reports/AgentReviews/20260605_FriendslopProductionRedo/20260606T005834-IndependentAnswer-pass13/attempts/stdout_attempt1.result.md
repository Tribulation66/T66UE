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
