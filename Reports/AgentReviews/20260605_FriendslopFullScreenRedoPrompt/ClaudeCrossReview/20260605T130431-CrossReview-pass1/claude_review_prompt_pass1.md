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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFullScreenRedoPrompt\original_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFullScreenRedoPrompt\codex_final_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFullScreenRedoPrompt\ClaudeIndependent\20260605T125923-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
User request:

Ok no, it failed EVERYWHERE youre focusing only on the width but the we also need visual gates for quality, and it should result in 96 fails, but this chat has gotten to laggy, so I need you to basically give the prompt for the agent, give him full context and information of where stuff is, and then have him make another attempt at the full screen, to look like the reference image. For all image generation he should deploy a codex CLI. The next screen needs that the elements on the assembled screen look like the elements from the extracted elements image, and all the sizing and positioning is correct. Go ahead and produce the prompt so I can retire this chat

Task contract:

```text
Working task:
Operator: Codex
Validator: Claude
Scope: produce a handoff prompt for the next agent to redo the full FriendslopStyle Main Menu against the reference, with full context, paths, constraints, and the corrected quality/fidelity expectations. No implementation in this chat.
Stop condition: provide a usable prompt and artifact path for the next agent.
```

Ask for Claude:

Review the planned handoff prompt for omissions or dangerous ambiguity. The output should help the next agent redo the full screen, using Codex CLI workers for image generation, and should make clear that visual quality gates must cover every load-bearing element, not just layout/width containment.

</original_prompt>

<validator_independent_answer>
I have enough grounding to produce the independent Validator answer.

Result: OK

## Independent Answer

The handoff prompt is internally producible — no user decision is pending. Below is the prompt I would hand to the next agent, followed by the omissions/ambiguities Codex must close before shipping its draft.

---

**Handoff prompt for the next agent (FriendslopStyle Main Menu full-screen redo):**

> **Role:** Operator. Redo the entire FriendslopStyle Main Menu so the assembled runtime screen visually matches the approved reference. Do not declare done on a structural PASS count alone — the prior pass did exactly that (`PASS=94 FAIL=0`) and the screen still failed everywhere.
>
> **Target reference (whole-screen):** `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round06\main_menu_reference_01_current_capture_stronger_rubber_cli.png`
>
> **Extracted-elements truth (per-element appearance):** `C:\UE\T66\UI\FriendslopStyle\Elements\main_menu_element_manifest.md` — every chrome plate the assembled screen renders must look like its extracted-element counterpart, not a generic reused plate. The manifest's status note explicitly says the prior asset names are NOT accepted final plates; re-derive plates that don't match at exact runtime size.
>
> **Process you must follow:** `UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md` (the loop) + `UI\FriendslopStyle\FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` + `UI\Instructions\UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md`. AGENTS.md enforces the loop as the acceptance gate.
>
> **Code surfaces:** `Source/T66/UI/Style/T66FriendslopStyle.{h,cpp}`, `Source/T66/UI/Screens/T66MainMenuScreen.cpp`, `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`, `Source/T66/UI/T66FrontendTopBarWidget.cpp`. Runtime assets under `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/` and `SourceAssets/UI/FriendslopStyle/MainMenu/`.
>
> **Image generation:** all image generation (plate/chrome regeneration, slices) must be produced by deploying a **Codex CLI worker** per the imagegen guidelines (`FriendSlop/IMAGEGEN_GUIDELINES.md`), not by hand or by reusing stale plates.
>
> **Quality gates — this is the whole point of the redo:**
> - The numeric result from `Scripts/VerifyUIFidelity.py` is a structured-data check only (width/containment/structural string-compares). It is NOT the fidelity verdict — the loop doc §2.3 states this explicitly.
> - Add a **blocking visual scorecard** covering *every load-bearing element*: top bar + buttons + ticket badge, left friends/party panel, profile row, search field, online/offline toggles, friend rows, action buttons, party slots, center title (must read live `CHADPOCALYPSE` text, not baked), Enter-Tribulation CTA, Load CTA, leaderboard filter buttons, leaderboard panel, time/metric dropdowns, ranking rows. For each, score: correct plate appearance vs extracted element, correct size, correct position. Per-element, per-quality-axis — that is what expands the gate to the expected ~96 checks (and the prior implementation should register as failing, not passing).
> - A pass is NOT layout/width containment alone. An element that is contained but visually wrong = FAIL.
>
> **Stop condition:** a current 1920×1080 capture + dump + visual scorecard where every load-bearing element matches the extracted-element appearance and is correctly sized/positioned, with the scorecard showing zero visual fails — or a documented hard blocker. Write the pass log per the loop.
>
> **Output artifacts:** captures/dumps/scorecard under `C:\UE\T66\Saved\Codex\UI\FriendslopStyle\MainMenu\` (next free `friendslop_passNN_*`).

---

## Evidence Checked

- `original_prompt.md` — user request + task contract (handoff prompt only, no implementation).
- `Reports/.../20260605_FriendslopStyleMainMenuImplementation/codex_final_draft.md` — the prior pass that claimed `PASS=94 FAIL=0`, confirming the gate-was-structural-only failure mode.
- `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` §2.3 — explicitly: numeric PASS count is not the whole-screen verdict; a blocking visual scorecard is required for FriendslopStyle.
- `UI/FriendslopStyle/Elements/main_menu_element_manifest.md` — the extracted-elements truth, full element list, and the note that prior plates are not final.
- `UI/FriendslopStyle/Reference/MainMenu/Round06/...` — approved reference path.
- `Scripts/VerifyUIFidelity.py`, Saved Codex MainMenu captures — verifier and artifact locations confirmed to exist.

## Questions Or Blockers

None requiring the user. The user already decided scope (full-screen redo), tooling (Codex CLI for imagegen), and the gate correction (quality gates over every element, not just width). No Codex `operator_draft.md` is saved in the redo folder yet — I reviewed against the live repo and prior implementation rather than Codex's actual draft text, so cross-check the final draft against the element list above.

## Caveats

- The expected "96 fails" is a user expectation, not a number I can independently reproduce — it depends on how many elements × quality axes the scorecard enumerates. The prompt should make the *axes* explicit (appearance/size/position per element) rather than hard-coding 96, so the count falls out naturally.
- "Look like the extracted elements image" + "sizing and positioning correct" are two distinct gates; the prompt must keep them separate so a correctly-placed-but-wrong-looking plate still fails.
- The manifest warns prior plate names are not accepted final assets — the next agent must avoid reusing them as-is, or the redo repeats the original failure.

</validator_independent_answer>

<codex_draft>
# Codex Final Draft

I produced the next-agent handoff prompt at:

`C:\UE\T66\Reports\AgentReviews\20260605_FriendslopFullScreenRedoPrompt\next_agent_prompt.md`

The prompt instructs the next agent to redo the full FriendslopStyle Main Menu, not just the leaderboard width. It names:

- the approved Round06 reference;
- the extracted elements/material target image;
- the failed pass08 capture/contact/report;
- the amended process/tooling reports;
- relevant code and asset paths;
- Codex CLI worker requirement for all image generation;
- full-screen visual scorecard requirements that should fail the old pass broadly;
- completion bar and expected output locations.

Please review the prompt for missing critical context, dangerous ambiguity, or anything that would let the next agent repeat the prior mistake of passing from structural verifier counts alone.

</codex_draft>
