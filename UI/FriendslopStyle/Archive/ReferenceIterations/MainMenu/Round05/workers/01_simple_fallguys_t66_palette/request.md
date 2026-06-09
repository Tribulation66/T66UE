This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Critical isolation rule:
- You must create a NEW image by calling the built-in account-backed image generation capability in this worker.
- Do not satisfy this task by searching existing C:\Users\DoPra\.codex\generated_images folders or copying any image created before this worker started at UTC 2026-06-05T11:54:22.5723944Z.
- You may copy only the PNG produced by this worker's own image-generation call. If you cannot identify such a newly created PNG, return IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round05 output/log paths.
- Two images are attached: current UI layout screenshot and original background art. Use roles from the prompt.

Image prompt:
# Round05 Prompt 01: Simple Bouncy UI With Current T66 Palette

Generate one 1920x1080 internal game UI vision-board mockup.

Input image roles:
- Attached background art: use as the primary background composition. Preserve the starfield, golden statue head, fiery eclipse/ring behind the head, and reflective dark water mood. The background already contains `CHADPOCALYPSE` and `If you're not Chad it's over`; keep that as the title/subtitle source and do not add a second title.
- Attached current main-menu screenshot: use only as a layout/content reference for the UI regions and interactive controls.

Preserve the recognizable current main-menu organization:
- Shared top navigation bar across the top with compact icon buttons and ticket value.
- Left social/account panel: profile row, `Local Player`, level/progress, search field, online/offline groups, party slots.
- Center CTA stack below the existing background title/subtitle: `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
- Right leaderboard panel: filter buttons, `GLOBAL CHAD RANKING`, weekly/all-time tabs, score filters, ranking rows.

Style direction:
- Use simple Fall Guys-like UI shape language, but not Fall Guys IP.
- Translate that into generic bouncy UI: clean rounded rectangles, chunky pill buttons, circular icon buttons, soft thick borders, smooth inflated bevels, subtle glossy rubber/plastic highlights, playful touchable controls.
- Keep the UI elements simple. Do not add complex themed chrome, props, badges, parchment, mechanical parts, dungeon details, casino objects, cloth/leather texture, paper texture, or rust texture.
- The buttons and panels should feel soft, bouncy, and clickable, with minimal decoration.

Current T66 palette:
- Dominant: near-black `#08080C`, dark fill `#17171E`, neutral border `#4A4A55`, soft text `#DCD7EB`, primary text `#F0F0F5`.
- Selected/primary action accent: red `#E1232D` and `#FF505F`.
- Hover/ready accent: green `#1FB358` and `#4FD088`.
- Small accents only: cyan `#3CDCF0` and yellow ticket accent.
- Do not use a rainbow, candy, or pastel palette. The overall UI should still feel like T66's current dark/red/green scheme.

Text fidelity priority:
- Must be legible if possible: `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`, `GLOBAL CHAD RANKING`.
- Small leaderboard row names/scores can be approximate; this is a visual direction mockup, not production text.

Hard negatives:
- No Fall Guys characters, no bean characters, no mascots, no costumes, no Mediatonic/Epic logos.
- No rainbow candy palette.
- No duplicate `Chadpocalypse` title; keep the background title/subtitle only.
- No complex themed Round04 chrome.
- No gritty grain, low-fi texture, real rust, stone, wood, leather, cloth, paper, casino felt, or fantasy bevel plates.
- No arbitrary new layout; preserve the current main-menu regions.
- Do not reuse any prior generated image or prior Round03/Round04/Round05 output.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round05\main_menu_reference_01_simple_fallguys_t66_palette_cli.png
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
