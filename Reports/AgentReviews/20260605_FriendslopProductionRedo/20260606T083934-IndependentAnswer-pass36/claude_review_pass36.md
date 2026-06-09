Result: OK

## Independent Answer

This is implementable by Codex internally. The repo already contains the proven mechanisms the user is asking to "officialize" — a CLI worker pattern (`worker_processes.json` + `workers/<id>/request.md` + `logs/*.stdout/stderr.log` + `last_message.txt`, producing `_cli.png` outputs at `WorkerTokensSpent`-tracked cost) and a full per-screen pipeline in the authority doc. The task is encoding two rules and writing a handoff prompt, not making any user-only decision.

**1. Which active docs Codex should edit**

- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — the single authority. This is where both rules belong (§8 ImageGen Rules, §2.1 Visual Asset Authorship Authority, §11 Per-Screen Pipeline Steps D/E/I/J).
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` — only if a cross-reference is needed (§2.4 already references imagegen for content stubs). Keep edits here to a one-line pointer back to the authority doc to avoid a second source of truth.
- Do **not** edit `AGENTS.md`, `UI/UI_AGENTS.md`, or the FlatStyle docs for this — the contract scope is FriendslopStyle process only. A `UI/UI_AGENTS.md` router amendment is already separately flagged in the authority doc §1 and is not part of this task's stop condition.

Avoid creating a new doc; the rules say the authority file is the single FriendslopStyle process source.

**2. Full-screen element inventory loop — rule shape**

Encode as an explicit, every-pass, every-element enumeration in §11. Shape:

- Each iteration must walk the **complete element manifest** (`Screens/<Screen>/element_manifest.md`) — not just elements that visibly regressed. Every row is evaluated every pass.
- Each element gets one verdict per pass: `PASS` / `REGENERATE` (asset visual failure) / `REBUILD` (geometry/containment/material/content/interaction failure) / `DEFER` (with reason).
- Verdicts route to the existing §11 Step I failure taxonomy (asset-visual → regenerate via imagegen; packaging → recrop existing approved pixels only; geometry/containment/material/content/interaction → Slate/code fix).
- The pass output must report a **count**: total elements, how many `REGENERATE`, how many `REBUILD`, how many `PASS` — matching the user's "telling me of the elements how many had to be regenerated."
- Close each iteration with the §10/§11 holistic gate (Step J) producing the side-by-side reference-vs-product contact sheet + visual scorecard.

This is an additive tightening of the existing per-element manifest (Step D) and correction loop (Step I), so it stays consistent with the doc.

**3. Codex CLI imagegen worker — rule shape**

Formalize the *already-used* worker pattern as mandatory, not a new tool:

- All iteration image generation (reference and runtime chrome) MUST run through a local Codex CLI worker process, **not** interactive chat in the Codex app.
- Account-backed built-in imagegen only. **No `OPENAI_API_KEY`, no API scripts, no Pillow/OpenCV pixel synthesis** (consistent with §2.1 / §2.3 authorship authority).
- Each worker run must record the existing artifact set: a `request.md` (prompt), stdout/stderr logs, `last_message.txt`, and a `worker_processes.json` entry capturing `Id`, `Output` path, `Result` (e.g. `IMAGE_SAVED`), and `WorkerTokensSpent`.
- Outputs land under the established paths (`SourcePrompts/<Screen>/`, `SourceAssets/UI/FriendslopStyle/<Screen>/`, reference under `Reference/<Screen>/...`).
- State the manual exception path: a user-approved per-turn exception documented in the active screen contract (§2.1) is the only deviation allowed.

**4. Pitfalls for the fresh-chat handoff prompt**

- Don't let the fresh agent treat the approved full-screen reference as importable chrome (the doc's central failure mode — §2/§2.1).
- Don't let it skip the PPF / Artifact Parity / Mechanism gates (§5–7) before generating.
- Make the every-element enumeration explicit so the agent doesn't only patch visible regressions — it must report the regenerate count over the whole manifest.
- Require CLI-worker imagegen explicitly and forbid interactive-chat generation and any API-key/pixel-edit fallback.
- Require the final side-by-side (reference vs product) + visual scorecard with a `Result: PASS/FAIL` line before claiming done; `FULL` vs `PARTIAL` per §13.
- Point at the *current* artifacts only: Round06 reference, the fresh baseline named in the current manifest, and `Screens/MainMenu/checklist.md` — and explicitly warn off the stale `UI/Checklists/main_menu_checklist.md` (§12).
- Keep the prompt self-contained (fresh chat has no memory): include absolute reference path, native source res `1672x941` → normalize to 1920×1080, and the worker-artifact contract.

## Evidence Checked

- `.t66/operator-state.json` — confirms Codex=Operator, Claude=Validator.
- `pass18_process_inventory_cli_docs_prompt.md` — the prompt/contract.
- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — full read; §2.1, §8, §11 (Steps D/E/I/J), §12, §13 are the edit targets.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` §1–2.4 — visual scorecard gate, content-stub imagegen reference.
- `Archive/ReferenceIterations/MainMenu/Round06/worker_processes.json` — confirms the CLI worker pattern (`_cli.png`, request/logs/last_message, `WorkerTokensSpent`) already exists to be formalized.

## Questions Or Blockers

None requiring the user. Everything in the contract is implementable from existing repo evidence. The only adjacent open item — the `UI/UI_AGENTS.md` raster-chrome router amendment — is explicitly out of this task's scope and already separately tracked in the authority doc §1.

## Caveats

- I did not view the actual reference/capture images, so I cannot validate visual fidelity claims — only the process-doc structure.
- Keep edits concentrated in the single authority doc; if Codex adds the CLI rule to `UI_FIDELITY_LOOP_INSTRUCTIONS.md` too, make it a pointer, not a duplicated rule, to preserve single-source-of-truth.
- The contract's "basic text verification" should mean confirming the new rule text is present and internally consistent (e.g., grep for the new headings) — not a heavy review ceremony.
- 100+ files mention "CLI/imagegen"; most are archived worker logs. Codex should edit only the active authority doc, not retro-edit archived pass artifacts.
