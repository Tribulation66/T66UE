# Video Prompts

This folder stores editable prompt text for frontend video generation.

## Canonical Layout

- `HeroSelection/Heroes/<HeroID>/<SkinID>/<BodyType>.md`
- `HeroSelection/Companions/<CompanionID>/<SkinID>.md`
- `MainMenu/Background.md`

The canonical prompt paths are referenced by `Video Generation/Manifests/frontend_video_jobs.json`.

## Reference Prompts

`Reference` contains older one-off prompts that are still useful for context, especially the first main-menu and Arthur LTX2B pass. These files are not the canonical prompt paths for roster automation.

## Editing Rule

When polishing a clip, update the canonical prompt for that target and keep the runtime MP4 path stable. If a prompt is only experimental, place it under `Reference` or a dated run folder instead of replacing the canonical prompt.
