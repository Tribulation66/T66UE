# Operator Packet: Hero 1 Pierce Vertical Slash VFX

## Working Task And Validation Depth

Working task: build the Hero 1 / Chad 1 Pierce weapon projectile VFX as a forward vertical slash, using the red, blue, and white material language from the Hero 1 AOE slash family.

Validation depth: full Combat VFX validation. This is production VFX binding/runtime work with Niagara assets, data refresh, gameplay integration, and Unreal-owned proof requirements.

Scope: `Hero_1_black_pierce`, Pierce attack category, Combat VFX binding/data setup, Hero 1 axe Pierce packet, reusable runtime spawn path for PathAnchored weapon VFX, production/lab Pierce assets, validator/proof updates. Mini/minigame scope is excluded.

Stop condition: validated implementation and Unreal-owned evidence, or a protocol decision gate if Claude discovers a user-only decision.

## Roles And Tool Profile

Operator: Claude, `claude-opus-4-8`.

Validator/Finisher: Codex.

Read-only Operator run: `Reports/AgentReviews/ClaudeDirectRead/20260529T053439-Hero1PierceVerticalSlashVFXChangeRequest-Operator`.

Read-only manifest: `Reports/AgentReviews/ClaudeDirectRead/20260529T053439-Hero1PierceVerticalSlashVFXChangeRequest-Operator/manifest.json`.

Full Operator approval artifact: `Reports/AgentReviews/Hero1PierceVerticalSlashVFX/codex_operator_approval.md`.

Approved full Operator profile: `Scripts/Invoke-ClaudeDirectRead.ps1 -Mode Operator -Model claude-opus-4-8 -Effort high -ToolProfile FullOperator -PermissionMode bypassPermissions -CodexApprovalPath Reports/AgentReviews/Hero1PierceVerticalSlashVFX/codex_operator_approval.md -AddDir C:\UE\T66`.

## User Constraints And Out Of Scope

User direction: Pierce should be a slash, specifically a vertical slash forward, using the same red/blue/white colors and material/texture language as the AOE attack.

Implementation is approved by the user's "go on and make" request.

Out of scope: DOT, Bounce, idols, Mini/minigame systems, balance/stat retuning, Git commit/push/tag/reset/clean, broad Git/LFS scans, credential/billing changes, imagegen visual-target approval as a hard blocker for structural binding work.

## Applicable Instructions Read

- `AGENTS.md`: prompt-native task contract, always-on validation, Claude Operator/Codex Validator, Combat VFX process fidelity, Unreal-owned capture, no Tier footer.
- `OPERATOR_VALIDATOR_PROTOCOL.md`: Codex approval artifact required before Claude full Operator mode, first-line approval contract, full Operator tool profile, token accounting.
- `Reports/AGENTS.md`: durable review/approval artifacts belong under `Reports/AgentReviews/<TaskSlug>`.
- `Gameplay/GAMEPLAY_AGENTS.md`: combat VFX routes through `Gameplay/Combat/CombatVFXAuthoringProcedure.md`; gameplay runtime changes need compile/build verification.
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`: start future combat VFX from the index, per-effect packet, alignment proof, impact-context proof, and production binding review.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`: ArcSlash/PathAnchored carrier, mask/material/manifold gates, visual target gate, editor isolation, gameplay capture, anti-lookalike.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`: production binding, gameplay capture, visual/damage alignment, and impact-context evidence matrix.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`: Pierce is `PathAnchored`; visual path endpoints and width must map to authoritative line/capsule damage.
- `Gameplay/Combat/CombatVFXImpactContextContract.md`: weapon effects that can drive downstream sources publish official impact contexts with source identity and impact point.
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`: CSV and DataTable move together; production `.uasset` paths require packet/validator review.
- `Gameplay/Combat/MASTER_COMBAT.md`: current combat runtime spine, Pierce/AOE behavior, temporary projectile contract, VFX as presentation.
- Relevant pending issues: Combat, Gameplay, Scripts, and Data pending issue files read; none block this Pierce pass.

## Evidence And Live Findings

Claude's read-only Operator artifact is `Reports/AgentReviews/ClaudeDirectRead/20260529T053439-Hero1PierceVerticalSlashVFXChangeRequest-Operator/claude_direct_read_operator.md`.

The current Pierce packet is an infrastructure scaffold and says the old intended read was "straight horizontal axe-force slash or fissure that travels along a line." The user now explicitly overrides that with "vertical slash forward."

Current runtime seam in `Source/T66/Gameplay/T66CombatComponent.cpp`: `PerformPierce` builds `FT66CombatImpactContext` with `ImpactPoint`, `Forward`, `LineLength`, `TubeRadius`, and `bImpactPointValid=true`, then publishes the weapon context, but does not spawn a bound production VFX. `PerformSlash` already calls `TrySpawnBoundWeaponBaseSlashVFX(...)` after building its impact context, so Pierce should mirror the binding-driven spawn seam.

Current data seam: `Content/Data/CombatVFXBindings.csv` has the active AOE row only. `Content/Data/Weapons.csv` contains `Hero_1_black_pierce` as the first Pierce production target.

Current script seam: `Scripts/SetupCombatVFXBindingsDataTable.py` and `Scripts/ValidateCombatVFXProductionBindings.py` are AOE-aware and must be extended without weakening the existing `Hero1Axe_AOE_Base` and `BaseVisualRadius=411.4` checks.

## PPF And Process Gates

PPF CHECK
Objective: create the Pierce projectile VFX as a forward vertical slash using Hero 1 AOE material language.
Proven process: `Gameplay/Combat/CombatVFXAuthoringProcedure.md`, `VFX_PROCESS_INDEX.md`, `CombatVFXVisualDamageAlignmentContract.md`, `CombatVFXImpactContextContract.md`, and the existing Hero 1 AOE production binding process.
My planned implementation: update the Pierce effect packet, author lab and production Niagara/material/mesh assets, add a Pierce production binding row and DataTable refresh, add PathAnchored runtime spawn support, and validate with structural scripts plus Unreal-owned visual/gameplay proof.
Same method class: YES for structural implementation and binding proof.
If NO, why: N/A.
User approval required before proceeding: NO for structural implementation; YES later for final visual acceptance if a same-view imagegen visual target is required.
Verification evidence: script validation, compile, logs, Unreal-owned capture/evidence bundle, and runtime `CombatVFXProductionSpawned` proof.

ARTIFACT PARITY GATE
Reference artifact/category: Hero 1 AOE Niagara/material/texture family.
Role: Primary.
Required: YES.
Planned artifact/path: `/Game/VFXLab/Hero1Axe/Pierce/NS_Hero1AxePierce_MeshSlash` and `/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash`.
Status: EQUIVALENT.
Evidence: same material/color/texture vocabulary reused, with new vertical PathAnchored carrier geometry and Pierce-specific line footprint.

MECHANISM MANIFEST
Reference/source: user's Pierce visual direction plus `CombatVFXAuthoringProcedure.md` ArcSlash/PathAnchored rules.
Required mechanisms:
1. Mechanism: forward lane travel along the Pierce direction.
   Required: YES.
   Planned implementation: Niagara/carrier and runtime spawn parameters use `Forward` and `LineLength`.
   Evidence needed: multi-frame capture showing the carrier moving or revealing forward along the lane.
2. Mechanism: vertical slash-plane silhouette.
   Required: YES.
   Planned implementation: authored mesh/ribbon/sprite carrier with vertical blade-plane orientation, not AOE radial crescent geometry.
   Evidence needed: same-view and gameplay captures where Pierce is distinguishable from AOE.
3. Mechanism: Hero 1 AOE material language reuse.
   Required: YES.
   Planned implementation: reuse or derive from existing AOE red/blue/white material/texture parameter vocabulary.
   Evidence needed: asset/material inspection and capture readability.
4. Mechanism: visual/damage alignment.
   Required: YES.
   Planned implementation: PathAnchored footprint maps to `LineLength` and `TubeRadius`; damage remains the authoritative Pierce line/capsule query.
   Evidence needed: debug DamageVolume/log proof plus VFX path/width evidence.

## Proposed Patch Approach

1. Update `Gameplay/Combat/Hero1AxePierceMechanismPacket.md` from scaffold to active vertical-forward-slash packet.
2. Add `Hero1Axe_Pierce_Base` to `Content/Data/CombatVFXBindings.csv`.
3. Regenerate `Content/Data/DT_CombatVFXBindings.uasset` from the CSV.
4. Add or adapt lab authoring/promotion scripts or commandlet paths needed for `/Game/VFXLab/Hero1Axe/Pierce/`.
5. Promote production assets under `/Game/VFX/Hero1/Axe/Pierce/`.
6. Extend `TrySpawnBoundWeaponBaseSlashVFX` for PathAnchored Pierce scaling/orientation without changing AOE behavior.
7. Call the bound VFX spawn from `PerformPierce` after `PierceImpactContext` is built.
8. Extend setup/validation scripts so AOE checks remain intact and Pierce checks become required.
9. Add or extend capture/proof automation only as needed for Pierce evidence.

Rollback: remove the Pierce binding row and regenerate the DataTable; remove the Pierce spawn call/PathAnchored branch; remove new Pierce asset folders and scripts/docs created by this pass.

## Verification Plan

- Verify no `ANTHROPIC_API_KEY` is set before Claude full Operator run.
- Run the full Claude Operator implementation through the approved helper profile.
- Validate changed files against the approved scope.
- Run the binding setup script and `Scripts/ValidateCombatVFXProductionBindings.py`.
- Run focused C++ compile for affected runtime code.
- Run Unreal-owned gameplay capture, preferably with evidence bundle, showing Pierce attack, spawned production VFX, and damage/log proof.
- Use ffprobe/contact sheet/log excerpts for video evidence.
- Do not claim final production visual acceptance without captured evidence and user/Pablo approval where required.

## Token Routing

TOKEN ROUTING
OperatorModel: claude-opus-4-8
OperatorTokensSpent: 1313673
OperatorRunDir: Reports/AgentReviews/ClaudeDirectRead/20260529T053439-Hero1PierceVerticalSlashVFXChangeRequest-Operator
OperatorManifest: Reports/AgentReviews/ClaudeDirectRead/20260529T053439-Hero1PierceVerticalSlashVFXChangeRequest-Operator/manifest.json
CodexApprovalPath: Reports/AgentReviews/Hero1PierceVerticalSlashVFX/codex_operator_approval.md
ExpectedValidatorDepth: deepened
ValidatorBudgetHint: check packet completeness, approved scope, AOE row preservation, Pierce row/path, PathAnchored runtime branch, script validator behavior, compile/capture/log evidence.

## Operator Position And Open Decisions

Operator position: approve structural implementation. The user has given enough direction for a first Pierce build, and no user-only decision blocks the structure.

Open decision: final visual acceptance may require an imagegen same-view target or user/Pablo approval after capture. This is not a blocker for the structural implementation pass.

## Anti-Lookalike Discriminator

Cheap wrong result: the Pierce VFX is just the AOE crescent recolored or rotated, or a static vertical mesh stuck to the hero.

Discriminator: Pierce is PathAnchored, uses line length and tube radius, travels or reveals forward along the locked Pierce lane, and is visually distinct from the AOE radial/band-anchored crescent in same-view and gameplay captures.
