# Main Menu Video Background Process

This doc preserves the useful parts of the old `Video Generation` workspace without keeping a separate root folder or task-specific scripts.

## Goal

Create a looping animated main-menu background from the current gold idol altar plate. The idol, altar, and camera must stay locked. Only environmental effects should move:

- stars softly twinkle in place
- the fire ring behaves like slow solar plasma
- gold highlights shimmer subtly
- no water, UI, text, camera pan, zoom, idol movement, face morphing, altar deformation, or geometry drift

## Current Paths

Current static background plate:

```text
C:\UE\T66\SourceAssets\UI\Reference\Screens\MainMenu\ScreenArt\mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_1920.png
```

Final cooked movie target:

```text
C:\UE\T66\Content\Movies\MainMenuBackground.mp4
```

Keep a source copy beside the main-menu source art if this feature resumes. Do not store API keys or provider credentials in repo docs.

## Recommended Generation Route

Try a modern web image-to-video path first with the same image as start and end frame:

- 1080p
- 5 seconds
- 16:9
- native audio off
- one output per generation while testing

Use the Motion Brush/static-mask route only if the web model moves the idol or altar too much.

## Prompt

```text
Locked camera cinematic game main menu background. The golden idol statue and gold altar remain perfectly still, centered, and unchanged. The composition stays identical to the input image. Only the environment animates: tiny stars softly twinkle in place across the black sky, and the fiery eclipse ring behind the idol flickers like solar plasma with slow flame licking and glowing corona motion. Subtle warm gold glints shimmer on the polished gold surfaces. Slow elegant ambient motion, seamless loop feeling.
```

Negative prompt, if supported:

```text
camera movement, zoom, pan, statue movement, face morphing, altar deformation, changing geometry, melting gold, water, text, UI, logo, smoke covering idol, glitch artifacts, distortion, blur, extra objects
```

## Unreal Integration Notes

After a final MP4 is chosen:

1. Copy it to `Content\Movies\MainMenuBackground.mp4`.
2. Add a main-menu media player path similar to the existing hero preview media code.
3. Use a transient `UMediaPlayer`, `UMediaTexture`, and `UFileMediaSource`.
4. Set looping true and play on open.
5. Use the media texture as the main-menu background brush.
6. Keep the static PNG as fallback if video open fails.

Existing media-code reference: `Source\T66\UI\Screens\HeroSelection\T66HeroSelectionPreviewController.cpp`.

The old Kling API notes were a dated balance/provider snapshot, not durable project policy. Recheck provider API support before rebuilding any automation.
