Codex Approval: APPROVE

## Approved Task

Implement and prove full category-native idol behavior for the first Pierce, Bounce, and DOT idol proofs driven by the official Hero 1 AOE weapon impact point.

## Roles

- Operator: Claude (`claude-opus-4-8`) through `Scripts\Invoke-ClaudeDirectRead.ps1`.
- Tool profile: `FullOperator`.
- Validator: Codex.

## Scope

Approved in scope:

- Use the existing Water/AOE idol impact-context path as the reusable reference.
- Generalize the AOE weapon impact-to-idol overlay pipeline so it can support all idol categories without hardcoding only Water.
- Add proof behavior for:
  - Pierce idol: `Idol_Light`.
  - Bounce idol: `Idol_Electric`.
  - DOT idol: `Idol_Poison`.
  - Preserve the existing AOE idol proof path for `Idol_Water`.
- The AOE weapon projectile impact point must be the upstream trigger and parent context for the idol effect.
- Each idol must publish or otherwise officialize its own impact context as `SourceType=IdolModifier`, `SourceID=<idol id>`, `ParentSourceID=Hero_1_black_aoe`, with a valid `ImpactPoint`.
- Each idol category must use its own category-native damage behavior and own damage source:
  - Pierce: line/pierce-style downstream damage from the AOE impact context.
  - Bounce: chained downstream damage from the AOE impact context to another enemy when possible.
  - DOT: delayed/ticking downstream damage owned by the DOT idol source.
- Presentation may use temporary placeholders and existing projectile-style placeholder helpers. Do not author final Niagara art in this phase.
- Extend the capture/proof harness only as needed to select and validate these proof idols.
- Add or update repo process/docs only if needed to record the generalized proof contract.

Explicitly out of scope:

- Mini/minigame systems.
- Final Niagara art, generated textures, imagegen work, or production idol VFX assets.
- Broad gameplay rebalancing.
- Staged standalone packaging unless a changed workflow explicitly requires it.
- Destructive git operations or reverting unrelated user/peer changes.

## Required Process

Follow:

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`

Prior planning packet:

- `Reports/AgentReviews/ClaudeDirectRead/20260530T002856-IdolPierceBounceDOTPlanning-Operator/claude_direct_read_operator.md`
- The user resolved its open decision as: full category-native idol behavior.

## PPF Approval Basis

Objective: extend the idol impact-context pipeline from Water/AOE to category-native Pierce, Bounce, and DOT idol behavior from the AOE weapon impact point.

Proven process: `Gameplay/Combat/CombatVFXImpactContextContract.md`, `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`, and the existing Water idol impact-context proof path.

Approved implementation method: generalize the Water-only branch into a reusable idol impact presentation/category-dispatch path with `Idol_Light`, `Idol_Electric`, and `Idol_Poison` as first proof idols.

Same method class: YES.

## Verification Required

At minimum, attempt current verification:

- Focused compile for the affected C++ target.
- Unreal-owned gameplay proof capture(s), using `Scripts\CaptureT66GameplayVideo.ps1` or an equivalent repo-owned capture route.
- Log proof for each proof idol:
  - weapon primary impact context for `Hero_1_black_aoe`;
  - idol modifier impact context with the correct `SourceID` and `ParentSourceID=Hero_1_black_aoe`;
  - category-native damage source under the idol ID;
  - no unintended extra legacy projectile lane or double damage lane for the proof path.
- Regression check for existing `Idol_Water` AOE impact behavior.

If any verification cannot be run, the Operator packet must say exactly what was attempted, why it failed, and what evidence is missing.

## Required Operator Output

Write a completion packet at:

`Reports/AgentReviews/IdolCategoryNativeImpact/operator_packet.md`

The first non-empty line must be exactly:

`Operator Packet: COMPLETE`

The packet must include:

- implemented files;
- verification commands and outcomes;
- proof artifact paths;
- relevant log excerpts or search anchors;
- known caveats or skipped verification;
- Claude token usage if exposed by the helper.
