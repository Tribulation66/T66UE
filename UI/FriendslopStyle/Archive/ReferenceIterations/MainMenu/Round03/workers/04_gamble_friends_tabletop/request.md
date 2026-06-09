This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, or browser screenshots.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round03 output/log paths.
- The attached image is the current T66 Main Menu baseline. Use it only to preserve layout regions and content roles, not to copy the flat visual style.

Image prompt:
# Round03 Prompt 04: Gamble With Your Friends-Style Tabletop Social

Generate one 1920x1080 internal game UI vision-board mockup.

Use the attached screenshot only as a layout and content reference. Preserve the same screen organization:
- Shared top navigation bar across the top with compact icons and ticket value.
- Left social/account panel occupying the left side: profile row, `Local Player`, `Level 1/100`, progress bar, `Level 2`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`, four party slots.
- Center title and CTA stack: title must read `Chadpocalypse`; subtitle must read `If you're not Chad it's over`; buttons must read `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`.
- Right leaderboard panel: filter icons/buttons, `GLOBAL CHAD RANKING`, `WEEKLY`, `ALL TIME`, `Solo`, `Easy`, `High Score`, `Speed Run`, table headers `RANK`, `NAME`, `SCORE`, and ranking rows.

Style direction: Gamble With Your Friends-inspired social gambling UI visual language, but keep it goofy and friendly. Use tabletop casino-night UI shapes: dark green felt panels, cream card-stock labels, red/blue/yellow poker-chip accent buttons, rounded playing-card tabs, dice-pip icon buttons, small gold trim lines, glossy plastic chip highlights, friendly table-game typography, compact social-lobby density. The menu should feel like friends gathered around a board-game/casino table, not a serious real-money casino.

Distinct title treatment for this image:
- `Chadpocalypse` is straight, not curved or arced.
- Use cream-and-gold slab display letters on a green felt casino-table plaque, with small chip accents on the ends.
- Keep it horizontal and compact, no marquee arch.

Visual constraints:
- Keep the current dense three-region main-menu composition recognizable.
- Buttons can resemble cards/chips, but the three CTA labels must remain in the same center stack order.
- The left party slots can read as card seats/chip trays; the right leaderboard can read as a scorecard table.
- Internal vision-board mockup only; not a runtime asset.

Hard negatives:
- No curved/arced `Chadpocalypse` title.
- No generic fantasy RPG bevel plates.
- No Megabonk-style medieval grey buttons.
- No candy obstacle-party UI.
- No grimy shop ledger UI.
- No cozy mountain sticker UI.
- No industrial hazard terminal UI.
- No text baked into distorted unreadable blobs.


Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round03\main_menu_reference_04_gamble_friends_tabletop_cli.png
- If the generated image is first written under CODEX_HOME/generated_images or another Codex image output directory, copy that generated PNG to the exact output path above.
- Create parent directories if needed.
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE
