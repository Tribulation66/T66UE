This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round03 output/log paths.
- The attached image is the current T66 Main Menu baseline. Use it only to preserve layout regions and content roles, not to copy the flat visual style.

Image prompt:
# Round03 Prompt 05: R.E.P.O.-Style Scrap Utility

Generate one 1920x1080 internal game UI vision-board mockup.

Use the attached screenshot only as a layout and content reference. Preserve the same screen organization:
- Shared top navigation bar across the top with compact icons and ticket value.
- Left social/account panel occupying the left side: profile row, `Local Player`, `Level 1/100`, progress bar, `Level 2`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`, four party slots.
- Center title and CTA stack: title must read `Chadpocalypse`; subtitle must read `If you're not Chad it's over`; buttons must read `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
- Right leaderboard panel: filter icons/buttons, `GLOBAL CHAD RANKING`, `WEEKLY`, `ALL TIME`, `Solo`, `Easy`, `High Score`, `Speed Run`, table headers `RANK`, `NAME`, `SCORE`, and ranking rows.

Style direction: R.E.P.O.-inspired co-op scrap-utility UI visual language, not horror theme copying. Use weird toy-industrial menu chrome: charcoal transparent panels, acid yellow utility labels, cyan-green status lights, chunky plastic/metal capsules, sticker-like warning tabs, uneven riveted corners, low-poly icon cutouts, scanline/dither texture, playful junk-shop tool labels, compact utilitarian buttons. It should feel like goofy co-op equipment UI made from salvage tags and toy machinery.

Distinct title treatment for this image:
- `Chadpocalypse` is straight, not curved or arced.
- Use blocky acid-yellow stencil letters on a black utility backing plate with small cyan indicator dots.
- Add slight misalignment and industrial label texture, but keep it readable.

Visual constraints:
- Keep the current dense three-region main-menu composition recognizable.
- The left social panel can feel like a crew roster terminal; the right leaderboard can feel like a salvage score board.
- Use dark charcoal, acid yellow, cyan-green, black, and dull metal grey; avoid warm brown fantasy UI.
- Internal vision-board mockup only; not a runtime asset.

Hard negatives:
- No curved/arced `Chadpocalypse` title.
- No generic fantasy RPG bevel plates.
- No Megabonk-style medieval grey buttons.
- No candy obstacle-party UI.
- No casino felt/chip UI.
- No grimy shop ledger/receipt UI.
- No cozy mountain sticker UI.
- No text baked into distorted unreadable blobs.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round03\main_menu_reference_05_repo_scrap_utility_cli.png
- If the generated image is first written under CODEX_HOME/generated_images or another Codex image output directory, copy that generated PNG to the exact output path above.
- Create parent directories if needed.
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
