# Pending Issues - Combat

## Superseded AOE Slash Screenshot Reference [Minor]

The earlier Hero 1 axe AOE slash visual target was based on a chat-provided screenshot showing a multi-shape slash, multi-color bands/core/edge highlights, and an impact spot near enemy contact.

Status: superseded on 2026-05-26 by Pablo's approved generated contact sheet at `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/hero1_axe_aoe_american_flag_contact_sheet.png`, now recorded in `Hero1AxeAOESlashMechanismPacket.md`.

No current action required unless Pablo asks to restore the older screenshot as an additional reference.

## Durable Niagara Editor-Isolation Capture Route Implemented [Resolved 2026-05-27]

- Severity tag: [Major]
- Resolution: Implemented by `Scripts/CaptureT66NiagaraMRQIsolation.ps1` and `Scripts/SetupT66NiagaraMRQIsolation.py`.
- Evidence: `Saved/VFXResearch/Hero1Axe/AOE_AmericanFlagVisualTarget/EditorIsolation/20260527_MRQSquareRouteProbe/manifest.json` reports `render_success=true`, `is_square=true`, `has_non_black_pixels=true`, `margins_pass=true`, and `has_particle_log_evidence=true`.
- Notes: The route proves repeatable editor-isolation capture only. It does not accept the current Hero 1 AOE visual, does not prove temporal slash mechanisms, and does not replace gameplay capture or hitbox evidence.

## Hero 1 AOE Evidence Capture Shows Possible Discontinuous Slash Timing [Minor]

- Severity tag: [Minor]
- What's wrong: Superseded. The earlier 2026-05-26 `hero1axeaoe` evidence capture at `Saved/VideoCaptures/Hero1AxeAOE_Evidence_20260526_203917/` did not prove a smooth start-to-impact-to-dissipate mechanism because the selected frames sampled a discontinuous-looking cycle.
- Why it is out of scope now: Resolved for Checkpoint 1 only on 2026-05-27 by adding lab-only Niagara component time-dilation and cycle-duration capture overrides, then selecting the first complete carrier/sweep cycle at `Saved/VideoCaptures/Hero1AxeAOE_Checkpoint1_CarrierOnlySlowCycle_20260527_011900/`. Full playback timing, hitbox sync, support particles, and final visual fidelity remain future gates in `Hero1AxeAOESlashMechanismPacket.md`.
- What fixing it would entail: No action for the old capture evidence. Future work should continue at Checkpoint 2 or later using the packet's current evidence paths and should not use the old 2026-05-26 capture as proof of the current carrier/sweep mechanism.

## Hero 1 AOE Candidate03 Initial Contact Sheet Sampled The Wrong Moment [Resolved 2026-05-27]

- Severity tag: [Minor]
- Resolution: Rebuilt the evidence bundle for `Saved/VideoCaptures/Hero1AxeAOE_Candidate03_NorthAuraHitbox_20260527/` using frames 39, 40, 41, and 46. Frame 40 now clearly shows the red/blue half-moon slash aligned to the debug AOE sector, while frames 41 and 46 show the damage-number proof.
- Notes: Future visual-polish passes may still tune gameplay brightness, scale, and timing, but the earlier "faint VFX" concern was primarily a selected-frame artifact rather than a blocker for the current hitbox/damage proof and Codex readability assessment.

## Hero 1 AOE Candidate03 Rotation Force Frozen For Visual Lock [Minor]

- Severity tag: [Minor]
- What's wrong: Candidate03 intentionally holds slash-layer `RotationForceZ` at `0.0` in `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp` to prevent red/blue layer peel-apart while the same-view shape, orientation, aura material read, and centered impact are being locked.
- Why it is out of scope now: Reintroducing layer rotation/timing is a later animation-polish decision. Doing it during this correction pass risks returning to the elemental/electric read Pablo rejected.
- What fixing it would entail: In a reviewed visual-polish pass, tune rotation/layer timing incrementally, then rerun `Scripts/ValidateHero1AxeAOELabVFX.py`, the same-view MRQ isolation capture, and the gameplay hitbox capture to prove the north-facing aura shape remains intact.

## Resolved: Hero 1 AOE Production Binding Proof Has Opt-In Auto Frame Selection [Minor]

- Severity tag: [Minor]
- Resolution: `Scripts/BuildT66VideoEvidenceBundle.py` now supports opt-in `--auto-select-frames` activity scanning, `Scripts/CaptureT66GameplayVideo.ps1` exposes `-EvidenceAutoSelectFrames`, and `Scripts/RunHero1AxeAOEVFXBindingProof.ps1` can pass that mode without changing default proof behavior.
- Remaining note: The already accepted 2026-05-28 hitbox cleanup proof remains manual-frame evidence for reproducibility. Future proof passes can opt into automated best-frame selection when the packet calls for it.

## Hero 1 AOE Visual Polish Deferred After Hitbox Close [Minor]

- Severity tag: [Minor]
- What's wrong: The current Hero 1 AOE visual is accepted for now as backend/hitbox aligned, but it is not a final visual-polish sign-off for the full attack family.
- Why it is out of scope now: Pablo explicitly redirected away from continued AOE visual design toward infrastructure, item/stat wiring, and future idol overlay planning.
- What fixing it would entail: Re-enter through `Hero1AxeAOESlashMechanismPacket.md` visual-fidelity gates after normal item/stat proof and idol-overlay backend planning are complete, then use same-view editor isolation plus gameplay capture to compare against the approved mockup/current target.

## Idol AOE Delay Data Is Still Not Runtime-Applied [Minor]

- Severity tag: [Minor]
- What's wrong: `FIdolData::AoeDelay` is authored in `Content/Data/Idols.csv`, but the current runtime idol payload path applies idol AOE damage immediately. The Water impact-context proof logs `AoeDelay` with `DelayApplied=false` to make the preserved legacy timing explicit.
- Why it is out of scope now: The current structure pass is limited to official weapon/idol impact contexts, Water's independent damage source/query, and the blue-sphere placeholder proof. Changing idol timing would be a gameplay behavior change for all AOE idols and needs its own reviewed acceptance gate.
- What fixing it would entail: Decide whether idol AOE delay should affect damage, VFX only, or both; then add a timer-backed application path, update proof captures for delayed impact timing, and verify Earth/Storm/Water behavior separately.
- Follow-up owner: Combat VFX/idol runtime port before final idol Niagara acceptance, because final authored idol effects should explicitly choose whether `AoeDelay` drives damage, presentation, or both.

## Water Idol Impact Capture Weakly Shows Base Weapon VFX [Resolved 2026-05-28]

- Severity tag: [Minor]
- What's wrong: Resolved. The first user-facing Water idol impact video at `Saved/VideoCaptures/Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528/` exposed that the Water placeholder inherited the Hero 1 crescent AOE's damage-query center, which could read as sitting on the hero instead of the visible weapon impact band.
- Why it is out of scope now: Resolved by moving the published impact context for frontal-sector hollow crescent AOEs to the annulus midpoint while leaving target gathering, damage authority, and the production Niagara spawn origin at the existing slash center. The temporary blue sphere is now a compact fixed-scale marker; it no longer tries to show Water's full damage radius.
- What fixing it would entail: No current action for the placeholder-placement bug after the 2026-05-28 fix and recapture. Future Water Niagara authoring should revisit the annulus-midpoint bridge against the final effect's actual visual impact point and area read; do not treat the temporary `0.5` band midpoint or the compact marker scale as final Water art direction.

## Water Idol Placeholder Still Lacks Final Area Footprint [Major]

- Severity tag: [Major]
- What's wrong: The current Water idol blue sphere is a temporary impact marker, not a production AOE area read. `Idol_Water` damage uses the authored idol AOE radius, so a compact marker by itself can still read smaller than the actual damage footprint.
- Why it is out of scope now: This pass updated the combat VFX process docs and alignment contract only. Authoring the final Water Niagara area footprint, or adding a separate area telegraph around the marker, is production VFX work that needs its own effect packet and capture gate.
- What fixing it would entail: Author the Water idol production VFX through `CombatVFXVisualDamageAlignmentContract.md`, choose whether the effect is an impact burst plus area read or a full footprint splash, calibrate its `BaseVisualRadius`/footprint mapping to `FIdolData::AoeRadius`, and capture the VFX with the DamageVolume overlay in the same frame range.

## Combat VFX Alignment Contract Is Not Yet Validator-Enforced [Minor]

- Severity tag: [Minor]
- What's wrong: `CombatVFXVisualDamageAlignmentContract.md` now requires every relevant packet to declare anchor, footprint mapping, offsets, tolerance, and same-frame VFX/DamageVolume evidence, but the generic production validator does not yet parse packet alignment blocks or numerically check visual/damage tolerance.
- Why it is out of scope now: The requested scope was to fix instruction and Markdown docs so future weapon and idol VFX follow one systematic rule. Adding hard validator enforcement is a separate tooling pass.
- What fixing it would entail: Extend the combat VFX validator layer to read alignment metadata from packets or binding rows, require expected log fields/evidence paths, and fail production promotion when the alignment block or proof artifacts are missing.

## Combat VFX Impact Context Contract Is Not Yet Validator-Enforced [Minor]

- Severity tag: [Minor]
- What's wrong: `CombatVFXImpactContextContract.md` now requires generalized weapon/idol context publication, downstream parity, source identity, skip/fallback counters, neutral controls, and damage-by-source proof, but the production validator and proof wrappers still rely on current bespoke runtime log checks where they exist.
- Why it is out of scope now: This pass officialized the reusable process contract and packet schema only. Adding hard validator enforcement would require a tooling/runtime proof pass, and could affect every future weapon and idol proof wrapper.
- What fixing it would entail: Extend the combat VFX validator layer and proof wrappers to parse the generalized impact-chain diagnostic schema, map legacy effect-specific fields such as the current Water diagnostic, fail when parity or neutral-control proof is missing, and require `DamageBySource` evidence for downstream idol-owned damage/status.
