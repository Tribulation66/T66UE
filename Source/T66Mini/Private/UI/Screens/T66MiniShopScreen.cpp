// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniShopScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66MiniCircusSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "JsonObjectConverter.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FName MiniShopActionBuy(TEXT("Buy"));
	const FName MiniShopActionSteal(TEXT("Steal"));
	const FName MiniShopActionSell(TEXT("Sell"));
	const FName MiniShopActionBuyback(TEXT("Buyback"));
	const FName MiniShopActionToggleLock(TEXT("ToggleLock"));
	const FName MiniShopActionReroll(TEXT("Reroll"));
	const FName MiniShopActionBorrow(TEXT("Borrow"));
	const FName MiniShopActionPayDebt(TEXT("PayDebt"));
	const FName MiniShopActionCircusGame(TEXT("CircusGame"));
	const FName MiniShopActionAlchemyTransmute(TEXT("AlchemyTransmute"));
	const FName MiniShopActionAlchemyDissolve(TEXT("AlchemyDissolve"));

	template <typename PayloadType>
	bool T66MiniSerializePayload(const PayloadType& Payload, FString& OutJson)
	{
		return FJsonObjectConverter::UStructToJsonObjectString(Payload, OutJson);
	}

	template <typename PayloadType>
	bool T66MiniDeserializePayload(const FString& Json, PayloadType& OutPayload)
	{
		return !Json.IsEmpty()
			&& FJsonObjectConverter::JsonObjectStringToUStruct(Json, &OutPayload, 0, 0);
	}

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

	FString T66MiniReadableName(const FName Id)
	{
		FString Result = Id.ToString();
		Result.RemoveFromStart(TEXT("Item_"));
		Result.ReplaceInline(TEXT("_"), TEXT(" "));
		return Result;
	}

	TSharedPtr<FSlateBrush> T66MiniMakeBrush(const FVector2D& Size)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->ImageSize = Size;
		return Brush;
	}

	bool T66MiniBindBrush(
		UT66MiniShopScreen* Owner,
		UT66UITexturePoolSubsystem* TexPool,
		const FString& TexturePath,
		const TSharedPtr<FSlateBrush>& Brush,
		const FString& Key)
	{
		if (!Owner || !TexPool || !Brush.IsValid() || TexturePath.IsEmpty())
		{
			return false;
		}

		const FSoftObjectPath TextureObjectPath(TexturePath);
		const TSoftObjectPtr<UTexture2D> TextureSoft{TextureObjectPath};
		if (TextureSoft.IsNull())
		{
			return false;
		}

		UTexture2D* Texture = TexPool->GetLoadedTexture(TextureSoft);
		if (!Texture)
		{
			TWeakPtr<FSlateBrush> WeakBrush = Brush;
			TexPool->RequestTexture(
				TextureSoft,
				Owner,
				FName(*Key),
				[WeakBrush](UTexture2D* LoadedTexture)
				{
					if (LoadedTexture)
					{
						if (const TSharedPtr<FSlateBrush> PinnedBrush = WeakBrush.Pin())
						{
							PinnedBrush->SetResourceObject(LoadedTexture);
						}
					}
				});
			return false;
		}

		Brush->SetResourceObject(Texture);
		return true;
	}

	TSharedRef<SWidget> T66MiniMakeIconPanel(
		UT66MiniShopScreen* Owner,
		UT66UITexturePoolSubsystem* TexPool,
		const FString& TexturePath,
		const FString& BrushKey,
		const FText& FallbackText,
		const FVector2D& Size)
	{
		const TSharedPtr<FSlateBrush> Brush = T66MiniMakeBrush(Size);
		T66MiniBindBrush(Owner, TexPool, TexturePath, Brush, BrushKey);
		return FT66Style::MakePanel(
			SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([Brush]() -> const FSlateBrush*
					{
						return Brush.Get();
					})
					.Visibility_Lambda([Brush]() -> EVisibility
					{
						return (Brush.IsValid() && Brush->GetResourceObject() != nullptr) ? EVisibility::Visible : EVisibility::Collapsed;
					})
				]
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FallbackText)
					.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
					.ColorAndOpacity(FT66Style::TextMuted())
					.Visibility_Lambda([Brush]() -> EVisibility
					{
						return (Brush.IsValid() && Brush->GetResourceObject() != nullptr) ? EVisibility::Collapsed : EVisibility::Visible;
					})
					.Justification(ETextJustify::Center)
				]
			],
			FT66PanelParams(ET66PanelType::Panel2).SetPadding(0.f));
	}

	TSharedRef<SWidget> T66MiniMakeMetricChip(const FText& Label, const FText& Value, const FLinearColor& ValueColor)
	{
		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66Style::MakeFont(TEXT("Bold"), 9))
				.ColorAndOpacity(FT66Style::TextMuted())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Value)
				.Font(FT66Style::MakeFont(TEXT("Bold"), 12))
				.ColorAndOpacity(ValueColor)
			],
			FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(8.f, 6.f)));
	}

	TSharedRef<SWidget> T66MiniMakeInfoRow(const FText& Label, const FText& Value)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66Style::MakeFont(TEXT("Bold"), 10))
				.ColorAndOpacity(FT66Style::TextMuted())
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 0.f, 0.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Value)
				.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
				.ColorAndOpacity(FT66Style::Text())
				.AutoWrapText(true)
			];
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
	CurrentStatusText = FText::GetEmpty();
	LastAppliedStateRevision = 0;
	LastShopUiStateKey.Reset();
	ProcessedRequestRevisionByPlayerId.Reset();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66PartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66MiniShopScreen::HandlePartyStateChanged);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66MiniShopScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MiniShop, true);
		}
	}

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniCircusSubsystem* CircusSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	bool bIsHost = false;
	const bool bOnlineMiniParty = IsOnlineMiniParty(&bIsHost);
	if (DataSubsystem && CircusSubsystem)
	{
		CircusSubsystem->SeedFromRunSave(ActiveRun);
		CircusSubsystem->PrimeVendorOffers(DataSubsystem);
	}

	if (FrontendState)
	{
		FrontendState->ResumeIntermissionFlow();
	}

	ResetMiniIntermissionTransport();
	if (bOnlineMiniParty)
	{
		if (bIsHost)
		{
			PublishAuthoritativeState(
				NSLOCTEXT("T66Mini.Shop", "HostIntermissionReady", "Mini intermission is live. Circus changes now sync to the whole party."),
				true);
		}
		else
		{
			ApplyAuthoritativeStateFromLobby(false);
		}

		ShopSyncTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UT66MiniShopScreen::HandleShopSyncTicker),
			0.25f);
	}

	LastShopUiStateKey = BuildShopUiStateKey();
}

void UT66MiniShopScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66PartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	if (ShopSyncTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ShopSyncTickerHandle);
		ShopSyncTickerHandle.Reset();
	}

	PartyStateChangedHandle.Reset();
	SessionStateChangedHandle.Reset();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniShopScreen::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66PartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	if (ShopSyncTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(ShopSyncTickerHandle);
		ShopSyncTickerHandle.Reset();
	}

	PartyStateChangedHandle.Reset();
	SessionStateChangedHandle.Reset();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66MiniShopScreen::BuildSlateUI()
{
	LastShopUiStateKey = BuildShopUiStateKey();
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniCircusSubsystem* CircusSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	if (CircusSubsystem && RunState && ActiveRun)
	{
		const FT66MiniHeroDefinition* HeroDef = DataSubsystem ? DataSubsystem->FindHero(ActiveRun->HeroID) : nullptr;
		const FT66MiniCompanionDefinition* CompanionDef = DataSubsystem ? DataSubsystem->FindCompanion(ActiveRun->CompanionID) : nullptr;
		const FT66MiniDifficultyDefinition* DifficultyDef = DataSubsystem ? DataSubsystem->FindDifficulty(ActiveRun->DifficultyID) : nullptr;
		const FT66MiniItemDefinition* AlchemyItem = DataSubsystem ? DataSubsystem->FindItem(FName(TEXT("Item_Alchemy"))) : nullptr;
		const int32 Gold = ActiveRun->Gold;
		const int32 Materials = ActiveRun->Materials;
		const int32 Debt = CircusSubsystem->GetDebt();
		const int32 AngerPct = FMath::RoundToInt(CircusSubsystem->GetAnger01() * 100.f);
		const int32 RerollCost = 12 + (CircusSubsystem->GetVendorRerollCount() * 6);
		const float CardWidth = FT66Style::Tokens::NPCCompactShopCardWidth;
		const float CardHeight = FT66Style::Tokens::NPCCompactShopCardHeight;
		const float CardIconSize = CardWidth - 10.f;

		TSharedRef<SVerticalBox> OwnedItemsPanel = SNew(SVerticalBox);
		TMap<FName, int32> OwnedCounts;
		for (const FName ItemID : ActiveRun->OwnedItemIDs)
		{
			OwnedCounts.FindOrAdd(ItemID) += 1;
		}
		const FT66MiniItemDefinition* AlchemySourceItem = nullptr;
		if (DataSubsystem)
		{
			for (const TPair<FName, int32>& Pair : OwnedCounts)
			{
				AlchemySourceItem = DataSubsystem->FindItem(Pair.Key);
				if (AlchemySourceItem)
				{
					break;
				}
			}
		}
		if (OwnedCounts.Num() == 0)
		{
			OwnedItemsPanel->AddSlot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66Mini.Shop", "NoOwnedItems", "No owned items yet."))
				.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
				.ColorAndOpacity(FT66Style::TextMuted())
			];
		}
		else
		{
			for (const TPair<FName, int32>& Pair : OwnedCounts)
			{
				const FT66MiniItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(Pair.Key) : nullptr;
				if (!Item)
				{
					continue;
				}

				OwnedItemsPanel->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					FT66Style::MakePanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							T66MiniMakeIconPanel(this, TexPool, Item->IconPath, FString::Printf(TEXT("MiniOwned_%s"), *Item->ItemID.ToString()), FText::FromString(T66MiniReadableName(Item->ItemID).Left(2)), FVector2D(40.f, 40.f))
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 10.f, 0.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::Format(NSLOCTEXT("T66Mini.Shop", "OwnedFmt", "{0} x{1}"), FText::FromString(T66MiniReadableName(Item->ItemID)), FText::AsNumber(Pair.Value)))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
							.ColorAndOpacity(FT66Style::Text())
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									FText::Format(NSLOCTEXT("T66Mini.Shop", "SellFmt", "SELL +{0}"), FText::AsNumber(Item->BaseSellGold)),
									FOnClicked::CreateLambda([this, ItemID = Item->ItemID]() { return HandleSellItemClicked(ItemID); }),
									ET66ButtonType::Danger)
								.SetFontSize(9)
								.SetPadding(FMargin(8.f, 5.f))
								.SetMinWidth(0.f))
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(8.f)))
				];
			}
		}

		const auto MakeRunPanel = [&]() -> TSharedRef<SWidget>
		{
			return FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "RunTitle", "RUN")).Font(FT66Style::MakeFont(TEXT("Bold"), 16)).ColorAndOpacity(FT66Style::Text())]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "HeroLabel", "Hero"), HeroDef ? FText::FromString(HeroDef->DisplayName) : FText::FromName(ActiveRun->HeroID))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "CompanionLabel", "Companion"), CompanionDef ? FText::FromString(CompanionDef->DisplayName) : FText::FromName(ActiveRun->CompanionID))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "DifficultyLabel", "Difficulty"), DifficultyDef ? FText::FromString(DifficultyDef->DisplayName) : FText::FromName(ActiveRun->DifficultyID))]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "WaveLabel", "Wave"), FText::AsNumber(ActiveRun->WaveIndex))],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f)));
		};

		TSharedRef<SWidget> ActiveTabContent = OwnedItemsPanel;
		FText DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultCircusStatus", "The Mini circus now uses the same compact circus shell as the main game.");
		if (ActiveTab == EMiniCircusTab::Vendor)
		{
			TSharedRef<SHorizontalBox> OfferRow = SNew(SHorizontalBox);
			for (const FName OfferID : CircusSubsystem->GetCurrentVendorOfferIDs())
			{
				const FT66MiniItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(OfferID) : nullptr;
				if (!Item)
				{
					continue;
				}
				const bool bAffordable = Gold >= Item->BaseBuyGold;
				OfferRow->AddSlot().AutoWidth().Padding(FMargin(10.f, 0.f, 0.f, 0.f))
				[
					SNew(SBox).WidthOverride(CardWidth).HeightOverride(CardHeight)
					[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(T66MiniReadableName(Item->ItemID).ToUpper())).Font(FT66Style::MakeFont(TEXT("Bold"), 9)).ColorAndOpacity(FT66Style::Text()).AutoWrapText(true).WrapTextAt(CardWidth - 10.f)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)[T66MiniMakeIconPanel(this, TexPool, Item->IconPath, FString::Printf(TEXT("MiniOffer_%s"), *Item->ItemID.ToString()), FText::FromString(T66MiniReadableName(Item->ItemID).Left(2)), FVector2D(CardIconSize, CardIconSize))]]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(T66MiniDescribeItemEffect(*Item))).Font(FT66Style::MakeFont(TEXT("Regular"), 7)).ColorAndOpacity(FT66Style::TextMuted()).AutoWrapText(true).WrapTextAt(CardWidth - 10.f)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
								[
									FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "Buy", "BUY"), FOnClicked::CreateLambda([this, ItemID = Item->ItemID]() { return HandleBuyItemClicked(ItemID); }), ET66ButtonType::Success).SetFontSize(9).SetPadding(FMargin(8.f, 5.f)).SetMinWidth(0.f).SetEnabled(bAffordable))
								]
								+ SHorizontalBox::Slot().FillWidth(1.f)
								[
									FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "Steal", "STEAL"), FOnClicked::CreateLambda([this, ItemID = Item->ItemID]() { return HandleStealItemClicked(ItemID); }), ET66ButtonType::Danger).SetFontSize(9).SetPadding(FMargin(8.f, 5.f)).SetMinWidth(0.f))
								]
							],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(5.f)))
					]
				];
			}
			ActiveTabContent = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)[SNew(SBox).WidthOverride(186.f)[MakeRunPanel()]]
				+ SHorizontalBox::Slot().FillWidth(1.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "VendorTitle", "VENDOR")).Font(FT66Style::MakeFont(TEXT("Bold"), 24)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[SNew(SScrollBox).Orientation(Orient_Horizontal) + SScrollBox::Slot()[OfferRow]]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "OwnedItemsTitle", "OWNED ITEMS")).Font(FT66Style::MakeFont(TEXT("Bold"), 10)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)[SNew(SScrollBox) + SScrollBox::Slot()[OwnedItemsPanel]], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f)))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f)[SNew(SBox).WidthOverride(220.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "HouseTitleVendor", "CIRCUS HOUSE")).Font(FT66Style::MakeFont(TEXT("Bold"), 16)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 0.f)[FT66Style::MakePanel(SNew(STextBlock).Text(FText::Format(NSLOCTEXT("T66Mini.Shop", "AngerFmt", "ANGER {0}%"), FText::AsNumber(AngerPct))).Font(FT66Style::MakeFont(TEXT("Bold"), 18)).ColorAndOpacity(AngerPct >= 75 ? FT66Style::Danger() : FT66Style::Accent2()), FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(10.f)))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "DebtLabel", "Debt"), FText::AsNumber(Debt))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "GoldLabel", "Gold"), FText::AsNumber(Gold))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(FText::Format(NSLOCTEXT("T66Mini.Shop", "RerollFmt", "REROLL ({0}G)"), FText::AsNumber(RerollCost)), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleRerollClicked), ET66ButtonType::Primary).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)).SetEnabled(Gold >= RerollCost))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "BorrowForty", "BORROW 40G"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleBorrowGoldClicked, 40), ET66ButtonType::Neutral).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "PayForty", "PAY 40 DEBT"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandlePayDebtClicked, 40), ET66ButtonType::Success).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)).SetEnabled(Debt > 0 && Gold > 0))], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f)))]];
		}
		else if (ActiveTab == EMiniCircusTab::Gambling)
		{
			DefaultStatus = NSLOCTEXT("T66Mini.Shop", "GamblingShellStatus", "Casino games now sit inside the shared circus shell, so the house economy stays visible while you gamble.");
			TSharedRef<SUniformGridPanel> GameGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f));
			struct FMiniCircusGameDef
			{
				const TCHAR* Title;
				const TCHAR* Description;
				const TCHAR* GameID;
				const TCHAR* IconPath;
			};
			static const FMiniCircusGameDef GameDefs[] = {
				{ TEXT("ROCK PAPER SCISSORS"), TEXT("The demon hand mind-game stays in the lineup."), TEXT("RockPaperScissors"), TEXT("/Game/UI/Sprites/Games/T_Game_RPS.T_Game_RPS") },
				{ TEXT("BLACK JACK"), TEXT("Card table variance with larger payouts."), TEXT("BlackJack"), TEXT("/Game/UI/Sprites/Games/T_Game_BJ.T_Game_BJ") },
				{ TEXT("COIN FLIP"), TEXT("Heads or tails with quick gold swings."), TEXT("CoinFlip"), TEXT("/Game/UI/Sprites/Games/T_Game_Coin.T_Game_Coin") },
				{ TEXT("LOTTERY"), TEXT("Cheap ticket, high spike payout."), TEXT("Lottery"), TEXT("/Game/UI/Sprites/Games/LOTTERY.LOTTERY") },
				{ TEXT("PLINKO"), TEXT("Drop and pray for the multiplier lane."), TEXT("Plinko"), TEXT("/Game/UI/Sprites/Games/PLINKO.PLINKO") },
				{ TEXT("BOX OPENING"), TEXT("Fast roll for gold or a random item."), TEXT("BoxOpening"), TEXT("/Game/UI/Sprites/Games/CRATE.CRATE") }
			};
			for (int32 Index = 0; Index < UE_ARRAY_COUNT(GameDefs); ++Index)
			{
				const FMiniCircusGameDef& GameDef = GameDefs[Index];
				GameGrid->AddSlot(Index % 3, Index / 3)
				[
					SNew(SBox).WidthOverride(CardWidth).HeightOverride(CardHeight)
					[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(GameDef.Title)).Font(FT66Style::MakeFont(TEXT("Bold"), 9)).ColorAndOpacity(FT66Style::Text()).AutoWrapText(true).WrapTextAt(CardWidth - 10.f)]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center)[T66MiniMakeIconPanel(this, TexPool, GameDef.IconPath, FString::Printf(TEXT("MiniGame_%s"), GameDef.GameID), FText::FromString(FString(GameDef.Title).Left(2)), FVector2D(CardIconSize, CardIconSize))]]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[SNew(STextBlock).Text(FText::FromString(GameDef.Description)).Font(FT66Style::MakeFont(TEXT("Regular"), 7)).ColorAndOpacity(FT66Style::TextMuted()).AutoWrapText(true).WrapTextAt(CardWidth - 10.f)]
							+ SVerticalBox::Slot().FillHeight(1.f).VAlign(VAlign_Bottom).Padding(0.f, 8.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "OpenGame", "OPEN"), FOnClicked::CreateLambda([this, GameName = FName(GameDef.GameID), Title = FString(GameDef.Title)]() { return HandleCircusGameClicked(GameName, Title); }), ET66ButtonType::Primary).SetFontSize(9).SetPadding(FMargin(8.f, 5.f)).SetMinWidth(0.f))],
							FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(5.f)))
					]
				];
			}
			ActiveTabContent = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)[SNew(SBox).WidthOverride(186.f)[MakeRunPanel()]]
				+ SHorizontalBox::Slot().FillWidth(1.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "GamblingTitle", "GAMBLING")).Font(FT66Style::MakeFont(TEXT("Bold"), 24)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[GameGrid], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f)))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f)[SNew(SBox).WidthOverride(220.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "HouseTitleGambling", "CASINO HOUSE")).Font(FT66Style::MakeFont(TEXT("Bold"), 16)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 0.f)[FT66Style::MakePanel(SNew(STextBlock).Text(FText::Format(NSLOCTEXT("T66Mini.Shop", "AngerFmt", "ANGER {0}%"), FText::AsNumber(AngerPct))).Font(FT66Style::MakeFont(TEXT("Bold"), 18)).ColorAndOpacity(AngerPct >= 75 ? FT66Style::Danger() : FT66Style::Accent2()), FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(10.f)))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "DebtLabel", "Debt"), FText::AsNumber(Debt))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[T66MiniMakeInfoRow(NSLOCTEXT("T66Mini.Shop", "GoldLabel", "Gold"), FText::AsNumber(Gold))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "BorrowForty", "BORROW 40G"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleBorrowGoldClicked, 40), ET66ButtonType::Neutral).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "PayForty", "PAY 40 DEBT"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandlePayDebtClicked, 40), ET66ButtonType::Success).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)).SetEnabled(Debt > 0 && Gold > 0))], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f)))]];
		}
		else
		{
			DefaultStatus = NSLOCTEXT("T66Mini.Shop", "AlchemyShellStatus", "Alchemy now lives in the same compact circus language as the main game.");
			ActiveTabContent = SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 12.f, 0.f)[SNew(SBox).WidthOverride(186.f)[MakeRunPanel()]]
				+ SHorizontalBox::Slot().FillWidth(1.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "AlchemyTitle", "ALCHEMY")).Font(FT66Style::MakeFont(TEXT("Bold"), 24)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "GoldChip", "GOLD"), FText::AsNumber(Gold), FT66Style::Text())]
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "DebtChip", "DEBT"), FText::AsNumber(Debt), Debt > 0 ? FT66Style::Danger() : FT66Style::TextMuted())]
						+ SHorizontalBox::Slot().AutoWidth()[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "AngerChip", "ANGER"), FText::Format(NSLOCTEXT("T66Mini.Shop", "AngerShortFmt", "{0}%"), FText::AsNumber(AngerPct)), FT66Style::Accent2())]]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 12.f, 0.f, 0.f)[SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(CardWidth).HeightOverride(CardHeight)[FT66Style::MakePanel(T66MiniMakeIconPanel(this, TexPool, AlchemySourceItem ? AlchemySourceItem->IconPath : FString(), TEXT("MiniAlchemySource"), NSLOCTEXT("T66Mini.Shop", "AlchemyFallback", "--"), FVector2D(CardIconSize, CardIconSize)), FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(5.f)))]]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16.f, 0.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "AlchemyArrow", "=>")).Font(FT66Style::MakeFont(TEXT("Bold"), 24)).ColorAndOpacity(FT66Style::Text())]
						+ SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(CardWidth).HeightOverride(CardHeight)[FT66Style::MakePanel(T66MiniMakeIconPanel(this, TexPool, AlchemyItem ? AlchemyItem->IconPath : FString(), TEXT("MiniAlchemyResult"), NSLOCTEXT("T66Mini.Shop", "AlchemyResultFallback", "AL"), FVector2D(CardIconSize, CardIconSize)), FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(5.f)))]]]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "OwnedItemsTitle", "OWNED ITEMS")).Font(FT66Style::MakeFont(TEXT("Bold"), 10)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)[SNew(SScrollBox) + SScrollBox::Slot()[OwnedItemsPanel]], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(16.f)))]
				+ SHorizontalBox::Slot().AutoWidth().Padding(12.f, 0.f, 0.f, 0.f)[SNew(SBox).WidthOverride(220.f)[FT66Style::MakePanel(SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66Mini.Shop", "HouseTitleAlchemy", "ALCHEMY HOUSE")).Font(FT66Style::MakeFont(TEXT("Bold"), 16)).ColorAndOpacity(FT66Style::Text())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "AlchemyTransmute", "TRANSMUTE 2 ITEMS"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyTransmuteClicked), ET66ButtonType::Primary).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)).SetEnabled(ActiveRun->OwnedItemIDs.Num() >= 2))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "AlchemyDissolve", "DISSOLVE OLDEST"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyDissolveClicked), ET66ButtonType::Danger).SetFontSize(12).SetPadding(FMargin(10.f, 6.f)).SetEnabled(ActiveRun->OwnedItemIDs.Num() > 0))], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(14.f)))]];
		}

		const auto MakeTabButton = [this](const EMiniCircusTab Tab, const FText& Label) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = (Tab == EMiniCircusTab::Vendor)
				? FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleVendorTabClicked)
				: (Tab == EMiniCircusTab::Gambling)
					? FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleGamblingTabClicked)
					: FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyTabClicked);
			return FT66Style::MakeButton(FT66ButtonParams(Label, OnClicked, ActiveTab == Tab ? ET66ButtonType::Primary : ET66ButtonType::Neutral).SetPadding(FMargin(10.f, 6.f)).SetFontSize(14).SetMinWidth(140.f));
		};

		const FText StatusToShow = CurrentStatusText.IsEmpty() ? DefaultStatus : CurrentStatusText;
		return FT66Style::MakeResponsiveRoot(
			FT66Style::MakeSafeFrame(
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[FT66Style::MakePanel(SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[FT66Style::MakePanel(SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "HeaderGold", "GOLD"), FText::AsNumber(Gold), FT66Style::Text())]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "HeaderMaterials", "MATS"), FText::AsNumber(Materials), FT66Style::Text())]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "HeaderDebt", "DEBT"), FText::AsNumber(Debt), Debt > 0 ? FT66Style::Danger() : FT66Style::TextMuted())]
							+ SHorizontalBox::Slot().AutoWidth()[T66MiniMakeMetricChip(NSLOCTEXT("T66Mini.Shop", "HeaderWave", "WAVE"), FText::AsNumber(ActiveRun->WaveIndex), FT66Style::Text())], FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(8.f, 6.f)))]
						+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(SSpacer)]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeTabButton(EMiniCircusTab::Vendor, NSLOCTEXT("T66Mini.Shop", "VendorTab", "VENDOR"))]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeTabButton(EMiniCircusTab::Gambling, NSLOCTEXT("T66Mini.Shop", "GamblingTab", "GAMBLING"))]
							+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(EMiniCircusTab::Alchemy, NSLOCTEXT("T66Mini.Shop", "AlchemyTab", "ALCHEMY"))]]
						+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(SSpacer)]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66Mini.Shop", "Continue", "CONTINUE"), FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleContinueClicked), ET66ButtonType::Success).SetPadding(FMargin(10.f, 6.f)).SetFontSize(14).SetMinWidth(150.f))], FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(10.f, 8.f)))]
					+ SVerticalBox::Slot().FillHeight(1.f)[ActiveTabContent]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[FT66Style::MakePanel(SAssignNew(StatusTextBlock, STextBlock).Text(StatusToShow).Font(FT66Style::MakeFont(TEXT("Regular"), 11)).ColorAndOpacity(CurrentStatusText.IsEmpty() ? FT66Style::TextMuted() : FT66Style::Text()).AutoWrapText(true), FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 10.f)))],
					FT66PanelParams(ET66PanelType::Bg).SetPadding(0.f).SetColor(FT66Style::Background())),
				FMargin(16.f)));
	}

	TSharedRef<SUniformGridPanel> OfferGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(12.f));
	TSharedRef<SVerticalBox> OwnedItemsPanel = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> AlchemyItemsPanel = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> BuybackItemsPanel = SNew(SVerticalBox);
	if (DataSubsystem && CircusSubsystem)
	{
		const TArray<FName>& OfferIDs = CircusSubsystem->GetCurrentVendorOfferIDs();
		for (int32 Index = 0; Index < OfferIDs.Num(); ++Index)
		{
			const FT66MiniItemDefinition* Item = DataSubsystem->FindItem(OfferIDs[Index]);
			if (!Item)
			{
				continue;
			}

			const bool bLocked = CircusSubsystem->IsVendorOfferLocked(Item->ItemID);
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
							.Padding(0.f, 0.f, 10.f, 0.f)
							[
								SNew(SButton)
								.OnClicked(FOnClicked::CreateLambda([this, ItemID = Item->ItemID]()
								{
									return HandleStealItemClicked(ItemID);
								}))
								.ButtonColorAndOpacity(T66MiniUI::AccentGold())
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66Mini.Shop", "Steal", "STEAL"))
									.Font(T66MiniUI::BoldFont(18))
									.ColorAndOpacity(T66MiniUI::ButtonTextDark())
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

		if (CircusSubsystem)
		{
			int32 RenderedBuybackRows = 0;
			for (const FName ItemID : CircusSubsystem->GetBuybackItemIDs())
			{
				if (RenderedBuybackRows >= 4)
				{
					break;
				}

				const FT66MiniItemDefinition* Item = DataSubsystem->FindItem(ItemID);
				if (!Item)
				{
					continue;
				}

				BuybackItemsPanel->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ItemID.ToString()))
						.Font(T66MiniUI::BodyFont(15))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.OnClicked(FOnClicked::CreateLambda([this, ItemID]()
						{
							return HandleBuybackClicked(ItemID);
						}))
						.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("BUYBACK %d"), FMath::Max(1, Item->BaseSellGold))))
							.Font(T66MiniUI::BoldFont(14))
							.ColorAndOpacity(T66MiniUI::ButtonTextDark())
						]
					]
				];

				++RenderedBuybackRows;
			}

			if (RenderedBuybackRows == 0)
			{
				BuybackItemsPanel->AddSlot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.Shop", "NoBuybackItems", "No sold items are waiting in buyback."))
					.Font(T66MiniUI::BodyFont(15))
					.ColorAndOpacity(T66MiniUI::MutedText())
				];
			}
		}
	}

	const int32 RerollCost = CircusSubsystem ? (12 + (CircusSubsystem->GetVendorRerollCount() * 6)) : 12;

	TSharedRef<SWidget> ActiveTabContent = SNullWidget::NullWidget;
	FText DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusVendor", "The mini circus vendor now supports buy, steal, reroll, lock, and buyback. Debt and anger persist through the intermission flow.");
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
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 14.f, 0.f, 8.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "BuybackTitle", "BUYBACK"))
						.Font(T66MiniUI::BoldFont(18))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						BuybackItemsPanel
					]
				]
			];
	}
	else if (ActiveTab == EMiniCircusTab::Gambling)
	{
		DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusGambling", "Every mini circus game now resolves real gold swings, debt pressure, and anger buildup.");
		TSharedRef<SUniformGridPanel> GamblingGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(12.f));
		struct FMiniCircusGameCard
		{
			const TCHAR* Title;
			const TCHAR* Description;
			const TCHAR* GameID;
		};

		static const FMiniCircusGameCard GameCards[] = {
			{ TEXT("COIN FLIP"), TEXT("Heads or tails with quick gold swings."), TEXT("CoinFlip") },
			{ TEXT("ROCK PAPER SCISSORS"), TEXT("The demon hand mind-game stays in the lineup."), TEXT("RockPaperScissors") },
			{ TEXT("BLACK JACK"), TEXT("Card table variance with larger payouts."), TEXT("BlackJack") },
			{ TEXT("LOTTERY"), TEXT("Cheap ticket, high spike payout."), TEXT("Lottery") },
			{ TEXT("PLINKO"), TEXT("Drop and pray for the multiplier lane."), TEXT("Plinko") },
			{ TEXT("BOX OPENING"), TEXT("Fast roll for gold or a random item."), TEXT("BoxOpening") }
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
							.OnClicked(FOnClicked::CreateLambda([this, GameID = FName(Card.GameID), Title = FString(Card.Title)]()
							{
								return HandleCircusGameClicked(GameID, Title);
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

		ActiveTabContent =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				GamblingGrid
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleBorrowGoldClicked, 40))
					.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "BorrowGold", "BORROW 40G"))
						.Font(T66MiniUI::BoldFont(18))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandlePayDebtClicked, 40))
					.ButtonColorAndOpacity(T66MiniUI::AccentGreen())
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "PayDebt", "PAY 40 DEBT"))
						.Font(T66MiniUI::BoldFont(18))
						.ColorAndOpacity(T66MiniUI::ButtonTextDark())
						.Justification(ETextJustify::Center)
					]
				]
			];
	}
	else
	{
		DefaultStatus = NSLOCTEXT("T66Mini.Shop", "DefaultStatusAlchemy", "Mini alchemy now dissolves owned items into gold or debt relief, and transmutes two owned items into a new circus piece.");
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
						.Text(NSLOCTEXT("T66Mini.Shop", "AlchemyBody", "Use transmutation to reroll owned items or dissolve old junk into gold and debt relief."))
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
		ActiveTabContent =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				ActiveTabContent
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyTransmuteClicked))
					.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "AlchemyTransmute", "TRANSMUTE 2 ITEMS"))
						.Font(T66MiniUI::BoldFont(18))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniShopScreen::HandleAlchemyDissolveClicked))
					.ButtonColorAndOpacity(T66MiniUI::AccentGold())
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66Mini.Shop", "AlchemyDissolve", "DISSOLVE OLDEST"))
						.Font(T66MiniUI::BoldFont(18))
						.ColorAndOpacity(T66MiniUI::ButtonTextDark())
						.Justification(ETextJustify::Center)
					]
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
				? NSLOCTEXT("T66Mini.Shop", "GamblingHint", "ANGER AND DEBT CARRY FORWARD")
				: NSLOCTEXT("T66Mini.Shop", "AlchemyHint", "ALCHEMY CAN CLEAN DEBT"))
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
								? FString::Printf(TEXT("Gold: %d  |  Materials: %d  |  Debt: %d  |  Anger: %d%%  |  Wave Complete: %d"),
									ActiveRun->Gold,
									ActiveRun->Materials,
									CircusSubsystem ? CircusSubsystem->GetDebt() : 0,
									CircusSubsystem ? FMath::RoundToInt(CircusSubsystem->GetAnger01() * 100.f) : 0,
									ActiveRun->WaveIndex)
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

bool UT66MiniShopScreen::HandleShopSyncTicker(const float DeltaTime)
{
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
		ActiveRun && ActiveRun->bOnlinePartyRun && SaveSubsystem)
	{
		FString CompatibilityFailure;
		if (!SaveSubsystem->IsRunCompatibleWithCurrentParty(ActiveRun, &CompatibilityFailure))
		{
			SetStatus(FText::FromString(CompatibilityFailure));
			if (FrontendState)
			{
				FrontendState->ExitIntermissionFlow();
			}
			ResetMiniIntermissionTransport();
			NavigateTo(ET66ScreenType::MiniMainMenu);
			return true;
		}
	}

	SyncToSharedPartyScreen();

	bool bIsHost = false;
	if (!IsOnlineMiniParty(&bIsHost))
	{
		return true;
	}

	if (bIsHost)
	{
		TryProcessRemoteRequests(false);
	}
	else
	{
		ApplyAuthoritativeStateFromLobby(false);
	}

	return true;
}

void UT66MiniShopScreen::HandlePartyStateChanged()
{
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	if (UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
		ActiveRun && ActiveRun->bOnlinePartyRun && SaveSubsystem)
	{
		FString CompatibilityFailure;
		if (!SaveSubsystem->IsRunCompatibleWithCurrentParty(ActiveRun, &CompatibilityFailure))
		{
			SetStatus(FText::FromString(CompatibilityFailure));
			if (FrontendState)
			{
				FrontendState->ExitIntermissionFlow();
			}
			ResetMiniIntermissionTransport();
			NavigateTo(ET66ScreenType::MiniMainMenu);
			return;
		}
	}

	SyncToSharedPartyScreen();

	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost))
	{
	if (bIsHost)
	{
		TryProcessRemoteRequests(true);
	}
	else
	{
		ApplyAuthoritativeStateFromLobby(true);
	}
	return;
	}

	RequestShopRebuildIfStateChanged();
}

void UT66MiniShopScreen::HandleSessionStateChanged()
{
	HandlePartyStateChanged();
}

void UT66MiniShopScreen::SyncToSharedPartyScreen()
{
	if (!UIManager)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SessionSubsystem || !SessionSubsystem->IsPartySessionActive() || SessionSubsystem->IsLocalPlayerPartyHost())
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	switch (DesiredScreen)
	{
	case ET66ScreenType::MiniSaveSlots:
	case ET66ScreenType::MiniCharacterSelect:
	case ET66ScreenType::MiniCompanionSelect:
	case ET66ScreenType::MiniDifficultySelect:
	case ET66ScreenType::MiniShop:
	case ET66ScreenType::MiniIdolSelect:
	case ET66ScreenType::MiniRunSummary:
	case ET66ScreenType::MiniMainMenu:
		if (ScreenType != DesiredScreen)
		{
			NavigateTo(DesiredScreen);
		}
		break;
	default:
		break;
	}
}

FString UT66MiniShopScreen::BuildShopUiStateKey() const
{
	FString StateKey = FString::Printf(
		TEXT("%d|%d|%s"),
		static_cast<int32>(ActiveTab),
		LastAppliedStateRevision,
		*CurrentStatusText.ToString());

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (ActiveRun)
	{
		StateKey += FString::Printf(
			TEXT("|R:%d:%d:%d:%d:%d:%f:%s:%s"),
			ActiveRun->bOnlinePartyRun ? 1 : 0,
			ActiveRun->bPendingShopIntermission ? 1 : 0,
			ActiveRun->WaveIndex,
			ActiveRun->ShopRerollCount,
			ActiveRun->CircusDebt,
			ActiveRun->CircusAnger01,
			*ActiveRun->DifficultyID.ToString(),
			*ActiveRun->OnlineHostSteamId);

		for (const FName& ItemId : ActiveRun->OwnedItemIDs)
		{
			StateKey += TEXT("|O:");
			StateKey += ItemId.ToString();
		}

		for (const FName& OfferId : ActiveRun->CurrentShopOfferIDs)
		{
			StateKey += TEXT("|C:");
			StateKey += OfferId.ToString();
		}

		for (const FName& LockedId : ActiveRun->LockedShopOfferIDs)
		{
			StateKey += TEXT("|L:");
			StateKey += LockedId.ToString();
		}

		for (const FName& BuybackId : ActiveRun->CircusBuybackItemIDs)
		{
			StateKey += TEXT("|B:");
			StateKey += BuybackId.ToString();
		}
	}

	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (SessionSubsystem)
	{
		StateKey += FString::Printf(
			TEXT("|S:%d:%d:%d"),
			SessionSubsystem->IsPartySessionActive() ? 1 : 0,
			SessionSubsystem->IsLocalPlayerPartyHost() ? 1 : 0,
			SessionSubsystem->IsLocalLobbyReady() ? 1 : 0);

		FT66LobbyPlayerInfo HostLobbyInfo;
		if (SessionSubsystem->GetHostLobbyProfile(HostLobbyInfo))
		{
			StateKey += FString::Printf(
				TEXT("|H:%d:%d:%d"),
				HostLobbyInfo.MiniIntermissionStateRevision,
				HostLobbyInfo.MiniIntermissionRequestRevision,
				HostLobbyInfo.bLobbyReady ? 1 : 0);
		}
	}

	return StateKey;
}

void UT66MiniShopScreen::RequestShopRebuildIfStateChanged()
{
	const FString NewUiStateKey = BuildShopUiStateKey();
	if (NewUiStateKey == LastShopUiStateKey)
	{
		return;
	}

	LastShopUiStateKey = NewUiStateKey;
	FT66Style::DeferRebuild(this);
}

bool UT66MiniShopScreen::IsOnlineMiniParty(bool* bOutIsHost) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	const UT66SessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bIsHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	if (bOutIsHost)
	{
		*bOutIsHost = bIsHost;
	}

	return PartySubsystem && PartySubsystem->HasRemotePartyMembers() && SessionSubsystem && SessionSubsystem->IsPartySessionActive();
}

bool UT66MiniShopScreen::ResolveLocalPartyIdentity(FString& OutPlayerId, FString& OutDisplayName) const
{
	OutPlayerId.Reset();
	OutDisplayName.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UT66PartySubsystem* PartySubsystem = GameInstance ? GameInstance->GetSubsystem<UT66PartySubsystem>() : nullptr;
	if (PartySubsystem)
	{
		OutPlayerId = PartySubsystem->GetLocalPlayerId();
		OutDisplayName = PartySubsystem->GetLocalDisplayName();
	}

	return !OutPlayerId.IsEmpty();
}

void UT66MiniShopScreen::ResetMiniIntermissionTransport() const
{
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GetGameInstance());
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!T66GameInstance)
	{
		return;
	}

	T66GameInstance->MiniIntermissionStateRevision = 0;
	T66GameInstance->MiniIntermissionStateJson.Reset();
	T66GameInstance->MiniIntermissionRequestRevision = 0;
	T66GameInstance->MiniIntermissionRequestJson.Reset();
	if (SessionSubsystem && SessionSubsystem->IsPartySessionActive())
	{
		SessionSubsystem->SyncLocalLobbyProfile();
	}
}

bool UT66MiniShopScreen::BuildAuthoritativeSyncPayload(FT66MiniShopSyncPayload& OutPayload) const
{
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!ActiveRun)
	{
		return false;
	}

	if (ActiveRun->bOnlinePartyRun && SaveSubsystem)
	{
		SaveSubsystem->CaptureLocalPartyPlayerSnapshotFromLegacyFields(ActiveRun);
	}

	OutPayload.StatusText = CurrentStatusText.ToString();
	OutPayload.HostSteamId = PartySubsystem ? PartySubsystem->GetLocalPlayerId() : FString();
	OutPayload.HostDisplayName = PartySubsystem ? PartySubsystem->GetLocalDisplayName() : FString();
	OutPayload.DifficultyID = ActiveRun->DifficultyID;
	OutPayload.WaveIndex = ActiveRun->WaveIndex;
	OutPayload.TotalRunSeconds = ActiveRun->TotalRunSeconds;
	OutPayload.CurrentShopOfferIDs = ActiveRun->CurrentShopOfferIDs;
	OutPayload.LockedShopOfferIDs = ActiveRun->LockedShopOfferIDs;
	OutPayload.ShopRerollCount = ActiveRun->ShopRerollCount;
	OutPayload.CircusDebt = ActiveRun->CircusDebt;
	OutPayload.CircusAnger01 = ActiveRun->CircusAnger01;
	OutPayload.CircusBuybackItemIDs = ActiveRun->CircusBuybackItemIDs;
	OutPayload.CircusVendorRerollCount = ActiveRun->CircusVendorRerollCount;
	OutPayload.OnlinePartyMemberIds = ActiveRun->OnlinePartyMemberIds;
	OutPayload.OnlinePartyMemberDisplayNames = ActiveRun->OnlinePartyMemberDisplayNames;
	OutPayload.PartyPlayerSnapshots = ActiveRun->PartyPlayerSnapshots;
	return true;
}

bool UT66MiniShopScreen::PublishAuthoritativeState(const FText& StatusText, const bool bIncrementRevision)
{
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GetGameInstance());
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!T66GameInstance || !SessionSubsystem)
	{
		return false;
	}

	if (!StatusText.IsEmpty())
	{
		CurrentStatusText = StatusText;
	}

	FT66MiniShopSyncPayload Payload;
	if (!BuildAuthoritativeSyncPayload(Payload))
	{
		return false;
	}

	Payload.Revision = bIncrementRevision
		? (T66GameInstance->MiniIntermissionStateRevision + 1)
		: FMath::Max(1, T66GameInstance->MiniIntermissionStateRevision);
	if (Payload.StatusText.IsEmpty() && !CurrentStatusText.IsEmpty())
	{
		Payload.StatusText = CurrentStatusText.ToString();
	}

	FString Json;
	if (!T66MiniSerializePayload(Payload, Json))
	{
		return false;
	}

	T66GameInstance->MiniIntermissionStateRevision = Payload.Revision;
	T66GameInstance->MiniIntermissionStateJson = MoveTemp(Json);
	LastAppliedStateRevision = Payload.Revision;
	SessionSubsystem->SyncLocalLobbyProfile();
	return true;
}

bool UT66MiniShopScreen::ApplyAuthoritativeStateFromLobby(const bool bForceRebuild)
{
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniCircusSubsystem* CircusSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!SessionSubsystem || !SaveSubsystem || !FrontendState || !CircusSubsystem || !ActiveRun)
	{
		return false;
	}

	FT66LobbyPlayerInfo HostLobbyInfo;
	if (!SessionSubsystem->GetHostLobbyProfile(HostLobbyInfo)
		|| HostLobbyInfo.MiniIntermissionStateRevision <= 0
		|| HostLobbyInfo.MiniIntermissionStateJson.IsEmpty())
	{
		return false;
	}

	if (!bForceRebuild && HostLobbyInfo.MiniIntermissionStateRevision == LastAppliedStateRevision)
	{
		return false;
	}

	FT66MiniShopSyncPayload Payload;
	if (!T66MiniDeserializePayload(HostLobbyInfo.MiniIntermissionStateJson, Payload))
	{
		return false;
	}

	ActiveRun->bOnlinePartyRun = true;
	ActiveRun->bPendingShopIntermission = true;
	ActiveRun->OnlineHostSteamId = Payload.HostSteamId;
	ActiveRun->OnlineHostDisplayName = Payload.HostDisplayName;
	ActiveRun->DifficultyID = Payload.DifficultyID;
	ActiveRun->WaveIndex = FMath::Max(1, Payload.WaveIndex);
	ActiveRun->TotalRunSeconds = FMath::Max(0.f, Payload.TotalRunSeconds);
	ActiveRun->CurrentShopOfferIDs = Payload.CurrentShopOfferIDs;
	ActiveRun->LockedShopOfferIDs = Payload.LockedShopOfferIDs;
	ActiveRun->ShopRerollCount = Payload.ShopRerollCount;
	ActiveRun->CircusDebt = Payload.CircusDebt;
	ActiveRun->CircusAnger01 = Payload.CircusAnger01;
	ActiveRun->CircusBuybackItemIDs = Payload.CircusBuybackItemIDs;
	ActiveRun->CircusVendorRerollCount = Payload.CircusVendorRerollCount;
	ActiveRun->OnlinePartyMemberIds = Payload.OnlinePartyMemberIds;
	ActiveRun->OnlinePartyMemberDisplayNames = Payload.OnlinePartyMemberDisplayNames;
	ActiveRun->PartyPlayerSnapshots = Payload.PartyPlayerSnapshots;
	SaveSubsystem->MirrorLocalPartyPlayerSnapshotToLegacyFields(ActiveRun);
	FrontendState->SeedFromRunSave(ActiveRun);
	FrontendState->ResumeIntermissionFlow();
	CircusSubsystem->SeedFromRunSave(ActiveRun);
	CircusSubsystem->PrimeVendorOffers(DataSubsystem);
	LastAppliedStateRevision = Payload.Revision;

	if (!Payload.StatusText.IsEmpty())
	{
		CurrentStatusText = FText::FromString(Payload.StatusText);
	}

	if (bForceRebuild || Payload.Revision > 0)
	{
		RequestShopRebuildIfStateChanged();
	}

	return true;
}

bool UT66MiniShopScreen::TryQueueRemoteRequest(const FT66MiniShopRequestPayload& Request, const FText& PendingStatus)
{
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GetGameInstance());
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!T66GameInstance || !SessionSubsystem)
	{
		return false;
	}

	FT66MiniShopRequestPayload PendingRequest = Request;
	PendingRequest.Revision = T66GameInstance->MiniIntermissionRequestRevision + 1;

	FString Json;
	if (!T66MiniSerializePayload(PendingRequest, Json))
	{
		return false;
	}

	T66GameInstance->MiniIntermissionRequestRevision = PendingRequest.Revision;
	T66GameInstance->MiniIntermissionRequestJson = MoveTemp(Json);
	SessionSubsystem->SyncLocalLobbyProfile();
	SetStatus(PendingStatus);
	return true;
}

bool UT66MiniShopScreen::TryProcessRemoteRequests(const bool bForceRebuild)
{
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!SessionSubsystem || !SaveSubsystem || !FrontendState || !ActiveRun)
	{
		return false;
	}

	if (ActiveRun->bOnlinePartyRun)
	{
		FString CompatibilityFailure;
		if (!SaveSubsystem->IsRunCompatibleWithCurrentParty(ActiveRun, &CompatibilityFailure))
		{
			SetStatus(FText::FromString(CompatibilityFailure));
			FrontendState->ExitIntermissionFlow();
			ResetMiniIntermissionTransport();
			NavigateTo(ET66ScreenType::MiniMainMenu);
			return false;
		}
	}

	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);

	bool bProcessedAny = false;
	bool bNavigateToIdolSelect = false;
	FText LatestStatus = CurrentStatusText;
	for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
	{
		if (LobbyInfo.bPartyHost
			|| LobbyInfo.MiniIntermissionRequestRevision <= 0
			|| LobbyInfo.MiniIntermissionRequestJson.IsEmpty())
		{
			continue;
		}

		int32& ProcessedRevision = ProcessedRequestRevisionByPlayerId.FindOrAdd(LobbyInfo.SteamId);
		if (LobbyInfo.MiniIntermissionRequestRevision <= ProcessedRevision)
		{
			continue;
		}

		FT66MiniShopRequestPayload Request;
		ProcessedRevision = LobbyInfo.MiniIntermissionRequestRevision;
		if (!T66MiniDeserializePayload(LobbyInfo.MiniIntermissionRequestJson, Request))
		{
			continue;
		}

		if (ExecuteAuthoritativeRequest(LobbyInfo.SteamId, LobbyInfo.DisplayName, Request, bNavigateToIdolSelect))
		{
			LatestStatus = CurrentStatusText;
			bProcessedAny = true;
		}
	}

	if (bProcessedAny)
	{
		PublishAuthoritativeState(LatestStatus, true);
		RequestShopRebuildIfStateChanged();
	}
	else if (bForceRebuild)
	{
		RequestShopRebuildIfStateChanged();
	}

	if (bNavigateToIdolSelect)
	{
		NavigateTo(ET66ScreenType::MiniIdolSelect);
	}

	return bProcessedAny;
}

bool UT66MiniShopScreen::ExecuteAuthoritativeRequest(
	const FString& SourcePlayerId,
	const FString& SourceDisplayName,
	const FT66MiniShopRequestPayload& Request,
	bool& bOutNavigateToIdolSelect)
{
	bOutNavigateToIdolSelect = false;

	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniCircusSubsystem* CircusSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!CircusSubsystem || !RunState || !SaveSubsystem || !ActiveRun)
	{
		return false;
	}

	if (ActiveRun->bOnlinePartyRun && !SourcePlayerId.IsEmpty())
	{
		SaveSubsystem->MirrorPartyPlayerSnapshotToLegacyFields(ActiveRun, SourcePlayerId);
	}

	CircusSubsystem->SeedFromRunSave(ActiveRun);

	bool bRequestApplied = false;
	FString Result;
	if (Request.Action == MiniShopActionBuy)
	{
		bRequestApplied = CircusSubsystem->TryBuyOffer(ActiveRun, DataSubsystem, Request.ItemID, Result);
	}
	else if (Request.Action == MiniShopActionSteal)
	{
		bRequestApplied = CircusSubsystem->TryStealOffer(ActiveRun, DataSubsystem, Request.ItemID, Result);
	}
	else if (Request.Action == MiniShopActionSell)
	{
		bRequestApplied = CircusSubsystem->TrySellOwnedItem(ActiveRun, DataSubsystem, Request.ItemID, Result);
	}
	else if (Request.Action == MiniShopActionBuyback)
	{
		bRequestApplied = CircusSubsystem->TryBuybackItem(ActiveRun, DataSubsystem, Request.ItemID, Result);
	}
	else if (Request.Action == MiniShopActionToggleLock)
	{
		CircusSubsystem->ToggleVendorOfferLock(Request.ItemID);
		Result = FString::Printf(TEXT("%s %s a circus lock on %s."),
			SourceDisplayName.IsEmpty() ? TEXT("A party member") : *SourceDisplayName,
			CircusSubsystem->IsVendorOfferLocked(Request.ItemID) ? TEXT("placed") : TEXT("removed"),
			*Request.ItemID.ToString());
		bRequestApplied = true;
	}
	else if (Request.Action == MiniShopActionReroll)
	{
		bRequestApplied = CircusSubsystem->TryRerollVendor(ActiveRun, DataSubsystem, Result);
	}
	else if (Request.Action == MiniShopActionBorrow)
	{
		bRequestApplied = CircusSubsystem->TryBorrowGold(ActiveRun, Request.Amount, Result);
	}
	else if (Request.Action == MiniShopActionPayDebt)
	{
		bRequestApplied = CircusSubsystem->TryPayDebt(ActiveRun, Request.Amount, Result);
	}
	else if (Request.Action == MiniShopActionCircusGame)
	{
		bRequestApplied = CircusSubsystem->TryPlayGame(Request.GameID, ActiveRun, DataSubsystem, Result);
	}
	else if (Request.Action == MiniShopActionAlchemyTransmute)
	{
		bRequestApplied = CircusSubsystem->TryAlchemyTransmute(ActiveRun, DataSubsystem, Result);
	}
	else if (Request.Action == MiniShopActionAlchemyDissolve)
	{
		bRequestApplied = CircusSubsystem->TryAlchemyDissolveOldest(ActiveRun, DataSubsystem, Result);
	}

	CircusSubsystem->WriteToRunSave(ActiveRun);
	if (ActiveRun->bOnlinePartyRun && !SourcePlayerId.IsEmpty())
	{
		SaveSubsystem->CapturePartyPlayerSnapshotFromLegacyFields(ActiveRun, SourcePlayerId);
		SaveSubsystem->MirrorLocalPartyPlayerSnapshotToLegacyFields(ActiveRun);
	}

	if (FrontendState)
	{
		FrontendState->SeedFromRunSave(ActiveRun);
		FrontendState->ResumeIntermissionFlow();
	}

	if (RunState->GetActiveSaveSlot() != INDEX_NONE)
	{
		SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	}

	SetStatus(FText::FromString(Result.IsEmpty() ? TEXT("Mini circus state updated.") : Result));
	return bRequestApplied;
}

FReply UT66MiniShopScreen::HandleVendorTabClicked()
{
	if (ActiveTab == EMiniCircusTab::Vendor)
	{
		return FReply::Handled();
	}

	ActiveTab = EMiniCircusTab::Vendor;
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleGamblingTabClicked()
{
	if (ActiveTab == EMiniCircusTab::Gambling)
	{
		return FReply::Handled();
	}

	ActiveTab = EMiniCircusTab::Gambling;
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleAlchemyTabClicked()
{
	if (ActiveTab == EMiniCircusTab::Alchemy)
	{
		return FReply::Handled();
	}

	ActiveTab = EMiniCircusTab::Alchemy;
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleBuyItemClicked(const FName ItemID)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionBuy;
		Request.ItemID = ItemID;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingBuy", "Purchase request sent to the host circus ledger."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionBuy;
	Request.ItemID = ItemID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleStealItemClicked(const FName ItemID)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionSteal;
		Request.ItemID = ItemID;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingSteal", "Steal attempt sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionSteal;
	Request.ItemID = ItemID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleSellItemClicked(const FName ItemID)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionSell;
		Request.ItemID = ItemID;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingSell", "Sell request sent to the host circus ledger."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionSell;
	Request.ItemID = ItemID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleBuybackClicked(const FName ItemID)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionBuyback;
		Request.ItemID = ItemID;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingBuyback", "Buyback request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionBuyback;
	Request.ItemID = ItemID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleLockClicked(const FName ItemID)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionToggleLock;
		Request.ItemID = ItemID;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingLock", "Lock change sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionToggleLock;
	Request.ItemID = ItemID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleRerollClicked()
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionReroll;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingReroll", "Vendor reroll request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionReroll;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleBorrowGoldClicked(const int32 Amount)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionBorrow;
		Request.Amount = Amount;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingBorrow", "Borrow request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionBorrow;
	Request.Amount = Amount;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandlePayDebtClicked(const int32 Amount)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionPayDebt;
		Request.Amount = Amount;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingPayDebt", "Debt payment request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionPayDebt;
	Request.Amount = Amount;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleCircusGameClicked(const FName GameID, const FString GameTitle)
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionCircusGame;
		Request.GameID = GameID;
		TryQueueRemoteRequest(Request, FText::FromString(FString::Printf(TEXT("%s request sent to the host."), *GameTitle)));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionCircusGame;
	Request.GameID = GameID;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleAlchemyTransmuteClicked()
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionAlchemyTransmute;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingTransmute", "Alchemy transmute request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionAlchemyTransmute;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleAlchemyDissolveClicked()
{
	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		FT66MiniShopRequestPayload Request;
		Request.Action = MiniShopActionAlchemyDissolve;
		TryQueueRemoteRequest(Request, NSLOCTEXT("T66Mini.Shop", "PendingDissolve", "Alchemy dissolve request sent to the host."));
		return FReply::Handled();
	}

	FString LocalPlayerId;
	FString LocalDisplayName;
	ResolveLocalPartyIdentity(LocalPlayerId, LocalDisplayName);
	FT66MiniShopRequestPayload Request;
	Request.Action = MiniShopActionAlchemyDissolve;
	bool bNavigateToIdolSelect = false;
	ExecuteAuthoritativeRequest(LocalPlayerId, LocalDisplayName, Request, bNavigateToIdolSelect);
	PublishAuthoritativeState(CurrentStatusText, IsOnlineMiniParty());
	RequestShopRebuildIfStateChanged();
	return FReply::Handled();
}

FReply UT66MiniShopScreen::HandleContinueClicked()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniCircusSubsystem* CircusSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!DataSubsystem || !FrontendState || !CircusSubsystem || !RunState || !SaveSubsystem || !ActiveRun)
	{
		return FReply::Handled();
	}

	bool bIsHost = false;
	if (IsOnlineMiniParty(&bIsHost) && !bIsHost)
	{
		SetStatus(NSLOCTEXT("T66Mini.Shop", "GuestContinueBlocked", "Only the mini party host can advance out of the circus."));
		return FReply::Handled();
	}

	FrontendState->RefreshIdolOffers(DataSubsystem);
	ActiveRun->bPendingShopIntermission = true;
	FrontendState->ResumeIntermissionFlow();
	CircusSubsystem->WriteToRunSave(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	ResetMiniIntermissionTransport();
	NavigateTo(ET66ScreenType::MiniIdolSelect);
	return FReply::Handled();
}

void UT66MiniShopScreen::SetStatus(const FText& InText)
{
	CurrentStatusText = InText;
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(InText);
		StatusTextBlock->SetColorAndOpacity(InText.IsEmpty() ? FT66Style::TextMuted() : FT66Style::Text());
	}
}
