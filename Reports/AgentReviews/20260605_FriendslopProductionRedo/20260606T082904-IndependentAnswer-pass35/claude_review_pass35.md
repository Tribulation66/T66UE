Result: OK

## Independent Answer

Yes, the proposed change is sound and largely *completes* a direction the docs already lean toward. The core problem the user names — passes drifting toward whatever was last discussed (the title) and declaring victory while everything else stays untouched — is real and is exactly what these docs were already fighting (the checklist header at `Screens/MainMenu/checklist.md:9-11` records a prior `PASS=94 FAIL=0` that failed visually; the manifest note at `element_manifest.md:9-14` explicitly says only generic non-matching chrome was "done"). Formalizing a mandatory per-element classification every iteration turns that intent into an enforced gate.

**Recommended encoding (no edits made — for Codex/user to apply):**

1. **Make the element manifest the per-iteration ledger, not a one-time mapping.** In `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` Step D (`:368-386`) and Step I (`:443-468`), add a rule: *every* pass must restate the full element inventory and assign each element one status. Add a `Pass Status` column to `Screens/<Screen>/element_manifest.md` (currently a static table at `element_manifest.md:17-36`).

2. **Status vocabulary.** Use the user's four states but bind them to the existing failure taxonomy so they aren't new abstractions:
   - `PASS` — meets reference at runtime size, with evidence.
   - `REGENERATE` — asset visual/material failure → re-author via account-backed imagegen (maps to Step I "Asset visual failure", `:451-453`).
   - `REBUILD` — geometry/containment/interaction/wiring failure → fix Slate (maps to Step I geometry/containment/interaction).
   - `ACCEPTED_DELTA` — deliberate divergence, but only when recorded in Step B reconciliation (`:341-351`) and the scorecard's `USER_ACCEPTED_DELTA` (`visual_scorecard_template.md:11-12`).
   Note packaging-only fixes (recrop/alpha/slice) are a distinct Step-I category (`:454-457`); keep them as `REBUILD`-class packaging, not `REGENERATE`, to avoid forcing needless re-authoring.

3. **Acceptance gate.** Amend Section 13 / `FULL` definition (`:592-595`): a screen is `FULL` only when every manifest element is `PASS` or `ACCEPTED_DELTA`. Any `REGENERATE`/`REBUILD` remaining = `PARTIAL`. This is the "force the whole thing unless an element is perfect" rule.

4. **Light touch on README/component_contract.** The folder router (`README.md`) and `component_contract_current.md` should point to the manifest as the authoritative per-element ledger; don't duplicate the table.

**Safeguards (the user explicitly asked for these):**

- *Against endless loops:* keep the existing caps — escalate to a user review packet when the same failure survives two passes or the screen hits five passes (`:470-477`). The new rule must not override these; classification feeds them. Decreasing-FAIL-count requirement: if the non-PASS set doesn't shrink across two passes, escalate.
- *Against superficial bulk regeneration:* require a root-cause classification reason per non-PASS element (Step I already mandates "classify each failure before editing"). Forbid blanket "regenerate everything" — each `REGENERATE` needs a cited contact-sheet/scorecard reason, and regenerated assets must re-pass the slice/contact-sheet gate (`:265-268`, `:400-414`) before import. This stops churn-for-churn's-sake.
- *Cost guard:* one representative-brush smoke test still gates full-family authoring (`:408-414`), so a wave of regenerations can't silently waste a pass.

On the user's closing claim ("currently everything other than the title needs to be redone"): the repo evidence supports treating the current Main Menu mapping as not-accepted (`element_manifest.md:9-14`), so a fresh full-inventory pass is justified.

## Evidence Checked
- `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` (full) — method class, Step D decomposition, Step I correction taxonomy, Step J scorecard, Section 13 FULL/PARTIAL.
- `Screens/MainMenu/element_manifest.md` — static mapping table + 2026-06-05 "not final plates" note.
- `Screens/MainMenu/checklist.md` — full-screen structure/geometry/containment/content/interactivity, prior false-PASS warning.
- `Screens/MainMenu/visual_scorecard_template.md` — PASS / FAIL / USER_ACCEPTED_DELTA verdict model.
- Folder listing of `UI/FriendslopStyle/`.

## Questions Or Blockers
None that block an answer. The proposal is a process-doc change Codex can draft directly. The only user-gated item is the *content* of any `ACCEPTED_DELTA` (taste decisions), which the existing Step B / scorecard flow already routes to the user — no new approval mechanism needed.

## Caveats
- I did not read `component_contract_current.md` or `README.md` in full; my recommendation for them is "point to the manifest," which is conservative and won't conflict.
- Scope is doc design only; no files were edited, per the stop condition. If the user wants this implemented, that's a separate explicit go-ahead.
- The four-state model should reuse existing vocabulary (failure taxonomy, `USER_ACCEPTED_DELTA`) rather than introduce parallel terms — otherwise the docs accumulate two competing classification systems.
