# Next Agent Prompt - Hero 1 AOE Normal Item/Stat Proof

You are working in `C:\UE\T66`. Start by creating or setting this goal:

> Confirm the normal item/stat route drives the Hero 1 black AOE production VFX and logical crescent hitbox without changing the accepted AOE visual polish yet.

Do not begin idol-overlay VFX design in this prompt. After the normal item/stat route is proven or explicitly blocked, stop and ask Pablo for the second go-ahead before opening the idol-overlay design task.

## Required First Reads

Read these before planning:

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/Hero1AxeVFXPlan.md`
- `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/MASTER_COMBAT.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`

Use Claude cross-review by default through `Scripts/Invoke-ClaudePlanReview.ps1`. If Claude is truly unavailable because of a subscription/session-limit style availability failure, use `Scripts/Invoke-CodexPlanReview.ps1` as the approved fallback reviewer. Do not treat malformed Claude output as a fallback trigger.

## Current Baseline

The current accepted AOE state is backend/hitbox aligned and visually good enough to stop design iteration for now.

Same-working-tree caveat:

- `git status --untracked-files=all` currently reports `Scripts/SetupCombatVFXBindingsDataTable.py`, `Scripts/ValidateCombatVFXProductionBindings.py`, and this proof/handoff packet as untracked.
- `git ls-files` returns no entries for `Content/Data/CombatVFXBindings.csv` or `Content/Data/DT_CombatVFXBindings.uasset`; they are live generated files on disk.
- Do not assume a fresh checkout has these files unless this pass has been committed with them. First action: run a narrow status/existence check for these exact files:
  - `Scripts/SetupCombatVFXBindingsDataTable.py`
  - `Scripts/ValidateCombatVFXProductionBindings.py`
  - `Scripts/BuildT66VideoEvidenceBundle.py`
  - `Scripts/pending_issues_Scripts.md`
  - `Content/Data/CombatVFXBindings.csv`
  - `Content/Data/DT_CombatVFXBindings.uasset`
  - `Gameplay/Combat/MASTER_COMBAT.md`
  - `Gameplay/Combat/Hero1AxeAOESlashMechanismPacket.md`
  - `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
  - `Gameplay/Combat/pending_issues_Combat.md`
  - `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/CLEANUP_STATUS.md`
  - `Reports/Proof/CombatVFX/Hero1AxeAOE_HitboxCleanup_20260528/HANDOFF_NEXT_AGENT.md`
- If continuing without a commit, verify the files exist, then run `Scripts/SetupCombatVFXBindingsDataTable.py` and `Scripts/ValidateCombatVFXProductionBindings.py`. If the files are absent, stop and ask Pablo for the prior working tree or commit containing the VFX cleanup infrastructure.
- If this handoff file itself is missing in a future clean checkout, do not reconstruct the process from memory. Ask Pablo for the final handoff packet/thread or for a commit containing the VFX cleanup infrastructure.

Authoritative current proof:

- Video: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/Hero1AxeAOE_HitboxCleanup.mp4`
- Contact sheet: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/contact_sheet.png`
- Visibility checklist: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/evidence/visibility_checklist.md`
- Runtime log: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log`

Key runtime values:

- Weapon: `Hero_1_black_aoe`
- Binding: `Hero1Axe_AOE_Base`
- Production Niagara: `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`
- `AoeInnerRadiusRatio=0.54`
- `BaseVisualRadius=411.4`
- Logged baseline: `EffectiveSlashRadius=437.52`, `EffectiveSlashInnerRadius=236.26`, `VisualScale=1.063`

## Task 1 - Confirm Normal Item/Stat Route

Do not start by inventing a new VFX. First prove that the existing player-facing item/stat path changes both combat and presentation.

Required checks:

- Increasing AOE scale through normal item/stat acquisition increases:
  - outer logical crescent radius,
  - inner logical crescent radius,
  - production Niagara visual scale,
  - debug crescent-band outline.
- Increasing AOE damage changes damage numbers and logged `EffectiveDamagePerShot`.
- Increasing AOE speed changes fire interval and logs both raw and clamped presentation playback.
- Proof should use normal item/stat route when possible. The existing proof harness grants deterministic proof items only for automation and should not be mistaken for player-facing acquisition.

Expected validation shape:

- Build if code changes are made.
- Reload affected DataTables if data changes are made.
- Run `Scripts/ValidateCombatVFXProductionBindings.py`.
- Capture through `Scripts/CaptureT66GameplayVideo.ps1` or extend an existing wrapper only if needed.
- Use Unreal-owned MP4/frame evidence plus log lines. Desktop screenshots are not proof.

## Parked Follow-Up - Idol Overlay VFX Pipeline

Do not start this work in the first next-agent pass. After Task 1 is proven or explicitly blocked, ask Pablo for a second go-ahead and then design the idol overlay system as a separate reviewed task.

Constraints:

- Weapon base VFX remains the base attack carrier.
- Idol visuals are additive overlays layered on top of the weapon-base VFX.
- Idol overlays must not use the old temporary projectile placeholder path as the real system.
- Damage authority remains logical combat queries and target handles, not Niagara collision or material opacity.
- Future implementation should likely use a reviewed binding/effect-packet extension such as `SourceType=IdolModifier`, but inspect the live data structures before committing to field names.
- Each idol overlay needs its own mechanism packet, mockup/reference gate, production asset path, binding row, and proof.

Questions to answer before implementation:

- Is one Niagara system spawned with weapon and idol parameters, or does combat spawn a base weapon Niagara plus a second idol overlay Niagara?
- How are color, timing, scale, and contact point shared between base and overlay?
- How does an idol overlay attach to different weapon hitbox shapes without changing the base damage authority?
- Which fields belong in `FT66CombatVFXBindingData`, and which need a new table or row type?

## Non-Goals For The Next Agent

- Do not continue visual polish on the AOE slash unless the user explicitly redirects.
- Do not create DOT/Pierce/Bounce VFX yet.
- Do not modify Mini/minigame systems.
- Do not use broad Git/LFS scans across `Content/` unless the task specifically requires it.

## Current Known Loose Ends To Preserve

- `Content/Data/CombatVFXBindings.csv` and `Content/Data/DT_CombatVFXBindings.uasset` are live generated binding files, but `git ls-files` returns no entries for them in this repo state. Inspect them directly and run the setup/validator scripts.
- Selected-frame evidence windows are still manual. If proof frames miss the VFX, inspect the retained frame sequence and rerun with corrected `EvidenceSelectedFrames`. The successful hitbox cleanup rerun used `start=60,mid=62,impact=64,dissipate=68`.
- `BuildT66VideoEvidenceBundle.py` accepts only `start`, `mid`, `impact`, and `dissipate` as selected-frame labels.
- The current VFX is presentation-only; combat hitbox proof comes from `UT66CombatComponent` logs and target HP deltas.
