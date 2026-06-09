User request:

Ok cool so I went in and found some problems, first of all, the chad 1 hero, and most of the enemies still used the old model, instead of the new one, however a few things used the new ones, which were the weapon idol, loot bag and some enemies, that previously were using placeholders the issue is they came without a texture and were fully blue, so now we need to create the friendslop import guidelines doc and the first thing to figure out is how to ensure the texture of the GLB is preserved when importing. And then go ahead and replace the models that are not using the ones we generated with the new ones we generated

Working task:
Operator: Codex
Validator: Claude
Scope: create a FriendSlop raw Pixal3D import guideline, diagnose why imported GLB-derived assets lost textures, fix the raw import path so textures/materials are preserved, and replace old-model runtime references with generated FriendSlop models where applicable.
Stop condition: guideline exists, runtime references point to generated models where applicable, imported assets have material texture bindings, and the staged standalone is refreshed or blocked verification is explicitly reported.

Relevant repo rules:
- Follow C:\UE\T66\AGENTS.md and OPERATOR_VALIDATOR_PROTOCOL.md.
- Codex is Operator; Claude is read-only Validator.
- This task is an explicit FriendSlop raw-import exception. Do not force ToonStyle/QuadRetro processing.
- Runtime-facing generated model changes need Unreal import validation and staged standalone verification.
- Avoid broad Git/LFS scans over Unreal binary content.

Independent answer request:
Inspect the repo read-only and identify the likely cause of blue/untextured imported raw Pixal3D assets, the safest texture-preservation repair path, and likely old-model runtime reference surfaces for Chad 1 and enemies.
