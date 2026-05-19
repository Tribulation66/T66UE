# Slime Retained Source Note

Date: 2026-05-17

`slime.glb` in this folder was copied from:

`SourceAssets/ToonStyle/Pixal3D/Phase1A/Slime/Raw/slime.glb`

Reason: the corrected Phase 1C Slime image passed image QA, but Pixal3D failed repeatedly at the original Phase 1C settings (`R1536 T4096`, default sampling) on the old A40 pod and on both CUDA devices of the fresh dual-RTX-4090 pod. Pablo approved retaining the old Slime model for Phase 1C so the test room has a deliberate comparator against the newly regenerated assets.

Downstream scripts and reports should treat this as a retained legacy comparator, not as a newly generated Phase 1C Pixal3D output.
