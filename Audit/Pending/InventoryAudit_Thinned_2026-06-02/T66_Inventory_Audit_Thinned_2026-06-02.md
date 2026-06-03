# T66 Inventory Audit - Thinned Tree Rerun - 2026-06-02

Operator: Codex  
Validator: Claude Code  
Scope: Current `C:\UE\T66` inventory after minigame/arcade demolition. Descriptive, read-only, no git, no deletion, no cleanup, no migration design.

## 1. Scope And Method

This is a fresh current-state inventory of the thinned tree. It replaces the prior inventory snapshot for migration planning purposes, but it does not delete, move, reclassify in git, or design the future repository.

No git commands were run. Therefore every current path whose actual index state would require git is marked `UNKNOWN-TRACKING`. `.gitignore` and `.gitattributes` are used only as policy evidence: `.gitignore:2`-`.gitignore:5` ignores `Binaries/`, `DerivedDataCache/`, `Intermediate/`, and `Saved/`; `.gitignore:42`-`.gitignore:47` says `SourceAssets/` and generated/review artifacts are local-policy areas; `.gitattributes:5`-`.gitattributes:24` routes Unreal assets, media, models, images, audio, and archives to LFS when tracked.

Evidence tiers:

| Tier | Meaning |
|---|---|
| READ | Direct file/path/config/doc/filesystem read in this rerun. |
| STATIC_TRACE | Current source/config reference supporting inventory state. |
| PRIOR_ARTIFACT | Prior report/review/proof artifact, not rerun as runtime proof. |
| RUNTIME_VERIFIED | Current runtime/editor/build proof. Not used in this read-only rerun. |

Lifecycle tokens used: `ACTIVE`, `DEMO_GATED`, `HIDDEN_RUNTIME`, `PARTIAL`, `DEPRECATED`, `COMPAT_LEGACY`, `BROKEN`, `STUB`, `ORPHAN_SUSPECT`, `UNKNOWN`, `SHELVED`.

`SHELVED` means built-but-disabled/parked and preserved for later. It is distinct from `DEPRECATED`, which is a deletion-candidate lifecycle. In this rerun, Mini/TD/Idle/Deck/Versus/Daily Descent shell routing and shelved gates are `SHELVED`. `T66Buried` is requested by the prompt but not found as a current source class/screen; it is tracked as a missing expected shelved surface.

Tracking axis:

| Tracking Token | Meaning In This Audit |
|---|---|
| UNKNOWN-TRACKING | Actual tracking was not inspected because git is prohibited. Assigned broadly. |
| SHOULD-NOT-TRACK | Migration/review flag based on policy/origin only; not an actual git state. Used only for obvious generated/cache/temp groups. |
| TRACKED / UNTRACKED / IGNORED / TRACKED-BUT-IGNORED / LFS-POINTER / TRACKED-NONLFS-BINARY | Not assigned here because they require git/index/LFS inspection. |

Origin axis: `SOURCE-AUTHORED`, `SOURCE-DATA`, `IMPORTED-RUNTIME-ASSET`, `LOCAL-SOURCE-ART`, `EXTERNAL-DEPENDENCY`, `GENERATED-DERIVED`, `COOKED-STAGED`, `PROOF-ARTIFACT`, `TEMP-SCRATCH`.

## 2. Current Scale And Delta

Prior baseline from the first full inventory: `98,722` working-tree files excluding `.git`, `134.03 GiB`. The prior user-facing wording said `GB`, but the scan used PowerShell byte division by `1GB`, which is GiB-equivalent; current sizes use the same basis.

Current live scan excluding `.git` after this rerun's local validator prompt artifact and audit folder creation: `97,311` files, `16,795` dirs, `133.98 GiB`.

Read-only subagent scan taken just before local artifacts were added: `97,306` files, `16,794` dirs, `133.98 GiB`.

Delta versus prior baseline using the local final pre-document scan: `-1,411` files, `-0.05 GiB`.

The file-count delta is larger than the user's approximate `~709` deleted files because this inventory compares two full working-tree snapshots, not just the demolition list. It includes other intervening file churn and audit/review artifacts while still using the same `.git` exclusion set.

`.git` was measured separately as local repository cache: `56,263` files, `25,076` dirs, `149.94 GiB`; `.git/lfs` alone was `140.43 GiB`.

Largest current roots:

| Inventory ID | Root | Files | Size | Tracking | Origin | Lifecycle | Evidence | Notes |
|---|---|---:|---:|---|---|---|---|---|
| INV-BULK-001 | `Saved` | 75,627 | 101.65 GiB | UNKNOWN-TRACKING | PROOF-ARTIFACT / COOKED-STAGED | UNKNOWN | READ | Dominant working-tree bulk; policy ignored at `.gitignore:5`. |
| INV-MODEL-001 | `Model Generation` | 6,236 | 9.49 GiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED | PARTIAL | READ | Raw generation/provenance and runs. |
| INV-ART-001 | `SourceAssets` | 2,791 | 8.37 GiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE / PARTIAL | READ | Mini/TD/Idle/Deck roots gone; arcade/UI/source-art residue remains. |
| INV-CONTENT-001 | `Content` | 3,938 | 7.06 GiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET / SOURCE-DATA | ACTIVE / SHELVED / STUB | READ | Main runtime content; Mini/TD/Idle/Deck roots gone. |
| INV-BUILD-001 | `Intermediate` | 2,578 | 5.38 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | READ | Build intermediates; policy ignored at `.gitignore:4`. |
| INV-BUILD-002 | `Binaries` | 35 | 1.22 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | READ | Local build products; policy ignored at `.gitignore:2`. |
| INV-VIDEO-001 | `Video Generation` | 446 | 251.61 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED | PARTIAL | READ | Prompt/manifest/run provenance. |
| INV-RUNTIME-001 | `RuntimeDependencies` | 817 | 207.27 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / SHELVED / PARTIAL | READ / STATIC_TRACE | Loose staged roots still include Arcade/UI/Video/Fonts. |
| INV-REPORT-001 | `Reports` | 2,549 | 189.85 MiB | UNKNOWN-TRACKING | PROOF-ARTIFACT / SOURCE-AUTHORED | ACTIVE / UNKNOWN | READ | Historical and active review artifacts. |
| INV-UI-001 | `UI` | 217 | 77.41 MiB | UNKNOWN-TRACKING | SOURCE-AUTHORED / LOCAL-SOURCE-ART | ACTIVE / PARTIAL | READ | UI instructions/reference/checklists. |
| INV-AUDIT-001 | `Audit` | 174 | 60.05 MiB | UNKNOWN-TRACKING | SOURCE-AUTHORED / PROOF-ARTIFACT | ACTIVE / PARTIAL | READ | Pending audits now exist despite stale README. |
| INV-TEMP-001 | `tmp` | 852 | 24.23 MiB | UNKNOWN-TRACKING | TEMP-SCRATCH | ORPHAN_SUSPECT | READ | Policy ignored at `.gitignore:28`. |
| INV-SOURCE-001 | `Source` | 661 | 8.40 MiB | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / SHELVED | READ | Only `T66` and `T66Editor` modules remain. |

Largest extension families:

| Ext | Files | Size | Notes |
|---|---:|---:|---|
| `.png` | 51,974 | 49.78 GiB | LFS policy if tracked; many generated/provenance images. |
| `.ucas` | 28 | 34.32 GiB | Cooked/staged package containers. |
| `.uasset` | 7,742 | 7.01 GiB | Runtime/cooked Unreal assets; LFS policy if tracked. |
| `.ubulk` | 1,067 | 6.74 GiB | Unreal payloads; LFS policy if tracked. |
| `.pdb` | 22 | 6.30 GiB | Local symbols; policy ignored by `*.pdb` at `.gitignore:16`. |
| `.uexp` | 4,060 | 5.81 GiB | Unreal payloads; LFS policy if tracked. |
| `.exe` | 33 | 4.74 GiB | Local binaries/staged products. |
| `.pch` | 2 | 4.19 GiB | Build intermediates. |
| `.fbx` | 900 | 3.55 GiB | Model source/provenance; LFS policy if tracked. |
| `.glb` | 430 | 2.17 GiB | Model source/provenance; LFS policy if tracked. |

## 3. Confirmed Deleted Roots

The following roots are absent in the current tree and are not re-enumerated:

| Inventory ID | Deleted Path | Current State | Evidence |
|---|---|---|---|
| INV-MINI-001 | `Content/Mini` | Gone | READ filesystem enumeration |
| INV-TD-001 | `Content/TD` | Gone | READ filesystem enumeration |
| INV-IDLE-001 | `Content/Idle` | Gone | READ filesystem enumeration |
| INV-DECK-001 | `Content/Deck` | Gone | READ filesystem enumeration |
| INV-MINI-002 | `SourceAssets/Mini` | Gone | READ filesystem enumeration |
| INV-TD-002 | `SourceAssets/TD` | Gone | READ filesystem enumeration |
| INV-IDLE-002 | `SourceAssets/Idle` | Gone | READ filesystem enumeration |
| INV-DECK-002 | `SourceAssets/Deck` | Gone | READ filesystem enumeration |
| INV-MINI-003 | `Source/T66Mini` | Gone | READ filesystem enumeration |
| INV-TD-003 | `Source/T66TD` | Gone | READ filesystem enumeration |
| INV-IDLE-003 | `Source/T66Idle` | Gone | READ filesystem enumeration |
| INV-DECK-003 | `Source/T66Deck` | Gone | READ filesystem enumeration |

`T66.uproject` now declares only `T66` and `T66Editor`: `T66.uproject:6`-`:16`.

## 4. Core Inventory Matrix

| Inventory ID | Path | Root | Kind | Size | Ext | Tracking | Origin | Lifecycle | Runtime/Owner | Content ID | Technical ID | Evidence | Notes |
|---|---|---|---|---:|---|---|---|---|---|---|---|---|---|
| INV-CONFIG-001 | `T66.uproject` | root | Unreal project descriptor | small | `.uproject` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Project/module registry | CONTENT-CONFIG-001 | TECH-CONFIG-001 | READ | Engine 5.7 at `T66.uproject:3`; only `T66` and `T66Editor` modules remain at `T66.uproject:8` and `T66.uproject:13`. |
| INV-CONFIG-002 | `Config/*.ini` | Config | Unreal config set | 15 files / 0.10 MiB | `.ini` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / SHELVED | Runtime/editor config | CONTENT-CONFIG-002 | TECH-CONFIG-002 | READ | `DefaultGame.ini:47`-`:52` loose roots; `DefaultGame.ini:54`-`:56` deprecated arcade settings. |
| INV-CONFIG-003 | `.gitignore`, `.gitattributes` | root | repo policy | small | text | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Repo hygiene policy | CONTENT-CONFIG-003 | TECH-CONFIG-003 | READ | Policy evidence only; actual git state deferred. |
| INV-SOURCE-002 | `Source/T66` | Source | runtime module source | 648 files total; 645 in child folders plus 3 module files | `.h`, `.cpp`, `.cs`, `.md` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / SHELVED | Main runtime | CONTENT-SOURCE-001 | TECH-SOURCE-001 | READ / STATIC_TRACE | Shelved shell/gate now live inside main module. |
| INV-SOURCE-003 | `Source/T66Editor` | Source | editor module source | 11 files | `.h`, `.cpp`, `.cs` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | Editor commandlets/tools | CONTENT-SOURCE-002 | TECH-SOURCE-002 | READ | Only editor module remaining besides `T66`. |
| INV-SHELL-001 | `Source/T66/Public/Core/T66ShelvedFeatureGate.h`, `Source/T66/Core/T66ShelvedFeatureGate.cpp` | Source | shelved feature gate | 2 files | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Central shelved feature switch | CONTENT-SHELL-001 | TECH-SHELL-001 | READ / STATIC_TRACE | Enum features at `T66ShelvedFeatureGate.h:8`-`:14`; all booleans hardcoded false at `T66ShelvedFeatureGate.cpp:7`-`:10`. |
| INV-SHELL-002 | `Source/T66/UI/Screens/T66ShelvedFeatureScreen.*` | Source | shared shelved screen | 2 files | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | UI shell for parked modes | CONTENT-SHELL-002 | TECH-SHELL-002 | READ / STATIC_TRACE | Displays `FEATURE SHELVED` and preserved-for-later body at `T66ShelvedFeatureScreen.cpp:31` and `:42`. |
| INV-MINI-004 | Mini screen tokens in `ET66ScreenType` | Source | shelved Mini routing | enum + resolver | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Shared shell | CONTENT-MINI-001 | TECH-MINI-001 | STATIC_TRACE | Mini enum tokens at `T66UITypes.h:45`-`:52`, `:63`; resolver returns shared shell at `T66PlayerController_Frontend.cpp:704`-`:721`. |
| INV-TD-004 | TD screen tokens in `ET66ScreenType` | Source | shelved TD routing | enum + resolver | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Shared shell | CONTENT-TD-001 | TECH-TD-001 | STATIC_TRACE | TD tokens at `T66UITypes.h:55`-`:57`; resolver returns shared shell at `T66PlayerController_Frontend.cpp:715`-`:721`. |
| INV-IDLE-004 | Idle screen token in `ET66ScreenType` | Source | shelved Idle routing | enum + resolver | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Shared shell | CONTENT-IDLE-001 | TECH-IDLE-001 | STATIC_TRACE | Idle token at `T66UITypes.h:60`; resolver returns shared shell at `T66PlayerController_Frontend.cpp:718`-`:721`. |
| INV-DECK-004 | Deck screen token in `ET66ScreenType` | Source | shelved Deck routing | enum + resolver | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Shared shell | CONTENT-DECK-001 | TECH-DECK-001 | STATIC_TRACE | Deck token at `T66UITypes.h:61`; resolver returns shared shell at `T66PlayerController_Frontend.cpp:719`-`:721`. |
| INV-VERSUS-001 | `VersusMainMenu` token and shelved gate | Source | shelved Versus route | enum + resolver | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED / PARTIAL | Shared shell plus residual arcade screen | CONTENT-VERSUS-001 | TECH-VERSUS-001 | STATIC_TRACE | Token at `T66UITypes.h:62`; screen allowed only if `VersusArcade` enabled at `T66ShelvedFeatureGate.cpp:62`-`:65`; residual `UT66VersusArcadeScreen` remains. |
| INV-BURIED-001 | `T66Buried` expected surface | Source | expected shelved shell | n/a | n/a | UNKNOWN-TRACKING | SOURCE-AUTHORED | UNKNOWN | Not found | CONTENT-BURIED-001 | TECH-BURIED-001 | READ | Prompt expected a `SHELVED` `T66Buried` shell, but no `T66Buried`, Buried screen enum, or Buried shell class was found in current source. |
| INV-DAILY-001 | `DailyDescent` source/backend/UI surfaces | Source | shelved Daily Descent | grouped | `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | SHELVED | Gate + shared shell + backend/UI guards | CONTENT-DAILY-001 | TECH-DAILY-001 | STATIC_TRACE | Feature token at `T66ShelvedFeatureGate.h:12`; hardcoded disabled at `T66ShelvedFeatureGate.cpp:9`; screen route returns shell at `T66PlayerController_Frontend.cpp:720`-`:721`. |
| INV-DATA-001 | `Content/Data` | Content | main game data | 60 files / 1.12 MiB | `.csv`, `.json`, `.uasset`, `.md` | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET | ACTIVE / SHELVED / PARTIAL | GameInstance/data tables | CONTENT-DATA-001 | TECH-DATA-001 | READ / STATIC_TRACE | 23 CSV / 577 rows, 6 JSON / 130 entries, 30 uasset, 1 md. |
| INV-ARCADE-001 | `ArcadeInteractables.json`, `DT_ArcadeInteractables.uasset`, arcade content/source/runtime residues | multiple | shelved/disabled arcade surface | grouped | mixed | UNKNOWN-TRACKING | SOURCE-DATA / IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | SHELVED / DEPRECATED / ORPHAN_SUSPECT | Deprecated settings + shelved gate | CONTENT-ARCADE-001 | TECH-ARCADE-001 | READ / STATIC_TRACE | `Content/Arcade` gone, but arcade residues remain; see findings. |
| INV-RUNTIME-002 | `RuntimeDependencies/T66` | RuntimeDependencies | loose runtime dependencies | 817 files / 207.27 MiB | mixed | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / SHELVED / PARTIAL | Runtime UI/video/font/arcade loaders | CONTENT-RUNTIME-001 | TECH-RUNTIME-001 | STATIC_TRACE | `DefaultGame.ini:47`-`:52`; `T66.Build.cs:86`-`:90`. |
| INV-UI-002 | `Source/T66/UI`, `Content/UI`, `RuntimeDependencies/T66/UI`, `UI` | multiple | UI source/assets/reference | grouped | mixed | UNKNOWN-TRACKING | SOURCE-AUTHORED / IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART | ACTIVE / SHELVED / PARTIAL | UI manager/screens/style/runtime UI | CONTENT-UI-001 | TECH-UI-001 | READ / STATIC_TRACE | `Source/T66/UI` has 203 files; Minigames and Versus residues remain. |
| INV-SCRIPT-001 | `Scripts` | Scripts | automation scripts | 176 files / 1.35 MiB | `.py`, `.ps1`, `.md`, `.pyc` | UNKNOWN-TRACKING | SOURCE-AUTHORED / GENERATED-DERIVED | ACTIVE / ORPHAN_SUSPECT | Import/setup/capture/review tooling | CONTENT-SCRIPT-001 | TECH-SCRIPT-001 | READ | 39 `.pyc` files remain under `Scripts/__pycache__`. |
| INV-MODEL-002 | `Model Generation` | Model Generation | model generation/provenance | 6,236 files / 9.49 GiB | mixed | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED | PARTIAL | ToonStyle/model pipeline | CONTENT-MODEL-001 | TECH-MODEL-001 | READ | Grouped, not row-enumerated. |
| INV-VIDEO-002 | `Video Generation`, `Content/Movies`, `RuntimeDependencies/T66/Video` | multiple | video generation/runtime registration | grouped | mixed | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / IMPORTED-RUNTIME-ASSET / GENERATED-DERIVED | ACTIVE / PARTIAL | Frontend video catalog | CONTENT-VIDEO-001 | TECH-VIDEO-001 | READ / STATIC_TRACE | Runtime manifest drift remains. |
| INV-THIRD-001 | `ThirdParty/WebView2` | ThirdParty | external dependency | 3 files / 2.79 MiB | `.dll`, `.h` | UNKNOWN-TRACKING | EXTERNAL-DEPENDENCY | ACTIVE / PARTIAL | WebView2 runtime support | CONTENT-THIRD-001 | TECH-THIRD-001 | READ / STATIC_TRACE | Header references license, but no local license/version manifest found. |
| INV-PERF-001 | `PerformanceSystem`, `Source/T66/PerformanceSystem` | PerformanceSystem / Source | performance docs/schema/source | 40 files + 9 source files | `.md`, `.json`, `.h`, `.cpp` | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / PARTIAL | Performance diagnostics | CONTENT-PERF-001 | TECH-PERF-001 | READ / STATIC_TRACE | README/runtime schema drift remains. |

## 5. Source And Shelved Surface Register

Current source modules:

| Inventory ID | Path | Files | Tracking | Origin | Lifecycle | Evidence | Notes |
|---|---|---:|---|---|---|---|---|
| INV-SOURCE-004 | `Source/T66` | 648 files; child folders: Gameplay 250, UI 203, Core 169, Public 12, PerformanceSystem 9, Data 2, plus module files | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE / SHELVED | READ | Main runtime now owns all remaining gameplay/UI/shelved shell code. Overall `Source` root is 661 files including `T66Editor` and target files. |
| INV-SOURCE-005 | `Source/T66Editor` | 11 | UNKNOWN-TRACKING | SOURCE-AUTHORED | ACTIVE | READ | Editor-only commandlets/tools. |
| INV-SOURCE-006 | `Source/T66Mini`, `Source/T66TD`, `Source/T66Idle`, `Source/T66Deck` | 0 | UNKNOWN-TRACKING | n/a | Gone | READ | Side module scaffolding removed. |

Shelved surface details:

| Inventory ID | Surface | Current Files / Tokens | Lifecycle | Evidence | Notes |
|---|---|---|---|---|---|
| INV-SHELL-003 | Central gate | `T66ShelvedFeatureGate.h/.cpp` | SHELVED | `T66ShelvedFeatureGate.h:8`-`:23`, `T66ShelvedFeatureGate.cpp:7`-`:10`, `:38`-`:52` | Features: `MinigameBundle`, `VersusArcade`, `DailyDescent`, `ArcadeInteractables`; all disabled. |
| INV-SHELL-004 | Shared shell class | `T66ShelvedFeatureScreen.h/.cpp` | SHELVED | `T66ShelvedFeatureScreen.h:9`-`:21`, `T66ShelvedFeatureScreen.cpp:31`, `:42`, `:69`-`:72` | The only confirmed current shell class for these parked screens. |
| INV-MINI-005 | Mini route tokens | `MiniMainMenu`, `MiniSaveSlots`, `MiniCharacterSelect`, `MiniDifficultySelect`, `MiniIdolSelect`, `MiniShop`, `MiniRunSummary`, `MiniCompanionSelect`, `MiniBattle` | SHELVED | `T66UITypes.h:45`-`:52`, `:63`; `T66ShelvedFeatureGate.cpp:16`-`:31`; `T66PlayerController_Frontend.cpp:704`-`:721` | No separate Mini code/content/source-assets remain. |
| INV-TD-005 | TD route tokens | `TDMainMenu`, `TDDifficultySelect`, `TDBattle` | SHELVED | `T66UITypes.h:55`-`:57`; `T66ShelvedFeatureGate.cpp:26`-`:28`; `T66PlayerController_Frontend.cpp:715`-`:721` | No separate TD code/content/source-assets remain. |
| INV-IDLE-005 | Idle route token | `IdleMainMenu` | SHELVED | `T66UITypes.h:60`; `T66ShelvedFeatureGate.cpp:29`; `T66PlayerController_Frontend.cpp:718`-`:721` | No separate Idle code/content/source-assets remain. |
| INV-DECK-005 | Deck route token | `DeckMainMenu` | SHELVED | `T66UITypes.h:61`; `T66ShelvedFeatureGate.cpp:30`; `T66PlayerController_Frontend.cpp:719`-`:721` | No separate Deck code/content/source-assets remain. |
| INV-VERSUS-002 | Versus route token | `VersusMainMenu`; residual `T66VersusArcadeScreen.*` | SHELVED / PARTIAL | `T66UITypes.h:62`; `T66ShelvedFeatureGate.cpp:62`-`:65`; `T66VersusArcadeScreen.cpp:68`-`:72`, `:296`-`:310` | Current resolver maps Versus to shell, but old arcade launcher class still exists. |
| INV-DAILY-002 | Daily Descent | `DailyDescent` screen token and backend/UI references | SHELVED | `T66ShelvedFeatureGate.h:12`; `T66ShelvedFeatureGate.cpp:46`-`:47`, `:67`-`:70`; `T66PlayerController_Frontend.cpp:720`-`:721` | Daily Descent preserved but disabled by gate. |
| INV-BURIED-002 | Buried | none found | UNKNOWN | READ broad search | Prompt says `T66Buried` shell was added and should be `SHELVED`; current source has no matching class, enum, or README. |

## 6. Data Table Register

`Content/Data` is unchanged in shape from the prior inventory: 60 files, 29 source data files, 30 `.uasset` assets, and 1 pending issue file.

| Inventory ID | Source File(s) | Runtime Asset(s) | Rows/Entries | Lifecycle | Evidence | Notes |
|---|---|---|---:|---|---|---|
| INV-DATA-002 | `ArcadeInteractables.json` | `DT_ArcadeInteractables.uasset` | 14 | SHELVED / DEPRECATED | READ / STATIC_TRACE | Data remains while arcade games/interactables are disabled/shelved. |
| INV-DATA-003 | `AudioEvents.json` | `DT_AudioEvents.uasset` | 96 | ACTIVE | READ | Audio event source. |
| INV-BOSS-001 | `BossAttackDefinitions.csv` | `DT_BossAttackDefinitions.uasset` | 25 | ACTIVE | READ | Boss attacks. |
| INV-BOSS-002 | `BossAttacks.csv` | `DT_BossAttacks.uasset` | 50 | ACTIVE | READ | Boss attacks. |
| INV-BOSS-003 | `BossEncounterMembers.csv` | `DT_BossEncounterMembers.uasset` | 23 | ACTIVE | READ | Encounter members. |
| INV-BOSS-004 | `BossEncounters.csv` | `DT_BossEncounters.uasset` | 20 | ACTIVE | READ | Boss encounters. |
| INV-BOSS-005 | `Bosses.csv` | `DT_Bosses.uasset` | 23 | ACTIVE / COMPAT_LEGACY | READ / STATIC_TRACE | Also supports pet fallback. |
| INV-BOSS-006 | `BossHazardDefinitions.csv` | `DT_BossHazardDefinitions.uasset` | 6 | ACTIVE | READ | Boss hazards. |
| INV-BOSS-007 | `BossMovementPatterns.csv` | `DT_BossMovementPatterns.uasset` | 8 | ACTIVE | READ | Boss movement. |
| INV-HERO-001 | `CharacterVisuals.csv` | `DT_CharacterVisuals.uasset` | 133 | ACTIVE | READ | Character visual map. |
| INV-COMBAT-001 | `CombatVFXBindings.csv` | `DT_CombatVFXBindings.uasset` | 20 | ACTIVE | READ | Combat VFX bindings. |
| INV-COMPANION-001 | `Companions.csv` | `DT_Companions.uasset` | 16 | ACTIVE | READ | Companion roster. |
| INV-DATA-004 | `DifficultyTuning.json` | `DT_DifficultyTuning.uasset` | 5 | ACTIVE | READ | Difficulty tuning. |
| INV-ENEMY-001 | `Enemies.csv` | `DT_Enemies.uasset` | 60 | ACTIVE / PARTIAL | READ / STATIC_TRACE | `StatusEffectOnHit=None` for all rows; see findings. |
| INV-HERO-002 | `Heroes.csv` | `DT_Heroes.uasset` | 12 | ACTIVE | READ | Hero roster. |
| INV-IDOL-001 | `Idols.csv` | `DT_Idols.uasset` | 16 | ACTIVE | READ | Idol roster. |
| INV-ITEM-001 | `Items.csv` | `DT_Items.uasset` | 30 | ACTIVE / ORPHAN_SUSPECT | READ | Removed item sprite residue remains. |
| INV-DATA-005 | `Leaderboard_ScoreTargets.csv` | `Leaderboard_ScoreTargets.uasset` | 20 | ACTIVE | READ | Non-`DT_` runtime asset naming. |
| INV-DATA-006 | `Leaderboard_SpeedrunTargets.csv` | `DT_Leaderboard_SpeedrunTargets.uasset` | 20 | ACTIVE | READ | Speedrun leaderboard data. |
| INV-ECONOMY-001 | `LoanShark.csv` | `DT_LoanShark.uasset`, `LoanShark.uasset` | 1 | PARTIAL | READ | Both `DT_` and legacy non-`DT_` assets remain. |
| INV-DATA-007 | `MobVertexAnimations.csv` | `DT_MobVertexAnimations.uasset` | 10 | ACTIVE | READ | VAT/animation data. |
| INV-WORLD-001 | `NPCs.csv` | `DT_NPCs.uasset` | 3 | ACTIVE | READ | NPC data. |
| INV-DATA-008 | `PlayerExperience.json` | `DT_PlayerExperience.uasset` | 5 | ACTIVE | READ | Player experience tuning. |
| INV-DATA-009 | `Stages.csv` | `DT_Stages.uasset` | 20 | ACTIVE | READ | Stage data. |
| INV-COMBAT-002 | `StatusEffects.csv` | `DT_StatusEffects.uasset` | 12 | PARTIAL | READ / STATIC_TRACE | Status effect rows exist; production enemy assignment still `None`. |
| INV-ENEMY-002 | `UniqueEnemies.csv` | `DT_UniqueEnemies.uasset` | 1 | ACTIVE | READ | Unique enemy data. |
| INV-WORLD-002 | `VehicleInteractables.json` | `DT_VehicleInteractables.uasset` | 6 | ACTIVE | READ | Vehicle interactables. |
| INV-WEAPON-001 | `Weapons.csv` | `DT_Weapons.uasset` | 48 | ACTIVE | READ | Weapon data. |
| INV-WORLD-003 | `WorldVisualProps.json` | `DT_WorldVisualProps.uasset` | 4 | ACTIVE / PARTIAL | READ | Pending issue still notes missing prop references. |
| INV-DATA-010 | `pending_issues_Data.md` | n/a | n/a | ACTIVE | READ | Carry-forward data findings source. |

## 7. Content And Runtime Breakdown

| Inventory ID | Root | Files | Size | Tracking | Origin | Lifecycle | Evidence | Notes |
|---|---|---:|---:|---|---|---|---|---|
| INV-CONTENT-002 | `Content/Characters` | 1,317 | 4.53 GiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | Heroes, companions, mobs, VAT, NPC/enemy assets. |
| INV-CONTENT-003 | `Content/World` | 489 | 1.18 GiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / SHELVED / STUB | READ | Includes arcade and arcade-machine residues. |
| INV-CONTENT-004 | `Content/Stylized_VFX_StPack` | 591 | 471.9 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | HIDDEN_RUNTIME | READ | Imported VFX/demo pack content. |
| INV-CONTENT-005 | `Content/ToonStyle` | 134 | 438.5 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ | ToonStyle/test assets. |
| INV-CONTENT-006 | `Content/Weapons` | 228 | 105.1 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | Weapon/projectile assets. |
| INV-CONTENT-007 | `Content/UE5RFX` | 289 | 101.6 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | HIDDEN_RUNTIME | READ | Imported retro FX demo/benchmark pack. |
| INV-AUDIO-001 | `Content/Audio` | 155 | 94.3 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / SHELVED | READ | Includes `Content/Audio/Arcade` residue. |
| INV-UI-003 | `Content/UI` | 202 | 77.2 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ | UI sprites/materials; `Content/UI/Minigames` is gone. |
| INV-VIDEO-003 | `Content/Movies` | 99 | 34.9 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | READ | Physical movies include more clips than runtime manifest exposes. |
| INV-IDOL-002 | `Content/Idols` | 48 | 26.3 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | READ | Idol sprites. |
| INV-VFX-001 | `Content/VFX` | 97 | 6.08 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / STUB | READ | `Content/VFX/Idols` still empty. |
| INV-ITEM-002 | `Content/Items` | 121 | 4.88 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / ORPHAN_SUSPECT | READ | Removed `Item_HpRegen` and `Item_LifeSteal` sprites remain. |
| INV-MAP-001 | `Content/Maps` | 3 | 0.02 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET / SOURCE-AUTHORED | ACTIVE | READ | Runtime maps and pending issue file. |
| INV-STUB-001 | `Content/Collections`, `Content/Developers`, `Content/T66MapAssets` | 0 | 0 | UNKNOWN-TRACKING | UNKNOWN | STUB | READ | Empty top-level roots. |
| INV-RUNTIME-003 | `RuntimeDependencies/T66/Arcade` | 61 | 16.95 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | SHELVED / ORPHAN_SUSPECT | READ / STATIC_TRACE | Still staged by `DefaultGame.ini:48` and `T66.Build.cs:86`. |
| INV-RUNTIME-004 | `RuntimeDependencies/T66/Fonts` | 2 | 0.08 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE | STATIC_TRACE | `DefaultGame.ini:47`, `T66.Build.cs:87`. |
| INV-RUNTIME-005 | `RuntimeDependencies/T66/UI` | 657 | 171.25 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | STATIC_TRACE | `DefaultGame.ini:49`, `T66.Build.cs:88`. |
| INV-RUNTIME-006 | `RuntimeDependencies/T66/Video` | 97 | 18.99 MiB | UNKNOWN-TRACKING | IMPORTED-RUNTIME-ASSET | ACTIVE / PARTIAL | STATIC_TRACE | `DefaultGame.ini:50`, `T66.Build.cs:89`. |

## 8. SourceAssets And Local Source Art

| Inventory ID | Root | Files | Size | Tracking | Origin | Lifecycle | Evidence | Notes |
|---|---|---:|---:|---|---|---|---|---|
| INV-ART-002 | `SourceAssets/ToonStyle` | 2,261 | 8.04 GiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE / PARTIAL | READ | Dominates source art size. |
| INV-ART-003 | `SourceAssets/WeaponSprites` | 254 | 231.39 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE | READ | Weapon source sprites. |
| INV-ART-004 | `SourceAssets/Audio` | 136 | 67.76 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE | READ | Audio source/provenance. |
| INV-ART-005 | `SourceAssets/IdolSprites` | 72 | 151.83 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE | READ | Idol source sprites. |
| INV-ARCADE-002 | `SourceAssets/Arcade` | 26 | 1.67 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | SHELVED / ORPHAN_SUSPECT | READ | Arcade source-art residue after demolition. |
| INV-ART-006 | `SourceAssets/Import` | 16 | 64.11 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | PARTIAL | READ | Import source residue/provenance. |
| INV-UI-004 | `SourceAssets/UI` | 11 | 2.50 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | PARTIAL | READ | `SourceAssets/UI/ContentStubs` still referenced; registry missing. |
| INV-ART-007 | `SourceAssets/Backrooms` | 5 | 9.41 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE | READ | Backrooms source art. |
| INV-ITEM-003 | `SourceAssets/ItemSprites` | 1 | ~250 B | UNKNOWN-TRACKING | LOCAL-SOURCE-ART | ACTIVE / PARTIAL | STATIC_TRACE | Still listed as loose root at `DefaultGame.ini:52`. |

## 9. Generated, Staged, Cooked, Cache Register

Grouped by scale. These are not enumerated row-by-row.

| Inventory ID | Path | Files | Size | Tracking | Origin | Lifecycle | Notes |
|---|---|---:|---:|---|---|---|---|
| INV-BULK-002 | `Saved/VideoCaptures` | 24,324 | 29.17 GiB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Capture/proof bulk. |
| INV-BULK-003 | `Saved/D2` | 12,671 | 24.93 GiB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged/cooked proof/build output. |
| INV-BULK-004 | `Saved/Cooked` | 9,231 | 12.99 GiB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Cooked Unreal output. |
| INV-BULK-005 | `Saved/StagedBuilds` | 1,649 | 10.38 GiB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged build output. |
| INV-BULK-006 | `Saved/Codex` | 6,416 | 7.09 GiB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Codex artifacts/logs. |
| INV-BULK-007 | `Saved/StagedBuilds_PetMobLootFoundation` | 1,593 | 5.26 GiB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Staged build variant. |
| INV-BULK-008 | `Intermediate/Build` | 2,077 | 5.18 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Build intermediates. |
| INV-BULK-009 | `Saved/StagedBuildsDemo` | 1,590 | 4.86 GiB | UNKNOWN-TRACKING | COOKED-STAGED | UNKNOWN | Demo staged build. |
| INV-BULK-010 | `Model Generation/Runs` | 2,347 | 4.84 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED / LOCAL-SOURCE-ART | PARTIAL | Raw generation runs. |
| INV-BULK-011 | `Model Generation/Rigging and Animation/Runs` | 2,721 | 2.70 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED / LOCAL-SOURCE-ART | PARTIAL | Rigging/animation runs. |
| INV-BUILD-003 | `Binaries/Win64` | 35 | 1.22 GiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Local build binary/symbols. |
| INV-BULK-012 | `Saved/Crashes` | 1,661 | 1.05 GiB | UNKNOWN-TRACKING | PROOF-ARTIFACT | UNKNOWN | Crash diagnostics. |
| INV-VIDEO-004 | `Video Generation` | 446 | 251.61 MiB | UNKNOWN-TRACKING | LOCAL-SOURCE-ART / GENERATED-DERIVED | PARTIAL | Prompt/manifests/runs. |
| INV-TEMP-002 | `tmp` | 852 | 24.23 MiB | UNKNOWN-TRACKING | TEMP-SCRATCH | ORPHAN_SUSPECT | Policy ignored at `.gitignore:28`. |
| INV-BUILD-004 | `DerivedDataCache` | 5 | 1.55 MiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Policy ignored at `.gitignore:3`. |
| INV-BUILD-005 | `Build` | 2 | 2.11 MiB | UNKNOWN-TRACKING | GENERATED-DERIVED | UNKNOWN | Policy ignored at `.gitignore:24`. |

## 10. Carry-Forward And New Findings

| Finding ID | Path / Surface | Lifecycle | Tracking / Origin | Evidence | Finding |
|---|---|---|---|---|---|
| INVFIND-001 | Mini/TD/Idle/Deck deleted roots | UNKNOWN | UNKNOWN-TRACKING | READ filesystem enumeration | The deleted roots are confirmed absent. The surviving route tokens/shared shell are `SHELVED`; the absent roots themselves have no current lifecycle object to preserve. |
| INVFIND-002 | `T66Buried` expected shell | UNKNOWN | SOURCE-AUTHORED / UNKNOWN-TRACKING | Broad `rg` over source/docs | Prompt says `T66Buried` shell was added and should be `SHELVED`, but no source class, enum, or README was found. |
| INVFIND-003 | Per-mode shell docs/READMEs | PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `Gameplay/Minigames/Mini/T66Mini_MasterImplementation.md:22`, `TD/T66TD_MasterImplementation.md:11`, `Idle/T66Idle_MasterImplementation.md:5`, `Deck/T66Deck_MasterImplementation.md:5` | Demolition-era READMEs that describe new shells were not found; old docs still describe dedicated modules/assets. |
| INVFIND-004 | `UT66MinigamesScreen` | SHELVED / ORPHAN_SUSPECT | SOURCE-AUTHORED / UNKNOWN-TRACKING | `T66MinigamesScreen.cpp:297`-`:330`, `:478`-`:510` | Launcher cards and navigation handlers for Versus/Mini/TD/Deck/Idle still exist, although target modes route to shared shell and modules/content are gone. |
| INVFIND-005 | `UT66VersusArcadeScreen` | SHELVED / ORPHAN_SUSPECT | SOURCE-AUTHORED / UNKNOWN-TRACKING | `T66VersusArcadeScreen.cpp:68`-`:72`, `:296`-`:310` | Residual arcade launcher behavior remains even though current resolver maps `VersusMainMenu` to shared shell. |
| INVFIND-006 | Arcade content/source/runtime residues | SHELVED / DEPRECATED / ORPHAN_SUSPECT | mixed / UNKNOWN-TRACKING | `DefaultGame.ini:48`, `:54`-`:56`; `T66.Build.cs:86`; `T66DeprecatedFeatureSettings.cpp:9`-`:22`; filesystem enumeration | `Content/Arcade` is gone, but `Content/World/Interactables/Arcade`, `Content/World/Interactables/ArcadeMachine`, `Content/Audio/Arcade`, `SourceAssets/Arcade`, `RuntimeDependencies/T66/Arcade`, `ArcadeInteractables.json`, and `DT_ArcadeInteractables.uasset` remain. |
| INVFIND-007 | Pets table seam | COMPAT_LEGACY / PARTIAL | SOURCE-DATA / UNKNOWN-TRACKING | No `Pets.csv`/`DT_Pets`; `T66GameInstance.h:513`-`:519`, `:577`; `T66GameInstance.cpp:737`-`:768` | No `Pets.csv` or `DT_Pets.uasset`; runtime synthesizes pet fallback from boss data. |
| INVFIND-008 | `Item_HpRegen` and `Item_LifeSteal` sprites | ORPHAN_SUSPECT / DEPRECATED candidate | IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | `Content/Data/pending_issues_Data.md:17`-`:22`; filesystem | Rows absent from `Items.csv`; 8 sprite uassets remain under `Content/Items/Sprites`. |
| INVFIND-009 | Status effects assigned `None` | PARTIAL | SOURCE-DATA / UNKNOWN-TRACKING | `Content/Data/pending_issues_Data.md:3`-`:8`; `Scripts/ValidateEnemyBossRosterData.py:151` | `StatusEffects.csv` exists, but production enemies still require `StatusEffectOnHit=None`. |
| INVFIND-010 | Video README/runtime manifest drift | PARTIAL | SOURCE-DATA / IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | `Video Generation/README.md:7`-`:10`; `RuntimeDependencies/T66/Video/frontend_videos.json:1`-`:377`; `Video Generation/Manifests/README.md:7`-`:25` | README says 48 hero / 48 companion / 1 main menu; runtime manifest resolves 34 hero / 16 companion / 1 main menu. |
| INVFIND-011 | Video source/runtime manifest mismatch | PARTIAL | SOURCE-DATA / IMPORTED-RUNTIME-ASSET / UNKNOWN-TRACKING | `Video Generation/Manifests/frontend_videos.json:6`; `Video Generation/Manifests/frontend_video_jobs.json:889` | Source/jobs include entries not reflected in runtime manifest. |
| INVFIND-012 | `ThirdParty/WebView2` license/version manifest | PARTIAL | EXTERNAL-DEPENDENCY / UNKNOWN-TRACKING | `ThirdParty/WebView2/include/WebView2EnvironmentOptions.h:2`; no local license/version file found | Header references a LICENSE file, but none was found under `ThirdParty/WebView2`. |
| INVFIND-013 | `UI/content_stubs_registry.md` | BROKEN / PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md:854`-`:864`; `T66HeroSelectionScreen_Private.h:314` | Required UI content stubs registry is missing while content-stub paths remain referenced. |
| INVFIND-014 | Empty placeholder roots | STUB | UNKNOWN / UNKNOWN-TRACKING | READ filesystem enumeration | Empty roots remain: `Content/Collections`, `Content/Developers`, `Content/T66MapAssets`, `Content/VFX/Idols`, `Content/UI/Sprites/Interactables`, `Content/VFXLab/Temp/MRQ`. |
| INVFIND-015 | `.pyc` cache residue | ORPHAN_SUSPECT | GENERATED-DERIVED / UNKNOWN-TRACKING | `.gitignore:37`-`:38`; `Scripts/__pycache__` enumeration | 39 `.pyc` files remain. Actual tracking unknown. |
| INVFIND-016 | Temp scratch roots | ORPHAN_SUSPECT | TEMP-SCRATCH / UNKNOWN-TRACKING | `.gitignore:28`; filesystem enumeration | `tmp` and `UET66SavedTmpCombatVFXValidatorSelfTest_Bounce` remain as scratch roots. |
| INVFIND-017 | PIXALTEST provenance | ORPHAN_SUSPECT / PARTIAL | IMPORTED-RUNTIME-ASSET / LOCAL-SOURCE-ART / UNKNOWN-TRACKING | `Reports/ToonStyle/Phase1A/Phase1A_Preflight_Inventory.md:74`; `T66GameMode.cpp:971`-`:987`; `T66GameMode_WorldInteractables.cpp:2377`-`:2383` | PIXALTEST source/runtime/proof artifacts still exist and source still references display helpers. |
| INVFIND-018 | Performance schema drift | PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `PerformanceSystem/README.md:60`; `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp:43`; `PerformanceSystem/schema/SCHEMA_CHANGELOG.md:3` | README says runtime schema v4; code/schema changelog indicate v8. |
| INVFIND-019 | Audit README pending drift | PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `Audit/README.md:13`; pending audit files in current tree | Audit README says no active pending audit files while multiple pending audits exist. |
| INVFIND-020 | Deprecated vs shelved coexistence | PARTIAL | SOURCE-AUTHORED / UNKNOWN-TRACKING | `T66DeprecatedFeatureSettings.cpp:5`, `:9`-`:22`; `T66ShelvedFeatureGate.cpp:90`-`:104` | Shelved gate and deprecated settings coexist. Arcade is both shelved by gate and disabled by deprecated settings, so migration classification needs care. |

## 11. Deferred Git-Tracking Register

| Deferred ID | Scope | Current Tracking | Needed Later | Reason |
|---|---|---|---|---|
| INV-DEFER-001 | All current paths | UNKNOWN-TRACKING | Path-limited `git status` / index inspection | Git prohibited. |
| INV-DEFER-002 | Unreal/media/model binaries | UNKNOWN-TRACKING | LFS pointer verification | `.gitattributes` is policy only. |
| INV-DEFER-003 | `Saved`, `Intermediate`, `Binaries`, `Build`, `tmp`, generated model/video roots | UNKNOWN-TRACKING / SHOULD-NOT-TRACK policy candidates | ignored-vs-tracked-vs-untracked audit | `.gitignore` is policy only; tracked-but-ignored requires git. |
| INV-DEFER-004 | Arcade residues | UNKNOWN-TRACKING | Reference trace plus git/index state | Content/source/runtime residues remain despite demolition. |
| INV-DEFER-005 | `SourceAssets` durable provenance vs scratch | UNKNOWN-TRACKING | Owner-by-owner migration classification | `SourceAssets` is policy-local but some files are still runtime fallback/provenance. |
| INV-DEFER-006 | `T66Buried` expected shell | UNKNOWN-TRACKING | Confirm whether omitted, renamed, or planned elsewhere | Prompt says it was added; current source search did not find it. |

## 12. Evidence Index

| Evidence | Path / Lines |
|---|---|
| Operator state | `.t66/operator-state.json` read: Operator Codex, Validator Claude. |
| Audit routing | `Audit/AUDIT_AGENTS.md`; `Audit/README.md:13` stale pending statement. |
| Current modules | `T66.uproject:6`-`:16`. |
| Shelved gate | `Source/T66/Public/Core/T66ShelvedFeatureGate.h:8`-`:23`; `Source/T66/Core/T66ShelvedFeatureGate.cpp:7`-`:10`, `:38`-`:52`, `:55`-`:72`, `:90`-`:104`. |
| Shared shelved UI | `Source/T66/UI/Screens/T66ShelvedFeatureScreen.h:9`-`:21`; `Source/T66/UI/Screens/T66ShelvedFeatureScreen.cpp:31`, `:42`, `:69`-`:72`. |
| Route-to-shell resolver | `Source/T66/Gameplay/T66PlayerController_Frontend.cpp:704`-`:721`; registration starts at `:1413`. |
| Screen enum tokens | `Source/T66/UI/T66UITypes.h:45`-`:63`. |
| Arcade deprecated settings | `Config/DefaultGame.ini:54`-`:56`; `Source/T66/Core/T66DeprecatedFeatureSettings.cpp:9`-`:22`. |
| Loose runtime dependencies | `Config/DefaultGame.ini:47`-`:52`; `Source/T66/T66.Build.cs:86`-`:90`. |
| Data table pet fallback | `Source/T66/Core/T66GameInstance.h:513`-`:519`, `:577`; `Source/T66/Core/T66GameInstance.cpp:737`-`:768`. |
| Status-effect pending issue | `Content/Data/pending_issues_Data.md:3`-`:8`; `Scripts/ValidateEnemyBossRosterData.py:151`. |
| Removed item sprite issue | `Content/Data/pending_issues_Data.md:17`-`:22`. |
| UI content stubs registry requirement | `UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md:854`-`:864`. |
| Git ignore/LFS policy | `.gitignore:2`-`:5`, `:23`-`:29`, `:36`-`:47`, `:48`-`:93`; `.gitattributes:5`-`:24`. |

## 13. Validation Status

Claude independent-answer pass returned `OK`:

`Saved/AgentReviews/InventoryAuditThinned/20260602T113512-IndependentAnswer-pass1/claude_review_pass1.md`

Claude cross-review pass returned `OK` on 2026-06-02 with no blocking issues. Artifact:

`Saved/AgentReviews/InventoryAuditThinned/20260602T114905-CrossReview-pass2/claude_review_pass2.md`

Cross-review cleanup applied: clarified that size units are GiB-equivalent, annotated the `-1,411` file delta versus the user's approximate `~709` demolition count, and changed the absent-root finding so only surviving route/shell surfaces carry `SHELVED`.
