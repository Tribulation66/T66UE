Result: OK

## Independent Answer

The user is asking for two parallel deliverables in this single Operator reply: (1) regenerate the Main Menu reference image with **only** the right-panel change described, and (2) a faithful recap of every issue raised plus a proposed solution per issue for the *next* iteration. No runtime implementation yet. None of this requires a user decision — the user already fully specified the reference change and the issue list — so Codex can execute internally. Result stays OK.

**Reference image change to apply (the ONLY edit this pass):**
- Right panel becomes the **same width as the left panel** (generate one panel, reuse for both sides).
- The three toggles (Global / Social / Streamers) move **out** of the leaderboard panel into their **own separate panel positioned above** the leaderboard panel.
- Leaderboard panel becomes **shorter** (to make room for the toggle panel above) and **wider** (to match left-panel width).
- Everything else stays identical to the current reference. Do not "fix" the rendering issues below in the reference — those are runtime-implementation problems, not reference-art problems.

**Recap of user issues + proposed solutions (for the next iteration, not this pass):**

1. **Cropping / gray bands (#1 priority).** Visible gray crop lines on weekly/all-time buttons, left & right panels, and the two center CTAs. Likely a systematic 9-slice/box-brush margin problem: slice insets cut into the bevel, or the source plate's transparent padding/edge pixels are being sampled. Solution: audit margins on every Slate Box/9-slice brush, re-derive slice insets from the plate (verify min/normal/wide survives), and check for premultiplied-alpha / edge-bleed in the plate. Fix once at the brush/slice spec layer rather than per-element.
2. **Pill-shaped vs. square (shape balance).** Top bar corners too pill-shaped; leaderboard elements too pill-shaped; but the two center CTAs over-corrected to too square. Solution: stop using forceful "square" language; target the reference's actual corner radii per element family instead of a global directive.
3. **Dropdowns — shape + functionality.** Runtime dropdown sits inside an outer square container; reference is just the dropdown button itself. Solution: remove the outer wrapper square so the button is the control.
4. **Leaderboard rows — cropping + internal sizing.** Row sub-panels (e.g., "Chadopino" card) have too much inner border and feel cramped/stuck. Solution: fix row sub-panel padding and the same crop issue from #1.
5. **Left/right panel borders too thick.** Reduce border thickness on the side panels.
6. **Background zoom/bend/outline.** Background statue is zoomed in too much vs. the reference, not bent/curved correctly, and wrong outline. Acknowledged as a harder technical problem to solve separately.
7. **Solo bro** — user said this one is fine; no action.

## Evidence Checked
- Current reference exists: `.../Reference/MainMenu/Current/main_menu_reference_02_rubber_statue_title_fixed_cli.png` plus `prompt.md`, `manifest.md`, `crops/`.
- Current worker prompt (`Current/prompt.md`) confirms the established CLI-worker pattern, input/output paths, and the "keep controls round, not square" art-direction note — consistent with the user's over-squaring complaint.
- `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` §2 confirms: references are art-direction only, runtime chrome must be authored plates via account-backed built-in imagegen in a **separate local Codex CLI worker**, and no baking of text/data into raster.

## Questions Or Blockers
None requiring the user. The reference change and issue list are fully specified.

## Caveats
- **Process risk (the real one):** the stop condition itself flags that an unavailable account-backed CLI imagegen worker is a hard blocker. If that worker cannot run, Codex must stop and surface it — do **not** fall back to OpenAI API, web image URLs, browser screenshots, old generated-image folders, or manual paint. That fallback failure would flip this to NEEDS_USER.
- Codex should write a new worker record + new Current `prompt.md`/`manifest.md` (don't silently overwrite the existing reference without provenance), per the worker-provenance rule.
- Keep the recap and the regenerate in the **same** reply as the user requested; the next reply is the iteration only.
- Ensure the new reference does **not** attempt to fix the cropping/border/zoom rendering bugs — those are runtime fixes; the only art change is the right-panel/toggle relayout.
