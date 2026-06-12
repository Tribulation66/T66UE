Verdict: APPROVE

## Blockers
- None.

## Major Issues
- None that block. The plan preserves the proven VAT method class (same WPO HLSL, same texture/BBox params, only the `Frame` source moves from MID scalar to `PerInstanceCustomData` slot 0), keeps original `M_Character_Unlit` / `M_EasyMobVAT_Unlit_UV2` / existing MIs untouched, keeps collision/hit-zones/registry actor-owned, and gates rollout behind a Stage 1 one-type proof. This satisfies PPF, the artifact parity gate, and the mechanism manifest without a method substitution, so no separate user approval is required on those grounds.

## Minor Issues
- `NumCustomDataFloats = 6` while the material consumes only slot 0. Writing five extra floats per active instance per frame is overhead carried purely for "B.13+ readiness," which is out-of-scope future-proofing on a change whose entire justification is FPS. Recommend Codex either (a) write only slot 0 in B.13 and defer slots 1–5, or (b) explicitly confirm the per-frame `SetCustomDataValue` cost for the unused slots is included in the after-capture and does not erase the draw-call win. The full-res lows gate will surface a regression, so this is not blocking.
- Editor automation script lives under `Reports/AgentReviews/.../`. Per AGENTS Script Lifecycle, fold any durable material-edit logic into a master script/process doc and delete the task-specific script at completion; the packet's verification section should note this teardown.
- Copy-back-to-live step is the riskiest mechanical action. The drift check ("stop and reconcile if a target file changed") is the right guard, but the packet should make explicit that the copy targets only the enumerated B.13 paths (the new material + the named source files) and never touches the unrelated dirty worktree files, and should record the pre/post drift snapshot as an artifact.

## Clarifying Questions
- Is the 6-slot custom-data layout a hard requirement from a later pass (Pass A contract), or can B.13 ship slot 0 only? This is the one judgement that affects the perf result.

## Required Verification
- Material audit confirming: new `M_EasyMobVAT_Unlit_UV2_Instanced` exists, original parent + all `MI_EasyMobVAT_*` unmodified, and the duplicate's WPO `Frame` input resolves to `MaterialExpressionPerInstanceCustomData` DataIndex 0.
- Stage 1 anti-lookalike discriminator: ≥3 same-type Slimes at visibly different frames/poses in one multi-frame capture (guards against lockstep / static-mesh-no-WPO failure).
- Rich placed-Slime/guardian smoke proving the original MID path still renders.
- Full-res before/after at `r.ScreenPercentage=100`, RetroFX off, 3 captures each, median + 1% + 0.1% lows, with SHA256 binary-hash stability and `PerformanceSystemOverheadMaxUs <= 10000`; lows must improve or hold or be investigated.
- Staged-standalone refresh + taskbar shortcut target verification after live integration.

## Rationale
The packet is scope-bounded, PPF/parity/mechanism-compliant, uses an isolated A/B tree to keep the dirty worktree constant, names concrete files, and specifies strong runtime + perf + material verification with explicit failure gates. The method is the same class as the existing VAT pipeline, so no method-substitution approval is owed. Remaining items (custom-data slot count, script teardown, copy-back precision) are Codex-resolvable refinements that the existing perf and drift gates already catch, so they do not warrant holding implementation.

