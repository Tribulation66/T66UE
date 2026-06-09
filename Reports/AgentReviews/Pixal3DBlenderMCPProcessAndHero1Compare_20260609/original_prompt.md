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
