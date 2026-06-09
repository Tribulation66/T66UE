This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Critical uniqueness correction:
- A prior candidate 01 drifted into casino/tabletop objects. Do not repeat that.
- You must create a NEW image by calling the built-in account-backed image generation capability in this worker.
- Do not satisfy this task by searching existing C:\Users\DoPra\.codex\generated_images folders or copying any image created before this worker started at UTC 2026-06-05T10:12:44.0281854Z.
- You may copy only the PNG produced by this worker's own image-generation call. If you cannot identify such a newly created PNG, return IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round03 output/log paths.
- The attached image is the current T66 Main Menu baseline. Use it only to preserve layout regions and content roles, not to copy the flat visual style.

Image prompt:
# Round03 Prompt 01: PEAK-Style Cozy Chunky

Generate one 1920x1080 internal game UI vision-board mockup.

Use the attached screenshot only as a layout and content reference. Preserve the same screen organization:
- Shared top navigation bar across the top with compact icons and ticket value.
- Left social/account panel occupying the left side: profile row, `Local Player`, `Level 1/100`, progress bar, `Level 2`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`, four party slots.
- Center title and CTA stack: title must read `Chadpocalypse`; subtitle must read `If you're not Chad it's over`; buttons must read `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
- Right leaderboard panel: filter icons/buttons, `GLOBAL CHAD RANKING`, `WEEKLY`, `ALL TIME`, `Solo`, `Easy`, `High Score`, `Speed Run`, table headers `RANK`, `NAME`, `SCORE`, and ranking rows.

Style direction: PEAK-inspired friendslop UI visual language. Use the cozy, playful, cooperative-adventure feeling of PEAK's UI: soft rounded black-charcoal plates, off-white puffy hand-lettered text, simple outdoor-adventure sticker icons, moss green, sky blue, coral, and warm cream accents, chunky pill buttons, gentle toy-like shadows, slightly imperfect hand-cut edges, readable camp-sign energy, and clean friendly spacing. Panels should feel like soft rubber patches, camp labels, and playful outdoor gear tags rather than casino, shop, fantasy, or sci-fi UI.

Distinct title treatment for this image:
- `Chadpocalypse` is straight, not curved or arced.
- Use off-white puffy block letters with a thick charcoal shadow, slightly uneven letter heights, like a friendly hand-painted sticker.
- Do not put the title on a giant fantasy plaque.

Visual constraints:
- Keep the current dense three-region main-menu composition recognizable.
- Keep all text separate from the UI plates, as if localizable live text will sit on top later.
- Background can be softly blurred and playful, but UI elements are the focus.
- Internal vision-board mockup only; not a runtime asset.

Hard negatives:
- No curved/arced `Chadpocalypse` title.
- No generic fantasy RPG bevel plates.
- No Megabonk-style medieval grey buttons.
- No warm brown wood UI.
- No casino chips, poker chips, dice, cards, coins, currency piles, roulette, betting table, or any gambling/tabletop casino objects.
- No candy party UI, no grimy shop ledger UI, no industrial hazard terminal UI.
- No text baked into distorted unreadable blobs.
- Do not reuse any prior generated image or prior Round03 output; this candidate must be newly generated and visibly distinct from the Gamble/tabletop prompt.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path, overwriting the rejected candidate 01 if needed:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round03\main_menu_reference_01_peak_cozy_chunky_cli.png
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
