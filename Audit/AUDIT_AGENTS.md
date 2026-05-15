# Audit Agents

## Owns

Audit organization, pending/finished/reference classification, cleanup inventories, and review packets.

## Trigger Words

Audit, cleanup inventory, pending, finished, reference, classify docs, review packet, stale docs, cleanup ledger.

## Read First

- `Audit/README.md`
- Active files under `Audit/Pending/` for in-progress work.
- Finished/reference files only as evidence, not as current instructions unless live repo checks confirm them.

## Hard Rules

- Do not execute old cleanup recommendations without verifying current repo state.
- Preserve user/current in-progress files unless explicitly asked to change them.
- Keep new review proposals in `Audit/Pending/` until approved or closed.
