# Decision Block - Hero Stat Rescale

## Current Request

Rescale hero stats so authored bases are 1-10 by character stat weight, level-up gains are fixed 1-5 per stat instead of ranges, and gameplay impact is greatly diminished. Example requested scale: `BaseSpeed=2` should become a stat value of `200` (`2 x 100`), with the same idea for other stats.

## Live Repo Facts

- `Content/Data/Heroes.csv` already authors hero base primary stats as whole 1-10-ish weights.
- `Content/Data/Heroes.csv` currently authors per-level gains as decimal min/max ranges.
- `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp` rolls those ranges on level-up.
- `Source/T66/Core/T66RunStateSubsystem.h` currently clamps primary display stat values to `MaxHeroStatValue = 99` and stores precision in tenths.
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp` currently converts Speed directly to movement as `Speed * 840 UU/s`.
- Gameplay impact is controlled by stat formulas/coefficient values, not by the displayed number alone.

## User-Only Decision

Choose the intended scale target before implementation:

### Option A - Recommended First Implementation

Keep the current 1-99 player-facing stat scale, collapse per-level gains to fixed 1-5 values by hero/stat, and reduce gameplay coefficients aggressively.

- Smaller blast radius.
- Keeps save/backend/UI/stat panel denominator assumptions intact.
- Directly fixes "too fast / too much evasion" by changing the actual effect formulas.
- Does not make the UI show `200` for `BaseSpeed=2`.

### Option B - Literal x100 Display/Stat Migration

Make authored/displayed primary stats use the requested x100 style, so `BaseSpeed=2` displays/serializes as `200`.

- Requires raising the current `99` cap.
- Requires updating UI denominator/presentation assumptions.
- Requires checking save snapshots, backend serialization, run summary, item rolls, permanent buffs, level-up propagation, and secondary formulas.
- Still also requires coefficient reductions, because x100 display alone does not reduce gameplay impact.

### Option C - Hybrid

Keep authored data as 1-10 and fixed gains as 1-5, but display them as x100 in UI only while formulas continue to use normalized authored points.

- Gives the large visible numbers the user described.
- Avoids a deep save/backend numeric migration.
- Needs clear labeling rules so UI, backend, and tuning files do not disagree.

## Needed Answer

Tell me which option to implement: A, B, or C.

Also provide either:

- `use your suggested weights`, meaning I infer fixed gains from existing hero stat weights and heavily dampen coefficients conservatively; or
- exact per-hero/per-stat weights and fixed per-level gains.

Until this is answered, implementation should not continue.
