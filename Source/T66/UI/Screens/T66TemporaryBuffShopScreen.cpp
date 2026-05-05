// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TemporaryBuffShopScreen.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ET66BuffShopButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleInactive
	};

	enum class ET66BuffShopButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed
	};

	struct FT66BuffShopSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FT66BuffShopButtonBrushSet
	{
		FT66BuffShopSpriteBrushEntry Normal;
		FT66BuffShopSpriteBrushEntry Hover;
		FT66BuffShopSpriteBrushEntry Pressed;
		FT66BuffShopSpriteBrushEntry Disabled;
	};

	const FLinearColor T66BuffShopTextColor(0.953f, 0.925f, 0.835f, 1.0f);
	const FLinearColor T66BuffShopMutedTextColor(0.738f, 0.708f, 0.648f, 1.0f);
	const FLinearColor T66BuffShopFallbackPanel(0.025f, 0.023f, 0.034f, 0.97f);

	const FSlateBrush* ResolveBuffShopSpriteBrush(
		FT66BuffShopSpriteBrushEntry& Entry,
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
					TEXT("TempBuffShopReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* ResolveBuffShopSpriteRegionBrush(
		FT66BuffShopSpriteBrushEntry& Entry,
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
					TEXT("TempBuffShopReferenceRegionSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetBuffShopContentShellBrush()
	{
		static FT66BuffShopSpriteBrushEntry Entry;
		return ResolveBuffShopSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffShop/Panels/temporarybuffshop_panels_fullscreen_fullscreen_panel_wide.png"),
			FVector2D(1588.f, 653.f),
			FMargin(0.060f, 0.090f, 0.060f, 0.105f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetBuffShopCardShellBrush()
	{
		static FT66BuffShopSpriteBrushEntry Entry;
		return ResolveBuffShopSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffShop/Panels/temporarybuffshop_panels_fullscreen_fullscreen_panel_tall.png"),
			FVector2D(208.f, 188.f),
			FMargin(0.115f, 0.055f, 0.115f, 0.055f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
	}

	const FScrollBarStyle* GetBuffShopReferenceScrollBarStyle()
	{
		static FScrollBarStyle Style = FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar");
		static FT66BuffShopSpriteBrushEntry TrackEntry;
		static FT66BuffShopSpriteBrushEntry ThumbEntry;
		static FT66BuffShopSpriteBrushEntry HoverEntry;

		const FString ControlsPath = TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffShop/Controls/temporarybuffshop_controls_controls_sheet.png");
		const FBox2f VerticalBarUV(
			FVector2f(4.f / 1350.f, 4.f / 926.f),
			FVector2f(90.f / 1350.f, 644.f / 926.f));

		const FSlateBrush* TrackBrush = ResolveBuffShopSpriteRegionBrush(
			TrackEntry,
			ControlsPath,
			FVector2D(14.f, 120.f),
			FMargin(0.42f, 0.085f, 0.42f, 0.085f),
			VerticalBarUV,
			FLinearColor(0.35f, 0.34f, 0.30f, 0.70f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* ThumbBrush = ResolveBuffShopSpriteRegionBrush(
			ThumbEntry,
			ControlsPath,
			FVector2D(16.f, 96.f),
			FMargin(0.38f, 0.115f, 0.38f, 0.115f),
			VerticalBarUV,
			FLinearColor(0.93f, 0.82f, 0.52f, 1.0f),
			ESlateBrushDrawType::Box,
			TextureFilter::TF_Nearest);
		const FSlateBrush* HoverBrush = ResolveBuffShopSpriteRegionBrush(
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

	FString GetBuffShopButtonPath(const ET66BuffShopButtonFamily Family, const ET66BuffShopButtonState State)
	{
		const TCHAR* Suffix = TEXT("normal");
		if (Family == ET66BuffShopButtonFamily::ToggleOn && State == ET66BuffShopButtonState::Normal)
		{
			Suffix = TEXT("selected");
		}
		else if (State == ET66BuffShopButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66BuffShopButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}

		return T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath(TEXT("Pill"), Suffix);
	}

	FVector2D GetBuffShopButtonSize(const ET66BuffShopButtonFamily Family, const ET66BuffShopButtonState State)
	{
		if (Family == ET66BuffShopButtonFamily::ToggleOn)
		{
			return State == ET66BuffShopButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		if (Family == ET66BuffShopButtonFamily::ToggleInactive)
		{
			return State == ET66BuffShopButtonState::Hovered ? FVector2D(186.f, 69.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66BuffShopButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66BuffShopButtonBrushSet& GetBuffShopButtonBrushSet(const ET66BuffShopButtonFamily Family)
	{
		static FT66BuffShopButtonBrushSet CompactNeutral;
		static FT66BuffShopButtonBrushSet ToggleOn;
		static FT66BuffShopButtonBrushSet ToggleInactive;

		if (Family == ET66BuffShopButtonFamily::ToggleOn)
		{
			return ToggleOn;
		}
		if (Family == ET66BuffShopButtonFamily::ToggleInactive)
		{
			return ToggleInactive;
		}
		return CompactNeutral;
	}

	const FSlateBrush* GetBuffShopButtonBrush(const ET66BuffShopButtonFamily Family, const ET66BuffShopButtonState State)
	{
		FT66BuffShopButtonBrushSet& Set = GetBuffShopButtonBrushSet(Family);
		FT66BuffShopSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66BuffShopButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66BuffShopButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}

		return ResolveBuffShopSpriteBrush(
			*Entry,
			GetBuffShopButtonPath(Family, State),
			GetBuffShopButtonSize(Family, State),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	const FSlateBrush* GetBuffShopDisabledButtonBrush()
	{
		FT66BuffShopButtonBrushSet& Set = GetBuffShopButtonBrushSet(ET66BuffShopButtonFamily::ToggleInactive);
		return ResolveBuffShopSpriteBrush(
			Set.Disabled,
			TEXT("SourceAssets/UI/Reference/Screens/TemporaryBuffShop/Buttons/temporarybuffshop_buttons_pill_disabled.png"),
			FVector2D(180.f, 69.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			TextureFilter::TF_Nearest);
	}

	TSharedRef<SWidget> MakeBuffShopSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor = T66BuffShopFallbackPanel)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeBuffShopSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66BuffShopButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize,
		const TAttribute<bool>& IsEnabled = true)
	{
		const FSlateBrush* NormalBrush = GetBuffShopButtonBrush(Family, ET66BuffShopButtonState::Normal);
		const FSlateBrush* HoverBrush = GetBuffShopButtonBrush(Family, ET66BuffShopButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetBuffShopButtonBrush(Family, ET66BuffShopButtonState::Pressed);
		const FSlateBrush* DisabledBrush = GetBuffShopDisabledButtonBrush();
		if (!NormalBrush)
		{
			return FT66Style::MakeButton(
				FT66ButtonParams(Label, OnClicked, Family == ET66BuffShopButtonFamily::ToggleOn ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetFontSize(FontSize)
				.SetEnabled(IsEnabled));
		}

		const TAttribute<FSlateColor> TextColorAttr = TAttribute<FSlateColor>::CreateLambda([IsEnabled]() -> FSlateColor
			{
				return IsEnabled.Get() ? FSlateColor(T66BuffShopTextColor) : FSlateColor(T66BuffShopMutedTextColor);
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
}

UT66TemporaryBuffShopScreen::UT66TemporaryBuffShopScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TemporaryBuffShop;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66TemporaryBuffShopScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66BuffSubsystem* UT66TemporaryBuffShopScreen::GetBuffSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66BuffSubsystem>();
	}
	return nullptr;
}

void UT66TemporaryBuffShopScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66TemporaryBuffShopScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66BuffSubsystem* Buffs = GetBuffSubsystem();
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	}

	const float ModalWidth = 1880.0f;
	const float ModalHeight = 1058.0f;
	const int32 Columns = 5;
	const float CardGap = 10.0f;

	const FText TitleText = NSLOCTEXT("T66.TempBuffShop", "Title", "TEMP BUFF SHOP");
	const FText BackText = NSLOCTEXT("T66.TempBuffShop", "BackToBuffs", "BACK TO BUFFS");
	const FText HintText = NSLOCTEXT("T66.TempBuffShop", "Hint", "Buy stackable temporary buffs here. Owned buffs stay in your inventory until you select up to 4 total on Hero Selection.");
	const FText BonusText = NSLOCTEXT("T66.TempBuffShop", "Bonus", "+10% when selected");
	const int32 Balance = Buffs ? Buffs->GetChadCouponBalance() : 0;
	const int32 Cost = Buffs ? Buffs->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
	const FText BalanceText = FText::Format(NSLOCTEXT("T66.TempBuffShop", "Balance", "{0} CC"), FText::AsNumber(Balance));

	const TArray<ET66SecondaryStatType> AllBuffs = UT66BuffSubsystem::GetAllSingleUseBuffTypes();
	BuffIconBrushes.Reset();
	BuffIconBrushes.Reserve(AllBuffs.Num());

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int32 Index = 0; Index < AllBuffs.Num(); ++Index)
	{
		const ET66SecondaryStatType StatType = AllBuffs[Index];
		const int32 OwnedCount = Buffs ? Buffs->GetOwnedSingleUseBuffCount(StatType) : 0;
		const FText NameText = Loc ? Loc->GetText_SecondaryStatName(StatType) : FText::FromString(TEXT("?"));
		const FText OwnedText = FText::Format(NSLOCTEXT("T66.TempBuffShop", "OwnedCount", "Owned: {0}"), FText::AsNumber(OwnedCount));
		const FText ButtonText = FText::Format(NSLOCTEXT("T66.TempBuffShop", "BuyCost", "BUY {0} CC"), FText::AsNumber(Cost));

		TSharedPtr<FSlateBrush> Brush = T66TemporaryBuffUI::CreateSecondaryBuffBrush(TexPool, this, StatType, FVector2D(52.f, 52.f));
		BuffIconBrushes.Add(Brush);

		const int32 Row = Index / Columns;
		const int32 Col = Index % Columns;
		Grid->AddSlot(Col, Row)
			.Padding(Col < Columns - 1 ? FMargin(0.f, 0.f, CardGap, CardGap) : FMargin(0.f, 0.f, 0.f, CardGap))
			[
				SNew(SBox)
				.WidthOverride(230.f)
				.HeightOverride(196.f)
				[
					MakeBuffShopSpritePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(SBox)
						.WidthOverride(52.f)
						.HeightOverride(52.f)
						[
							Brush.IsValid()
							? StaticCastSharedRef<SWidget>(
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								[
									SNew(SImage).Image(Brush.Get())
								])
							: StaticCastSharedRef<SWidget>(SNew(SSpacer))
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
					.AutoHeight()
					.Padding(0.f, 3.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(BonusText)
						.Font(FT66Style::Tokens::FontRegular(8))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(OwnedText)
						.Font(FT66Style::Tokens::FontRegular(8))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						MakeBuffShopSpriteButton(
							ButtonText,
							FOnClicked::CreateUObject(this, &UT66TemporaryBuffShopScreen::HandlePurchaseClicked, StatType),
							Balance >= Cost ? ET66BuffShopButtonFamily::ToggleOn : ET66BuffShopButtonFamily::ToggleInactive,
							0.f,
							36.f,
							10,
							Balance >= Cost)
					],
						GetBuffShopCardShellBrush(),
						FMargin(10.f),
						FT66Style::Tokens::Panel)
				]
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.009f, 0.011f, 0.016f, 1.0f))
		[
			SNew(SBox)
			.WidthOverride(ModalWidth)
			.HeightOverride(ModalHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeBuffShopSpritePanel(
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
								.Font(FT66Style::Tokens::FontBold(22))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 4.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(BalanceText)
								.Font(FT66Style::Tokens::FontRegular(10))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							MakeBuffShopSpriteButton(
								BackText,
								FOnClicked::CreateUObject(this, &UT66TemporaryBuffShopScreen::HandleBackToBuffsClicked),
								ET66BuffShopButtonFamily::CompactNeutral,
								170.f,
								38.f,
								12)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 12.f, 0.f, 12.f)
					[
						SNew(STextBlock)
						.Text(HintText)
						.Font(FT66Style::Tokens::FontRegular(10))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SScrollBox)
						.ScrollBarStyle(GetBuffShopReferenceScrollBarStyle())
						.ScrollBarVisibility(EVisibility::Visible)
						.ScrollBarThickness(FVector2D(14.f, 14.f))
						.ScrollBarPadding(FMargin(10.f, 0.f, 0.f, 0.f))
						+ SScrollBox::Slot()
						[
							Grid
						]
					],
					GetBuffShopContentShellBrush(),
					FMargin(24.f, 20.f),
					FT66Style::Panel())
			]
		];
}

FReply UT66TemporaryBuffShopScreen::HandlePurchaseClicked(ET66SecondaryStatType StatType)
{
	if (UT66BuffSubsystem* Buffs = GetBuffSubsystem())
	{
		if (Buffs->PurchaseSingleUseBuff(StatType))
		{
			RefreshScreen();
		}
	}

	return FReply::Handled();
}

FReply UT66TemporaryBuffShopScreen::HandleBackToBuffsClicked()
{
	ShowModal(ET66ScreenType::TemporaryBuffSelection);
	return FReply::Handled();
}
