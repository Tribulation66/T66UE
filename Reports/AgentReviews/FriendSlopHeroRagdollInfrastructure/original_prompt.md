Working task:
Operator: Codex
Validator: Claude
Scope: Implement hero-scoped ragdoll/knockback infrastructure for the FriendSlop/Fall Guys feel. The first playable target is the selected hero in the TestRoom wipeout-arm trap. Build the code/data shape so bosses, elites, and eventually cheap enemy fallbacks can be added later, but do not implement non-hero actors in this pass.
Stop condition: Code/data infrastructure compiles, TestRoom triggers the real hero ragdoll path instead of owning bespoke ragdoll state, and verification evidence is reported.

Original/latest user request:
Ok for now lets just do hero for the scope but build the data and infrastructure so that later on adding other elements to it, is easy, so go ahead and proceed.

Relevant prior user intent:
- The game should feel like Fall Guys: bouncy/physics-y.
- Traps and boss attacks should launch/incapacitate the hero so they cannot attack temporarily.
- The current TestRoom wipeout arm is the first validation surface.
- Current ragdoll behavior stretches/smears the FriendSlop body, so this is an infrastructure correction before tuning.
- User wants correct scalable infrastructure, not a TestRoom-only shortcut.

Repo/process constraints:
- C:\UE\T66, UE 5.7, gameplay logic in C++.
- Follow AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is operator, Claude is validator per .t66/operator-state.json.
- Runtime gameplay changes should prefer data-authored tuning over hardcoded C++ defaults and require compile/build verification plus staged standalone validation if playable standalone is affected.
- Keep scope hero-only for actor coverage.
- Do not mutate files in the Claude independent answer pass.

Current live context found by Codex before this pass:
- AT66HeroBase already owns CombatComponent and HeroMovementComponent.
- UT66CombatComponent already has SetAutoAttackSuppressed(bool).
- Hero visuals now use CharacterVisuals.csv; Hero_1_Chad points to SK_Hero_1_Chad_Male_FriendSlop plus raw walk/idle/jump/roll animation assets.
- T66GameMode_TestRoom.cpp currently owns wipeout-arm ragdoll state, PAC setup, movement/combat suppression, actor-follow, recovery, and applies AddImpulse(..., bVelChange=true) plus AddImpulseToAllBodiesBelow(..., bVelChange=true).
- The intended correction is to move reusable ragdoll/knockback ownership into a hero/character component and leave TestRoom as the trap trigger.

Please produce an independent repo-grounded implementation recommendation for Codex to compare against before editing:
- Which files/classes should own the component/data/profile interfaces?
- What minimal APIs should AT66HeroBase expose?
- How should TestRoom call the real path?
- What impulse/PAC defaults should avoid the current stretching while keeping future extensibility?
- What compile/proof checks should Codex run?
