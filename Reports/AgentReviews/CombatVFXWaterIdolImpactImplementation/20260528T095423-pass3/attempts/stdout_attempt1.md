Verdict: APPROVE

Blockers
- None.

Major Issues
- None blocking. Water's target set is intentionally redefined as "all actors within `IdolData.AoeRadius=300` at the slash center," which can expand or shift Water's hit set vs. the legacy `WeaponHitActors` loop. The plan treats this as the desired behavior (idol-owned damage source with its own impact point) and Pablo has explicitly approved that direction, so it is in-scope — but Codex should be ready for the proof log to show Water hitting targets that the weapon slash did not (e.g., outside sector/inner-radius) and not treat that as a regression.

Minor Issues
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` is only reconciled indirectly via `MASTER_COMBAT.md`. The architecture doc itself currently frames idol overlay as additive presentation; the change introduces idol-owned gameplay damage/query authority alongside that overlay role. A short clarifying note in `CombatVFXIdolOverlayArchitecture.md` (not a rewrite) would prevent future drift.
- "All-weapons/all-idols" is structurally satisfied via the `FT66CombatImpactContext` seam but proven only on AOE this pass. The plan is explicit about that scoping; just ensure the log lines for Pierce/Bounce/DOT context fills are guarded by `T66.Combat.ImpactSourceVerbose` and not emitted in normal play.
- Mechanism 5 (`TrySpawnBoundIdolImpactVFX` seam) is verified by inspection only. Acceptable given no active row exists, but if practical, add a one-line log line behind the verbose CVar confirming `ResolveCombatVFXBinding(IdolModifier, Idol_Water, AOE, …) -> None` so the seam is observably reached in the proof run.
- The placeholder sphere and Water damage fire at the same immediate time. Anti-Lookalike is addressed via the required log sequence; make sure the proof excerpt preserves chronological order (`WeaponBase` → `IdolModifier` → placeholder spawn → damage application) so the sphere alone cannot be misread as acceptance.

Clarifying Questions
- Confirm: when an idol's `IdolModifier` impact presentation path is active, only that idol's projectile-lane entry is suppressed in `VisualPayloadCount` — Hero-1 weapon presentation and other idols' projectile-lane entries remain unaffected. The packet implies this; please make it explicit in code comments and the `MASTER_COMBAT.md` update.
- Confirm intent for `Idol_Water` proof targets: the staged fixed targets should include at least one target that would have been inside the weapon slash hit set and one that would have been outside the slash but inside the 300u Water sphere, so the proof actively demonstrates the new query semantic rather than incidentally matching the old hit set.

Required Verification
- Focused C++ build (`T66Editor` Development Win64).
- `Scripts/ValidateCombatVFXProductionBindings.py` if production binding code path is touched (no row added expected).
- Unreal-owned capture via `Scripts/CaptureT66GameplayVideo.ps1 -T66GameplayAutoCapture=hero1axeaoewateridolimpact` with evidence bundle and `T66.Combat.ImpactSourceVerbose=1` via `ExecCmds`.
- Log tokens present and in order: `CombatImpactContext SourceType=WeaponBase`, `CombatImpactContext SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=…`, `CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Water Reason=ImpactPresentationActive`, `CombatVFXIdolImpactPlaceholderSpawned SourceID=Idol_Water`, `RadiusSource=IdolData.AoeRadius Radius=300`, `AoeDelay=0.15 Applied=false Reason=LegacyImmediatePreserved`, `DamageBySource` with `Idol_Water>0` separated from `AutoAttack`.
- Neutral `Idol_Earth` run: legacy idol path active, no `CombatVFXIdolProjectileLaneSuppressed SourceID=Idol_Earth`, `Idol_Earth` damage present via legacy path.
- Source-gate snippet showing the new Water target-query branch is guarded by `IdolID == Idol_Water`; non-Water idols still hit the legacy block.
- `Content/Data/Idols.csv`, `Content/Data/CombatVFXBindings.csv`, `Content/Data/DT_CombatVFXBindings.uasset` unchanged.
- Staged standalone refresh via `Scripts/StageStandaloneBuild.ps1` per the runtime-gameplay-change rule, with taskbar shortcut target check.
- Proof mode snapshot/restore of prior equipped idol state confirmed by post-run log line.

Rationale
- The packet directly addresses both prior REVISE passes: AoeDelay handled by preserving legacy timing with explicit `Applied=false` log; radius source named as `IdolData.AoeRadius=300`; behavior-neutral idols protected by a single-`IdolID` gate plus an `Idol_Earth` neutral proof and a source-gate snippet; projectile-lane vs. placeholder mutual exclusion has a precise rule and log line; placeholder uses the existing engine primitive via `FT66VisualUtil::GetBasicShapeSphere` so no new asset is introduced; verbose logging is CVar-gated; proof mode snapshots/restores idol state.
- Process gates are respected: no production binding row added, no final Water Niagara claimed, `VFX_PROCESS_INDEX` deferral preserved, `CombatVFXGeneratedAssetPolicy` not violated, `CombatVFXDefinitionOfDone` separation of presentation vs. damage authority maintained via `SourceID=Idol_Water` and `DamageBySource` separation, Unreal-owned capture used, `MASTER_COMBAT.md` updated, `pending_issues_Combat.md` only on a concrete gap.
- Scope matches Pablo's clarification: shared structural seam across weapons/idols, concrete first consumer limited to `Idol_Water` AOE with a temporary blue-sphere proof at the slash impact point, final Niagara explicitly deferred, Mini/minigame excluded.
- Pablo has explicitly green-lit the structural pass and temporary Water proof, so APPROVE here means safe for Codex to proceed under the stated verification plan; the minor items above should be folded into the implementation/proof rather than block it.

