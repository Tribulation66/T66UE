Result: OK

## Independent Answer

The user has explicitly authorized the full run ("Go ahead with the full implementation… the full thing"), and the authority file `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` is already marked "Approved for the Main Menu pilot on 2026-06-05" with Round06 confirmed as the pilot reference. So no user-only decision blocks the start — Codex can execute this internally and stop only if a hard CLI/Unreal blocker appears. This is OK to proceed.

Codex should run two phases in order:

**Phase 1 — Reference update (do not skip the archive).**
1. Archive the current authoritative reference. The next round is `Round07` — `UI/FriendslopStyle/Archive/ReferenceIterations/MainMenu/` currently goes through Round06, and the live `Current` reference (`main_menu_reference_01_current_capture_stronger_rubber_cli.png`) is the Round06 capture. Move the Current contents (PNG, `prompt.md`, `manifest.md`, `README.md`) into a new `Round07` archive folder before overwriting.
2. Generate the new full-screen reference via a **fresh local Codex CLI worker** using account-backed built-in imagegen (the `codex --ask-for-approval never exec … --image …` shape in Section 2.2.1, global flag before `exec`). Prompt must: preserve current layout, switch statue/background to a rubbery Fall-Guys-like material, and fix the CHADPOCALYPSE title artifact/spelling. Attach the prior reference and/or fresh capture as `--image` context. Record full worker provenance (request, start time, stdout/stderr, last_message, output path, SHA-256, token line, IMAGE_SAVED).
3. Refresh `Current/manifest.md`, `prompt.md`, `README.md` to point at the new image.

**Phase 2 — One complete five-family iteration** through Section 11 / Step I ordered loop:
- PPF gate, Artifact Parity gate, Mechanism Manifest written first (Sections 5–7).
- Fresh current capture + dump (Step A) via `CaptureT66UIScreen.ps1 -Screen MainMenu`.
- Geometry extraction/overlay against the **new 1672×941** reference normalized to 1920×1080 (do not assume the PNG is already 1080p).
- Assess all five families (`TopBar`, `LeftSocialPanel`, `RightLeaderboardPanel`, `CenterButtonStack`, `Background`). User expects none of the current elements are acceptable → be strict, mark FAIL on doubt. One CLI worker per failed family; generate blank standalone plates (no baked text/data).
- Implement every regenerated asset → sizing/fitting pass → wiring/functionality PASS/FAIL pass → compile → capture/dump/contact evidence → report.
- Apply the known targeted fixes: black/dark Load Game, green Invite, remove hard/double bevel, rounder rubber 3D buttons, thin/simple panel outlines, more padding in left subpanels and right leaderboard, remove coupon yellow lines, reflect new rubber statue background, preserve Lilita One.

Final status reports objective process coverage + wiring/functionality PASS/FAIL only — never FULL/PARTIAL or VerifyUIFidelity as visual acceptance. User owns visual sign-off.

## Evidence Checked
- `original_prompt.md` (task contract, scope, stop condition).
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (authority file; approved 2026-06-05; Sections 2.2 worker contract, 11 pipeline, 12 pilot notes, 13 completion close).
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` (process spine, generated-raster carve-out at 2.3/2.4, Step 5 ordered visual loop).
- `UI/FriendslopStyle/Reference/MainMenu/Current/manifest.md` (Round06 reference is current authoritative; 1672×941; CLI-worker provenance pattern).
- Archive layout via Glob (`Round01`–`Round06` exist; next is `Round07`).

## Questions Or Blockers
None requiring the user right now — the user already approved and said proceed. Codex should self-block and produce a user-review packet only if: a CLI imagegen worker fails non-transiently (after a fresh-worker retry on TooManyRequests/auth), a failed family can't be sent to a worker, generated assets can't be implemented, or a sizing/wiring blocker can't be fixed internally.

## Caveats
- **Archive before overwrite.** Confirm `Round07` is the correct next slot and that the existing Round06 Current is preserved before regenerating — losing the authoritative reference is the main irreversible risk in Phase 1.
- **No git operations** and **no native goal tools** per scope — Codex's "commit" steps in the loop doc must be treated as no-ops here.
- **Imagegen route is strict:** account-backed CLI worker only; no main-chat imagegen, no `OPENAI_API_KEY`/API scripts, no web URLs/browser screenshots, no reuse of old `generated_images` folders. The new reference and all runtime plates must each have an auditable worker record.
- **Stale FlatStyle checklist** (`UI/Checklists/main_menu_checklist.md`) must not be used as evidence; author/refresh `UI/FriendslopStyle/Screens/MainMenu/checklist.md` from the new geometry/dump.
- One generation batch per failed family per iteration — don't re-loop regeneration within the same pass; report remaining visual gaps honestly for the next user-directed iteration.
- Final report must not claim visual acceptance; show reference + capture/contact sheet and let the user decide.
