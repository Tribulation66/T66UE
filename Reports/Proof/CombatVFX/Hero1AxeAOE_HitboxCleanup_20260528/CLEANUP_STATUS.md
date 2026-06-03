# Hero 1 Axe AOE VFX Cleanup Status

**Date:** 2026-05-28  
**Scope:** Hero 1 black AOE VFX hitbox alignment, VFX tree organization status, and next-agent handoff.

## Result

The Hero 1 black AOE attack now has a logical crescent-band hitbox aligned to the current red/blue slash visual. Damage authority remains in `UT66CombatComponent`; Niagara remains presentation-only.

The current working tree now has a documented VFX cleanup handoff: current tree, hitbox contract, proof bundle, and known caveats are recorded so the next local agent can continue from the documented process instead of rediscovering the system. This is a pre-commit / working-tree-only handoff, not a committed/fresh-clone guarantee yet, and it is not finished as a generic all-effects pipeline: future weapons and idol overlays still need reviewed packet/binding work.

Working-tree caveat: this pass is not committed here. `git status --untracked-files=all` currently reports `Scripts/SetupCombatVFXBindingsDataTable.py`, `Scripts/ValidateCombatVFXProductionBindings.py`, and the report packet files as untracked, and `git ls-files` returns no entries for `Content/Data/CombatVFXBindings.csv` or `Content/Data/DT_CombatVFXBindings.uasset`. Include or restore those files before continuing from a fresh checkout.

## Current Authoritative Tree

Process and planning:

- `AGENTS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Gameplay/Combat/Hero1AxeSharedAuraMaterialResearchPlan.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/MASTER_COMBAT.md`

Runtime/data seams:

- `Source/T66/Data/T66DataTypes.h`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Content/Data/Weapons.csv`
- `Content/Data/DT_Weapons.uasset`
- `Content/Data/CombatVFXBindings.csv`
- `Content/Data/DT_CombatVFXBindings.uasset`

Setup, validation, and capture:

- `Scripts/SetupWeaponsDataTable.py`
- `Scripts/SetupCombatVFXBindingsDataTable.py`
- `Scripts/ValidateCombatVFXProductionBindings.py`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Scripts/RunHero1AxeAOEVFXBindingProof.ps1`
- `Scripts/CaptureT66NiagaraMRQIsolation.ps1`

Production and lab VFX asset roots:

- Production: `Content/VFX/Hero1/Axe/AOE/`
- Production shared: `Content/VFX/Hero1/Axe/Shared/`
- Lab: `Content/VFXLab/Hero1Axe/AOE/`
- Lab shared: `Content/VFXLab/Hero1Axe/Shared/`
- Research/visual targets: `Saved/VFXResearch/Hero1Axe/`

## Implemented This Pass

- Added `FWeaponData::AoeInnerRadiusRatio`.
- Regenerated `Weapons.csv` so `Hero_1_black_aoe` has `AoeInnerRadiusRatio=0.54` and other AOE rows remain `0.00`.
- Updated Hero 1 AOE target selection to use `EffectiveSlashInnerRadius` for non-primary secondary targets.
- Kept the primary target on the normal target-handle path so direct hits still land even if the target is inside the hollow center.
- Updated `Hero1Axe_AOE_Base` to `BaseVisualRadius=411.4`.
- Updated `Scripts/SetupCombatVFXBindingsDataTable.py` so the repo-local setup script enforces the `Hero1Axe_AOE_Base` row, including `BaseVisualRadius=411.4`, before reloading the untracked generated binding CSV/DataTable.
- Added proof targets for inside-band, inner-hollow, behind, and outside-radius cases.
- Added near-sector-edge proof targets for just-inside and just-outside angle cases.
- Updated the production binding validator to guard the weapon geometry contract and relevant source fragments.

## Verification

- Build: `T66Editor Win64 Development` succeeded on 2026-05-28.
- Weapons DataTable reload: `Scripts/SetupWeaponsDataTable.py` succeeded on 2026-05-28.
- Combat VFX binding DataTable reload: `Scripts/SetupCombatVFXBindingsDataTable.py` succeeded on 2026-05-28.
- Production binding validator: `Scripts/ValidateCombatVFXProductionBindings.py` succeeded on 2026-05-28.
- Gameplay capture: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/Hero1AxeAOE_HitboxCleanup.mp4`.
- Contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/contact_sheet.png`.
- Visibility review: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/visibility_checklist.md`.
- Runtime log: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log`.
- `git ls-files` returns no entries for `Content/Data/CombatVFXBindings.csv` or `Content/Data/DT_CombatVFXBindings.uasset` in this repository state. They are live generated binding files on disk; use `Scripts/SetupCombatVFXBindingsDataTable.py` and `Scripts/ValidateCombatVFXProductionBindings.py` to refresh/verify them. The setup script now enforces the Hero 1 AOE row before reload.
- `git status --untracked-files=all` currently reports `Scripts/SetupCombatVFXBindingsDataTable.py`, `Scripts/ValidateCombatVFXProductionBindings.py`, and the report packet files as untracked. Include them if this pass is committed.

Logged runtime proof:

- `CombatVFXProductionSpawned` reported `EffectiveSlashRadius=437.52`, `EffectiveSlashInnerRadius=236.26`, `AoeInnerRadiusRatio=0.540`, `BaseVisualRadius=411.40`, and `VisualScale=1.063`.
- The small apparent product difference between `BaseVisualRadius * VisualScale` and `EffectiveSlashRadius` comes from rounded log output; the unrounded runtime values are used for scaling/query math.
- `Primary`, `InsideBandForward`, `InsideBandSide`, and `InsideAngleEdge` logged `ExpectedHit=1 ActualHit=1 Result=PASS`.
- `InnerHollow`, `OutsideAngleEdge`, `OutsideBehind`, and `OutsideRadius` logged `ExpectedHit=0 ActualHit=0 Result=PASS`.

## Known Loose Ends

- Normal item acquisition still needs to be verified against this VFX path. Existing deterministic proof-item grants are automation-only.
- AOE scale, speed, and damage were already proven through the `hero1axeaoevfxbinding` proof harness, but the next pass should confirm the normal player-facing item/stat route.
- Future weapons need their own effect packets, binding rows, production-promotion proof, and logical hitbox contracts.
- Idol overlays need a designed `IdolModifier` or equivalent binding layer. Do not wire idol visuals by reviving the temporary projectile placeholder system.
- The evidence-bundle selected-frame window is still manual. The current proof uses corrected frames, but an automatic best-frame selector remains a future tooling improvement.
- The current AOE visual is accepted for now. Do not keep polishing it until the item/stat and idol-overlay infrastructure work has been planned and reviewed.
