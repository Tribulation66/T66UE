You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\Hero1BounceHorizontalSlashVFX\codex_operator_approval_phase2_runtime.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
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

