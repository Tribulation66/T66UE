Codex Approval: APPROVE

## Approved Task

Build the Hero 1 / Chad 1 Pierce weapon projectile VFX as a forward vertical slash using the red, blue, and white Hero 1 AOE material language.

## Approved Scope

- Update `Gameplay/Combat/Hero1AxePierceMechanismPacket.md`.
- Add a Pierce production binding row for `Hero_1_black_pierce`.
- Regenerate the matching Combat VFX DataTable asset from source CSV.
- Add Pierce lab assets under `/Game/VFXLab/Hero1Axe/Pierce/` if needed for the reviewed flow.
- Add Pierce production assets under `/Game/VFX/Hero1/Axe/Pierce/`.
- Extend `Source/T66/Gameplay/T66CombatComponent.cpp` so Pierce can spawn a bound PathAnchored production VFX from its official impact context.
- Extend binding setup/validation scripts while preserving all existing AOE behavior and checks.
- Add only the minimal proof/capture automation needed to validate this Pierce pass.

## Approved Tool Surface

Claude may use full Operator tooling through:

`Scripts\Invoke-ClaudeDirectRead.ps1 -Mode Operator -Model claude-opus-4-8 -Effort high -ToolProfile FullOperator -PermissionMode bypassPermissions -CodexApprovalPath Reports\AgentReviews\Hero1PierceVerticalSlashVFX\codex_operator_approval.md -AddDir C:\UE\T66`

Allowed within scope: file edits, shell commands, Unreal commandlets/editor automation, asset setup/promotion scripts, compile commands, validators, and Unreal-owned capture scripts.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Gameplay/Combat/VFX_PROCESS_INDEX.md`, `CombatVFXAuthoringProcedure.md`, `CombatVFXDefinitionOfDone.md`, `CombatVFXVisualDamageAlignmentContract.md`, `CombatVFXImpactContextContract.md`, and `CombatVFXGeneratedAssetPolicy.md`.
- Preserve AOE row behavior, AOE production assets, and `BaseVisualRadius=411.4` enforcement.
- Use Niagara/material/renderer/emitter logic for the primary Pierce silhouette; do not replace it with actor-side debug geometry or procedural C++ helper shapes.
- Damage authority remains combat logic, not Niagara collision or render geometry.
- Use Unreal-owned capture paths for visual proof.

## Explicitly Excluded Actions

- No Git commit, push, tag, reset, checkout, clean, or broad Git/LFS scan.
- No Mini/minigame inspection or edits.
- No DOT, Bounce, idol, or unrelated weapon implementation.
- No credential, billing, Anthropic API, or environment-token changes.
- No destructive deletes outside new Pierce lab/production artifacts created by this approved run.
- No final `FULL` visual-fidelity claim without captured evidence and user/Pablo approval where the effect packet requires it.

## Verification Required After Operator Run

- Binding setup script run and DataTable refresh result.
- `Scripts/ValidateCombatVFXProductionBindings.py` pass or exact failure.
- Focused compile/build verification for affected C++.
- Runtime log proof that Pierce spawns the bound production VFX and still applies authoritative Pierce damage.
- Unreal-owned gameplay video/evidence bundle if capture route succeeds; otherwise a concrete capture blocker and no visual-complete claim.
- Completion packet under `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/completion_packet.md`.

## Approval Rationale

The Operator Change Request is bounded, process-aware, and anchored to the current Combat VFX infrastructure. The user has provided enough visual direction for the structural implementation. The only user-owned decision is final visual approval/imagegen target, which does not block building and testing the reusable Pierce VFX structure.
