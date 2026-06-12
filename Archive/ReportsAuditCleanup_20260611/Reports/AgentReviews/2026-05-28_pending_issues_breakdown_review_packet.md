# Pending Issues Breakdown Review Packet

## Working Goal

Inventory repo pending-issues files in `C:\UE\T66` and summarize outstanding issues by game area, excluding Mini/minigame scope unless explicitly included.

## User Request

"I want you to go through all the pending issues across the folders, and give me a breakdown of the issues by area of the game"

## Assumptions And Constraints

- No clarification needed. The request is read-only and asks for a breakdown, not edits.
- Root `AGENTS.md` default Mini/minigame exclusion applies. I excluded `Source\T66Mini\pending_issues_T66Mini.md` and did not inspect Mini/minigame folders.
- Active breakdown separates current active issues from archived/resolved/superseded notes.
- No PPF check is required because this is a read-only inventory/report answer, not visual/media/VFX/import implementation.
- No build, cook, staged build, or runtime validation is needed because no source/content was changed.

## Repo Instructions Read

- `AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/World/WORLD_AGENTS.md`
- `Model Generation/MODEL_GENERATION_AGENTS.md`
- `Model Generation/Pixal3D/PIXAL3D_AGENTS.md`
- `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md`
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`
- `UI/UI_AGENTS.md`
- `Reports/AGENTS.md`

## Evidence Gathered

- `Get-ChildItem -Path . -Filter '*AGENTS.md' -Recurse -File`
- `Get-ChildItem -Path . -Filter 'pending_issues_*.md' -Recurse -File`
- Parsed all non-Mini `pending_issues_*.md` files.
- Count result including H1-only `UI/Checklists/pending_issues_Checklists.md`:
  - 21 pending files found total.
  - 1 Mini-owned file excluded.
  - 20 non-Mini pending files inspected.
  - 66 non-Mini issue/note entries parsed.
  - 51 active entries.
  - 15 archived/resolved/superseded entries.
  - Active severity split: 1 Blocker, 21 Major, 29 Minor.
  - `Content/Maps/pending_issues_Maps.md` reports no active map issues.

## Final Answer Draft To Review

I found 51 active non-Mini pending entries across 20 inspected pending files. Severity split: 1 Blocker, 21 Major, 29 Minor. I excluded `Source\T66Mini\pending_issues_T66Mini.md` under the root Mini/minigame opt-out rule, and `Content\Maps\pending_issues_Maps.md` currently says there are no map issues.

**Gameplay / Combat / Enemies**
- Highest priority is the Ranged autocapture blocker: basic-mob acceptance still needs a fresh staged capture pass after the placed-miniboss routing change, because CVar-on route validity previously leaked planned rich routes. See `Source/T66/Gameplay/pending_issues_Gameplay.md:17`.
- Enemy mechanics are still partly fallback-driven: missing Exploder/Stutterer/Burrower classes, spawn director still using fallback families, enemy family/role/archetype redundancy, and Hell having no ranged core mob. See `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md:3`, `Source/T66/Data/pending_issues_Data.md:3`, and `Source/T66/Gameplay/pending_issues_Gameplay.md:108`.
- Combat authority/debuggability needs cleanup: abstract damage sources are not all visual damage volumes, touch damage is split across old overlap and hero proximity logic, and routine trap projectile logs can pollute captures. See `Source/T66/Gameplay/pending_issues_Gameplay.md:50`, `:94`, and `:101`.
- Hero 1 AOE VFX has mostly polish/tooling debt now: rotation is intentionally frozen for visual lock, evidence-frame selection is manual, final visual polish is deferred, and the lab actor uses a deprecated Niagara readiness API. See `Gameplay/Combat/pending_issues_Combat.md:31` and `Source/T66/Gameplay/pending_issues_Gameplay.md:3`.

**World / Environment / Materials**
- Tower/environment presentation has seams and incomplete theme authoring: inter-walkable floor seams can remain, drop-hole floors are split around openings, and Forest/Ocean/Martian/Hell atmosphere specs are still neutral. See `Source/T66/Gameplay/pending_issues_Gameplay.md:129`, `:136`, and `:143`.
- Cleanup-sensitive world/material issues are mostly preservation/decision items: Backrooms textures are intentionally kept despite zero-referencer audits, legacy MainMapTerrain dirt/grass/rock names are dormant but confusing, and the view-space character master material still needs a visual-lock decision. See `Content/World/pending_issues_World.md:3`, `Source/T66/Gameplay/pending_issues_Gameplay.md:10`, and `Content/Materials/pending_issues_Materials.md:3`.

**Data / Saves / Runtime References**
- Data readiness issues include production mobs not using status effects, staged smoke logs missing data/assets/item rows, PlayerExperience tuning sometimes requested before its DataTable is loaded, legacy lab unlock IDs in saves, skeletal hero scaling ignoring `MeshRelativeScale`, and missing QuadRetro mob pixel textures. See `Content/Data/pending_issues_Data.md:3`, `:10`, `Source/T66/Gameplay/pending_issues_Gameplay.md:122`, and `Source/T66/Core/pending_issues_Core.md:3`.
- Naming cleanup remains in runtime interaction classes: NPC code still uses HouseNPC names, vehicle code still uses Tractor names, and `T66TutorialGate` remains after tutorial exits moved to StageGate. See `Source/T66/Gameplay/pending_issues_Gameplay.md:150`, `:157`, and `:164`.

**Model Generation / ToonStyle / Animation**
- Pixal3D has the densest Major cluster: Loot Bag Yellow and Slime fail `fill_holes` at locked settings, Loot Chest/Crate no-remesh topology can misbehave on outlines, safe-fill-holes outputs still need topology review, and manual camera mode is not wired through the endpoint/wrappers. See `Model Generation/Pixal3D/pending_issues_Pixal3D.md:3`, `:13`, `:24`, `:33`, and `:43`.
- Runtime animation/visual validation has two broader gates: VAT mob crowd performance is not proven at hundreds of enemies, and ToonStyle still has vertex-color alpha masking unavailable in outline WPO plus a cached include-path cook warning. See `Model Generation/Rigging and Animation/pending_issues_rigging_and_animation.md:5` and `ToonStyle/Source/pending_issues_Source.md:3`.

**Performance / Diagnostics**
- The active performance risks are measurement integrity: staged executable provenance can drift mid-diagnostic, Git/LFS scans can contaminate capture timing, and Source-side PerformanceSystem still has an intermittent framework overhead spike in Phase C.1 regression evidence. See `PerformanceSystem/pending_issues_PerformanceSystem.md:21`, `:35`, and `Source/T66/PerformanceSystem/pending_issues_PerformanceSystem.md:3`.

**UI / Frontend / Rewards**
- UI has four active items: no central controller-focus contract for frontend screens, loot wheel boost results lack a focused toast/card, WidgetGames still lives in the main T66 module, and the Main Menu checklist baseline is stale after layout changes. See `Source/T66/UI/pending_issues_UI.md:3`, `:10`, `Source/T66/Public/UI/WidgetGames/pending_issues_WidgetGames.md:3`, and `UI/Checklists/pending_issues_Checklists.md:1`.

**Audio**
- Staged gameplay smoke references missing audio assets/SoundClass/theme paths. Fixing it means choosing canonical music/SFX assets and restaging with audio enabled. See `Content/Audio/pending_issues_Audio.md:3`.

**Import / Capture / Workflow Tooling**
- Import tooling still has several Major blockers: static GLB helpers are still shared by active workflows, headless Interchange import can crash after saving, full-editor GLB import can return no assets, and Pixal3D FBX fallback emits extensionless WebP textures. See `Scripts/pending_issues_Scripts.md:3`, `:10`, `:17`, and `:24`.
- Capture/evidence tooling also has debt: gameplay HUD capture uses an invalid widget dump target, frontend capture has no tag-click step, and VFX evidence bundles still require fixed/manual frame labels. See `Scripts/pending_issues_Scripts.md:31`, `:38`, and `:52`.

Archived/resolved notes were present but not counted as active work. I did not run builds or staged validation because this was a read-only inventory.

## Review Request

Please review the answer draft for:

- Whether it respects root Mini/minigame exclusion.
- Whether the issue counts and grouping are defensible from the inspected files.
- Whether the final answer is safe to present as a substantive read-only breakdown.
- Whether any important active area is missing or misclassified.
