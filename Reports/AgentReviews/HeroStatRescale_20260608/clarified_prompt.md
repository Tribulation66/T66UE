User clarified request:

No you misunderstood I mean 2 base speed = 200 uu/s speed, and For now we can keep 1-99 stats dispay. We might have to rework that but go ahead and make this change now. Were not 100x the stats.

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: implement the clarified stat rescale without x100 stat display: keep 1-99 displayed stats, make Speed stat convert to 100 uu/s per point so BaseSpeed=2 means 200 uu/s, collapse hero per-level gains to fixed 1-5 values, and dampen primary stat gameplay coefficients.
Stop condition: code/data/docs updated, focused compile/build verification attempted, staged standalone validation handled or clearly caveated, and Claude validation incorporated.

Relevant repo rules:

- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator and Claude is Validator per .t66/operator-state.json.
- Validator pass is read-only: produce an independent repo-grounded implementation recommendation and flag missed scope/risk.
- Prefer data-authored tuning over hardcoded defaults.
- Runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect the playable standalone.
- Relevant docs/files include Gameplay/GAMEPLAY_AGENTS.md, Gameplay/Stats/MASTER_STATS.md, Gameplay/Movement/MASTER_MOVEMENT.md, Content/Data/Heroes.csv, Source/T66/Data/T66DataTypes.h, Source/T66/Core/T66GameInstance.cpp, Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp, Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp, and combat/stat consumers.
