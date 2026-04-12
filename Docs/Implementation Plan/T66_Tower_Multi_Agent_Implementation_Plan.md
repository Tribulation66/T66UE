# T66 Tower Multi-Agent Implementation Plan

Date: 2026-04-12  
Workspace: `C:\UE\T66`

This document is the execution pack for the current tower-expansion work. It is written so you can hand it to another agent with a direct instruction such as:

- `You are Agent 2. Read Docs/Implementation Plan/T66_Tower_Multi_Agent_Implementation_Plan.md and implement your part only.`

This plan intentionally splits the work into four parallel passes. Agent 1 is the integration owner.

## 1. Canonical Sources

Before implementing any part, every agent must treat these files as the source of truth:

- `MASTER DOCS/T66_MASTER_GUIDELINES.md`
- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `MASTER DOCS/MASTER_TRAPS.md`
- `Docs/Plans/T66_UI_IMPLEMENTATION_PLAN.md`
- `Scripts/StageStandaloneBuild.ps1`

Relevant project rules pulled forward from the master docs:

- T66 is tower-only at runtime. Do not design around the removed `Hilly` / `Flat` map flows.
- Packaged Development standalone is the runtime source of truth, not PIE.
- Runtime-facing changes must be propagated to the staged standalone deliverable.
- Multiple agents are assumed to be working in parallel. Do not overwrite or revert another agent's work.
- Shared helpers should be centralized, not duplicated into multiple `.cpp` files.

## 2. Locked Target State

These decisions are already made for this feature set.

### 2.1 Tower floor naming and structure

- The tower must become:
  - `Start Level`
  - gameplay `Level 1`
  - gameplay `Level 2`
  - gameplay `Level 3`
  - gameplay `Level 4`
  - gameplay `Level 5`
  - `Boss Level`
- Internal physical floor numbering should remain sequential for stability.
- Recommended mapping:
  - internal floor `1` = `Start Level`
  - internal floors `2..6` = display `Level 1..5`
  - internal floor `7` = `Boss Level`

### 2.2 Theme mapping

- `Easy -> Level 1 -> Dungeon`
- `Medium -> Level 2 -> Forest`
- `Hard -> Level 3 -> Ocean`
- `Very Hard -> Level 4 -> Martian`
- `Impossible -> Level 5 -> Hell`

The gameplay layout remains the same across all gameplay levels:

- same square tower footprint
- same maze-style internal wall structure
- different floor textures
- different roof textures
- different wall object families

### 2.3 Frontend preview behavior

- Hero selection and companion selection should show one active preview scene.
- The active preview changes when selected difficulty changes.
- The preview must visibly show the floor theme and wall objects for the corresponding gameplay level.

### 2.4 Trap direction

- Start and boss floors stay trap-free for now.
- Gameplay levels `1..5` each need their own trap identity.
- The trap system must become floor-driven, not selected-run-difficulty-driven.
- Level 1 already has its current dungeon traps and must add `Floor Spike Patch` as the third trap.

### 2.5 Current runtime state from the master docs

Current live runtime, before this pass set:

- tower is `1` start floor + `4` gameplay floors + `1` boss floor
- tower visuals are currently only stage-aware:
  - Stage 1 = dungeon
  - Stage 2 = forest
- current trap runtime only spawns:
  - `1` wall-arrow trap per gameplay floor
  - `1` floor-flame trap per gameplay floor

## 3. Shared Technical Contract

All agents must use the same naming so their work can merge cleanly.

### 3.1 Shared floor metadata contract

Agent 1 owns the final implementation of this contract, but all agents should code to these names if they need to reference them:

- `ET66TowerFloorRole`
  - `Start`
  - `Gameplay`
  - `Boss`
- `ET66TowerGameplayLevelTheme`
  - `Dungeon`
  - `Forest`
  - `Ocean`
  - `Martian`
  - `Hell`
- `GameplayLevelNumber`
  - valid only on gameplay floors
  - expected range `1..5`

Recommended floor metadata fields on `T66TowerMapTerrain::FFloor`:

- `FloorRole`
- `GameplayLevelNumber`
- `Theme`

### 3.2 Shared helper contract

Expected shared helpers:

- floor display naming helper
  - returns `Start Level`, `Level N`, or `Boss Level`
- tower gameplay theme resolver
  - resolves theme from gameplay level number
- difficulty-to-tower-level/theme helper
  - used by frontend preview and any menu logic that needs the themed scene

### 3.3 Shared runtime validation rule

Every agent may validate in editor while iterating, but runtime acceptance is packaged Development standalone.

Final staged runtime path:

- `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`

Repo launch shortcut:

- `C:\UE\T66\T66 Standalone.lnk`

User external shortcut to verify during final integration:

- `C:\Users\DoPra\Desktop\T66.uproject - Shortcut.lnk`

Important:

- Agent 1 owns the final shared standalone refresh and shortcut verification.
- Agents 2-4 should still locally validate packaged/runtime-safe behavior when their pass affects runtime behavior.

## 4. Ownership Model

This is the file ownership split. Agents should stay inside their area unless blocked.

### Agent 1

Owns:

- tower floor-count expansion
- tower naming
- tower floor metadata
- shared tower theme contract
- hardcoded gameplay-floor-count refactors
- final integration
- master-doc updates
- standalone and shortcut sync

Primary files:

- `Source/T66/Gameplay/T66TowerMapTerrain.h`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- `Source/T66/Gameplay/T66GameMode.cpp`
- any new shared tower-theme helper file Agent 1 chooses to add

### Agent 2

Owns:

- tower theme asset hookup
- wall families
- ground/roof material hookup
- per-floor ceiling/roof support
- cook/staging coverage for new tower theme assets

Primary files:

- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- any new tower-theme asset helper file
- relevant packaging/config updates if new runtime-loaded assets require them

### Agent 3

Owns:

- hero selection difficulty preview
- companion selection difficulty preview
- preview-stage environment buildout
- preview-mode separation from run-summary capture

Primary files:

- `Source/T66/Gameplay/T66PreviewStageEnvironment.h`
- `Source/T66/Gameplay/T66PreviewStageEnvironment.cpp`
- `Source/T66/Gameplay/T66HeroPreviewStage.h`
- `Source/T66/Gameplay/T66HeroPreviewStage.cpp`
- `Source/T66/Gameplay/T66CompanionPreviewStage.h`
- `Source/T66/Gameplay/T66CompanionPreviewStage.cpp`
- `Source/T66/Gameplay/T66FrontendGameMode.cpp`
- `Source/T66/UI/Screens/T66HeroSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66CompanionSelectionScreen.cpp`
- `Source/T66/UI/Screens/T66RunSummaryScreen.cpp`

### Agent 4

Owns:

- trap tuning/config path
- trap pool system
- trap family refactor
- maze-wall trap spawn helpers
- floor-tile-center trap spawn helpers
- `Floor Spike Patch`
- per-level trap pool definitions

Primary files:

- `Source/T66/Core/T66TrapSubsystem.h`
- `Source/T66/Core/T66TrapSubsystem.cpp`
- `Source/T66/Gameplay/Traps/*`
- `Source/T66/Gameplay/T66TowerMapTerrain.h`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- any new trap tuning config files under `Config/`

## 5. Merge and Conflict Rules

- Agent 1 is the integration owner.
- Agents 2-4 should not mass-edit `MASTER DOCS/*` in parallel. Instead, each agent should leave clear handoff notes in their branch or final summary for Agent 1 to consolidate into the master docs.
- Agents 2 and 4 both touch `T66TowerMapTerrain.*`. Keep edits narrowly scoped:
  - Agent 2 owns visual/theme sections.
  - Agent 4 owns trap spawn surface query helpers.
  - Agent 1 owns floor metadata/layout/traversal sections.
- If a new shared helper is needed, add it once with the canonical name above instead of cloning similar logic.

## 6. Agent 1 Instructions

Agent 1 is the spine owner and final integration owner.

### 6.1 Goal

Refactor the tower from `1 + 4 + 1` floors into `1 + 5 + 1`, establish the new naming model, add explicit floor metadata, and create the shared theme contract the other passes depend on.

### 6.2 Core deliverables

- Expand tower layout to `7` floors.
- Add `Start Level` / `Level 1..5` / `Boss Level` display naming.
- Add explicit floor role and gameplay-level metadata.
- Replace hardcoded assumptions that gameplay floors are only `2..5`.
- Provide shared theme-resolution helpers.
- Integrate Agent 2, 3, and 4 work after their branches are ready.
- Update master docs after integration.
- Refresh standalone build and verify shortcut behavior after integration.

### 6.3 Detailed tasks

1. Refactor `T66TowerMapTerrain::FLayout`
- Change:
  - `LastGameplayFloorNumber` from `5` to `6`
  - `BossFloorNumber` from `6` to `7`
- Update `BuildLayout(...)` to build `7` floors instead of `6`.
- Keep physical floor numbering sequential and deterministic.

2. Extend `T66TowerMapTerrain::FFloor`
- Add explicit metadata for:
  - floor role
  - gameplay level number
  - theme
- Ensure these values are assigned during layout build, not patched later.

3. Add display-name helpers
- One helper should return:
  - `Start Level`
  - `Level 1`
  - `Level 2`
  - `Level 3`
  - `Level 4`
  - `Level 5`
  - `Boss Level`
- Do not globally rename unrelated `Stage` usage. Many `Stage` references are run progression, not tower floors.

4. Remove hardcoded 4-floor assumptions
- Update any `TInlineAllocator<4>` or equivalent fixed gameplay-floor assumptions.
- Recheck:
  - tower interactable distribution
  - traversal-hole linking
  - cowardice gate placement
  - enemy/trap/per-floor scatter logic
  - any start of combat logic keyed on first gameplay floor

5. Add shared theme resolver contract
- Canonical mapping:
  - `1 -> Dungeon`
  - `2 -> Forest`
  - `3 -> Ocean`
  - `4 -> Martian`
  - `5 -> Hell`
- Make this reusable by gameplay and frontend preview code.

### 6.4 Non-goals

- Do not own trap implementation.
- Do not own preview-scene dressing.
- Do not own material-instance hookup.

### 6.5 Validation

- Tower traversal must be:
  - `Start Level -> Level 1 -> Level 2 -> Level 3 -> Level 4 -> Level 5 -> Boss Level`
- Floor classification and minimap floor selection must remain correct.
- No gameplay system should still assume only four gameplay floors.

### 6.6 Final integration responsibilities

After Agents 2-4 are merged or rebased in:

1. Update master docs:
- `MASTER DOCS/MASTER_MAP_DESIGN.md`
- `MASTER DOCS/MASTER_TRAPS.md`
- any other master doc affected by the final integrated result

2. Refresh standalone build:
- run `Scripts/StageStandaloneBuild.ps1`
- validate output at `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`

3. Verify shortcuts:
- verify `C:\UE\T66\T66 Standalone.lnk`
- inspect `C:\Users\DoPra\Desktop\T66.uproject - Shortcut.lnk`
- if the external shortcut is intended to launch the standalone build, repoint it to the latest staged executable in the same pass
- if the external shortcut is intentionally an editor/project shortcut, leave it alone and report that distinction in the handoff

## 7. Agent 2 Instructions

Agent 2 owns Pass 2: tower visual theme hookup.

### 7.1 Goal

Make each gameplay level visually represent its own difficulty theme while preserving the shared maze layout.

### 7.2 Core deliverables

- Per-floor floor-material selection
- Per-floor roof-material selection
- Per-floor wall-family selection
- Support for Ocean, Martian, and Hell content
- Dedicated ceiling/roof pass per gameplay floor if required

### 7.3 Detailed tasks

1. Replace whole-tower theme switching
- Current code only resolves a single tower theme for the stage.
- Replace that with per-floor theme selection using Agent 1's shared theme contract.

2. Hook up wall families
- `Dungeon`:
  - existing solid dungeon wall path
- `Forest`:
  - existing tree-wall path
- `Ocean`:
  - seaweed wall family
- `Martian`:
  - spine / mineral / basalt wall family
- `Hell`:
  - visual fire wall family with separate collision strategy if needed

3. Separate visuals from collision where needed
- Hell wall visuals should not depend on fire itself being the gameplay blocker.
- If a visual wall family cannot safely serve as collision, use a hidden blocker and separate visible VFX/mesh actors.

4. Add real per-floor ceiling support
- The current tower only has one dedicated top roof.
- If each gameplay level must have its own roof texture, add dedicated ceiling slabs or an equivalent per-floor roof solution.

5. Asset and cook contract
- Any new runtime-loaded tower assets must be cook-safe and package-safe in the same pass.
- Do not rely on editor-only loose file access.

### 7.4 Non-goals

- Do not own tower floor numbering or naming.
- Do not own hero/companion preview scenes.
- Do not own trap system changes.

### 7.5 Validation

- Easy gameplay floor reads as dungeon.
- Medium gameplay floor reads as forest.
- Hard gameplay floor reads as ocean.
- Very Hard gameplay floor reads as martian.
- Impossible gameplay floor reads as hell.
- Start and boss floors remain valid and visually intentional.
- Runtime works in packaged standalone, not only editor.

### 7.6 Agent 2 handoff note for Agent 1

When finished, provide Agent 1 with:

- exact new asset paths used
- any packaging/cook changes made
- whether Hell walls use split collision vs visual representation

## 8. Agent 3 Instructions

Agent 3 owns Pass 3: frontend difficulty preview scenes.

### 8.1 Goal

Make hero selection and companion selection show the correct environment preview for the currently selected difficulty, matching the new tower theme mapping.

### 8.2 Core deliverables

- one active preview scene that changes with selected difficulty
- visible floor and wall objects in the preview
- shared mapping with gameplay tower themes
- preview-mode split so run-summary capture does not inherit the selection-scene dressing

### 8.3 Detailed tasks

1. Replace the current lightweight preview environment
- The current preview environment only swaps floor material and easy-only props.
- Build a small tower vignette instead:
  - themed floor slab
  - 2-3 visible wall segments
  - optional ceiling element if it improves framing

2. Wire it to selected difficulty
- Easy shows Level 1 dungeon look.
- Medium shows Level 2 forest look.
- Hard shows Level 3 ocean look.
- Very Hard shows Level 4 martian look.
- Impossible shows Level 5 hell look.

3. Keep camera framing character-driven
- Do not let backdrop walls or ceiling drive camera framing.
- The preview should still frame the hero/companion cleanly.

4. Add preview-mode separation
- `AT66HeroPreviewStage` is reused by run summary.
- Add a mode or equivalent guard so selection-scene environment dressing does not leak into run-summary capture.

### 8.4 Non-goals

- Do not spawn the full tower in frontend.
- Do not own runtime tower floor expansion.
- Do not own trap placement or trap preview.

### 8.5 Validation

- Changing difficulty in hero selection immediately updates the preview scene.
- Companion selection uses the same themed environment logic.
- Run summary still renders correctly after the preview-mode split.
- Standalone frontend behavior matches editor behavior.

### 8.6 Agent 3 handoff note for Agent 1

When finished, provide Agent 1 with:

- any new preview-mode enum/flag name
- any shared helper dependency on Agent 1's theme contract
- any packaged standalone differences observed during validation

## 9. Agent 4 Instructions

Agent 4 owns Pass 4: tower trap identity by gameplay level.

### 9.1 Goal

Turn the current first-pass trap runtime into a level-driven, pool-based trap system with three traps per gameplay level and proper spawn counts.

### 9.2 Core deliverables

- config-backed trap tuning surface
- code-side trap registry
- per-level trap pools with min/max counts
- maze-wall trap spawn helper
- floor-tile-center trap spawn helper
- `Floor Spike Patch`
- updated Level 1 trap set

### 9.3 Detailed tasks

1. Refactor spawn ownership
- Keep trap spawning in `UT66TrapSubsystem`.
- Do not push trap logic into `AT66GameMode`.

2. Add tuning path
- Create a dedicated trap tuning config path similar to the current config-backed tuning systems.
- Use config for numeric tuning and counts.
- Keep class wiring in code, not config.

3. Fix wall trap placement
- Current wall trap placement uses outer-shell sampling and is a likely reason traps are hard to notice.
- Add `TryGetMazeWallSpawnLocation(...)` using `Floor.MazeWallBoxes`.

4. Add tile-center floor spawn helper
- Add a helper that picks valid tile centers from the same floor tile grid the tower already uses.
- Use this for floor-local traps like `Floor Spike Patch`.

5. Refactor trap families
- Establish reusable families:
  - wall projectile
  - floor burst
  - area control
- Current `WallArrow` and `FloorFlame` should be folded into this model, even if their class names stay for compatibility.

6. Implement `Floor Spike Patch`
- Level 1 third trap
- floor telegraph
- spikes rise
- damage pulses while raised
- retract and cooldown
- initial implementation can use Unreal primitive meshes before final Blender art

7. Add per-level trap pools
- Level 1 includes:
  - current dungeon wall trap
  - current dungeon floor burst trap
  - `Floor Spike Patch`
- Levels 2-5 each get three trap identities based on their level theme

### 9.4 Non-goals

- Do not own tower floor naming.
- Do not own tower visual theme asset hookup.
- Do not own hero/companion preview scenes.

### 9.5 Validation

- Start and boss floors remain trap-free.
- Wall traps spawn on interior maze walls, not just shell walls.
- Floor traps spawn on readable tile centers.
- Level 1 clearly shows all three trap types.
- Trap damage still routes through `UT66RunStateSubsystem::ApplyDamage()`.
- Packaged standalone behavior matches runtime expectations.

### 9.6 Agent 4 handoff note for Agent 1

When finished, provide Agent 1 with:

- new config filename and section names
- trap registry key names
- any new trap classes added
- any remaining placeholder art dependencies for Levels 2-5

## 10. Final Acceptance Checklist

The integrated result is complete only when all of the following are true:

- tower uses `Start Level`, gameplay `Level 1..5`, and `Boss Level`
- there are `5` gameplay floors, not `4`
- each gameplay floor has the intended theme mapping
- hero selection preview matches the selected difficulty's gameplay level theme
- companion selection preview matches the selected difficulty's gameplay level theme
- run summary preview remains correct
- trap system is gameplay-level-driven, not selected-run-difficulty-driven
- Level 1 includes `Floor Spike Patch`
- packaged Development standalone is refreshed
- `T66 Standalone.lnk` launches the latest staged runtime
- the user's external shortcut has been checked and either updated or intentionally left alone with a clear explanation

## 11. Short Agent Prompt Template

Use this exact handoff style when dispatching another agent:

`You are Agent X. Read C:\\UE\\T66\\Docs\\Implementation Plan\\T66_Tower_Multi_Agent_Implementation_Plan.md and implement only the Agent X section. Do not take ownership of other agents' files.`
