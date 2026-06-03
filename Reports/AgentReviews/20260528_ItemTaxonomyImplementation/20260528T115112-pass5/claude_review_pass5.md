Verdict: REVISE

## Blockers

None.

## Major Issues

- **Miniboss class hierarchy is asserted but not proven.** The plan's central OHKO rule depends on the claim that minibosses derive from `AT66EnemyBase`, not `AT66BossBase`. If any miniboss type actually derives from `AT66BossBase` (or a sibling/intermediate boss class), the helper will silently refuse to kill them and violate the user's "OHKO can affect minibosses" requirement. Codex must inspect the actual miniboss class declarations and the `AT66BossBase` hierarchy and quote the inheritance before implementing — not after.
- **Verification of the OHKO rule is optional ("only if existing hooks cannot prove").** This is the highest-risk runtime behavior change in the pass (changing what kills bosses vs. minibosses on three combat paths). The plan must commit upfront to a concrete proof — either name the existing hook/log path that will demonstrate boss-rejection + miniboss-acceptance, or commit to the narrow smoke path. "Optional" is too loose for a behavior contract that the user clarified explicitly.
- **Reward-scaling specifics for Loot Wheel boost reward are under-specified.** "Rounded/clamped to sensible positive values" hides a balance decision. Codex should name the rounding rule (e.g., `FMath::RoundToInt`), the minimum clamp (≥1 point, ≥1s?), and confirm that scaling both `BoostBonusStatPoints` and `BoostDurationSeconds` is intended versus picking one axis — otherwise a 3.0 multiplier compounds to 9× total value, which may exceed parity with Loot Crate/Chest gold scaling.

## Minor Issues

- **Discrete tier breakpoint feel.** `floor(Multiplier - 1.0)` produces a cliff at integer boundaries (1.99 → +0, 2.00 → +1). This is fine as a "deterministic single-reward analogue" but worth a one-line note that intermediate multipliers (1.0–1.99, 2.0–2.99) feel identical until the next breakpoint. Confirm this is acceptable rather than a probabilistic upgrade.
- **`Item_GamblersToken` row removal from CSV is implied, not stated.** Plan step 1 lists the new rows but does not explicitly say "remove the `Item_GamblersToken` row from `Items.csv`." Step 8 verification asserts the CSV does not contain it, so this should be made explicit in step 1 to avoid a missed deletion.
- **`Item_TreasureChest` row ID retained while display text becomes "Loot Chest" creates a discoverability tax** (similar to `EvasionChance` ↔ Dodge). The plan acknowledges this for compatibility — fine — but the audit list should include a search for any new code added in this pass that hard-codes "Loot Chest" as a row-ID or asset-path candidate.
- **`T66IsAccuracyFamilySecondaryStatType` change risk.** Removing old `Accuracy` from the family helper while keeping `GetAccuracyChance01()` reading the raw `Accuracy` value is fine, but the plan should explicitly grep for every call site of `T66IsAccuracyFamilySecondaryStatType` and confirm none of them gate hero-base-accuracy UI/serialization. The plan promises this audit but does not name the call-site list.
- **Enum tail-loop audit names `T66LeaderboardSubsystem` only.** The plan should commit to a repo-wide grep for `ET66SecondaryStatType::Accuracy` and any `static_cast<int>(ET66SecondaryStatType::...)` upper-bound usage, not just one named site.

## Clarifying Questions

- Should the Loot Wheel Boost reward scaling apply to both points and duration, or only one? (Affects whether 3.0× → 9× perceived power.)
- Is the discrete tier breakpoint behavior for Loot Bag / Loot Wheel acceptable, or does the user expect a probabilistic upgrade closer to Loot Crate's weighted bias?
- Confirm: should `Item_GamblersToken` be removed from `Content/Data/Items.csv` entirely, with the legacy ID surviving only via `NormalizeLegacyItemID()` for save/loot-bag compatibility?

## Required Verification

- Inspect `AT66MinibossBase` (or whatever the concrete miniboss class is named) and quote its parent class to prove the OHKO helper will reach it.
- Compile the editor target with the named Build.bat command and surface every `C4061`/`C4062`/`LogDataTable` warning for review, not just summarize.
- Run a deliberate in-engine or smoke test that exercises: (1) boss rejects Execute/Assassinate/Crush, (2) miniboss accepts all three, (3) Counter/Reflect still damages bosses non-lethally, (4) `Item_GamblersToken` save/drop resolves to `Item_VendorToken` with no missing-row warning, (5) high Loot Bag/Loot Wheel multipliers visibly change the locked reward.
- Narrow `git diff` against the pre-existing dirty-file list to confirm no unrelated work was reverted.

## Rationale

The plan is well-scoped, identifies the right seams (RunState accessors, CombatShared helper, NormalizeLegacyItemID, live-filter helpers, defensive switch surfaces), correctly preserves backend keys (`EvasionChance`, `TreasureChest`) for save compatibility, appropriately defers Mini-owned cleanup and shared sprite deletion, and correctly excludes idol `TryExecuteTarget` from the item-stat OHKO path. Keeping `Item_TreasureChest` as the compat key for Loot Chest is acceptable and consistent with the `EvasionChance`/Dodge precedent. The Loot Bag/Loot Wheel interactable-only scope now matches Loot Crate/Loot Chest parity at the consumption seam. The reasons for REVISE rather than APPROVE are: the miniboss-class assertion is load-bearing and unproven; verification of the OHKO behavior is too optional for a rule the user clarified explicitly; and the Boost-scaling math hides a balance decision. These are all fixable within Codex's scope — research the hierarchy, commit to a verification path, and tighten the boost formula — and do not require user direction.

