// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TemporaryBuffSelectionScreen.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/T66UIManager.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ET66TempBuffButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleInactive,
		CtaGreen
	};

	enum class ET66TempBuffButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed
	};

	struct FT66TempBuffSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FT66TempBuffButtonBrushSet
	{
		FT66TempBuffSpriteBrushEntry Normal;
		FT66TempBuffSpriteBrushEntry Hover;
		FT66TempBuffSpriteBrushEntry Pressed;
		FT66TempBuffSpriteBrushEntry Disabled;
	};

	const FLinearColor T66TempBuffFantasyText(0.953f, 0.925f, 0.835f, 1.0f);
	const FLinearColor T66TempBuffFantasyMuted(0.738f, 0.708f, 0.648f, 1.0f);
	const FLinearColor T66TempBuffFallbackPanel(0.025f, 0.023f, 0.034f, 0.97f);

	const FSlateBrush* ResolveTempBuffSpriteBrush(
		FT66TempBuffSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = DrawAs;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = Margin;
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("TempBuffSelectionReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* ResolveTempBuffSpriteRegionBrush(
		FT66TempBuffSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const FBox2f& UVRegion,
		const FLinearColor& Tint,
		const ESlateBrushDrawType::Type DrawAs,
		const TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
		}

		Entry.Brush->DrawAs = DrawAs;
		Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
		Entry.Brush->TintColor = FSlateColor(Tint);
		Entry.Brush->ImageSize = ImageSize;
		Entry.Brush->Margin = Margin;
		Entry.Brush->SetUVRegion(UVRegion);

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					Filter,
					true,
					TEXT("TempBuffSelectionReferenceRegionSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetTempBuffContentShellBrush()
	{
		static FT66TempBuffSpriteBrushEntry Entry;
		return ResolveTempBuffSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Panels/temporarybuffselection_panels_fullscreen_fullscreen_panel_wide.png"),
			FVector2D(1588.f, 653.f),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetTempBuffRowShellBrush()
	{
		static FT66TempBuffSpriteBrushEntry Entry;
		return ResolveTempBuffSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Panels/temporarybuffselection_panels_fullscreen_row_shell_quiet.png"),
			FVector2D(1632.f, 209.f),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetTempBuffCardShellBrush(const bool bSelected, const bool bDisabled = false)
	{
		static FT66TempBuffSpriteBrushEntry Normal;
		static FT66TempBuffSpriteBrushEntry Selected;
		static FT66TempBuffSpriteBrushEntry Disabled;
		FT66TempBuffSpriteBrushEntry& Entry = bDisabled ? Disabled : (bSelected ? Selected : Normal);
		return ResolveTempBuffSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Panels/temporarybuffselection_panels_fullscreen_row_shell_quiet.png"),
			FVector2D(1632.f, 209.f),
			FMargin(0.070f, 0.155f, 0.070f, 0.155f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetTempBuffSlotFrameBrush()
	{
		static FT66TempBuffSpriteBrushEntry Entry;
		return ResolveTempBuffSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Slots/temporarybuffselection_slots_reference_square_slot_frame_normal.png"),
			FVector2D(132.f, 132.f),
			FMargin(0.125f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetTempBuffReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FT66TempBuffSpriteBrushEntry TrackEntry;
		static FT66TempBuffSpriteBrushEntry ThumbEntry;
		static FT66TempBuffSpriteBrushEntry HoverEntry;

		const FString ControlsPath = TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Controls/temporarybuffselection_controls_controls_sheet.png");
		const FBox2f VerticalBarUV(
			FVector2f(4.f / 1350.f, 4.f / 926.f),
			FVector2f(90.f / 1350.f, 644.f / 926.f));

		const FSlateBrush* TrackBrush = ResolveTempBuffSpriteRegionBrush(
			TrackEntry,
			ControlsPath,
			FVector2D(14.f, 120.f),
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			FLinearColor(0.35f, 0.34f, 0.30f, 0.70f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveTempBuffSpriteRegionBrush(
			ThumbEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FLinearColor(0.93f, 0.82f, 0.52f, 1.0f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveTempBuffSpriteRegionBrush(
			HoverEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FLinearColor(1.0f, 0.90f, 0.62f, 1.0f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);

		if (TrackBrush && ThumbBrush && HoverBrush)
		{
			Style
				.SetVerticalBackgroundImage(*TrackBrush)
				.SetVerticalTopSlotImage(*TrackBrush)
				.SetVerticalBottomSlotImage(*TrackBrush)
				.SetNormalThumbImage(*ThumbBrush)
				.SetHoveredThumbImage(*HoverBrush)
				.SetDraggedThumbImage(*HoverBrush)
				.SetThickness(14.f);
		}

		return &Style;
	}

	FString GetTempBuffButtonPath(const ET66TempBuffButtonFamily Family, const ET66TempBuffButtonState State)
	{
		const bool bCta = Family == ET66TempBuffButtonFamily::CtaGreen;
		const TCHAR* Suffix = TEXT("normal");
		if (Family == ET66TempBuffButtonFamily::ToggleOn && State == ET66TempBuffButtonState::Normal)
		{
			Suffix = TEXT("selected");
		}
		else if (State == ET66TempBuffButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66TempBuffButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}

		return FString::Printf(
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Buttons/temporarybuffselection_buttons_%s_%s.png"),
			bCta ? TEXT("cta") : TEXT("pill"),
			Suffix);
	}

	FVector2D GetTempBuffButtonSize(const ET66TempBuffButtonFamily Family, const ET66TempBuffButtonState State)
	{
		if (Family == ET66TempBuffButtonFamily::CtaGreen)
		{
			return FVector2D(388.f, 100.f);
		}
		if (Family == ET66TempBuffButtonFamily::ToggleOn)
		{
			return State == ET66TempBuffButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66TempBuffButtonFamily::ToggleInactive)
		{
			return State == ET66TempBuffButtonState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66TempBuffButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66TempBuffButtonBrushSet& GetTempBuffButtonBrushSet(const ET66TempBuffButtonFamily Family)
	{
		static FT66TempBuffButtonBrushSet CompactNeutral;
		static FT66TempBuffButtonBrushSet ToggleOn;
		static FT66TempBuffButtonBrushSet ToggleInactive;
		static FT66TempBuffButtonBrushSet CtaGreen;

		if (Family == ET66TempBuffButtonFamily::ToggleOn)
		{
			return ToggleOn;
		}
		if (Family == ET66TempBuffButtonFamily::ToggleInactive)
		{
			return ToggleInactive;
		}
		if (Family == ET66TempBuffButtonFamily::CtaGreen)
		{
			return CtaGreen;
		}
		return CompactNeutral;
	}

	const FSlateBrush* GetTempBuffButtonBrush(const ET66TempBuffButtonFamily Family, const ET66TempBuffButtonState State)
	{
		FT66TempBuffButtonBrushSet& Set = GetTempBuffButtonBrushSet(Family);
		FT66TempBuffSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66TempBuffButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66TempBuffButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}

		return ResolveTempBuffSpriteBrush(
			*Entry,
			GetTempBuffButtonPath(Family, State),
			GetTempBuffButtonSize(Family, State),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetTempBuffDisabledButtonBrush(const ET66TempBuffButtonFamily Family)
	{
		const bool bCta = Family == ET66TempBuffButtonFamily::CtaGreen;
		FT66TempBuffButtonBrushSet& Set = GetTempBuffButtonBrushSet(Family);
		return ResolveTempBuffSpriteBrush(
			Set.Disabled,
			FString::Printf(
				TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffSelection/Buttons/temporarybuffselection_buttons_%s_disabled.png"),
				bCta ? TEXT("cta") : TEXT("pill")),
			bCta ? FVector2D(240.f, 200.f) : FVector2D(180.f, 69.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeTempBuffSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor = T66TempBuffFallbackPanel)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeTempBuffFramedSlotSurface(
		const TSharedRef<SWidget>& Content,
		const FLinearColor& InnerColor,
		const FLinearColor& FocusTint = FLinearColor::White)
	{
		const FSlateBrush* FrameBrush = GetTempBuffSlotFrameBrush();
		return SNew(SBorder)
			.BorderImage(FrameBrush ? FrameBrush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FrameBrush ? FocusTint : T66TempBuffFallbackPanel)
			.Padding(FMargin(8.f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(InnerColor)
				.Padding(FMargin(0.f))
				[
					Content
				]
			];
	}

	TSharedRef<SWidget> MakeTempBuffSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66TempBuffButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const TAttribute<bool>& IsEnabled = true)
	{
		const FSlateBrush* NormalBrush = GetTempBuffButtonBrush(Family, ET66TempBuffButtonState::Normal);
		const FSlateBrush* HoverBrush = GetTempBuffButtonBrush(Family, ET66TempBuffButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetTempBuffButtonBrush(Family, ET66TempBuffButtonState::Pressed);
		const FSlateBrush* DisabledBrush = GetTempBuffDisabledButtonBrush(Family);
		if (!NormalBrush)
		{
			return FT66Style::MakeButton(
				FT66ButtonParams(Label, OnClicked, Family == ET66TempBuffButtonFamily::ToggleOn || Family == ET66TempBuffButtonFamily::CtaGreen ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetFontSize(FontSize)
				.SetEnabled(IsEnabled));
		}

		const TAttribute<FSlateColor> TextColorAttr = TAttribute<FSlateColor>::CreateLambda([IsEnabled]() -> FSlateColor
			{
				return IsEnabled.Get() ? FSlateColor(T66TempBuffFantasyText) : FSlateColor(T66TempBuffFantasyMuted);
			});

		return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
			OnClicked,
			SNew(STextBlock)
			.Text(Label)
			.Font(FT66Style::Tokens::FontBold(FontSize))
			.ColorAndOpacity(TextColorAttr)
			.Justification(ETextJustify::Center)
			.AutoWrapText(true),
			NormalBrush,
			HoverBrush,
			PressedBrush,
			DisabledBrush,
			MinWidth,
			Height,
			FMargin(12.f, 7.f, 12.f, 6.f),
			IsEnabled);
	}

	FText T66TempBuffDifficultyText(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (!Loc)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium:
				return NSLOCTEXT("T66.TempBuffs", "MediumFallback", "Medium");
			case ET66Difficulty::Hard:
				return NSLOCTEXT("T66.TempBuffs", "HardFallback", "Hard");
			case ET66Difficulty::VeryHard:
				return NSLOCTEXT("T66.TempBuffs", "VeryHardFallback", "Very Hard");
			case ET66Difficulty::Impossible:
				return NSLOCTEXT("T66.TempBuffs", "ImpossibleFallback", "Impossible");
			case ET66Difficulty::Easy:
			default:
				return NSLOCTEXT("T66.TempBuffs", "EasyFallback", "Easy");
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
}

UT66TemporaryBuffSelectionScreen::UT66TemporaryBuffSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TemporaryBuffSelection;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66TemporaryBuffSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

UT66BuffSubsystem* UT66TemporaryBuffSelectionScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}

	return nullptr;
}

UT66HeroSelectionScreen* UT66TemporaryBuffSelectionScreen::GetLinkedHeroSelectionScreen() const
{
	return UIManager ? Cast<UT66HeroSelectionScreen>(UIManager->GetCurrentScreen()) : nullptr;
}

void UT66TemporaryBuffSelectionScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66TemporaryBuffSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	UT66HeroSelectionScreen* HeroSelectionScreen = GetLinkedHeroSelectionScreen();

	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.99f, FMath::Max(960.f, SafeFrameSize.X * 0.965f));
	const float ModalHeight = FMath::Min(SafeFrameSize.Y * 0.99f, FMath::Max(720.f, SafeFrameSize.Y * 0.94f));
	const int32 Columns = 6;
	const float CardGap = 10.0f;
	const float GridHorizontalReserve = 132.0f;
	const float CardWidth = (ModalWidth - GridHorizontalReserve - CardGap * static_cast<float>(Columns - 1)) / static_cast<float>(Columns);
	constexpr float CardHeight = 236.0f;
	const int32 FocusedSlotIndex = Buffs ? Buffs->GetSelectedSingleUseBuffEditSlotIndex() : 0;
	const TArray<ET66SecondaryStatType> ActiveLoadoutSlots = Buffs ? Buffs->GetSelectedSingleUseBuffSlots() : TArray<ET66SecondaryStatType>{};
	const int32 ChadCouponBalance = Buffs ? Buffs->GetChadCouponBalance() : 0;
	const int32 SingleUseBuffCost = Buffs ? Buffs->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
	const ET66Difficulty SelectedDifficulty = HeroSelectionScreen ? HeroSelectionScreen->SelectedDifficulty : (GI ? GI->SelectedDifficulty : ET66Difficulty::Easy);

	const FText TitleText = NSLOCTEXT("T66.TempBuffs", "EditTitle", "SELECT TEMP BUFFS");
	const FText DoneText = NSLOCTEXT("T66.TempBuffs", "Done", "DONE");
	const FText EnterText = NSLOCTEXT("T66.TempBuffs", "EnterTribulation", "ENTER TRIBULATION");
	const FText EmptyText = NSLOCTEXT("T66.TempBuffs", "EmptyOwned", "Pick a buff for the highlighted slot. Missing copies can be bought directly from each buff card.");
	const FText UseText = NSLOCTEXT("T66.TempBuffs", "Use", "USE");
	const FText BuySlotText = NSLOCTEXT("T66.TempBuffs", "BuySlot", "BUY");
	const FText ClearSlotText = NSLOCTEXT("T66.TempBuffs", "ClearSlot", "CLEAR");
	const FText SlotHintText = NSLOCTEXT("T66.TempBuffs", "SlotHint", "Click a top slot, then pick a buff below.");
	const FText EmptySlotText = NSLOCTEXT("T66.TempBuffs", "EmptyPresetSlot", "+");
	const FText CouponsHeaderText = NSLOCTEXT("T66.TempBuffs", "CouponsHeader", "CHAD COUPONS");
	const FText CouponsValueText = FText::Format(NSLOCTEXT("T66.TempBuffs", "CouponsValue", "{0} CC"), FText::AsNumber(ChadCouponBalance));

	BuffIconBrushes.Reset();
	FooterPartyAvatarBrushes.Reset();
	FooterPartyAvatarBrushes.SetNum(4);
	for (int32 SlotIndex = 0; SlotIndex < FooterPartyAvatarBrushes.Num(); ++SlotIndex)
	{
		if (!FooterPartyAvatarBrushes[SlotIndex].IsValid())
		{
			FooterPartyAvatarBrushes[SlotIndex] = MakeShared<FSlateBrush>();
			FooterPartyAvatarBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
			FooterPartyAvatarBrushes[SlotIndex]->Tiling = ESlateBrushTileType::NoTile;
			FooterPartyAvatarBrushes[SlotIndex]->ImageSize = FVector2D(52.f, 52.f);
		}
		FooterPartyAvatarBrushes[SlotIndex]->SetResourceObject(nullptr);
	}

	const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
	for (int32 PartyIndex = 0; PartyIndex < FooterPartyAvatarBrushes.Num(); ++PartyIndex)
	{
		if (PartyMembers.IsValidIndex(PartyIndex) && SteamHelper)
		{
			if (UTexture2D* AvatarTexture = SteamHelper->GetAvatarTextureForSteamId(PartyMembers[PartyIndex].PlayerId))
			{
				FooterPartyAvatarBrushes[PartyIndex]->SetResourceObject(AvatarTexture);
			}
			else if (PartyMembers[PartyIndex].PlayerId == SteamHelper->GetLocalSteamId())
			{
				FooterPartyAvatarBrushes[PartyIndex]->SetResourceObject(SteamHelper->GetLocalAvatarTexture());
			}
		}
	}

	TArray<TSharedPtr<FSlateBrush>> LoadoutSlotBrushes;
	LoadoutSlotBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = ActiveLoadoutSlots.IsValidIndex(SlotIndex) ? ActiveLoadoutSlots[SlotIndex] : ET66SecondaryStatType::None;
		if (T66IsLiveSecondaryStatType(SlotStat))
		{
			LoadoutSlotBrushes[SlotIndex] = T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, SlotStat, FVector2D(46.f, 46.f));
			BuffIconBrushes.Add(LoadoutSlotBrushes[SlotIndex]);
		}
	}

	auto MakeLoadoutSlotWidget = [this, Buffs, FocusedSlotIndex, ActiveLoadoutSlots, LoadoutSlotBrushes, BuySlotText, ClearSlotText, EmptySlotText](const int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const ET66SecondaryStatType SlotStat = ActiveLoadoutSlots.IsValidIndex(SlotIndex) ? ActiveLoadoutSlots[SlotIndex] : ET66SecondaryStatType::None;
		const bool bFilled = T66IsLiveSecondaryStatType(SlotStat);
		const bool bOwnedForSlot = Buffs ? Buffs->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		const bool bFocused = SlotIndex == FocusedSlotIndex;
		const FLinearColor InnerColor = bFilled
			? (bOwnedForSlot ? FT66Style::Tokens::Panel2 : FLinearColor(0.20f, 0.10f, 0.10f, 1.0f))
			: FT66Style::Tokens::Panel2;

		const TSharedRef<SWidget> SlotContent = bFilled
			? StaticCastSharedRef<SWidget>(
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SImage)
					.Image(LoadoutSlotBrushes.IsValidIndex(SlotIndex) && LoadoutSlotBrushes[SlotIndex].IsValid() ? LoadoutSlotBrushes[SlotIndex].Get() : nullptr)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, bOwnedForSlot ? 1.0f : 0.55f))
				])
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(EmptySlotText)
				.Font(FT66Style::Tokens::FontBold(24))
				.ColorAndOpacity(FLinearColor(0.76f, 0.13f, 0.95f, 1.0f)));

		const TSharedRef<SWidget> SlotAction = bFilled
			? StaticCastSharedRef<SWidget>(
				MakeTempBuffSpriteButton(
					bOwnedForSlot ? ClearSlotText : BuySlotText,
					bOwnedForSlot
						? FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotCleared, SlotIndex)
						: FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotPurchased, SlotIndex),
					bOwnedForSlot ? ET66TempBuffButtonFamily::CompactNeutral : ET66TempBuffButtonFamily::ToggleOn,
					0.f,
					26.f,
					9))
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));

		const FButtonStyle& SlotButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		const FLinearColor SlotInnerColor = bFilled ? InnerColor : FLinearColor(0.018f, 0.055f, 0.085f, 1.0f);

		return SNew(SBox)
			.WidthOverride(112.f)
			.HeightOverride(bFilled ? 112.f : 78.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.WidthOverride(112.f)
					.HeightOverride(78.f)
					[
						SNew(SButton)
						.ButtonStyle(&SlotButtonStyle)
						.ContentPadding(FMargin(0.f))
						.OnClicked(FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotClicked, SlotIndex))
						[
							MakeTempBuffFramedSlotSurface(
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SlotContent
								],
								SlotInnerColor,
								bFocused ? FLinearColor(1.0f, 0.88f, 0.48f, 1.0f) : FLinearColor::White)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(bFilled ? 26.f : 0.f)
					.Visibility(bFilled ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SlotAction
					]
				]
			];
	};

	auto MakePartyBox = [this, PartyMembers]() -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		for (int32 PartyIndex = 0; PartyIndex < 4; ++PartyIndex)
		{
			const FT66PartyMemberEntry* PartyMember = PartyMembers.IsValidIndex(PartyIndex) ? &PartyMembers[PartyIndex] : nullptr;
			const bool bHasAvatar = FooterPartyAvatarBrushes.IsValidIndex(PartyIndex)
				&& FooterPartyAvatarBrushes[PartyIndex].IsValid()
				&& ::IsValid(FooterPartyAvatarBrushes[PartyIndex]->GetResourceObject());
			const FString FallbackText = PartyMember && !PartyMember->DisplayName.IsEmpty()
				? PartyMember->DisplayName.Left(1).ToUpper()
				: FString::Printf(TEXT("%d"), PartyIndex + 1);

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(PartyIndex > 0 ? FMargin(10.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(52.f)
					.HeightOverride(52.f)
					[
						MakeTempBuffFramedSlotSurface(
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								bHasAvatar
									? StaticCastSharedRef<SWidget>(
										SNew(SImage)
										.Image(FooterPartyAvatarBrushes[PartyIndex].Get()))
									: StaticCastSharedRef<SWidget>(
										SNew(SBox)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FallbackText))
											.Font(FT66Style::Tokens::FontBold(10))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.Justification(ETextJustify::Center)
										])
							],
							PartyIndex == 0 ? FLinearColor(0.29f, 0.24f, 0.13f, 1.0f) : FLinearColor(0.15f, 0.17f, 0.19f, 1.0f),
							FLinearColor::White)
					]
				];
		}

		return MakeTempBuffSpritePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.TempBuffs", "PartyHeader", "PARTY"))
				.Font(FT66Style::Tokens::FontBold(10))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				PartySlots
			],
			GetTempBuffRowShellBrush(),
			FMargin(14.f, 10.f),
			FLinearColor(0.f, 0.f, 0.f, 0.95f));
	};

	auto MakeRunControls = [this, Loc, SelectedDifficulty, EnterText]() -> TSharedRef<SWidget>
	{
		auto MakeDifficultyMenu = [this, Loc, SelectedDifficulty]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			for (const ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
			{
				Box->AddSlot()
					.AutoHeight()
					[
						FT66Style::MakeDropdownOptionButton(
							T66TempBuffDifficultyText(Loc, Difficulty),
							FOnClicked::CreateLambda([this, Difficulty]()
							{
								if (UT66HeroSelectionScreen* HeroSelectionScreen = GetLinkedHeroSelectionScreen())
								{
									HeroSelectionScreen->SelectDifficulty(Difficulty);
									HeroSelectionScreen->RefreshScreen();
								}
								else if (UT66GameInstance* LinkedGI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
								{
									LinkedGI->SelectedDifficulty = Difficulty;
								}

								RefreshScreen();
								FSlateApplication::Get().DismissAllMenus();
								return FReply::Handled();
							}),
							SelectedDifficulty == Difficulty,
							150.f,
							34.f,
							11)
					];
			}
			return Box;
		};

		return MakeTempBuffSpritePanel(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.34f)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				FT66Style::MakeDropdown(
					FT66DropdownParams(
						SNew(STextBlock)
						.Text(T66TempBuffDifficultyText(Loc, SelectedDifficulty))
						.Font(FT66Style::Tokens::FontBold(11))
						.ColorAndOpacity(FT66Style::Tokens::Text),
						MakeDifficultyMenu)
					.SetMinWidth(150.f)
					.SetHeight(40.f)
					.SetPadding(FMargin(12.f, 8.f)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.66f)
			[
				MakeTempBuffSpriteButton(
					EnterText,
					FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleEnterClicked),
					ET66TempBuffButtonFamily::CtaGreen,
					0.f,
					42.f,
					11)
			],
			GetTempBuffRowShellBrush(),
			FMargin(12.f, 10.f),
			FLinearColor(0.f, 0.f, 0.f, 0.95f));
	};

	TSharedRef<SHorizontalBox> LoadoutSlotsRow = SNew(SHorizontalBox);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		LoadoutSlotsRow->AddSlot()
		.AutoWidth()
		.Padding(SlotIndex + 1 < UT66BuffSubsystem::MaxSelectedSingleUseBuffs ? FMargin(0.f, 0.f, 33.f, 0.f) : FMargin(0.f))
		[
			MakeLoadoutSlotWidget(SlotIndex)
		];
	}

	const TArray<ET66SecondaryStatType> AllBuffs = UT66BuffSubsystem::GetAllSingleUseBuffTypes();
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllBuffs.Num(); ++Index)
	{
		const ET66SecondaryStatType StatType = AllBuffs[Index];
		const int32 OwnedCount = Buffs ? Buffs->GetOwnedSingleUseBuffCount(StatType) : 0;
		const int32 AssignedCount = Buffs ? Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType) : 0;
		const ET66SecondaryStatType FocusedSlotStat = ActiveLoadoutSlots.IsValidIndex(FocusedSlotIndex) ? ActiveLoadoutSlots[FocusedSlotIndex] : ET66SecondaryStatType::None;
		const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
		const bool bCanUseOwnedCopy = OwnedCount > AssignedOutsideFocused;
		const bool bFocusedSlotMatches = FocusedSlotStat == StatType;
		const FText NameText = Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
		const FText DescText = Loc ? Loc->GetText_SecondaryStatDescription(StatType) : FText::GetEmpty();
		const FText OwnedCountText = FText::Format(NSLOCTEXT("T66.TempBuffs", "OwnedCount", "Owned: {0}"), FText::AsNumber(OwnedCount));
		const FText AssignedCountText = FText::Format(NSLOCTEXT("T66.TempBuffs", "AssignedCount", "Slotted: {0}"), FText::AsNumber(AssignedCount));
		const FText ActionText = bCanUseOwnedCopy
			? UseText
			: FText::Format(NSLOCTEXT("T66.TempBuffs", "BuyCost", "BUY {0} CC"), FText::AsNumber(SingleUseBuffCost));
		const bool bCanAffordPurchase = ChadCouponBalance >= SingleUseBuffCost;

		TSharedPtr<FSlateBrush> Brush = T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, StatType, FVector2D(56.f, 56.f));
		BuffIconBrushes.Add(Brush);

		const TSharedRef<SWidget> IconWidget = Brush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[
					SNew(SImage)
					.Image(Brush.Get())
				])
			: StaticCastSharedRef<SWidget>(SNew(SSpacer));

		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		Grid->AddSlot(Col, Row)
			.Padding(Col < Columns - 1 ? FMargin(0.f, 0.f, CardGap, CardGap) : FMargin(0.f, 0.f, 0.f, CardGap))
			[
				SNew(SBox)
				.WidthOverride(CardWidth)
				.HeightOverride(CardHeight)
				[
					MakeTempBuffSpritePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 6.f)
						[
							SNew(SBox)
							.WidthOverride(48.f)
							.HeightOverride(48.f)
							[
								IconWidget
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(NameText)
							.Font(FT66Style::Tokens::FontBold(9))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f, 6.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(DescText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(OwnedCountText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(AssignedCountText)
							.Font(FT66Style::Tokens::FontRegular(8))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							MakeTempBuffSpriteButton(
								ActionText,
								FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleUseOrBuyBuffForFocusedLoadoutSlot, StatType),
								(bFocusedSlotMatches || bCanUseOwnedCopy) ? ET66TempBuffButtonFamily::ToggleOn : ET66TempBuffButtonFamily::CompactNeutral,
								0.f,
								34.f,
								9,
								bCanUseOwnedCopy || bCanAffordPurchase)
						],
						GetTempBuffCardShellBrush(bFocusedSlotMatches, !(bCanUseOwnedCopy || bCanAffordPurchase)),
						FMargin(10.f),
						bFocusedSlotMatches ? FLinearColor(0.20f, 0.28f, 0.20f, 1.0f) : FT66Style::Tokens::Panel)
				]
			];
	}

	const TSharedRef<SWidget> Content = AllBuffs.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			.ScrollBarStyle(GetTempBuffReferenceScrollBarStyle())
			.ScrollBarVisibility(EVisibility::Visible)
			.ScrollBarThickness(FVector2D(14.f, 14.f))
			.ScrollBarPadding(FMargin(10.f, 0.f, 0.f, 0.f))
			+ SScrollBox::Slot()
			.Padding(0.f, 0.f, 36.f, 0.f)
			[
				Grid
			])
		: StaticCastSharedRef<SWidget>(
			MakeTempBuffSpritePanel(
				SNew(STextBlock)
				.Text(EmptyText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				,
				GetTempBuffRowShellBrush(),
				FMargin(20.f),
				FT66Style::Tokens::Panel));

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.WidthOverride(ModalWidth)
			.HeightOverride(ModalHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeTempBuffSpritePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(TitleText)
								.Font(FT66Style::Tokens::FontBold(24))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 2.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(SlotHintText)
								.Font(FT66Style::Tokens::FontRegular(11))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							MakeTempBuffSpritePanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
									.Text(CouponsHeaderText)
									.Font(FT66Style::Tokens::FontBold(10))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.f, 4.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(CouponsValueText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								],
								GetTempBuffRowShellBrush(),
								FMargin(14.f, 10.f),
								FLinearColor(0.f, 0.f, 0.f, 0.92f))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 8.f, 0.f, 18.f)
					[
						LoadoutSlotsRow
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						Content
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 18.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.36f)
						.Padding(0.f, 0.f, 12.f, 0.f)
						[
							MakePartyBox()
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 12.f, 0.f)
						[
							MakeTempBuffSpriteButton(
								DoneText,
								FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleDoneClicked),
								ET66TempBuffButtonFamily::CompactNeutral,
								150.f,
								42.f,
								12)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.64f)
						[
							MakeRunControls()
						]
					]
					,
					GetTempBuffContentShellBrush(),
					FMargin(24.f, 8.f, 24.f, 18.f),
					FT66Style::Panel())
			]
		];
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotClicked(const int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		Buffs->SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
	}

	RefreshScreen();
	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotCleared(const int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->ClearSelectedSingleUseBuffSlot(SlotIndex))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotPurchased(const int32 SlotIndex)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseSelectedSingleUseBuffSlot(SlotIndex))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleUseOrBuyBuffForFocusedLoadoutSlot(const ET66SecondaryStatType StatType)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		const int32 FocusedSlotIndex = Buffs->GetSelectedSingleUseBuffEditSlotIndex();
		const TArray<ET66SecondaryStatType> Slots = Buffs->GetSelectedSingleUseBuffSlots();
		const ET66SecondaryStatType FocusedSlotStat = Slots.IsValidIndex(FocusedSlotIndex) ? Slots[FocusedSlotIndex] : ET66SecondaryStatType::None;
		const int32 OwnedCount = Buffs->GetOwnedSingleUseBuffCount(StatType);
		const int32 AssignedCount = Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType);
		const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
		const bool bCanUseOwnedCopy = OwnedCount > AssignedOutsideFocused;

		if (!bCanUseOwnedCopy && !Buffs->PurchaseSingleUseBuff(StatType))
		{
			return FReply::Handled();
		}

		if (Buffs->SetSelectedSingleUseBuffSlot(FocusedSlotIndex, StatType))
		{
			const TArray<ET66SecondaryStatType> UpdatedSlots = Buffs->GetSelectedSingleUseBuffSlots();
			for (int32 SlotIndex = 0; SlotIndex < UpdatedSlots.Num(); ++SlotIndex)
			{
				if (UpdatedSlots[SlotIndex] == ET66SecondaryStatType::None)
				{
					Buffs->SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
					break;
				}
			}
		}

		RefreshScreen();
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleDoneClicked()
{
	CloseModal();
	return FReply::Handled();
}

FReply UT66TemporaryBuffSelectionScreen::HandleEnterClicked()
{
	if (UT66HeroSelectionScreen* HeroSelectionScreen = GetLinkedHeroSelectionScreen())
	{
		CloseModal();
		HeroSelectionScreen->OnEnterTribulationClicked();
	}
	else
	{
		CloseModal();
	}

	return FReply::Handled();
}
