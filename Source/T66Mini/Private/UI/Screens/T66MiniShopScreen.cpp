// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniShopScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UITypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FString T66MiniDescribeItemEffect(const FT66MiniItemDefinition& Item)
	{
		static const TMap<FString, FString> EffectLabels = {
			{ TEXT("Accuracy"), TEXT("+ accuracy and tighter targeting") },
			{ TEXT("AoeDamage"), TEXT("+ area damage") },
			{ TEXT("AoeScale"), TEXT("+ blast radius") },
			{ TEXT("AoeSpeed"), TEXT("+ area attack speed") },
			{ TEXT("Assassinate"), TEXT("+ execute chance") },
			{ TEXT("AttackRange"), TEXT("+ range") },
			{ TEXT("BounceDamage"), TEXT("+ bounce damage") },
			{ TEXT("BounceScale"), TEXT("+ bounce power") },
			{ TEXT("BounceSpeed"), TEXT("+ bounce speed") },
			{ TEXT("Cheating"), TEXT("+ fail-safe health") },
			{ TEXT("CounterAttack"), TEXT("+ retaliation chance") },
			{ TEXT("CritChance"), TEXT("+ crit chance") },
			{ TEXT("CritDamage"), TEXT("+ crit damage") },
			{ TEXT("Crush"), TEXT("+ crush chance") },
			{ TEXT("DamageReduction"), TEXT("+ armor") },
			{ TEXT("DotDamage"), TEXT("+ DOT damage") },
			{ TEXT("DotScale"), TEXT("+ DOT potency") },
			{ TEXT("DotSpeed"), TEXT("+ DOT attack speed") },
			{ TEXT("EvasionChance"), TEXT("+ evade chance") },
			{ TEXT("HpRegen"), TEXT("+ regen") },
			{ TEXT("Invisibility"), TEXT("+ invis chance") },
			{ TEXT("LifeSteal"), TEXT("+ life steal") },
			{ TEXT("LootCrate"), TEXT("+ loot crate frequency") },
			{ TEXT("PierceDamage"), TEXT("+ pierce damage") },
			{ TEXT("PierceScale"), TEXT("+ pierce count") },
			{ TEXT("PierceSpeed"), TEXT("+ pierce attack speed") },
			{ TEXT("ReflectDamage"), TEXT("+ reflect damage") },
			{ TEXT("Stealing"), TEXT("+ steal chance") },
			{ TEXT("Taunt"), TEXT("+ taunt") },
			{ TEXT("TreasureChest"), TEXT("+ chest frequency") }
		};

		if (const FString* Found = EffectLabels.Find(Item.SecondaryStatType))
		{
			return *Found;
		}

		return FString::Printf(TEXT("+ %s"), *Item.SecondaryStatType);
	}
}

UT66MiniShopScreen::UT66MiniShopScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniShop;
	bIsModal = false;
}

void UT66MiniShopScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (DataSubsystem && FrontendState && !FrontendState->ConsumeShouldSkipNextShopPrime())
	{
		FrontendState->PrimeShopOffers(DataSubsystem);
	}
}

TSharedRef<SWidget> UT66MiniShopScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;

	TSharedRef<SUniformGridPanel> OfferGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(12.f));
	TSharedRef<SVerticalBox> OwnedItemsPanel = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> AlchemyItemsPanel = SNew(SVerticalBox);
	if (DataSubsystem && FrontendState)
	{
		const TArray<FName>& OfferIDs = FrontendState->GetCurrentShopOfferIDs();
		for (int32 Index = 0; Index < OfferIDs.Num(); ++Index)
		{
			const FT66MiniItemDefinition* Item = DataSubsystem->FindItem(OfferIDs[Index]);
			if (!Item)
			{
				continue;
			}

			const bool bLocked = FrontendState->IsShopOfferLocked(Item->ItemID);
			const bool bAffordable = ActiveRun && ActiveRun->Gold >= Item->BaseBuyGold;
			const FLinearColor BuyColor = bAffordable ? T66MiniUI::AccentGreen() : T66MiniUI::RaisedFill();

			OfferGrid->AddSlot(Index % 2, Index / 2)
			[
				SNew(SBox)
				.WidthOverride(520.f)
				.HeightOverride(210.f)
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::PanelFill())
					.Padding(FMargin(18.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromName(Item->ItemID))
							.Font(T66MiniUI::BoldFont(22))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("%s  |  %s"), *Item->PrimaryStatType, *Item->SecondaryStatType)))
							.Font(T66MiniUI::BodyFont(16))
							.ColorAndOpacity(T66MiniUI::MutedText())
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(T66MiniDescribeItemEffect(*Item)))
							.Font(T66MiniUI::BodyFont(16))
							.ColorAndOpacity(FLinearColor::White)
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("Cost: %d gold"), Item->BaseBuyGold)))
							.Font(T66MiniUI::BoldFont(18))
							.ColorAndOpacity(T66MiniUI::AccentGold())
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.VAlign(VAlign_Bottom)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.Padding(0.f, 0.f, 10.f, 0.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateLambda([this, ItemID = Item->ItemID]()
								{
									return HandleBuyItemClicked(ItemID);
								}))
								.ButtonColorAndOpacity(BuyColor)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66Mini.Shop", "Buy", "BUY"))
									.Font(T66MiniUI::BoldFont(18))
									.ColorAndOpacity(bAffordable ? T66MiniUI::ButtonTextDark() : T66MiniUI::MutedText())
									.Justification(ETextJustify::Center)
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateLambda([this, ItemID = Item->ItemID]()
								{
									return HandleLockClicked(ItemID);
								}))
								.ButtonColorAndOpacity(bLocked ? T66MiniUI::AccentGold() : T66MiniUI::AccentBlue())
								[
									SNew(STextBlock)
									.Text(FText::FromString(bLocked ? TEXT("UNLOCK") : TEXT("LOCK")))
									.Font(T66MiniUI::BoldFont(18))
									.ColorAndOpacity(bLocked ? T66MiniUI::ButtonTextDark() : FLinearColor::White)
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				]
			];
		}
	}

	if (DataSubsystem && ActiveRun)
	{
		TMap<FName, int32> OwnedCounts;
		for (const FName ItemID : ActiveRun->OwnedItemIDs)
		{
			OwnedCounts.FindOrAdd(ItemID) += 1;
		}

		int32 RenderedOwnedRows = 0;
		for (const TPair<FName, int32>& Pair : OwnedCounts)
		{
			if (RenderedOwnedRows >= 6)
			{
				break;
			}

			const FT66MiniItemDefinition* Item = DataSubsystem->FindItem(Pair.Key);
			if (!Item)
			{
				continue;
			}

			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::RaisedFill())
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%s  x%d"), *Pair.Key.ToString(), Pair.Value)))
						.Font(T66MiniUI::BodyFont(16))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateLambda([this, ItemID = Pair.Key]()
						{
							return HandleSellItemClicked(ItemID);
						}))
						.ButtonColorAndOpacity(T66MiniUI::AccentGold())
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("SELL +%d"), Item->BaseSellGold)))
							.Font(T66MiniUI::BoldFont(15))
							.ColorAndOpacity(T66MiniUI::ButtonTextDark())
						]
					]
				]
			];

			AlchemyItemsPanel->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::RaisedFill())
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%s  x%d  |  ready for future transmutation inputs"), *Pair.Key.ToString(), Pair.Value)))
					.Font(T66MiniUI::BodyFont(15))
					.ColorAndOpacity(FLinearColor::White)
					.AutoWrapText(true)
				]
			];

			++RenderedOwnedRows;
		}

		if (RenderedOwnedRows == 0)
		{
			OwnedItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Shop", "NoOwnedItems", "No owned items yet. Buy a few upgrades and they will appear here with sell values."))
				.Font(T66MiniUI::BodyFont(16))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];

			AlchemyItemsPanel->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Shop", "NoAlchemyItems", "Mini alchemy uses owned items as inputs. As soon as you buy or loot items, they will appear here for future recipes."))
				.Font(T66MiniUI::BodyFont(16))
				.ColorAndOpacity(T66MiniUI::MutedText())
				.AutoWrapText(true)
			];
		}
	}

	const int32 RerollCost = FrontendState ? (12 + (FrontendState->GetShopRerollCount() * 6)) : 12;

	TSharedRef<SWidget> ActiveTabContent = SNullWidget::NullWidget;
	FText DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusVendor", "Buy mini-specific item upgrades, sell old pieces, reroll the vendor, then continue to idol selection for the next wave.");
	if (ActiveTab == EMiniCircusTab::Vendor)
	{
		ActiveTabContent =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				OfferGrid
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::CardFill())
				.Padding(FMargin(16.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "OwnedItemsTitle", "OWNED ITEMS"))
						.Font(T66MiniUI::BoldFont(20))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						OwnedItemsPanel
					]
				]
			];
	}
	else if (ActiveTab == EMiniCircusTab::Gambling)
	{
		DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusGambling", "The mini circus shell now exposes every gambling game from the main flow. Individual game logic can be wired on top of this tab without changing the stage-transition flow again.");
		TSharedRef<SUniformGridPanel> GamblingGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(12.f));
		struct FMiniCircusGameCard
		{
			const TCHAR* Title;
			const TCHAR* Description;
		};

		static const FMiniCircusGameCard GameCards[] = {
			{ TEXT("COIN FLIP"), TEXT("Heads or tails with quick gold swings.") },
			{ TEXT("ROCK PAPER SCISSORS"), TEXT("The demon hand mind-game stays in the lineup.") },
			{ TEXT("BLACK JACK"), TEXT("Keep the full card-table lane in the mini branch.") },
			{ TEXT("LOTTERY"), TEXT("Five-pick number draw preserved for the circus tab.") },
			{ TEXT("PLINKO"), TEXT("Drop-and-bounce risk curve still has a dedicated card.") },
			{ TEXT("BOX OPENING"), TEXT("Fast high-variance pulls stay exposed in mini too.") }
		};

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(GameCards); ++Index)
		{
			const FMiniCircusGameCard& Card = GameCards[Index];
			GamblingGrid->AddSlot(Index % 3, Index / 3)
			[
				SNew(SBox)
				.WidthOverride(340.f)
				.HeightOverride(188.f)
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::PanelFill())
					.Padding(FMargin(18.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Card.Title))
							.Font(T66MiniUI::BoldFont(22))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Card.Description))
							.Font(T66MiniUI::BodyFont(16))
							.ColorAndOpacity(T66MiniUI::MutedText())
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Bottom)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateLambda([this, Title = FString(Card.Title)]()
							{
								SetStatus(FText::FromString(FString::Printf(TEXT("%s is staged in the mini circus shell. The dedicated game interaction pass can now land on this slot without replacing the intermission screen again."), *Title)));
								return FReply::Handled();
							}))
							.ButtonColorAndOpacity(T66MiniUI::AccentGold())
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Mini.Shop", "OpenGame", "OPEN"))
								.Font(T66MiniUI::BoldFont(18))
								.ColorAndOpacity(T66MiniUI::ButtonTextDark())
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			];
		}

		ActiveTabContent = GamblingGrid;
	}
	else
	{
		DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusAlchemy", "Mini alchemy now has a dedicated tab in the stage vendor flow. Item-transmutation rules can iterate here later without changing the shell or save-hand-off again.");
		ActiveTabContent =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::PanelFill())
				.Padding(FMargin(18.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "AlchemyTitle", "ALCHEMY INPUTS"))
						.Font(T66MiniUI::BoldFont(22))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 8.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "AlchemyBody", "This tab now owns the mini-specific alchemy lane. The current pass stages the shell and surfaces the player inventory so recipe wiring can land without replacing the screen structure again."))
						.Font(T66MiniUI::BodyFont(16))
						.ColorAndOpacity(T66MiniUI::MutedText())
						.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 12.f, 0.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(T66MiniUI::WhiteBrush())
				.BorderBackgroundColor(T66MiniUI::CardFill())
				.Padding(FMargin(16.f))
				[
					AlchemyItemsPanel
				]
			];
	}

	const auto MakeTabButton = [this](const EMiniCircusTab Tab, const FText& Label, const FLinearColor& ActiveColor) -> TSharedRef<SWidget>
	{
		const bool bIsActive = ActiveTab == Tab;
		const FOnClicked OnClicked = (Tab == EMiniCircusTab::Vendor)
			? FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleVendorTabClicked)
			: (Tab == EMiniCircusTab::Gambling)
				? FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleGamblingTabClicked)
				: FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyTabClicked);

		return SNew(SButton)
			.OnClicked(OnClicked)
			.ButtonColorAndOpacity(bIsActive ? ActiveColor : T66MiniUI::RaisedFill())
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(T66MiniUI::BoldFont(18))
				.ColorAndOpacity(bIsActive ? T66MiniUI::ButtonTextDark() : FLinearColor::White)
				.Justification(ETextJustify::Center)
			];
	};

	TSharedRef<SWidget> BottomLeftWidget = SNew(SBorder)
		.BorderImage(T66MiniUI::WhiteBrush())
		.BorderBackgroundColor(T66MiniUI::RaisedFill())
		.Padding(FMargin(12.f))
		[
			SNew(STextBlock)
			.Text(ActiveTab == EMiniCircusTab::Gambling
				? NSLOCTEXT("T66Mini.Shop", "GamblingHint", "GAMES STAGED")
				: NSLOCTEXT("T66Mini.Shop", "AlchemyHint", "ALCHEMY STAGED"))
			.Font(T66MiniUI::BoldFont(16))
			.ColorAndOpacity(T66MiniUI::MutedText())
			.Justification(ETextJustify::Center)
		];
	if (ActiveTab == EMiniCircusTab::Vendor)
	{
		BottomLeftWidget =
			SNew(SButton)
			.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleRerollClicked))
			.ButtonColorAndOpacity(T66MiniUI::AccentGold())
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("REROLL (%dG)"), RerollCost)))
				.Font(T66MiniUI::BoldFont(18))
				.ColorAndOpacity(T66MiniUI::ButtonTextDark())
				.Justification(ETextJustify::Center)
			];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::ShellFill())
			.Padding(FMargin(28.f, 22.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 18.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.Shop", "Title", "Mini Circus"))
					.Font(T66MiniUI::TitleFont(34))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 14.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(SBorder)
						.BorderImage(T66MiniUI::WhiteBrush())
						.BorderBackgroundColor(T66MiniUI::PanelFill())
						.Padding(FMargin(12.f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(ActiveRun
								? FString::Printf(TEXT("Gold: %d  |  Materials: %d  |  Owned Items: %d  |  Wave Complete: %d"), ActiveRun->Gold, ActiveRun->Materials, ActiveRun->OwnedItemIDs.Num(), ActiveRun->WaveIndex)
								: TEXT("Mini run unavailable.")))
							.Font(T66MiniUI::BoldFont(18))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 14.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
					[
						MakeTabButton(EMiniCircusTab::Vendor, NSLOCTEXT("T66Mini.Shop", "VendorTab", "VENDOR"), T66MiniUI::AccentGreen())
					]
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
					[
						MakeTabButton(EMiniCircusTab::Gambling, NSLOCTEXT("T66Mini.Shop", "GamblingTab", "GAMBLING"), T66MiniUI::AccentGold())
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						MakeTabButton(EMiniCircusTab::Alchemy, NSLOCTEXT("T66Mini.Shop", "AlchemyTab", "ALCHEMY"), T66MiniUI::AccentBlue())
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.Padding(0.f, 0.f, 0.f, 14.f)
				[
					ActiveTabContent
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 16.f, 0.f, 0.f)
				[
					SAssignNew(StatusTextBlock, STextBlock)
					.Text(DefaultStatus)
					.Font(T66MiniUI::BodyFont(16))
					.ColorAndOpacity(T66MiniUI::MutedText())
					.AutoWrapText(true)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(28.f, 0.f, 0.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(220.f)
			.HeightOverride(48.f)
			[
				BottomLeftWidget
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0.f, 0.f, 28.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(220.f)
			.HeightOverride(54.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleContinueClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.Shop", "Continue", "CONTINUE"))
					.Font(T66MiniUI::BoldFont(20))
					.ColorAndOpacity(T66MiniUI::ButtonTextDark())
					.Justification(ETextJustify::Center)
				]
			]
		];
}

FReply UT66MiniShopScreen::HandleVendorTabClicked()
{
	ActiveTab = EMiniCircusTab::Vendor;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleGamblingTabClicked()
{
	ActiveTab = EMiniCircusTab::Gambling;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleAlchemyTabClicked()
{
	ActiveTab = EMiniCircusTab::Alchemy;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleBuyItemClicked(const FName ItemID)
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const FT66MiniItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!FrontendState || !RunState || !SaveSubsystem || !ActiveRun || !Item)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "MissingSubsystems", "Mini shop purchase failed because the mini run state is unavailable."));
		return FReply::Handled();
	}

	if (ActiveRun->Gold < Item->BaseBuyGold)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "NotEnoughGold", "Not enough gold for that purchase."));
		return FReply::Handled();
	}

	ActiveRun->Gold -= Item->BaseBuyGold;
	ActiveRun->OwnedItemIDs.Add(Item->ItemID);
	FrontendState->ConsumeShopOffer(Item->ItemID);
	FrontendState->RefillShopOffers(DataSubsystem);
	FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	SetStatus(FText::FromString(FString::Printf(TEXT("%s purchased. The bonus will apply on the next mini battle load."), *ItemID.ToString())));
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleSellItemClicked(const FName ItemID)
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const FT66MiniItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!RunState || !SaveSubsystem || !ActiveRun || !Item)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "SellMissingState", "Mini shop selling failed because the mini run state is unavailable."));
		return FReply::Handled();
	}

	const int32 RemovedCount = ActiveRun->OwnedItemIDs.RemoveSingle(ItemID);
	if (RemovedCount <= 0)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "SellMissingItem", "That item is not currently owned."));
		return FReply::Handled();
	}

	ActiveRun->Gold += Item->BaseSellGold;
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	SetStatus(FText::FromString(FString::Printf(TEXT("%s sold for %d gold."), *ItemID.ToString(), Item->BaseSellGold)));
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleLockClicked(const FName ItemID)
{
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!FrontendState)
	{
		return FReply::Handled();
	}

	FrontendState->ToggleShopOfferLock(ItemID);
	if (SaveSubsystem && RunState && ActiveRun)
	{
		FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
		SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	}
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleRerollClicked()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!DataSubsystem || !FrontendState || !RunState || !SaveSubsystem || !ActiveRun)
	{
		return FReply::Handled();
	}

	const int32 RerollCost = 12 + (FrontendState->GetShopRerollCount() * 6);
	if (ActiveRun->Gold < RerollCost)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "NoRerollFunds", "Not enough gold to reroll the mini circus vendor."));
		return FReply::Handled();
	}

	ActiveRun->Gold -= RerollCost;
	FrontendState->RerollShopOffers(DataSubsystem);
	FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleContinueClicked()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!DataSubsystem || !FrontendState || !RunState || !SaveSubsystem || !ActiveRun)
	{
		return FReply::Handled();
	}

	FrontendState->RefreshIdolOffers(DataSubsystem);
	FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	NavigateTo(ET66ScreenType::MiniIdolSelect);
	return FReply::Handled();
}

void UT66MiniShopScreen::SetStatus(const FText& InText)
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(InText);
	}
}
