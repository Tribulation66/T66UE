# Pending Issues - ToonStyle Source

## Vertex color A masking remains unavailable in outline WPO

Severity: [Major]

What's wrong: `Saved/Codex/ToonStyle/Phase1C/Diagnostic2/Phase1C_SM6_Diagnostic_Report.md` showed that graph-distance outline modulation cooks correctly, but `VertexColor.A` in World Position Offset paths fails because UE5.7 reduces the vertex color expression to `float3` in that derivative-aware WPO context. `ToonStyle/Source/SetupPhase1CToonMaterials.py` therefore locks mask behavior into vertex color G (`G=0` means no extrusion) and leaves A unused.

Why it's out of scope now: Fixing alpha access would require an engine/rendering-path change, which is explicitly outside the no-engine-mod ToonStyle architecture. The Phase 1C R1 remediation restores the stock-engine path by shifting the channel assignment instead.

What fixing it would entail: Either modify Unreal's material compiler / vertex factory path to preserve alpha for WPO-stage vertex color usage, or add a separate authored data channel that is accessible in stock UE5.7 without engine changes.

## M_Toon_Character cook logs invalid cached include path warning

Severity: [Minor]

What's wrong: The Development stage on 2026-05-19 completed successfully, but the cook log emitted `LogMaterial: Warning: Expression include file path '/Project/ToonStyle/ToonShadingCommon.ush' is invalid, removing from cached data for material '/Game/ToonStyle/Materials/M_Toon_Character.M_Toon_Character'`. The warning concerns the cached Custom node include path on the binary master material, not the Pixal3D importer parameter list.

Why it's out of scope now: This bulletproofing pass was scoped to fallback policy, pipeline docs, importer parameter binding, report alignment, and validation. It did not regenerate or edit material `.uasset` assets.

What fixing it would entail: Run a focused material-regeneration/inspection pass through `ToonStyle/Source/SetupPhase1CToonMaterials.py`, confirm the shader source directory mapping is registered before material compilation, and verify a cook without the cached include-path warning.
