# Combat VFX Definition Of Done

**Status:** Checklist and status matrix. This file indexes existing gates; it does not create a parallel process.

This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.

## Matrix

| Gate | Required evidence | Owner |
|---|---|---|
| Working goal | Full end-state goal and durable decision block when decisions are needed | `AGENTS.md` |
| Source/reference | Pablo-provided transcript, saved written source, or approved internal packet | `CombatVFXAuthoringProcedure.md` |
| Effect packet | Filled packet with PPF, artifact parity, mechanism manifest, anti-lookalike test, and proof plan | per-effect packet |
| Visual target | Approved saved mockup/contact sheet when imagegen gate is active | per-effect packet |
| Lab isolation | `/Game/VFXLab` cook isolation and validator guard when lab assets exist | validator/script |
| Editor-isolation visual proof | Same-view top-down/black-background capture when packet requires it | `CaptureT66NiagaraMRQIsolation.ps1` |
| Gameplay capture | Unreal-owned MP4/frame evidence; desktop screenshots do not satisfy this | `CaptureT66GameplayVideo.ps1` |
| Capture visibility hygiene | Locked VFX proof camera has the target effect visible and is not contaminated by camera wall-occlusion fade rectangles, unrelated mobs, occluders, or post-proof clutter | capture script/evidence bundle |
| Temporal evidence | Start/mid/impact/dissipate frames or equivalent frame range; stills alone do not prove motion | evidence bundle |
| Hitbox/damage authority | Combat query/log proof; Niagara collision/render mesh/material opacity cannot be authority | combat runtime |
| Visual/damage alignment | Declared alignment block with anchor, pivot, offsets, footprint mapping, and tolerance, plus same-frame VFX and DamageVolume capture/log proof of impact point, visual location, authoritative extents, `BaseVisualRadius`, and `VisualScale`; intentional mismatch requires recorded approval | `CombatVFXVisualDamageAlignmentContract.md` |
| Impact-context identity and parity | Weapon context publication at the official impact point, downstream/idol context consumption with preserved `ParentSourceID`, own downstream `SourceID`, own downstream impact point, expected-vs-actual context parity, skip/fallback counters, damage/status source proof, and neutral-control proof; video-only proof does not satisfy this gate | `CombatVFXImpactContextContract.md` |
| Production binding | CSV/DataTable row, production asset path, GameInstance assignment, no `/Game/VFXLab` dependency | `ValidateCombatVFXProductionBindings.py` |
| Item/stat proof | Production-path automation or literal UI proof that stats affect combat values and VFX presentation | proof wrapper |
| Idol overlay | Architecture/effect packet plus active rows only after user approval | `CombatVFXIdolOverlayArchitecture.md` |
| Staged commit | Staged manifest, LFS/pointer checks where relevant, Claude staged-diff review | reports |

## Completion Labels

- `FULL`: every required gate for that effect/status is present with evidence.
- `PARTIAL`: one or more required gates are missing, deferred, diagnostic only, unevidenced for visual/damage alignment, outside declared alignment tolerance without approval, missing impact-context parity, missing downstream source identity, missing neutral control, or relying on video-only proof for context wiring.
- `DEFERRED`: intentionally out of scope and documented with the next owning step.

Do not call an effect production-ready until Pablo approves captured visual evidence.
