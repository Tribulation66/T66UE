You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructurePlan\codex_operator_approval.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Claude Operator Prompt: Enemy Roster Restructure Investigation Plan

Working task:
Investigate the current code/data state for Pablo's locked enemy-roster restructure decisions, then produce one consolidated implementation plan for Pablo review. This is plan + investigation only.

Operator:
Claude

Validator:
Codex

Scope:
Read data/code/docs and write report-only artifacts. Do not implement any source, data, config, content, asset, save, runtime, build, stage, Git, cleanup, or deletion change.

Stop condition:
`C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md` exists, covers every numbered item in the prompt, names current foundations found/not found, proposes implementation approach and affected files/data, and has an explicit uncertainty/foundation-not-found section.

Context:
- This follows `C:\UE\T66\Reports\RosterReview\enemy_roster_review.md`.
- Pablo's locked design decisions are in this prompt.
- Some items overlap with a later cleanup pass; fold them into the implementation plan, but do not do cleanup now.
- Audience is Pablo reviewing a plan before implementation.

User-locked end state:

SPECIALS:
- Exactly two special enemies: Loan Shark and Backrooms Stalker.
- Loan Shark is expected to already have a foundation. Find it: class, behavior, spawn path, completeness. If genuinely not found, say so plainly.
- Backrooms Stalker is enabled as a room encounter, not a separate game mode; player enters the Backrooms room and flees. Stalker keeps Slime placeholder visual.
- Remove Goblin Thief entirely: enemy, gold-steal mechanic, Luck-biased wave spawn, RNG tuning entries only supporting it.
- Remove Debuff enemy entirely: floating debuff enemy, status projectile, Lab-only spawn, support code.

HIDDEN BOSS:
- Exactly one hidden boss: Vendor.
- Repurpose existing Gambler boss entity/mechanics into Vendor boss; strip Gambler identity.
- Trigger Vendor boss from existing "attempt to steal from shop" mechanic. Find the steal trigger foundation.
- Remove casino-anger-to-boss spawn and all Gambler-as-enemy references.
- Do not touch casino as an interactable/gambling feature; only remove its boss/enemy spawn path.

MINIBOSSES:
- Replace single placeholder Slime guardian with 12 mega-mob minibosses per difficulty.
- Structure: 3 miniboss gates per stage (floor 2->3, 3->4, 4->5), 4 stages per difficulty = 12 gate encounters per difficulty.
- Each difficulty has 12 basic mobs after additions below; each basic mob gets a mega version using the same model and behavior, scaled HP/damage/size.
- Mega-mobs fill the 12 gate slots per difficulty, 1:1. Propose sensible default assignment.
- Reuse/refactor existing placed guardian scaling/descent-gate wiring. Keep must-kill-to-proceed. Minibosses stay rich actors.

BASIC MOBS:
- End state: 12 basic mobs per theme.
- Add 10 proposed mobs using placeholder models/behavior:
  - Dungeon: Cursed Crow (Flying, placeholder Cave Bat); Famished Ghoul (Rush, placeholder Rat Pack)
  - Forest: Will-o-Wisp (Flying, placeholder Hive Wasp); Gore Stag (Rush, placeholder Tusker Boar)
  - Ocean: Gull Diver (Flying, placeholder Ghost Ray); Hammerjaw (Melee, placeholder Crab Guard)
  - Martian: Recon Orb (Flying, placeholder Saucer Drone); Carapace Brute (Melee, placeholder Crystal Crawler)
  - Hell: Cinder Wraith (Flying, placeholder Fire Skull); Brimstone Brute (Melee, placeholder Bone Knight)
- Plan adding these to enemy data/spawn pools and feeding the mega system.

TAG REMOVAL:
- Remove Exploder / Stutterer / Burrower archetype tags from the 12 mobs carrying them. They become plain family behavior.
- Remove MiniBoss-feel tag from 6 mobs. They remain ordinary basic mobs.

WORDING/NAMING:
- Rename "gameplay floors" to "mob floors" or a clear term.
- Tower is 5 floors: floor 1 start/no enemies, floors 2-4 mob floors, floor 5 boss/no mobs.
- Confirm gate guardians sit only on floor 2->3, 3->4, and 4->5 descent transitions, with no non-boss guardian outside that range.

DORMANT CLEANUP:
- Remove dormant random-miniboss-promotion tuning values such as MiniBossChancePerWave and related values, because minibosses are exclusively placed mega-mobs.

Required reads:
- Root/process:
  - `C:\UE\T66\AGENTS.md`
  - `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
  - `C:\UE\T66\Reports\AGENTS.md`
  - `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`
  - `C:\UE\T66\Gameplay\README.md`
- Prior roster report:
  - `C:\UE\T66\Reports\RosterReview\enemy_roster_review.md`
- Data:
  - `C:\UE\T66\Content\Data\Bosses.csv`
  - `C:\UE\T66\Content\Data\Enemies.csv`
  - `C:\UE\T66\Content\Data\Stages.csv`
  - `C:\UE\T66\Content\Data\BossEncounters.csv`
  - `C:\UE\T66\Content\Data\BossEncounterMembers.csv`
  - related enemy/boss/shop/casino/unique/backrooms data only as needed.
- Code/docs:
  - Enemy director/routing/wave spawn code.
  - Tower/floor/descent gate code.
  - Backrooms room/encounter/Stalker code.
  - Loan Shark code/data/docs: search thoroughly for LoanShark, Loan Shark, shark, loan, debt, lender, interest, collections, casino/shop debt, and similar foundations.
  - Shop/vendor/steal/attempt-to-steal code: search for steal, theft, shop, vendor, anger, casino anger, Gambler boss, Vendor boss.
  - Goblin Thief and Debuff enemy code and spawn paths.
  - RNG tuning/config entries supporting Goblin.
  - Data type definitions for enemy archetype/feeling/status fields.
  - Relevant pending issue files in Content/Data, Source/T66/Data, Source/T66/Gameplay, Source/T66/Gameplay/Enemies, Gameplay/Combat.

Deliverable:
Main document:
`C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`

Completion artifact:
`C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructurePlan\operator_completion.md`

Plan document requirements:

1. Executive Summary
   - What the restructure will achieve.
   - Recommended implementation pass split.
   - Any blockers/uncertain foundations.

2. Current Foundations Found
   - Loan Shark foundation: found or not found, with exact evidence.
   - Backrooms Stalker/room foundation.
   - Shop steal trigger foundation.
   - Gambler boss foundation.
   - Placed guardian/miniboss foundation.
   - Enemy data/spawn pool foundation.
   - Archetype/Feeling tag foundation.
   - Dormant random miniboss-promotion foundation.

3. Numbered Item Plan
   For EACH item 1 through 11:
   - Current state.
   - Proposed approach.
   - Files/data likely touched during implementation.
   - Verification needed.
   - Risks/open questions.

4. Affected Files/Data Map
   - Group by source code, data CSV/uasset refresh, docs, validation scripts/tests.

5. Sequencing / Pass Split
   - Recommend how to split implementation into sane reviewed passes so removals/additions don't tangle.
   - Include read-only validation gates for each pass.

6. Unclear / Missing Foundation Section
   - Put Loan Shark here if not found.
   - Put anything else not located or needing Pablo decision here.

7. Out-of-Scope / Non-Actions
   - Casino interactable remains.
   - No status-effect mob work.
   - No real models.
   - No B.13 sandbox deletion or deprecated rich-mob/CVar/projectile-class deletion unless overlap is only noted.

8. Technical Traceability
   - File/data/class references so implementation agents can follow the plan.

Tone:
- Human-readable but implementation-grade.
- Be explicit when an approach is uncertain.
- Do not invent a Loan Shark foundation if not found.

Validation expectations:
- Use targeted searches, not broad Git/LFS scans.
- No Mini/minigame inspection.
- Do not write outside `Reports/RosterReview` and this task's `Reports/AgentReviews` folder.
- Do not modify code/data/content/config/save files.

