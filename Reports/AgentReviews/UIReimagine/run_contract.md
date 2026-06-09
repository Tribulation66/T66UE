# UI Reimagine — Overnight Run Contract

Date agreed: 2026-06-10. Status: QUEUED (waiting on: in-flight UI pass commit, then
cleanup pass execution, then user's "go" + north-star approval).

## Mission

Regenerate every player-facing screen, reimagined: new reference image per screen via
Codex CLI imagegen, elements generated and implemented through the (improved) Friendslop
process, leveling up the entire UI/HUD in one overnight run (+ follow-up night for tail).

## Locked decisions (user, 2026-06-10)

1. **Art direction: Bright candy / Fall Guys-true.** Anchor = the minimap candy direction
   (bright teal/coral/yellow inflatable plates, light surfaces, chunky dark text via
   Lilita One, bouncy + colorful). This REPLACES the dark/red Friendslop identity
   everywhere. In-run HUD gets a slightly calmer variant for combat readability.
2. **Latitude: reskin + light recomposition.** References may resize/rearrange within
   reason; implementation preserves every handler, data binding, and screen flow.
   Structural changes only where cheap.
3. **Scope: redo EVERYTHING** including hand-migrated screens (MainMenu Round06,
   AccountStatus tabs, Achievements, PowerUp). Old plates stay in git for rollback.
4. **North star approved pre-sleep.** Night-of sequence: generate style bible + 2
   full-screen north-star references (HUD + Main Menu) first (~20-30 min), user gives a
   5-minute yes/adjust, THEN the autonomous run starts. No further approval gates.

## Night-of execution order

0. Precondition: in-flight UI pass committed; then execute the two deferred cleanups
   (coverage_audit.md item 2 + retro_removal_spec.md, ~1h with verification) so the
   regen builds on clean plumbing.
1. Style bible (palette/material/typography sheet) + 2 north-star refs -> USER APPROVAL.
2. Shared component kit regen in the new direction (panel, buttons+states, tabs, dropdown,
   rows, pills, checkbox, slider, progress track/fill, modal shell, tooltip, scrollbar,
   input field) with alpha-bounds + slice min/normal/wide + no-baked-text QA gates.
   Wire into FT66FriendslopStyle descriptors -> whole game inherits instantly via the
   choke-point flip. Commit checkpoint.
3. Per-screen passes, player-exposure order: HUD -> MainMenu -> HeroSelection -> Pause ->
   Vendor/Casino -> in-run overlays (idol/lab/crate/collector/loot wheel) -> Settings ->
   SaveSlots/RunSummary/GameOver -> modals -> tail (Challenges, grids, LanguageSelect,
   CompanionSelection, loading, etc.). Per screen: baseline capture -> reimagined
   reference (anchored on bible + current capture for layout) -> Claude vision review vs
   bible (regen once max) -> generate ONLY screen-specific elements the kit doesn't cover
   (backgrounds, title/hero art, special panels, icons) -> implement -> batched stage ->
   capture -> triage (1-2 iteration cap) -> commit per screen.
4. Morning packet: per-screen reference/before/after contact sheets, coverage ledger,
   deferred list, heartbeat log throughout.

## Process improvements over the documented loop (authorized: "follow/improve the process")

- All imagegen workers parallel (doc logs 1h20m/iteration serial as a known problem).
- Claude vision pre-screen kills bad candidates BEFORE implementation cost.
- Automated QA: alpha-bounds check, slice integrity at min/normal/wide, baked-text scan,
  worker records per asset (auditability preserved).
- 3-track pipeline: imagegen for screen N+1 runs while screen N stages/captures.
- Guardrails: per-screen time box, defer-don't-stick (same failure twice -> revert + defer),
  per-screen commits on branch `ui-reimagine`, no chat status (heartbeat log), no
  AskUserQuestion after the north-star gate.

## Honest throughput expectation (set with user)

One night = bible + full kit + top ~8-12 surfaces done properly; the tail inherits the
new kit immediately (nothing looks old, just not yet bespoke). Follow-up night finishes.

## Standing constraints

- Codex CLI account-backed imagegen only (no API keys, no browser).
- Live text/data never baked into raster art.
- git add only files this run touches (user's unrelated work may be in tree).
- Friendslop process artifacts: keep worker records + registry updates for accepted assets.
