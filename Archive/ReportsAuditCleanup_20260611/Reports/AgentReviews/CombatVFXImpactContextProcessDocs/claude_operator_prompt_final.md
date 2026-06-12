# Claude Operator Prompt Final: Combat VFX Impact-Context Process Docs

You are Claude acting as read-only Operator for `C:\UE\T66`.

Do not call tools for this run. Use only the live context supplied below and emit the Operator Packet directly. The prior direct-read attempts were limited by helper turn caps, and Codex has already supplied the required live-read findings.

Produce a complete Operator Packet for this Tier 1 documentation update:

With Claude as Operator and Codex as Validator/Integrator, update the Combat VFX process docs with a reusable impact-context and idol proof schema for all future weapon and idol VFX work.

## Live Context From Codex Reads

- `.t66/operator-state.json` currently sets `operator=Claude`, `validator=Codex`.
- `AGENTS.md` routes Tier 1 Claude/Codex work through `OPERATOR_VALIDATOR_PROTOCOL.md`.
- `Gameplay/GAMEPLAY_AGENTS.md` owns gameplay/combat docs and says combat VFX authoring reads `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md` routes agent-authored review packets under `Reports/AgentReviews`.
- `Gameplay/Combat/pending_issues_Combat.md` contains related pending items about Water placeholder area footprint and alignment validator enforcement.
- `VFX_PROCESS_INDEX.md` is the quick-start router. Its read order currently includes authoring procedure, definition of done, visual/damage alignment contract, effect packets, generated asset policy, `MASTER_COMBAT`, and infrastructure inventory.
- `CombatVFXDefinitionOfDone.md` already has gates for hitbox/damage authority, visual/damage alignment, production binding, item/stat proof, and idol overlay, but not a separate hard gate for weapon context publication and idol context consumption parity.
- `CombatVFXVisualDamageAlignmentContract.md` defines `Impact point` as the contact or trigger point carried by the impact context and has the Hero 1 AOE / Water worked reference.
- `CombatVFXIdolOverlayArchitecture.md` already says idol-owned damage uses a combat impact context, preserves `ParentSourceID`, and that the first Water proof validates the seam but not a real idol Niagara asset.
- `EffectPacketTemplate.md` already has a visual/damage alignment block and needs a separate impact-context contract block.
- Current runtime proof emits weapon context, idol context, Water-specific diagnostic parity counters, `DamageBySource SourceID=Idol_Water`, and a neutral Earth run that forbids Water diagnostics.
- This pass is docs/process only. No runtime code edits, asset edits, Niagara authoring, or validation-tool code changes are in scope.

## Proposed Patch Direction To Validate

Add:

- `Gameplay/Combat/CombatVFXImpactContextContract.md`

Update:

- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/EffectPacketTemplate.md`
- `Gameplay/Combat/pending_issues_Combat.md`

The new reusable contract should generalize the proof away from Water-only names and require weapon context publication, idol context consumption, `SourceType` / `SourceID` / `ParentSourceID` identity, own idol damage source proof, own idol impact context, context parity counters, skip/fallback counters, neutral-control proof, and explicit wording that video proof is not enough without runtime context/damage logs.

## Required Output

Write a complete Operator Packet with these exact sections:

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

Keep the packet concise. Do not include file diffs.
