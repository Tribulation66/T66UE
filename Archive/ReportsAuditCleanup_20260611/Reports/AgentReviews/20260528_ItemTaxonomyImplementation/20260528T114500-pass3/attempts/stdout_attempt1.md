Verdict: REVISE

## Blockers

None — no hard prerequisites or external-state issues prevent progress.

## Major Issues

- **Enum "deprecation" strategy is undefined.** Step 2 says append new values and "Mark old live-facing `Accuracy`, `Cheating`, `Stealing`, and `GamblerToken` as deprecated/inactive" while keeping them serializable. The plan does not specify the mechanism: a UMETA hidden flag, a `T66IsLiveSecondaryStatType()` filter (mentioned later but not defined here), a comment, or an enum attribute. Without a concrete mechanism, switch exhaustiveness, UI iteration, and `T66_GetAccuracySecondaryTypes()` membership are ambiguous. Codex needs to name the actual filter function and show how every iterator/switch consumer will branch.
- **Family placement of Counter / Reflect / Taunt is asserted, not verified.** The new order lists Counter under Evasion and Reflect/Taunt under Armor. Current Evidence does not confirm where they sit today in `T66_GetEvasionSecondaryTypes()` / `T66_GetArmorSecondaryTypes()` / `T66IsAccuracyFamilySecondaryStatType`. If any of these currently lives in a different family, moving it changes per-family multipliers (Evasion multiplier vs Armor multiplier) for existing rolls and saves silently. Codex must read each helper before committing to the moves.
- **Vendor Token alias resolution site is unnamed.** Step 6 says `Item_VendorToken` and `Item_GamblersToken` should both be accepted as input aliases "where saves/logs could still hold the old ID," but does not name the resolver. Different sites (DataTable lookup, save migration, structured event ingest, HUD card lookup, achievement check) each handle missing rows differently. Without an enumerated alias surface, legacy saves silently get a missing-row warning or a fallback synthetic row.
- **Loot Chest UI rename coverage is partial.** Step 1 keeps `Item_TreasureChest` row ID and `TreasureChest` enum key; step 7 names `T66LocalizationSubsystem`, `T66StatsPanelSlate`, `T66PowerUpScreen`, `T66TemporaryBuffUIUtils`, and hero-selection helper text for "final names/order." It does not commit to auditing every "Treasure Chest" user-facing string (item-card text, achievement strings, HUD special-cases, structured-event display names). The verification step does not text-grep for surviving "Treasure Chest" strings either.
- **`AT66LeaderboardSubsystem` and other "Accuracy-last" loops are flagged but not committed to.** The Known Risks section names the leaderboard subsystem as having an "Accuracy is last secondary" assumption. The implementation plan does not include a step that audits all such loops and lists them. This is exactly the kind of seam that breaks silently once enum values are appended.

## Minor Issues

- `BaseBuyGold=100`, `BaseSellGold=0` for `Item_VendorToken` is stated as a final value with no reference to whether `Item_GamblersToken` used the same numbers. If the existing row had different economy values, this is a silent rebalance.
- Verification step says "Add a narrow, non-shipping automation smoke path only if existing hooks cannot prove the changed runtime seams" without naming the candidate existing hooks. The decision criterion is left implicit; Codex may end up adding test scaffolding it does not need, or skipping coverage it does need.
- "Inspect compile output for warnings/errors related to enum switches or item IDs, not just the exit code" is good intent but no concrete grep is named. A `Switch on enum '...'` warning class or `LogDataTable` warning filter would make this auditable.
- `EvasionChance` stays as the serialized name while UI shows "Dodge Chance" — fine but the plan does not say whether structured events / leaderboard payloads should keep `EvasionChance` or switch to `DodgeChance`. Downstream telemetry consumers will need a decision.
- The "central OHKO helper" semantics for `AT66MobBase` are stated as "apply lethal damage through `TakeDamageFromHeroHitZone()` when alive" — for Execute on crit, the hit zone may not always be available at the call site. The helper's signature and required inputs are not sketched.
- Smoke proof for boss rejection covers OHKO from item paths but does not specify that non-OHKO reflected/counter boss damage still applies (i.e., that the helper's `return false` for bosses does not regress reflect/counter boss damage on the same combat path).

## Clarifying Questions

- Should the deprecated `Accuracy`/`Cheating`/`Stealing`/`GamblerToken` enum values be hidden from the editor (UMETA Hidden), or just filtered from runtime live lists? Both are valid; pick one and apply consistently.
- For Loot Bag / Loot Wheel multipliers, should the bias upgrade the rolled item rarity tier directly, or shift the rarity weights and re-roll? The packet says "biasing/upgrading the picked item rarity" — these have different determinism characteristics.
- For Loot Chest, should `GetLootChestRewardMultiplier()` scale the gold range, the gold roll result, or both? The packet says "gold range/result" which is ambiguous.

## Required Verification

- After implementation, grep for every literal occurrence of `Accuracy`, `Gambler`, `GamblersToken`, `Cheating`, `Stealing`, `Treasure Chest`, `HpRegen`, `LifeSteal` and classify each as legacy-internal vs player-facing leak.
- Compile editor target with the listed Build.bat command and inspect for `C4061`/`C4062` (switch on enum) warnings, `LogDataTable` row warnings, and any `Item_GamblersToken`/`Item_Accuracy` lookup logs at editor startup.
- Run the full path for at least one boss, one miniboss, one regular enemy, and one mob through Execute, Assassinate, and Crush — confirm boss survives and the other three die.
- Lock and consume one Loot Bag and one Loot Wheel reward at a known seed with multiplier `1.0` and again at `>1.0`; confirm reward differs.
- Load a save that contains `Item_GamblersToken` and confirm it resolves to Vendor Token in the HUD/inventory without a missing-row warning.
- Run `git diff --stat` against the pre-existing dirty-file list and confirm no unrelated reverts.
- Refresh staged standalone via `Scripts/StageStandaloneBuild.ps1` if not blocked; if blocked, report the blocker explicitly rather than claiming staged proof.

## Rationale

The plan demonstrates good repo reading (`T66CombatShared` selected correctly, idol Execute correctly excluded, scope bounded to interactable reward paths per user clarification, legacy persistent names retained for save safety, enum values appended rather than reordered, `TreasureChest` data key preserved). The user-clarified semantics for non-boss OHKO and Loot Bag / Loot Wheel parity are honored.

The plan is not safe to APPROVE because four implementation seams remain underspecified and would lead to silent regressions if Codex proceeded as written: the deprecated-enum filter mechanism, the family placement of Counter/Reflect/Taunt, the Vendor Token alias resolver surface, and the coverage of "Treasure Chest" user-facing strings. None of these require user direction — they are Codex-resolvable by reading existing helpers and tightening the plan. Hence REVISE rather than NEEDS_HUMAN_DECISION or BLOCK.

