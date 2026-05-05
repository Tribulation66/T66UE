// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

using namespace T66HeroSelectionPrivate;

UT66LocalizationSubsystem* UT66HeroSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

void UT66HeroSelectionScreen::OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid()) return;

	int32 Index = DifficultyOptions.IndexOfByKey(NewValue);
	if (Index != INDEX_NONE)
	{
		TArray<ET66Difficulty> Difficulties = {
			ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
			ET66Difficulty::VeryHard, ET66Difficulty::Impossible
		};
		if (Index < Difficulties.Num())
		{
			SelectedDifficulty = Difficulties[Index];
			CurrentDifficultyOption = NewValue;
			CommitLocalSelectionsToLobby(true);
			RefreshDifficultyDropdownText();
			RefreshHeroRecordRank();
			if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
			{
				HeroPreviewController->ApplySelectionDifficultyToPreviewStages(SelectedDifficulty);
			}
		}
	}
}

void UT66HeroSelectionScreen::CommitLocalSelectionsToLobby(bool bResetReady)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->PersistRememberedSelectionDefaults();

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (bResetReady)
			{
				SessionSubsystem->SetLocalLobbyReady(false);
			}
			else
			{
				SessionSubsystem->SyncLocalLobbyProfile();
			}
		}
	}
}

void UT66HeroSelectionScreen::HandlePartyStateChanged()
{
	SyncToSharedPartyScreen();
	FT66Style::DeferRebuild(this);
}

void UT66HeroSelectionScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	FT66Style::DeferRebuild(this);
}

void UT66HeroSelectionScreen::SyncToSharedPartyScreen()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>();
	if (!SessionSubsystem)
	{
		return;
	}

	SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	GI->SelectedDifficulty = SelectedDifficulty;
	RefreshDifficultyDropdownText();
	RefreshHeroRecordRank();

	if (!SessionSubsystem->IsPartyLobbyContextActive() || SessionSubsystem->IsLocalPlayerPartyHost() || !UIManager)
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	if ((DesiredScreen == ET66ScreenType::HeroSelection || DesiredScreen == ET66ScreenType::MainMenu)
		&& UIManager->GetCurrentScreenType() != DesiredScreen)
	{
		UIManager->ShowScreen(DesiredScreen);
	}
}

void UT66HeroSelectionScreen::RefreshDifficultyDropdownText()
{
	if (DifficultyOptions.Num() > 0)
	{
		static const TArray<ET66Difficulty> Difficulties = {
			ET66Difficulty::Easy,
			ET66Difficulty::Medium,
			ET66Difficulty::Hard,
			ET66Difficulty::VeryHard,
			ET66Difficulty::Impossible
		};

		const int32 CurrentDiffIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
		if (DifficultyOptions.IsValidIndex(CurrentDiffIndex))
		{
			CurrentDifficultyOption = DifficultyOptions[CurrentDiffIndex];
		}
	}

	if (DifficultyDropdownText.IsValid())
	{
		UT66LocalizationSubsystem* Loc = GetLocSubsystem();
		DifficultyDropdownText->SetText(
			CurrentDifficultyOption.IsValid()
				? FText::FromString(*CurrentDifficultyOption)
				: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy")));
	}
}

void UT66HeroSelectionScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
			PartyStateChangedHandle.Reset();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
			SessionStateChangedHandle.Reset();
		}

		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
			BackendMyRankReadyHandle.Reset();
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66HeroSelectionScreen::OnScreenActivated_Implementation()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const ET66Language CurrentLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;
	const bool bFirstActivation = !bHasBeenActivated;
	const bool bShouldWarmRebuild = HasBuiltSlateUI() && !bFirstActivation && LastBuiltLanguage != CurrentLanguage;

	if (bFirstActivation)
	{
		Super::OnScreenActivated_Implementation();
	}
	else if (bShouldWarmRebuild)
	{
		FT66Style::DeferRebuild(this);
	}

	if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
	{
		HeroPreviewController->ResetHeroSkinPreviewOverride();
	}
	if (Loc)
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66HeroSelectionScreen::OnLanguageChanged);
	}
	RefreshHeroList();
	bShowingStatsPanel = false;

	if (UT66GameInstance* GIPreload = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GIPreload->PrimeHeroSelectionAssetsAsync();
		GIPreload->PrimeHeroSelectionPreviewVisualsAsync();
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>();
		RefreshCompanionList();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66HeroSelectionScreen::HandlePartyStateChanged);
		}

		if (SessionSubsystem)
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66HeroSelectionScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::HeroSelection);
		}

		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (BackendMyRankReadyHandle.IsValid())
			{
				Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
				BackendMyRankReadyHandle.Reset();
			}
			BackendMyRankReadyHandle = Backend->OnMyRankDataReady.AddUObject(this, &UT66HeroSelectionScreen::HandleBackendMyRankDataReady);
		}

		SelectedDifficulty = SessionSubsystem ? SessionSubsystem->GetSharedLobbyDifficulty() : GI->SelectedDifficulty;
		SelectedBodyType = GI->SelectedHeroBodyType;

		FString RequestedBodyStyle;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroSelectionBody="), RequestedBodyStyle))
		{
			const FString NormalizedBodyStyle = RequestedBodyStyle.TrimStartAndEnd().ToLower();
			if (NormalizedBodyStyle == TEXT("stacy") || NormalizedBodyStyle == TEXT("typeb") || NormalizedBodyStyle == TEXT("b"))
			{
				SelectedBodyType = T66BodyTypeAliases::Stacy;
				GI->SelectedHeroBodyType = SelectedBodyType;
			}
			else if (NormalizedBodyStyle == TEXT("chad") || NormalizedBodyStyle == TEXT("typea") || NormalizedBodyStyle == TEXT("a"))
			{
				SelectedBodyType = T66BodyTypeAliases::Chad;
				GI->SelectedHeroBodyType = SelectedBodyType;
			}
		}

		FString RequestedPreviewHero;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroSelectionPreviewHero="), RequestedPreviewHero))
		{
			const FName RequestedHeroID(*RequestedPreviewHero.TrimStartAndEnd());
			if (AllHeroIDs.Contains(RequestedHeroID))
			{
				PreviewedHeroID = RequestedHeroID;
				GI->SelectedHeroID = RequestedHeroID;
			}
			else
			{
				UE_LOG(LogT66HeroSelection, Warning, TEXT("Hero selection automation: unknown preview hero '%s'"), *RequestedPreviewHero);
			}
		}

		PreviewedCompanionID = GI->SelectedCompanionID;
		TArray<FName> CompanionWheelIDs;
		CompanionWheelIDs.Add(NAME_None);
		CompanionWheelIDs.Append(AllCompanionIDs);
		CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(PreviewedCompanionID);
		if (CurrentCompanionIndex == INDEX_NONE)
		{
			CurrentCompanionIndex = 0;
		}
		bShowingCompanionSkins = false;
		if (!PreviewedHeroID.IsNone() && AllHeroIDs.Contains(PreviewedHeroID))
		{
			PreviewHero(PreviewedHeroID);
		}
		else if (!GI->SelectedHeroID.IsNone() && AllHeroIDs.Contains(GI->SelectedHeroID))
		{
			PreviewHero(GI->SelectedHeroID);
		}
		else if (AllHeroIDs.Num() > 0)
		{
			PreviewHero(AllHeroIDs[0]);
		}
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			if (!PreviewedHeroID.IsNone())
			{
				GI->SelectedHeroSkinID = SkinSub->GetEquippedHeroSkinID(PreviewedHeroID);
				if (GI->SelectedHeroSkinID.IsNone())
				{
					GI->SelectedHeroSkinID = FName(TEXT("Default"));
				}
			}
		}
	}
	if (UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController())
	{
		HeroPreviewController->ApplySelectionDifficultyToPreviewStages(SelectedDifficulty);
	}

	CommitLocalSelectionsToLobby(false);
	SyncToSharedPartyScreen();
}

void UT66HeroSelectionScreen::RefreshScreen_Implementation()
{
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData))
	{
		OnPreviewedHeroChanged(HeroData);
	}
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::RefreshHeroList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllHeroIDs = GI->GetAllHeroIDs();
	}
}

void UT66HeroSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		AllCompanionIDs = GI->GetAllCompanionIDs();
	}
}

void UT66HeroSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
