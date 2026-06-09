This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Critical isolation rule:
- You must create a NEW image by calling the built-in account-backed image generation capability in this worker.
- Do not satisfy this task by searching existing C:\Users\DoPra\.codex\generated_images folders or copying any image created before this worker started at UTC 2026-06-05T11:33:16.1021838Z.
- You may copy only the PNG produced by this worker's own image-generation call. If you cannot identify such a newly created PNG, return IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round04 output/log paths.
- The attached image is the current T66 Main Menu baseline. Use it only to preserve layout regions and content roles, not to copy the flat visual style.
- Do not reuse Round03 chrome or make a recolor of the Fall Guys bouncy candidate.

Image prompt:
# Round04 Prompt 04: Dungeon Slime Rubber

Generate one 1920x1080 internal game UI vision-board mockup.

Use the attached screenshot only as a layout and content reference. Preserve the same screen organization:
- Shared top navigation bar across the top with compact icons and ticket value.
- Left social/account panel occupying the left side: profile row, `Local Player`, `Level 1/100`, progress bar, `Level 2`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`, four party slots.
- Center title and CTA stack: title must read `Chadpocalypse`; subtitle must read `If you're not Chad it's over`; buttons must read `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
- Right leaderboard panel: filter icons/buttons, `GLOBAL CHAD RANKING`, `WEEKLY`, `ALL TIME`, `Solo`, `Easy`, `High Score`, `Speed Run`, table headers `RANK`, `NAME`, `SCORE`, and ranking rows.

Shared identity that must dominate:
- Bouncy glossy rubber/plastic UI, soft inflated edges, pill/circle geometry, squash-and-stretch feeling.
- Surfaces look HD, clean, tactile, toy-like, and touchable.
- Button plates should imply click rebound and jiggle physics.

Unique theme direction:
- Dungeon atmosphere reimagined as squishy rubber and glossy slime, not stone/wood fantasy UI.
- Panels are inflated obsidian-rubber blobs, rounded slime capsules, gel potion tabs, soft dungeon-door silhouettes, and squishy monster-mouth button shapes.
- Use glowing slime seams, potion-orb icons, soft lava/poison gel accents, and round rubber studs instead of metal rivets.
- Palette: glossy black-purple, slime green, potion teal, lava orange, warm torch amber, cream text.
- The UI should feel like a toy dungeon made of polished rubber and jelly.

Distinct title treatment:
- `Chadpocalypse` is straight, not arced.
- Use thick molded rubber letters with slime highlights and small glossy gel bubbles around the baseline.
- No rainbow bubble letters.

Hard negatives:
- No Fall Guys characters.
- No Round03 candy-party cyan/magenta/yellow/purple palette.
- No bubble-letter Fall Guys title treatment.
- No stone slabs, wood boards, medieval metal, parchment, leather, or generic fantasy RPG bevel plates.
- No gritty dungeon dirt, grain, rust, or matte realism.
- No sharp hard panels.
- Do not reuse any prior generated image or prior Round03/Round04 output.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round04\main_menu_reference_04_dungeon_slime_rubber_cli.png
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
