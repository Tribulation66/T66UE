You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation\codex_operator_approval_phase1.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Claude Operator Prompt â€” Enemy Roster Restructure Implementation Phase 1

## Working Task

Implement the approved enemy-roster restructure in `C:\UE\T66` as Phase 1 of one user-requested pass.

Operator: Claude (`claude-opus-4-8`, FullOperator)
Validator/Finisher: Codex
Scope: Source/data implementation for sections A-E below, plus static checks and a completion packet.
Stop condition: Apply the source/CSV implementation if cleanly possible, or stop and report any foundation mismatch/blocker without guessing.

## Required Process Files To Read First

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`
- `C:\UE\T66\UI\UI_AGENTS.md`
- `C:\UE\T66\Reports\AGENTS.md`
- `C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`
- Existing pending issue files adjacent to touched source/data folders.

## User Locked Decisions

- Vendor boss trigger: ANY failed steal attempt in the shop spawns the Vendor boss. No threshold.
- Token: rename `GamblersToken` -> `VendorToken` if any remaining runtime/data naming still says GamblersToken.
- Casino anger: remove the casino anger system entirely, including anger meter and boss-spawn path. Keep casino gambling interactable functional. Deeper casino loose ends go to Pass E, not this pass.
- Backrooms reward: Pablo confirms Backrooms reward uses different assets than deleted QuickReviveIcon/QuickReviveVending assets. If implementation verification shows reward actually depends on deleted assets and is broken, stop and surface it.
- `Stages.csv` expansion is a schema change: expand EnemyA..EnemyJ to EnemyA..EnemyL; update stage struct, CSV/DataTable source columns, and stage mob-ID resolver.
- Deeper game-concept/asset review is deferred to Pass E.
- Exclude Mini/minigame systems unless directly required by the explicitly named casino gambling surface; do not touch `Gameplay/Minigames`.

## Approved Scope For This Phase

Phase 1 may edit source/data/docs needed to implement Sections A-E below:

- Source under `Source/T66` relevant to enemies, tower, stage data, run state/economy, casino/vendor trigger, and data structs.
- Data source files under `Content/Data` relevant to enemies, stages, bosses, encounters, unique enemies, loan shark, RNG tuning source/config if present.
- Report/completion artifacts under `Reports/AgentReviews/20260529_EnemyRosterRestructureImplementation`.
- Build/source project metadata only if deletion/addition of source files requires it.

Phase 1 may run source/static checks and focused repo searches. Avoid broad Git/LFS scans over binary asset folders. Do not run editor import, DataTable rebuild commandlets, staged builds, or runtime smoke in Phase 1 unless needed only as a quick blocker check; those are expected in Phase 2.

## Explicitly Excluded Actions

- Do not commit, push, tag, stage files in Git, reset, clean, checkout, or revert user changes.
- Do not delete or mutate `Content` binary assets except generated DataTable assets if you deliberately decide they must be updated in this phase; prefer deferring uasset rebuild to Phase 2.
- Do not touch the 51 GB B.13 sandbox worktree.
- Do not delete deprecated rich-mob/CVar/projectile-class cleanup outside the requested roster restructure.
- Do not redesign casino gambling; keep the casino interactable functional.
- Do not implement real models for new mobs/mega-mobs/Stalker.
- Do not add status effects on mobs.
- Do not inspect/change Mini/minigame content except explicitly required casino gambling UI/runtime files.

## Implementation Order

Follow this order. Where two sections touch the same file, sequence edits in this order.

### Section A â€” Removals First

A1. Remove Goblin Thief entirely:
- Grep all references first.
- Remove wave-spawn block in `T66EnemyDirector.cpp`.
- Remove `GoblinWaveChanceBase` / `GoblinCountPerWave` RNG tuning fields and loaders/defaults/data rows.
- Remove Lab spawn branch.
- Remove `AT66GoblinThiefEnemy` class/source references.
- Remove gold-steal mechanic and enemy data/registry/Lab UI references.

A2. Remove Debuff enemy entirely:
- Confirm `AT66UniqueDebuffProjectile` is not shared by another enemy before deleting.
- Remove Lab spawn branch.
- Remove `AT66UniqueDebuffEnemy` and its status projectile class/source references.
- Remove data/registry/Lab UI references.

A3. Remove dormant random-miniboss-promotion tuning:
- Remove `MiniBossChancePerWave`, `MiniBossScale`, `MiniBossHPScalar`, `MiniBossDamageScalar`.
- Remove `ActiveMiniBoss` state and disabled promotion block.
- Confirm Tower guardian `ApplyMiniBossMultipliers` constants remain source of truth.

A4. Clear `Archetype` values `Exploder`, `Stutterer`, `Burrower` on the 12 affected `Enemies.csv` rows to plain family behavior. Confirm no spawn weighting/VFX/UI reads them for behavior before clearing.

A5. Clear `Feeling=MiniBossFeel` on the 6 affected rows. Capture the list first if needed for default mega-mob ordering.

### Section B â€” Vendor Hidden Boss

B1. Repurpose `AT66GamblerBoss` into Vendor boss:
- Rename class/BossID/visual identity from GamblerBoss to VendorBoss where practical in source/data.
- Strip "Gambler as enemy" identity.
- Keep mechanics, hit zones, AOE, token drop.
- Prefer source-file rename if clean; if build/project metadata makes that risky, document exact reason and preserve compatibility wrappers only as needed.

B2. Retarget trigger:
- In existing shop steal flow (`T66CasinoVendorTabWidget::OnStealStop` and underlying run-state steal outcome), ANY failed steal attempt spawns the Vendor boss.
- No threshold.

B3. Remove casino anger boss-spawn path and anger system:
- Remove `TriggerCasinoBossIfAngry` boss spawn path.
- Remove anger meter/state/serialization/config/UI as far as it is enemy/boss-spawn support.
- Keep casino gambling interactable functional.
- Any deeper casino loose ends after anger removal are Pass E notes, not solved here.

B4. Rename `GamblersToken` -> `VendorToken` if any remaining runtime/data names still use GamblersToken. Confirm exactly one hidden boss remains: Vendor.

### Section C â€” Mob-Floor Rename

C1. Rename "gameplay floor" -> "mob floor" across relevant identifiers, strings, logs, and docs, including `T66TowerMapTerrain.*`, `T66GameMode_Tower.cpp`, and dependent call sites.

C2. Confirm/document gate guardians sit only on floor 2->3, 3->4, and 4->5 descent transitions. No behavior change intended.

### Section D â€” Add 10 Mobs And Expand Stages

D1. Expand stage enemy slot schema from EnemyA..EnemyJ to EnemyA..EnemyL:
- Update stage structs/data loading.
- Update `Stages.csv` columns.
- Update stage mob-ID resolver, including `T66ResolveStageMobIDs` or equivalent.

D2. Add 10 `Enemies.csv` rows with `ModelStatus=Placeholder`, `StatusEffectOnHit=None`, copied family/role/colors/XP/rarity/visual/model behavior from named placeholder source:
- Dungeon: Cursed Crow (Flying, placeholder CaveBat); Famished Ghoul (Rush, placeholder RatPack)
- Forest: Will-o-Wisp (Flying, placeholder HiveWasp); Gore Stag (Rush, placeholder TuskerBoar)
- Ocean: Gull Diver (Flying, placeholder GhostRay); Hammerjaw (Melee, placeholder CrabGuard)
- Martian: Recon Orb (Flying, placeholder SaucerDrone); Carapace Brute (Melee, placeholder CrystalCrawler)
- Hell: Cinder Wraith (Flying, placeholder FireSkull); Brimstone Brute (Melee, placeholder BoneKnight)

D3. Populate each theme's stages so all 12 of that theme's mobs are referenced across that theme's 4 stages.

### Section E â€” Mega-Mob Gate Assignment

E1. Replace hardcoded `ConfigureAsMob("Slime")` gate guardian with per-gate mega-mob assignment. Each difficulty/theme gets 12 gate slots (4 stages x 3 gates), each slot gets a distinct one of that theme's 12 mobs. Mega means same model/behavior with `ApplyMiniBossMultipliers`; gate must-kill behavior stays.

E2. Identify the gate index from spawn/descent context. Use data-driven assignment (CSV/DataTable keyed by difficulty+stage+gate -> MobID) if practical; otherwise use a constant map with clear future data-table note.

E3. Use a sensible default mapping that distributes all 12 theme mobs across the 12 slots. Confirm `ConfigureAsMob` resolves model/behavior for every MobID used; report any failure.

## Completion Requirements For Phase 1

Produce:

- `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation\phase1_completion.md`

The completion packet must include:

- Summary by section A-E.
- Exact files changed.
- Deleted files/classes.
- Grep/static checks run and results.
- Any data-table/uasset/editor rebuild deliberately deferred to Phase 2.
- Any blockers or deviations from the approved plan.
- Any Pass E deferred notes.
- Token ledger if available from Claude.

Do not provide only a narrative. The packet must be concrete enough for Codex to validate changed files and decide whether to proceed to Phase 2.

