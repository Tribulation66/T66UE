This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Critical isolation rule:
- You must create a NEW image by calling the built-in account-backed image generation capability in this worker.
- Do not satisfy this task by searching existing C:\Users\DoPra\.codex\generated_images folders or copying any image created before this worker started at UTC 2026-06-05T11:33:16.0268406Z.
- You may copy only the PNG produced by this worker's own image-generation call. If you cannot identify such a newly created PNG, return IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round04 output/log paths.
- The attached image is the current T66 Main Menu baseline. Use it only to preserve layout regions and content roles, not to copy the flat visual style.
- Do not reuse Round03 chrome or make a recolor of the Fall Guys bouncy candidate.

Image prompt:
# Round04 Prompt 02: Post-Apocalyptic Inflatable

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
- Post-apocalyptic survival atmosphere, but the UI is still clean inflatable rubber/plastic.
- Think rubber safety gear, airbag-like survival pads, sealed hazard bumpers, inflatable road barrier shapes, and clean plastic warning labels.
- Dust, scratches, warning stripes, hazard marks, tape marks, and scuffs are printed decals on glossy rubber, not real rust or gritty metal.
- Palette: smoky charcoal, dusty olive, faded safety yellow, hazard orange, desaturated teal, cream labels.
- UI should feel like a goofy survival kit made of inflated rubber.

Distinct title treatment:
- `Chadpocalypse` is straight, not arced.
- Use molded rubber stencil letters on an inflated safety-label backing plate, with faded hazard stripe decals.
- No rainbow bubble letters.

Hard negatives:
- No Fall Guys characters.
- No Round03 candy-party cyan/magenta/yellow/purple palette.
- No bubble-letter Fall Guys title treatment.
- No real rusted metal, corroded panels, gritty scrap, low-fi grime, or matte dirt as the base UI material.
- No cloth, leather, paper, wood, felt, stone, or fantasy bevel plates.
- No sharp hard industrial terminal panels.
- Do not reuse any prior generated image or prior Round03/Round04 output.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round04\main_menu_reference_02_post_apocalyptic_inflatable_cli.png
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
