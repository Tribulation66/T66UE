Verdict: APPROVE

## Blockers
- None. Scope is Tier 1 runtime alignment touching three named C++ files, with focused compile plus Unreal-owned video capture evidence. No external-state, credential, or contradiction blockers found.

## Major Issues
- None that require revision. The damage-center/impact-point separation is internally consistent: Hero 1 AOE keeps the center-pivoted mesh at `DamageCenter = SlashCenter` while `ImpactPoint` stays at the annulus midpoint, and the Water placeholder is recentered on the actual idol damage source (`DamageCenter = ImpactPoint`) rather than a hero-centered guess. Sphere scale `Radius / 50.0` correctly yields visual radius 300 for the engine unit-50 sphere (vs. the current ~42.5).

## Minor Issues
- The discriminator depends on a same-frame capture showing the blue sphere centered on the **Water** DamageVolume, but the plan only states `T66.Combat.DebugView 2` and `BuildSlashTargets` debug draw. Confirm that DebugView 2 actually renders the `Idol_Water` spherical damage volume (radius 300) and not only the weapon slash sector — otherwise the "centered on the DamageVolume" proof cannot be read.
- New `DamageCenter`/`bDamageCenterValid` fields are added to `FT66CombatImpactContext` but only Hero 1 AOE and Water are populated. Confirm all other consumers of the impact context tolerate `bDamageCenterValid == false` (no implicit zero-vector reads).
- The opaque radius-300 sphere occlusion risk is acknowledged and bounded as temporary; acceptable for structure proof.

## Clarifying Questions
- Does `T66.Combat.DebugView 2` draw the idol Water damage sphere, or is an additional cvar/flag needed for the idol-owned source overlay?
- Is the band-centered Water idol footprint (DamageCenter = band ImpactPoint, not hollow center) the intended authoritative behavior, given the finding that idols trigger at the crescent band? The plan correctly aligns visual to current authority, but if the band placement is itself a latent bug, that is a separate, out-of-scope decision — flag it rather than silently bake the alignment around it.

## Required Verification
- Focused compile `T66Editor Win64 Development`.
- `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axeaoewateridolimpact -EvidenceBundle`.
- Evidence bundle: MP4, contact sheet, selected frames, ffprobe JSON, visibility checklist.
- Runtime logs: `CombatImpactContext` (with new `DamageCenter`/offset fields), `CombatVFXProductionSpawned` (`VisualPivot`, `VisualAnchorModel`, `ImpactOffsetFromDamageCenter`), Water `CombatIdolWaterImpactResolved`, placeholder spawn with `VisualRadius == Radius`, `DamageBySource SourceID=Idol_Water`, and target hit/miss results.
- Same-frame proof that the blue sphere is centered on the Water DamageVolume (see minor issue above).

## Rationale
The plan respects AGENTS.md Tier 1, PPF, ARTIFACT PARITY, and MECHANISM MANIFEST gates; reuses the existing production Niagara and Unreal-owned capture script rather than authoring new assets; and the blue-sphere placeholder is an explicitly user-requested temporary proof, so no method substitution requires fresh user approval. Verification is concrete and falsifiable via the named discriminator. The remaining items are confirmations Codex can resolve during implementation, not revisions to the approach.

