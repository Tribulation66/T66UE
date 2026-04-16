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
#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Style/T66Style.h"
#include "UI/T66TemporaryBuffUIUtils.h"
#include "UI/T66UIManager.h"
#include "Widgets/Images/SImage.h"
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
	const int32 Columns = SafeFrameSize.X >= 1700.f ? 6 : 5;
	const float CardGap = 12.0f;
	const float CardWidth = (ModalWidth - 88.0f - CardGap * static_cast<float>(Columns - 1)) / static_cast<float>(Columns);
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
		const FLinearColor ShellColor = bFocused ? FLinearColor(0.22f, 0.29f, 0.36f, 1.0f) : FT66Style::Tokens::Panel;
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
				.ColorAndOpacity(FT66Style::Tokens::TextMuted));

		const TSharedRef<SWidget> SlotAction = !bFilled
			? StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.HeightOverride(24.f))
			: StaticCastSharedRef<SWidget>(
				FT66Style::MakeButton(
					FT66ButtonParams(
						bOwnedForSlot ? ClearSlotText : BuySlotText,
						bOwnedForSlot
							? FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotCleared, SlotIndex)
							: FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotPurchased, SlotIndex),
						bOwnedForSlot ? ET66ButtonType::Neutral : ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(26.f)
					.SetFontSize(9)
					.SetPadding(FMargin(8.f, 5.f, 8.f, 4.f))
					.SetColor(bOwnedForSlot ? FT66Style::Tokens::Panel2 : FT66Style::Tokens::Accent)));

		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleLoadoutSlotClicked, SlotIndex), ET66ButtonType::Neutral)
					.SetMinWidth(0.f)
					.SetHeight(72.f)
					.SetPadding(FMargin(0.f))
					.SetColor(InnerColor)
					.SetContent(
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(InnerColor)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SlotContent
						]))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				SlotAction
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(ShellColor)
				.SetPadding(FMargin(4.f, 4.f, 4.f, 4.f)));
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
						FT66Style::MakeSlotFrame(
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								bHasAvatar
									? StaticCastSharedRef<SWidget>(
										SNew(SImage)
										.Image(FooterPartyAvatarBrushes[PartyIndex].Get()))
									: StaticCastSharedRef<SWidget>(
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.12f, 0.13f, 0.15f, 1.f))
										[
											SNew(STextBlock)
											.Text(FText::FromString(FallbackText))
											.Font(FT66Style::Tokens::FontBold(10))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
											.Justification(ETextJustify::Center)
										])
							],
							PartyIndex == 0 ? FLinearColor(0.29f, 0.24f, 0.13f, 1.0f) : FLinearColor(0.15f, 0.17f, 0.19f, 1.0f),
							FMargin(1.f))
					]
				];
		}

		return FT66Style::MakePanel(
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
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.95f))
				.SetPadding(FMargin(12.f, 10.f)));
	};

	auto MakeRunControls = [this, Loc, SelectedDifficulty, EnterText]() -> TSharedRef<SWidget>
	{
		auto MakeDifficultyMenu = [this, Loc]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
			for (const ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
			{
				Box->AddSlot()
					.AutoHeight()
					[
						FT66Style::MakeButton(
							FT66ButtonParams(
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
								ET66ButtonType::Neutral)
							.SetMinWidth(150.f)
							.SetHeight(34.f)
							.SetFontSize(11))
					];
			}
			return Box;
		};

		return FT66Style::MakePanel(
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
				FT66Style::MakeButton(
					FT66ButtonParams(EnterText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleEnterClicked), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(40.f)
					.SetFontSize(11))
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.95f))
				.SetPadding(FMargin(12.f, 10.f)));
	};

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
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 6.f)
						[
							SNew(SBox)
							.WidthOverride(56.f)
							.HeightOverride(56.f)
							[
								IconWidget
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(NameText)
							.Font(FT66Style::Tokens::FontBold(10))
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
							.Font(FT66Style::Tokens::FontRegular(9))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(OwnedCountText)
							.Font(FT66Style::Tokens::FontRegular(9))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(AssignedCountText)
							.Font(FT66Style::Tokens::FontRegular(9))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(ActionText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleUseOrBuyBuffForFocusedLoadoutSlot, StatType), bCanUseOwnedCopy ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
								.SetMinWidth(0.f)
								.SetHeight(42.f)
								.SetFontSize(10)
								.SetEnabled(bCanUseOwnedCopy || bCanAffordPurchase)
								.SetColor(bFocusedSlotMatches ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2)
								.SetContent(
									SNew(STextBlock)
									.Text(ActionText)
									.Font(FT66Style::Tokens::FontBold(10))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.AutoWrapText(true)
									.Justification(ETextJustify::Center)))
						],
						FT66PanelParams(ET66PanelType::Panel)
							.SetColor(bFocusedSlotMatches ? FLinearColor(0.20f, 0.28f, 0.20f, 1.0f) : FT66Style::Tokens::Panel)
							.SetPadding(FMargin(10.f)))
				]
			];
	}

	const TSharedRef<SWidget> Content = AllBuffs.Num() > 0
		? StaticCastSharedRef<SWidget>(
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				Grid
			])
		: StaticCastSharedRef<SWidget>(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(20.f))
			[
				SNew(STextBlock)
				.Text(EmptyText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]);

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
				.Padding(FMargin(24.f, 20.f, 24.f, 18.f))
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
								.Font(FT66Style::Tokens::FontBold(24))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 10.f, 0.f, 0.f)
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
							FT66Style::MakePanel(
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
								FT66PanelParams(ET66PanelType::Panel)
									.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.92f))
									.SetPadding(FMargin(14.f, 10.f)))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 16.f, 0.f, 18.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeLoadoutSlotWidget(0)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeLoadoutSlotWidget(1)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeLoadoutSlotWidget(2)]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeLoadoutSlotWidget(3)]
						+ SHorizontalBox::Slot().AutoWidth()[MakeLoadoutSlotWidget(4)]
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
							FT66Style::MakeButton(
								FT66ButtonParams(DoneText, FOnClicked::CreateUObject(this, &UT66TemporaryBuffSelectionScreen::HandleDoneClicked), ET66ButtonType::Neutral)
								.SetMinWidth(150.f)
								.SetHeight(42.f)
								.SetFontSize(12))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.64f)
						[
							MakeRunControls()
						]
					]
				]
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
