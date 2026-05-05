// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SaveSlotsScreen.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float T66SaveFlowActionHeight = 44.f;
	constexpr float T66SaveFlowPreviewSlotSize = 74.f;

	FLinearColor T66SaveFlowGoldText()
	{
		return FLinearColor(0.94f, 0.76f, 0.34f, 1.0f);
	}

	FLinearColor T66SaveFlowBrightText()
	{
		return FLinearColor(0.97f, 0.94f, 0.86f, 1.0f);
	}

	FLinearColor T66SaveFlowMutedText()
	{
		return FLinearColor(0.72f, 0.67f, 0.56f, 1.0f);
	}

	FLinearColor T66SaveFlowWarningText()
	{
		return FLinearColor(0.95f, 0.56f, 0.38f, 1.0f);
	}

	FLinearColor T66SaveFlowInkText()
	{
		return FLinearColor(0.12f, 0.075f, 0.035f, 1.0f);
	}

	FLinearColor T66SaveFlowInkMutedText()
	{
		return FLinearColor(0.33f, 0.23f, 0.13f, 1.0f);
	}

	const FSlateBrush* ResolveSaveFlowBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel,
			Filter);
	}

	const FSlateBrush* GetSaveFlowSceneBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/ScreenArt/saveslots_screen_art_mainmenu_main_menu_scene_plate_v1.png"),
			FMargin(0.f),
			TEXT("SaveSlotsScene"));
	}

	const FSlateBrush* GetSaveFlowContentShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Panels/saveslots_panels_fullscreen_fullscreen_panel_wide.png"),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			TEXT("SaveSlotsContentShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetSaveFlowRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Panels/saveslots_panels_fullscreen_row_shell_quiet.png"),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			TEXT("SaveSlotsRowShellV16"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetSaveFlowCardBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Panels/saveslots_panels_save_card_paper_frame.png"),
			FMargin(0.075f, 0.120f, 0.075f, 0.120f),
			TEXT("SaveSlotsCardPaperFrame"),
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetSaveFlowDropdownBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Controls/saveslots_controls_reference_dropdown_field_normal.png"),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f),
			TEXT("SaveSlotsDropdownField"));
	}

	const FSlateBrush* GetSaveFlowPartySlotFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveSaveFlowBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Slots/saveslots_slots_reference_square_slot_frame_normal.png"),
			FMargin(0.20f, 0.18f, 0.20f, 0.18f),
			TEXT("SaveSlotsPartySlotFrame"));
	}

	enum class ET66SaveFlowButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled,
		Selected
	};

	const FSlateBrush* GetSaveFlowButtonPlateBrush(const ET66SaveFlowButtonState State)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Neutral;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Selected;

		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Neutral;
		const TCHAR* StateName = TEXT("normal");
		const TCHAR* DebugLabel = TEXT("SaveSlotsButtonNormal");
		switch (State)
		{
		case ET66SaveFlowButtonState::Hovered:
			Entry = &Hovered;
			StateName = TEXT("hover");
			DebugLabel = TEXT("SaveSlotsButtonHover");
			break;
		case ET66SaveFlowButtonState::Pressed:
			Entry = &Pressed;
			StateName = TEXT("pressed");
			DebugLabel = TEXT("SaveSlotsButtonPressed");
			break;
		case ET66SaveFlowButtonState::Disabled:
			Entry = &Disabled;
			StateName = TEXT("disabled");
			DebugLabel = TEXT("SaveSlotsButtonDisabled");
			break;
		case ET66SaveFlowButtonState::Selected:
			Entry = &Selected;
			StateName = TEXT("selected");
			DebugLabel = TEXT("SaveSlotsButtonSelected");
			break;
		case ET66SaveFlowButtonState::Normal:
		default:
			break;
		}

		const FString RelativePath = FString::Printf(
			TEXT("SourceAssets/UI/Reference/Screens/SaveSlots/Buttons/saveslots_buttons_pill_%s.png"),
			StateName);

		return ResolveSaveFlowBrush(
			*Entry,
			*RelativePath,
			FMargin(0.f),
			DebugLabel,
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeSaveFlowBackground()
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.015f, 0.014f, 0.011f, 1.0f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(GetSaveFlowSceneBrush())
				.ColorAndOpacity(FLinearColor(0.88f, 0.88f, 0.88f, 1.0f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.48f))
			];
	}

	TSharedRef<SWidget> MakeSaveFlowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetSaveFlowContentShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FLinearColor(0.12f, 0.14f, 0.18f, 1.f))
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeSaveFlowRowShell(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bSelected, const bool bEnabled)
	{
		if (const FSlateBrush* RowBrush = GetSaveFlowRowShellBrush())
		{
			const FLinearColor Tint = bSelected
				? FLinearColor(1.09f, 1.02f, 0.76f, 1.f)
				: (bEnabled ? FLinearColor::White : FLinearColor(0.72f, 0.72f, 0.72f, 0.94f));
			return SNew(SBorder)
				.BorderImage(RowBrush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(bSelected ? FLinearColor(0.18f, 0.18f, 0.12f, 1.f) : FLinearColor(0.12f, 0.14f, 0.18f, 1.f))
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeSaveFlowCardShell(const TSharedRef<SWidget>& Content, const FMargin& Padding, const bool bSelected, const bool bEnabled)
	{
		if (const FSlateBrush* CardBrush = GetSaveFlowCardBrush())
		{
			const FLinearColor Tint = bSelected
				? FLinearColor(1.04f, 0.99f, 0.88f, 1.f)
				: (bEnabled ? FLinearColor::White : FLinearColor(0.96f, 0.88f, 0.72f, 1.f));
			return SNew(SBorder)
				.BorderImage(CardBrush)
				.BorderBackgroundColor(Tint)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return MakeSaveFlowRowShell(Content, Padding, bSelected, bEnabled);
	}

	TSharedRef<SWidget> MakeSaveFlowDropdownFrame(const TSharedRef<SWidget>& Content)
	{
		if (const FSlateBrush* DropdownBrush = GetSaveFlowDropdownBrush())
		{
			return SNew(SBorder)
				.BorderImage(DropdownBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(2.f))
				[
					Content
				];
		}

		return Content;
	}

	TSharedRef<SWidget> MakeSaveFlowDropdown(const FT66DropdownParams& Params)
	{
		static FComboButtonStyle FlatComboStyle = []()
		{
			FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
			Style.ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			return Style;
		}();

		TSharedRef<SComboButton> Combo = SNew(SComboButton)
			.ComboButtonStyle(&FlatComboStyle)
			.OnGetMenuContent_Lambda([OnGet = Params.OnGetMenuContent]()
			{
				return MakeSaveFlowShell(OnGet(), FMargin(8.f));
			})
			.ContentPadding(Params.Padding)
			.ButtonContent()
			[
				Params.Content
			];

		return SNew(SBox)
			.MinDesiredWidth(Params.MinWidth > 0.f ? Params.MinWidth : FOptionalSize())
			.HeightOverride(Params.Height > 0.f ? Params.Height : FOptionalSize())
			.Visibility(Params.Visibility)
			[
				MakeSaveFlowDropdownFrame(Combo)
			];
	}

	TSharedRef<SWidget> MakeSaveFlowPlateButton(FT66ButtonParams Params, const bool bEnabled)
	{
		if (!Params.CustomContent.IsValid())
		{
			const float ButtonHeight = Params.Height > 0.f ? Params.Height : T66SaveFlowActionHeight;
			Params
				.SetPadding(FMargin(6.f, 2.f))
				.SetContent(T66ScreenSlateHelpers::MakeFilledButtonText(
					Params,
					ButtonHeight,
					TAttribute<FSlateColor>(FSlateColor(bEnabled ? T66SaveFlowBrightText() : T66SaveFlowMutedText())),
					TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.70f))));
		}

		const bool bSelected = Params.Type == ET66ButtonType::ToggleActive;
		const TAttribute<bool> EnabledAttribute = TAttribute<bool>::CreateLambda([bEnabled, SourceEnabled = Params.IsEnabled]() -> bool
		{
			return bEnabled && SourceEnabled.Get(true);
		});
		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			Params.OnClicked,
			Params.CustomContent.ToSharedRef(),
			GetSaveFlowButtonPlateBrush(ET66SaveFlowButtonState::Normal),
			GetSaveFlowButtonPlateBrush(ET66SaveFlowButtonState::Hovered),
			GetSaveFlowButtonPlateBrush(ET66SaveFlowButtonState::Pressed),
			GetSaveFlowButtonPlateBrush(ET66SaveFlowButtonState::Disabled),
			Params.MinWidth,
			Params.Height > 0.f ? Params.Height : T66SaveFlowActionHeight,
			Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f),
			EnabledAttribute,
			Params.Visibility,
			0.105f,
			GetSaveFlowButtonPlateBrush(ET66SaveFlowButtonState::Selected),
			TAttribute<bool>(bSelected));
	}

	int32 T66PartySizeToMemberCount(const ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return 2;
		case ET66PartySize::Trio:
			return 3;
		case ET66PartySize::Quad:
			return 4;
		case ET66PartySize::Solo:
		default:
			return 1;
		}
	}

	FText T66PartySizeText(UT66LocalizationSubsystem* Loc, const ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo:
				return NSLOCTEXT("T66.SaveSlots", "DuoFallback", "Duo");
			case ET66PartySize::Trio:
				return NSLOCTEXT("T66.SaveSlots", "TrioFallback", "Trio");
			case ET66PartySize::Quad:
				return NSLOCTEXT("T66.SaveSlots", "QuadFallback", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.SaveSlots", "SoloFallback", "Solo");
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return Loc->GetText_Duo();
		case ET66PartySize::Trio:
			return Loc->GetText_Trio();
		case ET66PartySize::Quad:
			return Loc->GetText_Quad();
		case ET66PartySize::Solo:
		default:
			return Loc->GetText_Solo();
		}
	}

	FText T66DifficultyText(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (!Loc)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium:
				return NSLOCTEXT("T66.SaveSlots", "MediumFallback", "Medium");
			case ET66Difficulty::Hard:
				return NSLOCTEXT("T66.SaveSlots", "HardFallback", "Hard");
			case ET66Difficulty::VeryHard:
				return NSLOCTEXT("T66.SaveSlots", "VeryHardFallback", "Very Hard");
			case ET66Difficulty::Impossible:
				return NSLOCTEXT("T66.SaveSlots", "ImpossibleFallback", "Impossible");
			case ET66Difficulty::Easy:
			default:
				return NSLOCTEXT("T66.SaveSlots", "EasyFallback", "Easy");
			}
		}

		switch (Difficulty)
		{
		case ET66Difficulty::Medium:
			return Loc->GetText_Medium();
		case ET66Difficulty::Hard:
			return Loc->GetText_Hard();
		case ET66Difficulty::VeryHard:
			return Loc->GetText_VeryHard();
		case ET66Difficulty::Impossible:
			return Loc->GetText_Impossible();
		case ET66Difficulty::Easy:
		default:
			return Loc->GetText_Easy();
		}
	}

	void T66BuildSavedPartyPlayers(const UT66RunSaveGame* Loaded, TArray<FT66SavedPartyPlayerState>& OutPlayers)
	{
		OutPlayers.Reset();
		if (!Loaded)
		{
			return;
		}

		if (Loaded->SavedPartyPlayers.Num() > 0)
		{
			OutPlayers = Loaded->SavedPartyPlayers;
			return;
		}

		FT66SavedPartyPlayerState& HostPlayer = OutPlayers.AddDefaulted_GetRef();
		HostPlayer.PlayerId = Loaded->OwnerPlayerId;
		HostPlayer.DisplayName = Loaded->OwnerDisplayName;
		HostPlayer.HeroID = Loaded->HeroID;
		HostPlayer.HeroBodyType = Loaded->HeroBodyType;
		HostPlayer.CompanionID = Loaded->CompanionID;
		HostPlayer.PlayerTransform = Loaded->PlayerTransform;
		HostPlayer.bIsPartyHost = true;
	}

	FString T66BuildDateString(const FString& LastPlayedUtc)
	{
		FDateTime ParsedUtc;
		if (FDateTime::ParseIso8601(*LastPlayedUtc, ParsedUtc))
		{
			return ParsedUtc.ToString(TEXT("%Y-%m-%d"));
		}

		return LastPlayedUtc.IsEmpty() ? TEXT("--") : LastPlayedUtc.Left(10);
	}

	FString T66BuildTimeString(const FString& LastPlayedUtc)
	{
		FDateTime ParsedUtc;
		if (FDateTime::ParseIso8601(*LastPlayedUtc, ParsedUtc))
		{
			return ParsedUtc.ToString(TEXT("%H:%M"));
		}

		return LastPlayedUtc.IsEmpty() ? TEXT("--:--") : LastPlayedUtc.Mid(11, 5);
	}

	FString T66BuildInitials(const FString& DisplayName, const int32 SlotIndex)
	{
		const FString Trimmed = DisplayName.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return FString::Printf(TEXT("%d"), SlotIndex + 1);
		}

		TArray<FString> Parts;
		Trimmed.ParseIntoArrayWS(Parts);
		if (Parts.Num() >= 2)
		{
			return Parts[0].Left(1).ToUpper() + Parts[1].Left(1).ToUpper();
		}

		return Trimmed.Left(2).ToUpper();
	}
}

UT66SaveSlotsScreen::UT66SaveSlotsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SaveSlots;
	bIsModal = false;
}

TSharedRef<SWidget> UT66SaveSlotsScreen::BuildSlateUI()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	const bool bHostCanStartPartyLoad = !bIsPartyResumeFlow || (SessionSubsystem && SessionSubsystem->IsHostingPartySession());

	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText LoadText = NSLOCTEXT("T66.SaveSlots", "Load", "LOAD");
	const FText LoadGameTitleText = NSLOCTEXT("T66.SaveSlots", "LoadGameTitle", "LOAD GAME");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText PrevText = NSLOCTEXT("T66.SaveSlots", "PrevPage", "PREV");
	const FText NextText = NSLOCTEXT("T66.SaveSlots", "NextPage", "NEXT");
	const FText EmptySlotText = NSLOCTEXT("T66.SaveSlots", "EmptySlot", "Empty Slot");
	const FText StagePlaceholderText = NSLOCTEXT("T66.SaveSlots", "StagePlaceholder", "Stage --");
	const FText DatePlaceholderText = NSLOCTEXT("T66.SaveSlots", "DatePlaceholder", "--");
	const FText TimePlaceholderText = NSLOCTEXT("T66.SaveSlots", "TimePlaceholder", "--:--");

	SlotPartyAvatarBrushes.SetNum(SlotsPerPage);
	SlotHeroPortraitBrushes.SetNum(SlotsPerPage);
	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		SlotPartyAvatarBrushes[LocalIndex].SetNum(MaxPartyPreviewSlots);
		SlotHeroPortraitBrushes[LocalIndex].SetNum(MaxPartyPreviewSlots);
		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			if (!SlotPartyAvatarBrushes[LocalIndex][PartyIndex].IsValid())
			{
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex] = MakeShared<FSlateBrush>();
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->DrawAs = ESlateBrushDrawType::Image;
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->Tiling = ESlateBrushTileType::NoTile;
				SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->ImageSize = FVector2D(48.f, 48.f);
			}

			if (!SlotHeroPortraitBrushes[LocalIndex][PartyIndex].IsValid())
			{
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex] = MakeShared<FSlateBrush>();
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->DrawAs = ESlateBrushDrawType::Image;
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->Tiling = ESlateBrushTileType::NoTile;
				SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->ImageSize = FVector2D(48.f, 48.f);
			}

			SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
			SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
		}
	}

	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float SurfaceWidth = FMath::Clamp(SafeFrameSize.X - 168.f, 1180.f, 1504.f);
	const FMargin SurfacePadding(60.f, 42.f, 60.f, 36.f);
	constexpr int32 SlotColumns = 2;
	const float CardGapX = 36.f;
	const float CardGapY = 12.f;
	const float CardWidth = FMath::Max(600.f, (SurfaceWidth - SurfacePadding.Left - SurfacePadding.Right - (CardGapX * (SlotColumns - 1))) / SlotColumns);
	const float CardHeight = 318.f;
	const int32 CurrentPartyCount = PartySubsystem ? FMath::Max(1, PartySubsystem->GetPartyMemberCount()) : 1;

	auto MakePartySizeMenu = [this, Loc]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		for (const ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			const bool bSelectedPartySize = ActivePartySizeFilter == PartySize;
			Box->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 6.f)
				[
					FT66Style::MakeDropdownOptionButton(
						T66PartySizeText(Loc, PartySize),
						FOnClicked::CreateLambda([this, PartySize]()
						{
							ActivePartySizeFilter = PartySize;
							CurrentPage = 0;
							RefreshScreen();
							FSlateApplication::Get().DismissAllMenus();
							return FReply::Handled();
						}),
						bSelectedPartySize,
						150.f,
						38.f,
						18)
				];
		}
		return Box;
	};

	auto MakePartyMemberSlot = [this](const int32 LocalIndex, const int32 PartyIndex, const FT66SavedPartyPlayerState* SavedPlayer) -> TSharedRef<SWidget>
	{
		const bool bHasAvatar = SlotPartyAvatarBrushes.IsValidIndex(LocalIndex)
			&& SlotPartyAvatarBrushes[LocalIndex].IsValidIndex(PartyIndex)
			&& SlotPartyAvatarBrushes[LocalIndex][PartyIndex].IsValid()
			&& ::IsValid(SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->GetResourceObject());
		const FString Initials = SavedPlayer ? T66BuildInitials(SavedPlayer->DisplayName, PartyIndex) : FString::Printf(TEXT("%d"), PartyIndex + 1);
		const FLinearColor FrameTint = (SavedPlayer && SavedPlayer->bIsPartyHost)
			? FLinearColor(1.08f, 0.95f, 0.62f, 1.f)
			: (SavedPlayer ? FLinearColor::White : FLinearColor(0.55f, 0.55f, 0.55f, 0.86f));

		return SNew(SBox)
			.WidthOverride(T66SaveFlowPreviewSlotSize)
			.HeightOverride(T66SaveFlowPreviewSlotSize)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(GetSaveFlowPartySlotFrameBrush())
					.ColorAndOpacity(FrameTint)
				]
				+ SOverlay::Slot()
				.Padding(9.f)
				[
					bHasAvatar
						? StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(SlotPartyAvatarBrushes[LocalIndex][PartyIndex].Get()))
						: StaticCastSharedRef<SWidget>(
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.08f, 0.10f, 0.09f, 0.96f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(Initials))
								.Font(FT66Style::Tokens::FontBold(14))
								.ColorAndOpacity(SavedPlayer ? T66SaveFlowGoldText() : T66SaveFlowMutedText())
								.Justification(ETextJustify::Center)
							])
				]
			];
	};

	auto MakeHeroSlot = [this](const int32 LocalIndex, const int32 PartyIndex, const FT66SavedPartyPlayerState* SavedPlayer) -> TSharedRef<SWidget>
	{
		const bool bHasPortrait = SlotHeroPortraitBrushes.IsValidIndex(LocalIndex)
			&& SlotHeroPortraitBrushes[LocalIndex].IsValidIndex(PartyIndex)
			&& SlotHeroPortraitBrushes[LocalIndex][PartyIndex].IsValid()
			&& ::IsValid(SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->GetResourceObject());
		const FText PlaceholderText = SavedPlayer && !SavedPlayer->HeroID.IsNone()
			? FText::FromName(SavedPlayer->HeroID)
			: FText::FromString(TEXT("--"));
		const FLinearColor FrameTint = SavedPlayer ? FLinearColor::White : FLinearColor(0.55f, 0.55f, 0.55f, 0.86f);

		return SNew(SBox)
			.WidthOverride(T66SaveFlowPreviewSlotSize)
			.HeightOverride(T66SaveFlowPreviewSlotSize)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(GetSaveFlowPartySlotFrameBrush())
					.ColorAndOpacity(FrameTint)
				]
				+ SOverlay::Slot()
				.Padding(9.f)
				[
					bHasPortrait
						? StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(SlotHeroPortraitBrushes[LocalIndex][PartyIndex].Get()))
						: StaticCastSharedRef<SWidget>(
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.075f, 0.085f, 0.075f, 0.96f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(PlaceholderText)
								.Font(FT66Style::Tokens::FontBold(13))
								.ColorAndOpacity(SavedPlayer ? T66SaveFlowMutedText() : FLinearColor(0.46f, 0.43f, 0.36f, 1.f))
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
							])
				]
			];
	};

	TArray<int32> PageSlotIndices;
	TArray<bool> PageSlotHasVisibleSave;
	PageSlotIndices.Init(INDEX_NONE, SlotsPerPage);
	PageSlotHasVisibleSave.Init(false, SlotsPerPage);
	TSet<int32> UsedDisplaySlotIndices;
	auto CanUsePlaceholderSlot = [SaveSub, &UsedDisplaySlotIndices](const int32 SlotIndex) -> bool
	{
		return SlotIndex >= 0
			&& SlotIndex < UT66SaveSubsystem::MaxSlots
			&& !UsedDisplaySlotIndices.Contains(SlotIndex)
			&& (!SaveSub || !SaveSub->DoesSlotExist(SlotIndex));
	};

	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		const int32 VisibleIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
		if (VisibleSlotIndices.IsValidIndex(VisibleIndex))
		{
			const int32 SlotIndex = VisibleSlotIndices[VisibleIndex];
			PageSlotIndices[LocalIndex] = SlotIndex;
			PageSlotHasVisibleSave[LocalIndex] = true;
			UsedDisplaySlotIndices.Add(SlotIndex);
		}
	}

	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		if (PageSlotIndices[LocalIndex] != INDEX_NONE)
		{
			continue;
		}

		const int32 PreferredSlotIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
		if (CanUsePlaceholderSlot(PreferredSlotIndex))
		{
			PageSlotIndices[LocalIndex] = PreferredSlotIndex;
			UsedDisplaySlotIndices.Add(PreferredSlotIndex);
			continue;
		}

		for (int32 CandidateSlotIndex = 0; CandidateSlotIndex < UT66SaveSubsystem::MaxSlots; ++CandidateSlotIndex)
		{
			if (CanUsePlaceholderSlot(CandidateSlotIndex))
			{
				PageSlotIndices[LocalIndex] = CandidateSlotIndex;
				UsedDisplaySlotIndices.Add(CandidateSlotIndex);
				break;
			}
		}
	}

	auto MakeSlotCard = [this, SaveSub, Loc, GI, TexPool, SteamHelper, PreviewText, LoadText, EmptySlotText,
		StagePlaceholderText, DatePlaceholderText, TimePlaceholderText, CardWidth, CardHeight, CurrentPartyCount,
		bHostCanStartPartyLoad, PageSlotIndices, PageSlotHasVisibleSave, MakePartyMemberSlot, MakeHeroSlot](
		const int32 LocalIndex) -> TSharedRef<SWidget>
	{
		const int32 SlotIndex = PageSlotIndices.IsValidIndex(LocalIndex) ? PageSlotIndices[LocalIndex] : INDEX_NONE;
		if (SlotIndex == INDEX_NONE)
		{
			return SNew(SBox).Visibility(EVisibility::Collapsed);
		}

		const bool bHasVisibleSave = PageSlotHasVisibleSave.IsValidIndex(LocalIndex) && PageSlotHasVisibleSave[LocalIndex];
		UT66RunSaveGame* Loaded = (bHasVisibleSave && SaveSub) ? SaveSub->LoadFromSlot(SlotIndex) : nullptr;

		TArray<FT66SavedPartyPlayerState> SavedPlayers;
		T66BuildSavedPartyPlayers(Loaded, SavedPlayers);

		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);
			SlotHeroPortraitBrushes[LocalIndex][PartyIndex]->SetResourceObject(nullptr);

			const FT66SavedPartyPlayerState* SavedPlayer = SavedPlayers.IsValidIndex(PartyIndex) ? &SavedPlayers[PartyIndex] : nullptr;
			if (!SavedPlayer)
			{
				continue;
			}

			if (SteamHelper)
			{
				if (UTexture2D* AvatarTexture = SteamHelper->GetAvatarTextureForSteamId(SavedPlayer->PlayerId))
				{
					SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(AvatarTexture);
				}
				else if (SavedPlayer->PlayerId == SteamHelper->GetLocalSteamId())
				{
					SlotPartyAvatarBrushes[LocalIndex][PartyIndex]->SetResourceObject(SteamHelper->GetLocalAvatarTexture());
				}
			}

			if (GI && TexPool && !SavedPlayer->HeroID.IsNone())
			{
				FHeroData HeroData;
				if (GI->GetHeroData(SavedPlayer->HeroID, HeroData))
				{
					const TSoftObjectPtr<UTexture2D> PortraitSoft = GI->ResolveHeroPortrait(HeroData, SavedPlayer->HeroBodyType, ET66HeroPortraitVariant::Half);
					if (!PortraitSoft.IsNull())
					{
						T66SlateTexture::BindSharedBrushAsync(
							TexPool,
							PortraitSoft,
							this,
							SlotHeroPortraitBrushes[LocalIndex][PartyIndex],
							FName(*FString::Printf(TEXT("SaveSlotHero_%d_%d"), SlotIndex, PartyIndex)),
							true);
					}
				}
			}
		}

		const bool bHasRunData = Loaded != nullptr;
		const int32 RequiredPartyCount = bHasRunData
			? FMath::Max(1, SavedPlayers.Num() > 0 ? SavedPlayers.Num() : T66PartySizeToMemberCount(Loaded->PartySize))
			: 1;
		const bool bPartyCountMatches = CurrentPartyCount == RequiredPartyCount;
		const bool bCanLoad = bHasRunData && bHostCanStartPartyLoad && bPartyCountMatches;
		const FText StatusText = !bHasRunData
			? EmptySlotText
			: (bCanLoad
			? FText::Format(
				NSLOCTEXT("T66.SaveSlots", "DifficultyParty", "{0} / {1}"),
				T66DifficultyText(Loc, Loaded->Difficulty),
				T66PartySizeText(Loc, Loaded->PartySize))
			: FText::Format(
				NSLOCTEXT("T66.SaveSlots", "RequiresParty", "Requires {0} party"),
				T66PartySizeText(Loc, Loaded->PartySize)));
		const bool bIsSelected = bHasRunData && GI && GI->CurrentSaveSlotIndex == SlotIndex;
		const FText StageText = bHasRunData
			? FText::Format(NSLOCTEXT("T66.SaveSlots", "StageLabel", "Stage {0}"), FText::AsNumber(Loaded->StageReached))
			: StagePlaceholderText;
		const FText DateText = bHasRunData
			? FText::FromString(T66BuildDateString(Loaded->LastPlayedUtc))
			: DatePlaceholderText;
		const FText TimeText = bHasRunData
			? FText::FromString(T66BuildTimeString(Loaded->LastPlayedUtc))
			: TimePlaceholderText;

		TSharedRef<SGridPanel> SlotGrid = SNew(SGridPanel);
		for (int32 PartyIndex = 0; PartyIndex < MaxPartyPreviewSlots; ++PartyIndex)
		{
			const FT66SavedPartyPlayerState* SavedPlayer = SavedPlayers.IsValidIndex(PartyIndex) ? &SavedPlayers[PartyIndex] : nullptr;
			const bool bLastPreviewColumn = PartyIndex == MaxPartyPreviewSlots - 1;
			SlotGrid->AddSlot(PartyIndex, 0)
				.Padding(0.f, 0.f, bLastPreviewColumn ? 0.f : 8.f, 8.f)
				[
					MakePartyMemberSlot(LocalIndex, PartyIndex, SavedPlayer)
				];

			SlotGrid->AddSlot(PartyIndex, 1)
				.Padding(0.f, 0.f, bLastPreviewColumn ? 0.f : 8.f, 0.f)
				[
					MakeHeroSlot(LocalIndex, PartyIndex, SavedPlayer)
				];
		}

		return SNew(SBox)
			.WidthOverride(CardWidth)
			.HeightOverride(CardHeight)
			[
				MakeSaveFlowCardShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 12.f)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("T66.SaveSlots", "SlotHeader", "Save Slot {0}"), FText::AsNumber(SlotIndex + 1)))
						.Font(FT66Style::Tokens::FontBold(22))
						.ColorAndOpacity(bIsSelected ? FLinearColor(0.43f, 0.22f, 0.04f, 1.f) : T66SaveFlowInkText())
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						[
							SlotGrid
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(30.f, 2.f, 0.f, 0.f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(StageText)
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(bHasRunData ? T66SaveFlowInkText() : T66SaveFlowInkMutedText())
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(DateText)
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(T66SaveFlowInkMutedText())
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(TimeText)
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(T66SaveFlowInkMutedText())
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 6.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(DatePlaceholderText)
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(T66SaveFlowInkMutedText())
								.Visibility(bHasRunData ? EVisibility::Collapsed : EVisibility::Visible)
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.f)
							[
								SNew(SSpacer)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(StatusText)
								.Font(FT66Style::Tokens::FontBold(17))
								.ColorAndOpacity(!bHasRunData ? T66SaveFlowInkText() : (bCanLoad ? FLinearColor(0.34f, 0.20f, 0.04f, 1.f) : FLinearColor(0.52f, 0.17f, 0.10f, 1.f)))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 5.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(TimePlaceholderText)
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(T66SaveFlowInkMutedText())
								.Visibility(bHasRunData ? EVisibility::Collapsed : EVisibility::Visible)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 12.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(0.f, 0.f, 10.f, 0.f)
						[
							MakeSaveFlowPlateButton(
								FT66ButtonParams(PreviewText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePreviewClicked, SlotIndex), ET66ButtonType::Neutral)
								.SetMinWidth(128.f)
								.SetHeight(T66SaveFlowActionHeight)
								.SetFontSize(22)
								.SetEnabled(bHasRunData),
								bHasRunData)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							MakeSaveFlowPlateButton(
								FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleLoadClicked, SlotIndex), bCanLoad ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
								.SetMinWidth(128.f)
								.SetHeight(T66SaveFlowActionHeight)
								.SetFontSize(22)
								.SetEnabled(bCanLoad),
								bCanLoad)
						]
					],
					FMargin(32.f, 23.f, 32.f, 17.f),
					bIsSelected,
					bHasRunData && bCanLoad)
			];
	};

	TSharedRef<SGridPanel> CardGrid = SNew(SGridPanel);
	for (int32 LocalIndex = 0; LocalIndex < SlotsPerPage; ++LocalIndex)
	{
		const int32 Row = LocalIndex / SlotColumns;
		const int32 Column = LocalIndex % SlotColumns;
		CardGrid->AddSlot(Column, Row)
			.Padding(FMargin(
				0.f,
				0.f,
				Column + 1 < SlotColumns ? CardGapX : 0.f,
				Row + 1 < (SlotsPerPage / SlotColumns) ? CardGapY : 0.f))
			[
				MakeSlotCard(LocalIndex)
			];
	}

	const FText PageText = FText::Format(
		NSLOCTEXT("T66.SaveSlots", "PageFormat", "Page {0} / {1}"),
		FText::AsNumber(CurrentPage + 1),
		FText::AsNumber(TotalPages));
	const FText FilterHintText = FText::Format(
		NSLOCTEXT("T66.SaveSlots", "FilterHint", "Showing {0} saves stored on this machine."),
		T66PartySizeText(Loc, ActivePartySizeFilter));
	const bool bCanGoPrev = CurrentPage > 0;
	const bool bCanGoNext = CurrentPage < TotalPages - 1;

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			MakeSaveFlowBackground()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0.f, 10.f, 0.f, 10.f))
		[
			SNew(SBox)
			.WidthOverride(SurfaceWidth)
			[
				MakeSaveFlowShell(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 16.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							MakeSaveFlowPlateButton(
								FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleBackClicked), ET66ButtonType::Neutral)
								.SetMinWidth(124.f)
								.SetHeight(T66SaveFlowActionHeight)
								.SetFontSize(22),
								true)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LoadGameTitleText)
							.Font(FT66Style::Tokens::FontBold(54))
							.ColorAndOpacity(T66SaveFlowGoldText())
							.Justification(ETextJustify::Center)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(124.f)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f)
						[
							MakeSaveFlowDropdown(
								FT66DropdownParams(
									SNew(STextBlock)
									.Text(T66PartySizeText(Loc, ActivePartySizeFilter))
									.Font(FT66Style::Tokens::FontBold(20))
									.ColorAndOpacity(T66SaveFlowBrightText()),
									MakePartySizeMenu)
								.SetMinWidth(210.f)
								.SetHeight(54.f)
								.SetPadding(FMargin(16.f, 12.f)))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						.Padding(16.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FilterHintText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(T66SaveFlowMutedText())
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(PageText)
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(T66SaveFlowGoldText())
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0.f, 24.f, 0.f, 16.f)
					[
						CardGrid
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 12.f, 0.f)
						[
							MakeSaveFlowPlateButton(
								FT66ButtonParams(PrevText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandlePrevPageClicked), ET66ButtonType::Neutral)
								.SetMinWidth(118.f)
								.SetHeight(T66SaveFlowActionHeight)
								.SetFontSize(22)
								.SetEnabled(bCanGoPrev),
								bCanGoPrev)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							MakeSaveFlowPlateButton(
								FT66ButtonParams(NextText, FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleNextPageClicked), ET66ButtonType::Neutral)
								.SetMinWidth(118.f)
								.SetHeight(T66SaveFlowActionHeight)
								.SetFontSize(22)
								.SetEnabled(bCanGoNext),
								bCanGoNext)
						]
					],
					SurfacePadding)
			]
		];
}

FReply UT66SaveSlotsScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandleLoadClicked(const int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandlePreviewClicked(const int32 SlotIndex)
{
	OnPreviewClicked(SlotIndex);
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandlePrevPageClicked()
{
	OnPreviousPageClicked();
	return FReply::Handled();
}

FReply UT66SaveSlotsScreen::HandleNextPageClicked()
{
	OnNextPageClicked();
	return FReply::Handled();
}

void UT66SaveSlotsScreen::RebuildVisibleSlotIndices()
{
	VisibleSlotIndices.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		TotalPages = 1;
		CurrentPage = 0;
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < UT66SaveSubsystem::MaxSlots; ++SlotIndex)
	{
		bool bOccupied = false;
		FString LastPlayedUtc;
		FString HeroDisplayName;
		FString MapName;
		SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayedUtc, HeroDisplayName, MapName);
		if (!bOccupied)
		{
			continue;
		}

		UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
		if (!Loaded || Loaded->PartySize != ActivePartySizeFilter)
		{
			continue;
		}

		VisibleSlotIndices.Add(SlotIndex);
	}

	TotalPages = FMath::Max(1, (VisibleSlotIndices.Num() + SlotsPerPage - 1) / SlotsPerPage);
	CurrentPage = FMath::Clamp(CurrentPage, 0, TotalPages - 1);
}

int32 UT66SaveSlotsScreen::GetVisibleSlotIndexForPageEntry(const int32 LocalIndex) const
{
	const int32 VisibleIndex = (CurrentPage * SlotsPerPage) + LocalIndex;
	return VisibleSlotIndices.IsValidIndex(VisibleIndex) ? VisibleSlotIndices[VisibleIndex] : INDEX_NONE;
}

void UT66SaveSlotsScreen::PrepareGameInstanceForLoadedSave(UT66GameInstance* GI, const UT66RunSaveGame* Loaded, const int32 SlotIndex, const bool bPreviewMode)
{
	if (!GI || !Loaded)
	{
		return;
	}

	GI->SelectedHeroID = Loaded->HeroID;
	GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	GI->SelectedCompanionID = Loaded->CompanionID;
	GI->SelectedDifficulty = Loaded->Difficulty;
	GI->SelectedPartySize = Loaded->PartySize;
	GI->RunSeed = Loaded->RunSeed;
	if (Loaded->bIsDailyClimbRun && Loaded->DailyClimbChallenge.IsValid())
	{
		GI->CachedDailyClimbChallenge = Loaded->DailyClimbChallenge;
		GI->ActiveDailyClimbChallenge = Loaded->DailyClimbChallenge;
		GI->bIsDailyClimbRunActive = true;
		GI->SelectedRunModifierKind = ET66RunModifierKind::None;
		GI->SelectedRunModifierID = NAME_None;
		GI->SelectedPartySize = ET66PartySize::Solo;
	}
	else
	{
		GI->ClearActiveDailyClimbRun();
	}
	GI->CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
	GI->PendingLoadedTransform = Loaded->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->PendingLoadedRunSnapshot = Loaded->RunSnapshot;
	GI->bApplyLoadedRunSnapshot = Loaded->RunSnapshot.bValid;
	GI->CurrentSaveSlotIndex = SlotIndex;
	GI->bRunIneligibleForLeaderboard = Loaded->bRunIneligibleForLeaderboard;
	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->RestoreActiveRunContext(Loaded->IntegrityContext);
		Integrity->MarkLoadedSnapshot();
		GI->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard || !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
	}
	GI->CurrentRunOwnerPlayerId = Loaded->OwnerPlayerId;
	GI->CurrentRunOwnerDisplayName = Loaded->OwnerDisplayName;
	GI->CurrentRunPartyMemberIds = Loaded->PartyMemberIds;
	GI->CurrentRunPartyMemberDisplayNames = Loaded->PartyMemberDisplayNames;
	GI->bSaveSlotPreviewMode = bPreviewMode;
	GI->PersistRememberedSelectionDefaults();
}

bool UT66SaveSlotsScreen::CanLoadSlotWithCurrentParty(const UT66RunSaveGame* Loaded) const
{
	if (!Loaded)
	{
		return false;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	const UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	if (bIsPartyResumeFlow && !SessionSubsystem->IsHostingPartySession())
	{
		return false;
	}

	const int32 CurrentPartyCount = PartySubsystem ? FMath::Max(1, PartySubsystem->GetPartyMemberCount()) : 1;
	const int32 RequiredPartyCount = FMath::Max(1, Loaded->SavedPartyPlayers.Num() > 0 ? Loaded->SavedPartyPlayers.Num() : T66PartySizeToMemberCount(Loaded->PartySize));
	return CurrentPartyCount == RequiredPartyCount;
}

void UT66SaveSlotsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (GI && GI->bRestoreSaveSlotsState)
	{
		ActivePartySizeFilter = GI->PendingSaveSlotsPartyFilter;
		CurrentPage = FMath::Max(0, GI->PendingSaveSlotsPage);
		GI->bRestoreSaveSlotsState = false;
	}
	else
	{
		ActivePartySizeFilter = PartySubsystem ? PartySubsystem->GetCurrentPartySizeEnum() : ET66PartySize::Solo;
		CurrentPage = 0;
	}

	RebuildVisibleSlotIndices();
}

void UT66SaveSlotsScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	RebuildVisibleSlotIndices();
	ForceRebuildSlate();
}

void UT66SaveSlotsScreen::OnLoadClicked(const int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!GI || !SaveSub)
	{
		return;
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded || !CanLoadSlotWithCurrentParty(Loaded))
	{
		return;
	}

	const bool bIsPartyResumeFlow = SessionSubsystem
		&& SessionSubsystem->IsPartyLobbyContextActive()
		&& SessionSubsystem->GetCurrentLobbyPlayerCount() > 1;
	if (bIsPartyResumeFlow)
	{
		GI->bSaveSlotPreviewMode = false;
		if (!SessionSubsystem->IsHostingPartySession())
		{
			return;
		}

		if (SessionSubsystem->StartLoadedGameplayTravel(Loaded, SlotIndex))
		{
			if (UIManager)
			{
				UIManager->HideAllUI();
			}
			return;
		}
	}

	GI->bRestoreSaveSlotsState = false;
	GI->PendingSaveSlotsPage = 0;
	PrepareGameInstanceForLoadedSave(GI, Loaded, SlotIndex, false);

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	if (Loaded->RunSnapshot.bValid)
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
	}
	else
	{
		GI->TransitionToGameplayLevel();
	}
}

void UT66SaveSlotsScreen::OnPreviewClicked(const int32 SlotIndex)
{
	if (!VisibleSlotIndices.Contains(SlotIndex) || !IsSlotOccupied(SlotIndex))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !SaveSub)
	{
		return;
	}

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded)
	{
		return;
	}

	GI->bRestoreSaveSlotsState = true;
	GI->PendingSaveSlotsPartyFilter = ActivePartySizeFilter;
	GI->PendingSaveSlotsPage = CurrentPage;
	PrepareGameInstanceForLoadedSave(GI, Loaded, SlotIndex, true);

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	if (Loaded->RunSnapshot.bValid)
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
	}
	else
	{
		GI->TransitionToGameplayLevel();
	}
}

void UT66SaveSlotsScreen::OnSlotClicked(const int32 SlotIndex)
{
	OnLoadClicked(SlotIndex);
}

bool UT66SaveSlotsScreen::IsSlotOccupied(const int32 SlotIndex) const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!SaveSub)
	{
		return false;
	}

	bool bOccupied = false;
	FString LastPlayedUtc;
	FString HeroDisplayName;
	FString MapName;
	SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayedUtc, HeroDisplayName, MapName);
	return bOccupied;
}

void UT66SaveSlotsScreen::OnPreviousPageClicked()
{
	if (CurrentPage > 0)
	{
		--CurrentPage;
		RefreshScreen();
	}
}

void UT66SaveSlotsScreen::OnNextPageClicked()
{
	if (CurrentPage < TotalPages - 1)
	{
		++CurrentPage;
		RefreshScreen();
	}
}

void UT66SaveSlotsScreen::OnBackClicked()
{
	NavigateBack();
}
