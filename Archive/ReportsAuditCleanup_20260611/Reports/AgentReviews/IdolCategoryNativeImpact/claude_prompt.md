You are Claude acting as Operator for `C:\UE\T66`. Codex is Validator. The user requested full category-native idol behavior, with Claude implementing and Codex validating.

Working task:
Operator: Claude (`claude-opus-4-8`) using FullOperator.
Validator: Codex.
Scope: implement full category-native idol behavior for one Pierce idol, one Bounce idol, and one DOT idol triggered from the Hero 1 AOE weapon impact point, using the existing Water/AOE idol path as the reusable model. No Mini/minigame scope.
Stop condition: write the required Operator packet with implementation and proof evidence, or stop with a process-valid decision/blocker if implementation cannot proceed.

Important process rules:
- Do not use native goal tools.
- Read and follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Reports/AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, and the relevant Combat VFX docs.
- Do not inspect, edit, recommend, capture, validate, or include Mini/minigame systems.
- Do not use destructive git commands or revert unrelated user/Codex/peer changes.
- Use repo-owned Unreal capture routes for visual/gameplay proof.
- This is proof-bearing work, so current compile/capture/log proof must be attempted in this FullOperator phase. Prior evidence is not enough.

User decision:
- A prior read-only planning phase asked whether these idols should be presentation-only or full category-native behavior.
- The user answered: full category-native idol behavior.

Prior planning packet:
- `Reports/AgentReviews/ClaudeDirectRead/20260530T002856-IdolPierceBounceDOTPlanning-Operator/claude_direct_read_operator.md`
- Read it before implementation. It identified the core seam as the current Water-only impact-presentation branch and recommended default proof idols:
  - Pierce: `Idol_Light`
  - Bounce: `Idol_Electric`
  - DOT: `Idol_Poison`

Approved task:
Implement and prove full category-native idol behavior for `Idol_Light`, `Idol_Electric`, and `Idol_Poison` driven by the official `Hero_1_black_aoe` weapon impact point.

Required behavior:
- The AOE weapon projectile impact point is the upstream trigger and parent context for the idol effect.
- Every proof idol uses its own damage source under its own idol ID.
- Every proof idol has its own official impact context:
  - `SourceType=IdolModifier`
  - `SourceID=<idol id>`
  - `ParentSourceID=Hero_1_black_aoe`
  - valid `ImpactPoint`
  - category-appropriate context data where applicable.
- `Idol_Light` must behave as a Pierce idol from the AOE impact context.
- `Idol_Electric` must behave as a Bounce idol from the AOE impact context, chaining to another enemy when possible.
- `Idol_Poison` must behave as a DOT idol from the AOE impact context, with delayed/ticking idol-owned damage.
- Preserve/regression-check the existing `Idol_Water` AOE proof behavior.
- Presentation can be temporary placeholder/projectile-style proof. Do not author final Niagara art in this phase.
- Prefer a generalized reusable path over proof-idol hardcoding. A small proof allowlist is acceptable only if it is clearly isolated from the category dispatch and documented as proof gating.

Likely source seams to inspect:
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Content/Data/Idols.csv`
- `Content/Data/CombatVFXBindings.csv`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`

Known planning anchors:
- `UsesImpactPresentationForIdol` was Water-only.
- Water impact branch builds an idol `FT66CombatImpactContext` from `PrimaryWeaponImpactContext`, sets `ParentSourceID`, and applies idol-owned AOE damage.
- The capture mode `hero1axeaoewateridolimpact` and `-T66Hero1AxeAOEProofIdol` previously allowed only Water/Earth. Extend or add a generalized mode as needed, preserving the Water alias if touched.
- Existing bound production VFX can be attempted, but no final `IdolModifier` Niagara binding rows are required for this placeholder proof.

Verification required:
- Run a focused compile for affected C++.
- Run current Unreal-owned proof captures/log checks for `Idol_Light`, `Idol_Electric`, `Idol_Poison`, plus a Water regression if feasible.
- Validate logs for:
  - `CombatImpactContext` weapon primary source `Hero_1_black_aoe`;
  - idol modifier context for each proof idol with `ParentSourceID=Hero_1_black_aoe`;
  - idol-owned `DamageBySource SourceID=Idol_Light`, `Idol_Electric`, and `Idol_Poison`;
  - no unintended legacy projectile lane or double-damage lane for impact-presentation idols.
- Provide MP4/log paths for proof captures.

Output requirement:
Write `Reports/AgentReviews/IdolCategoryNativeImpact/operator_packet.md`.
The first non-empty line must be exactly:
`Operator Packet: COMPLETE`

Packet contents must include:
- changed files;
- implementation summary;
- verification commands and results;
- proof artifact paths;
- relevant log excerpts/search anchors;
- skipped verification or caveats;
- token usage if exposed.
