# Reports

`Reports` is the durable home for agent review packets, proof summaries, handoff reports, and cleanup manifests.

Use `Audit/` only when the user explicitly asks for an audit or when an existing audit workflow owns that output. Tool-owned execution byproducts may still write to their current paths, such as `Saved/Audits/WorldAssetAudit.json`, until their scripts are migrated. Durable human-readable conclusions should be linked or copied here.

## Structure

- `Reports/AgentReviews/` - Claude/Codex review packets and final review outputs.
- `Reports/AgentReviews/ClaudeDirectRead/` - Claude direct-read operator/reviewer run folders. Operator runs are proposal artifacts and require a paired review artifact before they can support implementation or completion claims.
- `Reports/Hygiene/` - cleanup manifests, inventories, keep-lists, and delete gates.
- `Reports/Proof/<Domain>/<TaskSlug>/` - temporary proof bundles and evidence summaries.
- `Reports/ToonStyle/` - ToonStyle report-only artifacts moved out of production folders after consumer checks.
- `Reports/ModelGeneration/` - model-generation reports after process docs/manifests are updated or verified path-independent.
- `Reports/UI/`, `Reports/Gameplay/`, `Reports/World/`, `Reports/Performance/` - domain handoffs and non-audit reports.

## Retention

Raw run folders expire after 15 days. Cleanup should delete the whole raw run folder, not individual files.

Before deleting an expired raw run folder, confirm all of the following are true:

- A durable summary or report exists outside the raw run folder.
- No active docs, manifests, pending issues, or source comments reference the raw folder.

New raw run folders should include a `.report-run.json` marker with `expiresAfterDays: 15` so future cleanup can identify them unambiguously. Existing raw run folders without a marker still follow the 15-day rule after the durable-summary and active-reference checks pass.

Durable manifests, including `Reports/Hygiene/<YYYY-MM-DD>/`, are not raw run folders unless they explicitly contain `.report-run.json`.
