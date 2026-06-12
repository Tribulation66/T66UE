You are Claude acting as a direct-read T66 operator. This is an operator artifact, not a review greenlight. Inspect files with the allowed read-only tools and produce a concrete proposal, findings, or handoff for Codex to integrate. Do not edit files, run shell commands, invoke Unreal Python, automate editors, or perform production asset writes.
# Claude Operator Prompt: Combat VFX Impact-Context Process Docs

You are Claude acting as read-only Operator for `C:\UE\T66`.

## Task

Produce a complete Operator Packet for this Tier 1 documentation/process update:

With Claude as Operator and Codex as Validator/Integrator, update the Combat VFX process docs with a reusable impact-context and idol proof schema for all future weapon and idol VFX work.

The user explicitly requested:

- make sure Claude is the Operator,
- read the AGENTS file for the new Operator/Validator guidelines,
- update existing process docs,
- generalize the Water proof into a reusable schema,
- update all docs that need updates.

## Must Read

Read these live files and cite exact anchors in your packet:

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `.t66/operator-state.json`
- `Reports/AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/CombatVFXGeneratedAssetPolicy.md`
- `Gameplay/Combat/EffectPacketTemplate.md`
- `Gameplay/Combat/pending_issues_Combat.md`

Use focused source/runtime anchors only if needed for the schema:

- `Source/T66/Gameplay/T66CombatComponent.cpp`
- `Scripts/RunHero1AxeAOEWaterIdolImpactProof.ps1`

## Required Output Format

Your output must be an Operator Packet with all required sections from `OPERATOR_VALIDATOR_PROTOCOL.md`:

1. Working Goal And Tier
2. Roles And Tool Profile
3. User Constraints And Out Of Scope
4. Applicable Instructions Read
5. Evidence And Live Findings
6. PPF And Process Gates
7. Proposed Patch Approach
8. Verification Plan
9. Token Routing
10. Operator Position And Open Decisions
11. Anti-Lookalike Discriminator

## Patch Intent To Evaluate

Preferred direction, unless live repo evidence contradicts it:

- Add `Gameplay/Combat/CombatVFXImpactContextContract.md` as the focused owner document for impact context publication, consumption, source identity, neutral controls, and diagnostics.
- Update:
  - `Gameplay/Combat/VFX_PROCESS_INDEX.md`
  - `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
  - `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
  - `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
  - `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
  - `Gameplay/Combat/EffectPacketTemplate.md`
  - `Gameplay/Combat/pending_issues_Combat.md`

Define a generalized proof schema that is not Water-specific. It should cover:

- weapon context publication,
- idol context consumption from weapon context,
- idol `SourceType=IdolModifier`,
- idol `SourceID=<idol>`,
- `ParentSourceID=<weapon source>`,
- context parity,
- skip/fallback counters,
- damage source proof,
- neutral-control proof,
- wording that video proof is not enough without runtime context/damage logs.

Do not propose runtime code changes in this pass unless a doc cannot honestly describe a reusable schema without them. If tooling validator changes are needed, mark them as pending issues, not as this pass.

## Validator Budget Hint

Make your anchors precise enough that Codex can perform targeted validation rather than rediscovering the whole repo.

