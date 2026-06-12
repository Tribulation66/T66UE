# Hero 1 Axe DOT Mechanism Packet

**Status:** Active production binding (runtime wired). The Hero 1 DOT attack spawns a single visible hero->target shot — now carried by the active `Hero1Axe_DOT_Base` production aura-ring Niagara silhouette transported by the moving visual projectile — plus three target-following sphere applicator markers, and applies the existing single authoritative DOT payload on impact. The DOT weapon production row is active like AOE/retired-lane/Bounce. This packet does NOT approve final visual fidelity, an imagegen visual target, or Pablo visual sign-off; the sphere markers remain intentional placeholders and the aura-ring carrier's final visual polish is still deferred to a later phase.

This validator proves Combat VFX binding structure, required assets, source guards, and declared data contracts. It does not prove visual fidelity, temporal mechanism quality, final player-facing readability, or Pablo visual approval.

## Intent

Future Hero 1 DOT should read as an axe aura that strikes a target and remains attached or orbiting while damage ticks. The current placeholder establishes the runtime structure (shot -> impact -> attached markers -> ticking) ahead of that final art.

## Implemented Mechanism Manifest (placeholder phase)

1. **Single moving hero->target shot (production carrier).** One visual-only `AT66HeroProjectile` spawned via `UT66CombatComponent::SpawnVisualTravelProjectile` travels from the hero attack origin to the resolved target aim/impact point using the proven Bounce timed-travel seam (`SetTimedVisualTravel`). It carries `Damage=0` and `NoCollision`; it is presentation-only. `PerformDOT` resolves the active `Hero1Axe_DOT_Base` binding and passes its Niagara system to `SpawnVisualTravelProjectile`, which attaches the authored aura-ring carrier (`/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`, built by the `T66Hero1AxeDOTVFX` commandlet with `-T66Hero1AxeDOTProduction`) to the moving projectile root and hides the temporary profile meshes — so the authored Niagara/material/mesh silhouette is the visible DOT shot and runtime only transports it. Log: `T66DotShotSpawned ... Carrier=/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash...`. If the binding fails to resolve, the temporary DOT profile is the dev-fallback mover so bring-up is never invisible.
2. **Hit-triggered staggered marker pulse.** Exactly three small sphere markers spawn at the target (not the hero) when the shot arrives, via `AT66DotMarkerVFX` + `UT66CombatComponent::SpawnDOTApplicatorMarkers`. Log: `T66DotApplicatorMarkersSpawned ... MarkerCount=3`. The markers are created hidden and reveal one at a time on a 0.5s cadence — index 0 immediately, index 1 at +0.5s, index 2 at +1.0s — rather than all appearing simultaneously. Each marker then disappears again after a short 0.3s visible pulse (`MarkerVisibleDurationSeconds = 0.3f`), so only one placeholder dot is visible at a time: marker 0 visible 0.00–0.30s, marker 1 visible 0.50–0.80s, marker 2 visible 1.00–1.30s. Each reveal logs `T66DotApplicatorMarkerRevealed ... MarkerIndex=<i> MarkerCount=3 PlannedRevealDelaySeconds=<0.00|0.50|1.00>`; each hide logs `T66DotApplicatorMarkerHidden ... MarkerIndex=<i> MarkerCount=3 PlannedRevealDelaySeconds=<0.00|0.50|1.00> VisibleDurationSeconds=0.30`. Reveal and hide are driven by the world timer manager; the timers are invalidated automatically if the marker actor self-destructs before a later reveal/hide fires, and both callbacks re-validate the marker index. This cadence is the marker-presentation contract for future DOT placeholder/final-art work: multi-dot applicator markers must reveal over time, not at once. The 0.3s visible-pulse duration is the **current placeholder** presentation only — it does not bind future DOT/final-art projectiles to that exact duration unless their own effect packet opts in.
3. **Data-authoritative DOT ticking (single payload).** Exactly one `UT66RunStateSubsystem::ApplyDOT(...)` call with `SourceID=HeroPrimaryDot`. The three spheres are visual applicators; they are NOT independent damage lanes and never multiply DOT damage. Log: `T66DotPayloadApplied ... Source=HeroPrimaryDot (single payload)`.
4. **Target-following persistence.** The marker actor hard-attaches to the target actor (`AttachToActor`) so the markers follow it for the DOT duration, then self-destructs (`SetLifeSpan(Duration)`).
5. **Impact context publication.** The weapon impact context is published synchronously at the official target aim/impact point in `PerformDOT` (preserving downstream idol-overlay consumption), before the deferred DOT payload.

## Authority and timing policy

- Initial contact damage, impact-context publication, frostbite slow, and SFX remain **synchronous** in `PerformDOT` so existing idol-overlay context consumption is preserved.
- Only the single `ApplyDOT` payload and the marker spawn are **deferred** to projectile arrival (weak-pointer + value-captured callback). If the shot fails to spawn, the payload + markers apply immediately so gameplay never loses DOT ticking.
- Gameplay travel is near-instant by default (`T66.DOT.ProofReadableTravelSeconds = 0`); the capture proof stretches travel to 0.60s for readable frames only. Runtime damage/targeting are unaffected by the proof CVar.
- DOT tuning (duration, tick interval, per-tick damage) is unchanged from prior data-authoritative values; the single-payload split (initial contact + remaining-over-ticks) is preserved.

## Production Carrier (active row; final visual polish deferred)

- Primary archetype: moving aura-ring carrier (`SM_Hero1AxeDOT_AuraRing` + shared Hero 1 slash materials, authored in the `NS_Hero1AxeDOT_MeshSlash` Niagara mesh renderer), transported by the single hero->target DOT shot.
- Secondary archetypes (final art, deferred): `PersistentAura`, `SupportImpact`, optional `RibbonTrail`.
- Binding status: active production row `Hero1Axe_DOT_Base` → `Hero_1_black_dot`; `bSuppressTemporaryProjectile=True` like AOE/retired-lane/Bounce. The sphere applicator markers remain placeholders, and the aura-ring carrier's final visual polish / Pablo sign-off is still deferred to a later phase.
- Active production row: yes (`Hero1Axe_DOT_Base`)

## Proof Route

- Compile: `T66Editor Win64 Development` (focused) — see `Reports/AgentReviews/Hero1DOTWeapon/`.
- Gameplay capture: `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode hero1axedotvfxbinding -UseHero1AxePreviewStaging -NoHero1AxeTargets -FrameCount 120 -EvidenceBundle -EvidenceAutoSelectFrames` (Unreal-owned MP4 + logs). The preview staging flags are part of the accepted standard weapon-proof camera; omitting them can leave the camera behind a tower wall and produce the pale occlusion block instead of usable VFX evidence.
- Evidence required: capture/log showing one initial projectile and three sphere markers after impact (revealed sequentially at planned delays 0.00/0.50/1.00s via `T66DotApplicatorMarkerRevealed`, each hidden after a 0.30s visible pulse via `T66DotApplicatorMarkerHidden ... VisibleDurationSeconds=0.30`), and that DOT tick damage flows through the single `HeroPrimaryDot` source rather than three multiplied lanes.

## Future Proof Route (final art promotion, deferred)

- source/reference packet,
- imagegen/mockup if requested,
- editor-isolation aura proof,
- gameplay target-attachment proof,
- tick-pulse frame evidence,
- production binding validation after promotion.
