Verdict: APPROVE

# Codex Validator Check - Combat VFX Impact Context Process Docs

## Packet Completeness Gate

- Working goal and tier: Present. The packet scopes this as a Tier 1 docs/process update for a reusable weapon/idol impact-context schema.
- Roles and tool profile: Present. Claude is Operator and Codex is Validator/Integrator; no writes by Claude.
- User constraints and out of scope: Present. Runtime code, assets, Niagara authoring, and validator-tool code are out of scope.
- Applicable instructions read: Present. Packet names `AGENTS.md`, `Gameplay/GAMEPLAY_AGENTS.md`, `Reports/AGENTS.md`, and `VFX_PROCESS_INDEX.md`.
- Evidence and live findings: Present and consistent with Codex live reads of the combat VFX docs.
- PPF and process gates: Present as documentation gates G1-G7; no asset/media PPF result is being accepted in this docs-only pass.
- Proposed patch approach: Present and bounded to a new contract doc plus existing process docs.
- Verification plan: Present. It calls for cross-link, Water-only-language, DoD, template, and round-trip checks.
- Token routing: Present. Claude Operator tokens are reported by helper manifest; Codex uses the active goal token count.
- Operator position and open decisions: Present. Recommended defaults are safe: insert after alignment contract and cross-reference pending items without closing them.
- Anti-lookalike discriminator: Present. It distinguishes geometry alignment from source identity and runtime context parity.

## Targeted Anchor Check

- `Gameplay/Combat/VFX_PROCESS_INDEX.md` currently lacks the impact-context contract in read order. Patch required.
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md` currently has visual/damage alignment gate only. Patch required for separate impact-context gate.
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md` currently lacks a hard weapon-context/idol-context parity gate. Patch required.
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md` already defines `Impact point`; patch should cross-link identity/parity to the new contract without duplicating it.
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md` already has parent-source and own-idol context language; patch should reference the new reusable proof schema.
- `Gameplay/Combat/EffectPacketTemplate.md` has alignment fields but no impact-context contract block. Patch required.
- `Gameplay/Combat/pending_issues_Combat.md` should keep existing Water and alignment issues open, and add validator/tooling enforcement for the new context contract as out-of-scope.

## Integration Position

Proceed with Codex integration using the Operator packet. Keep the new contract generalized and theme-agnostic, with Water/Earth only as current worked proof examples.
