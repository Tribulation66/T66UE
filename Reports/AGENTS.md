# Reports AGENTS.md

## Purpose

Use this folder for report/proof/review artifacts that should be findable outside production source folders and outside `Saved`.

This file only routes artifacts under `Reports/`. It does not override root `AGENTS.md`; follow the root process router first, then these folder-specific placement and retention rules.

## Routing Rules

- Use `Reports/AgentReviews` for review packets and reviewer outputs after this hygiene pass completes. Existing `Saved/AgentReviews` runs remain valid legacy evidence and must stay discoverable by docs or helper output.
- Use `Reports/AgentReviews/ClaudeDirectRead` for Claude operator/reviewer run folders created by `Scripts/Invoke-ClaudeDirectRead.ps1`. Full Operator runs may contain actual Claude-made changes, but those changes are not greenlights until Codex validates the real output under `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Store Codex approval artifacts for full Claude Operator runs in the durable task folder under `Reports/AgentReviews/<TaskSlug>/codex_operator_approval.md`; the first non-empty line must be exactly `Codex Approval: APPROVE` before the helper will run full Operator mode.
- Use `Reports/Hygiene/<YYYY-MM-DD>` for cleanup manifests, candidate inventories, keep-lists, and gate reports.
- Use `Reports/ToonStyle` instead of `ToonStyle/Reports` when the file is only a report and current process docs/manifests do not require the old location.
- Use `Reports/ModelGeneration` for report-only model-generation artifacts only after checking code, scripts, process docs, manifests, and generated metadata for old-path consumers.
- Use `Reports/Proof/<Domain>/<TaskSlug>` for temporary proof runs. Mark expirable raw run folders with `.report-run.json`.
- Keep `Audit/` for user-requested audits and active audit lifecycle only.

## Retention

Raw report/proof run folders expire after 15 days. New raw run folders should include `.report-run.json` with `expiresAfterDays: 15`, but existing raw run folders without a marker still follow the 15-day rule after a durable-summary check and an active-reference sweep.

Delete whole run folders only. Do not prune individual files from a run folder.

Do not apply retention to durable manifests such as `Reports/Hygiene/<YYYY-MM-DD>/` unless a `.report-run.json` marker explicitly says the folder is a raw run.

## Safety

Before moving or deleting report artifacts, run a narrow consumer sweep over code, scripts, docs, manifests, and generated metadata for the old path.

Do not move active review runs until the task that created them is complete. If a review started under `Saved/AgentReviews`, either migrate the final packet/output after completion or add a durable pointer under `Reports/AgentReviews`.
