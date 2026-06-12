# T66 Inventory Audit - 2026-06-02

Operator: Codex  
Validator: Claude Code  
Scope: Full working-tree inventory audit, Mini/minigames included at full depth. No git operations, no deletion, no cleanup, no migration design.

## 1. Scope And Method

This is a descriptive inventory pass for `C:\UE\T66`. It reports what exists, how it is grouped, and what lifecycle/tracking/origin state can be factually assigned from the live filesystem and repo-local policy files. It is intended to feed later content/technical cross-reference and a future repo migration decision pass.

This pass included Mini/minigame-class systems fully: `Source/T66Mini`, `Source/T66TD`, `Source/T66Deck`, `Source/T66Idle`, `Gameplay/Minigames`, `Content/Mini`, `Content/TD`, `Content/Deck`, `Content/Idle`, `SourceAssets/Mini`, `SourceAssets/TD`, `SourceAssets/Deck`, `SourceAssets/Idle`, and `Content/UI/Minigames`.

No git commands were run. Any actual tracking state that requires git index inspection is `UNKNOWN-TRACKING`. `.gitignore` and `.gitattributes` were read as repository policy evidence only: `.gitignore:2`-`.gitignore:5` ignores `Binaries/`, `DerivedDataCache/`, `Intermediate/`, and `Saved/`; `.gitignore:42`-`.gitignore:47` describes `SourceAssets/` and generated/review artifact policy; `.gitattributes:5`-`.gitattributes:24` routes Unreal assets, media, models, archives, and common image/audio formats to Git LFS when tracked.

Evidence tiers used:

| Tier | Meaning |
|---|---|
| READ | Direct file/path/config/doc/filesystem read. |
| STATIC_TRACE | Static source/config reference linked to inventory state. |
| PRIOR_ARTIFACT | Prior generated proof/review artifact, not rerun in this pass. |
| RUNTIME_VERIFIED | Current runtime/editor/build verification. Not used in this pass. |

## 2. Status And ID Taxonomy

Lifecycle status tokens are the shared cross-audit tokens: `ACTIVE`, `DEMO_GATED`, `HIDDEN_RUNTIME`, `PARTIAL`, `DEPRECATED`, `COMPAT_LEGACY`, `BROKEN`, `STUB`, `ORPHAN_SUSPECT`, `UNKNOWN`.

Inventory tracking axis:

| Tracking Token | Meaning In This Audit |
|---|---|
| TRACKED | Would require git verification; not assigned here. |
| UNTRACKED | Would require git verification; not assigned here. |
| IGNORED | Would require git check-ignore or index context; not assigned here. |
| TRACKED-BUT-IGNORED | Would require git verification; not assigned here. |
| LFS-POINTER | Would require git/LFS pointer verification; not assigned here. |
| TRACKED-NONLFS-BINARY | Would require git/LFS pointer verification; not assigned here. |
| SHOULD-NOT-TRACK | Factual migration/review flag based on policy and origin; not an actual git state. Not assigned in this pass because the user requested factual current state without git/index decisions. |
| UNKNOWN-TRACKING | Assigned to all current paths whose actual tracking state was not inspected with git. |

Origin axis:

| Origin Token | Meaning |
|---|---|
| SOURCE-AUTHORED | Authored source code, docs, scripts, or configuration. |
| SOURCE-DATA | CSV/JSON/table inputs and intentionally authored data manifests. |
| IMPORTED-RUNTIME-ASSET | Runtime/cooked Unreal or loose runtime asset intentionally used by game/UI. |
| LOCAL-SOURCE-ART | Source art, source sprites, Blender/model/video inputs, handoff art. |
| EXTERNAL-DEPENDENCY | Third-party vendored runtime/build dependency. |
| GENERATED-DERIVED | Derived build/cache/generated output. |
| COOKED-STAGED | Cooked or staged build output. |
| PROOF-ARTIFACT | Captures, logs, review artifacts, proof bundles, diagnostics. |
| TEMP-SCRATCH | Temporary scratch or local residue. |

Element ID format: `INV-{AREA}-{NNN}`. Area suffixes are aligned to Content/Technical where relevant (`HERO`, `COMBAT`, `PROJECTILE`, `IDOL`, `ECONOMY`, `PET`, `BOSS`, `SAVE`, `BACKEND`, `UI`, `PIPELINE`, `BUILD`, `MINI`, `TD`, `IDLE`, `DECK`) and extended with inventory-only areas (`DATA`, `CONFIG`, `MODEL`, `VIDEO`, `SCRIPT`, `VFX`, `AUDIO`, `MAP`, `WORLD`, `DOC`, `REPORT`, `RELEASE`, `THIRD`, `PERF`, `TEMP`).

Finding ID format: `INVFIND-{NNN}`.

## 3. Scale Summary

The broad filesystem scan included the complete current project folder. `.git` is reported separately as local repo cache, not as working-tree source inventory.

| Scope | Files | Dirs | Size | Evidence |
|---|---:|---:|---:|---|
| `C:\UE\T66` all-in | 154,673 | 42,187 | 283.97 GB | READ |
| `.git` local repo cache | 55,951 | 25,076 | 149.94 GB | READ |
| Working tree excluding `.git` | 98,722 | 17,111 | 134.03 GB | READ |

The working-tree count above was captured before this document was created. This audit adds files under `Audit/Pending/InventoryAudit_2026-06-02`.

Largest working-tree roots:

| Root | Files | Size | Primary Origin | Notes |
|---|---:|---:|---|---|
| `Saved` | 76,347 | 101.71 GB | PROOF-ARTIFACT / COOKED-STAGED / GENERATED-DERIVED | Dominates current working tree. Policy ignored at `.gitignore:5`. |
| `Model Generation` | 6,236 | 9.49 GB | LOCAL-SOURCE-ART / GENERATED-DERIVED | Raw generation, rigging, experiments, production runs. Policy split at `.gitignore:48`-`.gitignore:68`. |
| `SourceAssets` | 3,351 | 8.37 GB | LOCAL-SOURCE-ART | Source art/provenance. Policy says local-only at `.gitignore:42`-`.gitignore:43`. |
| `Content` | 4,019 | 7.07 GB | IMPORTED-RUNTIME-ASSET / SOURCE-DATA | Main runtime Unreal content. |
| `Intermediate` | 2,562 | 5.32 GB | GENERATED-DERIVED | Build intermediates. Policy ignored at `.gitignore:4`. |
| `Binaries` | 35 | 1.23 GB | GENERATED-DERIVED | Local build products. Policy ignored at `.gitignore:2`. |
| `Video Generation` | 446 | 251.61 MB | LOCAL-SOURCE-ART / GENERATED-DERIVED | Prompt/manifests/runs. Runtime videos only after copied/registered. |
| `RuntimeDependencies` | 817 | 207.27 MB | IMPORTED-RUNTIME-ASSET | Loose staged runtime dependencies listed in `Config/DefaultGame.ini:48`. |
| `Reports` | 2,536 | 189.72 MB | PROOF-ARTIFACT / SOURCE-AUTHORED | Review/proof/report artifacts. |
| `UI` | 217 | 77.41 MB | SOURCE-AUTHORED / LOCAL-SOURCE-ART | UI instructions, references, checklists. |
| `Audit` | 158 | 59.76 MB | SOURCE-AUTHORED / PROOF-ARTIFACT | Audit docs/reference/pending work. |
| `tmp` | 852 | 24.23 MB | TEMP-SCRATCH | Local scratch. Policy ignored at `.gitignore:28`. |
| `Source` | 755 | 9.45 MB | SOURCE-AUTHORED | Runtime/editor source modules. |

Extension-family scale:

| Family | Files | Size | Notes |
|---|---:|---:|---|
| Other / extensionless | 61,834 | 156.14 GB | Dominated by `.git/lfs` and `.git/objects`; working-tree extensionless files also exist under runtime dependencies. |
| Images/textures, mostly `.png` | 53,139 | 49.80 GB | LFS policy if tracked at `.gitattributes:19`. |
| Unreal staged containers, mostly `.ucas` | 70 | 34.51 GB | Cooked/staged outputs. |
| Unreal assets `.uasset` / `.ubulk` / `.uexp` | 13,020 | 20.24 GB | LFS policy if tracked at `.gitattributes:5`-`.gitattributes:8`. |
| Build binaries/symbols/intermediates | 710 | 13.67 GB | Includes `.pdb`, `.exe`, `.pch`. |
| 3D/model/art source | 1,511 | 6.95 GB | Includes `.fbx`, `.glb`, `.obj`, `.blend`. LFS policy if tracked at `.gitattributes:12`-`.gitattributes:17`. |
| Data/docs/logs | 18,425 | 1.49 GB | Mixed durable docs and local logs. |
| Video | 1,701 | 693.33 MB | LFS policy if tracked at `.gitattributes:10`. |
| Archives | 4 | 337.82 MB | LFS policy if tracked at `.gitattributes:24`. |
| Source/scripts/config | 4,112 | 88.00 MB | Mostly migration-relevant authored text. |
| Audio | 147 | 77.12 MB | LFS policy for `.ogg`/`.wav` at `.gitattributes:11` and `.gitattributes:22`. |

## 4. Core Inventory Matrix

| Inventory ID | Path | Root | Kind | Size | Ext | Tracking | Origin | Lifecycle | Runtime/Owner | Content ID | Technical ID | Evidence | Notes |
|---|---|---|---|---:|---|---|---|---|---|---|---|---|---|
| INV-CONFIG-001 | `T66.uproject` | root | Unreal project descriptor | small | `.uproject` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Project/module registry | CONTENT-CONFIG-001 | TECH-CONFIG-001 | READ | UE 5.7 at `T66.uproject:3`; modules at `T66.uproject:8`, `:13`, `:18`, `:23`, `:28`, `:33`. |
| INV-CONFIG-002 | `Config/*.ini` | Config | Unreal config set | 15 files root-count | `.ini` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Runtime/editor config | CONTENT-CONFIG-002 | TECH-CONFIG-002 | READ | `DefaultGame.ini:7` has `ProjectVersion=alpha-0.9`; `DefaultEngine.ini:17`-`:20` sets frontend map and game instance. |
| INV-CONFIG-003 | `.gitignore` / `.gitattributes` | root | Repo policy config | small | text | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Repo hygiene policy | CONTENT-CONFIG-003 | TECH-CONFIG-003 | READ | Actual git state not inspected; policy evidence only. |
| INV-BUILD-001 | `Binaries` | Binaries | build output | 35 files / 1.23 GB | mixed | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Unreal local build products | CONTENT-BUILD-001 | TECH-BUILD-001 | READ | Ignored by policy at `.gitignore:2`; actual tracked/untracked unknown. |
| INV-BUILD-002 | `Intermediate` | Intermediate | build output/cache | 2,562 files / 5.32 GB | mixed | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Unreal build intermediates | CONTENT-BUILD-002 | TECH-BUILD-002 | READ | Ignored by policy at `.gitignore:4`; largest PCH files are local build products. |
| INV-BUILD-003 | `Saved/Cooked`, `Saved/StagedBuilds*`, `Saved/D2` | Saved | cooked/staged build output | grouped | mixed | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Packaged build artifacts | CONTENT-BUILD-003 | TECH-BUILD-003 | READ | Summarized in generated register; not enumerated per file. |
| INV-SCRIPT-001 | `Scripts` | Scripts | automation scripts | 176 files / 1.35 MB | `.py`, `.ps1`, `.md`, `.pyc` | UNKNOWN-TRACKING | SOURCE-AUTHORED / GENERATED-DERIVED | ACTIVE | Import, setup, capture, validation, staging, review | CONTENT-SCRIPT-001 | TECH-SCRIPT-001 | READ | `.pyc` files are generated cache residue; see `.gitignore:36`-`:39`. |
| INV-PIPELINE-001 | `Tools` | Tools | operator/release tools | 13 files / 0.07 MB | `.py`, `.ps1`, `.md` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Durable operator tools | CONTENT-PIPELINE-001 | TECH-PIPELINE-001 | READ | `Tools/README.md:3` and `Tools/README.md:20` identify tool scope. |
| INV-DATA-001 | `Content/Data` | Content | main game data tables | 60 files | `.csv`, `.json`, `.uasset`, `.md` | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | `UT66GameInstance` and data setup scripts | CONTENT-DATA-001 | TECH-DATA-001 | READ / STATIC_TRACE | 23 CSV, 6 JSON, 30 uasset, 1 pending file; 577 CSV rows and 130 JSON entries. |
| INV-HERO-001 | `Content/Data/Heroes.csv`, `DT_Heroes.uasset`, hero content/source roots | Content / Source | hero data and assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | Main run hero roster | CONTENT-HERO-001 | TECH-HERO-001 | READ / STATIC_TRACE | Main data has 12 hero rows; Mini has separate hero data/assets. |
| INV-COMPANION-001 | `Content/Data/Companions.csv`, `DT_Companions.uasset`, companion roots | Content / SourceAssets | companion data/assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | ACTIVE | Main run companions | CONTENT-COMPANION-001 | TECH-COMPANION-001 | READ / STATIC_TRACE | 16 main rows; video/hero-selection companion path drift is a finding. |
| INV-ENEMY-001 | `Content/Data/Enemies.csv`, `UniqueEnemies.csv`, enemy roots | Content | enemy data/assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | Main combat roster | CONTENT-ENEMY-001 | TECH-ENEMY-001 | READ / STATIC_TRACE | `Enemies.csv` has 60 rows; `UniqueEnemies.csv` has 1 row. |
| INV-BOSS-001 | `Content/Data/Boss*.csv`, `Bosses.csv`, boss roots | Content / Source | boss data/assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | Boss systems | CONTENT-BOSS-001 | TECH-BOSS-001 | READ / STATIC_TRACE | Boss tables include attacks, encounters, movement patterns, hazard definitions. |
| INV-COMBAT-001 | `Content/Data/CombatVFXBindings.csv`, combat source/content | Content / Source | combat binding data and runtime source | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / SOURCE-AUTHORED | ACTIVE | Combat VFX bindings and gameplay | CONTENT-COMBAT-001 | TECH-COMBAT-001 | READ / STATIC_TRACE | `CombatVFXBindings.csv` has 20 rows; combat source lives under `Source/T66/Gameplay`. |
| INV-PROJECTILE-001 | `Content/Weapons/Projectiles`, weapon projectile scripts/source | Content / Scripts / Source | projectile runtime assets and setup | grouped | mixed | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET / SOURCE-AUTHORED | ACTIVE | Weapons/projectiles | CONTENT-PROJECTILE-001 | TECH-PROJECTILE-001 | READ | `Content/Weapons/Projectiles` is 36 files / 82.02 MB. |
| INV-WEAPON-001 | `Content/Data/Weapons.csv`, `DT_Weapons.uasset`, `Content/Weapons` | Content | weapon data/assets | 228 weapon files | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | Weapon roster | CONTENT-WEAPON-001 | TECH-WEAPON-001 | READ / STATIC_TRACE | `Weapons.csv` has 48 rows. |
| INV-ITEM-001 | `Content/Data/Items.csv`, `DT_Items.uasset`, `Content/Items`, `SourceAssets/ItemSprites` | Content / SourceAssets | item data/assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | ACTIVE / ORPHAN_SUSPECT | Economy/items | CONTENT-ITEM-001 | TECH-ITEM-001 | READ | `Items.csv` has 30 rows; removed-row sprite residue is tracked in pending issues. |
| INV-IDOL-001 | `Content/Data/Idols.csv`, `DT_Idols.uasset`, idol roots | Content / SourceAssets | idol data/assets | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE | Idol system | CONTENT-IDOL-001 | TECH-IDOL-001 | READ | `Idols.csv` has 16 rows; `Content/VFX/Idols` is empty and listed as STUB separately. |
| INV-PET-001 | no `Pets.csv` / no `DT_Pets` | Content/Data | absent data table seam | n/a | n/a | UNKNOWN-TRACKING | SOURCE-DATA | COMPAT_LEGACY | Pet fallback path | CONTENT-PET-001 | TECH-PET-001 | STATIC_TRACE | `T66GameInstance.h:513`-`:519` comments indicate pets synthesize fallback from boss data; runtime wiring was not executed in this pass. |
| INV-SAVE-001 | `Source/T66/Core/*Save*`, side-mode save subsystems | Source | save runtime source | grouped | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / DEMO_GATED | Main and side-mode save systems | CONTENT-SAVE-001 | TECH-SAVE-001 | STATIC_TRACE | Side-mode save classes are listed in Mini/TD/Deck/Idle module sections. |
| INV-UI-001 | `Source/T66/UI`, `Content/UI`, `RuntimeDependencies/T66/UI`, `UI` | Source / Content / RuntimeDependencies / UI | UI source, assets, reference/provenance | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | ACTIVE / PARTIAL | UI manager/screens/assets | CONTENT-UI-001 | TECH-UI-001 | READ / STATIC_TRACE | `Source/T66/UI` has 213 source files; `Content/UI` has 202 runtime files; `RuntimeDependencies/T66/UI` has loose UI runtime/reference files. |
| INV-MAP-001 | `Content/Maps` | Content | runtime maps | 3 files | `.umap`, `.md` | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET / SOURCE-AUTHORED | ACTIVE | Main maps | CONTENT-MAP-001 | TECH-MAP-001 | READ | `FrontendLevel.umap` and `GameplayLevel.umap` are in `DefaultGame.ini` cook lists. |
| INV-VFX-001 | `Content/VFX`, `Content/VFXLab`, VFX packs | Content | VFX assets/lab packs | grouped | `.uasset`, `.umap` | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / HIDDEN_RUNTIME / STUB | Combat and imported VFX | CONTENT-VFX-001 | TECH-VFX-001 | READ | `Content/VFXLab` is never-cook per `DefaultGame.ini`; `Content/VFX/Idols` is empty. |
| INV-AUDIO-001 | `Content/Audio`, `SourceAssets/Audio` | Content / SourceAssets | audio assets/source | grouped | mixed | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | ACTIVE | Audio event/runtime audio | CONTENT-AUDIO-001 | TECH-AUDIO-001 | READ | `Content/Audio` has 155 files / 94.32 MB; `AudioEvents.json` has 96 entries. |
| INV-WORLD-001 | `Content/World`, world data/source | Content / Source | world props/interactables/assets | 489 files / 1.21 GB | mixed | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / HIDDEN_RUNTIME / STUB | World/interactable systems | CONTENT-WORLD-001 | TECH-WORLD-001 | READ | Includes `WorldVisualProps.json`, `VehicleInteractables.json`, `ArcadeInteractables.json`; some arcade surfaces are disabled. |
| INV-MINI-001 | `Source/T66Mini`, `Content/Mini`, `SourceAssets/Mini`, `Gameplay/Minigames/Mini` | multiple | Mini arcade runtime/source/art/data | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / SOURCE-DATA / LOCAL-SOURCE-ART / IMPORTED-RUNTIME-ASSET | DEMO_GATED | Full-game Mini module hidden in demo | CONTENT-MINI-001 | TECH-MINI-001 | READ / STATIC_TRACE | Minigames are not deprecated; demo-gated docs at `Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79`. |
| INV-TD-001 | `Source/T66TD`, `Content/TD`, `SourceAssets/TD`, `Gameplay/Minigames/TD` | multiple | TD side mode | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / SOURCE-DATA / LOCAL-SOURCE-ART | DEMO_GATED | TD runtime module | CONTENT-TD-001 | TECH-TD-001 | READ / STATIC_TRACE | Module declared at `T66.uproject:18`. |
| INV-DECK-001 | `Source/T66Deck`, `Content/Deck`, `SourceAssets/Deck`, `Gameplay/Minigames/Deck` | multiple | Deck side mode | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / SOURCE-DATA / LOCAL-SOURCE-ART | DEMO_GATED / PARTIAL | Deck runtime module | CONTENT-DECK-001 | TECH-DECK-001 | READ / STATIC_TRACE | `Content/Deck/README.md:3` still says reserved/future while module/data/assets exist. |
| INV-IDLE-001 | `Source/T66Idle`, `Content/Idle`, `SourceAssets/Idle`, `Gameplay/Minigames/Idle` | multiple | Idle side mode | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / SOURCE-DATA / LOCAL-SOURCE-ART | DEMO_GATED | Idle runtime module | CONTENT-IDLE-001 | TECH-IDLE-001 | READ / STATIC_TRACE | Module declared at `T66.uproject:23`. |
| INV-MODEL-001 | `Model Generation` | Model Generation | model generation workflow/assets/runs | 6,236 files / 9.49 GB | mixed | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED | PARTIAL | ToonStyle/model pipeline | CONTENT-MODEL-001 | TECH-MODEL-001 | READ | Generated runs summarized, not enumerated row-by-row. Routing says raw generated files are not runtime dependencies. |
| INV-VIDEO-001 | `Video Generation`, `Content/Movies`, `RuntimeDependencies/T66/Video` | multiple | video generation and runtime video registration | grouped | mixed | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED / IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | Frontend video catalog | CONTENT-VIDEO-001 | TECH-VIDEO-001 | READ / STATIC_TRACE | Runtime catalog is `RuntimeDependencies/T66/Video/frontend_videos.json`; README count drift is a finding. |
| INV-BACKEND-001 | `Backend`, `Source/T66/Core/T66BackendSubsystem*` | Backend / Source | backend docs and runtime client | grouped | `.md`, `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / UNKNOWN | Backend integration docs/client | CONTENT-BACKEND-001 | TECH-BACKEND-001 | READ / STATIC_TRACE | External backend repo health was not inspected. |
| INV-THIRD-001 | `ThirdParty/WebView2` | ThirdParty | vendored dependency | 3 files / 2.79 MB | `.dll`, `.h` | UNKNOWN-TRACKING | EXTERNAL-DEPENDENCY | ACTIVE / PARTIAL | WebView2 runtime support | CONTENT-THIRD-001 | TECH-THIRD-001 | STATIC_TRACE | Referenced by `Source/T66/T66.Build.cs:62` and `T66WebView2Host.cpp:338`; local license/version manifest not found. |
| INV-PERF-001 | `PerformanceSystem`, `Source/T66/PerformanceSystem` | PerformanceSystem / Source | performance docs/schemas/runtime source | 40 docs/schema files plus 9 source files | `.json`, `.md`, `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / PARTIAL | Performance/perception diagnostics | CONTENT-PERF-001 | TECH-PERF-001 | READ / STATIC_TRACE | README/runtime schema mismatch is a finding. |
| INV-REPORT-001 | `Reports` | Reports | reports/proofs/review artifacts | 2,536 files / 189.72 MB | mixed | UNKNOWN-TRACKING | PROOF-ARTIFACT / SOURCE-AUTHORED | ACTIVE / UNKNOWN | Historical reports and proof bundles | CONTENT-REPORT-001 | TECH-REPORT-001 | READ | Report raw-run retention depends on reference sweeps. |
| INV-DOC-001 | `Audit`, `Demo`, `Release`, `Gameplay`, routers/readmes | multiple | process docs/audit docs | grouped | `.md`, mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / PROOF-ARTIFACT | ACTIVE / PARTIAL | Process and audit ownership docs | CONTENT-DOC-001 | TECH-DOC-001 | READ | `Audit/README.md` has doc drift against live pending files. |
| INV-TEMP-001 | `tmp`, `UET66SavedTmpCombatVFXValidatorSelfTest_Bounce`, root `log_temp.txt` | root/tmp | scratch/residue | grouped | mixed | UNKNOWN-TRACKING | TEMP-SCRATCH | UNKNOWN / ORPHAN_SUSPECT | Local temporary artifacts | CONTENT-TEMP-001 | TECH-TEMP-001 | READ | No deletion recommended here; migration review needed. |

## 5. Data Table Register

Main data table surface: `Content/Data` has 60 files, 577 CSV rows, and 130 JSON entries. Runtime access is statically visible through `Source/T66/Core/T66GameInstance.h:66`-`:150` and data getter declarations at `Source/T66/Core/T66GameInstance.h:342`-`:440`.

| Inventory ID | Source File(s) | Cooked/Runtime Asset(s) | Rows/Entries | Lifecycle | Notes |
|---|---|---|---:|---|---|
| INV-DATA-002 | `ArcadeInteractables.json` | `DT_ArcadeInteractables.uasset` | 14 | HIDDEN_RUNTIME | Arcade interactables disabled by deprecated settings; `UT66DeprecatedFeatureSettings.h:15`-`:19`. |
| INV-DATA-003 | `AudioEvents.json` | `DT_AudioEvents.uasset` | 96 | ACTIVE | Audio event data source. |
| INV-BOSS-002 | `BossAttackDefinitions.csv` | `DT_BossAttackDefinitions.uasset` | 25 | ACTIVE | Boss attack definitions. |
| INV-BOSS-003 | `BossAttacks.csv` | `DT_BossAttacks.uasset` | 50 | ACTIVE | Boss attack table. |
| INV-BOSS-004 | `BossEncounterMembers.csv` | `DT_BossEncounterMembers.uasset` | 23 | ACTIVE | Encounter composition table. |
| INV-BOSS-005 | `BossEncounters.csv` | `DT_BossEncounters.uasset` | 20 | ACTIVE | Boss encounter table. |
| INV-BOSS-006 | `Bosses.csv` | `DT_Bosses.uasset` | 23 | ACTIVE / COMPAT_LEGACY | Also used for pet fallback path. |
| INV-BOSS-007 | `BossHazardDefinitions.csv` | `DT_BossHazardDefinitions.uasset` | 6 | ACTIVE | Boss hazard table. |
| INV-BOSS-008 | `BossMovementPatterns.csv` | `DT_BossMovementPatterns.uasset` | 8 | ACTIVE | Boss movement table. |
| INV-HERO-002 | `CharacterVisuals.csv` | `DT_CharacterVisuals.uasset` | 133 | ACTIVE | Character visual mapping. |
| INV-COMBAT-002 | `CombatVFXBindings.csv` | `DT_CombatVFXBindings.uasset` | 20 | ACTIVE | Combat VFX binding source. |
| INV-COMPANION-002 | `Companions.csv` | `DT_Companions.uasset` | 16 | ACTIVE | Companion roster. |
| INV-DATA-004 | `DifficultyTuning.json` | `DT_DifficultyTuning.uasset` | 5 | ACTIVE | Difficulty tuning. |
| INV-ENEMY-002 | `Enemies.csv` | `DT_Enemies.uasset` | 60 | ACTIVE | Enemy roster. |
| INV-HERO-003 | `Heroes.csv` | `DT_Heroes.uasset` | 12 | ACTIVE | Main hero roster. |
| INV-IDOL-002 | `Idols.csv` | `DT_Idols.uasset` | 16 | ACTIVE | Idol roster. |
| INV-ITEM-002 | `Items.csv` | `DT_Items.uasset` | 30 | ACTIVE / ORPHAN_SUSPECT | Pending issue flags removed-row sprite residue. |
| INV-DATA-005 | `Leaderboard_ScoreTargets.csv` | `Leaderboard_ScoreTargets.uasset` | 20 | ACTIVE | Non-`DT_` uasset naming. |
| INV-DATA-006 | `Leaderboard_SpeedrunTargets.csv` | `DT_Leaderboard_SpeedrunTargets.uasset` | 20 | ACTIVE | Speedrun target table. |
| INV-ECONOMY-001 | `LoanShark.csv` | `DT_LoanShark.uasset`, `LoanShark.uasset` | 1 | PARTIAL | Mixed naming; exact BP assignment not runtime-verified. |
| INV-DATA-007 | `MobVertexAnimations.csv` | `DT_MobVertexAnimations.uasset` | 10 | ACTIVE | VAT/animation data. |
| INV-WORLD-002 | `NPCs.csv` | `DT_NPCs.uasset` | 3 | ACTIVE | NPC data. |
| INV-DATA-008 | `PlayerExperience.json` | `DT_PlayerExperience.uasset` | 5 | ACTIVE | Player XP/level data. |
| INV-DATA-009 | `Stages.csv` | `DT_Stages.uasset` | 20 | ACTIVE | Main stages. |
| INV-COMBAT-003 | `StatusEffects.csv` | `DT_StatusEffects.uasset` | 12 | PARTIAL | Pending issue says production mobs still assign `None`. |
| INV-ENEMY-003 | `UniqueEnemies.csv` | `DT_UniqueEnemies.uasset` | 1 | ACTIVE | Unique enemy table. |
| INV-WORLD-003 | `VehicleInteractables.json` | `DT_VehicleInteractables.uasset` | 6 | ACTIVE | Vehicle interactables. |
| INV-WEAPON-002 | `Weapons.csv` | `DT_Weapons.uasset` | 48 | ACTIVE | Weapon data table. |
| INV-WORLD-004 | `WorldVisualProps.json` | `DT_WorldVisualProps.uasset` | 4 | ACTIVE | World prop mapping. |
| INV-DATA-010 | `pending_issues_Data.md` | n/a | n/a | ACTIVE | Inventory-relevant issue source; do not count headings blindly. |

## 6. Source Module Register

| Inventory ID | Module / Path | Files | Kind | Tracking | Origin | Lifecycle | Runtime/Owner | Evidence | Notes |
|---|---|---:|---|---|---|---|---|---|---|
| INV-SOURCE-001 | `Source/T66` | 656 | Runtime module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Main T66 runtime | READ / STATIC_TRACE | Declared Runtime/Default at `T66.uproject:8`; module implementation at `Source/T66/T66.cpp:31`. |
| INV-SOURCE-002 | `Source/T66Editor` | 11 | Editor module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Editor tooling | READ / STATIC_TRACE | Declared Editor/Default at `T66.uproject:33`; commandlets are diagnostic/editor tooling. |
| INV-MINI-002 | `Source/T66Mini` | 42 | Runtime module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | DEMO_GATED / PARTIAL | Mini runtime/UI/save/data | READ / STATIC_TRACE | Declared Runtime/Default at `T66.uproject:13`; build include warning in `pending_issues_T66Mini.md:6`. |
| INV-TD-002 | `Source/T66TD` | 20 | Runtime module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | DEMO_GATED | TD runtime/UI/save/data | READ / STATIC_TRACE | Declared Runtime/Default at `T66.uproject:18`. |
| INV-IDLE-002 | `Source/T66Idle` | 12 | Runtime module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | DEMO_GATED | Idle runtime/UI/save/data | READ / STATIC_TRACE | Declared Runtime/Default at `T66.uproject:23`. |
| INV-DECK-002 | `Source/T66Deck` | 12 | Runtime module source | UNKNOWN-TRACKING | SOURCE-AUTHORED | DEMO_GATED | Deck runtime/UI/save/data | READ / STATIC_TRACE | Declared Runtime/Default at `T66.uproject:28`. |

## 7. Mini And Minigame-Class Full-Depth Register

Minigames are not deprecated. `Demo/DEPRECATED_CONTENT.md:44` says minigames moved out of deprecated classification, and `Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79`-`:85` classifies them as full-game present but hidden/blocked in demo. `Gameplay/Minigames/MINIGAMES_AGENTS.md:3`-`:5` owns Mini/TD/Deck/Idle isolation.

### 7.1 T66Mini Source Files

| Inventory ID | Path | Lifecycle | Origin | Notes |
|---|---|---|---|---|
| INV-MINI-003 | `Source/T66Mini/T66Mini.Build.cs` | DEMO_GATED / PARTIAL | SOURCE-AUTHORED | Loose dependencies include `Content/Mini/Data` and `SourceAssets/Mini`; include-dir warning tracked in pending issue. |
| INV-MINI-004 | `Source/T66Mini/Private/T66MiniModule.cpp` | DEMO_GATED | SOURCE-AUTHORED | Module implementation. |
| INV-MINI-005 | `Source/T66Mini/Private/Core/T66MiniCircusSubsystem.cpp`; `Public/Core/T66MiniCircusSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Circus subsystem. |
| INV-MINI-006 | `Source/T66Mini/Private/Core/T66MiniDataSubsystem.cpp`; `Public/Core/T66MiniDataSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Mini data loading. |
| INV-MINI-007 | `Source/T66Mini/Private/Core/T66MiniFrontendStateSubsystem.cpp`; `Public/Core/T66MiniFrontendStateSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Frontend state. |
| INV-MINI-008 | `Source/T66Mini/Private/Core/T66MiniLeaderboardSubsystem.cpp`; `Public/Core/T66MiniLeaderboardSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Mini leaderboard. |
| INV-MINI-009 | `Source/T66Mini/Private/Core/T66MiniRunStateSubsystem.cpp`; `Public/Core/T66MiniRunStateSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Run state. |
| INV-MINI-010 | `Source/T66Mini/Private/Core/T66MiniRuntimeSubsystem.cpp`; `Public/Core/T66MiniRuntimeSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Runtime subsystem. |
| INV-MINI-011 | `Source/T66Mini/Private/Core/T66MiniVisualSubsystem.cpp`; `Public/Core/T66MiniVisualSubsystem.h` | DEMO_GATED | SOURCE-AUTHORED | Visual subsystem. |
| INV-MINI-012 | `Source/T66Mini/Public/Data/T66MiniDataTypes.h` | DEMO_GATED | SOURCE-AUTHORED | Mini data schema. |
| INV-MINI-013 | `Source/T66Mini/Private/Save/T66MiniSaveSubsystem.cpp`; `Public/Save/T66MiniSaveSubsystem.h`; `Public/Save/T66MiniProfileSaveGame.h`; `Public/Save/T66MiniRunSaveGame.h` | DEMO_GATED | SOURCE-AUTHORED | Mini save/profile/run save. |
| INV-MINI-014 | `Source/T66Mini/Private/UI/Screens/T66MiniBattleScreen.cpp`; `Public/UI/Screens/T66MiniBattleScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Battle screen. |
| INV-MINI-015 | `Source/T66Mini/Private/UI/Screens/T66MiniCharacterSelectScreen.cpp`; `Public/UI/Screens/T66MiniCharacterSelectScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Character select. |
| INV-MINI-016 | `Source/T66Mini/Private/UI/Screens/T66MiniCompanionSelectScreen.cpp`; `Public/UI/Screens/T66MiniCompanionSelectScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Companion select. |
| INV-MINI-017 | `Source/T66Mini/Private/UI/Screens/T66MiniDifficultySelectScreen.cpp`; `Public/UI/Screens/T66MiniDifficultySelectScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Difficulty select. |
| INV-MINI-018 | `Source/T66Mini/Private/UI/Screens/T66MiniIdolSelectScreen.cpp`; `Public/UI/Screens/T66MiniIdolSelectScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Idol select. |
| INV-MINI-019 | `Source/T66Mini/Private/UI/Screens/T66MiniMainMenuScreen.cpp`; `Public/UI/Screens/T66MiniMainMenuScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Mini main menu. |
| INV-MINI-020 | `Source/T66Mini/Private/UI/Screens/T66MiniRunSummaryScreen.cpp`; `Public/UI/Screens/T66MiniRunSummaryScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Run summary. |
| INV-MINI-021 | `Source/T66Mini/Private/UI/Screens/T66MiniSaveSlotsScreen.cpp`; `Public/UI/Screens/T66MiniSaveSlotsScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Save slots. |
| INV-MINI-022 | `Source/T66Mini/Private/UI/Screens/T66MiniShopScreen.cpp`; `Public/UI/Screens/T66MiniShopScreen.h` | DEMO_GATED | SOURCE-AUTHORED | Shop screen. |
| INV-MINI-023 | `Source/T66Mini/Private/UI/Screens/T66MiniGeneratedScreenChrome.h`; `Private/UI/T66MiniUIStyle.h` | DEMO_GATED | SOURCE-AUTHORED | Generated chrome/style headers. |
| INV-MINI-024 | `Source/T66Mini/pending_issues_T66Mini.md` | ACTIVE | SOURCE-AUTHORED | Active issue file for Mini module. |

### 7.2 Mini Runtime Data And Runtime Assets

| Inventory ID | Path | Count | Lifecycle | Origin | Notes |
|---|---|---:|---|---|---|
| INV-MINI-025 | `Content/Mini/Data/README.md` | 1 | DEMO_GATED | SOURCE-AUTHORED | Mini cooked/data folder note. |
| INV-MINI-026 | `Content/Mini/Data/T66Mini_Bosses.csv` | 66 rows | DEMO_GATED | SOURCE-DATA | Mini boss data. |
| INV-MINI-027 | `Content/Mini/Data/T66Mini_CircusGames.csv` | 6 rows | DEMO_GATED | SOURCE-DATA | Mini circus game data. |
| INV-MINI-028 | `Content/Mini/Data/T66Mini_Companions.csv` | 24 rows | DEMO_GATED | SOURCE-DATA | Mini companion data. |
| INV-MINI-029 | `Content/Mini/Data/T66Mini_Difficulties.csv` | 5 rows | DEMO_GATED | SOURCE-DATA | Mini difficulty data. |
| INV-MINI-030 | `Content/Mini/Data/T66Mini_Enemies.csv` | 4 rows | DEMO_GATED | SOURCE-DATA | Mini enemy data. |
| INV-MINI-031 | `Content/Mini/Data/T66Mini_Heroes.csv` | 12 rows | DEMO_GATED | SOURCE-DATA | Mini hero data. |
| INV-MINI-032 | `Content/Mini/Data/T66Mini_Idols.csv` | 16 rows | DEMO_GATED | SOURCE-DATA | Mini idol data. |
| INV-MINI-033 | `Content/Mini/Data/T66Mini_Interactables.csv` | 4 rows | DEMO_GATED | SOURCE-DATA | Mini interactables. |
| INV-MINI-034 | `Content/Mini/Data/T66Mini_Items.csv` | 31 rows | DEMO_GATED | SOURCE-DATA | Mini item data. |
| INV-MINI-035 | `Content/Mini/Data/T66Mini_RuntimeTuning.csv` | 295 rows | DEMO_GATED | SOURCE-DATA | Mini tuning table. |
| INV-MINI-036 | `Content/Mini/Data/T66Mini_Stages.csv` | 50 rows | DEMO_GATED | SOURCE-DATA | Mini stages. |
| INV-MINI-037 | `Content/Mini/Data/T66Mini_Waves.csv` | 50 rows | DEMO_GATED | SOURCE-DATA | Mini waves. |
| INV-MINI-038 | `Content/Mini/Sprites/Bosses` | 4 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Cow, Goat, Pig, Roost boss sprites. |
| INV-MINI-039 | `Content/Mini/Sprites/Enemies` | 4 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Cow, Goat, Pig, Roost enemy sprites. |
| INV-MINI-040 | `Content/Mini/Sprites/Heroes` | 16 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Arthur, Asmon, Billy, Dog, Forsen, George, LuBu, Merlin, Mike, Moist, North, Rabbit, Robo, Shroud, xQc, Zeus. |
| INV-MINI-041 | `Content/Mini/Sprites/Interactables` | 7 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Fountain, LootBag black/red/yellow, LootCrate, QuickReviveMachine, TreasureChest. |
| INV-MINI-042 | `Content/Mini/Sprites/NPCs` | 3 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Gambler, Ouroboros, Saint. |
| INV-MINI-043 | `Content/UI/Minigames` | 4 uassets | DEMO_GATED | IMPORTED-RUNTIME-ASSET | Deck and Idle gameplay/main menu mockups. |

### 7.3 Mini Source Art Register

| Inventory ID | Path | Count | Size | Lifecycle | Origin | Notes |
|---|---|---:|---:|---|---|---|
| INV-MINI-044 | `SourceAssets/Mini/Background.png` and `README.md` / manifest | 3 top-level files | small | DEMO_GATED | LOCAL-SOURCE-ART / SOURCE-AUTHORED | Mini source art root and manifest. |
| INV-MINI-045 | `SourceAssets/Mini/Bosses/Singles` | 4 | 0.01 MB | DEMO_GATED | LOCAL-SOURCE-ART | Cow/Goat/Pig/Roost boss source sprites. |
| INV-MINI-046 | `SourceAssets/Mini/Enemies/Singles` | 4 | 0.01 MB | DEMO_GATED | LOCAL-SOURCE-ART | Cow/Goat/Pig/Roost enemy source sprites. |
| INV-MINI-047 | `SourceAssets/Mini/Heroes/Singles` | 12 | 0.02 MB | DEMO_GATED | LOCAL-SOURCE-ART | 12 Mini hero single sprites. |
| INV-MINI-048 | `SourceAssets/Mini/Heroes/AnimationSets/*` | 144 | 0.23 MB | DEMO_GATED | LOCAL-SOURCE-ART | 12 heroes x 12 files each: idle, walk, attack, projectiles by side. |
| INV-MINI-049 | `SourceAssets/Mini/Companions/Singles` | 24 | 0.05 MB | DEMO_GATED | LOCAL-SOURCE-ART | 24 companion single sprites. |
| INV-MINI-050 | `SourceAssets/Mini/Companions/AnimationSets/*` | 240 | 0.48 MB | DEMO_GATED | LOCAL-SOURCE-ART | 24 companions x 10 files each: idle, walk A/B/C, attack by side. |
| INV-MINI-051 | `SourceAssets/Mini/HUD` | 8 | 3.21 MB | DEMO_GATED | LOCAL-SOURCE-ART | Input/prompt/passive/quick revive/ultimate HUD source sprites. |
| INV-MINI-052 | `SourceAssets/Mini/Effects` | 8 | 0.01 MB | DEMO_GATED | LOCAL-SOURCE-ART | Enemy projectiles and trap source effects. |
| INV-MINI-053 | `SourceAssets/Mini/Idols/Effects/Singles` | 16 | 0.01 MB | DEMO_GATED | LOCAL-SOURCE-ART | 16 idol effect source sprites. |
| INV-MINI-054 | `SourceAssets/Mini/Interactables/Singles` | 7 | 0.01 MB | DEMO_GATED | LOCAL-SOURCE-ART | Fountain, loot bags, crate, quick revive, chest. |

### 7.4 TD / Deck / Idle Registers

| Inventory ID | Path | Files / Rows | Lifecycle | Origin | Notes |
|---|---|---:|---|---|---|
| INV-TD-003 | `Source/T66TD` | 20 files | DEMO_GATED | SOURCE-AUTHORED | Data, visual, frontend, save, battle/difficulty/main menu UI screens. |
| INV-TD-004 | `Content/TD/Data` | 10 files; 175 CSV rows; 20 JSON entries | DEMO_GATED | SOURCE-DATA | BattleTuning 96, Difficulties 5, EnemyArchetypes 5, HeroCombat 12, Heroes 12, Layouts JSON 20, Maps 20, Stages 20, ThemeModifierRules 5. |
| INV-TD-005 | `SourceAssets/TD` | 46 files | DEMO_GATED | LOCAL-SOURCE-ART | 5 bosses, 5 enemies, 12 heroes, 21 map backgrounds, 1 UI plate, README, manifest. |
| INV-DECK-003 | `Source/T66Deck` | 12 files | DEMO_GATED | SOURCE-AUTHORED | Data, frontend, save, main menu source. |
| INV-DECK-004 | `Content/Deck/Data` | 11 files; 36 CSV rows | DEMO_GATED / PARTIAL | SOURCE-DATA | Cards 8, companions 3, encounters 5, enemies 7, heroes 3, items 3, relics 2, stages 1, starting decks 3, tuning 1. |
| INV-DECK-005 | `SourceAssets/Deck` | 26 files | DEMO_GATED | LOCAL-SOURCE-ART | 2 backgrounds, 8 cards, 4 effects, 7 enemies, 3 heroes, README, manifest. |
| INV-IDLE-003 | `Source/T66Idle` | 12 files | DEMO_GATED | SOURCE-AUTHORED | Data, frontend, save, main menu source. |
| INV-IDLE-004 | `Content/Idle/Data` | 9 files; 33 CSV rows | DEMO_GATED | SOURCE-DATA | Companions 4, enemies 4, heroes 5, idols 4, items 4, stages 10, tuning 1, zones 1. |
| INV-IDLE-005 | `SourceAssets/Idle` | 18 files | DEMO_GATED | LOCAL-SOURCE-ART | 2 backgrounds, 4 effects, 4 enemies, 6 heroes, README, manifest. |
| INV-DOC-002 | `Gameplay/Minigames` | 15 files | ACTIVE | SOURCE-AUTHORED | Mini/TD/Deck/Idle implementation docs, memory progression docs, animation/pipeline instructions, manifests. |

## 8. Root And Domain Breakdown

| Inventory ID | Root / Domain | Count / Size | Tracking | Origin | Lifecycle | Evidence | Notes |
|---|---|---:|---|---|---|---|---|
| INV-CONTENT-001 | `Content/Characters` | 1,317 files / 4.64 GB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | Heroes, companions, mobs, VAT, NPCs, enemies. |
| INV-WORLD-005 | `Content/World` | 489 files / 1.21 GB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / STUB | READ | Interactables, terrain, boosts, visual props, gates, loot bags; Cliffs/Sky empty. |
| INV-WEAPON-003 | `Content/Weapons` | 228 files / 105.08 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | 36 projectile files and 192 sprite files. |
| INV-AUDIO-002 | `Content/Audio` | 155 files / 94.32 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | OSTS, HeltonPixelCombat, SFX, Arcade. |
| INV-VIDEO-002 | `Content/Movies` | 99 files / 34.93 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ | Frontend and hero selection runtime movies. |
| INV-VFX-002 | `Content/VFX` | 97 files / 6.08 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / STUB | READ | Hero1, Foundation, Projectiles; Idols empty. |
| INV-UI-002 | `Content/UI` | 202 files / 77.19 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / DEMO_GATED | READ | Sprites, Minigames, Leaderboard, Materials. |
| INV-ART-001 | `Content/ToonStyle` | 134 files / 438.55 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ | TestAssets dominate. |
| INV-ITEM-003 | `Content/Items` | 121 files / 4.88 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / ORPHAN_SUSPECT | READ | Item sprites. |
| INV-IDOL-003 | `Content/Idols` | 48 files / 26.33 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | Idol sprites. |
| INV-MINI-055 | `Content/Mini` | 48 files / 12.25 MB | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | DEMO_GATED | READ | Full Mini data/sprites. |
| INV-TD-006 | `Content/TD` | 11 files | UNKNOWN-TRACKING | SOURCE-DATA | DEMO_GATED | READ | TD data and README. |
| INV-DECK-006 | `Content/Deck` | 12 files | UNKNOWN-TRACKING | SOURCE-DATA | DEMO_GATED / PARTIAL | READ | Deck data and README. |
| INV-IDLE-006 | `Content/Idle` | 10 files | UNKNOWN-TRACKING | SOURCE-DATA | DEMO_GATED | READ | Idle data and README. |
| INV-RUNTIME-001 | `RuntimeDependencies/T66` | 817 files / 207.27 MB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ / STATIC_TRACE | Loose staged roots listed in `DefaultGame.ini:48`-`:62`. |
| INV-MODEL-002 | `SourceAssets` | 3,351 files / 8.37 GB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE / PARTIAL | READ | Policy says local-only source art at `.gitignore:42`-`:43`; actual tracking unknown. |
| INV-DOC-003 | `Release` | 5 files / 0.03 MB | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | READ | Release policy and Steam/QA docs. |
| INV-BACKEND-002 | `Backend` | 7 files / 0.08 MB | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / UNKNOWN | READ | T66-side backend docs; external backend not inspected. |
| INV-PERF-002 | `PerformanceSystem` | 40 files / 0.49 MB | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / PARTIAL | READ | Docs/schema root; runtime source under `Source/T66/PerformanceSystem`. |
| INV-REPORT-002 | `Audit` | 158 files / 59.76 MB | UNKNOWN-TRACKING | SOURCE-AUTHORED / PROOF-ARTIFACT | ACTIVE / PARTIAL | READ | Pending/Finished/Reference; README doc drift. |
| INV-REPORT-003 | `Reports` | 2,536 files / 189.72 MB | UNKNOWN-TRACKING | PROOF-ARTIFACT / SOURCE-AUTHORED | ACTIVE / UNKNOWN | READ | Agent reviews, hygiene, proof, roster, ToonStyle reports. |

## 9. Script And Tool Register

`Scripts` has 176 files: 109 `.py`, 26 `.ps1`, 2 `.md`, and 39 `.pyc`. `Scripts/README.md:11`-`:19` defines script areas: build/package helpers, review helpers, UI capture/import helpers, data-table setup, import core, maintenance/audit/repair/verification.

| Inventory ID | Family | Representative Files | Tracking | Origin | Lifecycle | Notes |
|---|---|---|---|---|---|---|
| INV-SCRIPT-002 | Build/stage/release | `StageStandaloneBuild.ps1`, `StageDemoBuild.ps1`, `GuardT66RuntimeAssetContract.ps1` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Build/stage helpers; not run in this audit. |
| INV-SCRIPT-003 | Operator/validator review | `Invoke-ClaudePlanReview.ps1`, `Invoke-ClaudeDirectRead.ps1`, `Invoke-ClaudeReadOnlyOperator.ps1`, `Invoke-CodexPlanReview.ps1`, `Set-T66Operator.ps1`, `Get-CodexTokenUsage.ps1` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | T66 process helpers; Claude independent and cross-review use these. |
| INV-SCRIPT-004 | UI capture/fidelity | `CaptureT66UIScreen.ps1`, `CaptureT66UIWidget.ps1`, `CompareUIScreen.ps1`, `CompareUIScreen.py`, `VerifyUIFidelity.py`, `GenerateUIGeometryOverlay.py` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | UI proof/capture pipeline. |
| INV-SCRIPT-005 | Gameplay/video/VFX capture | `CaptureT66GameplayVideo.ps1`, `CaptureT66NiagaraMRQIsolation.ps1`, `CaptureT66NiagaraEditorIsolation.ps1`, `CaptureT66OutgoingTravelerSwarmProof.ps1` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Unreal-owned capture helpers. |
| INV-SCRIPT-006 | Data setup/reload | `Setup*DataTable.py`, `ReloadActiveHeroRosterDataTables.py`, `ReloadCleanedInteractableDataTablesAndExit.py`, `ReloadHeadshotStatDataTablesAndExit.py` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | DataTable creation/reload scripts. |
| INV-SCRIPT-007 | Asset import | `ImportMiniSprites.py`, `RunImportMiniSpritesAndExit.py`, `ImportItemSprites.py`, `ImportWeaponSpritesAndSetup.py`, `ImportQuadRetro*Visuals.py`, `RunImport*AndExit.py` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / PARTIAL | Import scripts include active pending issues and crash/fallback cautions. |
| INV-SCRIPT-008 | Validation/audit | `Audit*AndExit.py`, `Validate*`, `Verify*` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Static/Unreal validation scripts. |
| INV-SCRIPT-009 | Generated Python caches | `Scripts/**/*.pyc` | 39 files | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN / ORPHAN_SUSPECT | Policy ignored at `.gitignore:36`-`:39`; actual tracking unknown. |

## 10. Generated, Derived, Large-Artifact Register

The following are summarized bulk registers by design. They are not enumerated row-by-row.

| Finding / Inventory ID | Path / Family | Files | Size | Tracking | Origin | Lifecycle | Notes |
|---|---|---:|---:|---|---|---|---|
| INV-BUILD-004 | `.git/lfs` | 30,774 | 140.43 GB | local cache, no git operation | n/a | UNKNOWN | Local repository cache only; not working-tree migration content. |
| INV-BUILD-005 | `.git/objects` | 25,015 | 9.50 GB | local cache, no git operation | n/a | UNKNOWN | Local repository object cache. |
| INV-REPORT-004 | `Saved/VideoCaptures` | 24,324 | 29.17 GB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Captures/proof artifacts. |
| INV-BUILD-006 | `Saved/D2` | 12,671 | 24.93 GB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged/cooked proof/build artifacts. |
| INV-BUILD-007 | `Saved/Cooked` | 9,299 | 13.03 GB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Cooked Unreal output. |
| INV-BUILD-008 | `Saved/StagedBuilds` | 2,253 | 10.41 GB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged standalone output. |
| INV-REPORT-005 | `Saved/Codex` | 6,416 | 7.09 GB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Codex local artifacts/logs. |
| INV-BUILD-009 | `Saved/StagedBuilds_PetMobLootFoundation` | 1,593 | 5.26 GB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged build variant. |
| INV-BUILD-010 | `Intermediate/Build` | 2,063 | 5.12 GB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Build intermediates. |
| INV-BUILD-011 | `Saved/StagedBuildsDemo` | 1,590 | 4.86 GB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Demo staged build. |
| INV-MODEL-003 | `Model Generation/Runs` | 2,347 | 4.84 GB | UNKNOWN-TRACKING | GENERATED-DERIVED / LOCAL-SOURCE-ART | PARTIAL | Raw generation runs; `.gitignore:50`-`:64` filters generated run outputs. |
| INV-MODEL-004 | `Model Generation/Rigging and Animation/Runs` | 2,721 | 2.70 GB | UNKNOWN-TRACKING | GENERATED-DERIVED / LOCAL-SOURCE-ART | PARTIAL | Rigging/animation runs. |
| INV-BUILD-012 | `Binaries/Win64` | 35 | 1.23 GB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Local build binary/symbols. |
| INV-REPORT-006 | `Saved/Crashes` | 1,661 | 1.05 GB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Crash diagnostics. |
| INV-VIDEO-003 | `Video Generation/Runs` and generation tree | 446 total in root | 251.61 MB | UNKNOWN-TRACKING | GENERATED-DERIVED / LOCAL-SOURCE-ART | PARTIAL | Runtime registration requires copy to `Content/Movies` and catalog entry. |

Largest concrete files seen:

| Path | Size | Origin | Notes |
|---|---:|---|---|
| `Saved/StagedBuilds/Windows/T66/Content/Paks/T66-Windows.ucas` | 4.21 GB | COOKED-STAGED | Staged build package. |
| `Saved/StagedBuilds_PetMobLootFoundation/Windows/T66/Content/Paks/T66-Windows.ucas` | 4.21 GB | COOKED-STAGED | Staged build variant package. |
| `Saved/StagedBuildsDemo/Windows/T66/Content/Paks/T66-Windows.ucas` | 3.85 GB | COOKED-STAGED | Demo staged package. |
| `Intermediate/Build/Win64/x64/T66Editor/...SharedPCH...pch` | 2.25 GB | GENERATED-DERIVED | Build intermediate. |
| `Binaries/Win64/T66.pdb` | 414.78 MB | GENERATED-DERIVED | Local symbol file. |
| `Binaries/Win64/T66.exe` | 306.15 MB | GENERATED-DERIVED | Local build executable. |
| `Content/Characters/Companions/Companion_01/Default/Walk/CompanionWalk.uasset` | 95.51 MB | IMPORTED-RUNTIME-ASSET | Large durable-looking runtime content asset. |

## 11. Duplicate / Orphan / Should-Review Register

These are factual flags, not deletion recommendations.

| Finding ID | Path / Surface | Lifecycle | Origin / Tracking | Evidence | Finding |
|---|---|---|---|---|---|
| INVFIND-001 | `Audit/README.md` vs live `Audit/Pending` | PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `Audit/README.md:7`, `Audit/README.md:13`, `Audit/AUDIT_AGENTS.md:14` | Audit README says there are no active pending files while live pending audit files exist. Doc drift. |
| INVFIND-002 | Mini/minigames docs | DEMO_GATED | SOURCE-AUTHORED | `Demo/DEPRECATED_CONTENT.md:44`, `Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79` | Minigames are explicitly not deprecated; stale "deprecated" assumptions should not be applied to Mini/TD/Deck/Idle. |
| INVFIND-003 | `Content/Deck/README.md` wording | PARTIAL | SOURCE-AUTHORED / SOURCE-DATA | `Content/Deck/README.md:3`, `T66.uproject:28` | Deck README describes future/reserved structure while Deck module, data, and source art are present. Treat as doc drift/partial, not absence. |
| INVFIND-004 | `PerformanceSystem/README.md` vs runtime code | PARTIAL | SOURCE-AUTHORED | `PerformanceSystem/README.md:60`, `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:43` | README says current runtime schema is 4 while runtime code/schema files indicate v8. |
| INVFIND-005 | `RuntimeDependencies/T66/UI/Reference/asset_inventory.csv` | PARTIAL | IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | `RuntimeDependencies/T66/UI/Reference/README.md:13`, `asset_inventory.csv:1`, `asset_inventory.csv:102` | Runtime dependency tree contains inventory CSV whose `File` column points back to `SourceAssets/UI/Reference`, creating source/runtime provenance ambiguity. |
| INVFIND-006 | Hero selection companion source paths | PARTIAL / ORPHAN_SUSPECT | SOURCE-AUTHORED / LOCAL-SOURCE-ART | `Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h:314`, `:400`, `UI/hero_selection_closeout_and_stage2_readiness.md:91` | Code builds companion paths under `SourceAssets/UI/HeroSelection/Companions` while stubs exist under `SourceAssets/UI/ContentStubs/HeroSelection`. |
| INVFIND-007 | Chest reward art | PARTIAL | LOCAL-SOURCE-ART / IMPORTED-RUNTIME-ASSET | `UI/Processes/LootChestAnimationMechanismPacket.md:45`, `Source/T66/UI/HUD/T66GameplayHUDWidget_Private.h:341` | Docs planned yellow chest/coin while code can request red/yellow/white/black closed/open variants. Non-yellow route is a factual gap unless another asset route exists. |
| INVFIND-008 | `SourceAssets/Deck`, `SourceAssets/Idle`, `SourceAssets/TD` manifests | PARTIAL | LOCAL-SOURCE-ART | `SourceAssets/TD/ROTmgPixelSetManifest.json:10`, `SourceAssets/Idle/ROTmgPixelSetManifest.json:10`, `SourceAssets/Deck/ROTmgPixelSetManifest.json:9` | Manifests list cross-mode outputs. Looks like shared-generation provenance, but folder README ownership can read narrower than generated set reality. |
| INVFIND-009 | root `SourceAssets/PIXALTEST*.png` | ORPHAN_SUSPECT | LOCAL-SOURCE-ART | `Source/T66/Gameplay/T66GameMode.cpp:971`, `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:2333` | Provenance suspect, but gameplay code still references related PIXALTEST assets; do not classify as orphan without deeper trace. |
| INVFIND-010 | `Content/Data` removed item sprite residue | ORPHAN_SUSPECT | IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | `Content/Data/pending_issues_Data.md:17` | `Item_HpRegen` / `Item_LifeSteal` sprite residue remains after main-run row removal pending Mini-inclusive reference audit. |
| INVFIND-011 | `Content/Data/StatusEffects.csv` | PARTIAL | SOURCE-DATA | `Content/Data/pending_issues_Data.md:17` | Status effects exist, but pending issue says production mobs still assign `None`. |
| INVFIND-012 | `Source/T66Mini/T66Mini.Build.cs` include path | PARTIAL | SOURCE-AUTHORED | `Source/T66Mini/pending_issues_T66Mini.md:6`, `Source/T66Mini/T66Mini.Build.cs:47` | Build rules reference missing `Public/UI/Components`; warning only, build not rerun here. |
| INVFIND-013 | `Video Generation/README.md` vs runtime manifest | PARTIAL | SOURCE-AUTHORED / IMPORTED-RUNTIME-ASSET | `Video Generation/README.md:7`, `RuntimeDependencies/T66/Video/frontend_videos.json:8`, `frontend_videos.json:376` | README claims 48 hero and 48 companion clips; live manifest parse found 34 hero entries, 16 companion entries, 12 movie-enabled heroes, 4 movie-enabled companions. |
| INVFIND-014 | `Video Generation/Manifests/frontend_videos.json` vs runtime catalog | PARTIAL | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | READ hash comparison | Source catalog copy and runtime catalog are not byte-identical. |
| INVFIND-015 | `UI/content_stubs_registry.md` | BROKEN / PARTIAL | SOURCE-AUTHORED | `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md:856` | UI fidelity docs require a content stubs registry, but that file was not present in this checkout. |
| INVFIND-016 | `ThirdParty/WebView2` | PARTIAL | EXTERNAL-DEPENDENCY / UNKNOWN-TRACKING | `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:68`, `Source/T66/T66.Build.cs:62`, `Source/T66/Core/T66WebView2Host.cpp:338` | Active dependency references exist, but no local README/license/version manifest was found. |
| INVFIND-017 | `Scripts/**/*.pyc` | ORPHAN_SUSPECT / UNKNOWN | GENERATED-DERIVED / UNKNOWN-TRACKING | `.gitignore:36`-`:39` | Generated Python caches present under scripts; actual tracking unknown. |
| INVFIND-018 | `tmp`, `UET66SavedTmpCombatVFXValidatorSelfTest_Bounce`, root temp files | ORPHAN_SUSPECT / UNKNOWN | TEMP-SCRATCH / UNKNOWN-TRACKING | READ | Temporary/scratch roots exist; no deletion action taken. |
| INVFIND-019 | `Content/VFX/Idols`, `Content/World/Cliffs`, `Content/World/Sky`, `Collections`, `Developers`, `T66MapAssets` | STUB | IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | READ | Empty or near-empty placeholder roots; may be intentional stubs. |
| INVFIND-020 | `RuntimeDependencies/T66` extensionless files | UNKNOWN | IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | READ | Many extensionless runtime dependency files were seen but not individually classified. |

## 12. Deferred Git-Tracking Register

Because no git commands were allowed, this pass did not determine:

| Deferred ID | Scope | Current Tracking Tag | Needed Later | Why Deferred |
|---|---|---|---|---|
| INV-DEFER-001 | All source/config/docs/scripts | UNKNOWN-TRACKING | `git status` / path-limited index inspection | Git operations prohibited. |
| INV-DEFER-002 | All `.uasset`, `.umap`, `.ubulk`, `.uexp`, `.mp4`, `.png`, `.fbx`, `.glb`, `.obj`, `.blend`, `.wav`, `.zip` | UNKNOWN-TRACKING | LFS pointer verification and tracked binary audit | `.gitattributes` gives policy, not actual pointer state. |
| INV-DEFER-003 | `Saved`, `Intermediate`, `Binaries`, `Build`, `tmp`, `SourceAssets`, model/video generated roots | UNKNOWN-TRACKING | Ignored-vs-tracked-vs-untracked audit | `.gitignore` gives policy; tracked-but-ignored requires git index. |
| INV-DEFER-004 | SourceAssets promoted/currently-used art | UNKNOWN-TRACKING | Narrow reference trace plus git index inspection | Some source art may be durable provenance despite local-only policy. |
| INV-DEFER-005 | Duplicate filename groups such as `Image_0.uasset`, `Material_0.uasset`, repeated review artifact names | UNKNOWN-TRACKING | Hash/reference/content-owner duplicate review | Filename repetition alone is not a duplicate proof. |
| INV-DEFER-006 | Runtime video manifest/catalog pair | UNKNOWN-TRACKING | Owner reconciliation and hash/source-of-truth decision | Current copies differ; no migration decision made. |

## 13. Factual Gaps And Risks

1. Actual tracking state is unknown for every working-tree path because git operations were prohibited. This is the main migration blocker left for a later git-enabled pass.
2. LFS compliance is unknown. Policy routes binaries/media to LFS, but no pointer verification was performed.
3. Runtime verification was not performed. Lifecycle statuses are based on reads and static traces, not current build/editor/game execution.
4. Some comments/docs use deprecated/unused/reserved language that lags current reality. This audit treats those labels as evidence to verify, not final truth.
5. Mini/minigame source and data are present and demo-gated, not deprecated. A future migration that excludes minigames would be a scope decision, not a current-state conclusion.
6. Generated/cooked/staged bulk is too large for row-level enumeration and is intentionally summarized. The largest current migration-risk bulk is `Saved` at 101.71 GB plus local `.git` cache at 149.94 GB.
7. `SourceAssets` is both policy-local and operationally referenced by scripts/build loose dependencies. A future migration decision must distinguish durable provenance from temporary import residue.
8. `ThirdParty/WebView2` is active by static trace but lacks local provenance/version/license metadata in this checkout.

## 14. Evidence Index

Primary process/config evidence:

| Evidence | Path / Line |
|---|---|
| Operator state | `.t66/operator-state.json` read in this pass: Operator Codex, Validator Claude. |
| Audit ownership | `Audit/AUDIT_AGENTS.md:14`; active pending audits belong under `Audit/Pending`. |
| Project version/modules | `T66.uproject:3`, `T66.uproject:8`, `:13`, `:18`, `:23`, `:28`, `:33`. |
| Runtime loose roots | `Config/DefaultGame.ini:48`-`:62`. |
| Demo/minigame lifecycle | `Demo/DEPRECATED_CONTENT.md:44`; `Demo/DEMO_GATED_INVISIBLE_CONTENT.md:79`-`:85`. |
| Deprecated arcade settings | `Source/T66/Core/T66DeprecatedFeatureSettings.h:15`-`:19`; `Config/DefaultGame.ini:64` onward. |
| Git ignore policy | `.gitignore:2`-`:5`, `.gitignore:23`-`:29`, `.gitignore:42`-`:47`, `.gitignore:48`-`:93`. |
| LFS policy | `.gitattributes:5`-`:24`. |
| Script ownership | `Scripts/README.md:3`, `Scripts/README.md:11`-`:19`, `Scripts/README.md:68`. |
| Minigame ownership | `Gameplay/Minigames/MINIGAMES_AGENTS.md:3`-`:5`, `Gameplay/Minigames/MINIGAMES_AGENTS.md:19`-`:21`. |
| Video runtime catalog | `Source/T66/UI/T66FrontendVideoCatalog.cpp:14`; `RuntimeDependencies/T66/Video/frontend_videos.json:8`. |
| WebView2 dependency | `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:68`; `Source/T66/T66.Build.cs:62`; `Source/T66/Core/T66WebView2Host.cpp:338`. |
| Performance schema drift | `PerformanceSystem/README.md:60`; `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:43`. |
| UI registry gap | `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md:856`. |
| Data table runtime source | `Source/T66/Core/T66GameInstance.h:66`-`:150`; `Source/T66/Core/T66GameInstance.h:342`-`:440`. |
| Pet compatibility fallback | `Source/T66/Core/T66GameInstance.h:513`-`:519`. |

Subagent evidence slices were read-only and closed:

| Agent | Slice | Result |
|---|---|---|
| Mencius `019e8740-f227-7aa0-a00c-daf1bce9ff7a` | Filesystem/generated bulk scale | Completed; no edits, no git. |
| Boole `019e8741-0615-7841-aff1-d6064050a8ec` | Data/config/content/Mini | Completed; no edits, no git. |
| Godel `019e8741-1a50-75f3-bb6a-ac3b298599f4` | Source/scripts/modules | Completed; no edits, no git. |
| Dalton `019e8741-2e73-7852-b504-56208dcda46c` | Docs/provenance/lifecycle | Completed; no edits, no git. |

## 15. Validation Status

Claude independent-answer pass was run before the document draft and returned `OK` with artifact:

`Saved/AgentReviews/InventoryAuditFinal/20260602T043306-IndependentAnswer-pass1/claude_review_pass1.md`

Claude cross-review pass returned `OK` on 2026-06-02 with no blocking issues. Artifact:

`Saved/AgentReviews/InventoryAuditFinal/20260602T045818-CrossReview-pass2/claude_review_pass2.md`

Cross-review cleanup applied: closed this validation section, clarified that `SHOULD-NOT-TRACK` was intentionally not assigned in the no-git pass, and kept the pet fallback tag as static/comment evidence rather than runtime proof.
