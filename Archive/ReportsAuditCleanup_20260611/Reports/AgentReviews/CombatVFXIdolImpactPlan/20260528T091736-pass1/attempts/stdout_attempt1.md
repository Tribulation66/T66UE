Verdict: REVISE

## Blockers

None. The packet is read-only analysis with no implementation, and explicitly defers all asset/code changes to user go-ahead per AGENTS.md.

## Major Issues

- **Whiff/no-primary-target case is undefined.** The proposed impact context derives its impact point from `GetTargetAimPoint(PrimaryHandle)` (per finding #5 at `T66CombatComponent.cpp:1587-1607`). The answer to Pablo should state what the overlay does when an AOE swing hits zero actors — fall back to `SlashCenter`, `AttackOrigin`, suppress overlay entirely, or treat as a design choice. Right now Codex says "impact point from `GetTargetAimPoint`" without acknowledging this case, which is the most likely first bug.
- **Multi-impact attack semantics are deferred without flagging the consequence.** Codex says "Later Pierce/Bounce/DOT can define their own impact anchors," but the user's stated vision ("explosion at impact point") implies per-hit overlays. The answer should make explicit that for AOE the first pass is *one* overlay at the slash center (or primary target), not N overlays at N hit actors — otherwise Pablo will read "impact point" and assume the latter, then the existing `WeaponHitActors` loop (finding #8, lines 2366-2418) will silently invite a per-actor overlay implementation that contradicts the single-impact design.
- **Idol_Lava AOE/DOT mismatch needs a sharper "do not silently change data" statement.** The packet correctly identifies that `Idol_Lava` is `DOT` while the user's "fire explosion" reads as `AOE`, but the recommended next step #1 leaves it as a design choice without warning that flipping `Idol_Lava` from `DOT` to `AOE` would change live combat behavior beyond visuals. State explicitly that changing the row's category is a gameplay change, not a VFX change.

## Minor Issues

- The schema-extension question (Codex's point 4) lands on "one row is probably enough" but does not name which future fields (compatible attack categories, stacking/priority, overlay material parameter set) are deferred *and tracked*. Without a TODO/issue handoff, those fields will likely be forgotten until a second idol forces the question.
- `AttackCategory` is single-valued per row (finding #3, `T66DataTypes.h:31-83`); the answer should clarify that multi-category coverage is via multiple rows, not a list field, so Pablo does not expect schema work that already isn't needed.
- The packet lists `pending_issues_Combat.md` and `Source/T66/Gameplay/pending_issues_Gameplay.md` as read but does not quote whether either already tracks idol overlay work. If they do, the new packet should reference the entry; if they don't, the next-step list should include adding one.
- Point #6 ("suppress placeholder idol projectile only when a bound idol overlay exists") is the right rule but does not say where the suppression check lives. Naming `TryFire` (`T66CombatComponent.cpp:2263-2291`) and `SpawnWeaponProjectileVisual` (1486-1534) as the gating sites would make the answer actionable.
- The recommendation to use `SupportImpact`/explosion burst as the primary carrier for the idol effect packet is reasonable but unsourced — a pointer to where carriers are enumerated in `CombatVFXAuthoringProcedure.md` or the carrier list would let Pablo verify.
- "Idol category" vs "weapon attack category" choice in Codex point #3 is left as a design choice but not framed: state that keying overlays by weapon's `AttackCategory` lets one idol cover all categories implicitly, while keying by idol category requires per-(weapon, idol) rows. That framing is what Pablo needs to decide.

## Clarifying Questions

1. Does Pablo want the first idol overlay to fire on the slash *center* (one burst per swing) or on each hit *actor* (N bursts per swing)? The packet must answer this before any context refactor.
2. For whiff swings, should the overlay still play at `SlashCenter`/`AttackOrigin`, or suppress entirely?
3. Is `Idol_Lava` being kept as `DOT` (visual-only fire overlay, existing damage path) or migrated to `AOE` (new combat rule with separate proof)? The packet flags the decision but should ask it directly.
4. Should the impact context live on `UT66CombatComponent` as a transient member, or be returned as a struct from the attack functions for explicit ownership/lifetime?

## Required Verification

This pass is read-only analysis, so no build/capture is required. Before any implementation pass that follows from this packet, the existing DoD (`CombatVFXDefinitionOfDone.md:19`) still applies:
- Combat query/log proof must remain the authority for hitboxes/damage, not Niagara collision.
- Base-only vs base+idol capture proof must be produced through the existing capture/proof harness, separately, before the active row is accepted.
- The validator script (`Scripts/ValidateCombatVFXProductionBindings.py`) must be extended to cover the new idol row and its asset path *before* the row is enabled, not after.
- Verify the Hero 1 AOE row remains intact (`Content/Data/CombatVFXBindings.csv:2`) under the setup-script change.

## Rationale

The packet is grounded in current code with specific file:line citations and correctly distinguishes architecture-only state from production binding state. It respects the "do not resurrect placeholder path" rule (`CombatVFXIdolOverlayArchitecture.md:11-15`), flags the damage-authority/visual-overlay separation, defers implementation pending user go-ahead per AGENTS.md, and excludes Mini per user constraints. However, three load-bearing gaps — whiff-case impact point, single-vs-per-hit overlay semantics, and the gameplay implication of `Idol_Lava` `DOT→AOE` — would each create predictable confusion or rework when Pablo reads the answer. These are answerable in the same read-only pass and should be folded in before the packet is presented as the next-step recommendation. Hence REVISE, not APPROVE.

