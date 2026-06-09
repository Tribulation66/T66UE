I have enough evidence for an independent answer.

Result: OK

## Independent Answer

The fix is a one-line constant change in `Source\T66\Gameplay\Movement\T66HeroMovementComponent.cpp:21`:

```cpp
constexpr float T66HeroWalkSpeedUnitsPerSpeedPoint = 100.f;  // → 300.f
```

The live conversion is purely linear: `T66ResolveWalkSpeedFromSpeedStat()` (line 105-108) returns `FMath::Max(1, SpeedStat) * T66HeroWalkSpeedUnitsPerSpeedPoint`. Today the constant is `100.f`, so displayed Speed 2 → 200 uu/s (the value the user just tested as too slow). To make displayed Speed 2 resolve to 600 uu/s, set the constant to **300.f** (600 ÷ 2 = 300).

Notes on correctness:
- This preserves the 1–99 displayed stat model — `GetSpeedStat()` (`T66RunStateSubsystem_Stats.cpp:776`) is unchanged; only the per-point walk-speed gain scales.
- It leaves the prior fixed-gain/coefficient work alone — the secondary `MovementSpeed` multiplier path and `T66PrimaryAttackSpeedMultiplierPerPoint` are untouched.
- The 300.f gain stays clear of the `T66HeroMinimumWalkSpeed = 100.f` clamp (line 22) and the 10000.f ceiling (line 253), so no clamp interference at Speed 2 or across the displayed range until ~Speed 34 (where ×300 hits the 10000 ceiling — same ceiling behavior the model already has, just reached at a different stat value).

Per repo rules, after the edit Codex should attempt a focused compile/build and a staged standalone playable check (movement is a playable-standalone-affecting runtime change), or explicitly caveat if that verification can't be completed locally.

## Evidence Checked
- `T66HeroMovementComponent.cpp:21-22` (the two constants), `:105-108` (conversion), `:246-253` (refresh + clamp), `:278-280` (base-stat path)
- `T66RunStateSubsystem_Stats.cpp:776-778` (displayed `GetSpeedStat`)
- Prior session reviews under `Reports\AgentReviews\HeroStatRescale_20260608\` confirming the 100.f value and Hero_1 base Speed = 2
- `original_prompt.md` and `.t66/operator-state.json` (Codex=Operator, Claude=Validator)

## Questions Or Blockers
None requiring the user. The arithmetic (300.f) follows directly from the user's stated target "2=600."

## Caveats
- Confirm the user means the **displayed** Speed stat of 2 (Hero_1's base). The refresh path uses `GetSpeedStat()` (displayed), so 300.f is correct for that reading. If they instead meant a precise/internal value, recompute — but the wording "I just tested" points to the displayed in-game stat.
- No `.md`/data docs reference this constant (grep returned none under `Source`), so the "docs updated" part of the stop condition may be a no-op; verify there's no tuning note elsewhere before claiming docs were touched.
- The value is a hardcoded `constexpr`, not a data/tuning hook, so this is an in-place source edit requiring a recompile — not a hot-reloadable data tweak.
