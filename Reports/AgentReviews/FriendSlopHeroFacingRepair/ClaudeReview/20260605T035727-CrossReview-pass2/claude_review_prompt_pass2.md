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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\FriendSlopHeroFacingRepair\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\FriendSlopHeroFacingRepair\codex_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\FriendSlopHeroFacingRepair\ClaudeReview\20260605T034448-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:
Ok great it works except for one thing which is the hero is facing the wrong way, so back is front and front is back for him, an issue the mobs dont have, so I think we need to figure out the central cause and solve it systematically so future heros dont face this problem.

Task contract:
Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: identify why raw FriendSlop hero meshes face backward while mobs do not, fix the import path systematically for future heroes, apply the fix to Hero 1 Chad, and verify the playable staged build.
Stop condition: root cause is documented, the reusable import/data path has a hero-facing correction rule, Hero 1 Chad faces correctly in runtime evidence, and build/capture verification is reported.

Relevant repo rules:
- Follow AGENTS.md task contract and Operator/Validator protocol.
- Use Model Generation instructions for FriendSlop raw Pixal3D imports.
- Do not use ToonStyle or post-processing for FriendSlop raw imports.
- Playable content changes require DataTable reload, staged standalone refresh, shortcut verification, and Unreal-owned visual proof when orientation is being judged.

</original_prompt>

<validator_independent_answer>
Result: OK

## Independent Answer

**Root cause (high confidence from data evidence):** The mesh "forward" axis convention differs by pipeline, and the FriendSlop raw hero mesh is baked 180° opposite to the convention its DataTable row assumes.

Concretely, in `Content/Data/CharacterVisuals.csv`:
- **All heroes/companions/NPCs** use `MeshRelativeRotation (Pitch=0, Yaw=-90, Roll=0)` — including the new `Hero_1_Chad` FriendSlopRaw row (CharacterVisuals.csv:2).
- **All mobs and bosses** (the `Enemies/Mobs/Bosses` Pixal3D meshes) use `Yaw=+90` (CharacterVisuals.csv:81+, 58+).

So mobs "don't have the problem" because their meshes are authored to one forward convention and their rows carry the matching `+90`. The ToonStyle hero meshes (Hero_2–5) are authored to the *opposite* forward convention and their rows carry `-90`. The FriendSlop raw export for Hero_1 Chad bakes the model facing the **mob-style** direction, but it inherited the **hero** row value `-90` — leaving it exactly 180° wrong (back-as-front). This is the single 180° flip the user reported.

**Systematic fix — pick the import-path correction, not a per-row hack.** Two options:
1. **(Recommended) Normalize at import.** Bake a 180° yaw into the FriendSlop raw import/export step (Blender export or `ImportFriendSlopRawPixal3DFBXAndExit.py` / `export_accurig_textured_batch.py`) so the raw hero mesh's forward axis matches the ToonStyle hero convention. Then the shared `Yaw=-90` row works unchanged and **every future FriendSlop hero** is correct automatically. Add an explicit "hero-facing correction" rule to `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` (it currently has none).
2. **(Fallback) Data-row override.** Set `Hero_1_Chad` (and `Hero_1_Chad_DemoSkin`) `MeshRelativeRotation` Yaw to `+90`. This fixes Hero 1 but bakes a per-pipeline exception into data that must be remembered for every future FriendSlop hero — fragile, and weaker against the "future heroes" stop condition.

Prefer option 1 to satisfy "solve the central cause systematically so future heroes don't face this problem." Whichever path Codex takes, the documented rule belongs in doc 11.

**Verification still required by contract:** DataTable reload → staged standalone refresh → shortcut verification → Unreal-owned visual capture showing Hero_1 Chad facing forward. That runtime proof is Operator (Codex) work in the live editor/build; it is not yet in evidence here.

## Evidence Checked
- `original_prompt.md` and the task contract.
- `Source/T66/Data/T66DataTypes.h:2262,2365` — `MeshRelativeRotation` default `FRotator(0,-90,0)`.
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp:1084,1532,1614` — row rotation applied via `SetRelativeRotation`.
- `Content/Data/CharacterVisuals.csv` — heroes/companions/NPCs `Yaw=-90`; mobs/bosses `Yaw=+90` (61 vs 75 rows); Hero_1_Chad FriendSlopRaw row uses `-90`.
- `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md` — no existing orientation/facing rule.

## Questions Or Blockers
None requiring the user. Scope is approved; the fix-location choice and runtime verification are Operator-executable.

## Caveats
- I cannot read the binary `.uasset`/FBX to directly confirm the mesh's baked forward axis — the 180° conclusion is inferred from the user's "back is front" report plus the identical `Yaw=-90` shared with correctly-facing ToonStyle heroes. Codex should confirm in-editor before committing the import-time 180°.
- I performed no runtime capture; orientation proof for Hero_1 Chad is unverified here and remains a hard stop-condition item.
- If any future FriendSlop hero is instead authored to the hero forward convention, an unconditional import-time 180° would over-rotate it — so the doc rule should state the expected raw-export forward axis, not just "always add 180°."

</validator_independent_answer>

<codex_draft>
Codex draft for cross-review.

Root cause:
- `CharacterVisuals.csv` is the runtime orientation adapter for static character meshes.
- Legacy/processed heroes use `MeshRelativeRotation=(Pitch=0,Yaw=-90,Roll=0)`.
- Raw Pixal3D mobs use `MeshRelativeRotation=(Pitch=0,Yaw=90,Roll=0)`.
- The new raw FriendSlop Hero 1 mesh used the raw/static forward convention but inherited the legacy processed-hero `-90` yaw, causing the exact back/front inversion the user reported.

Changes made:
- Added a Facing Preservation Rule to `Model Generation/Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
- Added reusable manifest-driven helper `Scripts/ApplyFriendSlopRawCharacterVisualRows.py`.
- Added `character_visual_rows` and `character_visual_yaw: 90.0` to `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json`.
- Updated `Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` so the raw FriendSlop Hero 1 rows are applied from manifest before `DT_CharacterVisuals` is reloaded.
- Updated `Scripts/ValidateFriendSlopRawPixal3DAndExit.py` so raw Hero 1 rows fail validation if they regress from expected yaw 90.
- Updated `Content/Data/CharacterVisuals.csv`:
  - `Hero_1_Chad` yaw changed from `-90.000000` to `90.000000`.
  - `Hero_1_Chad_DemoSkin` yaw changed from `-90.000000` to `90.000000`.

Verification:
- `python -m py_compile Scripts/ApplyFriendSlopRawCharacterVisualRows.py Scripts/ValidateFriendSlopRawPixal3DAndExit.py Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py` passed.
- `python Scripts/ApplyFriendSlopRawCharacterVisualRows.py "Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/FriendSlopProbe_Hero1Male_20260604_1415_manifest.json"` applied two rows at yaw 90.
- Unreal DataTable reload via `UnrealEditor.exe -ExecutePythonScript=Scripts/ReloadFriendSlopEasyPixal3DDataTablesAndExit.py -NullRHI` exited 0. Log evidence in `Saved/Logs/T66-backup-2026.06.05-06.50.28.log` shows:
  - `Applied FriendSlop raw CharacterVisual row count: 2`
  - `=== SetupCharacterVisualsDataTable DONE ===`
  - `=== ReloadFriendSlopEasyPixal3DDataTablesAndExit DONE ===`
- Unreal validator via `UnrealEditor.exe -ExecutePythonScript=Scripts/ValidateFriendSlopRawPixal3DAndExit.py -NullRHI` exited 0.
- Validator report `Reports/AgentReviews/FriendSlopEasyPixal3D/raw_runtime_reference_validation.json` has `ok=true`, `errors=[]`, and both Hero 1 rows have `actual_yaw=90.0`, `expected_yaw=90.0`, `yaw_ok=true`.
- Current Unreal-owned gameplay capture:
  - `Reports/AgentReviews/FriendSlopHeroFacingRepair/VisualProof_Hero1ChadRawFacing/Hero1ChadRawFacing.mp4`
  - `Reports/AgentReviews/FriendSlopHeroFacingRepair/VisualProof_Hero1ChadRawFacing/evidence/contact_sheet.png`
  - Visual inspection: chase-camera view shows Hero 1's back toward the camera while he moves toward the enemies, which is the expected in-game forward orientation.
- `Scripts/StageStandaloneBuild.ps1` succeeded. Log: `Reports/AgentReviews/FriendSlopHeroFacingRepair/stage_standalone.log`
  - `BUILD SUCCESSFUL`
  - staged exe ready at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut target verification:
  - `C:\UE\T66\T66 Standalone.lnk` target is staged exe.
  - taskbar `T66 Standalone.lnk` target is staged exe.

Caveats:
- The hero is still raw/static because FriendSlop has no rigging/processing pipeline yet, matching the user's prior instruction.
- The systematic correction is in manifest-driven CharacterVisuals row application, not in baked mesh reorientation. That preserves the raw generated mesh and makes the runtime yaw explicit for future raw heroes.

Proposed final answer:
- Tell the user the hero-facing issue is fixed and staged.
- State the cause: raw FriendSlop hero used raw/static forward convention but inherited legacy processed-hero yaw.
- Mention the reusable manifest/document/validator additions.
- Provide proof paths and staged exe path.

</codex_draft>
