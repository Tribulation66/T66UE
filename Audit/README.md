# T66 Audit Index

This folder keeps audit material organized by action status.

## Folder Map

- `Pending/`: audits and trackers where fixes or validation are not complete.
- `Finished/`: audits whose cleanup or fix pass has been completed and kept only as a closed audit trail.
- `Reference/`: technical background, broad inventories, external reviews, and historical drafts that are useful to read but are not an active fix queue.

## Pending Entry Points

- No active pending audit files currently. Put new audits here only when fixes or validation are still incomplete.

## Finished Entry Points

- `Finished/T66_ARCHIVE_ASSET_CLEANUP_LEDGER.md`: closed source-asset archive cleanup audit trail.
- `Finished/T66_FULL_AUDIT_2026-05-04.md`: full current-source audit remediated through the 2026-05-05 closeout waves.
- `Finished/T66_MASTER_OPTIMIZATION_AUDIT_V5.md`: optimization master remediated through the 2026-05-05 closeout waves.
- `Finished/T66_PACKAGING_CLEANUP_TRACKER.md`: packaging/runtime asset-contract tracker closed with guard and fresh staged build validation.
- `Finished/PERFORMANCE_AUDIT.md`: older performance audit verified closed against current source.
- `Finished/T66_DOCS_CLEANUP_LEDGER.md`: docs cleanup ledger verified closed against current docs.

## Reference Entry Points

- `Reference/T66_UI_AUDIT.md`: detailed UI framework audit and related findings.
- `Reference/T66_UI_TECHNICAL_HANDOFF_FOR_CLAUDE.md`: comprehensive technical handoff explaining how T66 builds UI with Unreal C++/Slate, screen routing, style helpers, runtime assets, and verification.
- `Reference/Historical/2026-04-16-optimization/`: superseded optimization drafts and review passes.
- `Reference/Historical/2026-04-17-optimization-wave/T66_OPTIMIZATION_AGENT_ASSIGNMENTS.md`: old operational handoff kept as historical reference only.

Root-level audit files should be treated as an inbox for new in-progress drafts. Once a draft is accepted, classify it as `Pending`, `Finished`, or `Reference`.
