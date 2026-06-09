Working task:
Operator: Codex
Validator: Claude
Scope: Generate only the FriendslopStyle Main Menu Background family PNGs requested by the worker prompt. Use account-backed built-in image generation only. Use the attached/local approved reference only as visual context. Do not use OPENAI_API_KEY, API scripts, web image URLs, browser screenshots, old generated folders, cached candidates, manual painting, clone/inpaint/blur/smear repair, procedural synthesis, or reference-pixel cropping. Mechanical crop/resize/copy of this worker's own generated image is allowed. No UI/runtime/code/import/build changes.
Stop condition: Both requested opaque PNG files exist at 1920x1080, contain background art only with no UI/text, and SHA-256 hashes plus Codex token count are reported, or IMAGE_FAILED is reported with the blocker.

Original user request:
# FriendslopStyle Main Menu Pass20 Worker: Background

You are a separate local Codex CLI worker for a bounded T66 image-generation subtask.

Use account-backed built-in image generation only. Do not use `OPENAI_API_KEY`, OpenAI API scripts, web image URLs, browser screenshots, old generated-image folders, cached candidates, manual painting, clone/inpaint/blur/smear repair, or procedural image synthesis as substitutes for imagegen.

Input visual context image attached with `--image`:
`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_01_current_capture_stronger_rubber_cli.png`

Goal:
Generate a no-text, no-UI Main Menu background art PNG matching the approved reference's starfield, fiery orange halo, golden statue/bust, pyramid/base, and water/reflection mood. The output is background art only. Do not include top bar, side panels, buttons, title, subtitle, skull icons, leaderboard, player names, labels, numbers, UI controls, or any full-screen screenshot with UI.

Required final output files:
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass20_workers\background\background_family_worker_output.png`
- `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass20_workers\background\mainmenu_screen_art_mainmenu_newmm_rubbery_friendslop_pass20_1920_worker_output.png`

Target package size:
- background: 1920x1080

Use normal opaque PNG output for the background. Do not make the background transparent. Cropping/resizing the worker's own generated image is allowed. Do not crop pixels from the reference.

Final response must include:
- `IMAGE_SAVED` or `IMAGE_FAILED`
- output PNG paths
- SHA-256 for each saved PNG when available
- Codex token count when available

