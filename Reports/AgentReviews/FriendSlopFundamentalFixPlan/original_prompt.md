Working task:
Operator: Codex
Validator: Claude
Scope: read-only solution analysis for three issues: restoring a compatible walk animation, camera following thrown ragdoll motion, and making the ragdoll controlled/limited instead of twisty/spazzy.
Stop condition: give a repo-grounded recommendation for each issue, including what should change, where, and what Claude agrees/disagrees with; no code changes.

User request:
Ok so were actually not ready to do small tuning because there are some more fundemental issues, that need to be solved, so first, I'd like you to add the walk animation to the character, the same one that previous models used, I beleive it was either the Quotornious one or a self built one, there should be documentation for this, look into it, the second thing is the camera does not follow the character when he gets thrown that needs to be fixxed, lastly the ragdol, I think is ragdolling to many points so it becomes not like a rag doll, with its components still being physical for example in fall guys the arms will go over the body one leg will go one way while the other goes the other but the components dont twist, like the leg always looks like the solid leg, torso etc, ours is twsiting in every way so the torso gets completly deformed, let know if you understand what im saying very clearly if not let me know for more information from me, but this is the key point for the ragdoll, for the model to not full spazz out but its a controlled and limited ragdoll. Tell me what you and claude think should be the solutions for these 3 problems.

Repository rules:
- No code changes in this turn.
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Operator is Codex; Validator is Claude per .t66/operator-state.json.
- Use current live repo state and docs.
