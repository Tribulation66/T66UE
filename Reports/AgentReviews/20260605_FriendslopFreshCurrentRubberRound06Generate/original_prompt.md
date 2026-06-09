# Claude Independent Review Brief

## User Request

The user rejected the previous FriendslopStyle main-menu mockup because it appeared to use a stale screenshot/reference: it showed old content such as Minigames/Daily Descent, had top-bar/count problems, still leaned purple, and lost the rubbery/bouncy feel. The user asked to delete the screenshot used as reference, pull a true fresh screenshot of the current Main Menu, confirm the current colors (likely red and green as the two light colors), preserve current content including online/offline friends, and regenerate a stronger rubbery/bouncy main-menu reference.

## Working Task

Operator: Codex
Validator: Claude
Scope: Remove the stale main-menu screenshot used as the previous layout reference, capture a fresh current Main Menu screenshot through the Unreal-owned UI capture path, verify current content/palette from live repo/assets, then generate a corrected rubbery bouncy reference from the fresh screenshot and original star/fire/statue background. No runtime UI implementation unless capture tooling requires a narrowly scoped fix.
Stop condition: stale reference removed, fresh screenshot saved, corrected image saved, worker closed, manifest/QA notes written, Claude validation completed, and token reporting included.

## Relevant Repo Rules

- Use `UI/UI_AGENTS.md` as the UI router.
- Runtime UI chrome must remain Slate-native; generated raster art in this task is only a visual reference/mockup.
- Do not bake live localized text or player data into future runtime chrome. For this reference mockup, text should visually preserve the current screenshot contents.
- Use `Scripts/CaptureT66UIScreen.ps1` / Unreal-owned capture paths for UI screenshots, not desktop screenshots.
- Use the account-backed built-in imagegen path via a local Codex CLI worker for generated bitmap references; do not use API scripts or `OPENAI_API_KEY` fallbacks.
- Current flat palette from `UI/Reference/UI_FLAT_REDESIGN_REFERENCE.md`: near-black/dark fills, neutral gray borders/text, red selected/progress (`#E1232D` / `#FF505F`), green ready/hover/online (`#1FB358` / `#4FD088`), and no dominant purple.

## Planned Operator Steps

1. Delete `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png` after verifying the path.
2. Capture a fresh current Main Menu PNG and paired JSON dump using `Scripts\CaptureT66UIScreen.ps1 -Screen MainMenu`.
3. Inspect the fresh PNG/dump for current top bar, main CTAs, online/offline friends, and whether Daily Descent is actually present.
4. Write a strict Round06 imagegen prompt under `UI/FriendslopStyle/Reference/MainMenu/Round06/prompts/`.
5. Spawn one fresh local Codex CLI worker for the imagegen call, attaching the fresh capture and original background image.
6. Inspect the generated image for stale content, purple drift, layout/content preservation, and stronger rubber/rubberized bouncy treatment.
7. Write a manifest/QA note and run a Claude cross-review.

## Requested Claude Output

Give a concise independent review of the plan. Focus on anything likely to break the user's stated goal: stale capture sources, missing current-content evidence, palette mistakes, process violations, or anything that would make the generated mockup less useful as a FriendslopStyle direction reference.
