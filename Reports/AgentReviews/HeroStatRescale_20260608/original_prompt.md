User request:

Ok we need to do a full rescale of stats, because right now the gain is all very small numerically, but the effect is too much, meaning I get too fast, get too much evasion etc. We need to do a large The base should be changed from 1-10 depending on the weight of stats for that character and every level up should give from 1-5 for every stat, depending on the weight, but the per level gain should be a fixed number not a range. However the impact of that in the gameplay needs to be greatly diminished, for example with base speed 2 the speed stat should 200 2x100, and and so on same for all the other stats.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: rescale the hero primary-stat system so authored bases are weight-style values around 1-10, fixed per-level gains are weight-style values around 1-5 instead of ranges, and runtime gameplay impact is dampened by converting authored stat points into a larger internal scale such as 2 -> 200.
Stop condition: either implement and verify the rescale end to end, or stop at the first user-only design decision if the exact stat weights/effect curves cannot be safely inferred from the live project.

Relevant repo rules:

- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator and Claude is Validator per .t66/operator-state.json.
- Validator pass is read-only: produce an independent repo-grounded answer/recommendation and flag any user-only decisions.
- Prefer data-authored tuning over hardcoded defaults.
- Gameplay runtime changes need compile/build verification and staged standalone validation when they affect the playable standalone.
- Relevant docs/files include Gameplay/GAMEPLAY_AGENTS.md, Gameplay/Stats/MASTER_STATS.md, Content/Data/Heroes.csv, Source/T66/Data/T66DataTypes.h, Source/T66/Core/T66GameInstance.cpp, Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp, Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp, and combat/stat consumers.
