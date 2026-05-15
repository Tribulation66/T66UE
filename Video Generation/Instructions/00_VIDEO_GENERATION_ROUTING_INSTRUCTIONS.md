# Video Generation Routing

## Decision Tree

1. For runtime registration, update `RuntimeDependencies/T66/Video/frontend_videos.json`.
2. For shipped movie files, copy encoded MP4s to `Content/Movies`.
3. For prompts, write one prompt file per target under `Video Generation/Prompts`.
4. For repeatable generation, use a manifest under `Video Generation/Manifests` and a reusable script under `Video Generation/Scripts`.
5. For final gameplay-facing changes, build, stage, and verify the standalone shortcut target.

## Folder Rules

- `Prompts`: canonical per-target prompts and negative prompts.
- `Prompts/Reference`: useful legacy or one-off prompts that are not canonical target paths.
- `Manifests`: generation inputs, intended output paths, runtime registration paths, and job status metadata.
- `Scripts`: reusable local/RunPod helpers only.
- `Runs`: run evidence, logs, status rows, and review copies. Runs are not runtime content.

## Runtime Contract

The frontend video manifest stores logical UI video entries. Runtime code resolves entries to local MP4 files under `Content/Movies`. If a movie is absent or fails to open, the UI must keep using its static fallback.

## Status Contract

- `placeholder_generated`: local draft generated from a poster plate.
- `ai_accepted`: hand-approved AI clip that predates the roster batch.
- `ai_generated_ltx2b_fast`: first-pass full-roster LTX2B clip.

If a higher-quality replacement is accepted, keep the same movie path and update the job manifest with the model/run evidence.
