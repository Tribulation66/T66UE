# Hygiene Manifest 2026-05-27

This folder is a durable cleanup manifest, not a raw report run.

Do not delete this folder via the 15-day raw-run retention rule unless a future pass intentionally adds a `.report-run.json` marker. This folder records decisions, evidence, and gate status for the T66 hygiene pass started on 2026-05-27.

Gate status:

- Gate 0A: Complete - Reports bootstrap and read-only inventory.
- Gate 0B: Complete - Native class/member reference audit tooling added and dry-run through Unreal.
- Gate A: Pending - code/docs/report moves after working tree isolation.
- Gate B: Pending - Unreal binary asset deletes after referencer proof.
- Gate C: Pending - generated-output purge after keep-lists and remeasured size estimates.

Gate 0 proof artifacts:

- `Reports/Hygiene/2026-05-27/candidate_manifest.md`
- `Reports/Hygiene/2026-05-27/native_reference_audit_dryrun.json`
- `Reports/Hygiene/2026-05-27/native_reference_audit_positive_control.json`
- `Reports/Hygiene/2026-05-27/world_asset_audit.json`
