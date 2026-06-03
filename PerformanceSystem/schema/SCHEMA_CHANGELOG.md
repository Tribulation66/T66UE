# PerformanceSystem Schema Changelog

## SchemaVersion 8

- Added `LiveLightweightRangedMobs` to `board_saturation_samples.jsonl` for Pass B.10 Ranged-family parity captures.
- Bumped event and session report schema constants to v8 so all PerformanceSystem outputs share the same schema version.

## SchemaVersion 7

- Added `LiveLightweightFlyingMobs` to `board_saturation_samples.jsonl` for Pass B.9 Flying-family parity captures.

## SchemaVersion 6

- Added lightweight family split counters to `board_saturation_samples.jsonl`: `LiveLightweightMeleeMobs` and `LiveLightweightRushMobs`. These support Pass B.8 Rush-family parity captures without hot-path Log/Display telemetry.
- Bumped event and session report schema constants to v6 so all PerformanceSystem outputs share the same schema version.

## SchemaVersion 5

- Added lightweight-mob pool counters to `board_saturation_samples.jsonl`: reuse acquires, releases, current inactive count, and peak inactive count. These support Pass B.7 pool validation without hot-path Log/Display telemetry.
- Bumped event and session report schema constants to v5 so all PerformanceSystem outputs share the same schema version.

## SchemaVersion 4

- Added `LiveRichEnemies` and `LiveLightweightMobs` to `board_saturation_samples.jsonl` so Lightweight Actor parity captures can measure rich actor and `AT66MobBase` population share.
- Hitch/event attributions now include the same split counts alongside combined `LiveRegularEnemies`.

## SchemaVersion 3

- Removed Mass placeholder counters introduced in v2. Mass migration was superseded by the Lightweight Actor approach; placeholders were dead schema.

## SchemaVersion 2

- Added Mass enemy counters to `board_saturation_samples.jsonl`: total, family split, LOD-tier split, promoted actor count, promotion/demotion rates, and projectile fire request rate.
- Added Mass processor frame-cost placeholders to `board_saturation_samples.jsonl`.
- Added `MassEnemyCounters` and `MassProcessorFrameCostsUs` objects to PerformanceSystem session summaries. These values are expected to remain zero until live Mass entity spawning and processor logic land.
