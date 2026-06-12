# Operator Packet — Combat VFX Impact-Context Process Docs

## 1. Working Goal And Tier
**Tier 1 docs/process update.** Codify a reusable, weapon-and-idol-agnostic impact-context and idol-proof schema across the Combat VFX process docs, replacing the Water-only worked example with a generalized contract that all future weapon and idol VFX work must satisfy. No runtime, asset, or Niagara changes.

## 2. Roles And Tool Profile
- **Operator:** Claude (read-only authoring of this packet; no file writes, no shell, no Unreal Python, no editor automation).
- **Validator/Integrator:** Codex (performs live reads, applies patches, runs validation).
- Confirmed by `.t66/operator-state.json` (`operator=Claude`, `validator=Codex`).

## 3. User Constraints And Out Of Scope
- Docs/process only. **Out of scope:** runtime code, asset edits, Niagara authoring, validation-tool code changes.
- No file diffs in this packet.
- Operator emits proposal only; Codex integrates.

## 4. Applicable Instructions Read
- `AGENTS.md` → routes Tier 1 Claude/Codex work through `OPERATOR_VALIDATOR_PROTOCOL.md`.
- `Gameplay/GAMEPLAY_AGENTS.md` → owns gameplay/combat docs; authoring reads `CombatVFXAuthoringProcedure.md`.
- `Reports/AGENTS.md` → agent review packets land under `Reports/AgentReviews`.
- `VFX_PROCESS_INDEX.md` → quick-start router defining canonical read order.

## 5. Evidence And Live Findings
- `VFX_PROCESS_INDEX.md` read order omits any impact-context contract entry.
- `CombatVFXDefinitionOfDone.md` has gates for hitbox/damage authority, visual/damage alignment, production binding, item/stat proof, and idol overlay — **but no hard gate for weapon-context publication ↔ idol-context consumption parity.**
- `CombatVFXVisualDamageAlignmentContract.md` defines `Impact point` as the contact/trigger point carried by the impact context; holds the Hero 1 AOE / Water worked reference.
- `CombatVFXIdolOverlayArchitecture.md` states idol-owned damage uses a combat impact context, preserves `ParentSourceID`; Water proof validates the seam but **not a real idol Niagara asset.**
- `EffectPacketTemplate.md` has a visual/damage alignment block but **no separate impact-context contract block.**
- Runtime proof currently emits weapon context, idol context, Water-specific parity counters, `DamageBySource SourceID=Idol_Water`, plus a neutral Earth run that forbids Water diagnostics.
- `pending_issues_Combat.md` carries related open items: Water placeholder area footprint and alignment-validator enforcement.

## 6. PPF And Process Gates
New/changed gates the contract introduces:
- **G1 — Weapon context publication:** every weapon VFX must publish an impact context with `SourceType` / `SourceID`.
- **G2 — Idol context consumption parity:** idol-owned damage must consume an impact context preserving `ParentSourceID`, with publish↔consume counts matching.
- **G3 — Own idol damage-source proof:** `DamageBySource` must show the idol's own `SourceID` (not the parent weapon's).
- **G4 — Own idol impact context:** idol must carry its own impact context, not merely inherit the weapon's.
- **G5 — Parity + skip/fallback counters:** context parity counters and skip/fallback counters must be emitted and reconciled.
- **G6 — Neutral-control proof:** a neutral/non-themed run must forbid the themed diagnostics (generalized from the Earth-forbids-Water pattern).
- **G7 — Log-not-video gate:** explicit wording that video proof is insufficient without runtime context/damage logs.

## 7. Proposed Patch Approach
**Add** `Gameplay/Combat/CombatVFXImpactContextContract.md` — canonical, theme-agnostic contract defining: impact-context identity fields (`SourceType` / `SourceID` / `ParentSourceID`), weapon publication requirement, idol consumption requirement, own-idol damage source + own-idol impact context, parity counters, skip/fallback counters, neutral-control requirement, and the log-not-video rule. Use Water/Earth strictly as a worked example, not as named requirements.

**Update:**
- `VFX_PROCESS_INDEX.md` — insert the new contract into the read order (logically after the visual/damage alignment contract, before effect packets).
- `CombatVFXAuthoringProcedure.md` — add a step pointing authors at the new contract during weapon and idol authoring.
- `CombatVFXDefinitionOfDone.md` — add the hard parity gate (G1–G7) referencing the new contract.
- `CombatVFXVisualDamageAlignmentContract.md` — cross-link impact-context identity to the existing `Impact point` definition; mark Water as illustrative.
- `CombatVFXIdolOverlayArchitecture.md` — reference the contract for `ParentSourceID` preservation and own-idol proof; note Water seam is example-only pending a real idol Niagara asset.
- `EffectPacketTemplate.md` — add a separate impact-context contract block alongside the alignment block.
- `pending_issues_Combat.md` — note generalization supersedes Water-only framing where relevant; leave footprint/validator items intact.

## 8. Verification Plan
- **Validator (Codex) reads** each touched doc to confirm cross-links resolve and the new contract is in the index read order.
- **Consistency check:** no remaining Water-only *requirement* language; Water/Earth appear only as worked examples.
- **Gate completeness:** DoD enumerates G1–G7 and points to the contract; EffectPacketTemplate has the new block.
- **Round-trip:** an author following INDEX → AUTHORING → CONTRACT → DoD can satisfy a hypothetical non-Water weapon+idol without missing a gate.
- Since docs-only, no build/runtime/asset verification; explicitly confirm none was performed.

## 9. Token Routing
- Operator (Claude): this packet only.
- Validator/Integrator (Codex): live reads of the 8 paths, patch application, cross-link verification, commit.
- Review packet (if produced) routes to `Reports/AgentReviews` per `Reports/AGENTS.md`.

## 10. Operator Position And Open Decisions
**Position:** Proceed with the proposed patch direction; it is correctly scoped and addresses the missing parity gate.
**Open decisions for user/Validator:**
- (a) Exact insertion slot for the new contract in the INDEX read order — recommend after the alignment contract.
- (b) Whether `pending_issues_Combat.md` should *close/supersede* the Water footprint framing or merely cross-reference it — recommend cross-reference only, to avoid touching unrelated open items.
- (c) Whether a `Reports/AgentReviews` packet is required for a docs-only Tier 1 pass or the operator-state handoff suffices.

## 11. Anti-Lookalike Discriminator
This contract is **not** a rename of the existing Water proof and **not** a duplicate of the Visual/Damage Alignment Contract. Distinguishing markers:
- Alignment contract governs *where* the impact lands (geometry/`Impact point`); this contract governs *who* owns it (`SourceType`/`SourceID`/`ParentSourceID` identity + publish/consume parity).
- It mandates **own-idol** damage source and impact context — a real Niagara idol asset must eventually replace the Water seam; a passing Water run alone does **not** satisfy it.
- It elevates **log-based** proof over video. Any future doc that accepts video-only proof, or that names "Water" as a requirement rather than an example, is the lookalike to reject.

---
This is an operator artifact for Codex to integrate, not a merge greenlight. No tools were called; findings rely on the supplied live context.
