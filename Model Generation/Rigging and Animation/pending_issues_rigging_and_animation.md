# Pending Issues - Rigging And Animation

Historical humanoid rigging issues were intentionally removed from this active pending list because automated hero rigging is no longer part of this folder's working scope. Heroes and humanoid companions are being rigged manually outside this mob/VAT process.

## VAT Crowd Performance Is Not Proven For Hundreds Yet

Severity tag: [Major]

What's wrong: `06_MOB_ANIMATION_GUIDELINES.md` and `03_FINDINGS_AND_LIMITATIONS_REFERENCE.md` document VAT as the intended mob runtime direction, but the current Easy mob VAT path has not yet been profiled with 50 / 100 / 200 / 300 active enemies. The current implementation can still bottleneck on draw calls, per-enemy dynamic material instances, material parameter updates, AI, collision, pooling, or culling.

Why it's out of scope now: This pass created the mob animation guidance and removed obsolete humanoid automation clutter. It did not change playable runtime content or run a crowd profiling pass.

What fixing it would entail: Build a focused crowd performance scenario, run it through the `PerformanceSystem` workflow, compare CPU/GPU cost at increasing enemy counts, then decide whether to keep per-enemy material instances or move playback to instancing/per-instance custom data, pooling, culling, lower texture sizes, and reduced clip frame counts.
