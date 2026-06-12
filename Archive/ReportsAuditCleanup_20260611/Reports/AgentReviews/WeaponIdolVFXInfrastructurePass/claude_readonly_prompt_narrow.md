You are Claude acting as read-only Operator for `C:\UE\T66`. Codex is Validator.

Task: read-only infrastructure assessment for the current weapon/idol Combat VFX system. Do not edit files.

Scope:
- Four weapon categories now exist in the Hero 1 VFX workstream: AOE, Pierce, Bounce, DOT.
- Four idol category proofs now exist: Water/AOE, Light/Pierce, Electric/Bounce, Poison/DOT.
- Identify placeholder/scaffolding/temporary parts and behavior-preserving infrastructure cleanup opportunities.
- No Mini/minigame scope. Do not inspect or discuss Mini.
- Do not propose behavior changes as cleanup. Behavior changes must be explicitly labeled deferred or needs user decision.

Read only these anchors unless blocked:
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXInfrastructureInventory.md`
- `Gameplay/Combat/pending_issues_Combat.md`
- `Reports/AgentReviews/IdolCategoryNativeImpact/codex_validator_report.md`
- `Source/T66/Gameplay/T66CombatComponent.cpp` around:
  - `UsesImpactPresentationForIdol`
  - `CombatVFXIdolProjectileLaneSuppressed`
  - `CombatImpactChainDiagnostic`
  - the Pierce/Bounce/DOT/AOE idol category switch
- `Source/T66/Gameplay/T66CombatVFX.cpp` around `SpawnIdolImpactPlaceholderVFX`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp` around `hero1axeaoewateridolimpact` proof idol selection/target specs
- `Scripts/CaptureT66GameplayVideo.ps1` around `Hero1AxeProofIdol`
- `Scripts/RunHero1AxeIdolCategoryNativeImpactProof.ps1`

Write `Reports/AgentReviews/WeaponIdolVFXInfrastructurePass/operator_packet.md`.
First non-empty line exactly:
`Operator Packet: COMPLETE`

Packet format:
1. One-paragraph current-state summary.
2. Inventory table: production, proof-only, placeholder, scaffolding, temporary names/logs, validator gaps.
3. Prioritized cleanup plan:
   - Phase 1: behavior-preserving code naming/structure cleanup.
   - Phase 2: proof runner / validator hardening.
   - Phase 3: docs/process consolidation.
   - Phase 4: deferred production art/content.
4. Specific no-touch list for behavior changes.
5. File/line anchors for important claims.

Keep it concise and concrete. If exact line numbers are expensive, use searchable symbol/name anchors.
