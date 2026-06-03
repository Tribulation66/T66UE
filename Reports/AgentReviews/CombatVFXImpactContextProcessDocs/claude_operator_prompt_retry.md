# Claude Operator Prompt Retry: Combat VFX Impact-Context Process Docs

You are Claude acting as read-only Operator for `C:\UE\T66`.

Produce a complete Operator Packet for this Tier 1 documentation update:

With Claude as Operator and Codex as Validator/Integrator, update the Combat VFX process docs with a reusable impact-context and idol proof schema for all future weapon and idol VFX work.

## Context Already Established By Codex Live Reads

- `.t66/operator-state.json` currently sets `operator=Claude`, `validator=Codex`.
- `AGENTS.md` routes Tier 1 Claude/Codex work through `OPERATOR_VALIDATOR_PROTOCOL.md`.
- `Gameplay/GAMEPLAY_AGENTS.md` owns gameplay/combat docs and says combat VFX authoring reads `Gameplay/Combat/CombatVFXAuthoringProcedure.md`.
- `VFX_PROCESS_INDEX.md` is the quick-start router. Its read order currently includes authoring procedure, definition of done, visual/damage alignment contract, effect packets, generated asset policy, `MASTER_COMBAT`, and infrastructure inventory.
- `CombatVFXDefinitionOfDone.md` already has gates for hitbox/damage authority, visual/damage alignment, production binding, item/stat proof, and idol overlay, but not a separate hard gate for weapon context publication and idol context consumption parity.
- `CombatVFXVisualDamageAlignmentContract.md` defines `Impact point` as the contact or trigger point carried by the impact context and has the Hero 1 AOE / Water worked reference.
- `CombatVFXIdolOverlayArchitecture.md` already says idol-owned damage uses a combat impact context, preserves `ParentSourceID`, and that the first Water proof validates the seam but not a real idol Niagara asset.
- `EffectPacketTemplate.md` already has a visual/damage alignment block. It needs a separate impact-context contract block.
- `pending_issues_Combat.md` already contains related pending items about Water placeholder area footprint and alignment validator enforcement.
- Current runtime proof emits:
  - `CombatImpactContext Phase=WeaponPrimary SourceType=WeaponBase ...`
  - `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water ParentSourceID=Hero_1_black_aoe ...`
  - `CombatIdolImpactDiagnostic SourceID=Idol_Water WaterIdolContextParity=PASS WeaponImpactContexts=1 EligibleWeaponImpactContexts=1 ImpactPresentationIdolSlots=1 ExpectedWaterIdolImpactContexts=1 WaterIdolImpactContexts=1 WaterSkippedNoWeaponContext=0 WaterSkippedInvalidImpactPoint=0 WaterLegacyFallbacks=0 ...`
  - `DamageBySource SourceID=Idol_Water`
- The current proof wrapper requires the Water diagnostic and forbids it in the Earth-neutral case.

## Proposed Patch Direction To Validate

Add a focused contract doc:

- `Gameplay/Combat/CombatVFXImpactContextContract.md`

Update these existing docs:

- `Gameplay/Combat/VFX_PROCESS_INDEX.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXIdolOverlayArchitecture.md`
- `Gameplay/Combat/EffectPacketTemplate.md`
- `Gameplay/Combat/pending_issues_Combat.md`

The new contract should define a generalized reusable schema, not Water-specific names. It should include:

- weapon context publication requirements,
- idol context consumption requirements,
- `SourceType`, `SourceID`, and `ParentSourceID` identity requirements,
- damage source proof requirements,
- context parity counters,
- skip/fallback counters,
- neutral-control proof,
- explicit wording that video proof is not enough without runtime context/damage logs.

No runtime code edits in this pass. Tooling validator hardening can be a pending issue.

## Required Output

Write a complete Operator Packet with these sections from `OPERATOR_VALIDATOR_PROTOCOL.md`:

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

Keep it concise but complete. The Validator will check the packet completeness gate and targeted anchors.
