Verdict: REVISE

## Blockers

None.

## Major Issues

- **Lede buries the real answer.** The user asked "does selecting the Hero 1 AOE weapon at the altar currently apply the production AOE VFX?" The honest answer is conditional on two unverified facts: (a) which rarity the altar actually resolves to via difficulty tuning, and (b) whether a fresh altar-pick → AOE-fire sequence has been observed. Both are deferred. Restructure so the user can see in the first sentence: "Code path supports it for the black-rarity offer only; not confirmed manually this pass; current working tree depends on untracked assets."
- **Altar-rarity conditional is load-bearing but soft.** The packet says "If difficulty tuning resolves the altar to black, the AOE slot is `Hero_1_black_aoe`." That "if" is the entire question. The packet should either (a) state what difficulty tuning currently resolves to (verifiable from the relevant tuning data without running the game) or (b) state explicitly that the answer reduces to "yes for black, unknown for other rarities, and we have not checked what rarity is being offered right now."
- **Untracked-asset risk understated as caveat, not as a blocker to "currently works."** `DT_CombatVFXBindings.uasset`, `CombatVFXBindings.csv`, `NS_Hero1AxeAOE_MeshSlash.uasset`, and the setup/validator scripts being untracked means the answer "it works" is true only in this exact working tree on this machine. Any answer phrased as a present-tense fact ("it works") should be paired with an explicit action item (stage/commit those assets) or downgraded to "it works in my current working tree, not in HEAD."
- **Validator success is not equivalent to runtime parity.** The packet says the validator "implies `DT_CombatVFXBindings.uasset` is in sync with the CSV." Implication is not verification of in-editor reload. A CSV edit without rerunning the setup script is mentioned, but the answer should state whether the setup script was rerun after the most recent CSV touch, or acknowledge that this is also unverified.

## Minor Issues

- "The VFX does not play at the moment of selecting the weapon card; it plays when the attack fires" is correct and worth keeping, but could be lifted earlier — it preempts a common misread of "applies the production AOE VFX."
- The EdgeFinal proof reference (`Saved/VideoCaptures/.../EdgeFinal_20260528_014810/`) is useful audit context but should be labeled clearly as "binding-proof harness, not altar-pick" up front rather than at the end, so the reader doesn't conflate it with altar verification.
- The phrase "code-path and validator verified" is doing a lot of work. Spell out the gap: validator confirms binding row + asset presence + GI DataTable pointer; it does not confirm the altar offer build, the rarity selection, the equip path, or the actual Niagara spawn.

## Clarifying Questions

- Does the user want this answer escalated to a fresh altar-pick capture before they act on it, or is the code-path + validator answer sufficient for their current decision?
- Should the untracked assets be staged/committed as part of closing this question, so the answer survives a clean checkout?
- Is the altar's current rarity resolution (under whatever difficulty tuning is active) already known, or is that a separate read we owe the user?

## Required Verification

- A manual altar-pick → AOE auto-attack sequence with `CombatVFXProductionSpawned` observed in `T66.log` tied to `Hero_1_black_aoe` (not the proof harness path). This is the only thing that closes the "currently applies" question end-to-end.
- Confirm whether the altar's `WeaponOfferRarity` resolves to `Black` under the active difficulty tuning at the test moment; if not, the binding will not fire and the answer flips.
- Confirm the setup script (`SetupCombatVFXProductionBindings.py` or equivalent) was rerun after the most recent `CombatVFXBindings.csv` edit so the `.uasset` reflects current CSV state.

## Rationale

The packet is honest, well-evidenced, and traces the code path correctly, so it isn't a BLOCK. But as written it leads with "should work" and trails the disqualifying caveats — altar-rarity dependency, no manual verification, untracked production assets — instead of letting them shape the headline. AGENTS.md framing favors explicit known/unknown separation before any "it works" claim, and the current draft inverts that. A revise pass that (1) leads with the conditional, (2) hardens the untracked-asset risk into an action item, and (3) clarifies what the validator does and does not prove will make this safe to present at the user go-ahead gate.

