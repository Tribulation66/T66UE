# Pending Cleanup Program — Progress Log

Run start: 2026-06-09 ~23:00 local. Baseline: v1.2 (c8da91343), clean tree, usage 34% 5h / 7% weekly.
Mode: overnight autonomous (user asleep). Phases 2→3→4. Phase 1 owned by separate cleanup agent.
Limit protocol: park at ~90% five_hour, resume after resets_at; weekly hard stop ~78%.
Shared-tree protocol: targeted `git add` of own files only; wait-and-retry on cross-agent blockers.

## Checkpoint state

- Phase: 2 COMPLETE (compile-verified) · Phase 3 stats display LANDED (panel +%, primaries purged, relics +%; snapshot-writer alignment agent in flight) · Phase 4 prep DONE (16 ledger entries resolved in-style; deferred_backlog.md written; audit_retriage.md agent in flight)
- USAGE PARK: 73% five-hour at ~22:57 local; park timer armed to wake 23:41 (post-reset). On wake: StageStandaloneBuild (detached) → verification suite (lab/crate modes, durable gate slot 7, lifecycle gate, content-corrections smoke, pre-release suite, cook-warning checks: SC_*, DT_HouseNPCs, BrokenVase, StatusEffects, PlayerExperience, GamblersToken/Alchemy) → casino re-check if cleanup agent landed → Phase 4 closeout (audit re-triage actions + Audit README + 99 tracked-but-ignored rm --cached) → surgical commit (ONLY files in "my footprint" below) → push → morning report.
- My commit footprint (Source): PlayerController_Overlays/Combat/Frontend/Input/ScopedUlt/WorldDialogue/PlayerController.cpp, T66DirectEntry.cpp, T66NiagaraIsolationCaptureCommandlet.cpp, T66Hero1AxeAOEVFXLabActor.cpp, T66CombatComponent.cpp, T66GameMode_Tower.cpp, T66WorldRuntimeProofCommands.cpp, T66PlayerExperienceSubSystem.h/.cpp, T66NPCBase.cpp, T66SaveMigration.h, T66AchievementsSubsystem.cpp, T66StatsPanelSlate.h/.cpp, T66PowerUpScreen.cpp, T66RunStateSubsystem.h (decl move), deleted T66TutorialGate.h/.cpp (+ writer-alignment agent files: T66LeaderboardSubsystem.cpp, HeroSelection populator, possibly RunSummaryScreen.cpp). Scripts: durable/prerelease/lifecycle ps1, SetupAudioSoundClasses.py (new), FixGameInstanceNPCsTableRef.py (new; DELETE after verify), ImportItemSprites.py. Config: DefaultGame.ini (BasicShapes; check for cleanup-agent UE5RFX hunk at commit). Content: Data CSVs/JSON + DT_StatusEffects/DT_CharacterVisuals/DT_WorldVisualProps/DT_NPCs?(no) uassets, Audio SC_* + OSTS re-saves + Hero_7 move (+Alice deletions), Items sprite deletions (8), BP_T66GameInstance. Docs: 5 pending_issues ledgers, STATS_REWORK_SPEC.md, program folder. EXCLUDE: MotionRig/*, Companion materials, Content/Archive deletions, anything else.
- Pending verification (needs staged build + smokes): lab/crate modes, durable gate slot 7, lifecycle mob-loot expectation, SafeZone bubble (BasicShapes cook), content-corrections smoke, cook-warning absence (SC_*, DT_HouseNPCs, BrokenVase, StatusEffects cells), PlayerExperience warning absence, packaged quit exit code observation
- Phase 2 additions since first checkpoint:
  9. BasicShapes cook fix (DefaultGame.ini) — /Engine/BasicShapes loaded by string at runtime were never cooked; staged builds had null spheres → SafeZoneVisualBubblePresent failure + every placeholder-shape visual broken in cooked builds.
  10. Mob-loot shelve-aware gates: stress manifest now reports mob_loot_enabled (T66WorldRuntimeProofCommands), lifecycle gate expectation tracks it (RunLifecycleTransitionSmokeGate.ps1), content-corrections MobLoot check passes-with-note while shelved (T66GameMode_Tower).
  11. PlayerExperience tuning: replaced async preload with synchronous TryLoad in Initialize (race caused "tuning requested before DataTable available" in staged smokes).
  12. BP_T66GameInstance NPCsDataTable repointed DT_HouseNPCs→DT_NPCs via FixGameInstanceNPCsTableRef.py; binary verified clean (cook dependency warning root cause).
  13. WorldVisualProps: removed dead BrokenVase_Easy row (asset purged in v1.2); DT reloaded.
  14. Item_HpRegen/Item_LifeSteal sprites deleted (8 uassets; Mini-inclusive audit found zero references) + ImportItemSprites.py preserve-list updated. Item_Headshot ledger item found already resolved by v1.2.
  15. TutorialGate class fully removed (zero content refs proven; 7 includes + dead interact branch + 2 class files).
  16. Lab unlock save migration: profile SaveVersion 18 remaps the 25 legacy theme-prefixed enemy IDs (T66MigrateLegacyLabEnemyID; successors verified from roster diff 8d3549e81→cdd3f896b).
  17. Casino gambler tab ROOT CAUSE established empirically (captures + widget dump in gambler_diag/): the magenta frame is the legacy master-panel brush rendering the BloodyRetro reference plate (bone-white/blood border baked in PNG, magenta-ish at scale) behind the sparse dialogue page; vendor tab covers it with content. Resolution belongs to the cleanup agent's BloodyRetro preset collapse (retro spec item 8) — re-verify after their pass; residual fallback fix = route gambler root through Friendslop plate.
  18. Smoke matrix SettingsRetroFX anchor: current matrix only lists it as ForbiddenDump (harmless resurrection guard) — Settings/top-bar smoke issues are timing-class; re-verify at gate.
- Multi-agent situation: cleanup agent (Codex) executing retro/archive deletions; hero MotionRig agent actively building Source/T66/Gameplay/MotionRig (21:15-21:28). The FBodyInstance::GetSimplePhysicalMaterial CDO error in commandlet boots comes from in-flight MotionRig constructor code my compiles picked up — NOT mine; judge commandlet runs by script markers until it settles. Commit discipline: add only my files.

## Completed edits (pending compile + verification)

1. StatusEffects.csv — rows 2-12 were missing the DisplayName cell (all values shifted left one column); inserted proper display names. Reimport via SetupCombatRosterDataTables.py in the chain.
2. T66PlayerController_Overlays.cpp — registered lab/laboverlay/crate/crateoverlay as mode-only capture modes (they early-outed in HandleGameplayAutomationPrepare when no screenshot/dump arg); added Warning log when the Lab run-category gate skips the lab overlay.
3. T66DirectEntry.cpp — Warning logs when PetSelection/DailyDescent direct entry is denied by FT66ShelvedFeatureGate (shelved by design; failures now explained). Issue reclassified: works-as-designed.
4. T66NiagaraIsolationCaptureCommandlet.cpp:433 — replaced deprecated FNiagaraEmitterInstance::IsReadyToRun (C4996) with versioned emitter data readiness. NOTE: ledger blamed T66Hero1AxeAOEVFXLabActor.cpp but its three IsReadyToRun calls are all non-deprecated classes (SystemInstance/System/EmitterData); commandlet was the real site. Compile will confirm zero C4996.
5. T66CombatComponent.cpp — removed unreached Bounce ImpactAnchored branch in TrySpawnBoundWeaponBaseSlashVFX (verified: only Pierce flow line ~2947 and Slash flow line ~3234 call it; PerformBounce uses SpawnBounceLinkProjectile).
6. RunDurableSaveIntegritySmokeGate.ps1 + RunPreReleaseSmokeSuite.ps1 — durable gate slot 8→7 (slot-8 collision with RunSessionLoadedTravelSmoke fixture made reload verification read the stale loaded-travel marker).
7. Audio: moved Hero_AliceInWonderlandRabbit/OST.ogg → Heroes/Hero_7/MUS_Hero_7_RabbitChad.ogg (Rabbit Chad=Hero_7, MapTheme=Hero_7), deleted unreachable old uasset; new SetupAudioSoundClasses.py creates /Game/Audio/SC_Music + SC_SFX (warnings-only fix per ledger). MainTheme already exists at /Game/Audio/OSTS/MainTheme (audio build-out) — that part of the ledger entry is already resolved.
8. CaptureT66UIWidget.ps1 GameplayHUD ledger item — already resolved by current state: script now passes caller-supplied -Target; runtime accepts Class=/Tag=/ViewportIndex=/Actor=.

## Background

- Commandlet chain (4 editor boots): CombatRoster DTs → CharacterVisuals DT → SoundClasses → OST import. Logs: logs/<step>.log.
- Investigation agents: all 6 returned. Casino gambler tab = inconclusive (needs empirical capture vs staged build).

## Log

- 23:0x — Preflight done (clean tree @ v1.2, no UE/UBT processes, validator off, usage 34%/7%). Decision block amended (Phase 1 skipped — other agent). Launched 6 investigation agents.
- 23:3x — All quick-fix edits landed (8 items above). Commandlet chain launched in background.
