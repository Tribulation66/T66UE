This is a separate fresh Codex CLI worker for a bounded T66 image-generation subtask.

Critical isolation rule:
- You must create a NEW image by calling the built-in account-backed image generation capability in this worker.
- Do not satisfy this task by searching existing C:\Users\DoPra\.codex\generated_images folders or copying any image created before this worker started at UTC 2026-06-05T12:14:05.3127516Z.
- You may copy only the PNG produced by this worker's own image-generation call. If you cannot identify such a newly created PNG, return IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE.

Process override for this worker:
- Do not invoke Claude, validator review helpers, Operator/Validator scripts, or create AgentReview/decision-block artifacts.
- Work only on this one image-generation request and the exact output path below.
- Use account-backed built-in Codex image generation only. Do not use OPENAI_API_KEY, OpenAI API scripts, web search, external image URLs, browser screenshots, or old generated image folders.
- Generate exactly one raster image. Do not edit source code, Unreal assets, Content assets, AGENTS.md, or any files outside the Round06 output/log paths.
- Two images are attached: fresh current UI layout screenshot and original background art. Use roles from the prompt.

Image prompt:
# Round06 Prompt 01: Fresh Current Capture, Stronger Rubber UI

Generate one 1920x1080 internal game UI vision-board mockup.

This is a NEW generated mockup, not a literal paintover/edit of the screenshots. Use the attached images as references for layout, content, composition, and mood.

Input image roles:
- Attached fresh current main-menu screenshot: primary authority for layout, visible screen content, regions, controls, friend online/offline state, top-bar contents, and relative placement.
- Attached original background art: primary authority for background mood and title direction. Preserve the starfield, golden statue head, fiery eclipse/ring behind the head, and reflective dark water mood. Use `CHADPOCALYPSE` as the title wording from this background, not `TRIBULATION 66` and not `T66`.

Preserve the current main-menu organization from the fresh screenshot:
- Top navigation bar across the full top: compact gear icon button, globe/language icon button, `ACCOUNT`, selected `HOME`, `POWER UP`, `ACHIEVEMENTS`, ticket value `53`, and power icon button.
- Left social/account panel: player profile `Solobro`, level/progress, search field, `ONLINE (1)` group with one online friend row and `INVITE`, `OFFLINE (4)` group with offline friend rows, and bottom `PARTY` slots.
- Center composition: title/subtitle above the statue/fire ring, then only two CTA buttons: `ENTER TRIBULATION` and `LOAD GAME`.
- Right leaderboard panel: vertical category icon buttons, `GLOBAL CHAD RANKING`, `WEEKLY` / `ALL TIME`, dropdowns `SOLO` and `EASY`, metric toggles `High Score` and `Speed Run`, and a local ranking row.

Hard content exclusions:
- Do not show `DAILY DESCENT`.
- Do not show `MINIGAMES` or `MINI GAMES`.
- Do not show old/stale menu categories or extra center buttons.
- Do not use any May 12 baseline screenshot content or any previous Round03/Round04/Round05 generated image as source content.

Style direction:
- Use a bouncy rubber/plastic UI identity inspired by simple party-game shape language, but without copying any specific game IP.
- Make the rubber feel obvious: inflated pill buttons, soft rounded panels, thick squishy bevels, high-quality specular highlights, subtle elastic edge wobble, soft contact shadows, and touchable silicone/plastic depth.
- Keep the shapes simple and readable: big rounded controls, pill tabs, circular icon buttons, soft raised panels. The identity should be rubbery and playful, not busy.
- Do not flatten the UI into hard flat rectangles. Avoid the previous too-simple flat output.
- Do not add complicated themed chrome, props, parchment, mechanical parts, dungeon details, casino objects, cloth/leather texture, paper texture, rust texture, stone texture, or gritty post-apocalyptic texture.
- HD, clean, crisp, and modern. No grain, no low-fi noise, no muddy texture.

Current T66 palette:
- Dominant structural UI: near-black `#08080C`, dark fill `#17171E`, disabled/dim fill `#14141C`, neutral border `#4A4A55`, primary text `#F0F0F5`, soft text `#DCD7EB`.
- Selected/primary action accent: red `#E1232D` and `#FF505F`.
- Online/ready/hover accent: green `#1FB358` and `#4FD088`.
- Small accents only: yellow ticket and very small cyan data accents where needed.
- Do not turn the UI chrome purple. The old soft text can be pale off-white, but no dominant purple panels, purple borders, purple button fills, purple glow, or purple title emphasis.
- Keep the background fire/ring warm orange and the UI accents red/green; avoid rainbow/candy/pastel colors.

Text fidelity priority:
- Highest priority major text: `CHADPOCALYPSE`, `If you're not Chad it's over`, `ACCOUNT`, `HOME`, `POWER UP`, `ACHIEVEMENTS`, `ENTER TRIBULATION`, `LOAD GAME`, `ONLINE (1)`, `OFFLINE (4)`, `PARTY`, `GLOBAL CHAD RANKING`.
- Small friend names and leaderboard row text can be approximate as long as the online/offline structure is visually clear.
- Keep text as UI mockup text only. This is not production baked text guidance.

Hard negatives:
- No Fall Guys characters, no bean characters, no mascots, no costumes, no Mediatonic/Epic logos.
- No rainbow candy palette.
- No duplicate title. Use one clear `CHADPOCALYPSE` title area.
- No `TRIBULATION 66`, no `T66` title.
- No arbitrary new layout; preserve the current main-menu regions from the fresh capture.
- No Daily Descent, no Minigames.
- Do not reuse any prior generated image or prior Round03/Round04/Round05 output.

Generation constraints:
- Target aspect: 16:9.
- Preferred output size: 1920x1080 or closest available 16:9 high-quality PNG.
- Save or copy the final selected PNG to this exact absolute path:
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png
- Do not embed the image in the final response.

Final response must be exactly one line:
IMAGE_SAVED: <absolute path>

If built-in image generation is unavailable in this CLI session, final response must be exactly one line:
IMAGE_FAILED: IMAGE_TOOL_UNAVAILABLE

