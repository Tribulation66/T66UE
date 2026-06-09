User prompt:
Ok so lets give it a shot, we will need process docs, under UI friendslop, but before we start creating the individual elements we need to have a visual direction, i believe our process for friendslop should be generate a reference image with imagegen and then you and claude use that to make the screen look like the reference image, and have something to check against until it looks like the reference image. What do you and claude think about this?

Working task:
Operator: Codex
Validator: Claude
Scope: Evaluate the proposed FriendslopUI process before implementation: generate a visual reference image first, then have Codex/Claude build screens against that reference with objective comparison until the UI matches. No process-doc edits or UI asset creation unless the user explicitly confirms after this recommendation.
Stop condition: Provide a joint Codex/Claude recommendation, including risks, required guardrails, and whether the process should become the basis for `UI/FriendslopUI`.

Relevant live repo context:
- Current `UI/README.md` says the active frontend pipeline is flat Slate through `FT66FlatStyle`, not generated raster chrome.
- Current `UI/UI_AGENTS.md` says generated raster chrome is forbidden for the flat pipeline.
- Current `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md` already uses reference images, geometry extraction, screenshot/dump verification, and pass logs for flat UI migration.
- Imagegen skill supports generated raster references/assets, but project-bound assets must be persisted into the workspace and transparent outputs need a cautious path.

Question for Claude:
- Is the proposed FriendslopUI reference-image-first process sound?
- What guardrails are required so it does not repeat the retired imagegen-chrome failure mode?
- What should Codex tell the user before writing process docs?
