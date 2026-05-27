# T66 PerformanceSystem

`PerformanceSystem` is the permanent diagnostics and perception layer for Chadpocalypse. Its job is to detect runtime anomalies and leave useful local evidence without requiring a developer to watch the game live.

## Current Shape

- Root docs and schemas live in `PerformanceSystem/`.
- Runtime implementation lives in `Source/T66/PerformanceSystem/`.
- Runtime output is written locally to `Saved/PerformanceSystem/`.
- The first build is in-engine only. Hardware telemetry sidecars are future optional enrichment.

## Architecture

1. Diagnostics framework: versioned events, rolling frame/log/state snapshots, session reports, retention, redaction, detector self-cost.
2. Signal adapters: thin wrappers over available UE signals. Missing signals degrade to `Unavailable`.
3. Detectors: isolated anomaly checks with cadence, budget, build gate, and fail-open degradation.
4. Optional sidecar enrichment: named-pipe hardware telemetry later, with LibreHardwareMonitor as the preferred starting point.

## First Runtime Pass

The first runtime pass implements cheap, Shipping-safe diagnostics:

- Single-frame hitch detection.
- Sustained low FPS and frame variance detection.
- Rolling 1% / 0.1% low frame-time summaries.
- Physical memory growth slope detection.
- GC pause spike detection through UE GC delegates.
- Basic in-engine hang signal via very large frame delta.
- Periodic snapshots and final per-session reports.
- Project-side operation stall events bridged from the existing T66 lag tracker.

The first pass does not implement per-draw-call attribution, automatic Insights triggering, reliable external hang detection, sidecar telemetry, or cross-session regression comparison.

## Output

Runtime output is local-only:

- `Saved/PerformanceSystem/snapshot.current.json`
- `Saved/PerformanceSystem/snapshot.previous.json`
- `Saved/PerformanceSystem/Sessions/<session>/events.jsonl`
- `Saved/PerformanceSystem/Sessions/<session>/board_saturation_samples.jsonl`
- `Saved/PerformanceSystem/Sessions/<session>/session_summary.json`
- `Saved/PerformanceSystem/Sessions/<session>/session_summary.md`

Default retention budgets:

- Development: 256 MB total, 25 MB per session target.
- Shipping: 64 MB total, 10 MB per session target.

## Privacy

Reports never intentionally include Steam IDs, usernames, hostnames, user home paths, IPs, MACs, hardware serials, save contents, backend URLs, API keys, or auth tokens. User-profile paths in captured log lines are redacted before persistence.

Hardware model strings are config-gated and default off in Shipping builds.

## Schema

Versioned schemas live in `PerformanceSystem/schema/`. Every persisted event/report carries a monotonic integer `SchemaVersion`.

Current runtime schema version is `4`. Version 4 adds rich-actor versus lightweight-mob board-saturation split counts for the Lightweight Actor migration captures.
