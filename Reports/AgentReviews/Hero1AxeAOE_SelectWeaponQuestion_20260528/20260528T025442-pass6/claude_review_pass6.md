Verdict: APPROVE

## Blockers
None.

## Major Issues
None. The answer is correctly hedged: it claims code-path + validator confirmation only, not manual playthrough confirmation, and it flags the untracked-asset risk that would otherwise let a reader assume a clean checkout reproduces the VFX.

## Minor Issues
- The packet repeats the "untracked / not playthrough-confirmed" caveat across three sections (Proposed Answer top, Important caveats, Evidence). Consolidating would tighten it, but the redundancy is defensible since each occurrence is load-bearing for a different claim.
- "If difficulty tuning resolves the altar to black, the AOE slot is `Hero_1_black_aoe`" is the right hedge, but the answer never actually states what the current difficulty-tuning row resolves to. A reader could still walk away assuming black is guaranteed. The caveat list mentions this once; consider stating it more directly in the lead paragraph.
- `MakeWeaponID` formatting claim (`Hero_1` + `black` + `aoe` → `Hero_1_black_aoe`) silently assumes `HeroID.ToString()` already contains the underscore (`Hero_1`, not `Hero1`). That matches the CSV row, so it is internally consistent, but the answer does not explicitly confirm the HeroID literal. Worth a one-line note if challenged.

## Clarifying Questions
- Is the intent to surface this as a "yes, but verify with one altar-pick capture" answer to the user, or as a definitive yes? The packet reads as the former, which is appropriate; confirm that framing is what gets presented.
- Should the answer also note what happens at non-black rarities (no binding row → silent fallback / no VFX / different VFX)? The packet says "will not fire unless that other rarity gets its own binding row" but does not describe the user-visible behavior in that case.

## Required Verification
Before this is treated as fully playthrough-confirmed (per AGENTS.md verification discipline and the packet's own caveat), one manual run is still owed:
- Launch editor, enter a run where the altar resolves to black rarity, pick `Ashfall Cleaver`, fire an AOE auto-attack on an enemy, and confirm `NS_Hero1AxeAOE_MeshSlash` spawns. Capture to `Saved/VideoCaptures/` alongside the existing EdgeFinal proof.
- Separately, before any commit that depends on this path, stage `Content/Data/DT_CombatVFXBindings.uasset`, `Content/Data/CombatVFXBindings.csv`, and `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset` — currently untracked — or the next clean checkout breaks the route.

The packet itself flags both items, which is why this is APPROVE rather than REVISE.

## Rationale
The proposed answer matches the live repo evidence cited: CSV rows, Niagara asset on disk, validator success, and the C++ call chain from `OnChooseSlot` → `SelectWeapon` → `RefreshCachedStats` → `PerformSlash` → `TrySpawnBoundWeaponBaseSlashVFX`. The scope of the claim ("wired in this working tree, not manual-playthrough-proved") is honest and consistent with the EdgeFinal harness proof being binding-validator output rather than altar-click capture. The three limits (rarity gate, working-tree-only, harness vs manual) are the correct caveats and are stated up front rather than buried. Nothing in the packet contradicts AGENTS.md verification posture — it explicitly defers the manual capture rather than claiming it. Safe for Codex to present at the user go-ahead gate.

