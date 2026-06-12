Verdict: APPROVE

## Blockers
None.

## Major Issues
None. The change is tightly scoped to the temporary placeholder visual scale, leaves damage radius / query / source ID / impact context untouched, and preserves the structural proof the first patch established (`IdolPrimary.ImpactPoint=X=696.89` driven by `WeaponPrimary`).

## Minor Issues
- `0.85f` is a magic number selected by trial-and-error (0.5f rejected as too small). For a documented temporary placeholder this is acceptable, but consider a named local `constexpr float TempImpactMarkerScale = 0.85f;` so the intent is unambiguous and the value is greppable when the Niagara replacement lands.
- The planned comment is justified here (non-obvious decoupling between visual scale and damage radius) — keep it to one short line per repo comment discipline.
- Ensure the doc edit in `pending_issues_Combat.md` makes clear the Water item is still a structural proof and not the final Water AOE read, so the decoupling decision is not later mistaken for the shipping behavior.

## Clarifying Questions
- Was the 0.5f→0.85f rejection driven by mesh visibility from the proof camera framing, or by occlusion behind the hero stack? If it's framing, the fixed scale is fine; if it's occlusion, the marker may still read wrong from other camera angles used in future captures.
- Is `SpawnWaterIdolImpactPlaceholderVFX` invoked on any other idol source where the same decoupling would change the visual contract? Confirm the change is scoped to the Water fallback path only.

## Required Verification
The plan's verification list is sufficient as written. Must-haves on the captured log:
- `CombatImpactContext Phase=WeaponPrimary ... ImpactPoint=V(X=696.89, Z=64.00)`
- `CombatImpactContext Phase=IdolPrimary ... ImpactPoint=V(X=696.89, Z=64.00)` with `ParentSourceID=Hero_1_black_aoe`
- `CombatVFXIdolImpactPlaceholderSpawned ... Radius=300.00 ... VisualScale=0.850`
- `DamageBySource SourceID=Idol_Water` present and unchanged in count/timing vs. pre-patch capture
- `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base Location=V(X=360.00, Z=134.00)` unchanged
- Contact-sheet frame showing the blue marker as a compact sphere centered at the corrected impact point with hero silhouette visible.

## Rationale
The packet correctly diagnoses that the first patch fixed the impact-context center but left an opaque damage-radius-sized placeholder that visually contradicts the corrected center. Decoupling the temporary marker's visual scale from `IdolData.AoeRadius` while preserving `Radius` in the log and in damage application is the minimum change needed to make the corrected impact point visually legible. PPF check shows SAME method class on the same placeholder artifact, ARTIFACT PARITY GATE stays SAME, and the MECHANISM MANIFEST keeps the weapon-drives-idol context and idol-owned damage source intact. Risk that the placeholder no longer communicates AOE size is acknowledged and acceptable since damage radius is logged and the final Niagara will own the real area read. Codex may proceed to implement under the stated scope.

