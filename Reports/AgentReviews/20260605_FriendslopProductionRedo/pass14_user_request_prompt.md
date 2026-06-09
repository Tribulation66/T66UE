# Pass14 User Request Prompt

User request:

> Ok so the too many requests I think was a bug, if you get that you should just close that codex CLI and deploy a new one. I tested the iamgegen request and it was fine. So I think you were really locked out of doing the changes that needed to be done, because of that errror, because there is still obvious masking, fitting errors, and etc. Also one thing to point out is even the title is cut out. Now whats really important here is that we are going to have to reproduce this process for every screen, generating a reference image and then getting the screen to look like it, so all of the solutions you are coming up with that work, like example how to fit stuff, in the panels, you need to be noting this down in the process doc, and as you come with solutions do not look for quick fixes but systematic approach solutions so the problem does not occur for future screens. This is done in parallel with working on this screen, its still not anywhere near completion, so you need to work on figuring out the final solutions for the problems (or implementing if you were simply blocked by imagegen) and once the screen is finalized and perfectly matches the reference image, then compelte the process doc.

Working task:
Operator: Codex
Validator: Claude
Scope: continue the FriendslopStyle main menu fidelity pass toward a real reference match, retrying account-backed imagegen where needed, implementing systematic reusable fixes instead of one-off masks, and updating the Friendslop process documentation with solutions as they are proven.
Stop condition: produce a new verified screen pass with current capture/scorecard, or stop at a concrete blocker where the method/tooling must change before more implementation is honest.

Repo/process rules:
- Follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, UI/UI_AGENTS.md, UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, and UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md.
- Codex is Operator and Claude is Validator according to `.t66/operator-state.json`.
- The task is process-governed UI reference fidelity work; use PPF/parity/mechanism discipline.
- Do not use API fallback for imagegen; use account-backed built-in imagegen by default.
- Do not accept a structural PASS count if visual fidelity fails.

Context from pass13:
- Pass13 compiled and captured but was intentionally PARTIAL.
- Verifier was `PASS=250 FAIL=1 UNSURE=0`; the single fail was the holistic visual scorecard.
- Remaining known gaps include title crop, side-panel rubber chrome, exact button silhouettes/gloss, search chrome, masking/fitting artifacts, and responsive/manual gates not yet verified.

Please provide an independent repo-grounded answer:
- What systematic fixes should Codex implement next?
- Which fixes should update the Friendslop process doc as proven reusable rules?
- Which artifacts/proofs should pass14 produce before it is allowed to be accepted?
- Call out any likely blockers or method mistakes before Codex edits further.
