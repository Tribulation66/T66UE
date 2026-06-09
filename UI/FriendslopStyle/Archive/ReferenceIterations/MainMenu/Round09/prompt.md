# FriendslopStyle Main Menu Reference Pass23 Worker Request

Use account-backed built-in imagegen only. Do not use OPENAI_API_KEY, OpenAI API scripts, web image URLs, browser screenshots, old generated-image folders, cached candidates, or manual painting/clone/inpaint/blur/smear/procedural synthesis as substitutes for imagegen.

Attached image/context path:
`C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round08\main_menu_reference_02_rubber_statue_title_fixed_cli.png`

Goal: create the next full-screen Main Menu reference image by making ONLY this right-side layout change:

1. Keep the overall image style, top bar, left social panel, center title/subtitle, center red/dark CTA buttons, golden rubber statue background, star field, colors, and text style the same as the attached current reference.
2. Change the right side so the right leaderboard column/panel is the SAME WIDTH as the left social panel.
3. Remove the current vertical three-button filter rail from the left side of the leaderboard.
4. Put the three leaderboard filter toggles as a separate small rubber panel ABOVE the leaderboard panel. This separate panel contains the three options: GLOBAL, SOCIAL, STREAMERS. It should be visually integrated with the Friendslop rubber style, but separate from the leaderboard panel below it.
5. Make the leaderboard panel below that toggle panel shorter vertically to make room for the separate toggle panel above it.
6. The leaderboard panel below should be wider than the old one, matching the left-panel width, and should retain the header, weekly/all-time buttons, dropdowns, metrics, rank/name/score header, and local row layout in the same general style.
7. Do not change the background zoom, title wording, CTA labels, left panel text, topbar text, or other composition except what is needed to make the right panel wider and move the filter toggles above it.

Important: this is a reference mockup, not a runtime UI asset sheet. It may contain text as part of the full-screen reference, but the runtime implementation later will keep text/data/icons live. Do not crop from the input image into an output file; use the input only as visual context for imagegen.

Output requirements:

- Save the generated full-screen PNG to:
  `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass23_workers\reference_equal_width_right_panel\main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png`
- Save `operator_final_status.txt` with `IMAGE_SAVED` or `IMAGE_FAILED`, final status, source generated image path, output path, SHA-256, token count when exposed, and a note that account-backed built-in imagegen was used.
- Save `sha256.txt` with the final output SHA-256.
- Final message must include `IMAGE_SAVED` or `IMAGE_FAILED`, output PNG path, SHA-256 when available, and token count when available.
