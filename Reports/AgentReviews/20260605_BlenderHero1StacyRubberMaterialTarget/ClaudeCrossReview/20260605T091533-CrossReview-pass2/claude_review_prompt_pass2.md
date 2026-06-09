You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_BlenderHero1StacyRubberMaterialTarget\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_BlenderHero1StacyRubberMaterialTarget\operator_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_BlenderHero1StacyRubberMaterialTarget\ClaudeIndependent\20260605T091246-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
# Original Prompt And Task Contract

## Original Prompt

MISSION: Develop the "rubber" material look for the hero character, entirely in Blender. Do not touch Unreal. Output is a locked visual target plus a transferable, documented material recipe. The user clarified to use the female Hero 1 character, the currently used/recently generated female Hero 1 in the FriendSlop/FriendslopStyle direction, and target a Fall Guys-like rubber/vinyl material read. First task: locate and confirm the source asset path with Pablo before doing anything else.

## Working Task

Working task: Locate the intended source asset for the female Hero 1 Pixel3D/FriendSlop-style character and stop for Pablo confirmation before any Blender look-development work.
Operator: Codex
Validator: Claude
Scope: Read-only repo discovery plus a durable decision block. No Blender material creation yet, no Unreal editor/runtime work, no Unreal import, no rigging/jiggle, no Git operation.
Stop condition: Pablo confirms the exact source path to use, or corrects the source path.

## Relevant Repo Rules

- Root `AGENTS.md`: derive a task contract; use current live repo state; read `.t66/operator-state.json`; use Operator/Validator loop when available.
- Root `AGENTS.md`: if only the user can decide how the task proceeds, ask once, save `Reports/AgentReviews/<TaskSlug>/decision_block.md`, and stop.
- Model Generation router: Pixal3D/Blender asset source work routes through `Model Generation`.
- User hard boundary: do not touch Unreal. UE5 reproduction is out of scope.


</original_prompt>

<validator_independent_answer>
Result: NEEDS_USER

## Independent Answer

The task contract's own stop condition is "Pablo confirms the exact source path," and the discovery surfaces a genuine ambiguity that only Pablo can resolve, so stopping for confirmation is correct (not a defect Codex can patch).

There is **no female Hero 1 mesh in the FriendSlop 3D direction** in the repo. What exists for the "FriendSlop female Hero 1" splits into two non-equivalent candidates:

1. **2D FriendSlop female reference image (most recent, FriendSlop-direction):**
   `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png` (generated 2026-06-04)
   - This is the "recently generated female Hero 1 in the FriendSlop direction," but it is a flat concept render, not a 3D mesh. It cannot host Blender look-dev directly.

2. **Existing female 3D mesh (different style — Pixal3D toon, not FriendSlop):**
   `Content/Characters/Heroes/Hero_1/Stacy/Pixal3DToonStyle/SM_Hero_1_Stacy.uasset` (+ `MI_Hero_1_Stacy`, textures)
   - This is the live female 3D asset, but in the Pixal3D toon style, not the FriendSlop/FriendSlopStyle direction the mission names.

For contrast, the **male** has a true FriendSlop raw mesh — `Content/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/SM_Hero_1_Chad_Male.uasset` (2026-06-05) — with **no female counterpart**. Note also a naming mismatch: the female FriendSlop image is foldered as "Hero_1_Chad_Female," while the female in `Content/` is "Stacy."

Recommended question for Pablo (one decision, then stop):
- Which is the source asset for Blender rubber look-dev — (a) the 2D FriendSlop female reference PNG (to be modeled/used as concept target), (b) the existing `SM_Hero_1_Stacy` Pixal3D mesh, or (c) a female FriendSlop mesh that does not yet exist and must be generated first? If (c), the model-generation prerequisite blocks this task.

## Evidence Checked
- `.t66/operator-state.json` — Operator=Codex, Validator=Claude (confirmed loop).
- Glob of `Content/Characters/Heroes/Hero_1/**` — Stacy assets only in Pixal3D/AnimatedToon/Beachgoer/DemoSkin styles; no FriendSlop female mesh.
- `find -iname "*friendslop*"` — only female asset is the 2026-06-04 reference PNG under `Hero_1_Chad_Female`.
- Female reference manifest + prompt — confirms it is a 2D reference repro, FriendSlop direction, two design changes (pinkish skin, oven-mitt hands).
- `Chad/FriendSlopRaw/` — male has a FriendSlop 3D mesh; no female equivalent.

## Questions Or Blockers
- Only Pablo can pick the canonical source among the 2D reference, the existing Pixal3D Stacy mesh, or a not-yet-existing FriendSlop female mesh.
- If a female FriendSlop mesh is required, model generation (routed through `Model Generation`) is an unmet prerequisite before any Blender look-dev.

## Caveats
- Read-only inspection; no files were opened in Blender and the PNG was not visually rendered here.
- Timestamps are filesystem mtimes, which can differ from authoring intent.
- Per AGENTS.md, save a `Reports/AgentReviews/<TaskSlug>/decision_block.md` capturing the candidates and stop for Pablo — consistent with the task's stop condition.

</validator_independent_answer>

<codex_draft>
# Operator Draft

I located the likely source candidates but am stopping at the required source-confirmation gate.

Important correction: the recent FriendSlop female Hero 1 artifact appears to be a 2D reference PNG, not a 3D mesh:

`C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`

The best existing female Hero 1 3D source I found is:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb`

Reason: it is the raw generated GLB for `Hero_1_Stacy`, exists on disk, and the live `CharacterVisuals.csv` row shows `Hero_1_Stacy` is the active female Hero 1 row, while `Hero_1_Stacy_DemoSkin` maps to the beachgoer/demo variant. The raw GLB is also the best existing 3D baseline for the requested "raw Pixel3D output vs Fall Guys rubber version" Blender comparison.

I have not touched Unreal, opened Blender, created materials, rendered images, or changed source assets. The decision gate is recorded in `Reports/AgentReviews/20260605_BlenderHero1StacyRubberMaterialTarget/decision_block.md`.

Question for Pablo: confirm whether I should use `C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb` for Blender look-dev now, or whether the recent FriendSlop female PNG is the required source direction and we need a female FriendSlop 3D mesh generated first. You can also give a different exact path, or say `no clarification needed` if I should proceed with the recommended GLB.

</codex_draft>
