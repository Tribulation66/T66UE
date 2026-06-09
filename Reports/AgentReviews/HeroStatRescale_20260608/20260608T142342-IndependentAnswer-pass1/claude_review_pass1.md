Result: NEEDS_USER

## Independent Answer

This is a tuning/design rescale whose core inputs only you can set, so the Operator should stop and confirm before writing code. The system is well-understood from the repo, but three of the required numbers cannot be safely inferred.

What the live system actually does today (so we're grounded):
- Authored bases in `Heroes.csv` are already whole weight-style values roughly 1–10 (e.g. Hero_1 = Dmg3/AtkSpd3/Scale2/Acc2/Armor3/Eva2/Luck3/Speed2). So "base 1–10 by weight" is mostly a re-weighting of existing data, not a new schema.
- Per-level gains are currently **decimal ranges** (`Lvl*Min`/`Lvl*Max`, e.g. `LvlSpeedMin=0.2`, `LvlSpeedMax=0.4`) rolled at each level-up in `ApplyOneHeroLevelUp` (`T66RunStateSubsystem_Stats.cpp:512`). Moving to a **fixed per-level gain** means collapsing each Min/Max pair to one number and the roll path can stay (Min==Max returns Max at line 300).
- Internal precision is already a fixed-point "tenths" scale: `HeroStatTenthsScale = 10`, display = whole number, hard clamps `MaxHeroStatValue = 99` and `MaxHeroLevel = 99` (`T66RunStateSubsystem.h:151-153`).
- Gameplay impact is applied by *small per-point coefficients on the displayed whole value*, not by the internal scale:
  - Speed → walk speed = `Speed * 840 UU/s` (`T66HeroMovementComponent.cpp:21,106`)
  - Damage `1 + (Damage-1)*0.015`, AtkSpd `*0.012`, Scale `*0.008`, Accuracy `*0.010` (MASTER_STATS §5.2)
  - Armor `(Armor-1)*0.008`, Evasion `(Evasion-1)*0.006` (`T66RunStateSubsystem_Stats.cpp:903-904`)

The key realization the Operator must not miss: **multiplying the stat by 100 and dividing the coefficient by 100 nets to zero gameplay change.** Your "2 → 200" is a *presentation/scale* change; the actual fix you're describing ("effect too much, I get too fast / too much evasion") is **lowering the per-point effect coefficients**. Those are two independent edits, and the second is pure tuning that only you can specify.

Recommended split for the Operator once you decide:
1. Data: collapse `Lvl*Min`/`Lvl*Max` to single fixed gains (1–5 by weight) per hero; optionally re-weight bases (1–10).
2. Scale: if you literally want 200-style numbers on screen, that is a *display* multiplier, not a change to `HeroStatTenthsScale` — and it collides with the `99` clamp and the `/99`/`/100` UI denominators (`T66StatsPanelSlate.cpp:217,363`, `T66CasinoVendorTabWidget.cpp:234`). Raising the cap touches save/backend/UI.
3. Effect: re-derive each coefficient against the new scale so the *net* per-level gameplay delta is smaller than today.

## Evidence Checked
- `Gameplay/Stats/MASTER_STATS.md` (formulas §5.2–5.3, scale, leveling §6)
- `Content/Data/Heroes.csv` (all 12 heroes: bases already ~1–10; per-level are decimal ranges)
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp` (level-up roll, gain application, secondary propagation, `GetSecondaryStatValue`, armor/evasion `(stat-1)*k`)
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp` (`Speed * 840 UU/s`)
- `Source/T66/Core/T66RunStateSubsystem.h` (`HeroStatTenthsScale=10`, `MaxHeroStatValue=99`, clamps)
- UI denominator references (`/99`, `/100`) in stats panel and casino widgets

## Questions Or Blockers
These are the user-only decisions the stop condition points at:
1. **Per-hero weights:** give the fixed per-level gain (1–5) per stat per hero, and confirm whether bases should be re-weighted or left as-is. (Current ranges hint at intent but the fixed targets are a design call.)
2. **Effect magnitude:** "greatly diminished" by how much? e.g. target top walking speed, and roughly how much speed/evasion/armor a max-level run should grant. Without a target the coefficients can't be re-derived safely.
3. **Display intent:** do you literally want the panel to read 200 (requires raising the `99` cap and changing `/99`·`/100` UI + save/backend), or do you just want gains that *feel* bigger while the panel keeps 1–99? This decides whether this is a small data/coefficient change or a cross-cutting scale migration.

## Caveats
- A literal ×100 internal rescale is high blast-radius: item line-1 rolls (Black 1..3 … White 20..30), diploma `+1` points, single-use multipliers, secondary-stat propagation factors, save snapshots (schema 15), and backend serialization all sit on the current 1–99/tenths economy and would need migrating together; otherwise saved runs and item/buff power break. Prefer the data+coefficient route unless you explicitly want the bigger on-screen numbers.
- Gameplay-affecting; per repo rules it needs compile + staged standalone validation after implementation.
- Codex can proceed to implement immediately once items 1–3 are answered — none of this requires escalation beyond those design inputs.
