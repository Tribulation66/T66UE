# Original Prompt And Task Contract

User request:

> Ok ok so for all of these new ones, the designs became a bit more complicated, what I really like about the original Fall Guys ons is that all the buttons, and UI elements were pretty simple, so lets go back to that, use my original background image, with the stars, and the ring of fire behind an eclipse behind the head of the character, And then for colors use the same color scheme that I currently use but with Falls guys style of UI.

Working task:
Operator: Codex
Validator: Claude
Scope: Generate one focused Round05 FriendslopStyle main-menu reference that returns to simpler Fall Guys-like bouncy UI element shapes, uses the original starfield / fire eclipse ring / golden statue head background, and uses the current T66 palette rather than a rainbow Fall Guys palette. Preserve the recognizable current main-menu layout/content and `Chadpocalypse`. No runtime UI implementation, no Unreal import, no source code edit, no Git operation.
Stop condition: saved PNG, fresh CLI worker provenance, worker no longer running, direct visual inspection, manifest/QA notes, Claude validation, and token reporting.

Relevant live artifacts:

- Layout reference screenshot: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- Original background image: `C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens\MainMenu\ScreenArt\mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_1920.png`
- Structural inventory: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`
- Current palette source: `C:\UE\T66\UI\Reference\UI_FLAT_REDESIGN_REFERENCE.md`

Current palette values to use:

- Background: `#08080C`
- Default fill: `#17171E`
- Default border: `#4A4A55`
- Default text: `#DCD7EB`
- Primary text: `#F0F0F5`
- Secondary text: `#A7A7B0`
- Selected/accent red: `#E1232D`, `#FF505F`
- Hover/ready green: `#1FB358`, `#4FD088`
- Data cyan: `#3CDCF0` only as a small accent
- Yellow ticket accent preserved as a small accent

Process:

- Use a fresh local `codex exec` worker.
- Attach both the layout reference screenshot and original background image.
- Use account-backed built-in Codex image generation only.
- Do not use `OPENAI_API_KEY` or OpenAI API image scripts.
- Do not copy prior generated outputs.

Process gates:

```text
PPF CHECK
Objective: Generate one focused Round05 main-menu reference: simple Fall Guys-like bouncy UI, original star/fire-eclipse/statue background, current T66 palette.
Proven process: AGENTS.md Image generation row plus the local Codex CLI worker pattern used in Rounds 03/04.
My planned implementation: One prompt contract, one fresh `codex exec` worker, account-backed built-in imagegen only, attaching both the current UI layout screenshot and the original background image.
Same method class: YES
If NO, why: Not applicable.
User approval required before proceeding: NO
Verification evidence: saved PNG, worker log/exit check, direct visual inspection, manifest, Claude validation.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: original main-menu background.
Role: Primary
Required: YES
Planned artifact/path: attach `RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/ScreenArt/mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_1920.png`.
Status: SAME
Evidence: opened and verified it contains starfield, fire eclipse ring, and golden statue head.

Reference artifact/category: current main-menu layout/content.
Role: Primary
Required: YES
Planned artifact/path: attach `Saved/Codex/UI/MainMenu/baseline_capture.png`.
Status: SAME
Evidence: existing baseline and structural inventory.

Reference artifact/category: simple bouncy UI.
Role: Primary
Required: YES
Planned artifact/path: prompt requires simple rounded/pill controls, not complex themed chrome.
Status: EQUIVALENT
Evidence: final visual inspection.
```

Question for Claude:

Before Codex runs generation, identify any missed constraints or prompt risks. Focus on keeping the UI simple, using the original background, using the current T66 palette, and avoiding Fall Guys IP/rainbow characters.
