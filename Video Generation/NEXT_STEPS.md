# Next Steps

## Fastest Recommended Path

Try Kling Video 3.0 first from the web UI.

1. Open https://kling.ai/app/video/new
2. Go to `Video Generation`.
3. Select `VIDEO 3.0`.
4. Upload the same image as both start and end frame:

   `C:\Users\DoPra\Desktop\T66_Kling_Upload\t66_main_menu_kling_start_end.png`

5. Use:

   - 1080p
   - 5s
   - 16:9
   - Native Audio off
   - Multi-Shot off
   - 1 output per generation while testing

6. Use the Video 3.0 prompt from `PROMPTS.md`.
7. Generate 3 to 5 candidates.
8. Pick the one where the idol and altar move the least.
9. Download the MP4 into:

   `C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\VideoGen\`

Suggested name:

`main_menu_kling_v3_candidate_01.mp4`

## If Video 3.0 Drifts Too Much

Use the API Motion Brush fallback.

1. Open https://kling.ai/dev
2. Add/recharge API balance.
3. Confirm the local env vars still exist:

   ```powershell
   [Environment]::GetEnvironmentVariable('KLING_ACCESS_KEY','User')
   [Environment]::GetEnvironmentVariable('KLING_SECRET_KEY','User')
   ```

4. Regenerate the mask if needed:

   ```powershell
   python '.\Video Generation\scripts\regenerate_kling_masks.py'
   ```

5. Submit the Motion Brush job:

   ```powershell
   python '.\Video Generation\scripts\submit_kling_motion_brush.py'
   ```

6. Poll/download the result. The submit script records the task response in:

   `C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\VideoGen\kling_job_static_mask.json`

## Unreal Integration After MP4 Is Chosen

Target movie path:

`C:\UE\T66\Content\Movies\MainMenuBackground.mp4`

Keep a source copy here too:

`C:\UE\T66\SourceAssets\UI\MasterLibrary\ScreenArt\MainMenu\NewMM\VideoGen\main_menu_final_loop.mp4`

Implementation plan:

1. Copy the chosen final MP4 to `Content\Movies\MainMenuBackground.mp4`.
2. Add a main menu media player path similar to the existing hero preview code.
3. Use a transient `UMediaPlayer`, `UMediaTexture`, and `UFileMediaSource`.
4. Set looping true and play on open.
5. Use the media texture as the main menu background brush.
6. Keep `main_menu_newmm_base_1920.png` as fallback if video open fails.

Existing media code reference:

`C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.cpp`
