You are Claude acting as FullOperator for `C:\UE\T66`. Codex is Validator.

Working task:
Operator: Claude (`claude-opus-4-8`) using FullOperator.
Validator: Codex.
Scope: behavior-preserving cleanup of the current weapon/idol Combat VFX infrastructure. No Mini/minigame scope, no behavior changes, no final art.
Stop condition: write the required Operator packet with compile/proof evidence, or stop with a process-valid blocker/decision if a requested cleanup cannot remain behavior-preserving.

Do not use native goal tools.

Approved cleanup targets:
1. De-Water generalized idol impact infrastructure naming.
   - Rename stale generalized names/logs such as `ImpactPresentationOwnsWaterPlaceholder`, `WaterIdolImpactContextCount`, `ExpectedWaterIdolImpactContexts`, `bWaterContextParity`, and similar generalized counters.
   - Preserve or deliberately map any Water-specific compatibility diagnostic needed for Water regression proof.
   - Do not change runtime behavior or diagnostic meaning.
2. Centralize proof-idol metadata for runtime and overlay C++ where practical.
   - Current proof membership must remain Water=AOE, Light=Pierce, Electric=Bounce, Poison=DOT.
   - Keep Earth neutral proof behavior.
   - Avoid broad data-driven runtime changes unless they are clearly behavior-preserving and low-risk.
3. Keep proof runner patterns in sync with renamed diagnostics.
4. Refresh stale docs so current baseline matches live binding data:
   - `Content/Data/CombatVFXBindings.csv` has active Hero 1 AOE, Pierce, and Bounce weapon rows.
   - DOT still has no active Hero 1 production binding row.
   - idol category proofs remain placeholder/proof infrastructure, not production idol Niagara rows.

No-touch behavior list:
- damage behavior, target selection, logical hitbox, falloff/default tuning, `AoeDelay`, `AoeInnerRadiusRatio`, `0.20s` playback clamp, proof target HP/staging, placeholder visual scale/lifespan, final Niagara art, DOT production binding, production idol binding rows.

Read first:
- `AGENTS.md`
- `.t66/operator-state.json`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Reports/AgentReviews/WeaponIdolVFXInfrastructurePass/codex_validator_report.md`

Likely edit seams:
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- optionally a small shared helper file/header if needed and consistent with repo style
- `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1`
- `Scripts/CaptureT66GameplayVideo.ps1` only if proof naming needs it
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` only if wording is stale

Verification:
- Run focused compile: `T66Editor Win64 Development`.
- Run `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1` or explain any blocker.
- Check logs prove Light/Electric/Poison/Water still pass and Earth neutral still behaves as neutral.

Output:
Write `Reports/AgentReviews/WeaponIdolVFXCleanup/operator_packet.md`.
First non-empty line must be exactly:
`Operator Packet: COMPLETE`

Include changed files, exact verification, proof paths, caveats, and token usage if exposed.
