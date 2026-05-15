# Video Manifests

This folder tracks the source-side metadata for frontend videos.

## Files

- `frontend_videos.json`: source copy of the runtime catalog. The runtime copy lives at `RuntimeDependencies/T66/Video/frontend_videos.json`.
- `frontend_video_jobs.json`: one generation job per hero skin/body and companion skin. This is the main file for reviewing coverage, prompt paths, source plates, runtime movie paths, and generation provenance.

## Current Job Statuses

- `ai_accepted`: the original hand-reviewed Arthur clip.
- `ai_generated_ltx2b_fast`: first-pass full-roster RunPod LTX2B clips.

There should be no `placeholder_generated` jobs once the first-pass AI roster pass has been accepted.

## Validation

Run from the project root:

```powershell
python "Video Generation/Scripts/validate_frontend_video_catalog.py"
```

The validator checks roster coverage and verifies that every registered MP4 and poster exists.
