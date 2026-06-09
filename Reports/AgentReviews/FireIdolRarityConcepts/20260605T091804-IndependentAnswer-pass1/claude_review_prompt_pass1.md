You are Claude providing the independent Validator answer for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Inspect the live repo read-only when repo context is needed.
- Treat Codex as the Operator/final router and you as the independent Validator.
- Produce the answer you would give to the user from the current evidence.
- Look for scope constraints, repo instructions, user-only decisions, missing evidence, and caveats.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the answer practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown answer with exactly these headings:
Independent Answer
Evidence Checked
Questions Or Blockers
Caveats

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the answer body and keep the result OK.

Independent answer scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\FireIdolRarityConcepts\original_prompt.md
- Output scope: read-only independent Validator answer for comparison with Codex's draft.

<original_prompt>
# Original User Request

The user approved/revised the central idol concepts and now wants Fire only, broken down by category and rarity.

Relevant current request:

> Fire red AOE, that's fine. DOT should be a burn thing: the VFX takes place on the enemy body, not on the floor; hit the enemy and the body catches fire. Pierce is fine. Bounce should be embers snapping and bouncing: it hits and flicks like four embers to four close enemies. Next, go one by one. First, Fire, all four rarities, how they change. Break it down by category and rarity: black, red, yellow, white.

## Task Contract

Working task: Planning-only Fire idol rarity breakdown.
Operator: Codex
Validator: Claude
Scope: Fire AOE, Fire DOT, Fire Pierce, Fire Bounce only. For each, propose Black/Red/Yellow/White rarity visual changes. No imagegen, no Unreal edits, no asset generation.
Stop condition: Return approval-ready rarity concepts for Fire idols.

## Live Roster Evidence

`Content/Data/Idols.csv` contains:

- `Idol_Fire_DOT`
- `Idol_Fire_AOE`
- `Idol_Fire_Pierce`
- `Idol_Fire_Bounce`

## Constraints

- Fire activation VFX should stay red/orange/fire-themed regardless of rarity label.
- Avoid a mechanical 1/3/5/one-big pattern. Rarity should change shape grammar, timing, density, and impact expression while preserving category identity.
- Keep the concepts feasible for simple placeholder shapes first: spheres, rings, capsules, cones, crescent/arc shapes, short line/ribbon trails, and billboard flame/ember sprites.

</original_prompt>
