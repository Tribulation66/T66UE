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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\open_in_blender_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\open_in_blender_operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\ClaudeOpenBlender\20260605T113435-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:
Open them in blender one next to another.

Working task:
Operator: Codex
Validator: Claude
Scope: Open the existing Blender look-dev scene so the raw male Hero 1 Pixal3D model and rubber candidate are visible side by side. No Unreal, no rigging, no runtime import, no material changes unless needed to make the scene open correctly.
Stop condition: Blender is launched on the comparison scene, or the launch failure is reported.

Relevant context:
- Source GLB: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Outputs\Hero_1_Chad_Male.glb`
- Look-dev blend: `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`
- The scene already contains the raw Pixal3D GLB and rubber material variations in a comparison layout.


</original_prompt>

<validator_independent_answer>
Both target files exist. This is a launch-an-app task Codex can execute internally; no user decision is required.

Result: OK

## Independent Answer

The launch is straightforward and the scene is intact, so Codex can proceed without user input. Recommended approach:

1. Launch Blender directly on the existing comparison scene (do **not** re-import or rebuild):
   ```powershell
   & "<blender.exe>" "C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend"
   ```
   The scene already holds the raw Pixal3D GLB and the rubber candidate in a side-by-side layout, so simply opening it satisfies "one next to another."

2. The Blender MCP tools are **not** the right launch path — they attach to an already-running connected Blender instance, not cold-start a .blend file. If Blender is already open and connected, MCP can confirm/screenshot the side-by-side result, but the actual file open should be the shell launch above.

3. Stop condition is binary: report success once Blender opens on the scene, or report the exact launch failure (e.g., blender.exe path not found).

One thing for Codex to confirm before claiming success: the `blender.exe` path. The prompt does not specify a Blender install location, so Codex should resolve it (PATH, standard install dir, or prior run logs) rather than guess.

## Evidence Checked
- `Hero_1_Chad_Male_RubberLookDev.blend` — exists (plus `.blend1` autosave backup).
- Source `Hero_1_Chad_Male.glb` — exists in Outputs.
- Supporting `rubber_recipe.json/.md` and `build_summary.json` present, consistent with a pre-built look-dev scene.

## Questions Or Blockers
- None requiring the user. Codex must resolve the `blender.exe` path itself; if it genuinely cannot locate any Blender install, that becomes a missing-tool blocker worth surfacing — but it should attempt resolution first.

## Caveats
- I did not open the .blend, so I can't independently verify the internal layout actually places both models side by side; I'm trusting the prompt's statement that the comparison layout already exists. If the scene opens but the models overlap or one is missing, that exceeds the "open it" scope and should be reported, not silently fixed.
- Stay within scope: no Unreal, no rigging, no material edits unless the file fails to open without them.

</validator_independent_answer>

<codex_draft>
Operator draft:

Codex opened the existing male Hero 1 rubber look-dev `.blend` in a visible Blender window.

Actions performed:
- Created and saved a dedicated scene named `SideBySide_Raw_vs_V04` inside `C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605\Hero_1_Chad_Male_RubberLookDev.blend`.
- The scene places the raw Pixal3D GLB on the left and `V04 Candy Rubber` on the right, using the same fixed soft Blender lighting rig from the look-dev pass.
- Launched visible Blender with `open_side_by_side_scene.py`, which switches the window to `SideBySide_Raw_vs_V04` and camera view.

Verification:
- Blender background save reported `T66_SIDE_BY_SIDE_SCENE_READY=SideBySide_Raw_vs_V04`.
- Visible Blender process started: `blender`, PID `29744`, `Responding=True`.

Scope:
- No Unreal work.
- No rigging/jiggle/runtime import work.
- Only Blender look-dev/report helper scripts and the look-dev `.blend` were touched.


</codex_draft>
