You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\Pixal3DBlenderMCPProcessAndHero1Compare_20260609\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\Pixal3DBlenderMCPProcessAndHero1Compare_20260609\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\Pixal3DBlenderMCPProcessAndHero1Compare_20260609\20260609T062625-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok one thing I do want you to add/ammend/edit to the process docs is that always when I ask to open something in blender it opens default cube scene lets figure out the solution so it opens the correct and then add those instructions to the proecss docs. Also an instruction to always run the Blender MCP before working on blender so we dont face these situations where the MCP is not running and forces you to use something else. And then lastly I want you to put the current model used by Hero 1 male next to these two in blender.

Working task:
Operator: Codex
Validator: Claude if the local T66 validator path is available
Scope: Update the Blender/model-generation process docs so Blender work starts with Blender MCP and opens the intended file instead of the default cube; then add the current Hero 1 male source model beside the two newly generated Pixal3D models in Blender.
Stop condition: Docs contain the new Blender launch/MCP rule, current Hero 1 male is identified from live repo source/runtime evidence, a Blender scene contains all three models side by side, and Blender is opened to that correct scene with MCP available or a concrete blocker is reported.

Relevant repo rules:
- Follow AGENTS.md root process router, OPERATOR_VALIDATOR_PROTOCOL.md, Model Generation agent docs, and Reports/AGENTS.md for review artifacts.
- Do not use native goal tools.
- Use current live repo state, not stale memory.
- For process-governed model/Blender work, provide PPF/artifact/mechanism gates and close with evidence.
- Before Claude validator use, verify ANTHROPIC_API_KEY is not set in Process, User, or Machine.
- Use Blender MCP before Blender work; update docs so future work does not silently fall back to an unverified Blender path.

Evidence gathered before validator:
- `.t66/operator-state.json`: Codex operator, Claude validator.
- `Content/Data/CharacterVisuals.csv` row `Hero_1_Chad` uses `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`.
- Current source-run README/manifest: `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415` identifies `Hero_1_Chad_Male` for runtime rows `Hero_1_Chad` and `Hero_1_Chad_DemoSkin`.
- Source GLB path to add: `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`.
- Existing two-model run: `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536`.
- Existing helper `Model Generation/Scripts/Core/Blender/OpenBlenderScene.ps1` uses `System.Diagnostics.ProcessStartInfo.ArgumentList`, which avoids the path-with-spaces default-cube failure when used.

</original_prompt>

<validator_independent_answer>
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

</validator_independent_answer>

<codex_draft>
## Draft Completion

Operator: Codex
Validator: Claude

Implemented:

- Updated `Model Generation/Instructions/04_BLENDER_PROCESSING_AND_RIGGING_INSTRUCTIONS.md` to make Blender Lab MCP launch/verification mandatory before Blender work, add `OpenBlenderScene.ps1` as a durable helper, forbid bare/unquoted user-facing scene opens, and require loaded-file verification.
- Updated `Model Generation/Tools/BlenderLabMCP/BLENDER_LAB_MCP_INSTRUCTIONS.md` with a required MCP preflight and exact scene-opening instructions using either `OpenBlenderScene.ps1 -BlendFile "<path-to-blend>"` or MCP `bpy.ops.wm.open_mainfile(filepath=r"<path-to-blend>")`.
- Updated `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/build_side_by_side_scene.py` so the comparison scene imports:
  - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`
  - `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/Outputs/Hero1Stacy.glb`
  - `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/Outputs/Hero2Chad.glb`
- Built and saved `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/Blender/HeroChadStacy_with_current_hero1_male.blend`.
- Rendered `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/QA/HeroChadStacy_with_current_hero1_male.png`.

Verification:

- `ANTHROPIC_API_KEY` was checked in Process/User/Machine before Claude validator use and was unset in all scopes.
- Blender MCP initially failed to connect, then `Model Generation/Tools/BlenderLabMCP/launch_blender_lab_mcp.ps1 -Visible` launched Blender and reported `127.0.0.1:9876` accepting connections.
- Blender MCP `get_objects_summary` first showed the startup cube, then after running the scene builder through MCP showed three review roots: `CurrentHero1Male_ReviewRoot`, `Hero1Stacy_ReviewRoot`, `Hero2Chad_ReviewRoot`, with three mesh objects.
- Blender MCP `get_blendfile_summary_path_info` reported:
  - `filepath`: `C:\UE\T66\Model Generation\Runs\Pixal3D\HeroChadStacy_SourceAssets_20260609_0536\Blender\HeroChadStacy_with_current_hero1_male.blend`
  - `is_saved`: true
  - `is_dirty`: false
  - `file_size_bytes`: 34935272
- Render artifact exists and was visually checked; all three bodies are framed in the QA PNG.
- Live runtime/source evidence for current Hero 1 male:
  - `Content/Data/CharacterVisuals.csv` row `Hero_1_Chad` uses `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`.
  - `FriendSlopProbe_Hero1Male_20260604_1415/README.md` and manifest identify `Hero_1_Chad_Male` as the source asset for rows `Hero_1_Chad` and `Hero_1_Chad_DemoSkin`.

PPF CLOSE
Process used: Model-generation Blender processing docs plus BlenderLab MCP launcher/open workflow.
Matches declared process: YES
Evidence: MCP was launched and verified before Blender scene work; the scene was built through MCP and Blender reports the intended saved file, not the default cube.

MECHANISM CLOSE
Mechanism: MCP readiness before Blender work
Status: PRESENT
Evidence: launcher socket success and MCP summaries before/after scene build.
Discriminator test: default-cube startup was observed before work; final MCP path/object summary shows the intended file and three review roots.
Reported status: FULL

MECHANISM CLOSE
Mechanism: correct file opening despite spaces in paths
Status: PRESENT
Evidence: docs now require `OpenBlenderScene.ps1` or MCP `open_mainfile`; current file path includes `Model Generation` and is loaded in Blender MCP as the saved comparison scene.
Discriminator test: Blender filepath and object summary prove the intended `.blend`, not a bare startup scene.
Reported status: FULL

MECHANISM CLOSE
Mechanism: scene-only comparison placement
Status: PRESENT
Evidence: exact source GLBs imported as three review roots; source GLBs were not modified.
Discriminator test: the QA render shows three distinct models side by side, and MCP reports three mesh objects under the three review roots.
Reported status: FULL

Open caveat:

- The current Hero 1 male shown in Blender is the source GLB for the current runtime asset lineage, not a UE-exported skeletal mesh. That is the practical representation available in the model-generation workspace and matches the live source-run/row evidence.

</codex_draft>
