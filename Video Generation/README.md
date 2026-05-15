# T66 Video Generation

This folder owns the source workflow for animated frontend videos. The game runtime does not call RunPod. RunPod creates candidate MP4s; approved clips are copied into `Content/Movies` and registered through the frontend video manifest.

## Current State

- Active runtime catalog: `RuntimeDependencies/T66/Video/frontend_videos.json`
- Source catalog copy: `Video Generation/Manifests/frontend_videos.json`
- Job manifest: `Video Generation/Manifests/frontend_video_jobs.json`
- Current coverage: `48` hero-selection hero clips, `48` companion clips, and `1` main menu clip.
- Placeholder status: `0` placeholder jobs remain in the current job manifest.
- Current main-menu background run: `Video Generation/Runs/MainMenu_NoComet_SourceLoop_20260514`
- Full-roster AI run: `Video Generation/Runs/FrontendRoster_LTX2B_20260513`
- Current character-video model: `Lightricks/LTX-Video ltxv-2b-0.9.8-distilled`

These are functional first-pass AI loops for runtime review. Final polishing should replace MP4s in-place under `Content/Movies/Frontend` without changing the catalog unless a new character, skin, body type, or UI slot is added.

## Folder Map

- `Instructions`: routing rules and runtime contract for this folder.
- `Manifests`: generated source catalog and per-video job metadata.
- `Prompts`: editable prompt sources, organized by UI target.
- `Prompts/MainMenu`: current prompt direction for the main-menu background slot.
- `Prompts/Reference`: legacy or one-off prompts that are still useful as source notes but are not the canonical per-target prompt path.
- `Runs`: local evidence from generation and smoke runs. These are not runtime inputs.
- `Scripts`: reusable local and RunPod helpers.

## Runtime Targets

- Main menu background: `Content/Movies/Frontend/MainMenu/MainMenuBackground.mp4`
- Hero-selection hero clips: `Content/Movies/Frontend/HeroSelection/Heroes/<HeroID>/<SkinID>/<BodyType>.mp4`
- Hero-selection companion clips: `Content/Movies/Frontend/HeroSelection/Companions/<CompanionID>/<SkinID>.mp4`
- Runtime manifest: `RuntimeDependencies/T66/Video/frontend_videos.json`

## Catalog Pipeline

The frontend video catalog is intentionally separate from the hero and companion gameplay data tables. Gameplay data owns stable character metadata; this folder owns prompt/source media, placeholder loops, and RunPod replacement workflow.

Run from the project root:

```powershell
$env:T66_PYTHON_PACKAGE_PATH = "$env:TEMP\t66_imageio_ffmpeg"
python "Video Generation/Scripts/build_frontend_video_catalog.py" --generate-videos --force --jobs 2
python "Video Generation/Scripts/validate_frontend_video_catalog.py"
```

Use `--replace-ai-movies` only when intentionally discarding current AI clips and rebuilding placeholder loops.

The build script writes:

- Runtime catalog: `RuntimeDependencies/T66/Video/frontend_videos.json`
- Source catalog copy: `Video Generation/Manifests/frontend_videos.json`
- Generation job list: `Video Generation/Manifests/frontend_video_jobs.json`
- One prompt per target under `Video Generation/Prompts/HeroSelection`
- Poster plates under `RuntimeDependencies/T66/Video/Posters`

After AI clips exist, the builder preserves jobs marked as AI-generated and will
not overwrite their MP4s with placeholders unless `--replace-ai-movies` is
explicitly supplied.

## Roster AI Pass

The first full-roster AI replacement pass used `run_ltx2b_frontend_roster_batch.py`
on the RunPod A40. It reads `frontend_video_jobs.json`, conditions each job on
its poster plate, and encodes H.264 MP4s back into the same runtime-relative
movie paths.

Current run evidence:

- Run log: `Video Generation/Runs/FrontendRoster_LTX2B_20260513/run.log`
- Status rows: `Video Generation/Runs/FrontendRoster_LTX2B_20260513/frontend_roster_status.jsonl`
- Model: `Lightricks/LTX-Video ltxv-2b-0.9.8-distilled`
- First-pass frame count: `49`

The encoded outputs were copied over the existing `Content/Movies/Frontend/...`
paths. The catalog did not need to change because the runtime paths stayed
stable.

## Review And Replacement Loop

1. Pick the target in `frontend_video_jobs.json`.
2. Read its prompt under `Video Generation/Prompts`.
3. Generate a replacement on RunPod, preserving H.264, yuv420p, 30 fps, and no audio.
4. Copy the approved MP4 over the matching `Content/Movies/Frontend/...` file.
5. Update the job status/source fields if the model, prompt, or run changes.
6. Run `validate_frontend_video_catalog.py`.
7. For playable changes, run `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development` and verify the taskbar shortcut.

## RunPod Boundary

Use `Model Generation/LOCAL_ACCESS.env` or a local-only note for pod secrets. Do not paste secrets into repo docs. The current pod smoke target is SSH-driven setup first; serverless/API automation can come after the runtime path is accepted.

AI-generated clips should replace the existing MP4 path for a job, not add a second ad hoc filename. This keeps runtime code and review notes stable while quality improves.

## Approved Movie Encoding

- MP4 container
- H.264 video
- yuv420p pixel format
- 30 fps unless a final clip needs another rate
- `+faststart`
- no audio for frontend loops unless explicitly needed
