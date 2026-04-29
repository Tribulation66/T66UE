# Main Menu Video Generation Handoff

Last updated: 2026-04-29

## Goal

Create a looping animated main menu background from the current gold idol altar plate.

The idol, altar, and camera must stay locked. Only the stars and fiery eclipse should move:

- Stars softly twinkle in place.
- The fire ring behaves like solar plasma.
- Gold highlights may shimmer subtly.
- No water.
- No UI or text baked into the video.
- No camera pan, zoom, idol movement, face morphing, altar deformation, or geometry drift.

## Current Repo State

Current static background plate:

`C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\main_menu_newmm_base_1920.png`

4K upscaled master:

`C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\main_menu_newmm_gold_altar_upscaled_3840.png`

Foreground occluder:

`C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\main_menu_newmm_foreground_occluder.png`

The occluder is intentionally fully transparent now. The new generated plate is a complete image, so the old foreground cover layer would reintroduce the earlier collage problem.

The image was also copied for easy browser upload:

`C:\Users\DoPra\Desktop\T66_Kling_Upload\t66_main_menu_kling_start_end.png`

## Main Menu Code State

The old manual animated effect layers were removed from the main menu path. The main menu now uses the plain background plate path through `BuildMainMenuBackgroundWidget()`.

`ST66AnimatedBackground` was left in the project because other screens may still use it.

Previous build check:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Result: succeeded / target up to date.

## Kling Account/API State

The local machine has these user environment variables set:

- `KLING_ACCESS_KEY`
- `KLING_SECRET_KEY`

Do not paste these keys into chat and do not write them into repo files.

Auth was verified against Kling API by requesting a fake image-to-video task id. Both hosts returned a normal authenticated API error:

- `https://api-singapore.klingai.com`
- `https://api.klingai.com`

The important blocker is API balance. The latest submitted `kling-v1` Motion Brush job failed with:

```json
{
  "code": 1102,
  "message": "Account balance not enough"
}
```

Job record:

`C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\VideoGen\kling_job_static_mask.json`

## Model Decision

There are two realistic routes.

### Route A: Kling Video 3.0, Best Visual Quality

Use this first if the goal is highest visual quality.

Pros:

- Latest web model shown in the UI.
- Better image quality and prompt adherence.
- Supports start/end frame workflow in the web UI.

Cons:

- Motion Brush / Static Brush is not clearly exposed through the official API for Video 3.0.
- The idol may drift even with strict prompting.

Recommended Video 3.0 attempt:

- Use the same image as start frame and end frame.
- 1080p, 5 seconds, 16:9.
- Native Audio off.
- Multi-Shot off.
- Generate a few variants and pick the one with least idol movement.

### Route B: Kling v1 Motion Brush, Best Control

Use this if Video 3.0 moves the idol too much.

Pros:

- Official API recognizes Motion Brush style mask parameters.
- Static mask can freeze the idol, altar, eclipse center, and bottom edge.

Cons:

- Older model.
- Lower visual quality than Video 3.0.
- Needs API balance.

API findings:

- `kling-v1-6` with `static_mask` returned: `kling-v1-6 is not supported with motion brush`.
- `kling-v1` reached billing, so it is the viable Motion Brush API path.
- Both `kling-v1` `pro` and `std` failed only because API balance was not enough.

## Useful Links

- Kling app: https://kling.ai/app/video/new
- Kling developer/API entry: https://kling.ai/dev
- Kling API docs entry: https://kling.ai/document-api
- Kling start/end frames guide: https://kling.ai/quickstart/ai-video-start-end-frames
- Kling Motion Brush guide: https://kling.ai/quickstart/how-to-animate-image-parts
- Kling Motion Control guide: https://kling.ai/quickstart/motion-control-user-guide

Important note: the `Motion Control` tab in the Kling web app is not the right tool for this background. It is for character action transfer from a reference video to a character image. It is not the same as Motion Brush / Static Brush.

## Next Time

1. Decide whether to try Video 3.0 quality first or v1 Motion Brush control first.
2. If using API, add API balance in Kling developer console first.
3. Generate a 5 second test video.
4. Review for idol/camera drift.
5. Save the chosen MP4 into the repo.
6. Integrate it into the main menu with the PNG still available as fallback.
