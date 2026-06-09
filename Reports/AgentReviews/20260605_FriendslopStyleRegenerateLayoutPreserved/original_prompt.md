User prompt:
Ok so first things first, big issue, the key is I want to preserve the UI layout of my main menu and the contents, while these all chose an arbitrary layout and then designed it, also I want to change in game the T66 to Chadpocalypse, let me know if the T66 is text or part of the background image if part of the background image ignore it for now, but have the generated mockups use Chadpocalypse. Regenarate the images preserving layout and contents

Working task:
Operator: Codex
Validator: Claude
Scope: Regenerate the five FriendslopStyle Main Menu references, preserving the current T66 main-menu layout and contents rather than inventing a new layout. Use `Chadpocalypse` in generated mockups. Check whether the in-game `T66` title is live UI text or part of a background/image asset and report it. No runtime UI implementation or process-doc authoring.
Stop condition: Deliver five regenerated reference images saved in the repo, with preserved-layout basis, paths/contact sheet, title-source finding, Claude validation, and token reporting.

PPF CHECK:
Objective: Regenerate five FriendslopStyle Main Menu reference candidates while preserving the live main-menu layout/content and changing the title target to `Chadpocalypse`.
Proven process: Account-backed imagegen reference generation, using the live T66 Main Menu baseline capture/structural inventory as the reference artifact.
My planned implementation: Use the current baseline screenshot and structural inventory as the strict layout reference; generate five restyled full-screen references into `UI/FriendslopStyle/Reference/MainMenu/Round02`; inspect and save a manifest/contact sheet. These remain reference targets only.
Same method class: YES
If NO, why: n/a
User approval required before proceeding: NO, user asked to regenerate.
Verification evidence: Baseline reference path, generated PNG paths, contact sheet, visual inspection, Claude review.

ARTIFACT PARITY GATE:
Reference artifact/category: Current Main Menu baseline layout and visible content.
Role: Primary.
Required: YES.
Planned artifact/path: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png` plus `UI\Geometry\main_menu_structural_inventory.md`.
Status: SAME.
Evidence: Baseline capture inspected; structural inventory read; source confirms title is live text.

Live repo facts:
- `UI\Geometry\main_menu_structural_inventory.md` defines the current normalized layout: top bar, full background, left social/profile/friends/party panel, center title/subtitle/three CTA stack, right leaderboard/filter panel.
- `UI\Checklists\main_menu_checklist.md` defines current text content including `TRIBULATION 66`, `If you're not Chad it's over`, `ENTER TRIBULATION`, `LOAD GAME`, `DAILY DESCENT`, `Local Player`, `Level 1/100`, `Search friends...`, `ONLINE (0)`, `OFFLINE (0)`, `PARTY`, `GLOBAL CHAD RANKING`, leaderboard rows, and top bar entries.
- `Source\T66\UI\Screens\T66MainMenuScreen.cpp:470-474` creates `MainMenu.Center.Title` via `FT66FlatStyle::MakeFlatLabel(NSLOCTEXT(..., "TRIBULATION 66"), ...)`, so the center title is live Slate text, not part of the background image.
- `Source\T66\UI\Screens\T66MainMenuScreen.cpp:1896-1902` loads a background image separately.

Generation guardrails:
- Preserve layout/content from the baseline screenshot; do not invent the previous Round01 simplified vertical-button layout.
- Use `Chadpocalypse` in the center title region of generated mockups.
- Keep all other current menu content/roles visible: top bar tabs/icons, left panel, center subtitle and three CTA buttons, right leaderboard.
- Use the five style vocabularies only as UI-element style poles, not theme/IP copies.
- Output full-screen reference images only, not runtime UI assets.
