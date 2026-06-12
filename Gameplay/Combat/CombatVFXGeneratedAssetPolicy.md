# Combat VFX Generated Asset Policy

**Status:** Combat-VFX-local policy. Repo-wide generated CSV/DataTable/uasset policy is out of scope for this pass.

## Scope

This policy covers Combat VFX generated or paired assets such as:

- `Content/Data/CombatVFXBindings.csv`
- `Content/Data/DT_CombatVFXBindings.uasset`
- promoted combat VFX `.uasset` files under `Content/VFX/`
- generated lab assets under `/Game/VFXLab` when explicitly retained for proof

## Source Of Truth

- CSV source owns editable binding intent.
- DataTable `.uasset` owns runtime consumption and must be generated/refreshed from the CSV by the owning setup script.
- Promoted production Niagara/material/mesh assets under `Content/VFX/` are runtime assets and may be committed only when a packet and validator identify them as production-bound.
- `/Game/VFXLab` assets are lab-only unless a reviewed promotion process copies or regenerates them into `Content/VFX/`.

## Commit Rules

- Commit CSV and matching DataTable together when a binding row is active.
- Run the owning setup/reload script before committing a DataTable change.
- Run `Scripts/ValidateCombatVFXProductionBindings.py` before committing production binding changes.
- Record SHA256, byte size, and modified time for untracked binary runtime assets before staging.
- For `.uasset` files, verify Git LFS attributes and staged pointer-vs-blob state before commit.
- Do not commit `/Game/VFXLab` assets unless a reviewed packet explicitly says they are durable lab evidence.

## Current Hero 1 AOE Pair

`Hero1Axe_AOE_Base` is the only active Combat VFX production binding in this baseline. Future DOT/Summon/Bounce rows are deferred until their effect packets approve an active production asset.
