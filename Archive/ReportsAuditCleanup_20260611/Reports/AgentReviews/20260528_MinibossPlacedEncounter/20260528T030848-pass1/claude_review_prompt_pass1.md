You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260528_MinibossPlacedEncounter\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Claude Review Packet: Miniboss Placed Encounter System

## Working Goal

Replace the random per-wave miniboss promotion with a deliberate placed-encounter system: one scaled-up slime miniboss spawns on the floor-transition exit for normal tower floors 2, 3, and 4, and the player must kill it before using that floor's descent door. This packet is plan/review only; no code has been changed for the implementation.

## User Constraints

- Full replacement, not tuning: random per-wave miniboss promotion must be disabled.
- Minibosses spawn on the exit doors for `2->3`, `3->4`, and `4->5` transitions only.
- No miniboss on `1->2`.
- Boss-only finale stages do not receive minibosses.
- One miniboss type for now: the basic `Slime` mob model, scaled up, same across all stages.
- Minibosses stay rich `AT66EnemyBase`; `bIsMiniBoss` continues to block lightweight routing.
- Minibosses are melee in this pass; no miniboss projectile work.
- Door stays locked while the miniboss is alive and becomes usable after the miniboss dies.
- Functional gameplay smoke test, not FPS capture.
- Do not touch boss projectile manager migration, enemywaveperf finalization, unique miniboss models, specials, B.11+ optimization, or Mini/minigame systems.

## Applicable Repo Instructions

- Root `AGENTS.md`:
  - derive the working goal and inspect live repo state before acting;
  - read folder-owned `*_AGENTS.md`;
  - create an implementation plan before code/doc changes;
  - use Claude review by default;
  - wait for user go-ahead after review before implementation;
  - report exact verification evidence;
  - do not include Mini/minigame scope unless explicitly requested.
- `Source/T66/Gameplay/GAMEPLAY_AGENTS.md`:
  - gameplay runtime changes require compile/build verification and staged standalone validation;
  - read `Gameplay/README.md` and relevant gameplay instruction docs.
- `Gameplay/World/WORLD_AGENTS.md`:
  - tower/floor systems are world-owned; read `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`.
- `Reports/AGENTS.md`:
  - review packets and reviewer outputs belong under `Reports/AgentReviews`.
- Pending issues read:
  - `Source/T66/Gameplay/pending_issues_Gameplay.md`.

## Live Floor And Door System Findings

### Floor Indexing

Live tower floor numbering is fixed by `Source/T66/Gameplay/T66TowerMapTerrain.cpp`:

- `T66TowerStartFloorNumber = 1`
- `T66TowerFirstGameplayFloorNumber = 2`
- `T66TowerLastGameplayFloorNumber = 4`
- `T66TowerBossFloorNumber = 5`

`BuildLayout` assigns floors as:

- floor `1`: start
- floors `2`, `3`, `4`: gameplay
- floor `5`: boss

For normal stages, `Floor.bHasDropHole = Floor.FloorNumber < OutLayout.BossFloorNumber`, so floors 1, 2, 3, and 4 own descent holes. Boss-only finale stages use a 2-floor layout and are handled separately.

### Transition Door Class

The floor-to-floor "door" is not `AT66StageGate`.

- `AT66StageGate` is the stage-to-stage portal used after boss clear and tutorial exit.
- `AT66TowerDescentHole` is the tower floor transition actor.

`Gameplay/World/T66_MAP_DESIGN_REFERENCE.md` also states that tower uses dedicated descent-hole trigger actors for floor-to-floor progression while keeping the existing boss-kill `StageGate` portal for actual stage-to-stage travel.

### Existing Gating Mechanism

`AT66TowerDescentHole` already supports guardian-based gating:

- `InitializeHole(..., bInRequiresGuardianDefeated)` stores `bRequiresGuardianDefeated`.
- `SetGuardianEnemy(AT66EnemyBase*)` stores the guardian weak pointer.
- `Interact()` refuses to open if `CanOpenGate()` fails.
- `CanOpenGate()` refuses if `bRequiresGuardianDefeated && !IsGuardianDefeated()`.
- `IsGuardianDefeated()` returns true when there is no guardian, the guardian has `CurrentHP <= 0`, or the guardian is hidden.

This means the must-kill door gate can reuse the existing `AT66TowerDescentHole` guardian path instead of creating another door system.

### Existing Tower Gate Guardian Path

`Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` already spawns a rich guardian on non-start descent holes:

- `SpawnTowerDescentHolesIfNeeded()` creates descent holes.
- Normal-stage loop currently sets:
  - `bRequiresWeaponSelection = Floor.FloorNumber == StartFloorNumber`
  - `bRequiresGuardianDefeated = Floor.FloorNumber != StartFloorNumber`
- When guardian gating is required, it calls `T66SpawnTowerGateGuardian()` and assigns it with `HoleActor->SetGuardianEnemy(Guardian)`.
- Boss-rush finale stages return early through a separate start-to-boss hole path with `bInRequiresGuardianDefeated=false`.

`T66SpawnTowerGateGuardian()` currently:

- picks a guardian mob from the stage's `EnemyA..EnemyJ` list using `T66PickTowerGateGuardianMob()`;
- resolves a rich class from the mob family;
- spawns an `AT66EnemyBase` rich actor;
- tags it with `T66_Tower_DescentGuardian`;
- sets `bDropsLoot=false`;
- calls `ConfigureAsMob(MobID)`;
- applies stage/difficulty scaling;
- applies current guardian miniboss multipliers `2.75 HP`, `1.65 damage`, `1.9 actor scale`;
- records boss/guardian route attribution;
- snaps and validates the actor on the requested floor.

This path is very close to the requested placed-encounter system, but it is not locked to one slime miniboss type and it has an extra death side effect described below.

## Design Mapping And Assumptions

### Transition Mapping

Implementation assumption: because the user explicitly named `2->3`, `3->4`, and `4->5`, this pass will place minibosses on floors 2, 3, and 4 in the normal five-floor tower layout.

Important wording conflict for review: live code maps `4->5` to "enter the boss floor." The prompt also says "not the boss-floor approach." I interpret the explicit transition list as authoritative and will include the floor 4 descent hole unless Pablo corrects this before implementation. Boss-only finale stages still get no miniboss.

### Spawn Timing

Default implementation: spawn the placed miniboss when tower descent holes are spawned during stage setup, using the existing `SpawnTowerDescentHolesIfNeeded()` path. This positions and locks all qualifying exit holes from the beginning of the stage.

This differs slightly from the prompt phrase "on entering a gameplay floor," but it is safer with the existing architecture because:

- the descent holes and guardians are already created together;
- the guardian pointer can be assigned before the player reaches the floor;
- current floor population can already exist before arrival;
- the door gate does not need a new floor-entry event or late-spawn race.

Smoke verification will validate the player-facing behavior: no miniboss on `1->2`, visible scaled slime on floors 2/3/4 exits, and no progression until the guardian dies.

### Slime MobID

`Content/Data/Enemies.csv` has a basic `Slime` row:

- `MobID=Slime`
- `FamilyID=Melee`
- `Archetype=MowDown`
- `ThemeID=Dungeon`
- `MeshStatus=MeshReady`

This is the placeholder miniboss MobID for this pass. It is a basic mob row, not the `Dungeon_SewerSlimeKing` boss from `Content/Data/Bosses.csv`.

### Scaling

Use the existing random-miniboss scaling values as the placed-miniboss default:

- `MiniBossHPScalar = 3.0f`
- `MiniBossDamageScalar = 2.0f`
- `MiniBossScale = 1.75f`

Those are currently declared on `AT66EnemyDirector` and applied to wave-promoted minibosses. The implementation should avoid depending on the director at tower-gate spawn time; use local named constants or a small shared helper if that is cleaner. The existing tower-gate guardian values (`2.75f`, `1.65f`, `1.9f`) will be replaced so there is only one miniboss tuning source for this pass.

### Guardian Death Side Effect

Current `AT66GameMode::HandleTowerGateGuardianDefeated()` spawns an idol altar at the guardian death location, snaps it to the floor, and syncs the miasma source anchor. The user requested "kill miniboss to unlock the door"; they did not request an idol altar reward. This plan treats the idol altar spawn as legacy guardian behavior and removes or disables it for the placed slime miniboss unless Pablo explicitly asks to keep it.

This is the largest gameplay-side assumption in the packet. Keeping the altar would preserve current behavior but adds an unrequested reward/interaction to the new placed miniboss system.

## Current Random Promotion Model To Replace

`Source/T66/Gameplay/T66EnemyDirector.h` currently defines:

- `MiniBossChancePerWave = 0.10f`
- `MiniBossScale = 1.75f`
- `MiniBossHPScalar = 3.0f`
- `MiniBossDamageScalar = 2.0f`

`Source/T66/Gameplay/T66EnemyDirector.cpp` currently:

- chooses a miniboss plan slot before final mob identity is rolled;
- rolls `MiniBossWaveSpawned` at `MiniBossChancePerWave`;
- picks one regular mob slot as `MiniBossIndex`;
- later rolls final `MobID`;
- sets `Slot.bIsMiniBoss = bIsMiniBossSlot`;
- routes `bIsMiniBoss` rich;
- applies miniboss multipliers at spawn and assigns `ActiveMiniBoss`.

B.10.1D Resume5 proved this is family-neutral and can promote a final Ranged `MobID`, producing a planned rich route during CVar-on lightweight acceptance. This pass removes that model from normal trickle waves.

## Proposed Implementation Plan

### 1. Disable Random Per-Wave Miniboss Promotion

Files:

- `Source/T66/Gameplay/T66EnemyDirector.h`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`

Plan:

- Remove or hard-disable the `MiniBossIndex` chance roll in `BuildSpawnPlan` / wave planning.
- Ensure `Slot.bIsMiniBoss` is never set by the trickle-wave promotion path.
- Keep `ApplyMiniBossMultipliers()` on `AT66EnemyBase`.
- Retain old tuning constants only if needed for compatibility, but mark wave miniboss promotion as disabled/replaced by placed tower miniboss encounters. Avoid deleting broad UPROPERTY state if that would risk Blueprint/default serialization churn.
- Preserve route attribution enums for historical diagnostics unless removing them is clearly safe; no need to churn PerformanceSystem parsing in this pass.

Expected result:

- No random miniboss appears in normal wave/trickle spawns.
- `enemywaveperf` naturally stops seeing random rich miniboss promotions.

### 2. Convert Tower Gate Guardians Into Fixed Slime Minibosses

File:

- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`

Plan:

- Replace `T66PickTowerGateGuardianMob()` stage-roster selection with a fixed placed-miniboss MobID, `Slime`.
- Rename local helper/log wording if practical (`Tower gate guardian` can remain as an internal compatibility term, but logs/docs should call it a placed miniboss).
- Resolve the rich enemy class through the existing family resolver using `Slime`, which should produce a rich melee enemy class.
- Call `ConfigureAsMob(Slime)`.
- Apply stage scaling and difficulty scalar as current guardians already do.
- Apply placed miniboss multipliers `3.0f HP`, `2.0f damage`, `1.75f scale`.
- Keep `bDropsLoot=false` unless the design later requests loot.
- Keep the `T66_Tower_DescentGuardian` tag unless a rename is low-risk; it is already used by the death hook.
- Keep actor registry behavior via `AT66EnemyBase::BeginPlay()`.
- Keep route attribution as boss/guardian rich route so lightweight acceptance can identify it as planned rich special/miniboss traffic if relevant.

Expected result:

- The same scaled Slime miniboss appears on each qualifying exit across all normal stages.

### 3. Tighten Qualifying Floor Rule

File:

- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`

Plan:

- In the normal-stage descent-hole loop, replace the broad `Floor.FloorNumber != StartFloorNumber` guardian requirement with an explicit gameplay-floor check:
  - `Floor.FloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber`
  - `Floor.FloorNumber <= CachedTowerMainMapLayout.LastGameplayFloorNumber`
  - `Floor.bHasDropHole`
- In the current five-floor normal layout this maps exactly to floors 2, 3, and 4.
- Keep boss-rush finale path guardian-free.
- Keep start floor `1->2` weapon-selection behavior and no miniboss.

Expected result:

- Miniboss on floors 2, 3, 4.
- No miniboss on floor 1.
- No miniboss on boss-only finale stages.

### 4. Reuse Existing Must-Kill Gate

Files:

- likely no edit needed in `Source/T66/Gameplay/T66TowerDescentHole.*`
- maybe local log/feedback edits in `T66GameMode_Tower.cpp` only

Plan:

- Continue passing `bRequiresGuardianDefeated=true` for qualifying descent holes.
- Continue assigning the spawned miniboss with `HoleActor->SetGuardianEnemy(Guardian)`.
- Treat "unlock" as: once the miniboss is dead, `AT66TowerDescentHole::CanOpenGate()` returns true and the next player interaction opens the gate and triggers descent.
- Do not create a new door class or parallel state.

Expected result:

- The player cannot interact/use the hole while the miniboss is alive.
- After kill, the existing gate interaction proceeds.

### 5. Remove Or Disable Legacy Idol-Altar On Guardian Death

File:

- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`

Plan:

- Update `HandleTowerGateGuardianDefeated()` so placed slime miniboss death does not spawn an idol altar unless review/user asks to preserve that behavior.
- Keep useful logging that the placed miniboss was defeated and floor gate is now eligible to open.
- Avoid miasma-anchor side effects unless a remaining tower mechanic needs them; if removing this is risky, gate it narrowly and document.

Expected result:

- The miniboss encounter's functional reward is progression through the locked descent hole, not an extra altar.

### 6. Documentation

Files:

- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- final combined packet under `Reports/AgentReviews/20260528_MinibossPlacedEncounter/`

Plan:

- Document the placed miniboss rule:
  - normal tower floors 2, 3, 4 only;
  - fixed Slime placeholder;
  - rich actor, not lightweight;
  - must kill to use descent hole;
  - no random wave promotion;
  - unique miniboss creatures are future work.
- Update the pending issue that currently tracks family-neutral miniboss promotion as replaced/resolved by the placed-encounter model.
- Note that `enemywaveperf` is naturally free of random miniboss promotions after this change.
- Deliver one combined completion packet after implementation/verification, per the user's standing preference.

## Intended Edit Scope

Allowed:

- `Source/T66/Gameplay/T66EnemyDirector.h`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`
- `Gameplay/World/T66_MAP_DESIGN_REFERENCE.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Reports/AgentReviews/20260528_MinibossPlacedEncounter/*`

Possible but avoid unless needed:

- `Source/T66/Gameplay/T66TowerDescentHole.*` for logging/feedback only if the existing gate cannot express the smoke test.

Out of scope:

- `AT66StageGate` changes.
- Boss projectile migration.
- Special enemy systems (`Goblin`, `UniqueDebuff`, `Gambler`).
- Unique miniboss models/data rows.
- Ranged miniboss/projectile mechanics.
- enemywaveperf harness or performance captures.
- B.11+ optimization work.
- Mini/minigame systems.

## Risks And Mitigations

- Risk: Prompt wording conflicts around `4->5` vs "not boss-floor approach."
  - Mitigation: Treat explicit `4->5` list as authoritative in this plan; call the conflict out before implementation.
- Risk: Current guardian-death idol altar may be intentional progression/reward.
  - Mitigation: Call it out in review; remove only as part of this explicitly reviewed placed-miniboss replacement.
- Risk: Disabling `MiniBossChancePerWave` might leave stale editor-exposed properties.
  - Mitigation: Prefer behavior disable plus comments over broad property deletion; cleanup can happen later.
- Risk: Slime is a Dungeon-themed row but user wants same miniboss across all stages.
  - Mitigation: This is explicitly locked design for now; document as placeholder future-authoring debt.
- Risk: Smoke testing all floor transitions is time-consuming.
  - Mitigation: Use staged standalone, logs, and Unreal-owned screenshots; if manual kill/progression takes too long, use existing dev/automation controls only if already present and document the exact method.
- Risk: Existing gate cover only visually opens on interaction, not immediately on miniboss death.
  - Mitigation: Treat "unlocked" as gate eligibility after death; smoke test confirms interaction succeeds after kill. If immediate visual opening is desired, that is a small follow-up or an explicit implementation addition.

## Verification Plan

After implementation:

1. Focused compile/build:
   - Run the repo's standard focused Unreal build path for gameplay C++ changes.
   - If the first two attempts fail with the same signature, stop repetition and diagnose per root instructions.
2. Staged standalone:
   - Refresh staged standalone with `Scripts\StageStandaloneBuild.ps1`.
   - Verify the taskbar standalone target remains `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
3. Functional smoke in staged standalone:
   - Stage 1 Dungeon normal tower.
   - Confirm floor `1->2`: no miniboss, normal descent after weapon requirement.
   - Confirm floor 2 exit: scaled Slime miniboss present on/near descent hole; descent interaction fails while alive.
   - Kill miniboss; confirm descent interaction succeeds to floor 3.
   - Repeat for floor 3 and floor 4.
   - Confirm no random minibosses appear in normal trickle waves.
   - Confirm boss-only/finale path receives no placed miniboss if a quick deterministic entry route exists; otherwise document why this specific check was skipped.
4. Evidence artifacts:
   - Log: `Saved/StandaloneLogs/T66_MinibossPlacedEncounter_Smoke.log`.
   - Unreal-owned screenshots under `Saved/Codex/Gameplay/MinibossPlacedEncounter/`.
   - Combined completion packet under `Reports/AgentReviews/20260528_MinibossPlacedEncounter/completion_packet.md`.

## Review Request

Please review this plan as a read-only reviewer. Focus on:

- whether the `AT66TowerDescentHole` guardian seam is the correct implementation target;
- whether the floor mapping should include `4->5` given the prompt wording conflict;
- whether disabling the guardian idol-altar death side effect is safe or needs user confirmation;
- whether the random miniboss promotion disable plan is sufficient to prevent wave minibosses;
- whether any required gameplay docs, pending issues, or validation gates are missing;
- whether the plan accidentally touches out-of-scope special/boss/Mini/performance systems.

First non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

</review_packet>
