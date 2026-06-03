Verdict: APPROVE

## Blockers
None.

## Major Issues
None. The diagnosis is internally consistent: the weapon `SlashCenter` is currently published as the impact-context point even though for Hero 1's crescent (`AoeInnerRadiusRatio=0.54`) the visual band sits forward of that center, so the idol placeholder inherits the wrong XY. The chosen seam (publish a band-midpoint into `SlashImpactContext.ImpactPoint` without mutating `SlashCenter` or the production VFX spawn origin) preserves combat-code damage authority and weapon-VFX silhouette ownership, which matches `CombatVFXIdolOverlayArchitecture.md`.

## Minor Issues
- The Water idol's own damage query will move forward with the corrected impact point. The packet acknowledges this and the user's stated intent ("idols should work on top of weapons and be triggered at the weapon... impact point") supports it, but the existing `hero1axeaoewateridolimpact` proof targets were calibrated against the old center. Expect at least one proof-target expectation update; the conditional "in scope only if..." clause covers it but should not be skipped.
- The `0.5` annulus midpoint is a tuned placeholder. That is correctly disclosed and a follow-up marker is planned in `pending_issues_Combat.md`. Keep that marker explicit enough that the final Water Niagara author re-tunes against the real effect, not against the bridge value.
- The gate `bUseHeroOneFrontalSector && EffectiveSlashInnerRadius > KINDA_SMALL_NUMBER && EffectiveSlashRadius > EffectiveSlashInnerRadius` is data-driven by `AoeInnerRadiusRatio`. The negative-scope check (other AOE rows authored at `0.00`) is included in verification — good. Worth one log assertion in the capture confirming non-Hero-1 AOE attacks still publish the center, if such a path exists in the proof capture; otherwise note it as deferred.
- Z offsets in `SpawnWaterIdolImpactPlaceholderVFX` (+76) and bound idol spawn (+72) intentionally remain. That is correct for this pass, but worth a one-line note in the architecture doc next pass that `ImpactPoint` is the planar/grounded source and presentation Z is owned by the spawner.

## Clarifying Questions
None for Codex — the packet's planned code change, gate predicate, and verification log lines are concrete enough to implement and verify without further user input.

## Required Verification
- `git diff --check` on the two listed files.
- Focused `T66Editor Win64 Development` compile against `T66.uproject`.
- Unreal-owned `hero1axeaoewateridolimpact` capture with `-EvidenceBundle`.
- Log assertions exactly as listed: `WeaponPrimary SourceID=Hero_1_black_aoe`, `IdolPrimary SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe`, `CombatVFXIdolImpactPlaceholderSpawned`, `DamageBySource SourceID=Idol_Water`, and `CombatVFXProductionSpawned Binding=Hero1Axe_AOE_Base` (to prove the weapon silhouette path was not disturbed).
- Numeric check: post-fix `WeaponPrimary`/`IdolPrimary` XY should equal `SlashCenter + Forward * 336.89` (±float tolerance) given the cited `EffectiveSlashInnerRadius=236.26`, `EffectiveSlashRadius=437.52`. If the actual values shift due to current weapon data drift, recompute from the new effective radii and document.
- Contact-sheet review: blue sphere no longer overlapping the hero silhouette; sits over the crescent band.
- If any `hero1axeaoewateridolimpact` proof target fails, classify the failure (moved-center vs. unrelated gameplay regression) before editing expectations.

## Rationale
The plan correctly identifies the bug as a wrong impact-context source for Hero 1's hollow-crescent AOE, not a capture-selection or Niagara-collision issue. The fix is narrowly scoped, gated by data that today only matches `Hero_1_black_aoe`, preserves weapon damage authority and the production VFX silhouette, leaves `SlashCenter` (target gathering, query, spawn origin) intact, and respects the architecture rule that idol context owns its own source/query/damage with the impact point inherited from the weapon. The annulus-midpoint approximation is honestly labeled as a placeholder. Verification is concrete with specific log lines and an expected numeric offset, and the deprecated temporary projectile is explicitly kept disabled. No human-only decision is gating progress: the user already articulated the intent ("impact at weapon visual impact point"), and the gameplay implication (idol damage center moves forward with the visual impact) follows directly from that intent. Codex can proceed to implement under this scope and rerun review only if verification surprises require a method-class change.

