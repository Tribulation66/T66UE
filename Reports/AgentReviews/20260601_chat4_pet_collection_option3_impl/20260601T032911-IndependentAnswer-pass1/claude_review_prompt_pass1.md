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
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260601_chat4_pet_collection_option3_impl\original_prompt.md
- Output scope: read-only independent Validator answer for comparison with Codex's draft.

<original_prompt>
CHAT 4 -- DECISION: Option 3, with refinement. Proceed on the Foundation-side pet work now; defer only the stack-deposit proof.

Important boundary clarification that simplifies your blocker: credit timing is now locked as CREDIT-ON-SELL (Chat 3's decision). That means:
- The pet NEVER touches gold, and collection does NOT credit gold. Collection grows Economy's 999-capped COLLECTED STACK; gold is credited only later, at the vendor Sell card. So the "source-aware Economy gold credit path" is NOT on the pet's critical path at all -- the pet's job ends at depositing the collected drop into Economy's collected stack.
- Do NOT have the pet call AddGold or any gold path. The pet reserves, walks, collects via Foundation, and the collected value flows into Economy's collected stack. The vendor sale (Chat 3) is what credits gold, entirely outside the pet.

Build NOW (fully unblocked against proven Foundation API):
- The active pet's reserve/walk/collect/release loop: QueryAndReserveMobLoot (CollectorType=Pet), walk to the reserved drop, CollectReservedMobLoot in range, ReleaseMobLootReservation on unreachable/cancel. Confirmed all present and proven in T66MobLootSubsystem.
- Bond affects ONLY movement speed (already so in your code) -- keep it.
- Pass an ExclusionSphere through the query filter even if empty, so the future "ignore drops in hero radius" rule is a later filter change, not a rework. (Foundation already honors ExclusionSpheres.)
- No loot-bag actor/registry references (already clean).

DEFER only this: the end-to-end proof that pet collection GROWS ECONOMY'S COLLECTED STACK. That stack is greenfield Economy work Chat 3 is landing now. So:
- Build the pet so its collected drops deposit into Economy's collected-stack entry point. If that entry point isn't in live source yet, build against the documented boundary (collection returns value to Economy's stack), and PROVE the Foundation side now: pet reserves, walks, collects, releases, reservation blocks the player from the same drop, bond affects only speed.
- The full "pet collection -> collected stack grows -> sell credits gold" proof resumes once Chat 3's collected-stack path is live. Flag it as the deferred proof, don't fake it.

New content needs a FULL cook/stage, not -SkipCook. No git operations. Proceed on the pet behavior + Foundation-side proof now; defer the stack-growth proof to when Chat 3 lands.

</original_prompt>
