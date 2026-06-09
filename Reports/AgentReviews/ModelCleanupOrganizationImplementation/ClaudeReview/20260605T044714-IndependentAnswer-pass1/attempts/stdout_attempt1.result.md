Result: OK

## Independent Answer

This prompt is internally actionable by the Operator/Validator pair — the user already approved ("go for it"), so no user-only decision is pending. Codex can execute it within the contract's gates. The correct shape of the answer:

1. **Runtime `Content/` model deletion** — only delete assets that pass *both* gates: Unreal package reference proof (via asset registry / reference viewer, not raw FS) **and** text/data/binary reference proof. Use Unreal asset APIs for the actual delete. Do **not** remove whole hero folders unless the current audit explicitly marks them whole-folder candidates. Every candidate that fails a gate must be listed as *skipped, with the reason*.

2. **Generated model-output cleanup** (`Model Generation/Runs`, `Scenes`, `Archive`, `Reference`, one-off batch folders) — delete only after imported assets are verified/rejected and durable summaries/manifests are preserved. The existing README "Cleanup Policy" (lines 18–24) already states this intent; the durable per-model workflow should be added there so it isn't rediscovered each pass.

3. **Documentation** — the user asked for the org/cleanup approach "under read.me in the model generation." Best home is `Model Generation/README.md` and/or `Instructions/` (the README points to `Instructions/README.md` as canonical cleanup-policy index). Add a concrete, repeatable checklist: gate sequence, which folders are transient vs durable, what manifest/summary must survive, and the Unreal-API delete step.

One gap Codex should close before reporting done: I found **no dedicated audit artifact** for this specific model cleanup in the task folder (only `original_prompt.md` and the review prompt). The scope says "audit-gated." Codex must point to the actual audit that marks safe candidates (or generate one) — deletions without that proof violate the gates. This is a Codex fixable item, not a user blocker.

## Evidence Checked

- `original_prompt.md` — confirms user approval and full gate set.
- `Model Generation/README.md` — existing "Cleanup Policy" and workspace-shape sections; the natural doc target.
- `Instructions/README.md` referenced as canonical cleanup-policy index.
- Glob of `Model Generation/**/*.md` — confirms Runs/Archive/Experiments/Production output folders exist as cleanup targets.
- Task folder contents — no model-cleanup audit artifact present yet.

## Questions Or Blockers

None requiring the user. The cleanup was already approved and the gates are defined; remaining items (locating/producing the gating audit, performing reference proofs) are Operator work.

## Caveats

- Deletions are hard to reverse. The "audit-gated" requirement is load-bearing — if Codex's draft proceeds to delete without citing the per-asset reference proofs and the governing audit, that draft is not safe to ship and should be corrected before answering the user.
- Confirm the doc lands where the user expects (Model Generation README / Instructions), and that it's a *reusable* workflow, not a one-time changelog of this pass.
- I was read-only; I did not verify individual asset reference counts. Those proofs are the Operator's responsibility per the contract.
