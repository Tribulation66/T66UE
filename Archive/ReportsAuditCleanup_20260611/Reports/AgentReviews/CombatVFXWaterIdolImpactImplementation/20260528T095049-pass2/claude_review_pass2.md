Verdict: REVISE

Blockers
- None.

Major Issues
- `Idol_Water` AoeDelay not addressed. `Content/Data/Idols.csv` row has `AoeDelay=0.15`. Mechanism 3 swaps the target source to a context-driven Water AOE query but does not say whether the delay still gates the Water damage tick, or whether the impact context/placeholder spawn waits for it. This is a gameplay-observable behavior surface; please commit to either preserving the existing delay or call out the change as intentional, and tie that decision into the proof log (e.g., `AoeDelay=0.15 Applied=true`).
- Water target-query radius source not specified. Mechanism 3 says "Water-owned AOE sphere centered at the idol impact point" but does not state that the sphere uses `Idol_Water.AoeRadius=300` (vs. the weapon slash effective radius). Different sources will yield different gameplay-observable hit sets. Please name the radius source explicitly and reflect it in the proof log.
- Verification plan lacks regression coverage for the "behavior-neutral" idols. The plan correctly narrows the gameplay-observable change to `Idol_Water`, but the Verification section only runs the new Water proof mode. There is no concrete check that `Idol_Earth`, `Idol_Storm`, or DOT/Pierce/Bounce idols still target the same actors and apply the same damage. At minimum, add a manifest-level regression note: either re-run an existing proof mode that exercises another idol (if one exists in `T66PlayerController_Overlays.cpp`) or capture a log-diff of `DamageBySource` against the same staged targets before/after. Without this, "no observable change" is asserted but not evidenced.

Minor Issues
- Idol impact-point definition for non-AOE weapon categories is left implicit. The struct seam is fine for this pass, but please state in the plan that the field is populated for Pierce/Bounce/DOT only as the most recent meaningful point (e.g., last per-hit location) and that nothing consumes it yet, so future passes do not silently inherit a wrong convention.
- `FT66VisualUtil::GetBasicShapeSphere` is asserted as already exposed. Confirm during edits that this symbol exists and is callable from `T66CombatVFX.cpp`; if it requires a header change, list it in the Planned Edit Scope.
- Idol overlay architecture nuance. `CombatVFXIdolOverlayArchitecture.md` casts idol overlay as additive secondary presentation; the Water flow here adds idol-owned target query/damage selection, not just presentation. That is consistent with Pablo's clarification and `FIdolData` semantics, but the `MASTER_COMBAT.md` update should explicitly reconcile the wording so the architecture doc and the new infrastructure do not drift.
- Suppression rule scope. The mutual-exclusion log line uses `Reason=ImpactPresentationActive`. Confirm in the plan that "ImpactPresentationActive" is true for both the temporary placeholder path and a future bound `IdolModifier` Niagara, so the seam doesn't accidentally re-enable the legacy projectile lane once a real binding lands.

Clarifying Questions
- Does the Water AOE sphere use the idol's `AoeRadius` (300) directly, or is it gated by hero/tier/effective-radius modifiers comparable to the weapon AOE pipeline?
- Is `AoeDelay` honored on the new path, and is the placeholder sphere spawned at trigger time or at damage-application time?
- Should the proof mode also stage one Earth- or Storm-only attack to log a no-op DamageBySource baseline, given the strict "behavior-neutral" claim?

Required Verification
- Focused `T66Editor` Development Win64 build after edits (as planned).
- `Scripts/ValidateCombatVFXProductionBindings.py` if any binding code/scripts are touched.
- Unreal-owned capture via `hero1axeaoewateridolimpact` (or the new wrapper) with `T66.Combat.ImpactSourceVerbose=1`, evidence bundle on, ffprobe metadata included.
- Log excerpt must show, in order: weapon `CombatImpactContext`, derived `IdolModifier` context for `Idol_Water` with `ParentSourceID`, `CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Water Reason=ImpactPresentationActive`, `CombatVFXIdolImpactPlaceholderSpawned SourceID=Idol_Water`, `DamageBySource` separating `Idol_Water` from weapon source, and HP delta pass/fail for Water-hit and Water-miss targets.
- Confirm `Content/Data/Idols.csv`, `Content/Data/CombatVFXBindings.csv`, and `Content/Data/DT_CombatVFXBindings.uasset` are unchanged.
- Staged standalone refresh via `Scripts/StageStandaloneBuild.ps1` after the runtime gameplay change.
- Add the regression evidence requested in Major Issues (existing idol's behavior unchanged) before closing the proof.

Rationale
- The revised plan correctly responds to pass-1 feedback: scope is narrowed to `Idol_Water`, Earth/Storm/non-AOE idols are explicitly neutral, the projectile-lane mutual-exclusion is precise, placeholder identity is non-asset and logged, and the impact-context seam is structurally honest about deferring final Niagara. Damage authority stays in `ApplyDamageToTargetHandle`, `SourceID=Idol_Water` is preserved, and no production binding row is created — all consistent with `CombatVFXGeneratedAssetPolicy.md`, `CombatVFXIdolOverlayArchitecture.md`, and `CombatVFXDefinitionOfDone.md`.
- The reason this is REVISE rather than APPROVE: the plan changes Water's gameplay-observable target selection but is silent on `AoeDelay` and on which radius drives the new Water AOE query; either omission could produce a quiet behavior drift that is invisible in the proof unless it is logged and asserted. The regression claim for non-Water idols is also asserted without a verification artifact. These are tractable additions, not architectural problems, so the structure can move to APPROVE once the three Major items are tightened in the packet.

