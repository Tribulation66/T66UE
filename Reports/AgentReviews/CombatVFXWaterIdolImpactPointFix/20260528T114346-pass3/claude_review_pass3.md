Verdict: APPROVE

Blockers
- None.

Major Issues
- None. The plan correctly preserves the gameplay-side contract (Radius, SourceID, ImpactPoint, damage query) and changes only the temporary placeholder's visual scale. Architectural separation between damage radius and visual marker is the right call for a "point marker" semantic and is consistent with `CombatVFXIdolOverlayArchitecture.md`.

Minor Issues
- The new scale formula `FMath::Clamp(Radius / 600.f, 0.35f, 0.65f)` is effectively a near-constant: it saturates at the low clamp for Radius ≤ 210 and at the high clamp for Radius ≥ 390, giving exactly the band you'd care about (~Radius 210–390) a swing of only 0.3. If the intent is "decouple visual scale from damage radius," consider either (a) a fixed scale (e.g., `0.5f`) with a brief comment, or (b) a wider, intentional mapping. As written it suggests a relationship that's almost not there, which is a maintenance trap.
- No record of the base placeholder mesh dimensions in the packet, so we cannot pre-predict whether 0.5 will read as "compact marker" vs. still overlapping the hero silhouette from the proof camera. This is acceptable because the capture step verifies visually, but worth acknowledging that the chosen constants are tuned against the capture, not derived.
- The pass criterion for the contact sheet ("blue marker no longer covers the hero silhouette") is subjective. Consider tightening to a measurable check (e.g., marker bounding box must not overlap hero capsule in proof frame N), so a future re-tune doesn't drift.

Clarifying Questions
- None blocking. (Mesh base size question is informational; the capture proof will adjudicate.)

Required Verification
- Verification plan as specified is sufficient and Unreal-owned:
  - Focused compile via `Build.bat T66Editor`.
  - `Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact ... -EvidenceBundle`.
  - Log assertions: `WeaponPrimary` and `IdolPrimary` `ImpactPoint=V(X=696.89, Z=64.00)`, `CombatVFXIdolImpactPlaceholderSpawned ... Radius=300.00 ... VisualScale=0.500`, `DamageBySource SourceID=Idol_Water`, `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base`.
  - Contact-sheet inspection of the blue marker placement.
- Suggested additions (non-blocking):
  - Include a side-by-side contact frame (pre-patch VisualScale=1.500 vs. post-patch VisualScale=0.500) so the visual delta is auditable, not narrated.
  - Confirm `SpawnWaterIdolImpactPlaceholderVFX` is not invoked by any other context where the larger scale was intentional (one grep).

Rationale
- The scope is bounded to a single function's visual-scale line in a temporary placeholder plus a doc note in `pending_issues_Combat.md`. Damage radius, source ID, impact context, slash binding, color, lifespan, and Z offset are all untouched, so the idol-overlay mechanism manifest is preserved. The runtime evidence after the prior patch already shows the corrected `ImpactPoint=X=696.89` for both `WeaponPrimary` and `IdolPrimary`; this packet's job is purely to make the placeholder read as a marker for that point, which is exactly what the change does. Verification is Unreal-owned and includes both log assertions and visual proof. The minor issues above are stylistic/clarity concerns, not safety or correctness issues, and do not warrant another revision cycle — they can be folded in during implementation or addressed in the next iteration if the capture surfaces a problem.

