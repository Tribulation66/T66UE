You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\Hero1BounceHorizontalSlashVFX\codex_operator_approval_phase1.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Operator Packet: Hero 1 Bounce Horizontal Slash VFX

## Working Task And Tier

Working task: implement Hero 1 / Chad 1 Bounce as a small horizontal red/blue slash that hits the locked primary enemy and then targets a second enemy.

Validation depth: full. This is process-governed Combat VFX work with runtime code, production assets, data binding, commandlets, and Unreal-owned proof.

Tier: substantive full-validation task.

Scope: Hero 1 Bounce only, expected weapon source `Hero_1_black_bounce`, Bounce attack category, Bounce packet, Bounce runtime impact context, Bounce production VFX binding/assets, proof capture. Mini/minigame systems, idols, DOT, AOE, Pierce, unrelated weapons, Git commit/push/reset/clean, and broad Git/LFS scans are out of scope.

## Roles And Tool Profile

Operator: Claude (`claude-opus-4-8`).

Validator/Finisher: Codex.

Read-only Operator artifact: `C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\20260529T065702-Hero1BounceSmallHorizontalSlashVFXChangeRequestV2-Operator\claude_direct_read_operator.md`.

Read-only manifest: `C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\20260529T065702-Hero1BounceSmallHorizontalSlashVFXChangeRequestV2-Operator\manifest.json`.

Approved first mutating phase will use FullOperator:

```text
Scripts\Invoke-ClaudeDirectRead.ps1 -Mode Operator -Model claude-opus-4-8 -Effort high -ToolProfile FullOperator -PermissionMode bypassPermissions -CodexApprovalPath Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval_phase1.md -AddDir C:\UE\T66
```

## User Constraints And Out Of Scope

User requested Claude implementation.

User visual direction: small horizontal slash; red and blue colors; after hitting an enemy it targets a second enemy.

Implementation is approved by the user's "lets go ahead and move on" request, but broad work must be phased.

Imagegen is not a blocker for structural implementation because the user supplied a concrete visual direction and did not ask for a generated mockup. Final visual approval remains separate.

Out of scope: DOT, Pierce, AOE redesign, idols, Mini/minigame systems, balance/stat retuning, staged release, Git mutation, broad Git/LFS scans, credential or billing changes.

## Applicable Instructions Read

- `AGENTS.md`: task contract, no native goal tools, PPF, artifact parity, mechanism manifest, process fidelity, Claude Operator routing, Unreal-owned capture.
- `OPERATOR_VALIDATOR_PROTOCOL.md`: Claude must produce/execute phase-bounded Operator work only after Codex approval; Codex validates actual results.
- `.t66/operator-state.json`: Claude Operator, Codex Validator.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat VFX work routes through `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md`: review artifacts go under `Reports/AgentReviews`.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`: Bounce is currently infrastructure-only with no active production row.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: Bounce is typically `BeamHop` plus `RibbonTrail`/`SupportImpact`, but user-requested slash carrier must be declared and justified in the effect packet.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: production readiness needs packet, binding, visual/damage alignment, impact-context proof, gameplay capture, and Pablo visual approval.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: Bounce links use `PathAnchored` or `ImpactAnchored`; final packet must map visible slash to authoritative hit points.
- `Gameplay/Combat/CombatVFXImpactContextContract.md`: chained weapons should declare publication policy and official impact points; video alone cannot prove context wiring.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`: active binding requires CSV + DataTable refresh + production validator.
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`: current scaffold only; no active binding or gameplay integration.
- Relevant pending issue files in Combat, Gameplay, Scripts, and Data were read; none block Phase 1.

## Evidence And Live Findings

Claude read-only Operator findings are in `C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\20260529T065702-Hero1BounceSmallHorizontalSlashVFXChangeRequestV2-Operator\claude_direct_read_operator.md`.

Load-bearing findings from that artifact:

- `Content/Data/CombatVFXBindings.csv` has AOE and Pierce rows but no Bounce row.
- `Content/Data/Weapons.csv` contains the expected Bounce weapon row `Hero_1_black_bounce`.
- `Source/T66/Gameplay/T66CombatComponent.cpp` `PerformBounce` builds a Bounce context and applies chained damage, but does not spawn a bound production VFX.
- `TrySpawnBoundWeaponBaseSlashVFX` supports AOE/Pierce-style dispatch but no Bounce per-link branch yet.
- Legacy `SpawnBounceVFX` / `TrySpawnHeroBounceVariantPixels` is an existing lookalike path and must not become the accepted primary carrier.

Codex Validator correction: for final Bounce implementation, use `PerChainLink` as the intended impact-context policy unless a later Operator packet proves a safer equivalent. Bounce has separate primary and second-target impact points, and future idol/chaining systems need official per-hit contexts instead of one aggregated context.

## PPF And Process Gates

PPF CHECK
Objective: Build Hero 1 Bounce as a small horizontal red/blue slash that appears on primary hit and second chained hit.
Proven process: `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, the accepted Hero 1 AOE/Pierce Niagara/material/mesh production-binding pattern, `CombatVFXVisualDamageAlignmentContract.md`, and `CombatVFXImpactContextContract.md`.
My planned implementation: activate the Bounce packet, then phase in runtime per-link contexts/spawns, Bounce Niagara/material/mesh assets, binding setup/validation, and Unreal-owned capture proof.
Same method class: YES for structural implementation, because the primary silhouette will live in Niagara/material/renderer assets and damage remains combat-authoritative.
If NO, why: N/A.
User approval required before proceeding: NO for Phase 1 structural packet activation; YES later for final visual acceptance if Pablo requires same-view imagegen/mockup approval.
Verification evidence: packet completeness now; later compile, commandlets, binding validator, runtime logs, and Unreal-owned capture.

ARTIFACT PARITY GATE
Reference artifact/category: Hero 1 weapon slash production VFX family.
Role: Primary.
Required: YES.
Planned artifact/path: `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md` in Phase 1; later `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash`.
Status: SAME method class, Bounce-specific carrier.
Evidence: Phase 1 packet will declare the carrier, artifacts, mechanisms, proof, and exclusions before runtime/assets are authored.

MECHANISM MANIFEST
Reference/source: Pablo's Bounce request plus Combat VFX procedure.
Required mechanisms:
  1. Mechanism: per-link horizontal slash impact placement.
     Required: YES.
     Planned implementation: `ImpactAnchored` ArcSlash per chain link.
     Evidence needed: runtime logs and capture showing primary slash then second-target slash.
  2. Mechanism: per-link official impact context.
     Required: YES.
     Planned implementation: `PerChainLink` weapon contexts for Bounce.
     Evidence needed: `CombatImpactContext` logs for each Bounce link.
  3. Mechanism: red/blue Hero 1 material language reuse.
     Required: YES.
     Planned implementation: reuse shared slash material family where practical, with Bounce-specific horizontal carrier.
     Evidence needed: asset inspection and capture.

## Proposed Patch Approach

Phase plan:

1. Phase 1: activate `Hero1AxeBounceMechanismPacket.md` only.
2. Phase 2: runtime Bounce chain context and bound VFX spawn wiring.
3. Phase 3: Bounce Niagara/material/mesh commandlet and production/lab assets.
4. Phase 4: CombatVFX binding setup/validator and capture proof harness.
5. Phase 5: final verification and completion packet.

Approved first mutating phase:

- Edit `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`.
- Rewrite scaffold into an active structural packet.
- Lock the requested small horizontal red/blue slash as the current structural direction.
- Record the carrier decision: `ImpactAnchored` ArcSlash per chain link, with optional later BeamHop/RibbonTrail support.
- Record context policy: `PerChainLink`.
- Include PPF, artifact parity, mechanism manifest, mask/material expectations, visual/damage alignment block, impact-context block, anti-lookalike discriminator, verification plan, and final-visual-approval caveat.
- Do not edit code, CSV, scripts, commandlets, or assets in Phase 1.

Rollback: revert `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`. No runtime or asset side effects.

## Verification Plan

Phase 1 verification:

- Codex reads the updated Bounce packet.
- Packet must include all process gates above.
- Packet must explicitly state that final visual approval is not claimed.
- No compile/capture needed for doc-only Phase 1.

Later phases:

- Editor build.
- Bounce asset commandlet(s).
- `SetupCombatVFXBindingsDataTable.py`.
- `ValidateCombatVFXProductionBindings.py`.
- Unreal-owned gameplay MP4/evidence bundle proving primary and second-target slash, damage, and per-link context logs.

## Token Routing

TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: 1155604 for read-only change request
OperatorRunDir: C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\20260529T065702-Hero1BounceSmallHorizontalSlashVFXChangeRequestV2-Operator
OperatorManifest: C:\UE\T66\Reports\AgentReviews\ClaudeDirectRead\20260529T065702-Hero1BounceSmallHorizontalSlashVFXChangeRequestV2-Operator\manifest.json
ExpectedValidatorDepth: deepened
ValidatorBudgetHint: check the Phase 1 packet for carrier/context/process completeness; do not rediscover implementation.

## Operator Position And Open Decisions

Claude recommended `ImpactAnchored` ArcSlash per link and flagged the scaffold's `BeamHop` carrier as conflicting with the user's new slash wording.

Codex resolves the open carrier decision as: `ImpactAnchored` ArcSlash per link for the primary structural carrier, with optional BeamHop/RibbonTrail support deferred.

Codex resolves the open context-policy decision as: `PerChainLink`, because Bounce has separate impact points and future downstream systems should not infer them from an aggregated context.

No user-only decision blocks Phase 1.

## Anti-Lookalike Discriminator

Cheap wrong result: reuse the legacy `SpawnBounceVFX` pixel/segment line, recolor the AOE crescent, or spawn one static slash at the hero/primary only.

Discriminator: production Bounce must use a Niagara/material/mesh horizontal slash carrier at each chain impact point, publish/record per-link Bounce weapon impact contexts, and prove primary and second-target hits through combat logs plus multi-frame Unreal-owned capture.

