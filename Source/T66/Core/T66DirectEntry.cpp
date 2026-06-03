// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DirectEntry.h"

#include "Core/T66GameInstance.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Gameplay/T66PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UI/T66UIManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66DirectEntry, Log, All);

namespace
{
	const FName DefaultDirectHeroID(TEXT("Hero_2"));

	FString NormalizeAlias(FString Value)
	{
		Value.TrimStartAndEndInline();
		Value.RemoveFromStart(TEXT("-"));
		Value.ReplaceInline(TEXT(" "), TEXT(""));
		Value.ReplaceInline(TEXT("_"), TEXT(""));
		Value.ReplaceInline(TEXT("-"), TEXT(""));
		return Value.ToLower();
	}

	FString ResolveArgValue(const TArray<FString>& Args, const TCHAR* RequestedKey)
	{
		for (const FString& Arg : Args)
		{
			FString Key;
			FString Value;
			if (Arg.Split(TEXT("="), &Key, &Value) && Key.Equals(RequestedKey, ESearchCase::IgnoreCase))
			{
				return Value.TrimQuotes();
			}
		}
		return FString();
	}

	void ApplyOptionalCommandLineValues(FT66DirectEntryRequest& Request)
	{
		FString HeroName;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero="), HeroName) && !HeroName.IsEmpty())
		{
			Request.HeroID = FName(*HeroName);
		}

		FString CompanionName;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Companion="), CompanionName))
		{
			Request.CompanionID = CompanionName.Equals(TEXT("None"), ESearchCase::IgnoreCase)
				? NAME_None
				: FName(*CompanionName);
		}

		FString DifficultyName;
		ET66Difficulty Difficulty = Request.Difficulty;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Difficulty="), DifficultyName)
			&& T66DirectEntry::TryResolveDifficultyName(DifficultyName, Difficulty))
		{
			Request.Difficulty = Difficulty;
		}

		FString ModalName;
		ET66ScreenType Modal = ET66ScreenType::None;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Modal="), ModalName)
			&& T66DirectEntry::TryResolveFrontendScreenName(ModalName, Modal))
		{
			Request.Modal = Modal;
		}
	}

	bool ParseConsoleEntryArgs(const TArray<FString>& Args, FT66DirectEntryRequest& OutRequest, FString& OutError)
	{
		if (Args.Num() == 0)
		{
			OutError = TEXT("Missing entry name. Usage: T66.Entry Screen:Settings | Run:TestRoom | TestRoom");
			return false;
		}

		if (!T66DirectEntry::TryParseEntryValue(Args[0], OutRequest, OutError))
		{
			return false;
		}

		const FString HeroName = ResolveArgValue(Args, TEXT("Hero"));
		if (!HeroName.IsEmpty())
		{
			OutRequest.HeroID = FName(*HeroName);
		}

		const FString CompanionName = ResolveArgValue(Args, TEXT("Companion"));
		if (!CompanionName.IsEmpty())
		{
			OutRequest.CompanionID = CompanionName.Equals(TEXT("None"), ESearchCase::IgnoreCase)
				? NAME_None
				: FName(*CompanionName);
		}

		const FString DifficultyName = ResolveArgValue(Args, TEXT("Difficulty"));
		ET66Difficulty Difficulty = OutRequest.Difficulty;
		if (!DifficultyName.IsEmpty() && T66DirectEntry::TryResolveDifficultyName(DifficultyName, Difficulty))
		{
			OutRequest.Difficulty = Difficulty;
		}

		const FString ModalName = ResolveArgValue(Args, TEXT("Modal"));
		ET66ScreenType Modal = ET66ScreenType::None;
		if (!ModalName.IsEmpty() && T66DirectEntry::TryResolveFrontendScreenName(ModalName, Modal))
		{
			OutRequest.Modal = Modal;
		}

		return true;
	}

	void ExecuteEntryConsoleCommand(const TArray<FString>& Args, UWorld* World)
	{
		FT66DirectEntryRequest Request;
		FString Error;
		if (!ParseConsoleEntryArgs(Args, Request, Error))
		{
			UE_LOG(LogT66DirectEntry, Warning, TEXT("T66.Entry failed: %s"), *Error);
			return;
		}

		if (!T66DirectEntry::ExecuteRequest(World, Request, Error))
		{
			UE_LOG(LogT66DirectEntry, Warning, TEXT("T66.Entry failed: %s"), *Error);
		}
	}

	void ExecuteScreenConsoleCommand(const TArray<FString>& Args, UWorld* World)
	{
		TArray<FString> EntryArgs = Args;
		if (EntryArgs.Num() > 0 && !EntryArgs[0].Contains(TEXT(":")))
		{
			EntryArgs[0] = FString::Printf(TEXT("Screen:%s"), *EntryArgs[0]);
		}
		ExecuteEntryConsoleCommand(EntryArgs, World);
	}

	void ExecuteRunConsoleCommand(const TArray<FString>& Args, UWorld* World)
	{
		TArray<FString> EntryArgs = Args;
		if (EntryArgs.Num() > 0 && !EntryArgs[0].Contains(TEXT(":")))
		{
			EntryArgs[0] = FString::Printf(TEXT("Run:%s"), *EntryArgs[0]);
		}
		ExecuteEntryConsoleCommand(EntryArgs, World);
	}

	static FAutoConsoleCommandWithWorldAndArgs T66EntryCommand(
		TEXT("T66.Entry"),
		TEXT("Directly opens a frontend screen or gameplay run. Usage: T66.Entry Screen:Settings | Run:TestRoom | TestRoom Hero=Hero_2"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ExecuteEntryConsoleCommand));

	static FAutoConsoleCommandWithWorldAndArgs T66ScreenCommand(
		TEXT("T66.Screen"),
		TEXT("Directly opens a frontend screen. Usage: T66.Screen Settings Modal=ReportBug"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ExecuteScreenConsoleCommand));

	static FAutoConsoleCommandWithWorldAndArgs T66RunCommand(
		TEXT("T66.Run"),
		TEXT("Directly starts a gameplay run category. Usage: T66.Run TestRoom Hero=Hero_2"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ExecuteRunConsoleCommand));
}

FString T66DirectEntry::GetAcceptedFrontendScreenNamesForLog()
{
	return TEXT(
		"MainMenu, HeroSelection, HeroSelect, SaveSlots, SaveSlot, CompanionSelection, CompanionSelect, "
		"GirlfriendSelection, GirlfriendSelect, "
		"PetSelection, PetSelect, Pets, "
		"Settings, SettingsScreen, LanguageSelect, Language, Achievements, PauseMenu, Pause, "
		"ReportBug, RunSummary, PowerUp, HeroGrid, CompanionGrid, GirlfriendGrid, QuitConfirmation, Quit, PartyInvite, "
		"AccountStatus, Account, PlayerSummaryPicker, SummaryPicker, SavePreview, "
		"Challenges, DailyDescent, Overview, History, Relics, Steroids, Diplomas, "
		"Drugs, SteamAchievements, Steam, SettingsRetroFX, RetroFX, SettingsGameplay, SettingsGraphics, "
		"SettingsControls, SettingsMediaViewer, SettingsMedia, SettingsAudio, LoadGame");
}

FString T66DirectEntry::GetAcceptedRunNamesForLog()
{
	return TEXT("Tower, Enter, Gameplay, Lab, Tutorial, TestRoom, Test, ToonStyle");
}

bool T66DirectEntry::TryResolveFrontendScreenName(const FString& ScreenName, ET66ScreenType& OutScreenType)
{
	const FString Key = NormalizeAlias(ScreenName);
	if (Key == TEXT("mainmenu") || Key == TEXT("main"))
	{
		OutScreenType = ET66ScreenType::MainMenu;
		return true;
	}
	if (Key == TEXT("heroselection") || Key == TEXT("heroselect"))
	{
		OutScreenType = ET66ScreenType::HeroSelection;
		return true;
	}
	if (Key == TEXT("saveslots") || Key == TEXT("saveslot") || Key == TEXT("loadgame"))
	{
		OutScreenType = ET66ScreenType::SaveSlots;
		return true;
	}
	if (Key == TEXT("companionselection") || Key == TEXT("companionselect")
		|| Key == TEXT("girlfriendselection") || Key == TEXT("girlfriendselect"))
	{
		OutScreenType = ET66ScreenType::CompanionSelection;
		return true;
	}
	if (Key == TEXT("petselection") || Key == TEXT("petselect") || Key == TEXT("pets"))
	{
		OutScreenType = ET66ScreenType::PetSelection;
		return true;
	}
	if (Key == TEXT("settings") || Key == TEXT("settingsscreen") || Key == TEXT("settingsretrofx")
		|| Key == TEXT("retrofx") || Key == TEXT("settingsgameplay") || Key == TEXT("settingsgraphics")
		|| Key == TEXT("settingscontrols") || Key == TEXT("settingsmediaviewer") || Key == TEXT("settingsmedia")
		|| Key == TEXT("settingsaudio"))
	{
		OutScreenType = ET66ScreenType::Settings;
		return true;
	}
	if (Key == TEXT("languageselect") || Key == TEXT("language"))
	{
		OutScreenType = ET66ScreenType::LanguageSelect;
		return true;
	}
	if (Key == TEXT("achievements") || Key == TEXT("steamachievements") || Key == TEXT("steam"))
	{
		OutScreenType = ET66ScreenType::Achievements;
		return true;
	}
	if (Key == TEXT("pausemenu") || Key == TEXT("pause"))
	{
		OutScreenType = ET66ScreenType::PauseMenu;
		return true;
	}
	if (Key == TEXT("reportbug"))
	{
		OutScreenType = ET66ScreenType::ReportBug;
		return true;
	}
	if (Key == TEXT("runsummary"))
	{
		OutScreenType = ET66ScreenType::RunSummary;
		return true;
	}
	if (Key == TEXT("powerup") || Key == TEXT("relics") || Key == TEXT("steroids")
		|| Key == TEXT("diplomas") || Key == TEXT("drugs"))
	{
		OutScreenType = ET66ScreenType::PowerUp;
		return true;
	}
	if (Key == TEXT("herogrid"))
	{
		OutScreenType = ET66ScreenType::HeroGrid;
		return true;
	}
	if (Key == TEXT("companiongrid") || Key == TEXT("girlfriendgrid"))
	{
		OutScreenType = ET66ScreenType::CompanionGrid;
		return true;
	}
	if (Key == TEXT("quitconfirmation") || Key == TEXT("quit"))
	{
		OutScreenType = ET66ScreenType::QuitConfirmation;
		return true;
	}
	if (Key == TEXT("partyinvite"))
	{
		OutScreenType = ET66ScreenType::PartyInvite;
		return true;
	}
	if (Key == TEXT("accountstatus") || Key == TEXT("account") || Key == TEXT("overview") || Key == TEXT("history"))
	{
		OutScreenType = ET66ScreenType::AccountStatus;
		return true;
	}
	if (Key == TEXT("playersummarypicker") || Key == TEXT("summarypicker"))
	{
		OutScreenType = ET66ScreenType::PlayerSummaryPicker;
		return true;
	}
	if (Key == TEXT("savepreview"))
	{
		OutScreenType = ET66ScreenType::SavePreview;
		return true;
	}
	if (Key == TEXT("challenges"))
	{
		OutScreenType = ET66ScreenType::Challenges;
		return true;
	}
	if (Key == TEXT("dailydescent"))
	{
		if (!FT66ShelvedFeatureGate::IsScreenAllowed(ET66ScreenType::DailyDescent))
		{
			return false;
		}
		OutScreenType = ET66ScreenType::DailyDescent;
		return true;
	}
	return false;
}

bool T66DirectEntry::TryResolveRunCategoryName(const FString& RunName, ET66RunCategory& OutRunCategory)
{
	const FString Key = NormalizeAlias(RunName);
	if (Key == TEXT("tower") || Key == TEXT("enter") || Key == TEXT("gameplay") || Key == TEXT("run"))
	{
		OutRunCategory = ET66RunCategory::Tower;
		return true;
	}
	if (Key == TEXT("lab"))
	{
		OutRunCategory = ET66RunCategory::Lab;
		return true;
	}
	if (Key == TEXT("tutorial"))
	{
		OutRunCategory = ET66RunCategory::Tutorial;
		return true;
	}
	if (Key == TEXT("testroom") || Key == TEXT("test") || Key == TEXT("toonstyle") || Key == TEXT("toonstyletest"))
	{
		OutRunCategory = ET66RunCategory::TestRoom;
		return true;
	}
	return false;
}

bool T66DirectEntry::TryResolveDifficultyName(const FString& DifficultyName, ET66Difficulty& OutDifficulty)
{
	const FString Key = NormalizeAlias(DifficultyName);
	if (Key == TEXT("easy"))
	{
		OutDifficulty = ET66Difficulty::Easy;
		return true;
	}
	if (Key == TEXT("normal"))
	{
		OutDifficulty = ET66Difficulty::Medium;
		return true;
	}
	if (Key == TEXT("medium"))
	{
		OutDifficulty = ET66Difficulty::Medium;
		return true;
	}
	if (Key == TEXT("hard"))
	{
		OutDifficulty = ET66Difficulty::Hard;
		return true;
	}
	if (Key == TEXT("veryhard") || Key == TEXT("expert"))
	{
		OutDifficulty = ET66Difficulty::VeryHard;
		return true;
	}
	if (Key == TEXT("impossible"))
	{
		OutDifficulty = ET66Difficulty::Impossible;
		return true;
	}
	return false;
}

bool T66DirectEntry::TryParseEntryValue(const FString& EntryValue, FT66DirectEntryRequest& OutRequest, FString& OutError)
{
	FString Prefix;
	FString Value;
	if (EntryValue.Split(TEXT(":"), &Prefix, &Value))
	{
		if (Prefix.Equals(TEXT("Screen"), ESearchCase::IgnoreCase) || Prefix.Equals(TEXT("UI"), ESearchCase::IgnoreCase))
		{
			ET66ScreenType Screen = ET66ScreenType::None;
			if (!TryResolveFrontendScreenName(Value, Screen))
			{
				OutError = FString::Printf(TEXT("Unknown screen '%s'. Accepted screens: %s"), *Value, *GetAcceptedFrontendScreenNamesForLog());
				return false;
			}
			OutRequest = FT66DirectEntryRequest{};
			OutRequest.Kind = ET66DirectEntryKind::FrontendScreen;
			OutRequest.Screen = Screen;
			OutRequest.Source = EntryValue;
			return true;
		}

		if (Prefix.Equals(TEXT("Run"), ESearchCase::IgnoreCase) || Prefix.Equals(TEXT("Mode"), ESearchCase::IgnoreCase)
			|| Prefix.Equals(TEXT("Map"), ESearchCase::IgnoreCase))
		{
			ET66RunCategory RunCategory = ET66RunCategory::Tower;
			if (!TryResolveRunCategoryName(Value, RunCategory))
			{
				OutError = FString::Printf(TEXT("Unknown run '%s'. Accepted runs: %s"), *Value, *GetAcceptedRunNamesForLog());
				return false;
			}
			OutRequest = FT66DirectEntryRequest{};
			OutRequest.Kind = ET66DirectEntryKind::GameplayRun;
			OutRequest.RunCategory = RunCategory;
			OutRequest.Source = EntryValue;
			return true;
		}
	}

	ET66RunCategory RunCategory = ET66RunCategory::Tower;
	if (TryResolveRunCategoryName(EntryValue, RunCategory))
	{
		OutRequest = FT66DirectEntryRequest{};
		OutRequest.Kind = ET66DirectEntryKind::GameplayRun;
		OutRequest.RunCategory = RunCategory;
		OutRequest.Source = EntryValue;
		return true;
	}

	ET66ScreenType Screen = ET66ScreenType::None;
	if (TryResolveFrontendScreenName(EntryValue, Screen))
	{
		OutRequest = FT66DirectEntryRequest{};
		OutRequest.Kind = ET66DirectEntryKind::FrontendScreen;
		OutRequest.Screen = Screen;
		OutRequest.Source = EntryValue;
		return true;
	}

	OutError = FString::Printf(
		TEXT("Unknown direct entry '%s'. Use Screen:<name> or Run:<name>. Accepted runs: %s. Accepted screens: %s"),
		*EntryValue,
		*GetAcceptedRunNamesForLog(),
		*GetAcceptedFrontendScreenNamesForLog());
	return false;
}

bool T66DirectEntry::TryParseCommandLine(FT66DirectEntryRequest& OutRequest, FString& OutError)
{
	FString EntryValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66Entry="), EntryValue))
	{
		if (!TryParseEntryValue(EntryValue, OutRequest, OutError))
		{
			return false;
		}
		ApplyOptionalCommandLineValues(OutRequest);
		return true;
	}

	if (FParse::Param(FCommandLine::Get(), TEXT("T66AutomationTestRoom")))
	{
		OutRequest = FT66DirectEntryRequest{};
		OutRequest.Kind = ET66DirectEntryKind::GameplayRun;
		OutRequest.RunCategory = ET66RunCategory::TestRoom;
		OutRequest.HeroID = DefaultDirectHeroID;
		OutRequest.bLeaderboardIneligible = true;
		OutRequest.Source = TEXT("-T66AutomationTestRoom");
		ApplyOptionalCommandLineValues(OutRequest);
		return true;
	}

	return false;
}

void T66DirectEntry::ApplyRequestToGameInstance(UT66GameInstance& GameInstance, const FT66DirectEntryRequest& Request)
{
	if (Request.Kind == ET66DirectEntryKind::FrontendScreen)
	{
		GameInstance.PendingFrontendScreen = Request.Screen;
		GameInstance.PendingDirectEntryModal = Request.Modal;
		GameInstance.ClearPendingDirectGameplayEntry();
		UE_LOG(LogT66DirectEntry, Display, TEXT("Direct entry queued frontend screen %d from '%s'."), static_cast<int32>(Request.Screen), *Request.Source);
		return;
	}

	if (Request.Kind != ET66DirectEntryKind::GameplayRun)
	{
		return;
	}

	GameInstance.SelectedPartySize = ET66PartySize::Solo;
	GameInstance.SelectedHeroID = GameInstance.ResolvePlayableHeroID(Request.HeroID.IsNone() ? DefaultDirectHeroID : Request.HeroID);
	GameInstance.SelectedCompanionID = GameInstance.ResolvePlayableCompanionID(Request.CompanionID);
	GameInstance.SelectedDifficulty = GameInstance.ResolvePlayableDifficulty(Request.Difficulty);
	const ET66RunCategory RequestedRunCategory = Request.RunCategory;
	const ET66RunCategory ResolvedRunCategory = GameInstance.ResolvePlayableRunCategory(RequestedRunCategory);
	GameInstance.SelectedHeroBodyType = ET66BodyType::Chad;
	GameInstance.SelectedCompanionBodyType = ET66BodyType::Chad;
	GameInstance.ClearActiveDailyClimbRun();
	GameInstance.SelectedRunMode = ET66RunMode::Regular;
	GameInstance.SelectedRunCategory = ResolvedRunCategory;
	GameInstance.SelectedRunModifierKind = ET66RunModifierKind::None;
	GameInstance.SelectedRunModifierID = NAME_None;
	GameInstance.bRunIneligibleForLeaderboard = Request.bLeaderboardIneligible;
	GameInstance.bIsNewGameFlow = true;
	GameInstance.bIsStageTransition = false;
	GameInstance.PendingLoadedTransform = FTransform();
	GameInstance.bApplyLoadedTransform = false;
	GameInstance.PendingLoadedRunSnapshot = FT66SavedRunSnapshot{};
	GameInstance.bApplyLoadedRunSnapshot = false;
	GameInstance.RunSeed = FMath::Rand();
	GameInstance.ApplyConfiguredMainMapLayoutVariant();
	GameInstance.MarkPendingDirectGameplayEntry(Request.Source.IsEmpty() ? TEXT("DirectEntry") : Request.Source);
	if (ResolvedRunCategory != RequestedRunCategory)
	{
		UE_LOG(LogT66DirectEntry, Display, TEXT("Direct entry run category %d is unavailable for this release variant; using %d."), static_cast<int32>(RequestedRunCategory), static_cast<int32>(ResolvedRunCategory));
	}

	UE_LOG(
		LogT66DirectEntry,
		Display,
		TEXT("Direct entry configured gameplay run Category=%d Hero=%s Difficulty=%d Source='%s'."),
		static_cast<int32>(ResolvedRunCategory),
		*GameInstance.SelectedHeroID.ToString(),
		static_cast<int32>(GameInstance.SelectedDifficulty),
		*Request.Source);
}

bool T66DirectEntry::ExecuteRequest(UWorld* World, const FT66DirectEntryRequest& Request, FString& OutError)
{
	if (!World)
	{
		OutError = TEXT("No world available.");
		return false;
	}

	UT66GameInstance* GameInstance = Cast<UT66GameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		OutError = TEXT("No T66 game instance available.");
		return false;
	}

	ApplyRequestToGameInstance(*GameInstance, Request);

	if (Request.Kind == ET66DirectEntryKind::FrontendScreen)
	{
		AT66PlayerController* T66PC = Cast<AT66PlayerController>(World->GetFirstPlayerController());
		if (T66PC && T66PC->IsFrontendLevel())
		{
			T66PC->ShowScreen(Request.Screen);
			if (Request.Modal != ET66ScreenType::None)
			{
				if (UT66UIManager* UIManager = T66PC->GetUIManager())
				{
					UIManager->ShowModal(Request.Modal);
				}
			}
		}
		else
		{
			UGameplayStatics::OpenLevel(World, UT66GameInstance::GetFrontendLevelName());
		}
		return true;
	}

	if (Request.Kind == ET66DirectEntryKind::GameplayRun)
	{
		GameInstance->ClearPendingDirectGameplayEntry();
		GameInstance->TransitionToGameplayLevel();
		return true;
	}

	OutError = TEXT("Direct entry request was empty.");
	return false;
}
