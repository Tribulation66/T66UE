# Humanoid Hero And Companion Animation Pipeline

This process is for full humanoid action sets for heroes and companions. Use it for bipedal player-scale characters that can share a humanoid skeleton, humanoid retarget map, or Rigify/Rigodotify control rig.

Monster, boss, quadruped, flying, blob, and non-humanoid rigs need a separate pipeline. Do not force them through this humanoid process just because they need animation clips.

Typical hero scope:

- idle
- walk or run
- jump
- roll or dodge
- attack
- later ability-specific actions

Typical companion scope:

- idle
- walk or follow
- alert or interact
- attack or support action when gameplay requires it

## Acceptance Bar

Do not accept a humanoid action because it is merely recognizable. A production candidate must satisfy:

- readable silhouette from gameplay and preview camera angles
- stable proportions across the whole action
- no severe shoulder, elbow, wrist, hip, knee, or ankle collapse
- foot contact that does not slide unless intentionally stylized
- root or pelvis motion that matches the intended in-game behavior
- clean start and recovery poses for blending
- no mesh clipping that is visible from front, side, and three-quarter review angles

Roll-specific checks:

- clear crouch or dip before launch
- believable plant and tuck
- shoulder/back/side contact implied by body arc
- pelvis and head follow a coherent path
- feet recover under the body
- no full-body spin that reads like a generic tumble instead of an intentional combat roll
- for in-place gameplay rolls, side and gameplay-camera sheets must show the character tucking forward in the same direction the actor travels; a backward-opening backflip pose fails even if the runtime launch direction is correct

Jump-specific checks:

- anticipation before takeoff
- upward extension during launch
- airborne shape readable from the gameplay camera
- landing compression and recovery
- no looping airborne pose unless the runtime deliberately holds it

## Required Loop

Every humanoid animation pass must follow this order:

1. Identify the character, current source mesh, skeleton, visual row, and required gameplay states.
2. Create or update a Blender run folder and treat its `.blend` file as the source of truth.
3. Build or confirm the rig:
   - If the character already has a production skeletal mesh, preserve that mesh and skeleton unless there is a clear reason to rerig.
   - If the character is static or has an unusable skeleton, build a Rigify/Rigodotify-compatible humanoid rig in Blender.
   - Keep deformation bones and control bones understandable; name any generated retarget map in the run manifest.
4. Retarget or author the actions in Blender.
5. Render preview frames before export.
6. Review the contact sheets and write the QA result.
7. Make Blender-side corrections for any accepted blocker.
8. Rerender previews after corrections.
9. Export action clips only after the corrected Blender preview pass is acceptable.
10. Import into Unreal and verify asset type, skeleton, play length, and data wiring.
11. If gameplay-visible, refresh staged standalone and verify the shortcut target.

Skipping Step 5-8 is allowed only for a deliberate rough prototype, and the final report must say it is not production accepted.

## Blender Rigging Rules

- Keep one source `.blend` per character/run under `Runs/<Character>_<Purpose>_<YYYYMMDD>/`.
- Keep the target character armature, source/reference armature, retarget map, baked actions, preview camera setup, and export settings in that scene.
- Do not destructively edit vendor `.blend` files under `External/`; append/link or import into the run scene.
- Prefer action names that match Unreal output names, for example `AM_Hero_1_Chad_Walk`.
- For heroes, preserve the canonical game mesh scale and skeleton identity unless the task is explicitly to replace the rig.
- For companions, use the same humanoid checks, but action scope can be smaller if gameplay only needs idle/follow/alert.

## Visual QA

Required Blender preview evidence for production acceptance:

- front contact sheet
- side contact sheet
- three-quarter contact sheet
- gameplay-camera contact sheet when the runtime camera is different enough to hide contact or silhouette issues
- frame samples that include anticipation, contact, apex/impact, and recovery poses
- written QA notes listing either `PASS` or concrete issues per action

Review each action for:

- silhouette readability
- foot and hand contact
- pelvis path
- head path
- shoulder, elbow, wrist, hip, knee, and ankle collapse
- visible mesh clipping
- scale drift or mesh deformation
- start and recovery poses that can blend back to idle/walk

When a preview fails, fix the Blender action first. Do not compensate for a bad action by hiding it in Unreal, shortening the clip blindly, or declaring it done from a single still frame.

## Unreal Import And Runtime Wiring

Heroes currently use `CharacterVisuals.csv` / `DT_CharacterVisuals` animation slots:

- `LoopingAnimation`: walk/run loop
- `AlertAnimation`: idle/alert loop
- `RunAnimation`: current jump slot for the Arthur pilot
- `RollAnimation`: one-shot roll

Companions currently consume humanoid movement/alert slots through the character visual subsystem. Add companion-specific runtime state only when the companion gameplay requires an action that the current slots cannot represent.

Unreal validation must confirm:

- imported assets are `AnimSequence`
- skeleton path matches the character skeleton
- play length is nonzero and expected
- data row points to the imported assets
- runtime preload path includes new animation slots
- staged standalone is refreshed when playable behavior changed

## Arthur Pilot

Use Arthur as the first proof case before scaling to all heroes.

1. Select the canonical Arthur mesh for the pilot.
2. Fit or generate a Rigodotify/Rigify-compatible humanoid rig.
3. Retarget or author `Idle`, `Walk`, `Jump`, and `Roll`.
4. Save the `.blend` scene as the source of truth under a run folder.
5. Render front, side, and three-quarter action previews.
6. Export action clips only after Blender QA passes.
7. Import into Unreal through the existing skeletal import path.
8. Wire a test animation set before switching default Arthur visuals.

Deleted Arthur pilot run:

- `Hero_1_Chad_AnimPilot`, the old root-level `SK_Hero_1_Chad` skeletal mesh, and the pilot-specific tools/runs were deleted after the QuadRetro pass was validated.
- That pilot was useful only as a failed research path. It was not the selected Royal Chad/Arthur visual and previously caused the old headless/giant-model failure when wired live.
- Do not restore or rerun the deleted pilot path for production Arthur work.

Use forward slashes for Unreal Python `-script=` paths when invoking tools under this folder; backslashes before filenames such as `verify_...` can be parsed as escape characters by the commandlet path reader.

When invoking Rigging and Animation tools from Unreal, prefer the wrapper `Scripts/RunRiggingAnimationToolAndExit.py` with `T66_RIGGING_ANIMATION_TOOL_SCRIPT` set to the real tool path. Do not use 8.3 short paths such as `MODELG~1` / `RIGGIN~1` as a workaround for spaces; Unreal's Python runner can treat those as Python text and fail with `SyntaxError`.

Accepted Royal Chad/Arthur QuadRetro UAL retarget run:

- Correct playable visual row: `Hero_1_Chad`.
- Correct source model: `../Runs/Pixal3D/HeroArthur01/Post/QuadRetro/arthur_royal_chad/Models/arthur_royal_chad_QuadRetro.glb`.
- Correct skeletal texture: `/Game/Characters/Heroes/Hero_1/Chad/QuadRetro/RoyalChad_QuadRetro/Textures/RoyalChad_QuadRetro_Pixelated_512.RoyalChad_QuadRetro_Pixelated_512`.
- Do not apply the normalized static-mesh atlas to the GLB-derived skeletal mesh. The animated mesh keeps the GLB UV layout, so using `RoyalChad_QuadRetro_Pixelated_512_Normalized` scrambles the runtime texture.
- Correct animation source: `External/Quaternius/Universal Animation Library Source/UAL1.blend`.
- UAL action mapping: `Idle_Loop` -> idle, `Walk_Formal_Loop` -> walk, `Jump_Start` + `Jump_Loop` + `Jump_Land` -> jump, `Roll_RM` -> roll with root motion stripped and the local-X sagittal mirror correction applied by `Tools/create_arthur_quadretro_ual_animation_source.py`.
- Blender source of truth: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_QuadRetro_UAL_Retarget.blend`.
- FBX exports: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Exports/SK_Hero_1_Chad_QuadRetroUALQA.fbx` and `AM_Hero_1_Chad_QuadRetroUALQA_{Idle,Walk,Jump,Roll}.fbx`.
- QA contact sheets: `Runs/Arthur_QuadRetro_UAL_Retarget_RollForward_20260515/Arthur_All_Actions_{front,side,three_quarter,gameplay}_Contact_Sheet.png`.
- Unreal import tool: `Tools/import_arthur_quadretro_animation_to_unreal.py`.
- Unreal verification tool: `Tools/verify_arthur_quadretro_animation_in_unreal.py`.
- Temporary validation row: `Hero_1_Chad_QuadRetroUALQA`.
- Promoted live row: `Hero_1_Chad`, after the temporary row was visually verified in gameplay.
- Correct promoted-row forward fix: `MeshRelativeRotation=(Pitch=0,Yaw=90.000000,Roll=0)`. The older static row used the opposite yaw; carrying that value onto the skeletal pass makes the hero walk backward.
- Correct roll-direction fix: the runtime roll burst remains actor-forward, but the in-place roll animation must use the corrected roll bake above. `Roll` and uncorrected `Roll_RM` both read as a backward flip after source root motion is stripped.
- Live promotion is opt-in: set `T66_ARTHUR_QUADRETRO_PROMOTE_LIVE=1` when running `Tools/import_arthur_quadretro_animation_to_unreal.py`. Without that flag, the importer should only refresh the temporary validation row.
- The verifier must check the skeletal material instance texture parameters (`EmissiveTexture`, `BaseColorTexture`, and `DiffuseColorMap`), not only the row's `PixelatedTextureAssetPath`. The runtime skeletal material rebuild reads the material texture parameters.

Keep this sequence for future live hero replacements:

1. Trace the live row and source mesh from normal selection.
2. Rig the exact visible source mesh in Blender when no production skeleton exists.
3. Render and inspect front, side, three-quarter, and gameplay contact sheets for all required actions.
4. Import to a temporary row first.
5. Capture the temporary row in gameplay through the real hero pawn path.
6. Promote the live row only after the temporary row keeps the correct look, scale, and attachments.
7. Verify texture atlas compatibility, promoted-row yaw, and material texture parameters before staging.
8. Reload data tables, build, stage standalone, verify the shortcut target, and smoke boot the staged executable when playable content changes.

## Runtime Boundary

Current T66 runtime supports character visual rows with looping/walk, alert/idle, run/jump, and one-shot roll animation slots. A richer state machine still needs a separate animation-set table or AnimBP later, but the accepted QuadRetro Arthur pass now populates all four required gameplay clips through `CharacterVisuals.csv` / `DT_CharacterVisuals`.
