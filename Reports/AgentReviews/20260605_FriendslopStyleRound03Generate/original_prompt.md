# Original Prompt And Task Contract

User asks to generate five new FriendslopStyle main-menu image references for internal vision boarding.

Latest user instruction:

> I do want you to deploy a fresh codex CLI for each iamge though

Context from immediately preceding prompts:

- Round02 preserved layout but still drifted into generic imagegen UI bias and was not close enough to the named game references.
- User wants source-reference closeness because the images are internal vision-board material only, not published assets.
- Replace the previous Lethal Company direction with Fall Guys.
- Generate five candidates total.
- Preserve the current T66 main-menu layout and contents.
- Use `Chadpocalypse` in the mockups instead of the current runtime title text.
- Ensure the five images are truly unique from each other, with high variance and no shared title bend/chunky fantasy plate drift.
- Each image generation should use a fresh Codex CLI worker so outputs do not inherit context or visual bias from prior generations.

Working task:
Operator: Codex
Validator: Claude
Scope: Generate five Round03 FriendslopStyle Main Menu references, preserving the current main-menu layout/content, replacing the title region with `Chadpocalypse`, using closer internal vision-board references for PEAK, Schedule I, Fall Guys, Gamble With Your Friends, and R.E.P.O., with stronger uniqueness/isolation gates. Use a fresh local Codex CLI execution for each image. No runtime UI implementation, no Unreal asset import, no source code edit, no Git operation.
Stop condition: Deliver five saved images, contact sheet, manifest, layout/uniqueness inspection notes, Claude validation, and token reporting.

Relevant repo rules:

- `AGENTS.md` Image generation row: use the approved built-in account-backed imagegen path. For repo-bound mockups that would clutter the main chat, use a separate local Codex CLI worker with the same account-backed imagegen process. Do not use or revive `OPENAI_API_KEY` API scripts.
- `UI/UI_AGENTS.md`: generated raster art is not runtime chrome for the active flat pipeline. These outputs are visual-direction references only and must not be treated as implementation-ready UI chrome.
- `OPERATOR_VALIDATOR_PROTOCOL.md`: Codex is Operator, Claude is Validator. Claude is read-only and advisory.

Baseline artifacts:

- Current main-menu screenshot: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- Current main-menu layout copy: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round02\layout_reference_current_main_menu.png`
- Structural inventory: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`

Implementation plan:

1. Create a Round03 reference folder under `UI/FriendslopStyle/Reference/MainMenu/Round03`.
2. Write five isolated prompt contracts, one per style direction.
3. Run five independent `codex exec` commands, each with the baseline screenshot attached via `--image`, one output path, and instructions to use account-backed built-in imagegen only.
4. Inspect outputs for layout/content preservation, title text, and pairwise uniqueness.
5. Create a contact sheet, manifest, and notes.
6. Ask Claude to cross-review the generated-artifact summary and notes before final response.

Process gates:

```text
PPF CHECK
Objective: Generate five internal FriendslopStyle main-menu visual-direction mockups.
Proven process: AGENTS.md Image generation row plus the existing ToonStyle/Combat VFX Codex CLI imagegen worker pattern.
My planned implementation: One self-contained prompt per reference direction, run through one fresh Codex CLI worker per image, using account-backed built-in imagegen and saving project-bound PNGs under UI/FriendslopStyle/Reference/MainMenu/Round03.
Same method class: YES
If NO, why: Not applicable.
User approval required before proceeding: NO
Verification evidence: saved PNGs, worker logs, manifest/contact sheet, visual layout/content check, uniqueness notes, Claude cross-review.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: current T66 main-menu layout/content screenshot.
Role: Primary
Required: YES
Planned artifact/path: Round03 prompt contract references C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png and the structural inventory.
Status: SAME
Evidence: existing baseline capture and main-menu structural inventory are used as hard layout anchors.

Reference artifact/category: source-game UI style references.
Role: Primary
Required: YES for vision direction, internal only.
Planned artifact/path: five style-specific prompts naming PEAK, Schedule I, Fall Guys, Gamble With Your Friends, and R.E.P.O. as visual-language references.
Status: EQUIVALENT
Evidence: prompt signatures and post-generation review notes.
```

```text
MECHANISM MANIFEST
Reference/source: FriendslopStyle Round03 generation plan.
Required mechanisms:
  1. Mechanism: layout preservation
     Required: YES
     Planned implementation: every prompt locks the same screen regions and labels.
     Evidence needed: visual inspection against baseline.
  2. Mechanism: source-style transfer without theme copying
     Required: YES
     Planned implementation: each prompt names UI-material, shape, typography, edge, and color rules from one game direction while preserving Chadpocalypse content.
     Evidence needed: per-image notes.
  3. Mechanism: worker isolation
     Required: YES
     Planned implementation: five separate Codex CLI executions, one image each.
     Evidence needed: individual worker logs/session folders.
  4. Mechanism: uniqueness discrimination
     Required: YES
     Planned implementation: reject/check repeated title arcs, repeated chunky fantasy plates, shared palette, or shared panel geometry.
     Evidence needed: contact sheet and uniqueness notes.
```

Question for Claude:

Before Codex runs generation, identify any missed constraints, unsafe process issues, or prompt/design risks that would likely repeat the Round02 failure. Keep this advisory and read-only.
