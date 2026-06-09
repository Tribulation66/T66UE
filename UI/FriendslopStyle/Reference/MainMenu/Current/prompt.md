# FriendslopStyle Main Menu Reference Pass24 Worker Request

Use account-backed built-in imagegen only. Do not use `OPENAI_API_KEY`, OpenAI API scripts, web image URLs, browser screenshots, old generated-image folders, cached candidates, copy/paste from the input image, or manual pixel repair.

Input visual context:

`C:\UE\T66\UI\FriendslopStyle\Archive\ReferenceIterations\MainMenu\Round09\main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png`

Output PNG:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_workers\reference_zorder_icons_coupon\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`

Status file:

`C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_workers\reference_zorder_icons_coupon\operator_final_status.txt`

## Task Contract

Working task: Generate one fresh full-screen FriendslopStyle Main Menu reference PNG at 1672x941 with only the requested right z-order, icon-filter, and coupon-ticket corrections.
Operator: Codex
Validator: Claude when available; otherwise unavailable Validator is reported.
Scope: reference PNG plus worker status/hash record only.
Stop condition: PNG exists at the requested path, dimensions match 1672x941, SHA-256 is recorded, and `operator_final_status.txt` includes `IMAGE_SAVED` or `IMAGE_FAILED`.

## PPF CHECK

Objective: Produce a fresh FriendslopStyle Main Menu reference image based on the current reference while preserving its composition and applying three specific visual corrections.
Proven process: `AGENTS.md` Image generation process plus `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` local worker imagegen contract.
My planned implementation: Use the built-in account-backed imagegen tool from this local Codex worker with the current reference as visual context, save the generated image to the requested workspace path, and perform only mechanical resize/hash/status recording if needed.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: NO
Verification evidence: output PNG path, image dimensions, SHA-256, and final worker status.

## ARTIFACT PARITY GATE

Reference artifact/category: full-screen FriendslopStyle Main Menu reference mockup.
Role: Primary
Required: YES
Planned artifact/path: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass24_workers\reference_zorder_icons_coupon\main_menu_reference_04_foreground_right_panel_icon_filters_coupon_cli.png`
Status: SAME
Evidence: same 1672x941 canvas target, same full-screen Main Menu composition class, fresh built-in imagegen output.

## MECHANISM MANIFEST

Reference/source: current authoritative Main Menu reference plus user correction request.
Required mechanisms:

1. Mechanism: Foreground UI occlusion/compositing
   Required: YES
   Planned implementation: Prompt the right filter panel and right leaderboard panel as solid foreground glossy rubber UI surfaces that occlude the golden statue/background; statue/background must remain behind them.
   Evidence needed: direct visual inspection of output showing no statue or background art drawn over or inside the right panels except normal dark rubber interior texture.

2. Mechanism: Filter label replacement with icon controls
   Required: YES
   Planned implementation: Replace the `GLOBAL`, `SOCIAL`, and `STREAMERS` text in the small top right filter panel with three readable icon-only glyph controls: globe/world, people/friends, and broadcast/streamer.
   Evidence needed: direct visual inspection showing three icon-only controls with no text labels in those controls.

3. Mechanism: Coupon badge readability
   Required: YES
   Planned implementation: Prompt the top bar yellow badge as a classic fair/carnival coupon ticket silhouette with rectangular body, notched/perforated ends, and simple ticket/coupon shape.
   Evidence needed: direct visual inspection showing a ticket/coupon silhouette rather than an abstract yellow blob.

4. Mechanism: Composition preservation
   Required: YES
   Planned implementation: Keep the left social panel, top bar layout, central title/subtitle/CTA stack, golden rubber statue, star/fire background, equal-width side panel concept, colors, lighting, and reference labels unchanged except for the three requested corrections.
   Evidence needed: direct visual inspection against the current reference.

Cheapest wrong result: a near-copy where the statue bleeds over the right panels, text remains on the three filter toggles, or the top badge stays blob-like.
Discriminator: foreground right panels visibly occlude background/statue; the three top filter controls are icon-only; the coupon badge reads as a notched ticket.

