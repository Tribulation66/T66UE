// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66DailyClimbScreen.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D T66DailyClimbBackgroundImageSize(1920.f, 1080.f);
	const FVector2D T66DailyClimbPanelReferenceSize(1920.f, 1080.f);
	const FVector2D T66DailyClimbLeftShellSize(634.f, 710.f);
	const FVector2D T66DailyClimbCenterStackSize(445.f, 311.f);
	const FVector2D T66DailyClimbPrimaryButtonSize(588.f, 116.f);
	const FVector2D T66DailyClimbSecondaryButtonSize(388.f, 97.f);
	const FVector2D T66DailyClimbCompactButtonSize(180.f, 68.f);

	struct FDailyReferenceBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FDailyPlateBrushSet
	{
		const FSlateBrush* Normal = nullptr;
		const FSlateBrush* Hover = nullptr;
		const FSlateBrush* Pressed = nullptr;
		const FSlateBrush* Disabled = nullptr;
	};

	FLinearColor DailyGoldText()
	{
		return FLinearColor(0.94f, 0.76f, 0.34f, 1.0f);
	}

	FLinearColor DailyBrightText()
	{
		return FLinearColor(0.97f, 0.94f, 0.86f, 1.0f);
	}

	FLinearColor DailyMutedText()
	{
		return FLinearColor(0.73f, 0.68f, 0.58f, 1.0f);
	}

	FLinearColor DailyDividerColor()
	{
		return FLinearColor(0.48f, 0.35f, 0.14f, 0.52f);
	}

	void EnsureDailyClimbRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	const FSlateBrush* ResolveDailyReferenceBrush(
		FDailyReferenceBrushEntry& Entry,
		const TCHAR* RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin = FMargin(0.f),
		const bool bUseBoxDraw = false)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = bUseBoxDraw ? ESlateBrushDrawType::Box : ESlateBrushDrawType::Image;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid() && RelativePath && *RelativePath)
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					bUseBoxDraw,
					TEXT("DailyClimbReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		Entry.Brush->ImageSize = Entry.Texture.IsValid()
			? FVector2D(FMath::Max(1, Entry.Texture->GetSizeX()), FMath::Max(1, Entry.Texture->GetSizeY()))
			: ImageSize;
		return Entry.Brush.Get();
	}

	const FSlateBrush* GetDailyLeftShellBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_panel_tall_shell.png"),
			T66DailyClimbLeftShellSize,
			FMargin(0.05f, 0.08f, 0.05f, 0.08f),
			true);
	}

	const FSlateBrush* GetDailyCenterStackBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_panel_shell.png"),
			T66DailyClimbCenterStackSize,
			FMargin(0.05f, 0.18f, 0.05f, 0.18f),
			true);
	}

	const FSlateBrush* GetDailyRowShellBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_row_shell.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f),
			true);
	}

	const FSlateBrush* GetDailyCompactButtonBrush(const TCHAR* RelativePath)
	{
		static TMap<FString, TSharedPtr<FDailyReferenceBrushEntry>> Entries;
		TSharedPtr<FDailyReferenceBrushEntry>& Entry = Entries.FindOrAdd(RelativePath ? FString(RelativePath) : FString());
		if (!Entry.IsValid())
		{
			Entry = MakeShared<FDailyReferenceBrushEntry>();
		}
		return ResolveDailyReferenceBrush(
			*Entry,
			RelativePath,
			T66DailyClimbCompactButtonSize,
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			true);
	}

	const FSlateBrush* GetDailyCTAButtonBrush(const TCHAR* RelativePath, const FVector2D& ImageSize)
	{
		static TMap<FString, TSharedPtr<FDailyReferenceBrushEntry>> Entries;
		TSharedPtr<FDailyReferenceBrushEntry>& Entry = Entries.FindOrAdd(RelativePath ? FString(RelativePath) : FString());
		if (!Entry.IsValid())
		{
			Entry = MakeShared<FDailyReferenceBrushEntry>();
		}
		return ResolveDailyReferenceBrush(*Entry, RelativePath, ImageSize);
	}

	FDailyPlateBrushSet GetDailyStartButtonBrushes()
	{
		return {
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_primary_normal.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_primary_hover.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_primary_pressed.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_primary_disabled.png"), T66DailyClimbPrimaryButtonSize),
		};
	}

	FDailyPlateBrushSet GetDailyContinueButtonBrushes()
	{
		return {
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_blue_normal.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_blue_hover.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_blue_pressed.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_cta_blue_disabled.png"), T66DailyClimbPrimaryButtonSize),
		};
	}

	FDailyPlateBrushSet GetDailyBackButtonBrushes()
	{
		return {
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_button_neutral_normal.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_button_neutral_hover.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_button_neutral_pressed.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/runflow_button_neutral_disabled.png")),
		};
	}

	class ST66DailyClimbPlateButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DailyClimbPlateButton)
			: _NormalBrush(nullptr)
			, _HoverBrush(nullptr)
			, _PressedBrush(nullptr)
			, _DisabledBrush(nullptr)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
			SLATE_ARGUMENT(const FSlateBrush*, HoverBrush)
			SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
			SLATE_ARGUMENT(const FSlateBrush*, DisabledBrush)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(TAttribute<bool>, IsEnabled)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			NormalBrush = InArgs._NormalBrush;
			HoverBrush = InArgs._HoverBrush;
			PressedBrush = InArgs._PressedBrush;
			DisabledBrush = InArgs._DisabledBrush;
			ContentPadding = InArgs._ContentPadding;

			ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle.SetNormalPadding(FMargin(0.f));
			ButtonStyle.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(this, &ST66DailyClimbPlateButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(this, &ST66DailyClimbPlateButton::GetContentPadding)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&ButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return DisabledBrush ? DisabledBrush : (NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush"));
			}
			if (Button->IsPressed() && PressedBrush)
			{
				return PressedBrush;
			}
			if (Button->IsHovered() && HoverBrush)
			{
				return HoverBrush;
			}
			return NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(ContentPadding.Left, ContentPadding.Top + 1.f, ContentPadding.Right, FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		const FSlateBrush* NormalBrush = nullptr;
		const FSlateBrush* HoverBrush = nullptr;
		const FSlateBrush* PressedBrush = nullptr;
		const FSlateBrush* DisabledBrush = nullptr;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle ButtonStyle;
		TSharedPtr<SButton> Button;
	};

	void SetupDailyClimbRuntimeImageBrush(
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& TextureHandle,
		const TCHAR* AssetPath,
		const TCHAR* StagedRelativePath,
		const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
		}

		EnsureDailyClimbRuntimeImageBrush(Brush, ImageSize);

		if (!TextureHandle.IsValid())
		{
			if (AssetPath && *AssetPath)
			{
				if (UTexture2D* AssetTexture = T66RuntimeUITextureAccess::LoadAssetTexture(
					AssetPath,
					TextureFilter::TF_Trilinear,
					TEXT("DailyClimbRuntimeImage")))
				{
					TextureHandle.Reset(AssetTexture);
				}
			}

			if (!TextureHandle.IsValid() && StagedRelativePath && *StagedRelativePath)
			{
				for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(StagedRelativePath))
				{
					if (!FPaths::FileExists(CandidatePath))
					{
						continue;
					}

					if (UTexture2D* FileTexture = T66RuntimeUITextureAccess::ImportFileTexture(
						CandidatePath,
						TextureFilter::TF_Trilinear,
						false,
						TEXT("DailyClimbRuntimeImage")))
					{
						TextureHandle.Reset(FileTexture);
						break;
					}
				}
			}
		}

		if (!TextureHandle.IsValid())
		{
			Brush->SetResourceObject(nullptr);
			return;
		}

		Brush->SetResourceObject(TextureHandle.Get());
		Brush->ImageSize = FVector2D(
			FMath::Max(1, TextureHandle->GetSizeX()),
			FMath::Max(1, TextureHandle->GetSizeY()));
	}

	FString DifficultyLabel(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return TEXT("Medium");
		case ET66Difficulty::Hard: return TEXT("Hard");
		case ET66Difficulty::VeryHard: return TEXT("Very Hard");
		case ET66Difficulty::Impossible: return TEXT("Impossible");
		default: return TEXT("Easy");
		}
	}

	FDateTime ParseDailySaveTimestamp(const FString& Timestamp)
	{
		FDateTime Parsed = FDateTime::MinValue();
		if (!Timestamp.IsEmpty())
		{
			FDateTime::ParseIso8601(*Timestamp, Parsed);
		}
		return Parsed;
	}
}

UT66DailyClimbScreen::UT66DailyClimbScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::DailyClimb;
	bIsModal = false;
}

void UT66DailyClimbScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	ContinueSaveSlotIndex = INDEX_NONE;
	bStartRequestInFlight = false;
	RefreshContinueAvailability();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnDailyClimbChallengeReady.RemoveAll(this);
			Backend->OnDailyClimbChallengeReady.AddUObject(this, &UT66DailyClimbScreen::HandleDailyClimbStatusReady);
			Backend->FetchCurrentDailyClimb();
		}
	}
}

void UT66DailyClimbScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnDailyClimbChallengeReady.RemoveAll(this);
		}
	}

	bStartRequestInFlight = false;
	ContinueSaveSlotIndex = INDEX_NONE;
	Super::OnScreenDeactivated_Implementation();
}

void UT66DailyClimbScreen::HandleDailyClimbStatusReady(const FString& RequestTag)
{
	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!Backend)
	{
		return;
	}

	if (Backend->HasCachedDailyClimbChallenge() && T66GI)
	{
		T66GI->CacheDailyClimbChallenge(Backend->GetCachedDailyClimbChallenge());
	}

	RefreshContinueAvailability();

	if (RequestTag.Equals(TEXT("start"), ESearchCase::IgnoreCase))
	{
		bStartRequestInFlight = false;
		const FT66DailyClimbChallengeData& Challenge = Backend->GetCachedDailyClimbChallenge();
		if (T66GI && Challenge.IsValid() && Challenge.HasStartedAttempt() && !Challenge.HasCompletedAttempt())
		{
			T66GI->BeginDailyClimbRun(Challenge);
			T66GI->TransitionToGameplayLevel();
			return;
		}
	}

	ForceRebuildSlate();
}

int32 UT66DailyClimbScreen::ComputeSeedQualityPreview(const int32 RunSeed) const
{
	const int32 SeedBase = (RunSeed != 0) ? RunSeed : 1;
	FRandomStream SeedLuckStream(SeedBase ^ 0x4C55434B);
	return FMath::Clamp(SeedLuckStream.RandRange(0, 100), 0, 100);
}

FReply UT66DailyClimbScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66DailyClimbScreen::HandleContinueClicked()
{
	if (ContinueSaveSlotIndex == INDEX_NONE)
	{
		return FReply::Handled();
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !T66GI || !SaveSub)
	{
		return FReply::Handled();
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(ContinueSaveSlotIndex);
	if (!Loaded || !Loaded->bIsDailyClimbRun || !Loaded->DailyClimbChallenge.IsValid() || !Loaded->RunSnapshot.bValid)
	{
		RefreshContinueAvailability();
		ForceRebuildSlate();
		return FReply::Handled();
	}

	if (T66GI->CachedDailyClimbChallenge.IsValid()
		&& !Loaded->DailyClimbChallenge.ChallengeId.Equals(T66GI->CachedDailyClimbChallenge.ChallengeId, ESearchCase::CaseSensitive))
	{
		RefreshContinueAvailability();
		ForceRebuildSlate();
		return FReply::Handled();
	}

	T66GI->SelectedHeroID = Loaded->HeroID;
	T66GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	T66GI->SelectedCompanionID = Loaded->CompanionID;
	T66GI->SelectedDifficulty = Loaded->Difficulty;
	T66GI->SelectedPartySize = ET66PartySize::Solo;
	T66GI->RunSeed = Loaded->RunSeed;
	T66GI->CachedDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->ActiveDailyClimbChallenge = Loaded->DailyClimbChallenge;
	T66GI->bIsDailyClimbRunActive = true;
	T66GI->SelectedRunModifierKind = ET66RunModifierKind::None;
	T66GI->SelectedRunModifierID = NAME_None;
	T66GI->CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
	T66GI->PendingLoadedTransform = Loaded->PlayerTransform;
	T66GI->bApplyLoadedTransform = true;
	T66GI->PendingLoadedRunSnapshot = Loaded->RunSnapshot;
	T66GI->bApplyLoadedRunSnapshot = Loaded->RunSnapshot.bValid;
	T66GI->CurrentSaveSlotIndex = ContinueSaveSlotIndex;
	T66GI->bRunIneligibleForLeaderboard = Loaded->bRunIneligibleForLeaderboard;
	T66GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	T66GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	T66GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	T66GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;
	T66GI->bSaveSlotPreviewMode = false;

	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->RestoreActiveRunContext(Loaded->IntegrityContext);
		Integrity->MarkLoadedSnapshot();
		T66GI->bRunIneligibleForLeaderboard = T66GI->bRunIneligibleForLeaderboard || !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
	}

	T66GI->PersistRememberedSelectionDefaults();
	T66GI->TransitionToGameplayLevel();
	return FReply::Handled();
}

FReply UT66DailyClimbScreen::HandleStartClicked()
{
	if (bStartRequestInFlight)
	{
		return FReply::Handled();
	}

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	const FT66DailyClimbChallengeData* Challenge = nullptr;
	if (Backend && Backend->HasCachedDailyClimbChallenge())
	{
		Challenge = &Backend->GetCachedDailyClimbChallenge();
	}
	else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
	{
		Challenge = &T66GI->CachedDailyClimbChallenge;
	}

	if (!Backend || !Challenge || !Challenge->IsValid() || Challenge->HasStartedAttempt() || Challenge->HasCompletedAttempt())
	{
		return FReply::Handled();
	}

	bStartRequestInFlight = true;
	Backend->StartDailyClimbAttempt();
	return FReply::Handled();
}

void UT66DailyClimbScreen::RefreshContinueAvailability()
{
	ContinueSaveSlotIndex = INDEX_NONE;

	UGameInstance* GI = GetGameInstance();
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!SaveSub)
	{
		return;
	}

	FString ActiveChallengeId;
	if (Backend && Backend->HasCachedDailyClimbChallenge())
	{
		ActiveChallengeId = Backend->GetCachedDailyClimbChallenge().ChallengeId;
	}
	else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
	{
		ActiveChallengeId = T66GI->CachedDailyClimbChallenge.ChallengeId;
	}

	FDateTime BestTimestamp = FDateTime::MinValue();
	for (int32 SlotIndex = 0; SlotIndex < UT66SaveSubsystem::MaxSlots; ++SlotIndex)
	{
		if (!SaveSub->DoesSlotExist(SlotIndex))
		{
			continue;
		}

		UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
		if (!Loaded
			|| !Loaded->bIsDailyClimbRun
			|| !Loaded->DailyClimbChallenge.IsValid()
			|| Loaded->DailyClimbChallenge.HasCompletedAttempt()
			|| !Loaded->RunSnapshot.bValid)
		{
			continue;
		}

		if (!ActiveChallengeId.IsEmpty()
			&& !Loaded->DailyClimbChallenge.ChallengeId.Equals(ActiveChallengeId, ESearchCase::CaseSensitive))
		{
			continue;
		}

		const FDateTime SaveTimestamp = ParseDailySaveTimestamp(Loaded->LastPlayedUtc);
		if (ContinueSaveSlotIndex == INDEX_NONE || SaveTimestamp >= BestTimestamp)
		{
			ContinueSaveSlotIndex = SlotIndex;
			BestTimestamp = SaveTimestamp;
		}
	}
}

void UT66DailyClimbScreen::RequestBackgroundTexture()
{
	SetupDailyClimbRuntimeImageBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		nullptr,
		TEXT("SourceAssets/UI/RunFlowReference/Backgrounds/daily_climb_plate_1920x1080.png"),
		T66DailyClimbBackgroundImageSize);
}

TSharedRef<SWidget> UT66DailyClimbScreen::BuildSlateUI()
{
	RequestBackgroundTexture();

	UGameInstance* GI = GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	const FT66DailyClimbChallengeData* Challenge = nullptr;
	if (Backend && Backend->HasCachedDailyClimbChallenge())
	{
		Challenge = &Backend->GetCachedDailyClimbChallenge();
	}
	else if (T66GI && T66GI->CachedDailyClimbChallenge.IsValid())
	{
		Challenge = &T66GI->CachedDailyClimbChallenge;
	}

	const bool bHasChallenge = Challenge && Challenge->IsValid();
	const bool bAttemptStarted = bHasChallenge && Challenge->HasStartedAttempt();
	const bool bAttemptConsumed = bHasChallenge && Challenge->HasCompletedAttempt();
	const bool bCanContinueChallenge = ContinueSaveSlotIndex != INDEX_NONE;
	const bool bCanStartChallenge = bHasChallenge && !bAttemptStarted && !bAttemptConsumed && !bStartRequestInFlight;
	const int32 SeedPreview = bHasChallenge ? ComputeSeedQualityPreview(Challenge->RunSeed) : 0;

	FString HeroName = TEXT("Unknown Hero");
	if (bHasChallenge && T66GI)
	{
		FHeroData HeroData;
		if (T66GI->GetHeroData(Challenge->HeroID, HeroData))
		{
			HeroName = HeroData.DisplayName.ToString();
		}
		else
		{
			HeroName = Challenge->HeroID.ToString();
		}
	}

	const FText StatusText =
		!Backend ? NSLOCTEXT("T66.DailyClimb", "StatusOffline", "Backend unavailable.") :
		(!bHasChallenge
			? FText::FromString(Backend->GetLastDailyClimbMessage().IsEmpty() ? TEXT("Loading Daily Challenge...") : Backend->GetLastDailyClimbMessage())
			: (bCanContinueChallenge
				? NSLOCTEXT("T66.DailyClimb", "ResumeReady", "Your Daily Challenge run is in progress. Continue to resume it.")
				: (bAttemptConsumed
					? NSLOCTEXT("T66.DailyClimb", "AttemptUsed", "Today's Daily Challenge is already completed.")
					: (bAttemptStarted
						? NSLOCTEXT("T66.DailyClimb", "AttemptStarted", "Your Daily attempt is already active. Resume from your saved Daily run.")
						: NSLOCTEXT("T66.DailyClimb", "Ready", "Everyone gets the same seed, hero, and rules. One attempt only.")))));

	const FText StartButtonText =
		bAttemptConsumed
			? NSLOCTEXT("T66.DailyClimb", "AttemptUsedButton", "ATTEMPT USED")
			: (bAttemptStarted
				? NSLOCTEXT("T66.DailyClimb", "AttemptLockedButton", "RUN IN PROGRESS")
				: (bStartRequestInFlight
					? NSLOCTEXT("T66.DailyClimb", "Starting", "STARTING...")
					: NSLOCTEXT("T66.DailyClimb", "StartChallenge", "START CHALLENGE")));

	const FText ContinueButtonText = NSLOCTEXT("T66.DailyClimb", "ContinueChallenge", "CONTINUE CHALLENGE");

	TSharedRef<SVerticalBox> RulesBox = SNew(SVerticalBox);
	if (bHasChallenge && Challenge->Rules.Num() > 0)
	{
		for (const FT66DailyClimbRule& Rule : Challenge->Rules)
		{
			RulesBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBorder)
				.BorderImage(GetDailyRowShellBrush())
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(16.f, 12.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(Rule.Label))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(DailyBrightText())
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Rule.Description))
						.Font(FT66Style::Tokens::FontRegular(15))
						.ColorAndOpacity(DailyMutedText())
						.AutoWrapText(true)
					]
				]
			];
		}
	}
	else
	{
		RulesBox->AddSlot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(GetDailyRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(FMargin(16.f, 12.f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.DailyClimb", "NoRules", "No special rules are published for this Daily yet."))
				.Font(FT66Style::Tokens::FontRegular(15))
				.ColorAndOpacity(DailyMutedText())
				.AutoWrapText(true)
			]
		];
	}

	auto MakeInfoCard = [](const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(GetDailyRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(FMargin(14.f, 10.f, 14.f, 12.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontRegular(13))
					.ColorAndOpacity(DailyMutedText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Value)
					.Font(FT66Style::Tokens::FontBold(19))
					.ColorAndOpacity(DailyBrightText())
					.AutoWrapText(true)
				]
			];
	};

	auto MakeMenuButton = [this](
		const FText& Text,
		FReply (UT66DailyClimbScreen::*ClickFunc)(),
		const bool bEnabled,
		const FDailyPlateBrushSet& BrushSet,
		const FVector2D& ButtonSize,
		const int32 FontSize) -> TSharedRef<SWidget>
	{
		FSlateFontInfo ButtonFont = FT66Style::MakeFont(TEXT("Bold"), FontSize);
		ButtonFont.LetterSpacing = 0;

		return SNew(SBox)
			.WidthOverride(ButtonSize.X)
			.HeightOverride(ButtonSize.Y)
			[
				SNew(ST66DailyClimbPlateButton)
				.NormalBrush(BrushSet.Normal)
				.HoverBrush(BrushSet.Hover)
				.PressedBrush(BrushSet.Pressed)
				.DisabledBrush(BrushSet.Disabled)
				.IsEnabled(bEnabled)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ContentPadding(FMargin(12.f, 2.f, 12.f, 0.f))
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(ButtonFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(bEnabled ? DailyBrightText() : FLinearColor(0.74f, 0.70f, 0.62f, 0.76f))
					.ShadowOffset(FVector2D(0.f, 1.f))
					.ShadowColorAndOpacity(FLinearColor(0.10f, 0.07f, 0.03f, 0.95f))
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
			];
	};

	const FDailyPlateBrushSet StartButtonBrushes = GetDailyStartButtonBrushes();
	const FDailyPlateBrushSet ContinueButtonBrushes = GetDailyContinueButtonBrushes();

	const TSharedRef<SWidget> PrimaryActionWidget = bCanContinueChallenge
		? MakeMenuButton(
			ContinueButtonText,
			&UT66DailyClimbScreen::HandleContinueClicked,
			true,
			ContinueButtonBrushes,
			T66DailyClimbPrimaryButtonSize,
			20)
		: MakeMenuButton(
			StartButtonText,
			&UT66DailyClimbScreen::HandleStartClicked,
			bCanStartChallenge,
			StartButtonBrushes,
			T66DailyClimbPrimaryButtonSize,
			24);

	const TSharedRef<SWidget> RulesPanel =
		SNew(SBox)
		.WidthOverride(T66DailyClimbLeftShellSize.X)
		.HeightOverride(T66DailyClimbLeftShellSize.Y)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			SNew(SOverlay)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(GetDailyLeftShellBrush())
			]
			+ SOverlay::Slot()
			.Padding(FMargin(32.f, 34.f, 32.f, 30.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "RulesPanelTitle", "TODAY'S RULES"))
					.Font(FT66Style::Tokens::FontHeading())
					.ColorAndOpacity(DailyGoldText())
					.ShadowOffset(FVector2D(0.f, 1.f))
					.ShadowColorAndOpacity(FLinearColor(0.04f, 0.025f, 0.01f, 1.f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 10.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(DailyDividerColor())
					.Padding(FMargin(0.f, 1.f))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(GetDailyRowShellBrush())
					.BorderBackgroundColor(FLinearColor::White)
					.Padding(FMargin(16.f, 12.f))
					[
						SNew(STextBlock)
						.Text(StatusText)
						.Font(FT66Style::Tokens::FontRegular(15))
						.ColorAndOpacity(DailyBrightText())
						.AutoWrapText(true)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.f, 0.f, 7.f, 0.f)
					[
						MakeInfoCard(
							NSLOCTEXT("T66.DailyClimb", "HeroLabel", "Hero"),
							FText::FromString(bHasChallenge ? HeroName : TEXT("--")))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						MakeInfoCard(
							NSLOCTEXT("T66.DailyClimb", "DifficultyLabel", "Difficulty"),
							FText::FromString(bHasChallenge ? DifficultyLabel(Challenge->Difficulty) : TEXT("--")))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.f, 0.f, 7.f, 0.f)
					[
						MakeInfoCard(
							NSLOCTEXT("T66.DailyClimb", "SeedQualityLabel", "Seed Quality"),
							FText::FromString(bHasChallenge ? FString::Printf(TEXT("%d / 100"), SeedPreview) : TEXT("--")))
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						MakeInfoCard(
							NSLOCTEXT("T66.DailyClimb", "RewardLabel", "Reward"),
							FText::FromString(bHasChallenge ? FString::Printf(TEXT("%d Chad Coupons"), Challenge->CouponReward) : TEXT("--")))
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "RulesHeader", "Rule Stack"))
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(DailyBrightText())
				]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					SNew(SScrollBox)
					.ScrollBarVisibility(EVisibility::Collapsed)
					+ SScrollBox::Slot()
					[
						RulesBox
					]
				]
			]
		];

	const TSharedRef<SWidget> Canvas = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0.f)
		[
			SNew(SBox)
			.WidthOverride(T66DailyClimbPanelReferenceSize.X)
			.HeightOverride(T66DailyClimbPanelReferenceSize.Y)
			[
				SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(SkyBackgroundBrush.Get())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.28f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.f, 136.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(920.f)
				.HeightOverride(132.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.DailyClimb", "Title", "DAILY CHALLENGE"))
						.Font(FT66Style::MakeFont(TEXT("Black"), 62))
						.ColorAndOpacity(DailyGoldText())
						.ShadowOffset(FVector2D(0.f, 2.f))
						.ShadowColorAndOpacity(FLinearColor(0.08f, 0.04f, 0.01f, 1.f))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.DailyClimb", "Subtitle", "One seed. One attempt. Same puzzle for everyone."))
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(DailyBrightText())
						.ShadowOffset(FVector2D(0.f, 1.f))
						.ShadowColorAndOpacity(FLinearColor(0.06f, 0.035f, 0.01f, 1.f))
						.Justification(ETextJustify::Center)
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(33.f, 320.f, 0.f, 0.f))
			[
				RulesPanel
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(1288.f, 904.f, 0.f, 0.f))
			[
				PrimaryActionWidget
			]
			]
		];

	return Canvas;
}
