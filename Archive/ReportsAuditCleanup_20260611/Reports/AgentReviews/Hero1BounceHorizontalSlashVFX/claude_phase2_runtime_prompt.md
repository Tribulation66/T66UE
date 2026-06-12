# Claude FullOperator Phase 2: Hero 1 Bounce Runtime Wiring

You are Claude Operator. Codex approved this bounded phase in:

`Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval_phase2_runtime.md`

Read that approval artifact and stay within it exactly.

Task: implement runtime-only Hero 1 Bounce wiring so Bounce publishes official per-link weapon impact contexts and calls the production VFX dispatcher once per chain link. No assets, CSV, DataTable, scripts, captures, Git, Mini/minigame, imagegen, credentials, or broad Git/LFS scans.

Required source of truth:

- `Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/plan_packet.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatVFX.cpp` if needed
- `Gameplay/Combat/MASTER_COMBAT.md` only if a concise runtime note is needed

Runtime requirements:

- Preserve current Bounce damage authority and target selection.
- Publish one `FT66CombatImpactContext` per resolved Bounce chain link (`PerChainLink`), with `ChainIndex` and the link's impact point/target handle/effective damage.
- Call the bound production VFX dispatcher for each link.
- Add Bounce support to the dispatcher as `ImpactAnchored`, small fixed footprint/scale.
- If no Bounce production binding exists yet, fail gracefully and do not break or suppress existing temporary presentation.
- Preserve AOE and Pierce behavior.
- Preserve downstream/idol processing so future systems can consume each Bounce link context.
- Keep edits tightly scoped.

Verification:

- Run focused editor build if possible:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
- Report exact changed files, compile result, and source anchors.
- If blocked, stop and explain.
