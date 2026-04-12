// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TemporaryBuffShopScreen.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

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

	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float ModalWidth = FMath::Min(SafeFrameSize.X * 0.90f, 1160.0f);
	const float ModalHeight = FMath::Min(SafeFrameSize.Y * 0.90f, 680.0f);
	const int32 Columns = 5;
	const float CardGap = 10.0f;

	const FText TitleText = NSLOCTEXT("T66.TempBuffShop", "Title", "TEMP BUFF SHOP");
	const FText BackText = NSLOCTEXT("T66.TempBuffShop", "BackToBuffs", "BACK TO BUFFS");
	const FText HintText = NSLOCTEXT("T66.TempBuffShop", "Hint", "Buy stackable temporary buffs here. Owned buffs stay in your inventory until you select up to 5 total on Hero Selection.");
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
				FT66Style::MakePanel(
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
						FT66Style::MakeButton(
							FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffShopScreen::HandlePurchaseClicked, StatType), ET66ButtonType::Primary)
							.SetMinWidth(0.f)
							.SetHeight(34.f)
							.SetFontSize(10)
							.SetColor(Balance >= Cost ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2)
							.SetEnabled(Balance >= Cost))
					],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FT66Style::Tokens::Panel)
						.SetPadding(FMargin(8.f)))
			];
	}

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
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Panel())
				.Padding(FMargin(24.f, 20.f))
				[
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
							FT66Style::MakeButton(
								FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffShopScreen::HandleBackToBuffsClicked), ET66ButtonType::Neutral)
								.SetMinWidth(170.f)
								.SetHeight(38.f)
								.SetFontSize(12))
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
						+ SScrollBox::Slot()
						[
							Grid
						]
					]
				]
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
