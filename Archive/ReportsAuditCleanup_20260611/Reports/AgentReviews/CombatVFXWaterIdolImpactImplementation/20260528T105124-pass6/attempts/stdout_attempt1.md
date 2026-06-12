Verdict: APPROVE

## Blockers
- None.

## Major Issues
- None blocking the declared scope. The pre-existing unrelated hunks in `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` are not a blocker for this completion review because Codex enumerated which hunks are task-owned vs pre-existing, but they become a commit-hygiene risk at staging time (see Required Verification).

## Minor Issues
- `CombatIdolWaterImpactResolved ... AoeDelay=0.150 DelayApplied=false Reason=LegacyImmediatePreserved`: the authored `FIdolData::AoeDelay` is logged but ignored. The packet declares this out-of-scope and tracked in `pending_issues_Combat.md`, which is fine for this pass, but the deferral should be explicit in the final summary so it is not mistaken for working behavior.
- Future-binding seam is only proven as a fallthrough (`Result=None` → placeholder). That is expected because no Water Niagara is authored, but the seam will not have a real-asset spawn proof until the next pass — call this out as the explicit gating condition for the next content step.
- "Suppressed only Water's old visual-only idol projectile lane when the new impact presentation is active": the Earth neutral proof gives good evidence that the gate is Water+AOE only. Worth a one-line callout in `CombatVFXIdolOverlayArchitecture.md` (if not already present) naming the exact predicate (`UsesImpactPresentationForIdol` restricted to `Idol_Water` + AOE) so a future agent does not widen it by accident.
- Staged smoke is correctly described as a frontend launch check only. No action needed — just keep that disclaimer in the final summary so reviewers do not over-read the staged log as combat-path validation.

## Clarifying Questions
- None for this review pass. Scope, deferrals, and proof boundaries are stated unambiguously in the packet.

## Required Verification
- Before any commit: confirm the diff staged for the Water-idol-impact change matches the task-owned hunk list in the packet (impact-context branch in `T66CombatComponent.cpp:2628`, the header struct, the VFX binding helper, the proof mode + wrapper script, and the two enumerated doc bullets). The pre-existing unrelated changes in `T66PlayerController_Overlays.cpp` and `MASTER_COMBAT.md` must either land in a separate commit or be explicitly acknowledged in the commit message — they should not be silently bundled.
- Re-run `RunHero1AxeAOEWaterIdolImpactProof.ps1` on the exact tree being committed and confirm both `WaterImpact` and `EarthNeutral` still report `Result=PASS` and that the Earth forbidden-pattern list (1 Water + 4 Earth patterns) remains absent.
- Spot-check that `CombatVFXIdolOverlayArchitecture.md` documents the current placeholder gate and the `DelayApplied=false` deferral so the next idol consumer does not assume delay honoring is in place.
- No need to re-run the staged standalone unless the diff changes; the existing staged smoke screenshot + log scan are sufficient evidence for frontend launch.

## Rationale
The declared scope was "structure + Water proof with blue-sphere placeholder, no final Niagara authoring." Verification covers that scope: editor build success, production binding validator exit 0, Water proof with the expected `IdolPrimary / SourceID=Idol_Water / ParentSourceID=Hero_1_black_aoe` log chain and `DamageBySource SourceID=Idol_Water TotalDamage=32`, Earth neutral proof with the four Water-only forbidden patterns absent and its own `DamageBySource SourceID=Idol_Earth`, plus a staged refresh and main-menu smoke. Deferrals (final Water Niagara, `AoeDelay` honoring) are stated and tracked, not hidden. The pre-existing unrelated working-tree hunks are flagged transparently with a hunk-level breakdown, which is the right disclosure even though it shifts a commit-hygiene burden onto the staging step. Nothing in the packet contradicts repo instructions or the architecture doc, and the placeholder is correctly scoped behind `UsesImpactPresentationForIdol` with Earth neutral as the negative control. Safe for Codex to present the final completion summary under the scope reviewed.

