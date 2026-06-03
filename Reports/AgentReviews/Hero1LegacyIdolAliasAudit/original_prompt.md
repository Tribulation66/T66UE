# Original User Request

Do the save proof audit so we can delete the legacy aliases and move on. Other than that its good go.

# Task Contract

Working task:
Operator: Codex unless the repo state says otherwise.
Validator: configured non-Operator if available.
Scope: audit save/proof/content references for legacy idol aliases, decide whether alias deletion is safe, and if safe make the scoped alias-removal/normalization changes needed to proceed with the temporary Hero 1/idol visual plan. Mini/minigames excluded.
Stop condition: report the audit result, any changes made, and current verification evidence.

# Relevant Repo Rules

- Root router: `AGENTS.md`.
- Role protocol: `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Current role state: `.t66/operator-state.json` says Codex operator and Claude validator.
- Do not use native goal tools.
- Default scope excludes Mini/minigames.
- Runtime-facing gameplay changes need compile/build verification and staged standalone validation when they affect playable standalone.
- Data asset import/reload work must run the owning Unreal commandlet/import script when source CSV/JSON changes affect DataTables.
- Do not claim current proof that was not run.

# Current Plan Context

Previous planning packet accepted:
`Reports/AgentReviews/Hero1TempShapesImplementationPlan/codex_draft.md`

The relevant plan step is to remove legacy idol aliases from authored temporary visual keys and runtime visual lookup, while keeping compatibility mappings only if live save/proof/content audit proves they are still needed.

# Validator Request

Provide an independent repo-grounded answer for this exact request:

1. What should be audited before deleting legacy idol aliases?
2. Which files/systems are most likely to contain save, proof, data, or runtime dependencies on aliases such as `Idol_Water`, `Idol_Storm`, `Idol_Electric`, `Idol_Light`, `Idol_Poison`, `Idol_Lava`, etc.?
3. If the audit finds no required dependencies, what scoped implementation is safe?
4. If it finds dependencies, what should be changed first before deleting aliases?
5. What verification should Codex run?

Return `Result: OK` if this can be handled internally. Return `Result: NEEDS_USER` only if the user must choose or approve a blocker.
