I have sufficient grounding. The prompt is a Validator request for an independent advisory answer — it asks me to identify constraints, file areas, pitfalls, and required evidence, not to make a decision only the user can make.

Result: OK

## Independent Answer

This is a well-scoped two-part pass that Codex can execute internally. Sequence it as: (1) regenerate the reference image with the three requested changes via a separate local Codex CLI imagegen worker, archive/promote it as the new Current, then (2) run a full five-family FriendslopStyle Main Menu iteration against the new reference.

**Reference regeneration (Part 1) — the three changes, stated precisely so the worker prompt is unambiguous:**
1. Right-side layering: leaderboard panel + toggle panel must be clearly foreground; the statue/background must sit behind them. Phrase this as a compositing/z-order instruction, not a "move the panel" instruction — the prior worker prompt (`Current/prompt.md`) carried forward composition verbatim, so be explicit that the panel now occludes the statue.
2. Replace GLOBAL/SOCIAL/STREAMERS **text** in the small top filter panel with **icons** (e.g., globe, friends/people, broadcast/streamer glyph). Keep it a separate rubber sub-panel above the leaderboard.
3. Topbar coupon icon → a classic fair/carnival ticket: perforated edge, notched ends, "ticket/coupon" silhouette, not an abstract yellow mark.
- Keep everything else (style, topbar, left social panel, center title/CTAs, statue, starfield, equal-width right panel, toggle-above-CLI layout) unchanged.

**Implementation iteration (Part 2):** evaluate all five families (TopBar, LeftSocialPanel, RightLeaderboardPanel, CenterButtonStack, Background); for each visual-FAIL family launch exactly one imagegen worker that produces assets for all failed elements in that family; implement generated assets; then run the layout/sizing-fitting gate and the wiring/functionality gate; capture/dump/contact evidence; report process coverage + wiring PASS/FAIL only.

**Likely areas to inspect:**
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (authority file — owns the worker-record contract and the "process coverage + wiring PASS/FAIL only" reporting exception).
- `UI/FriendslopStyle/Screens/MainMenu/` (component_contract_current.md, element_manifest.md, geometry.md, slice_specs, production_plate_plan) — these define the family/element breakdown and the topbar coupon + filter-toggle elements.
- `UI/FriendslopStyle/Reference/MainMenu/` (manifest.md, README.md) and `Archive/ReferenceIterations/MainMenu/` for promote/archive convention.
- The MainMenu Slate screen source (`Source/T66/UI/Screens/MainMenu...`) for where the coupon icon and GLOBAL/SOCIAL/STREAMERS toggles are wired, and the runtime texture/brush access path for swapping text→icon assets.
- `Saved\Codex\UI\FriendslopStyle\MainMenu\` for worker output + pass-log location.

**Likely pitfalls:**
- **Reference z-order in a flat raster:** "bring panel to front" is a generation instruction in the mockup but a *layout/occlusion* concern in runtime Slate — don't conflate. In runtime, the statue is Background family; the leaderboard must draw above it. Verify in the dump/capture, not just the regenerated PNG.
- **Icons baked into reference vs. runtime:** the reference may *show* icons, but runtime must use live `SImage` icon assets (one worker-generated icon per filter), not a crop of the reference. Same rule for the coupon icon.
- **Worker discipline:** account-backed built-in imagegen only — no OPENAI_API_KEY/API/web/manual-paint fallback. One worker per failed family. Each worker must record request, start time/run id, input paths, logs/transcript, final status, output PNG path, token count + SHA-256 when available, and emit IMAGE_SAVED/IMAGE_FAILED.
- **Reporting scope:** report process coverage + wiring/functionality PASS/FAIL only — no FULL/PARTIAL, no visual scorecard Result. Visual final acceptance is user-owned.
- **No git/release ops, no native goal tools, no main-chat imagegen.**

**Evidence the final answer should include:**
- New reference PNG path + SHA-256, archive of old reference, confirmation it was promoted to Current, and the worker record for the reference regeneration.
- Per-family visual PASS/FAIL assessment for all five families; for each FAIL family, the worker record + generated asset paths + confirmation they were implemented.
- Fresh capture + dump + contact-sheet paths from a single scripted run.
- Layout/sizing-fitting result and wiring/functionality PASS/FAIL per family.
- The new reference image sent to the user for review (the user explicitly asked for it).
- Pass-log entry; any hard blocker documented if a step couldn't complete.

## Evidence Checked
- `Current/prompt.md` — confirms the prior worker prompt format, the equal-width/toggle-above layout, and the imagegen-source restrictions to carry forward.
- `UI/FriendslopStyle/` and `Screens/MainMenu/` directory listings — confirm the family/element doc set and reference/archive structure exist.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` (§2.4, §3.x, §5, Step 5 generated-raster process) — confirms the FriendslopStyle exception, the five-family/one-worker-per-failed-family process, dump/verify/contact evidence, and worker-record requirements.

## Questions Or Blockers
None that require the user. The three reference changes are concrete and the process is fully specified by the authority docs. Codex can proceed. (No required tool is unavailable from the evidence seen; if the local CLI imagegen worker cannot run, that would be a hard blocker to document, not a user decision.)

## Caveats
- I did not open `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` or the MainMenu component docs line-by-line — Codex should confirm the exact worker-record fields and the precise archive/promote path convention against the authority file before executing.
- Final visual acceptance of the regenerated reference and the implemented screen is user-owned; Codex should present, not self-certify, the visual result.
- "Bring leaderboard to front" must be handled in both artifacts (reference compositing and runtime draw/occlusion order); a fix in only one is incomplete.
