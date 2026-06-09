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
