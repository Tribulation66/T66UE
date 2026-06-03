Codex implementation draft for Validator cross-review.

Implemented:
- Removed the hero-selection right-column VideoPanel while leaving the central HeroPreviewController/CharacterPreviewHost path intact.
- Added a right-column kit preview panel above stats with Weapon and Ult buttons. Button clicks update selected kit slot, resolve kit display through a temporary adapter over FHeroData plus weapon manager data, and route the selected kit through T66FrontendVideoCatalog::ResolveHeroKitPreview into UT66FrontendVideoPlayer with placeholder fallback behavior.
- Added PetSelection screen and Pet button from HeroSelection. Registered PetSelection routing in T66UITypes, T66PlayerController_Frontend, and T66DirectEntry.
- Added FPetData keyed by source boss ID, GameInstance pet data resolution with fallback synthesis from Bosses data when DT_Pets is absent, selected pet persistence restore, and pet profile save fields appended at the end of the profile save object.
- Added save migration version 17 with default no-pets/no-active-pet state, plus Achievements pet APIs for capture, active pet, skins, and movement-only bond progress.
- Added AT66PetCaptureInteractable for guaranteed capture and AT66PetActor for one-active-pet follow behavior.
- Added pet spawning on player restart, with Mob Loot collection explicitly disabled until Foundation API signatures exist.
- Added boss-flow capture hook after RemainingBossCount reaches zero and after final difficulty-completion branches return. The non-final sequence is TrySpawnPetCaptureForBoss -> SpawnIdolAltarAtLocation -> SpawnStageGateAtLocation. Repeat captures are skipped when Achievements already has the pet.
- Added stage-clear bond progress on stage gate interaction. Bond is used only by pet movement speed multiplier; no collection amount/value/spawn/radius/count code was added.

Relevant files:
- Source/T66/Data/T66DataTypes.h
- Source/T66/Core/T66GameInstance.h/.cpp
- Source/T66/Core/T66ProfileSaveGame.h
- Source/T66/Core/T66SaveMigration.h
- Source/T66/Core/T66AchievementsSubsystem.h/.cpp
- Source/T66/Gameplay/T66PetActor.h/.cpp
- Source/T66/Gameplay/T66PetCaptureInteractable.h/.cpp
- Source/T66/Gameplay/T66GameMode.h
- Source/T66/Gameplay/GameMode/T66GameModePrivate.h
- Source/T66/Gameplay/GameMode/T66GameMode_Spawning.cpp
- Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp
- Source/T66/Gameplay/T66StageGate.cpp
- Source/T66/UI/T66FrontendVideoCatalog.h/.cpp
- Source/T66/UI/T66UITypes.h
- Source/T66/UI/Screens/T66HeroSelectionScreen.h/.cpp
- Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Build.cpp
- Source/T66/UI/Screens/HeroSelection/T66HeroSelectionScreen_Preview.cpp
- Source/T66/UI/Screens/T66PetSelectionScreen.h/.cpp
- Source/T66/Gameplay/T66PlayerController_Frontend.cpp
- Source/T66/Core/T66DirectEntry.cpp

Validation performed:
- rg seam check confirmed pet actor/capture code has no AT66LootBagPickup, loot-bag registry, or loot bag scan references; only disabled Foundation seam comments mention CollectMobLootAt.
- Source check confirmed boss flow order: RemainingBossCount gate, bCompletedSelectedDifficulty final branches return before capture, and non-final hook order TrySpawnPetCaptureForBoss -> SpawnIdolAltarAtLocation -> SpawnStageGateAtLocation.
- Build.bat T66 Win64 Development succeeded.
- StageStandaloneBuild.ps1 -SkipCook succeeded after the final build and refreshed C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe plus the standalone shortcut.
- CaptureT66UIScreen.ps1 produced 1920x1080 staged screenshots:
  - C:\UE\T66\Reports\AgentReviews\20260531_chat4_meta_ui_option2_implementation\screens\hero_selection_final.png
  - C:\UE\T66\Reports\AgentReviews\20260531_chat4_meta_ui_option2_implementation\screens\pet_selection_final.png
- Shortcut verification showed C:\UE\T66\T66 Standalone.lnk targets C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe and target exists.

Known caveats:
- Mob Loot claim/reservation/walk/collect remains disabled behind the Foundation seam by request.
- No gameplay boss-capture smoke was run because there is no scoped automation path in this turn; verification is source/order proof plus staged runtime/UI proof.
- A clean full editor-module rebuild initially exposed an unrelated T66Editor Niagara commandlet linker issue, but the playable T66 target and staged standalone path succeeded.

