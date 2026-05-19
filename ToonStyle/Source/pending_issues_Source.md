# Pending Issues - ToonStyle Source

## Vertex color A masking remains unavailable in outline WPO

Severity: [Major]

What's wrong: `Saved/Codex/ToonStyle/Phase1C/Diagnostic2/Phase1C_SM6_Diagnostic_Report.md` showed that graph-distance outline modulation cooks correctly, but `VertexColor.A` in World Position Offset paths fails because UE5.7 reduces the vertex color expression to `float3` in that derivative-aware WPO context. `ToonStyle/Source/SetupPhase1CToonMaterials.py` therefore locks mask behavior into vertex color G (`G=0` means no extrusion) and leaves A unused.

Why it's out of scope now: Fixing alpha access would require an engine/rendering-path change, which is explicitly outside the no-engine-mod ToonStyle architecture. The Phase 1C R1 remediation restores the stock-engine path by shifting the channel assignment instead.

What fixing it would entail: Either modify Unreal's material compiler / vertex factory path to preserve alpha for WPO-stage vertex color usage, or add a separate authored data channel that is accessible in stock UE5.7 without engine changes.
