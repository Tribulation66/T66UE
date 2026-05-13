// Copyright Tribulation 66. All Rights Reserved.

class SBorder;
class SBox;
class STextBlock;

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureDefines.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Screens/T66SelectionScreenUtils.h"
#include "UI/Screens/T66ChallengesScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
// Render target removed — in-world preview uses main viewport camera with full Lumen GI.

using namespace T66HeroSelectionPrivate;

namespace
{
	FText FormatCompanionRecordRankText(const int32 Rank)
	{
		if (Rank <= 0)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnranked", "Unranked");
		}

		if (Rank <= 10000)
		{
			return FText::Format(
				NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankExactFormat", "#{0}"),
				FText::AsNumber(Rank));
		}

		if (Rank <= 25000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop25K", "Top 25K");
		}

		if (Rank <= 50000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop50K", "Top 50K");
		}

		if (Rank <= 100000)
		{
			return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankTop100K", "Top 100K");
		}

		return NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnranked", "Unranked");
	}

	FText FormatCompanionPassiveHealText(const ET66Difficulty Difficulty)
	{
		const float Amount = AT66CompanionBase::GetHealingAmountForDifficulty(Difficulty);
		const float Interval = AT66CompanionBase::GetHealingIntervalSecondsForDifficulty(Difficulty);
		return FText::Format(
			NSLOCTEXT("T66.CompanionSelection", "PassiveHealFormat", "PASSIVE: Heals the hero {0} health every {1} seconds."),
			FText::AsNumber(FMath::RoundToInt(Amount)),
			FText::AsNumber(Interval));
	}

	FText ResolveCompanionLoreText(const UT66LocalizationSubsystem* Loc, const FCompanionData& CompanionData)
	{
		if (!CompanionData.LoreText.IsEmpty())
		{
			return CompanionData.LoreText;
		}
		return Loc
			? Loc->GetText_CompanionLore(CompanionData.CompanionID)
			: NSLOCTEXT("T66.CompanionSelection", "FallbackCompanionLore", "A mysterious companion.");
	}

	AT66PlayerController* T66GetLocalFrontendCompanionPlayerController(UObject* ContextObject)
	{
		return ContextObject ? Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(ContextObject, 0)) : nullptr;
	}

	void T66PositionCompanionPreviewCamera(UObject* ContextObject)
	{
		if (!ContextObject)
		{
			return;
		}

		if (UWorld* World = ContextObject->GetWorld())
		{
			if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			{
				GM->PositionCameraForCompanionPreview();
				return;
			}
		}

		if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(ContextObject))
		{
			PC->PositionLocalFrontendCameraForCompanionPreview();
		}
	}

	FName CompanionSelectionTag(const TCHAR* Tag)
	{
		return FName(Tag);
	}

	FName CompanionSelectionIndexedTag(const TCHAR* Prefix, const int32 Index)
	{
		return FName(*FString::Printf(TEXT("%s%02d"), Prefix, Index + 1));
	}

	TSharedRef<SWidget> MakeCompanionSelectionLabel(
		const FText& Text,
		const ET66FlatLabelRole Role,
		const FName Tag,
		const ETextJustify::Type Justification = ETextJustify::Left,
		const float WrapAt = 0.f)
	{
		TSharedRef<STextBlock> Label = SNew(STextBlock)
			.Text(Text)
			.Font(Role == ET66FlatLabelRole::Body || Role == ET66FlatLabelRole::Caption
				? FT66FlatStyle::MakeFont(Role == ET66FlatLabelRole::Caption ? 14 : 18)
				: FT66FlatStyle::MakeBoldFont(Role == ET66FlatLabelRole::Title ? 30 : 20))
			.ColorAndOpacity(Role == ET66FlatLabelRole::PurpleAccent
				? FT66FlatStyle::PurpleAccent()
				: (Role == ET66FlatLabelRole::Caption || Role == ET66FlatLabelRole::Body ? FT66FlatStyle::SecondaryText() : FT66FlatStyle::PrimaryText()))
			.Justification(Justification)
			.AutoWrapText(WrapAt > 0.f)
			.WrapTextAt(WrapAt)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(Label, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	}

	TSharedRef<SWidget> MakeCompanionSelectionColorRect(const FLinearColor& Color, const FName Tag, const FString& Role)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(Color)
			.Clipping(EWidgetClipping::ClipToBounds),
			Tag,
			Role,
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakeCompanionSelectionSwatch(const FLinearColor& Color, const FName Tag)
	{
		return FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(3.f),
			MakeCompanionSelectionColorRect(Color, NAME_None, TEXT("SwatchFill")),
			nullptr,
			Tag);
	}

}

UT66CompanionSelectionScreen::UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionSelection;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66CompanionSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

bool UT66CompanionSelectionScreen::IsCompanionUnlocked(FName CompanionID) const
{
	if (CompanionID.IsNone())
	{
		return true;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
		{
			return Unlocks->IsCompanionUnlocked(CompanionID);
		}
	}
	return true; // fail-open so we don't hard-lock the UI if subsystem is missing
}

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SkinSubsystem* Skin = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	FName CompanionForSkins = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;
	if (Skin && !CompanionForSkins.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(ET66SkinEntityType::Companion, CompanionForSkins);
	}
	else
	{
		T66SelectionScreenUtils::PopulateDefaultOwnedSkins(PlaceholderSkins);
	}
}

void UT66CompanionSelectionScreen::RefreshSkinsList()
{
	GeneratePlaceholderSkins();
	if (SkinsListBoxWidget.IsValid())
	{
		SkinsListBoxWidget->ClearChildren();
		AddSkinRowsToBox(SkinsListBoxWidget);
	}
	if (ACBalanceTextBlock.IsValid())
	{
		ACBalanceTextBlock->SetText(T66SelectionScreenUtils::FormatAchievementCoinBalance(
			GetLocSubsystem(),
			T66SelectionScreenUtils::GetAchievementCoinBalance(this)));
	}
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	static constexpr int32 BeachgoerPriceAC = UT66SkinSubsystem::DefaultSkinPriceAC;
	const FText BeachgoerPriceText = T66SelectionScreenUtils::FormatAchievementCoinBalance(Loc, BeachgoerPriceAC);

	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FName SkinIDCopy = Skin.SkinID;
		bool bIsDefault = Skin.bIsDefault;
		bool bIsOwned = Skin.bIsOwned;
		bool bIsEquipped = Skin.bIsEquipped;
		FName CID = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;
		const FName RowTag = bIsDefault
			? CompanionSelectionTag(TEXT("CompanionSelection.Skins.DefaultRow"))
			: CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerRow"));

		const FLinearColor SkinSwatchFill = bIsDefault
			? FLinearColor(0.13f, 0.08f, 0.04f, 1.0f)
			: FLinearColor(0.02f, 0.20f, 0.28f, 1.0f);

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 12.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(42.f)
				.HeightOverride(42.f)
				[
					MakeCompanionSelectionSwatch(
						SkinSwatchFill,
						bIsDefault ? CompanionSelectionTag(TEXT("CompanionSelection.Skins.DefaultSwatch")) : CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerSwatch")))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				MakeCompanionSelectionLabel(
					Loc ? Loc->GetText_SkinName(SkinIDCopy) : FText::FromName(SkinIDCopy),
					ET66FlatLabelRole::Body,
					bIsDefault ? CompanionSelectionTag(TEXT("CompanionSelection.Skins.DefaultName")) : CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerName")))
			];

		if (bIsDefault)
		{
			Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
				[
					bIsEquipped
					? MakeCompanionSelectionLabel(
						EquippedText,
						ET66FlatLabelRole::StatValue,
						CompanionSelectionTag(TEXT("CompanionSelection.Skins.DefaultEquipped")),
						ETextJustify::Center)
					: FT66FlatStyle::MakeFlatButton(
						ET66FlatState::Default,
						EquipText,
							FOnClicked::CreateLambda([this, CID]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, UT66SkinSubsystem::DefaultSkinID);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
						nullptr,
						nullptr,
						FMargin(10.f, 5.f),
						96.f,
						36.f,
						true,
						15,
						CompanionSelectionTag(TEXT("CompanionSelection.Skins.DefaultEquipButton")))
				];
		}
		else
		{
			Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(6.f, 0.f)
				[
					FT66FlatStyle::MakeFlatButton(
					PreviewedCompanionSkinIDOverride == SkinIDCopy ? ET66FlatState::Selected : ET66FlatState::Default,
					PreviewText,
					FOnClicked::CreateLambda([this, SkinIDCopy]()
					{
						PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinIDCopy) ? NAME_None : SkinIDCopy;
						UpdateCompanionDisplay();
						return FReply::Handled();
					}),
					nullptr,
					nullptr,
					FMargin(10.f, 5.f),
					96.f,
					36.f,
					true,
					15,
					CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerPreviewButton")))
				];
			const FText ActionText = !bIsOwned
				? FText::Format(NSLOCTEXT("T66.CompanionSelection", "BuyWithPrice", "{0} {1}"), BuyText, BeachgoerPriceText)
				: (bIsEquipped ? EquippedText : EquipText);
			if (bIsEquipped)
			{
				Row->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					MakeCompanionSelectionLabel(
						ActionText,
						ET66FlatLabelRole::StatValue,
						CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerEquipped")),
						ETextJustify::Center)
				];
			}
			else
			{
				Row->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					FT66FlatStyle::MakeFlatButton(
						!bIsOwned ? ET66FlatState::Selected : ET66FlatState::Default,
						ActionText,
						!bIsOwned
						? FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>();
								if (!SkinSub || !SkinSub->PurchaseCompanionSkin(CID, SkinIDCopy, BeachgoerPriceAC)) return FReply::Handled();
								SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
								PreviewedCompanionSkinIDOverride = NAME_None;
								RefreshSkinsList();
								return FReply::Handled();
							})
						: FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
						nullptr,
						nullptr,
						FMargin(10.f, 5.f),
						132.f,
						36.f,
						true,
						14,
						CompanionSelectionTag(TEXT("CompanionSelection.Skins.BeachgoerBuyButton")))
				];
			}
		}
		Box->AddSlot()
			.AutoHeight()
			.Padding(0.f, 5.f)
			[
				SNew(SBox)
				.HeightOverride(72.f)
				[
					FT66FlatStyle::MakeFlatSubPanel(
						bIsEquipped ? ET66FlatState::Selected : ET66FlatState::Default,
						FMargin(14.f, 9.f),
						Row,
						nullptr,
						RowTag)
				]
			];
	}
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	RefreshCompanionList();

	if (T66GI)
	{
		SelectedDifficulty = T66GI->ResolvePlayableDifficulty(SessionSubsystem ? SessionSubsystem->GetSharedLobbyDifficulty() : T66GI->SelectedDifficulty);
		T66GI->SelectedDifficulty = SelectedDifficulty;
	}

	if ((PreviewedCompanionID.IsNone() || !AllCompanionIDs.Contains(PreviewedCompanionID)) && T66GI)
	{
		if (!T66GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(T66GI->SelectedCompanionID))
		{
			PreviewedCompanionID = T66GI->SelectedCompanionID;
		}
		else if (AllCompanionIDs.Num() > 0)
		{
			PreviewedCompanionID = AllCompanionIDs[0];
		}
	}
	CurrentCompanionIndex = AllCompanionIDs.IndexOfByKey(PreviewedCompanionID);
	if (CurrentCompanionIndex == INDEX_NONE && AllCompanionIDs.Num() > 0)
	{
		CurrentCompanionIndex = 0;
		PreviewedCompanionID = AllCompanionIDs[0];
	}

	GeneratePlaceholderSkins();
	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	const FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.CompanionSelection", "Skins", "SKINS");
	const FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.CompanionSelection", "Lore", "LORE");
	const FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : NSLOCTEXT("T66.CompanionSelection", "ConfirmCompanion", "CONFIRM COMPANION");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText EnterText = NSLOCTEXT("T66.CompanionSelection", "EnterShort", "ENTER");
	const FText ReadyText = NSLOCTEXT("T66.CompanionSelection", "Ready", "READY");
	const FText UnreadyText = NSLOCTEXT("T66.CompanionSelection", "Unready", "UNREADY");
	const FText WaitingForPartyText = NSLOCTEXT("T66.CompanionSelection", "WaitingForParty", "WAITING FOR PARTY");
	const FText ACBalanceText = T66SelectionScreenUtils::FormatAchievementCoinBalance(Loc, T66SelectionScreenUtils::GetAchievementCoinBalance(this));

	DifficultyOptions.Empty();
	const TArray<ET66Difficulty> Difficulties = T66GI ? T66GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	for (const ET66Difficulty Difficulty : Difficulties)
	{
		DifficultyOptions.Add(MakeShared<FString>((Loc ? Loc->GetText_Difficulty(Difficulty) : FText::FromString(TEXT("?"))).ToString()));
	}
	const int32 CurrentDifficultyIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
	if (DifficultyOptions.IsValidIndex(CurrentDifficultyIndex))
	{
		CurrentDifficultyOption = DifficultyOptions[CurrentDifficultyIndex];
	}
	else if (DifficultyOptions.Num() > 0)
	{
		CurrentDifficultyOption = DifficultyOptions[0];
		SelectedDifficulty = Difficulties.IsValidIndex(0) ? Difficulties[0] : ET66Difficulty::Easy;
	}

	FText CurrentCompanionName = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanion", "NO COMPANION");
	FText CurrentCompanionLore = NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone.");
	FLinearColor PreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = ResolveCompanionLoreText(Loc, Data);
			PreviewColor = Data.PlaceholderColor;
		}
	}
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		PreviewColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	}

	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	const bool bIsLocalPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	const bool bPartyLobbyContextActive = SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive();
	const int32 LobbyPlayerCount = SessionSubsystem ? SessionSubsystem->GetCurrentLobbyPlayerCount() : 0;
	const bool bHasRemotePartyMembers = bPartyLobbyContextActive
		? LobbyPlayerCount > 1
		: (PartySubsystem && PartySubsystem->HasRemotePartyMembers());
	const bool bUsePartyReadyFlow = bPartyLobbyContextActive && (!bIsLocalPartyHost || bHasRemotePartyMembers);
	const bool bCanEditDifficulty = !bUsePartyReadyFlow || bIsLocalPartyHost;
	const bool bLocalReady = SessionSubsystem && SessionSubsystem->IsLocalLobbyReady();
	const bool bCanStartPartyRun = !bUsePartyReadyFlow || !SessionSubsystem || SessionSubsystem->AreAllPartyMembersReadyForGameplay();
	const FText PrimaryActionText = bUsePartyReadyFlow && !bIsLocalPartyHost
		? (bLocalReady ? UnreadyText : ReadyText)
		: (bCanStartPartyRun ? EnterText : WaitingForPartyText);

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};
	auto MakeTrackedLabel = [](TSharedPtr<STextBlock>& Target, const FText& Text, const int32 FontSize, const bool bBold, const FLinearColor& Color, const FName Tag, const ETextJustify::Type Justification = ETextJustify::Left, const float WrapAt = 0.f) -> TSharedRef<SWidget>
	{
		TSharedRef<STextBlock> TextWidget = SAssignNew(Target, STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.AutoWrapText(WrapAt > 0.f)
			.WrapTextAt(WrapAt)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(TextWidget, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	};
	auto MakeStaticLabel = [](const FText& Text, const int32 FontSize, const bool bBold, const FLinearColor& Color, const FName Tag, const ETextJustify::Type Justification = ETextJustify::Left, const float WrapAt = 0.f) -> TSharedRef<SWidget>
	{
		TSharedRef<STextBlock> TextWidget = SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.AutoWrapText(WrapAt > 0.f)
			.WrapTextAt(WrapAt)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(TextWidget, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	};

	AddSlot(0.f, 0.f, 1920.f, 1080.f,
		MakeCompanionSelectionColorRect(FLinearColor(0.01f, 0.01f, 0.014f, 0.38f), CompanionSelectionTag(TEXT("CompanionSelection.Background")), TEXT("Background")));
	AddSlot(0.f, 17.f, 548.f, 750.f,
		FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), SNullWidget::NullWidget, nullptr, CompanionSelectionTag(TEXT("CompanionSelection.LeftPanel"))));
	AddSlot(15.f, 31.f, 112.f, 31.f,
		FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, BackText, FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBackClicked), nullptr, nullptr, FMargin(10.f, 4.f), 0.f, 0.f, true, 15, CompanionSelectionTag(TEXT("CompanionSelection.BackButton"))));
	AddSlot(364.f, 31.f, 166.f, 31.f,
		FT66FlatStyle::AttachMetadata(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				MakeStaticLabel(NSLOCTEXT("T66.CompanionSelection", "BalanceLabel", "CC"), 14, true, FT66FlatStyle::SecondaryText(), CompanionSelectionTag(TEXT("CompanionSelection.BalanceIcon")), ETextJustify::Center)
			]
			+ SHorizontalBox::Slot().FillWidth(2.f).VAlign(VAlign_Center)
			[
				MakeTrackedLabel(ACBalanceTextBlock, ACBalanceText, 14, true, FT66FlatStyle::PrimaryText(), CompanionSelectionTag(TEXT("CompanionSelection.BalanceValue")), ETextJustify::Center)
			],
			CompanionSelectionTag(TEXT("CompanionSelection.BalanceBadge")), TEXT("Badge"), ET66FlatState::Default));
	AddSlot(206.f, 76.f, 136.f, 38.f,
		MakeStaticLabel(SkinsText, 24, true, FT66FlatStyle::SelectedText(), CompanionSelectionTag(TEXT("CompanionSelection.SkinsTitle")), ETextJustify::Center));
	AddSlot(19.f, 126.f, 510.f, 600.f,
		FT66FlatStyle::MakeFlatTransparentRegion(
			ET66FlatState::Default,
			FMargin(0.f),
			SNew(SScrollBox)
			+ SScrollBox::Slot()[SkinsListBoxWidget.ToSharedRef()],
			CompanionSelectionTag(TEXT("CompanionSelection.SkinsList"))));

	TSharedRef<SHorizontalBox> CarouselRow = SNew(SHorizontalBox);
	CarouselRow->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
	[
		FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, FText::FromString(TEXT("<")), FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked), nullptr, nullptr, FMargin(6.f), 54.f, 54.f, AllCompanionIDs.Num() > 1, 24, CompanionSelectionTag(TEXT("CompanionSelection.Carousel.PrevButton")))
	];
	for (int32 SlotIdx = 0; SlotIdx < HeroSelectionCarouselVisibleSlots; ++SlotIdx)
	{
		const int32 Offset = SlotIdx - HeroSelectionCarouselCenterIndex;
		const int32 Idx = AllCompanionIDs.Num() > 0 ? (CurrentCompanionIndex + Offset + AllCompanionIDs.Num() * 2) % AllCompanionIDs.Num() : INDEX_NONE;
		const FName CompanionID = AllCompanionIDs.IsValidIndex(Idx) ? AllCompanionIDs[Idx] : NAME_None;
		const bool bCenterSlot = Offset == 0;
		const bool bUnlocked = CompanionID.IsNone() || IsCompanionUnlocked(CompanionID);
		FName CompanionIDCopy = CompanionID;
		const TSharedRef<SWidget> Portrait = CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) && CompanionCarouselPortraitBrushes[SlotIdx].IsValid()
			? StaticCastSharedRef<SWidget>(SNew(SImage).Image(CompanionCarouselPortraitBrushes[SlotIdx].Get()).ColorAndOpacity(bUnlocked ? FLinearColor::White : FLinearColor(1.f, 1.f, 1.f, 0.35f)))
			: SNullWidget::NullWidget;
		CarouselRow->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
		[
			FT66FlatStyle::MakeFlatToggleGroupButton(
				bCenterSlot ? ET66FlatState::Selected : (bUnlocked ? ET66FlatState::Default : ET66FlatState::Disabled),
				SNew(SBox).Padding(6.f)[Portrait],
				FOnClicked::CreateLambda([this, CompanionIDCopy]()
				{
					if (!CompanionIDCopy.IsNone())
					{
						PreviewCompanion(CompanionIDCopy);
					}
					return FReply::Handled();
				}),
				FMargin(0.f),
				88.f,
				88.f,
				bUnlocked && !CompanionIDCopy.IsNone(),
				CompanionSelectionIndexedTag(TEXT("CompanionSelection.Carousel.Slot"), SlotIdx),
				CompanionSelectionTag(TEXT("CompanionSelectionCarousel")))
		];
	}
	CarouselRow->AddSlot().AutoWidth().VAlign(VAlign_Center)
	[
		FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, FText::FromString(TEXT(">")), FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked), nullptr, nullptr, FMargin(6.f), 54.f, 54.f, AllCompanionIDs.Num() > 1, 24, CompanionSelectionTag(TEXT("CompanionSelection.Carousel.NextButton")))
	];
	AddSlot(576.f, 11.f, 710.f, 130.f,
		FT66FlatStyle::MakeFlatTransparentRegion(ET66FlatState::Default, FMargin(10.f), CarouselRow, CompanionSelectionTag(TEXT("CompanionSelection.Carousel"))));

	AddSlot(576.f, 151.f, 710.f, 626.f,
		FT66FlatStyle::AttachMetadata(
			SNew(SOverlay)
			+ SOverlay::Slot()[MakeCompanionSelectionColorRect(FLinearColor(0.03f, 0.035f, 0.05f, 0.20f), NAME_None, TEXT("PreviewFill"))]
			+ SOverlay::Slot()[CreateCompanionPreviewWidget(PreviewColor)],
			CompanionSelectionTag(TEXT("CompanionSelection.PreviewPanel")), TEXT("PreviewPanel"), ET66FlatState::Default));

	AddSlot(1325.f, 11.f, 595.f, 756.f,
		FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), SNullWidget::NullWidget, nullptr, CompanionSelectionTag(TEXT("CompanionSelection.RightPanel"))));
	AddSlot(1392.f, 51.f, 462.f, 46.f,
		MakeTrackedLabel(CompanionNameWidget, CurrentCompanionName, 30, true, FT66FlatStyle::SelectedText(), CompanionSelectionTag(TEXT("CompanionSelection.CompanionName")), ETextJustify::Center));
	AddSlot(1372.f, 118.f, 502.f, 118.f,
		FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(10.f), MakeStaticLabel(NSLOCTEXT("T66.CompanionSelection", "PortraitStrip", "COMPANION PORTRAIT"), 16, true, FT66FlatStyle::SecondaryText(), CompanionSelectionTag(TEXT("CompanionSelection.PortraitLabel")), ETextJustify::Center), nullptr, CompanionSelectionTag(TEXT("CompanionSelection.PortraitPanel"))));
	AddSlot(1372.f, 261.f, 502.f, 64.f,
		FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(12.f),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.4f).VAlign(VAlign_Center)[MakeStaticLabel(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankLabel", "RANK"), 18, true, FT66FlatStyle::SecondaryText(), CompanionSelectionTag(TEXT("CompanionSelection.RankLabel")))]
			+ SHorizontalBox::Slot().FillWidth(0.6f).VAlign(VAlign_Center)[MakeTrackedLabel(CompanionRecordRankWidget, NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankDefault", "..."), 18, true, FT66FlatStyle::PrimaryText(), CompanionSelectionTag(TEXT("CompanionSelection.RankValue")), ETextJustify::Right)],
			nullptr,
			CompanionSelectionTag(TEXT("CompanionSelection.RankRow"))));
	AddSlot(1372.f, 343.f, 502.f, 96.f,
		SAssignNew(CompanionUnionBox, SBox)
		[
			FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Ready, FMargin(12.f),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.45f).VAlign(VAlign_Center)[MakeStaticLabel(NSLOCTEXT("T66.CompanionSelection", "CompanionUnityLabel", "UNITY"), 18, true, FT66FlatStyle::SecondaryText(), CompanionSelectionTag(TEXT("CompanionSelection.UnityLabel")))]
					+ SHorizontalBox::Slot().FillWidth(0.55f).VAlign(VAlign_Center)[MakeTrackedLabel(CompanionUnionText, NSLOCTEXT("T66.CompanionSelection", "UnityStagesDefault", "0 / 50"), 16, true, FT66FlatStyle::PrimaryText(), CompanionSelectionTag(TEXT("CompanionSelection.UnityValue")), ETextJustify::Right)]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[FT66FlatStyle::MakeFlatProgressBar(TAttribute<float>::CreateLambda([this]() { return FMath::Clamp(CompanionUnionProgress01, 0.f, 1.f); }), TOptional<FLinearColor>(FT66FlatStyle::GoodStandingGreen()), CompanionSelectionTag(TEXT("CompanionSelection.UnityProgress")))],
				nullptr,
				CompanionSelectionTag(TEXT("CompanionSelection.UnityRow")) )
		]);
	AddSlot(1372.f, 458.f, 502.f, 46.f,
		FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, LoreText, FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked), nullptr, nullptr, FMargin(12.f, 5.f), 0.f, 0.f, true, 18, CompanionSelectionTag(TEXT("CompanionSelection.LoreHeader"))));
	AddSlot(1372.f, 514.f, 502.f, 132.f,
		FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(12.f), MakeTrackedLabel(CompanionLoreWidget, CurrentCompanionLore, 16, false, FT66FlatStyle::SecondaryText(), CompanionSelectionTag(TEXT("CompanionSelection.LoreText")), ETextJustify::Left, 478.f), nullptr, CompanionSelectionTag(TEXT("CompanionSelection.LorePanel"))));
	AddSlot(1372.f, 664.f, 502.f, 70.f,
		FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(12.f), MakeTrackedLabel(CompanionUnionHealingText, FormatCompanionPassiveHealText(SelectedDifficulty), 15, false, FT66FlatStyle::PrimaryText(), CompanionSelectionTag(TEXT("CompanionSelection.PassiveText")), ETextJustify::Left, 478.f), nullptr, CompanionSelectionTag(TEXT("CompanionSelection.PassivePanel"))));

	TSharedRef<SHorizontalBox> PartyRow = SNew(SHorizontalBox);
	for (int32 SlotIdx = 0; SlotIdx < 4; ++SlotIdx)
	{
		PartyRow->AddSlot().FillWidth(1.f).Padding(SlotIdx > 0 ? FMargin(8.f, 0.f, 0.f, 0.f) : FMargin(0.f))
		[
			FT66FlatStyle::MakeFlatSubPanel(
				SlotIdx == 0 ? ET66FlatState::Ready : ET66FlatState::Default,
				FMargin(6.f),
				MakeStaticLabel(FText::Format(NSLOCTEXT("T66.CompanionSelection", "PartySlotFormat", "P{0}"), FText::AsNumber(SlotIdx + 1)), 18, true, SlotIdx == 0 ? FT66FlatStyle::GoodStandingGreen() : FT66FlatStyle::SecondaryText(), CompanionSelectionIndexedTag(TEXT("CompanionSelection.PartySlotLabel"), SlotIdx), ETextJustify::Center),
				nullptr,
				CompanionSelectionIndexedTag(TEXT("CompanionSelection.PartySlot"), SlotIdx))
		];
	}
	AddSlot(0.f, 799.f, 672.f, 216.f,
		FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(18.f), PartyRow, nullptr, CompanionSelectionTag(TEXT("CompanionSelection.PartyPanel"))));
	AddSlot(710.f, 799.f, 499.f, 216.f,
		FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(26.f),
			FT66FlatStyle::MakeFlatButton(
				ET66FlatState::Selected,
				ConfirmText,
				FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked),
				nullptr,
				nullptr,
				FMargin(20.f, 12.f),
				0.f,
				126.f,
				TAttribute<bool>::CreateLambda([this]() { return !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID); }),
				24,
				CompanionSelectionTag(TEXT("CompanionSelection.ConfirmButton"))),
			nullptr,
			CompanionSelectionTag(TEXT("CompanionSelection.ConfirmPanel"))));

	TSharedRef<SHorizontalBox> RunControls = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SBox).WidthOverride(230.f).HeightOverride(70.f).IsEnabled(bCanEditDifficulty)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					TAttribute<FText>::CreateLambda([this, Loc]()
					{
						return CurrentDifficultyOption.IsValid()
							? FText::FromString(*CurrentDifficultyOption)
							: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
					}),
					[this]()
					{
						TSharedRef<SVerticalBox> OptionsBox = SNew(SVerticalBox);
						for (const TSharedPtr<FString>& Opt : DifficultyOptions)
						{
							if (!Opt.IsValid())
							{
								continue;
							}
							TSharedPtr<FString> Captured = Opt;
							OptionsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
							[
								FT66FlatStyle::MakeFlatButton(
									CurrentDifficultyOption.IsValid() && *CurrentDifficultyOption == *Opt ? ET66FlatState::Selected : ET66FlatState::Default,
									FText::FromString(*Opt),
									FOnClicked::CreateLambda([this, Captured]()
									{
										OnDifficultyChanged(Captured, ESelectInfo::Direct);
										FSlateApplication::Get().DismissAllMenus();
										return FReply::Handled();
									}),
									nullptr,
									nullptr,
									FMargin(10.f, 6.f),
									210.f,
									42.f,
									true,
									16,
									NAME_None)
							];
						}
						return OptionsBox;
					},
					true,
					230.f,
					70.f,
					18,
					CompanionSelectionTag(TEXT("CompanionSelection.DifficultyDropdown")))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SBox).WidthOverride(170.f).HeightOverride(70.f).IsEnabled(bCanStartPartyRun)
			[
				FT66FlatStyle::MakeFlatButton(bCanStartPartyRun ? ET66FlatState::Selected : ET66FlatState::Disabled, PrimaryActionText, FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleEnterClicked), nullptr, nullptr, FMargin(12.f, 8.f), 170.f, 70.f, bCanStartPartyRun, 22, CompanionSelectionTag(TEXT("CompanionSelection.EnterButton")))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
		[
			FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, NSLOCTEXT("T66.CompanionSelection", "ChallengesButtonText", "CHALLENGES"), FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleChallengesClicked), nullptr, nullptr, FMargin(10.f, 8.f), 112.f, 70.f, true, 15, CompanionSelectionTag(TEXT("CompanionSelection.ChallengesButton")) )
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			FT66FlatStyle::MakeFlatButton(ET66FlatState::Default, NSLOCTEXT("T66.CompanionSelection", "ModsButtonText", "MODS"), FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleModsClicked), nullptr, nullptr, FMargin(10.f, 8.f), 88.f, 70.f, true, 15, CompanionSelectionTag(TEXT("CompanionSelection.ModsButton")) )
		];
	AddSlot(1248.f, 799.f, 653.f, 216.f,
		FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(26.f, 72.f, 26.f, 26.f), RunControls, nullptr, CompanionSelectionTag(TEXT("CompanionSelection.RunPanel"))));

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
		[
			Canvas
		];

	TSharedRef<SWidget> Root = FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootContent
			]
		],
		CompanionSelectionTag(TEXT("CompanionSelection.Root")),
		TEXT("Root"),
		ET66FlatState::Default);

	UpdateCompanionDisplay();
	return Root;
}
FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { bShowingLore = !bShowingLore; UpdateCompanionDisplay(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleEnterClicked() { OnEnterTribulationClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleChallengesClicked() { OnChallengesClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleModsClicked() { OnModsClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66CompanionSelectionScreen::OnDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type /*SelectInfo*/)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	const TArray<ET66Difficulty> Difficulties = GI ? GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	const int32 Index = DifficultyOptions.IndexOfByKey(NewValue);
	if (!Difficulties.IsValidIndex(Index))
	{
		return;
	}

	SelectedDifficulty = GI ? GI->ResolvePlayableDifficulty(Difficulties[Index]) : Difficulties[Index];
	CurrentDifficultyOption = NewValue;
	if (GI)
	{
		GI->SelectedDifficulty = SelectedDifficulty;
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(false);
		}
	}
	RefreshDifficultyDropdownText();
	UpdateCompanionDisplay();
	RefreshCompanionRecordRank();
	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewDifficulty(SelectedDifficulty);
	}
	if (AT66HeroPreviewStage* HeroStage = GetHeroPreviewStage())
	{
		HeroStage->SetPreviewDifficulty(SelectedDifficulty);
	}
}

void UT66CompanionSelectionScreen::RefreshDifficultyDropdownText()
{
	if (DifficultyOptions.Num() > 0)
	{
		UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
		const TArray<ET66Difficulty> Difficulties = GI ? GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
			ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
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

void UT66CompanionSelectionScreen::RefreshCompanionRecordRank()
{
	if (!CompanionRecordRankWidget.IsValid())
	{
		return;
	}

	CompanionRecordRankRequestKey.Reset();
	if (PreviewedCompanionID.IsNone())
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnavailable", "--"));
		return;
	}

	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66BackendSubsystem* Backend = GIBase ? GIBase->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankUnavailableNoBackend", "--"));
		return;
	}

	int32 PartySize = 1;
	if (T66GI)
	{
		if (UT66SessionSubsystem* SessionSubsystem = T66GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartyLobbyContextActive())
			{
				PartySize = FMath::Max(1, SessionSubsystem->GetCurrentLobbyPlayerCount());
			}
		}
		if (PartySize <= 1)
		{
			if (UT66PartySubsystem* PartySubsystem = T66GI->GetSubsystem<UT66PartySubsystem>())
			{
				PartySize = FMath::Max(1, PartySubsystem->GetPartyMemberCount());
			}
		}
	}

	const FString CompanionID = PreviewedCompanionID.ToString();
	const FString DifficultyKey = HeroSelectionDifficultyToApiString(SelectedDifficulty);
	const FString PartyKey = HeroSelectionPartySizeToApiString(PartySize);
	const FString RankKey = UT66BackendSubsystem::MakeMyRankCacheKey(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("companion"),
		CompanionID);
	CompanionRecordRankRequestKey = RankKey;

	bool bRankSuccess = false;
	int32 Rank = 0;
	int32 TotalEntries = 0;
	if (Backend->GetCachedMyRank(RankKey, bRankSuccess, Rank, TotalEntries))
	{
		static_cast<void>(TotalEntries);
		CompanionRecordRankWidget->SetText(bRankSuccess && Rank > 0 ? FormatCompanionRecordRankText(Rank) : FormatCompanionRecordRankText(0));
		return;
	}

	if (!Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
	{
		CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankOffline", "--"));
		return;
	}

	CompanionRecordRankWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "CompanionRecordRankPending", "..."));
	Backend->FetchMyRankFiltered(
		TEXT("score"),
		TEXT("alltime"),
		PartyKey,
		DifficultyKey,
		TEXT("companion"),
		CompanionID);
}

void UT66CompanionSelectionScreen::HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries)
{
	static_cast<void>(bSuccess);
	static_cast<void>(Rank);
	static_cast<void>(TotalEntries);

	if (!CompanionRecordRankRequestKey.Equals(Key) || !HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	RefreshCompanionRecordRank();
}

void UT66CompanionSelectionScreen::HandlePartyStateChanged()
{
	FT66Style::DeferRebuild(this);
}

void UT66CompanionSelectionScreen::HandleSessionStateChanged()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
			GI->SelectedDifficulty = SelectedDifficulty;
		}
	}
	RefreshDifficultyDropdownText();
	RefreshCompanionRecordRank();
	FT66Style::DeferRebuild(this);
}

AT66CompanionPreviewStage* UT66CompanionSelectionScreen::GetCompanionPreviewStage() const
{
	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(const_cast<UT66CompanionSelectionScreen*>(this)))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66CompanionPreviewStage* CachedStage = CachedCompanionPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: the frontend preview scene is resolved once and then
	// cached so carousel refreshes do not rescan the world.
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
	{
		CachedCompanionPreviewStage = *It;
		return CachedCompanionPreviewStage.Get();
	}
	return nullptr;
}

AT66HeroPreviewStage* UT66CompanionSelectionScreen::GetHeroPreviewStage() const
{
	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(const_cast<UT66CompanionSelectionScreen*>(this)))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}

	if (AT66HeroPreviewStage* CachedStage = CachedHeroPreviewStage.Get())
	{
		return CachedStage;
	}

	// UI setup fallback: cached after first resolve for this screen.
	for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
	{
		CachedHeroPreviewStage = *It;
		return CachedHeroPreviewStage.Get();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::CreateCompanionPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage();

	if (Stage)
	{
		const TWeakObjectPtr<AT66CompanionPreviewStage> WeakStage(Stage);
		// In-world preview: transparent overlay for drag-rotate/zoom.
		// The main viewport renders the companion with full Lumen GI behind the UI.
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateStagePreview)
				.DegreesPerPixel(0.28f)
				.OnRotateYaw(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float DeltaYaw)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewYaw(DeltaYaw);
					}
				}))
				.OnZoom(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float ZoomDelta)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewZoom(ZoomDelta);
						T66PositionCompanionPreviewCamera(PreviewStage);
					}
				}))
			];
	}
	return SAssignNew(CompanionPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	FName EffectiveSkin = PreviewedCompanionSkinIDOverride;
	if (EffectiveSkin.IsNone() && !PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				EffectiveSkin = SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
			}
		}
	}
	if (EffectiveSkin.IsNone()) EffectiveSkin = FName(TEXT("Default"));

	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			Stage->SetPreviewDifficulty(GI->SelectedDifficulty);
		}
		Stage->SetPreviewCompanion(PreviewedCompanionID, EffectiveSkin);
		T66PositionCompanionPreviewCamera(this);
	}
	else if (CompanionPreviewColorBox.IsValid())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(Data.PlaceholderColor);
		}
		else
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
		}

		if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));
		}
	}

	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanionTitle", "No Companion"));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				UT66LocalizationSubsystem* Loc = GetLocSubsystem();
				CompanionNameWidget->SetText(Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName);
			}
		}
	}

	if (CompanionLoreWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			CompanionLoreWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone."));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				CompanionLoreWidget->SetText(ResolveCompanionLoreText(GetLocSubsystem(), Data));
			}
		}
	}

	// Unity is profile progression only; healing strength is fixed by difficulty.
	if (CompanionUnionBox.IsValid())
	{
		const bool bShowUnion = !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID);
		CompanionUnionBox->SetVisibility(bShowUnion ? EVisibility::Visible : EVisibility::Collapsed);
	}

	CompanionUnionProgress01 = 0.f;
	CompanionUnionStagesCleared = 0;
	if (!PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				CompanionUnionStagesCleared = FMath::Max(0, Ach->GetCompanionUnionStagesCleared(PreviewedCompanionID));
				CompanionUnionProgress01 = FMath::Clamp(Ach->GetCompanionUnionProgress01(PreviewedCompanionID), 0.f, 1.f);

				const int32 Needed = UT66AchievementsSubsystem::UnionTier_HyperStages;
				if (CompanionUnionText.IsValid())
				{
					CompanionUnionText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "UnityStagesFormat", "{0} / {1}"),
						FText::AsNumber(CompanionUnionStagesCleared),
						FText::AsNumber(Needed)));
				}
			}
		}
	}
	if (CompanionUnionHealingText.IsValid())
	{
		CompanionUnionHealingText->SetText(FormatCompanionPassiveHealText(SelectedDifficulty));
	}
	RefreshCompanionRecordRank();

	RefreshCompanionCarouselPortraits();
}

void UT66CompanionSelectionScreen::RefreshCompanionCarouselPortraits()
{
	const int32 NumCarousel = AllCompanionIDs.Num();
	const int32 CarouselIndex = NumCarousel > 0 ? FMath::Clamp(CurrentCompanionIndex, 0, NumCarousel - 1) : 0;

	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; ++Offset)
		{
			const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
			if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}
			const int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
			const FName CompanionID = NumCarousel > 0 && AllCompanionIDs.IsValidIndex(Idx) ? AllCompanionIDs[Idx] : NAME_None;
			TSoftObjectPtr<UTexture2D> PortraitSoft;
			if (!CompanionID.IsNone())
			{
				FCompanionData D;
				if (GI->GetCompanionData(CompanionID, D))
				{
					PortraitSoft = !D.SelectionPortrait.IsNull() ? D.SelectionPortrait : D.Portrait;
				}
			}
			const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
			if (PortraitSoft.IsNull() || !TexPool)
			{
				CompanionCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, CompanionCarouselPortraitBrushes[SlotIdx], FName(TEXT("CompanionCarousel"), SlotIdx + 1), /*bClearWhileLoading*/ false);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66CompanionSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
	}
	RefreshCompanionList();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->PrimeHeroSelectionAssetsAsync();
		GI->PrimeHeroSelectionPreviewVisualsAsync();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66CompanionSelectionScreen::HandlePartyStateChanged);
		}
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66CompanionSelectionScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::CompanionSelection);
			SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
		}
		else
		{
			SelectedDifficulty = GI->ResolvePlayableDifficulty(GI->SelectedDifficulty);
		}
		GI->SelectedDifficulty = SelectedDifficulty;
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (BackendMyRankReadyHandle.IsValid())
			{
				Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
				BackendMyRankReadyHandle.Reset();
			}
			BackendMyRankReadyHandle = Backend->OnMyRankDataReady.AddUObject(this, &UT66CompanionSelectionScreen::HandleBackendMyRankDataReady);
		}
		if (!GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(GI->SelectedCompanionID))
		{
			PreviewCompanion(GI->SelectedCompanionID);
		}
		else if (AllCompanionIDs.Num() > 0)
		{
			PreviewCompanion(AllCompanionIDs[0]);
		}
	}
	T66PositionCompanionPreviewCamera(this);
}

void UT66CompanionSelectionScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
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

void UT66CompanionSelectionScreen::RefreshScreen_Implementation()
{
	FCompanionData Data;
	bool bIsNoCompanion = PreviewedCompanionID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		AllCompanionIDs = GI->GetAllCompanionIDs();
}

TArray<FCompanionData> UT66CompanionSelectionScreen::GetAllCompanions()
{
	TArray<FCompanionData> Companions;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& ID : AllCompanionIDs)
		{
			FCompanionData Data;
			if (GI->GetCompanionData(ID, Data)) Companions.Add(Data);
		}
	}
	return Companions;
}

bool UT66CompanionSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutData)
{
	if (PreviewedCompanionID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		return GI->GetCompanionData(PreviewedCompanionID, OutData);
	return false;
}

void UT66CompanionSelectionScreen::PreviewCompanion(FName ID)
{
	PreviewedCompanionID = ID;
	PreviewedCompanionSkinIDOverride = NAME_None;
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	RefreshSkinsList();
}

void UT66CompanionSelectionScreen::SelectNoCompanion() { PreviewCompanion(NAME_None); }

void UT66CompanionSelectionScreen::PreviewNextCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex = (FMath::Max(CurrentCompanionIndex, 0) + 1) % AllCompanionIDs.Num();
	PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::PreviewPreviousCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex = (FMath::Max(CurrentCompanionIndex, 0) - 1 + AllCompanionIDs.Num()) % AllCompanionIDs.Num();
	PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66CompanionSelectionScreen::OnCompanionLoreClicked() { if (!PreviewedCompanionID.IsNone()) ShowModal(ET66ScreenType::CompanionLore); }
void UT66CompanionSelectionScreen::OnConfirmCompanionClicked()
{
	// Locked companions cannot be confirmed/selected.
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->PersistRememberedSelectionDefaults();
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(false);
		}

		if (AT66HeroPreviewStage* HeroStage = GetHeroPreviewStage())
		{
			FName EffectiveHeroSkinID = GI->SelectedHeroSkinID;
			if (EffectiveHeroSkinID.IsNone())
			{
				EffectiveHeroSkinID = FName(TEXT("Default"));
			}

			HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
			HeroStage->SetPreviewDifficulty(GI->SelectedDifficulty);
			HeroStage->SetPreviewHero(GI->SelectedHeroID, GI->SelectedHeroBodyType, EffectiveHeroSkinID, GI->SelectedCompanionID);
		}
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OpenCommunityContent(const bool bOpenMods)
{
	const ET66CommunityContentKind ContentKind = bOpenMods
		? ET66CommunityContentKind::Mod
		: ET66CommunityContentKind::Challenge;

	ShowModal(ET66ScreenType::Challenges);

	UT66ChallengesScreen* ChallengesScreen = UIManager
		? Cast<UT66ChallengesScreen>(UIManager->GetCurrentScreen())
		: nullptr;
	if (!ChallengesScreen && UIManager)
	{
		ChallengesScreen = Cast<UT66ChallengesScreen>(UIManager->GetCurrentModal());
	}

	if (ChallengesScreen)
	{
		ChallengesScreen->OpenContentKind(ContentKind);
	}
}

void UT66CompanionSelectionScreen::OnChallengesClicked() { OpenCommunityContent(false); }

void UT66CompanionSelectionScreen::OnModsClicked() { OpenCommunityContent(true); }

void UT66CompanionSelectionScreen::OnEnterTribulationClicked()
{
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->PersistRememberedSelectionDefaults();
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	if (SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive() && GI)
	{
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
		GI->SelectedDifficulty = SelectedDifficulty;
		SessionSubsystem->SyncLocalLobbyProfile();

		if (!SessionSubsystem->IsLocalPlayerPartyHost())
		{
			SessionSubsystem->SetLocalLobbyReady(!SessionSubsystem->IsLocalLobbyReady());
			ForceRebuildSlate();
			return;
		}

		FString FailureReason;
		if (!SessionSubsystem->AreAllPartyMembersReadyForGameplay(&FailureReason))
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *FailureReason);
			ForceRebuildSlate();
			return;
		}

		if (UIManager) UIManager->HideAllUI();
		SessionSubsystem->StartGameplayTravel();
		return;
	}

	if (UIManager) UIManager->HideAllUI();
	if (GI)
	{
		GI->TransitionToGameplayLevel();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
	}
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

