I have sufficient evidence. Writing the validator answer.

Result: OK

## Independent Answer

The request decomposes into three internally-handleable parts; Codex can execute all of them and close with evidence. No user-only decision is blocking.

**1. Docs: open the intended file, not the default cube.** Root cause is launching `blender.exe` with no file argument (or a space-containing path that gets split), so Blender boots its startup `.blend` (default cube). The fix already exists in-repo: `Model Generation/Scripts/Core/Blender/OpenBlenderScene.ps1` passes the resolved path via `ProcessStartInfo.ArgumentList` (single argument, space-safe) and even background-verifies the file loaded before showing the window (lines 24–43, 45–48). The docs do **not** yet reference this helper or state the rule. Codex should add, to `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` (and cross-link from `BLENDER_LAB_MCP_INSTRUCTIONS.md`):
   - "Never launch Blender bare. Open a scene with `OpenBlenderScene.ps1 -BlendFile <path>`, which resolves the path, verifies load in `--background`, and confirms the window title — this prevents the default-cube fallback and path-with-spaces splitting."

**2. Docs: always start Blender MCP first.** Add an explicit precondition rule: before any Blender work, start the official bridge via `Model Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1` (use `-Visible` when a window is needed) and confirm the MCP socket is reachable; do not silently fall back to background/`--python-expr` or a non-MCP path. Belongs in `04_…RIGGING_INSTRUCTIONS.md` Policy section and reinforced in `BLENDER_LAB_MCP_INSTRUCTIONS.md`.

**3. Add current Hero 1 male beside the two models.** Evidence is consistent and confirmed live:
   - Runtime row `Hero_1_Chad` → `SK_Hero_1_Chad_PhysicsFirst`; `Hero_1_Chad_DemoSkin` → `SM_Hero_1_Chad_Male` (both under `FriendSlopRaw`).
   - Both derive from the source asset `Hero_1_Chad_Male` in run `FriendSlopProbe_Hero1Male_20260604_1415`; source GLB `…/Outputs/Hero_1_Chad_Male.glb` exists.
   - The "two models" are `Hero1Stacy.glb` and `Hero2Chad.glb` in run `HeroChadStacy_SourceAssets_20260609_0536/Outputs/`.
   - So importing `Hero_1_Chad_Male.glb` next to those two is the correct interpretation of "current model used by Hero 1 male."

Codex should confirm whether `HeroChadStacy_SourceAssets…` already has a target `.blend` containing the two models (import the male into it) or assemble a fresh side-by-side scene, then open that exact scene via `OpenBlenderScene.ps1` with MCP running. If MCP can't be started, report that as the concrete blocker per the stop condition rather than falling back silently.

## Evidence Checked
- `.t66/operator-state.json` — Codex operator, Claude validator (matches prompt).
- `Content/Data/CharacterVisuals.csv` — `Hero_1_Chad` → `SK_Hero_1_Chad_PhysicsFirst`; `Hero_1_Chad_DemoSkin` → `SM_Hero_1_Chad_Male`.
- `FriendSlopProbe_Hero1Male_20260604_1415/README.md` — asset `Hero_1_Chad_Male` serves runtime rows `Hero_1_Chad` + `…_DemoSkin`.
- Glob confirms `Hero_1_Chad_Male.glb`, and `Hero1Stacy.glb` + `Hero2Chad.glb` exist.
- `OpenBlenderScene.ps1` — space-safe `ArgumentList` launch + background load verification.
- `04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` and `BLENDER_LAB_MCP_INSTRUCTIONS.md` — neither yet states the "open intended file" or "MCP-first" rules; correct edit targets.

## Questions Or Blockers
- None require the user. The ANTHROPIC_API_KEY precondition is the harness's gate (validator already invoked); my direct env reads were sandbox-blocked, so I did not independently confirm it — flagging as an evidence gap, not a blocker.

## Caveats
- "Current Hero 1 male" in runtime is a skeletal mesh (`SK_…PhysicsFirst`); adding the source `Hero_1_Chad_Male.glb` represents that model's geometry but is the source GLB, not the imported UE skeletal asset. If the user specifically wants the exact UE runtime skeletal mesh rather than its source, that's a scope clarification — but the source GLB is the reasonable and available representation.
- Confirm the target `.blend` for the two existing models before importing; if none exists, a new scene must be built (Codex's call, still OK).
- MCP availability is environmental; if the bridge won't start, that is the one place this turns into a reported blocker rather than a completed task.
