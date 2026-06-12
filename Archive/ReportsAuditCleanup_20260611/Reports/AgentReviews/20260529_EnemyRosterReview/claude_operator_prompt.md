# Claude Operator Prompt: Enemy Roster Review

Working task:
Produce a human-readable inventory of the entire T66 enemy roster for Pablo's non-technical design review.

Operator:
Claude

Validator:
Codex

Scope:
Read enemy/boss roster data and gameplay wiring, then write one readable report document plus an operator completion artifact. No source, config, content, save, data-table, or runtime behavior changes.

Stop condition:
`C:\UE\T66\Reports\RosterReview\enemy_roster_review.md` exists, covers bosses, hidden bosses, minibosses, specials, basic mobs, theme/stage coverage, and gap flags in plain language, with technical traceability kept separate from the main prose.

Audience:
Pablo as designer. Main text must be plain language and readable without class names or file paths. Keep technical row IDs, data tables, and classes in a separate reference column, appendix, or footnote area.

User context:
- This review happens before cleanup/deletion Pass D so Pablo can catch forgotten or missing enemies before deprecated code is deleted.
- Gap flags are for Pablo to react to, not changes to make.
- Scope is ENEMY ROSTER ONLY: bosses, hidden bosses, minibosses, specials, and basic mobs.
- Out of scope: NPCs, interactables, items/weapons/idols, hero, companions, projectile systems, Mini/minigames, and any cleanup/deletion.

Instructions to read first:
- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Reports\AGENTS.md`
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`
- `C:\UE\T66\Gameplay\README.md`
- Relevant pending issue files before relying on related claims:
  - `C:\UE\T66\Content\Data\pending_issues_Data.md`
  - `C:\UE\T66\Source\T66\Data\pending_issues_Data.md`
  - `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
  - `C:\UE\T66\Source\T66\Gameplay\Enemies\pending_issues_Enemies.md`
  - `C:\UE\T66\Gameplay\Combat\pending_issues_Combat.md`

Data/code sources to inspect read-only:
- `C:\UE\T66\Content\Data\Bosses.csv`
- `C:\UE\T66\Content\Data\Enemies.csv`
- `C:\UE\T66\Content\Data\Stages.csv`
- `C:\UE\T66\Content\Data\BossEncounters.csv`
- `C:\UE\T66\Content\Data\BossEncounterMembers.csv`
- Related enemy/boss data files only if these tables point to them.
- Boss, miniboss, special, and mob class implementations under `Source\T66`, excluding Mini/minigame modules/paths.
- Spawn/routing/encounter code needed to distinguish defined-in-data vs reachable-in-play.

Deliverable:
- Main report: `C:\UE\T66\Reports\RosterReview\enemy_roster_review.md`
- Operator completion: `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterReview\operator_completion.md`

Required report sections:

1. Quick Summary Counts
   - Total bosses
   - Total hidden bosses
   - Total miniboss types/system entries
   - Total specials
   - Total basic mob types
   - Any caveats about "defined in data" vs "actually reachable"

2. Bosses
   - Every boss.
   - For each: name, stage/theme, concept in plain language, implemented/reachable status.
   - Include total count.

3. Hidden Bosses
   - Count and list.
   - For each: what it is, how it is triggered/found, stage/condition, reachable status.

4. Minibosses
   - Describe current system: placed Slime guardians on floor doors for floors 2, 3, and 4.
   - Explain they are scaled-up slime placeholders.
   - Confirm whether any other miniboss types exist beyond that placeholder.

5. Specials
   - List Goblin Thief, Unique Debuff, Gambler boss, and any other special enemy found.
   - For each: what it is, what makes it different from a normal enemy, trigger/spawn method, active vs latent/unspawned.

6. Basic Mobs
   - List families: Melee, Rush, Flying, Ranged.
   - List distinct mob types within those families.
   - For each: name, theme(s), plain-language movement/attack behavior.

7. Theme/Stage Coverage
   - Plain-language summary for Dungeon, Forest, Ocean, Martian, Hell.
   - For each: bosses, minibosses, specials, basic mobs, and obvious coverage gaps.

8. Pablo Attention Flags
   - Anything thin, inconsistent, half-built, referenced-but-missing, duplicate, orphaned, naming-inconsistent, data-defined but unspawned, code-wired but missing data, or otherwise likely worth Pablo's attention.
   - Phrase as "Flag" / "Why it caught attention" / "Decision Pablo may want to make".
   - Do not propose or make code/data changes.

9. Technical Traceability
   - Keep data row, file, and class references here or in narrow reference columns.
   - Main descriptions should avoid technical naming.

Validation expectations for the operator:
- Use structured parsing for CSVs where practical instead of eyeballing.
- Cross-check data tables against spawn/routing code.
- Clearly distinguish:
  - defined in data;
  - implemented in code;
  - reachable in normal play;
  - latent/unspawned.
- If confidence is partial, say so plainly in the report.
- Do not edit any code, content, config, save, CSV, uasset, or source data.
- Do not inspect Mini/minigame paths.
- Avoid broad Git/LFS scans.

Operator completion artifact must include:
- Files created/changed.
- Data sources read.
- Code areas inspected.
- Validation notes and known confidence limits.
- Token routing metadata if available.
