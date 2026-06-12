The packet write is awaiting your permission grant — I won't retry further. Here's the assessment so you have it regardless of the write.

## Read-only infrastructure assessment (weapon/idol Combat VFX)

**Current state:** One production weapon-base binding exists (`Hero_1_black_aoe → NS_Hero1AxeAOE_MeshSlash`). Pierce/Bounce/DOT weapons are packet-only (no production rows). All four idol category proofs (Water/AOE, Light/Pierce, Electric/Bounce, Poison/DOT) are structural/runtime proofs spawning **reused placeholder primitives**, not authored idol Niagara. The idol lane is *already general* (switches on `FIdolData.Category`, emits category-neutral `CombatImpactChainDiagnostic`) but is wrapped in stale Water-era naming and three duplicated hardcoded idol lists.

**Biggest behavior-preserving cleanup wins (Phase 1):**
- Rename log reason `ImpactPresentationOwnsWaterPlaceholder` — Codex already flagged this (`codex_validator_report.md:96`); `T66CombatComponent.cpp:3184`.
- Rename Water-prefixed but category-general locals (`WaterIdolImpactContextCount`, `bWaterContextParity`, etc.), `T66CombatComponent.cpp:3327-3760`.
- De-duplicate the **three** independent hardcoded idol allowlists that must stay in sync: runtime `UsesImpactPresentationForIdol` (`2172`), overlay `SupportedProofIdols` (`Overlays:3366`), runner cases (`RunHero1AxeIdolCategoryNativeImpactProof.ps1:69-113`).
- The aggregate `CombatIdolImpactDiagnostic` hardcodes `SourceID=Idol_Water` even for Light/Electric/Poison (`3744`) — needs-decision: remap to neutral schema or drop as redundant.

**Validator gaps:** impact-context + alignment contracts not validator-enforced; production validator is Hero-1-AOE-specific; `CombatVFXBindings.csv`/`.uasset` untracked by git.

**Explicit no-touch (behavior, not cleanup):** allowlist *membership*, `AoeDelay` immediate-apply, `AoeInnerRadiusRatio=0.54`, annulus-midpoint bridge, Pierce/Bounce/DOT falloff defaults, `0.20s` playback clamp, placeholder marker scale, automation HP/item grants.

The full packet (formatted per your spec, first line `Operator Packet: COMPLETE`) is staged for `Reports/AgentReviews/WeaponIdolVFXInfrastructurePass/operator_packet.md` — approve the write and I'll land it, or tell me to adjust first.
