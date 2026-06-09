# Question-Only Prompt: Pass12 Root-Cause Analysis

Working task:
Operator: Codex
Validator: Claude
Scope: answer the four process/root-cause questions only, with no implementation or asset changes.
Stop condition: provide concrete causes and solutions for tooling viability, sizing/icon layering, mask-smudge artifacts, and color/shape/detail mismatches.

User asks:

OpenCV and skimage and pillow what is the purpose of these things and should they even be used, can they really provide the professional quality we want? Also why do icons are clearly an icon on top of a button that already has an icon, Same with achievements the text clearly does not fit inside the button, Also for the two central buttons it looks like whitener was applied and stuff was written on top of it looks smudged and that there is an extra layer that makes it look like a manual mask was applied which is wrong. Also other than minimal differences there are some pretty big discrepencies, for example the row for the leaderboard is clearly red outline and black contents in the reference but the real thing has a red hilled interior for the row, also the high score checklist button, is clearly a uniform square with rounded edges in the reference while ours is not uniform in color or shape. Also for the friends panel the invite button is clearly green in the reference and the online sub header has a green circle next to it while our doesnt have either of those. So before we move on to the next iteration only answer with no implementation these 4 questions.

1. The openCV, skimage, pillow stuff, purpose and actual viability and if not viable what is the actual solution for the problem we were trying to solve by using it.
2. The problem with the sizing and fitting of buttons, and icons being placed on top of other icons, and what the solution is.
3. The cause of the masked and sloppy visual thing for buttons, where it looks like there is a layer of paint on top of the button and what is the solution.
4. The reason for the discrepancies in colors, shapes and details, and what the solution is.

Relevant evidence:

- Final pass12 capture: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_fixture_capture.png`
- Material crop sheet: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_crop_sheet.png`
- Material verdict: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\friendslop_pass12_material_verdict.md`
- Pass12 generator: `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\pass12_generate_reference_inpaint_plates.py`

Answer only. No implementation requested.
