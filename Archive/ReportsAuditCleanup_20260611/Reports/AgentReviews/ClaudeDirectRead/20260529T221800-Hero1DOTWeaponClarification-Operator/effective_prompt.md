You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to validate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, that requires the full Operator tool surface; do not claim a current-verification request is satisfied by recent or prior evidence. State plainly that this read-only profile cannot run it and that a full Operator run is required.
Working task:
Operator: Claude.
Validator: Codex.
Scope: Read-only clarification for Hero 1 DOT weapon projectile/VFX/combat path in C:\UE\T66. Mini/minigame systems are out of scope.
Stop condition: Identify whether the user's requested DOT weapon behavior has any blocking design questions before implementation, and summarize the safest implementation assumptions.

Repo rules to honor:
- Do not edit files.
- Use live repo state only.
- Treat AGENTS.md as the root process router.
- This is clarification/read-only only, not an implementation phase.

User request:
"Ok great job, the last one is going to be the DOT weapon, and for this one it will also a single projectile shot, so we can literally use what we already have for bounce but once it hits the enemy, we need a ticking damage, so what we can do is 3 projectiles will spawn on the enemy and tick for damage, and these projectiles can be small spheres for now. Let me know if you have any questions."

Context already observed by Codex:
- .t66/operator-state.json selects Claude as Operator and Codex as Validator.
- Hero 1 DOT weapon rows exist in Content/Data/Weapons.csv.
- Gameplay/Combat/Hero1AxeDOTMechanismPacket.md is currently infrastructure-only and describes future DOT as a persistent target aura.
- Source/T66/Gameplay/T66CombatComponent.cpp has PerformDOT at about line 2440. It publishes a DOT impact context, applies initial damage, and calls CachedRunState->ApplyDOT with HeroPrimaryDotSource. It does not currently stage a Bounce-style moving projectile before DOT is applied.
- Bounce now has reusable moving visual-only projectile staging via StageBounceProjectileChain / SpawnBounceLinkProjectile, including arrival callbacks.
- Source/T66/Core/RunState/T66RunStateSubsystem_Idols.cpp stores active DOTs by target plus SourceIdolID, so multiple simultaneous DOT lanes require distinct source IDs; using one source ID refreshes/replaces the existing source for that target.

Please answer:
1. Is there any blocking clarification question for the user before implementation?
2. If not, what assumptions should we state before proceeding?
3. What is the minimal implementation approach that fits the existing Bounce/DOT seams?
4. What evidence should Codex require after implementation?

