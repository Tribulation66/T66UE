# Original User Prompt - Fall Guys Full Physics Roadmap Review

User asks for no implementation yet. The requested output is analysis and planning only:

- Review the attached Claude proposal for a full Fall Guys-style hero physics feel.
- Give Codex + Claude's opinion on whether the proposal is the right direction.
- Treat the target as a broad `Physics` infrastructure layer, not physics/obstacle-only.
- Build toward heroes feeling like Fall Guys: bouncy/wobbly locomotion, balance, knockdown, get-up, obstacle-driven chaos.
- Monsters can later receive a different or cheaper degree of chaos.
- Provide an implementation roadmap to reach the full feel.

Working task:
Operator: Codex
Validator: Claude
Scope: Read-only review of the attached Claude proposal plus live T66 physics/ragdoll/trap context, then an implementation roadmap for a dedicated hero-first physics framework with no code changes.
Stop condition: Deliver a repo-grounded opinion and phased plan, with Claude cross-check and token reporting.

Relevant repo rules:

- `AGENTS.md` is the root process router.
- Do not use native goal tools for T66 work.
- Respect the user's planning-only boundary.
- Every prompt uses the Operator/Validator loop when the configured Validator is available.
- `.t66/operator-state.json` currently selects Codex as Operator and Claude as Validator.
- Claude must be run through local Claude Code helper scripts after confirming no `ANTHROPIC_API_KEY` is set.
- Current gameplay source and docs must override stale memory or prior reports.

Attachment to review:

`C:\Users\DoPra\.codex\attachments\04805a4d-4da0-4926-84f8-111673a995f5\pasted-text.txt`

Key live repo context already inspected for this prompt:

- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Source/T66/Gameplay/T66KnockbackComponent.h`
- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
- `Source/T66/Gameplay/Traps/T66TrapBase.h`
- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66Editor/T66CreateTestRoomPhysicsAssetCommandlet.cpp`
- `Reports/AgentReviews/FriendSlopUnrealRagdollImport/friendslop_humanoid_controlled_physics_asset_report.json`

Important live facts:

- `Source/T66/Gameplay/pending_issues_Gameplay.md` records the current hero direction as pure Chaos ragdoll with PAC off. It explicitly says PAC should not be tuned for hero ragdoll unless the hero physics architecture is reopened.
- `FT66KnockbackProfile` defaults still show a hit-triggered ragdoll profile, not always-on active ragdoll.
- `UT66KnockbackComponent::ApplyKnockbackLaunch` force-disables PAC for hero profiles and for detached ragdoll profiles.
- `UT66KnockbackComponent::ApplyPhysicalAnimationDrive` currently sets `FPhysicalAnimationData::bIsLocalSimulation=false`.
- The current TestRoom wipeout arm is enabled by default and has become stronger/more bouncy, but it remains a TestRoom-only hit-triggered prototype routed through `UT66KnockbackComponent`.
- Production traps are activation/damage/progression actors, not physical obstacle/reaction actors.
- `Gameplay/README.md` still has no `Physics` owner.

Please provide an independent read-only answer and, after Codex draft is supplied, cross-review it for missed constraints, bad architecture assumptions, and unclear phasing.
