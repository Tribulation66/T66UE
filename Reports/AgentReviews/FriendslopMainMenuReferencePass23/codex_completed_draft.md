# Codex Completed Draft - Main Menu Reference Pass23

Task completed: regenerated the FriendslopStyle Main Menu reference image with only the requested right-panel structural change, then prepared the recap/solutions for the next runtime iteration.

Generated reference:
`C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png`

Worker record:
`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass23_workers\reference_equal_width_right_panel`

Worker status: IMAGE_SAVED. Output SHA-256: C6EB25734BEADE73FDD6CA740BD927DE5E5AEE809147752F89875AF70BFF1374. Built-in account-backed imagegen was used; imagegen token count was unavailable/not exposed. No API/web/old-folder/manual fallback.

Archive: prior Current reference archived under `C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round08`.

Current docs updated: Current README, manifest, prompt, crops, Main Menu doc reference paths, FriendslopStyle instructions, and asset registry now point to the pass23 reference.

Visual check: the new reference keeps the same topbar/left/center/background composition, makes the right column the same width class as the left panel, removes the vertical right filter rail, adds a separate top filter panel containing GLOBAL/SOCIAL/STREAMERS, and shortens the leaderboard panel below.

Recap and solutions to tell user:

1. Cropping/gray bands are the number-one problem. They show on weekly/all-time, side panels, center CTAs, and rows. Solution for next iteration: treat this as a systematic packaging/slicing/alpha edge issue; validate source plates for transparent padding/edge bleed, stop cropping too tight, add safe transparent padding/bleed if needed, rederive 9-slice margins per plate, and run min/normal/wide stretch tests before runtime copy.
2. Shape mismatch remains. Topbar corners and leaderboard controls are too pill-shaped; center CTAs overcorrected too square. Solution: stop global square/pill language and specify per-family corner radii from the reference; generate family sheets with target aspect/shape examples and reject outputs outside that radius class.
3. Dropdowns have a runtime wrapper problem. Current dropdowns sit inside an outer square frame, while the reference shows only the dropdown button. Solution: remove/replace the outer wrapper in the runtime assembly so the dropdown chrome itself is the visible control.
4. Leaderboard rows and friend sub-panels are cramped. Solution: after crop/slice fix, increase internal padding and reduce border pressure inside rows/sub-panels; verify row content fit in the dump and capture.
5. Left/right side panel borders are too thick. Solution: new equal-width reference lets one shared side-panel frame family be generated; prompt it with thin simple rubber outline and dark interior, then use the same panel treatment on both sides.
6. Background zoom/outline/bending is still far off. Solution: lower zoom/crop pressure in the background prompt for the next iteration and preserve more space around the statue; treat subtitle bend/outline separately as title/subtitle treatment, not as a panel issue.
7. Subtitle is wrong: `If you're not Chad it's over` is not bent/outlined like reference. Solution: either generate a clean subtitle/title lockup asset or improve live Slate layered text arc/outline; keep it separate from CTA/button generation.
8. `Solobro` was called fine by the user; no action needed there.

Process close: PPF process matches the declared FriendslopStyle reference-generation process. No runtime iteration was run in this pass.
