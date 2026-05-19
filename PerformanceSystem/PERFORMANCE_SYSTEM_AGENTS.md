# PerformanceSystem Agents

## Scope

- Use this folder for T66 performance, diagnostics, profiling, telemetry, perception, and optimization-readiness work.
- Keep human-readable contracts, schemas, reports, and sidecar notes under `PerformanceSystem/`.
- Keep Unreal runtime code under `Source/T66/PerformanceSystem/` inside the existing `T66` game module unless a later prompt explicitly asks for a plugin or separate module.

## Rules

- Do not add optimizer fixes while working on the diagnostics system unless the user explicitly changes scope.
- Treat sidecar hardware telemetry as optional enrichment. The in-engine system must run without the sidecar.
- Keep Shipping privacy gates conservative: hardware model strings default off in Shipping and default on in Development.
- Every event, report, and sidecar payload must carry `SchemaVersion`.
- Add new detectors through the detector/runtime interfaces; avoid coupling one-off detector logic to unrelated gameplay classes.
- If a change affects playable runtime behavior, follow the root standalone shortcut verification rule.

