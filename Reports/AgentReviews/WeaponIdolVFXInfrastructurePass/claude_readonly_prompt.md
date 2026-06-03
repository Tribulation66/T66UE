You are Claude acting as read-only Operator for `C:\UE\T66`. Codex is Validator.

Working task:
Operator: Claude, read-only.
Validator: Codex.
Scope: inspect the current weapon/idol Combat VFX infrastructure after Hero 1 has four weapon categories and four idol category proofs. Identify what is placeholder, scaffolding, temporary, duplicated, awkward, or cleanup-worthy, and recommend behavior-preserving infrastructure improvements. Do not edit files.
Stop condition: write a complete read-only Operator packet with findings, priorities, and recommended behavior-preserving next steps.

Process rules:
- Do not use native goal tools.
- Read live repo files, not memory.
- No Mini/minigame scope.
- Do not modify files.
- Do not run broad git/LFS scans over `Content/` or generated binary folders.
- This is an infrastructure assessment, not visual production. Do not propose final Niagara art as the immediate scope unless classified as deferred production work.

Read first:
- `AGENTS.md`
- `.t66/operator-state.json`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/pending_issues_Combat.md`

Inspect likely implementation seams:
- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Source/T66/Gameplay/T66CombatComponent.h`
- `Source/T66/Gameplay/T66CombatVFX.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1`
- Existing weapon/idol proof scripts under `Scripts/RunHero1Axe*Proof.ps1`
- `Content/Data/CombatVFXBindings.csv`
- `Content/Data/Idols.csv`
- `Content/Data/Weapons.csv` if relevant
- `Reports/AgentReviews/IdolCategoryNativeImpact/operator_packet.md`
- `Reports/AgentReviews/IdolCategoryNativeImpact/codex_validator_report.md`

Answer shape:
1. Inventory of current production vs proof-only vs placeholder/scaffolding.
2. Highest-leverage behavior-preserving cleanup opportunities.
3. Specific temporary names/log fields/scripts/branches that should be generalized.
4. Risks and what not to touch yet.
5. Suggested phased plan for the next infrastructure pass, with validation per phase.

Classify each recommendation:
- Behavior-preserving cleanup now.
- Tooling/validator hardening.
- Documentation/process cleanup.
- Deferred production art/content.
- Needs user decision.

Output requirement:
Write `Reports/AgentReviews/WeaponIdolVFXInfrastructurePass/operator_packet.md`.
The first non-empty line must be exactly:
`Operator Packet: COMPLETE`

Include file/line anchors for important claims where feasible.
