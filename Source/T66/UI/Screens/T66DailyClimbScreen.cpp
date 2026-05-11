// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66DailyClimbScreen.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "UI/Components/T66LeaderboardPanel.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66ReferenceLayout.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
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
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D T66DailyClimbPanelReferenceSize(1920.f, 1080.f);
	const FVector2D T66DailyClimbLeftShellSize(T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Width, T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly.Height);
	const FVector2D T66DailyClimbRightShellSize(T66MainMenuReferenceLayout::Right::ShellFullReference.Width, T66MainMenuReferenceLayout::Right::ShellFullReference.Height);
	const FVector2D T66DailyClimbPrimaryButtonSize(460.f, 92.f);
	const FVector2D T66DailyClimbCompactButtonSize(136.f, 72.f);
	const FString DailyUltrakillElementDir = TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements");
	const FString DailyUltrakillSquareElementDir = DailyUltrakillElementDir / TEXT("SquareVariant");

	FString MakeDailySquareElementPath(const TCHAR* FileName)
	{
		return DailyUltrakillSquareElementDir / FString(FileName ? FileName : TEXT(""));
	}

	struct FDailyReferenceBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
		bool bSimpleFallback = false;
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
		const bool bUseBoxDraw = false,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		const FString SourceRelativePath(RelativePath ? RelativePath : TEXT(""));
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = bUseBoxDraw ? ESlateBrushDrawType::Box : ESlateBrushDrawType::Image;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid() && !Entry.bSimpleFallback && !SourceRelativePath.IsEmpty())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					bUseBoxDraw,
					TEXT("DailyClimbReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		if (Entry.Texture.IsValid())
		{
			Entry.bSimpleFallback = false;
			Entry.Brush->SetResourceObject(Entry.Texture.Get());
			Entry.Brush->ImageSize = FVector2D(FMath::Max(1, Entry.Texture->GetSizeX()), FMath::Max(1, Entry.Texture->GetSizeY()));
			return Entry.Brush.Get();
		}

		if (T66RuntimeUIBrushAccess::ShouldUseSimpleReferenceFallback(SourceRelativePath))
		{
			Entry.bSimpleFallback = true;
			T66RuntimeUIBrushAccess::ConfigureSimpleReferenceFallbackBrush(
				*Entry.Brush,
				SourceRelativePath,
				ImageSize,
				Margin,
				bUseBoxDraw ? ESlateBrushDrawType::Box : ESlateBrushDrawType::Image);
			return Entry.Brush.Get();
		}

		Entry.bSimpleFallback = false;
		Entry.Brush->SetResourceObject(nullptr);
		Entry.Brush->ImageSize = ImageSize;
		return Entry.Brush.Get();
	}

	const FSlateBrush* GetDailyLeftShellBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			*MakeDailySquareElementPath(TEXT("main_panel_normal_square_variant.png")),
			T66DailyClimbLeftShellSize,
			FMargin(0.070f, 0.045f, 0.070f, 0.045f),
			true,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetDailyRightShellBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			*MakeDailySquareElementPath(TEXT("main_panel_normal_square_variant.png")),
			T66DailyClimbRightShellSize,
			FMargin(0.070f, 0.045f, 0.070f, 0.045f),
			true,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetDailyLeaderboardFilterButtonBrush(const bool bSelected)
	{
		static FDailyReferenceBrushEntry NormalEntry;
		static FDailyReferenceBrushEntry SelectedEntry;
		return ResolveDailyReferenceBrush(
			bSelected ? SelectedEntry : NormalEntry,
			*MakeDailySquareElementPath(TEXT("cta_new_game_button_normal_red_square_variant.png")),
			T66DailyClimbCompactButtonSize,
			FMargin(0.f),
			false,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetDailyLeaderboardFilterIconBrush(const ET66LeaderboardFilter Filter)
	{
		static FDailyReferenceBrushEntry GlobalEntry;
		static FDailyReferenceBrushEntry FriendsEntry;
		static FDailyReferenceBrushEntry StreamersEntry;
		switch (Filter)
		{
		case ET66LeaderboardFilter::Friends:
			return ResolveDailyReferenceBrush(FriendsEntry, *(DailyUltrakillElementDir / TEXT("leaderboard_filter_friends_icon.png")), FVector2D(58.f, 58.f), FMargin(0.f), false, TextureFilter::TF_Nearest);
		case ET66LeaderboardFilter::Streamers:
			return ResolveDailyReferenceBrush(StreamersEntry, *(DailyUltrakillElementDir / TEXT("leaderboard_filter_streamers_icon.png")), FVector2D(58.f, 58.f), FMargin(0.f), false, TextureFilter::TF_Nearest);
		case ET66LeaderboardFilter::Global:
		default:
			return ResolveDailyReferenceBrush(GlobalEntry, *(DailyUltrakillElementDir / TEXT("leaderboard_filter_global_icon.png")), FVector2D(58.f, 58.f), FMargin(0.f), false, TextureFilter::TF_Nearest);
		}
	}

	const FSlateBrush* GetDailyRowShellBrush()
	{
		static FDailyReferenceBrushEntry Entry;
		return ResolveDailyReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/leaderboard_row_normal.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			true,
			TextureFilter::TF_Nearest);
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
			FMargin(0.f),
			false,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetDailyCTAButtonBrush(const TCHAR* RelativePath, const FVector2D& ImageSize)
	{
		static TMap<FString, TSharedPtr<FDailyReferenceBrushEntry>> Entries;
		TSharedPtr<FDailyReferenceBrushEntry>& Entry = Entries.FindOrAdd(RelativePath ? FString(RelativePath) : FString());
		if (!Entry.IsValid())
		{
			Entry = MakeShared<FDailyReferenceBrushEntry>();
		}
		return ResolveDailyReferenceBrush(*Entry, RelativePath, ImageSize, FMargin(0.f), false, TextureFilter::TF_Nearest);
	}

	FDailyPlateBrushSet GetDailyStartButtonBrushes()
	{
		return {
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_hover_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_pressed_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_disabled_square_variant.png"), T66DailyClimbPrimaryButtonSize),
		};
	}

	FDailyPlateBrushSet GetDailyContinueButtonBrushes()
	{
		return {
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_hover_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_pressed_square_variant.png"), T66DailyClimbPrimaryButtonSize),
			GetDailyCTAButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_disabled_square_variant.png"), T66DailyClimbPrimaryButtonSize),
		};
	}

	FDailyPlateBrushSet GetDailyBackButtonBrushes()
	{
		return {
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_normal_red_square_variant.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_hover_red_square_variant.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_pressed_red_square_variant.png")),
			GetDailyCompactButtonBrush(TEXT("SourceAssets/UI/Reference/Screens/MainMenu/Ultrakill/Elements/SquareVariant/cta_new_game_button_disabled_red_square_variant.png")),
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
							T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(
								TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateSP(this, &ST66DailyClimbPlateButton::GetCurrentBrush)),
								FVector2D(1.0f, 1.0f),
								0.105f)
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
	ScreenType = ET66ScreenType::DailyDescent;
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

	T66GI->SelectedHeroID = T66GI->ResolvePlayableHeroID(Loaded->HeroID);
	T66GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	T66GI->SelectedCompanionID = Loaded->CompanionID;
	T66GI->SelectedDifficulty = T66GI->ResolvePlayableDifficulty(Loaded->Difficulty);
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

TSharedRef<SWidget> UT66DailyClimbScreen::BuildSlateUI()
{
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
			? FText::FromString(Backend->GetLastDailyClimbMessage().IsEmpty() ? TEXT("Loading Daily Descent...") : Backend->GetLastDailyClimbMessage())
			: (bCanContinueChallenge
				? NSLOCTEXT("T66.DailyClimb", "ResumeReady", "Your Daily Descent run is in progress. Continue to resume it.")
				: (bAttemptConsumed
					? NSLOCTEXT("T66.DailyClimb", "AttemptUsed", "Today's Daily Descent is already completed.")
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
					: NSLOCTEXT("T66.DailyClimb", "StartDescent", "START DESCENT")));

	const FText ContinueButtonText = NSLOCTEXT("T66.DailyClimb", "ContinueDescent", "CONTINUE DESCENT");

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
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.BorderBackgroundColor(FLinearColor::Transparent)
				.Padding(FMargin(14.f, 6.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(150.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Rule.Label))
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(DailyBrightText())
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 8.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("-")))
						.Font(FT66Style::Tokens::FontBold(13))
						.ColorAndOpacity(DailyMutedText())
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Rule.Description))
						.Font(FT66Style::Tokens::FontRegular(13))
						.ColorAndOpacity(DailyBrightText())
						.AutoWrapText(true)
						.WrapTextAt(430.f)
						.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						.Clipping(EWidgetClipping::ClipToBounds)
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
			.Padding(FMargin(14.f, 8.f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.DailyClimb", "NoRules", "No special rules are published for this Daily yet."))
				.Font(FT66Style::Tokens::FontRegular(13))
				.ColorAndOpacity(DailyMutedText())
				.AutoWrapText(true)
				.WrapTextAt(588.f)
				.Clipping(EWidgetClipping::ClipToBounds)
			]
		];
	}

	auto MakeInfoCard = [](const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(GetDailyRowShellBrush())
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(FMargin(12.f, 7.f, 12.f, 8.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontRegular(12))
					.ColorAndOpacity(DailyMutedText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Value)
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(DailyBrightText())
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
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
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.StretchDirection(EStretchDirection::DownOnly)
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
				]
			];
	};

	const FDailyPlateBrushSet StartButtonBrushes = GetDailyStartButtonBrushes();
	const FDailyPlateBrushSet ContinueButtonBrushes = GetDailyContinueButtonBrushes();
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");

	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LeaderboardSubsystem = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66RunSaveGame* ContinuePreviewSave = (bCanContinueChallenge && SaveSub) ? SaveSub->LoadFromSlot(ContinueSaveSlotIndex) : nullptr;

	FString CompanionName = TEXT("None");
	const FName DisplayCompanionID = ContinuePreviewSave ? ContinuePreviewSave->CompanionID : NAME_None;
	if (T66GI && !DisplayCompanionID.IsNone())
	{
		FCompanionData CompanionData;
		CompanionName = T66GI->GetCompanionData(DisplayCompanionID, CompanionData)
			? CompanionData.DisplayName.ToString()
			: DisplayCompanionID.ToString();
	}

	const FText ContinueSaveText = ContinuePreviewSave
		? FText::Format(
			NSLOCTEXT("T66.DailyClimb", "ContinueSaveHint", "Saved Slot {0} / Stage {1}"),
			FText::AsNumber(ContinueSaveSlotIndex + 1),
			FText::AsNumber(FMath::Max(1, ContinuePreviewSave->StageReached)))
		: NSLOCTEXT("T66.DailyClimb", "NoContinueSaveHint", "No saved Daily run");

	const TSharedRef<SWidget> ContinueActionWidget =
		MakeMenuButton(
			ContinueButtonText,
			&UT66DailyClimbScreen::HandleContinueClicked,
			bCanContinueChallenge,
			ContinueButtonBrushes,
			T66DailyClimbPrimaryButtonSize,
			23);

	const TSharedRef<SWidget> StartActionWidget =
		MakeMenuButton(
			StartButtonText,
			&UT66DailyClimbScreen::HandleStartClicked,
			bCanStartChallenge,
			StartButtonBrushes,
			T66DailyClimbPrimaryButtonSize,
			23);

	auto MakeDetailRow = [MakeInfoCard](const FText& Label, const FText& Value) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				MakeInfoCard(Label, Value)
			];
	};

	const TSharedRef<SWidget> LeftDetailsPanel =
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
			.Padding(FMargin(20.f, 36.f, 20.f, 22.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "RulesPanelTitle", "RULES OF THE DAY"))
					.Font(FT66Style::MakeFont(TEXT("Black"), 34))
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
					.Padding(FMargin(14.f, 10.f))
					[
						SNew(STextBlock)
						.Text(StatusText)
						.Font(FT66Style::Tokens::FontRegular(12))
						.ColorAndOpacity(DailyBrightText())
						.AutoWrapText(true)
						.WrapTextAt(382.f)
						.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 9.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDetailRow(
							NSLOCTEXT("T66.DailyClimb", "HeroSelectedLabel", "Hero Selected"),
							FText::FromString(bHasChallenge ? HeroName : TEXT("--")))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDetailRow(
							NSLOCTEXT("T66.DailyClimb", "CompanionSelectedLabel", "Companion Selected"),
							FText::FromString(CompanionName))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDetailRow(
							NSLOCTEXT("T66.DailyClimb", "DifficultyLabel", "Difficulty"),
							FText::FromString(bHasChallenge ? DifficultyLabel(Challenge->Difficulty) : TEXT("--")))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeDetailRow(
							NSLOCTEXT("T66.DailyClimb", "ContinueSlotLabel", "Continue Save"),
							ContinueSaveText)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 5.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.DailyClimb", "ModifiersHeader", "Modifiers"))
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

	TSharedRef<SWidget> LeaderboardWidget =
		SAssignNew(LeaderboardPanel, ST66LeaderboardPanel)
		.LocalizationSubsystem(Loc)
		.LeaderboardSubsystem(LeaderboardSubsystem)
		.UIManager(UIManager)
		.DailyChallengeMode(true)
		.ReferenceMirrorMode(true);
	const TWeakPtr<ST66LeaderboardPanel> WeakLeaderboardPanel = LeaderboardPanel;
	const TSharedRef<TArray<TPair<ET66LeaderboardFilter, TWeakPtr<SImage>>>> LeaderboardFilterChromeImages =
		MakeShared<TArray<TPair<ET66LeaderboardFilter, TWeakPtr<SImage>>>>();
	auto GetLeaderboardFilterChromeBrush = [WeakLeaderboardPanel](const ET66LeaderboardFilter Filter) -> const FSlateBrush*
	{
		if (const TSharedPtr<ST66LeaderboardPanel> Panel = WeakLeaderboardPanel.Pin())
		{
			return GetDailyLeaderboardFilterButtonBrush(Panel->GetFilter() == Filter);
		}
		return GetDailyLeaderboardFilterButtonBrush(false);
	};
	auto RefreshLeaderboardFilterChrome = [LeaderboardFilterChromeImages, GetLeaderboardFilterChromeBrush]()
	{
		for (const TPair<ET66LeaderboardFilter, TWeakPtr<SImage>>& Entry : *LeaderboardFilterChromeImages)
		{
			if (const TSharedPtr<SImage> ButtonChromeImage = Entry.Value.Pin())
			{
				ButtonChromeImage->SetImage(GetLeaderboardFilterChromeBrush(Entry.Key));
			}
		}
	};
	auto MakeLeaderboardFilterButton = [WeakLeaderboardPanel, &NoBorderButtonStyle, LeaderboardFilterChromeImages, GetLeaderboardFilterChromeBrush, RefreshLeaderboardFilterChrome](
		const ET66LeaderboardFilter Filter) -> TSharedRef<SWidget>
	{
		TSharedPtr<SImage> ButtonChromeImage;
		TSharedRef<SWidget> ButtonWidget = SNew(SBox)
			.WidthOverride(T66MainMenuReferenceLayout::Right::FilterWorldButton.Width)
			.HeightOverride(T66MainMenuReferenceLayout::Right::FilterWorldButton.Height)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						FOnClicked::CreateLambda([WeakLeaderboardPanel, Filter, RefreshLeaderboardFilterChrome]()
						{
							if (const TSharedPtr<ST66LeaderboardPanel> Panel = WeakLeaderboardPanel.Pin())
							{
								Panel->SetFilter(Filter);
								Panel->Invalidate(EInvalidateWidgetReason::Layout);
							}
							RefreshLeaderboardFilterChrome();
							return FReply::Handled();
						}),
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SAssignNew(ButtonChromeImage, SImage)
							.Image(GetLeaderboardFilterChromeBrush(Filter))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(58.f)
							.HeightOverride(58.f)
							[
								FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
									SNew(SImage)
									.Image(GetDailyLeaderboardFilterIconBrush(Filter))
									.ColorAndOpacity(FLinearColor::White)))
							]
						])
					.SetButtonStyle(&NoBorderButtonStyle)
					.SetPadding(FMargin(0.f)))
			];
		LeaderboardFilterChromeImages->Emplace(Filter, ButtonChromeImage);
		return ButtonWidget;
	};

	const FT66ReferenceRect& RightPanelAssemblyRect = T66MainMenuReferenceLayout::MainMenu::RightPanelAssembly;
	const FT66ReferenceRect& RightShellRect = T66MainMenuReferenceLayout::Right::ShellFullReference;
	const float RightLeaderboardFrameInsetX = 18.f;
	const float RightLeaderboardFrameInsetY = 18.f;
	const FVector2D RightShellOffset(
		RightShellRect.X - RightPanelAssemblyRect.X,
		RightShellRect.Y - RightPanelAssemblyRect.Y);
	const FMargin RightLeaderboardContentPadding(
		RightShellOffset.X + RightLeaderboardFrameInsetX,
		RightShellOffset.Y + RightLeaderboardFrameInsetY,
		0.f,
		0.f);
	const TSharedRef<SWidget> RightLeaderboardPanel =
		SNew(SBox)
		.WidthOverride(RightPanelAssemblyRect.Width)
		.HeightOverride(RightPanelAssemblyRect.Height)
		[
			SNew(SOverlay)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(T66MainMenuReferenceLayout::Right::FilterWorldButton.X - RightPanelAssemblyRect.X, T66MainMenuReferenceLayout::Right::FilterWorldButton.Y - RightPanelAssemblyRect.Y, 0.f, 0.f))
			[
				MakeLeaderboardFilterButton(ET66LeaderboardFilter::Global)
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(T66MainMenuReferenceLayout::Right::FilterFriendsButton.X - RightPanelAssemblyRect.X, T66MainMenuReferenceLayout::Right::FilterFriendsButton.Y - RightPanelAssemblyRect.Y, 0.f, 0.f))
			[
				MakeLeaderboardFilterButton(ET66LeaderboardFilter::Friends)
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(T66MainMenuReferenceLayout::Right::FilterCrownButton.X - RightPanelAssemblyRect.X, T66MainMenuReferenceLayout::Right::FilterCrownButton.Y - RightPanelAssemblyRect.Y, 0.f, 0.f))
			[
				MakeLeaderboardFilterButton(ET66LeaderboardFilter::Streamers)
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(RightShellOffset.X, RightShellOffset.Y, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(RightShellRect.Width)
				.HeightOverride(RightShellRect.Height)
				[
					SNew(SImage)
					.Image(GetDailyRightShellBrush())
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(RightLeaderboardContentPadding)
			[
				SNew(SBox)
				.WidthOverride(RightShellRect.Width - (RightLeaderboardFrameInsetX * 2.f))
				.HeightOverride(RightShellRect.Height - (RightLeaderboardFrameInsetY * 2.f))
				[
					LeaderboardWidget
				]
			]
		];

	SetupDailyClimbRuntimeImageBrush(
		SkyBackgroundBrush,
		SkyBackgroundTexture,
		nullptr,
		TEXT("SourceAssets/UI/Reference/Screens/MainMenu/ScreenArt/mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_base_clean_bloodyretro_1920.png"),
		T66DailyClimbPanelReferenceSize);
	SetupDailyClimbRuntimeImageBrush(
		ForegroundOccluderBrush,
		ForegroundOccluderTexture,
		nullptr,
		TEXT("SourceAssets/UI/Reference/Screens/MainMenu/ScreenArt/mainmenu_screen_art_mainmenu_newmm_main_menu_newmm_foreground_occluder.png"),
		T66DailyClimbPanelReferenceSize);

	TSharedRef<SOverlay> Background = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SkyBackgroundBrush.IsValid() && SkyBackgroundBrush->GetResourceObject()
				? FT66Style::MakeRetroUIBackgroundImage(StaticCastSharedRef<SWidget>(SNew(SImage).Image(SkyBackgroundBrush.Get())))
				: StaticCastSharedRef<SWidget>(SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor::Black))
		];
	if (ForegroundOccluderBrush.IsValid() && ForegroundOccluderBrush->GetResourceObject())
	{
		Background->AddSlot()
		[
			FT66Style::MakeRetroUIBackgroundImage(StaticCastSharedRef<SWidget>(SNew(SImage).Image(ForegroundOccluderBrush.Get())))
		];
	}

	const FT66ReferenceRect& LeftPanelAssemblyRect = T66MainMenuReferenceLayout::MainMenu::LeftPanelAssembly;
	const FT66ReferenceRect& CtaStackRect = T66MainMenuReferenceLayout::Center::CtaStackFull;
	const float CenterButtonGap = 28.f;
	const float CenterButtonLeft = (CtaStackRect.Width - T66DailyClimbPrimaryButtonSize.X) * 0.5f;
	const float CenterButtonTop = (CtaStackRect.Height - (T66DailyClimbPrimaryButtonSize.Y * 2.f) - CenterButtonGap) * 0.5f;
	const TSharedRef<SWidget> CenterActions =
		SNew(SBox)
		.WidthOverride(CtaStackRect.Width)
		.HeightOverride(CtaStackRect.Height)
		[
			SNew(SOverlay)
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(CenterButtonLeft, CenterButtonTop, 0.f, 0.f))
			[
				StartActionWidget
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(FMargin(CenterButtonLeft, CenterButtonTop + T66DailyClimbPrimaryButtonSize.Y + CenterButtonGap, 0.f, 0.f))
			[
				ContinueActionWidget
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
			[
				Background
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.f, 328.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(760.f)
				.HeightOverride(150.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.DailyClimb", "Title", "DAILY DESCENT"))
						.Font(FT66Style::MakeFont(TEXT("Black"), 58))
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
			.Padding(FMargin(LeftPanelAssemblyRect.X, LeftPanelAssemblyRect.Y, 0.f, 0.f))
			[
				LeftDetailsPanel
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(CtaStackRect.X, CtaStackRect.Y, 0.f, 0.f))
			[
				CenterActions
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(RightPanelAssemblyRect.X, RightPanelAssemblyRect.Y, 0.f, 0.f))
			[
				RightLeaderboardPanel
			]
			]
		];

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				Canvas
			]
		];
}
