# Pixal3D

Read `PIXAL3D_AGENTS.md` first for routing and safety rules.

Pixal3D is production-cleared for T66 model replacement assets when the
active FriendSlop raw import workflow is used. Do not route playable assets
through old one-off imports, manual material setup, or archived ToonStyle /
QuadRetro processing unless the user explicitly revives that legacy path.

Primary production path:

1. Read `../Instructions/11_FRIENDSLOP_RAW_PIXAL3D_IMPORT_GUIDELINES.md`.
2. Use the approved FriendSlop source run or manifest evidence for the target rows.
3. Run the active FriendSlop import/reload/validation scripts named by that doc.
4. Finish with validation so generated GLB texture binding, material assignment,
   Unreal bindings, data-table reloads, and the production report are checked.

The pipeline reference is `PIXAL3D_PIPELINE_REFERENCE.md`.
