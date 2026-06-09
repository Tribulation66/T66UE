# Validator Prompt: FriendslopStyle Full Inventory Loop Process Change

User request:

> Ok I have an idea, for the process, because the whole point is the process here, and one issue I think we encounter in these iterations is that its often incomplete by nature of you focusing on the one component over other components for example the communication was mostly centered around the title so you really focused on the title, and did not focus on other things. And my suggestion actually helps protect against infirnitely iterrative loops. So here is my suggestion, basically for every screen a full inventory of elements is done, and then every single iteration, you work on it, it is a hard check to compare what we have against every single element, and every songle one that is not good, gets regenerated and re built. That way we force the whole thing to be done unless an element is perfect, so currently other than the title everything else needs to be redone. Does that make sense, how and where would you make these changes to our process docs?

Task contract:

Operator: Codex
Validator: Claude
Scope: answer whether the proposed per-screen full inventory/regenerate-every-failing-element loop makes sense, and identify exactly where/how to encode it in the FriendslopStyle process docs.
Stop condition: repo-grounded recommendation with specific doc targets and wording-level changes; no file edits unless the user explicitly asks to implement.

Relevant files to inspect read-only:

- `UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md`
- `UI/FriendslopStyle/README.md`
- `UI/FriendslopStyle/Screens/MainMenu/component_contract_current.md`
- `UI/FriendslopStyle/Screens/MainMenu/checklist.md`
- `UI/FriendslopStyle/Screens/MainMenu/visual_scorecard_template.md`
- `UI/FriendslopStyle/Screens/MainMenu/element_manifest.md`

Question for validator:

Should the process be changed so every screen pass starts from a complete per-element inventory and each iteration must classify every element as PASS / REGENERATE / REBUILD / ACCEPTED_DELTA, with all non-passing elements regenerated/rebuilt rather than focusing only on the latest conversationally salient component? If yes, where should this be documented, and what safeguards prevent endless loops or superficial bulk regeneration?

Return an independent repo-grounded recommendation and any concerns.
