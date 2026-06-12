You are Claude acting as Operator for `C:\UE\T66`. Codex is Validator.

This is a recovery continuation for `IdolCategoryNativeImpact`.

Working task:
Operator: Claude (`claude-opus-4-8`) using FullOperator.
Validator: Codex.
Scope: complete and prove full category-native idol behavior for `Idol_Light` Pierce, `Idol_Electric` Bounce, and `Idol_Poison` DOT triggered from the Hero 1 AOE weapon impact point. Preserve the existing `Idol_Water` AOE path. No Mini/minigame scope.
Stop condition: write `Reports/AgentReviews/IdolCategoryNativeImpact/operator_packet.md` with first line exactly `Operator Packet: COMPLETE`, or stop with a process-valid blocker.

Recovery context:
- A first FullOperator run was stopped after a long stall.
- It produced no `operator_packet.md` and no helper stdout/stderr.
- It did make partial edits in at least:
  - `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - `Scripts/CaptureT66GameplayVideo.ps1`
- You must inspect the current state before editing.
- Large files already contain unrelated prior work. Do not revert, simplify, or rewrite unrelated blocks.

Read these before acting:
- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Reports/AgentReviews/IdolCategoryNativeImpact/codex_operator_approval.md`
- `Reports/AgentReviews/IdolCategoryNativeImpact/codex_operator_approval_recovery.md`
- `Reports/AgentReviews/ClaudeDirectRead/20260530T002856-IdolPierceBounceDOTPlanning-Operator/claude_direct_read_operator.md`

Implementation requirements:
- Generalize the Water-only idol impact presentation path into reusable category dispatch.
- The AOE weapon impact point must be the upstream trigger and parent context.
- Proof idols:
  - `Idol_Light` = Pierce.
  - `Idol_Electric` = Bounce.
  - `Idol_Poison` = DOT.
  - `Idol_Water` = AOE regression.
- Each proof idol must use its own damage source.
- Each proof idol must officialize its own impact context:
  - `SourceType=IdolModifier`
  - `SourceID=<idol id>`
  - `ParentSourceID=Hero_1_black_aoe`
  - valid `ImpactPoint`
  - category-appropriate context details.
- Preserve or add generalized diagnostics such as `CombatImpactChainDiagnostic`.
- Presentation may be placeholder/projectile-style proof only. Do not author final Niagara art.
- Extend proof capture arguments/modes only as needed. Preserve existing Water alias behavior.

Verification requirements:
- Run a focused compile for the affected target.
- Run current Unreal-owned proof captures/log checks for:
  - `Idol_Light`
  - `Idol_Electric`
  - `Idol_Poison`
  - `Idol_Water` regression if feasible.
- Check logs for:
  - weapon primary `CombatImpactContext` with `SourceID=Hero_1_black_aoe`;
  - idol `CombatImpactContext`/diagnostic with `SourceType=IdolModifier`, correct `SourceID`, and `ParentSourceID=Hero_1_black_aoe`;
  - idol-owned `DamageBySource SourceID=<idol>`;
  - no unintended double legacy projectile lane for impact-presentation idols.
- If proof cannot be run, state the command attempted and exact blocker.

Output:
Write `Reports/AgentReviews/IdolCategoryNativeImpact/operator_packet.md`.
First non-empty line must be exactly:
`Operator Packet: COMPLETE`

Packet must include changed files, compile/capture commands and outcomes, artifact paths, log anchors, caveats, and token usage if available.
