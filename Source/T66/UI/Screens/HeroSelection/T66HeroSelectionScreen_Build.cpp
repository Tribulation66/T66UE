// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "TimerManager.h"
#include "Widgets/Input/SButton.h"

using namespace T66HeroSelectionPrivate;

namespace
{
constexpr float HeroSelectionCanvasW = 1920.f;
constexpr float HeroSelectionCanvasH = 1080.f;

FName HSName(const TCHAR* Tag)
{
return FName(Tag);
}

TSharedRef<SWidget> HSTaggedBox(const TCHAR* Tag, const TSharedRef<SWidget>& Content, const FString& Role = TEXT("Region"))
{
return FT66FlatStyle::AttachMetadata(
SNew(SBox)
.Visibility(EVisibility::SelfHitTestInvisible)
[
Content
],
HSName(Tag),
Role,
ET66FlatState::Default);
}

TSharedRef<SConstraintCanvas> HSMakeCanvas()
{
return SNew(SConstraintCanvas)
.Visibility(EVisibility::SelfHitTestInvisible);
}

TSharedRef<SWidget> HSTaggedText(
const TCHAR* Tag,
const FText& Text,
const int32 FontSize,
const FSlateColor& Color,
const ETextJustify::Type Justification = ETextJustify::Left)
{
return FT66FlatStyle::AttachMetadata(
SNew(STextBlock)
.Text(Text)
.Font(FT66FlatStyle::MakeBoldFont(FontSize))
.ColorAndOpacity(Color)
.Justification(Justification)
.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
HSName(Tag),
TEXT("Label"),
ET66FlatState::Default,
TOptional<FLinearColor>(),
false,
NAME_None,
true);
}

TSharedRef<SWidget> HSTextPanel(
const TCHAR* Tag,
const FText& Text,
const ET66FlatState State,
const int32 FontSize,
const FMargin Padding = FMargin(14.f, 8.f))
{
return FT66FlatStyle::MakeFlatPanel(
State,
Padding,
SNew(SBox)
.HAlign(HAlign_Center)
.VAlign(VAlign_Center)
[
SNew(STextBlock)
.Text(Text)
.Font(FT66FlatStyle::MakeBoldFont(FontSize))
.ColorAndOpacity(FT66FlatStyle::TextColorForState(State))
.Justification(ETextJustify::Center)
.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
],
nullptr,
HSName(Tag));
}

const FSlateBrush* HSLooseBrush(const FString& RelativePath, const FVector2D& ImageSize, const TCHAR* DebugLabel)
{
struct FCache
{
TSharedPtr<FSlateBrush> Brush;
TStrongObjectPtr<UTexture2D> Texture;
};

static TMap<FString, FCache> Cache;
const FString Key = FString::Printf(TEXT("%s|%.0fx%.0f"), *RelativePath, ImageSize.X, ImageSize.Y);
FCache& Entry = Cache.FindOrAdd(Key);
if (!Entry.Brush.IsValid())
{
Entry.Brush = MakeShared<FSlateBrush>();
Entry.Brush->DrawAs = ESlateBrushDrawType::Image;
Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
Entry.Brush->ImageSize = ImageSize;
}

if (!Entry.Texture.IsValid())
{
for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
{
if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
CandidatePath,
TextureFilter::TF_Trilinear,
DebugLabel))
{
Entry.Texture.Reset(Texture);
break;
}
}
}

Entry.Brush->ImageSize = ImageSize;
Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
}

void HSAddCanvasSlot(
const TSharedRef<SConstraintCanvas>& Canvas,
const float X,
const float Y,
const float W,
const float H,
const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Offset(FMargin(X, Y, W, H))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				Widget
			];
	}

	TSharedRef<SWidget> HSMakePanel(
		const TCHAR* Tag,
		const ET66FlatState State,
		const TSharedRef<SWidget>& Content,
		const FMargin Padding = FMargin(12.f))
	{
		return FT66FlatStyle::MakeFlatPanel(State, Padding, Content, nullptr, HSName(Tag));
	}

	TSharedRef<SWidget> HSMakeBackdropSlab(const TCHAR* Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.Visibility(EVisibility::HitTestInvisible)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66FlatStyle::BackgroundColor())
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SBox)
			],
			HSName(Tag),
			TEXT("BackdropSlab"),
			ET66FlatState::Default);
	}
}

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController();
	LastBuiltLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;

	RefreshHeroList();
	RefreshCompanionList();
	if (AllHeroIDs.Num() > 0 && PreviewedHeroID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
			GI && !GI->SelectedHeroID.IsNone() && AllHeroIDs.Contains(GI->SelectedHeroID))
		{
			PreviewedHeroID = GI->SelectedHeroID;
		}
		else
		{
			PreviewedHeroID = AllHeroIDs[0];
		}
	}
	if (PreviewedCompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			PreviewedCompanionID = GI->SelectedCompanionID;
		}
	}
	GeneratePlaceholderSkins();

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (SessionSubsystem)
	{
		SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	}

	DifficultyOptions.Empty();
	const TArray<ET66Difficulty> Difficulties = T66GI ? T66GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	for (ET66Difficulty Diff : Difficulties)
	{
		const FText DiffText = Loc ? Loc->GetText_Difficulty(Diff) : FText::FromString(TEXT("Easy"));
		DifficultyOptions.Add(MakeShared<FString>(DiffText.ToString()));
	}
	const int32 CurrentDiffIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
	CurrentDifficultyOption = DifficultyOptions.IsValidIndex(CurrentDiffIndex) ? DifficultyOptions[CurrentDiffIndex] : (DifficultyOptions.Num() > 0 ? DifficultyOptions[0] : nullptr);

	if (!HeroUltimateIconBrush.IsValid())
	{
		HeroUltimateIconBrush = MakeShared<FSlateBrush>();
		HeroUltimateIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroUltimateIconBrush->ImageSize = FVector2D(62.f, 62.f);
	}
	if (!HeroPassiveIconBrush.IsValid())
	{
		HeroPassiveIconBrush = MakeShared<FSlateBrush>();
		HeroPassiveIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroPassiveIconBrush->ImageSize = FVector2D(62.f, 62.f);
	}

	const int32 ACBalance = T66SelectionScreenUtils::GetAchievementCoinBalance(this);
	const FName CurrentFlatSkinID = T66GI && !T66GI->SelectedHeroSkinID.IsNone()
		? T66GI->SelectedHeroSkinID
		: FName(TEXT("Default"));
	const FName SkinSelectionGroup = HSName(TEXT("HeroSelection.SkinSelection"));
	const FName HeroCarouselGroup = HSName(TEXT("HeroSelection.HeroCarousel"));
	const FName GenderToggleGroup = HSName(TEXT("HeroSelection.GenderToggle"));
	const FSlateBrush* SkinDefaultBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/HeroSelection/Skins/skin_default_stub.png"), FVector2D(72.f, 72.f), TEXT("HeroSelectionDefaultSkinStub"));
	const FSlateBrush* SkinBeachBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/HeroSelection/Skins/skin_beachgoer_stub.png"), FVector2D(72.f, 72.f), TEXT("HeroSelectionBeachSkinStub"));
	const FSlateBrush* SteamBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/steam_placeholder.png"), FVector2D(64.f, 64.f), TEXT("HeroSelectionSteamStub"));
	const FSlateBrush* InfoBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/info.png"), FVector2D(26.f, 26.f), TEXT("HeroSelectionInfoIcon"));
	const FSlateBrush* LockBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/lock.png"), FVector2D(26.f, 26.f), TEXT("HeroSelectionLockIcon"));
	const FSlateBrush* SkullBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/skull.png"), FVector2D(30.f, 30.f), TEXT("HeroSelectionSkullIcon"));
	const FSlateBrush* LabBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/lab_flask.png"), FVector2D(32.f, 32.f), TEXT("HeroSelectionLabIcon"));
	const FSlateBrush* ChadIconBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/chad_icon.png"), FVector2D(24.f, 24.f), TEXT("HeroSelectionChadIcon"));
	const FSlateBrush* StacyIconBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/stacy_icon.png"), FVector2D(24.f, 24.f), TEXT("HeroSelectionStacyIcon"));
	const FSlateBrush* TicketBrush = HSLooseBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/ticket.png"), FVector2D(28.f, 28.f), TEXT("HeroSelectionTicketIcon"));

	auto MakeButton = [](const TCHAR* Tag, const FText& Label, const ET66FlatState State, FOnClicked OnClicked, const float MinWidth, const float Height, const int32 FontSize = 20, const TSharedPtr<SWidget>& Icon = nullptr, const FName ToggleGroup = NAME_None)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Label,
			MoveTemp(OnClicked),
			Icon,
			nullptr,
			FMargin(12.f, 7.f),
			MinWidth,
			Height,
			true,
			FontSize,
			HSName(Tag),
			ToggleGroup);
	};

	auto MakeIconWidget = [](const FSlateBrush* Brush, const FVector2D& Size, const FLinearColor& Tint) -> TSharedPtr<SWidget>
	{
		if (!Brush)
		{
			return nullptr;
		}
		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(Tint)
			];
	};

	auto MakeTaggedIconWidget = [](const TCHAR* Tag, const FSlateBrush* Brush, const FVector2D& Size, const FLinearColor& Tint) -> TSharedPtr<SWidget>
	{
		if (!Brush)
		{
			return nullptr;
		}

		return FT66FlatStyle::AttachMetadata(
			SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SImage)
				.Image(Brush)
				.ColorAndOpacity(Tint)
			],
			HSName(Tag),
			TEXT("Icon"),
			ET66FlatState::Default);
	};

	auto MakeSkinRow = [&](
		const TCHAR* RowTag,
		const TCHAR* PortraitTag,
		const TCHAR* NameTag,
		const FName SkinID,
		const FText& Name,
		const FSlateBrush* Brush,
		const float RowHeight,
		TFunction<void(const TSharedRef<SConstraintCanvas>&)> AddActions)
	{
		const ET66FlatState RowState = CurrentFlatSkinID == SkinID ? ET66FlatState::Selected : ET66FlatState::Default;
		const TSharedRef<SConstraintCanvas> RowCanvas = HSMakeCanvas();
		HSAddCanvasSlot(RowCanvas, 0.f, 0.f, 127.f, RowHeight,
			FT66FlatStyle::MakeFlatPortraitSlot(RowState, Brush, nullptr, FVector2D(118.f, RowHeight - 8.f), HSName(PortraitTag)));
		HSAddCanvasSlot(RowCanvas, 147.f, 20.f, 185.f, 34.f,
			HSTaggedText(NameTag, Name, 18, FT66FlatStyle::TextColorForState(RowState)));
		AddActions(RowCanvas);

		return FT66FlatStyle::MakeFlatToggleGroupButton(
			RowState,
			RowCanvas,
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleFlatSkinRowClicked, SkinID),
			FMargin(0.f),
			0.f,
			0.f,
			true,
			HSName(RowTag),
			SkinSelectionGroup);
	};

	auto MakeEmptyDrugSlot = [](const TCHAR* Tag)
	{
		return FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "FlatEmptyDrugSlot", "+"))
				.Font(FT66FlatStyle::MakeBoldFont(24))
				.ColorAndOpacity(FT66FlatStyle::SecondaryText())
			],
			nullptr,
			HSName(Tag));
	};

	auto MakePartySlot = [&](const TCHAR* Tag, const ET66FlatState State, const FSlateBrush* HeroPortraitBrush)
	{
		const FString TagString(Tag);
		const FName SteamAvatarTag(*FString::Printf(TEXT("%s.SteamAvatar"), *TagString));
		const FName HeroPortraitTag(*FString::Printf(TEXT("%s.HeroPortrait"), *TagString));
		const TSharedRef<SConstraintCanvas> SlotCanvas = HSMakeCanvas();
		HSAddCanvasSlot(SlotCanvas, 24.f, 18.f, 86.f, 86.f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(SteamBrush)
					.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				],
				SteamAvatarTag,
				TEXT("SteamAvatar"),
				ET66FlatState::Default));
		HSAddCanvasSlot(SlotCanvas, 86.f, 86.f, 50.f, 50.f,
			FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, HeroPortraitBrush, nullptr, FVector2D(42.f, 42.f), HeroPortraitTag));

		return FT66FlatStyle::MakeFlatPanel(
			State,
			FMargin(0.f),
			SlotCanvas,
			nullptr,
			HSName(Tag));
	};

	const TSharedRef<SConstraintCanvas> TopCanvas = HSMakeCanvas();
	HSAddCanvasSlot(TopCanvas, 0.f, 2.f, 150.f, 54.f,
		MakeButton(TEXT("HeroSelection.TopRow.BackButton"), NSLOCTEXT("T66.Common", "Back", "BACK"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked), 150.f, 54.f, 22));

	const TSharedRef<SConstraintCanvas> CarouselCanvas = HSMakeCanvas();
	HSAddCanvasSlot(CarouselCanvas, 0.f, 0.f, 46.f, 70.f,
		MakeButton(TEXT("HeroSelection.TopRow.HeroCarousel.LeftArrow"), FText::FromString(TEXT("<")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePrevClicked), 46.f, 70.f, 24));
	HeroCarouselPortraitBrushes.SetNum(HeroSelectionHeroCarouselVisibleSlots);
	HeroCarouselSlotVisibility.SetNum(HeroSelectionHeroCarouselVisibleSlots);
	HeroCarouselImageWidgets.SetNum(HeroSelectionHeroCarouselVisibleSlots);
	for (int32 Index = 0; Index < HeroSelectionHeroCarouselVisibleSlots; ++Index)
	{
		if (!HeroCarouselPortraitBrushes[Index].IsValid())
		{
			HeroCarouselPortraitBrushes[Index] = MakeShared<FSlateBrush>();
			HeroCarouselPortraitBrushes[Index]->DrawAs = ESlateBrushDrawType::Image;
			HeroCarouselPortraitBrushes[Index]->Tiling = ESlateBrushTileType::NoTile;
			HeroCarouselPortraitBrushes[Index]->TintColor = FSlateColor(FLinearColor::White);
		}
	}
	RefreshHeroCarouselPortraits();
	for (int32 Index = 0; Index < HeroSelectionHeroCarouselVisibleSlots; ++Index)
	{
		const bool bSelected = Index == HeroSelectionHeroCarouselCenterIndex;
		const float X = 65.f + Index * 82.f;
		const FString Tag = FString::Printf(TEXT("HeroSelection.TopRow.HeroCarousel.Portrait%02d"), Index + 1);
		const TSharedPtr<FSlateBrush> PortraitBrush = HeroCarouselPortraitBrushes.IsValidIndex(Index)
			? HeroCarouselPortraitBrushes[Index]
			: nullptr;
		HSAddCanvasSlot(CarouselCanvas, X, Index < 5 ? 1.f : 2.f, 67.f, Index < 5 ? 72.f : 71.f,
			FT66FlatStyle::MakeFlatToggleGroupButton(
				bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
				FT66FlatStyle::MakeFlatPortraitSlot(
					bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
					PortraitBrush.IsValid() ? PortraitBrush.Get() : nullptr,
					nullptr,
					FVector2D(67.f, Index < 5 ? 72.f : 71.f)),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroCarouselPortraitClicked, Index),
				FMargin(0.f),
				0.f,
				0.f,
				true,
				FName(*Tag),
				HeroCarouselGroup));
	}
	HSAddCanvasSlot(CarouselCanvas, 644.f, 0.f, 44.f, 70.f,
		MakeButton(TEXT("HeroSelection.TopRow.HeroCarousel.RightArrow"), FText::FromString(TEXT(">")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked), 44.f, 70.f, 24));
	HSAddCanvasSlot(TopCanvas, 570.f, 0.f, 688.f, 76.f,
		HSTaggedBox(TEXT("HeroSelection.TopRow.HeroCarousel"), CarouselCanvas, TEXT("HeroCarousel")));

	const TSharedRef<SConstraintCanvas> SkinsCanvas = HSMakeCanvas();
	HSAddCanvasSlot(SkinsCanvas, 23.f, 17.f, 120.f, 32.f,
		HSTaggedText(TEXT("HeroSelection.LeftColumn.SkinsPanel.Header"), NSLOCTEXT("T66.HeroSelection", "FlatSkins", "SKINS"), 25, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(SkinsCanvas, 413.f, 17.f, 48.f, 28.f,
		MakeTaggedIconWidget(TEXT("HeroSelection.LeftColumn.SkinsPanel.TicketBadge"), TicketBrush, FVector2D(28.f, 28.f), FLinearColor::White).ToSharedRef());
	HSAddCanvasSlot(SkinsCanvas, 486.f, 17.f, 36.f, 28.f,
		HSTaggedText(TEXT("HeroSelection.LeftColumn.SkinsPanel.TicketValue"), FText::AsNumber(ACBalance), 17, FSlateColor(FT66FlatStyle::PrimaryText()), ETextJustify::Center));
	HSAddCanvasSlot(SkinsCanvas, 19.f, 59.f, 495.f, 112.f,
		MakeSkinRow(
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default"),
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Portrait"),
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Name"),
			FName(TEXT("Default")),
			NSLOCTEXT("T66.HeroSelection", "FlatSkinDefault", "DEFAULT"),
			SkinDefaultBrush,
			112.f,
			[](const TSharedRef<SConstraintCanvas>& RowCanvas)
			{
				HSAddCanvasSlot(RowCanvas, 357.f, 52.f, 121.f, 43.f,
					HSTextPanel(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.EquippedBadge"), NSLOCTEXT("T66.HeroSelection", "FlatEquipped", "EQUIPPED"), ET66FlatState::Selected, 13, FMargin(8.f, 5.f)));
			}));
	HSAddCanvasSlot(SkinsCanvas, 19.f, 184.f, 495.f, 111.f,
		MakeSkinRow(
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer"),
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Portrait"),
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Name"),
			FName(TEXT("Beachgoer")),
			NSLOCTEXT("T66.HeroSelection", "FlatSkinHeroDemo", "DEMO"),
			SkinBeachBrush,
			111.f,
			[&](const TSharedRef<SConstraintCanvas>& RowCanvas)
			{
				HSAddCanvasSlot(RowCanvas, 147.f, 56.f, 134.f, 42.f,
					MakeButton(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.PreviewButton"), NSLOCTEXT("T66.Common", "Preview", "PREVIEW"), ET66FlatState::Default, FOnClicked::CreateLambda([]()
					{
						UE_LOG(LogT66HeroSelection, Warning, TEXT("Action SkinPreview clicked - backend not yet implemented"));
						return FReply::Handled();
					}), 0.f, 42.f, 13));
				HSAddCanvasSlot(RowCanvas, 357.f, 56.f, 121.f, 42.f,
					HSTextPanel(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Beachgoer.Cost"), FText::FromString(TEXT("50")), ET66FlatState::Default, 16, FMargin(8.f, 5.f)));
			}));

	const TSharedRef<SConstraintCanvas> DrugsCanvas = HSMakeCanvas();
	HSAddCanvasSlot(DrugsCanvas, 23.f, 11.f, 121.f, 32.f,
		HSTaggedText(TEXT("HeroSelection.LeftColumn.DrugsPanel.Header"), NSLOCTEXT("T66.HeroSelection", "FlatDrugs", "DRUGS"), 25, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(DrugsCanvas, 19.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot01")));
	HSAddCanvasSlot(DrugsCanvas, 94.f, 45.f, 56.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot02")));
	HSAddCanvasSlot(DrugsCanvas, 169.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot03")));
	HSAddCanvasSlot(DrugsCanvas, 238.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot04")));
	HSAddCanvasSlot(DrugsCanvas, 326.f, 44.f, 77.f, 50.f,
		MakeButton(TEXT("HeroSelection.LeftColumn.DrugsPanel.BuyButton"), NSLOCTEXT("T66.Common", "Buy", "BUY"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, 0), 0.f, 50.f, 16));
	HSAddCanvasSlot(DrugsCanvas, 421.f, 44.f, 92.f, 50.f,
		MakeButton(TEXT("HeroSelection.LeftColumn.DrugsPanel.ClearButton"), NSLOCTEXT("T66.Common", "Clear", "CLEAR"), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleClearTemporaryBuffsClicked), 0.f, 50.f, 16));

	const TSharedRef<SConstraintCanvas> LeftCanvas = HSMakeCanvas();
	HSAddCanvasSlot(LeftCanvas, 0.f, 0.f, 534.f, 574.f, HSMakePanel(TEXT("HeroSelection.LeftColumn.SkinsPanel"), ET66FlatState::Default, SkinsCanvas, FMargin(0.f)));
	HSAddCanvasSlot(LeftCanvas, 0.f, 591.f, 534.f, 120.f, HSMakePanel(TEXT("HeroSelection.LeftColumn.DrugsPanel"), ET66FlatState::Default, DrugsCanvas, FMargin(0.f)));

	TSharedPtr<SBox> CharacterPreviewHost;
	const FLinearColor CharacterPreviewFallbackColor = FLinearColor::Transparent;
	TSharedRef<SWidget> CharacterPreview = HeroPreviewController
		? HeroPreviewController->CreateHeroPreviewWidget(CharacterPreviewFallbackColor)
		: HSTextPanel(TEXT("HeroSelection.MiddleColumn.CharacterRender.Fallback"), FText::FromString(TEXT("GEORGE")), ET66FlatState::Default, 28);
	SAssignNew(CharacterPreviewHost, SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CharacterPreview
		];
	const TSharedRef<SOverlay> PreviewOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			HSTaggedBox(TEXT("HeroSelection.MiddleColumn.CharacterRender"), CharacterPreviewHost.ToSharedRef(), TEXT("CharacterRender"))
		];

	UT66AchievementsSubsystem* Achievements = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	auto GetPreviewHeroMasteryLevel = [this, Achievements]() -> int32
	{
		return Achievements ? Achievements->GetHeroMasteryLevel(PreviewedHeroID) : 1;
	};
	auto GetPreviewHeroMasteryXP = [this, Achievements]() -> int32
	{
		return Achievements ? Achievements->GetHeroMasteryXP(PreviewedHeroID) : 0;
	};
	auto MakeDynamicTaggedText = [](const TCHAR* Tag, TAttribute<FText> Text, const int32 FontSize, const FSlateColor& Color, const ETextJustify::Type Justification = ETextJustify::Left) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(FT66FlatStyle::MakeBoldFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			HSName(Tag),
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	const TSharedRef<SConstraintCanvas> RankCanvas = HSMakeCanvas();
	HSAddCanvasSlot(RankCanvas, 13.f, 15.f, 25.f, 25.f,
		FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, InfoBrush, nullptr, FVector2D(25.f, 25.f), HSName(TEXT("HeroSelection.RightColumn.RankPanel.InfoIcon"))));
	HSAddCanvasSlot(RankCanvas, 51.f, 16.f, 68.f, 30.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.RankPanel.Label"), NSLOCTEXT("T66.HeroSelection", "FlatRank", "RANK"), 20, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(RankCanvas, 124.f, 15.f, 23.f, 25.f,
		FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, LockBrush, nullptr, FVector2D(23.f, 25.f), HSName(TEXT("HeroSelection.RightColumn.RankPanel.LockIcon"))));
	HSAddCanvasSlot(RankCanvas, 516.f, 16.f, 24.f, 30.f,
		FT66FlatStyle::AttachMetadata(
			SAssignNew(HeroRecordRankWidget, STextBlock)
			.Text(FText::FromString(TEXT("--")))
			.Font(FT66FlatStyle::MakeBoldFont(20))
			.ColorAndOpacity(FSlateColor(FT66FlatStyle::PrimaryText()))
			.Justification(ETextJustify::Right)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			HSName(TEXT("HeroSelection.RightColumn.RankPanel.Value")),
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true));

	const TSharedRef<SConstraintCanvas> MasteryCanvas = HSMakeCanvas();
	HSAddCanvasSlot(MasteryCanvas, 13.f, 16.f, 25.f, 25.f,
		FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, InfoBrush, nullptr, FVector2D(25.f, 25.f), HSName(TEXT("HeroSelection.RightColumn.MasteryPanel.InfoIcon"))));
	HSAddCanvasSlot(MasteryCanvas, 51.f, 16.f, 100.f, 30.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.MasteryPanel.Label"), NSLOCTEXT("T66.HeroSelection", "FlatMastery", "MASTERY"), 20, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(MasteryCanvas, 177.f, 22.f, 177.f, 19.f,
		FT66FlatStyle::MakeFlatProgressBar(TAttribute<float>::CreateLambda([this, Achievements]() { return Achievements ? Achievements->GetHeroMasteryProgress01(PreviewedHeroID) : 0.0f; }), TOptional<FLinearColor>(FT66FlatStyle::ReadyBorder()), HSName(TEXT("HeroSelection.RightColumn.MasteryPanel.ProgressBar"))));
	HSAddCanvasSlot(MasteryCanvas, 410.f, 16.f, 58.f, 30.f,
		MakeDynamicTaggedText(TEXT("HeroSelection.RightColumn.MasteryPanel.Level"), TAttribute<FText>::CreateLambda([GetPreviewHeroMasteryLevel]() { return FText::Format(NSLOCTEXT("T66.HeroSelection", "MasteryLevelFormat", "LV {0}"), FText::AsNumber(GetPreviewHeroMasteryLevel())); }), 20, FSlateColor(FT66FlatStyle::PrimaryText()), ETextJustify::Right));
	HSAddCanvasSlot(MasteryCanvas, 479.f, 16.f, 92.f, 30.f,
		MakeDynamicTaggedText(TEXT("HeroSelection.RightColumn.MasteryPanel.XP"), TAttribute<FText>::CreateLambda([GetPreviewHeroMasteryXP]() { return FText::Format(NSLOCTEXT("T66.HeroSelection", "MasteryXPFormat", "{0} / 100 XP"), FText::AsNumber(GetPreviewHeroMasteryXP() % 100)); }), 16, FSlateColor(FT66FlatStyle::SecondaryText()), ETextJustify::Right));

	const TSharedRef<SConstraintCanvas> StatsCanvas = HSMakeCanvas();
	HSAddCanvasSlot(StatsCanvas, 223.f, 0.f, 115.f, 32.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Header"), FText::FromString(TEXT("STATS")), 22, FSlateColor(FT66FlatStyle::PrimaryText()), ETextJustify::Center));
	HSAddCanvasSlot(StatsCanvas, 27.f, 44.f, 198.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Damage"), FText::FromString(TEXT("Damage 4/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 27.f, 86.f, 198.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.AttSpeed"), FText::FromString(TEXT("ATT Speed 2/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 27.f, 128.f, 198.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.AttScale"), FText::FromString(TEXT("ATT Scale 2/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 27.f, 168.f, 198.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Accuracy"), FText::FromString(TEXT("Accuracy 2/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 295.f, 44.f, 242.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Armor"), FText::FromString(TEXT("Armor 7/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 295.f, 86.f, 242.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Evasion"), FText::FromString(TEXT("Evasion 1/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 295.f, 128.f, 242.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Luck"), FText::FromString(TEXT("Luck 2/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(StatsCanvas, 295.f, 168.f, 242.f, 28.f, HSTaggedText(TEXT("HeroSelection.RightColumn.StatsPanel.Speed"), FText::FromString(TEXT("Speed 2/99")), 16, FSlateColor(FT66FlatStyle::PrimaryText())));

	const TSharedRef<SConstraintCanvas> WeaponUltimateCanvas = HSMakeCanvas();
	HSAddCanvasSlot(WeaponUltimateCanvas, 2.f, 0.f, 275.f, 163.f,
		FT66FlatStyle::AttachMetadata(SNew(SBox), HSName(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.WeaponColumn")), TEXT("WeaponColumn"), ET66FlatState::Default));
	HSAddCanvasSlot(WeaponUltimateCanvas, 278.f, 0.f, 279.f, 163.f,
		FT66FlatStyle::AttachMetadata(SNew(SBox), HSName(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.UltimateColumn")), TEXT("UltimateColumn"), ET66FlatState::Default));
	HSAddCanvasSlot(WeaponUltimateCanvas, 180.f, 19.f, 106.f, 28.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.WeaponColumn.Label"), FText::FromString(TEXT("WEAPON")), 15, FSlateColor(FT66FlatStyle::SecondaryText()), ETextJustify::Center));
	HSAddCanvasSlot(WeaponUltimateCanvas, 98.f, 43.f, 117.f, 73.f,
		FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, HeroPassiveIconBrush.Get(), nullptr, FVector2D(74.f, 74.f), HSName(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.WeaponIcon"))));
	HSAddCanvasSlot(WeaponUltimateCanvas, 349.f, 19.f, 136.f, 28.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.UltimateColumn.Label"), FText::FromString(TEXT("ULTIMATE")), 15, FSlateColor(FT66FlatStyle::SecondaryText()), ETextJustify::Center));
	HSAddCanvasSlot(WeaponUltimateCanvas, 368.f, 30.f, 146.f, 81.f,
		FT66FlatStyle::MakeFlatPortraitSlot(ET66FlatState::Default, HeroUltimateIconBrush.Get(), nullptr, FVector2D(74.f, 74.f), HSName(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel.UltimateIcon"))));

	const TSharedRef<SConstraintCanvas> RightContent = HSMakeCanvas();
	HSAddCanvasSlot(RightContent, 31.f, 32.f, 186.f, 50.f,
		FT66FlatStyle::MakeFlatLabel(
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66HeroSelectionScreen::GetPreviewedHeroTitleText)),
			ET66FlatLabelRole::Title,
			ETextJustify::Left,
			HSName(TEXT("HeroSelection.RightColumn.HeaderRow.HeroName"))));
	HSAddCanvasSlot(RightContent, 455.f, 24.f, 119.f, 50.f,
		MakeButton(TEXT("HeroSelection.RightColumn.HeaderRow.LabButton"), NSLOCTEXT("T66.HeroSelection", "FlatLab", "LAB"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLabClicked), 119.f, 50.f, 22, MakeIconWidget(LabBrush, FVector2D(22.f, 22.f), FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(RightContent, 31.f, 97.f, 363.f, 24.f,
		FT66FlatStyle::MakeFlatLabel(
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66HeroSelectionScreen::GetPreviewedHeroSubtitleText)),
			ET66FlatLabelRole::PurpleAccent,
			ETextJustify::Left,
			HSName(TEXT("HeroSelection.RightColumn.Subtitle"))));
	HSAddCanvasSlot(RightContent, 18.f, 154.f, 557.f, 56.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.RankPanel"), ET66FlatState::Default, RankCanvas, FMargin(0.f)));
	HSAddCanvasSlot(RightContent, 18.f, 225.f, 557.f, 60.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.MasteryPanel"), ET66FlatState::Default, MasteryCanvas, FMargin(0.f)));
	HSAddCanvasSlot(RightContent, 16.f, 318.f, 559.f, 219.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.StatsPanel"), ET66FlatState::Default, HSTaggedBox(TEXT("HeroSelection.RightColumn.StatsPanel.Grid"), StatsCanvas, TEXT("StatsGrid")), FMargin(0.f)));
	HSAddCanvasSlot(RightContent, 16.f, 553.f, 559.f, 165.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.WeaponUltimatePanel"), ET66FlatState::Default, WeaponUltimateCanvas, FMargin(0.f)));

	const TSharedRef<SConstraintCanvas> MainBodyCanvas = HSMakeCanvas();
	HSAddCanvasSlot(MainBodyCanvas, 17.f, 92.f, 534.f, 711.f, HSTaggedBox(TEXT("HeroSelection.LeftColumn"), LeftCanvas, TEXT("Column")));
	HSAddCanvasSlot(MainBodyCanvas, 576.f, 102.f, 712.f, 680.f,
		HSTaggedBox(
			TEXT("HeroSelection.MiddleColumn"),
			FT66FlatStyle::MakeFlatTransparentRegion(
				ET66FlatState::Default,
				FMargin(10.f),
				PreviewOverlay,
				HSName(TEXT("HeroSelection.MiddleColumn.CharacterPreviewPanel"))),
			TEXT("Column")));
	HSAddCanvasSlot(MainBodyCanvas, 1311.f, 17.f, 591.f, 772.f,
		HSTaggedBox(
			TEXT("HeroSelection.RightColumn"),
			HSMakePanel(TEXT("HeroSelection.RightColumn.OuterPanel"), ET66FlatState::Default, RightContent, FMargin(0.f)),
			TEXT("Column")));

	static bool bLoggedSteamAvatarPlaceholder = false;
	if (!bLoggedSteamAvatarPlaceholder)
	{
		UE_LOG(LogT66HeroSelection, Warning, TEXT("TODO: Hero Selection Steam party slots use a placeholder Steam avatar until Steam profile image integration is exposed to the UI layer."));
		bLoggedSteamAvatarPlaceholder = true;
	}

	const TSharedRef<SConstraintCanvas> PartySlots = HSMakeCanvas();
	HSAddCanvasSlot(PartySlots, 45.f, 13.f, 120.f, 30.f,
		HSTaggedText(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot01.ReadyBadge"), FText::FromString(TEXT("READY")), 18, FSlateColor(FT66FlatStyle::ReadyBorder()), ETextJustify::Center));
	HSAddCanvasSlot(PartySlots, 23.f, 52.f, 146.f, 146.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot01"), ET66FlatState::Ready, SkinDefaultBrush));
	HSAddCanvasSlot(PartySlots, 190.f, 52.f, 146.f, 146.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot02"), ET66FlatState::Default, SkinDefaultBrush));
	HSAddCanvasSlot(PartySlots, 347.f, 52.f, 146.f, 146.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot03"), ET66FlatState::Default, SkinBeachBrush));
	HSAddCanvasSlot(PartySlots, 504.f, 52.f, 146.f, 146.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot04"), ET66FlatState::Default, SkinDefaultBrush));

	const TSharedRef<SConstraintCanvas> CompanionPanel = HSMakeCanvas();
	FT66FlatToggleGroupParams GenderToggleParams;
	GenderToggleParams.GroupName = GenderToggleGroup;
	GenderToggleParams.bMutuallyExclusive = true;
	FT66FlatToggleGroupItem ChadToggleItem;
	ChadToggleItem.Label = FText::FromString(TEXT("CHAD"));
	ChadToggleItem.bIsSelected = T66BodyTypeAliases::IsChad(SelectedBodyType);
	ChadToggleItem.OnClicked = FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChadBodyClicked);
	ChadToggleItem.OptionalLeftIcon = MakeTaggedIconWidget(TEXT("HeroSelection.BottomRow.CompanionPanel.ChadButton.Icon"), ChadIconBrush, FVector2D(22.f, 22.f), FLinearColor::FromSRGBColor(FColor(60, 200, 240)));
	ChadToggleItem.MinWidth = 0.f;
	ChadToggleItem.Height = 58.f;
	ChadToggleItem.FontSize = 16;
	ChadToggleItem.Tag = HSName(TEXT("HeroSelection.BottomRow.CompanionPanel.ChadButton"));
	GenderToggleParams.Items.Add(ChadToggleItem);
	FT66FlatToggleGroupItem StacyToggleItem;
	StacyToggleItem.Label = FText::FromString(TEXT("STACY"));
	StacyToggleItem.bIsSelected = T66BodyTypeAliases::IsStacy(SelectedBodyType);
	StacyToggleItem.OnClicked = FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStacyBodyClicked);
	StacyToggleItem.OptionalLeftIcon = MakeTaggedIconWidget(TEXT("HeroSelection.BottomRow.CompanionPanel.StacyButton.Icon"), StacyIconBrush, FVector2D(22.f, 22.f), FLinearColor::FromSRGBColor(FColor(240, 100, 180)));
	StacyToggleItem.MinWidth = 0.f;
	StacyToggleItem.Height = 58.f;
	StacyToggleItem.FontSize = 16;
	StacyToggleItem.Tag = HSName(TEXT("HeroSelection.BottomRow.CompanionPanel.StacyButton"));
	GenderToggleParams.Items.Add(StacyToggleItem);
	const TArray<TSharedRef<SWidget>> GenderToggleButtons = FT66FlatStyle::MakeFlatToggleGroup(GenderToggleParams);
	HSAddCanvasSlot(CompanionPanel, 19.f, 11.f, 209.f, 58.f,
		GenderToggleButtons[0]);
	HSAddCanvasSlot(CompanionPanel, 257.f, 13.f, 202.f, 58.f,
		GenderToggleButtons[1]);
	HSAddCanvasSlot(CompanionPanel, 19.f, 103.f, 374.f, 72.f,
		MakeButton(TEXT("HeroSelection.BottomRow.CompanionPanel.ChooseCompanionButton"), FText::FromString(TEXT("CHOOSE COMPANION")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked), 0.f, 72.f, 15));

	auto MakeDifficultyMenu = [this, Loc, Difficulties]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		for (ET66Difficulty Difficulty : Difficulties)
		{
			const FText Label = Loc ? Loc->GetText_Difficulty(Difficulty) : FText::FromString(TEXT("Easy"));
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				FT66FlatStyle::MakeFlatButton(
					Difficulty == SelectedDifficulty ? ET66FlatState::Selected : ET66FlatState::Default,
					Label,
					FOnClicked::CreateLambda([this, Difficulty]()
					{
						SelectDifficulty(Difficulty);
						return FReply::Handled();
					}),
					nullptr,
					nullptr,
					FMargin(10.f, 6.f),
					160.f,
					36.f,
					true,
					16)
			];
		}
		return HSMakePanel(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown.Menu"), ET66FlatState::Default, Menu, FMargin(6.f));
	};

	const FText DifficultyText = CurrentDifficultyOption.IsValid()
		? FText::FromString(*CurrentDifficultyOption)
		: FText::FromString(TEXT("Easy"));
	const TSharedRef<SVerticalBox> DifficultyPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			HSTaggedText(TEXT("HeroSelection.BottomRow.DifficultyPanel.Label"), FText::FromString(TEXT("DIFFICULTY")), 16, FSlateColor(FT66FlatStyle::PrimaryText()))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			FT66FlatStyle::MakeFlatDropdown(ET66FlatState::Default, TAttribute<FText>(DifficultyText), MakeDifficultyMenu, false, 0.f, 56.f, 20, HSName(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown")))
		];

	const TSharedRef<SConstraintCanvas> RightClusterCanvas = HSMakeCanvas();
	HSAddCanvasSlot(RightClusterCanvas, 21.f, 35.f, 156.f, 109.f,
		HSMakePanel(TEXT("HeroSelection.BottomRow.DifficultyPanel"), ET66FlatState::Default, DifficultyPanel, FMargin(10.f)));
	HSAddCanvasSlot(RightClusterCanvas, 209.f, 23.f, 204.f, 160.f,
		MakeButton(TEXT("HeroSelection.BottomRow.DifficultyPanel.EnterButton"), FText::FromString(TEXT("ENTER")), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked), 0.f, 160.f, 28, MakeIconWidget(SkullBrush, FVector2D(30.f, 30.f), FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(RightClusterCanvas, 440.f, 23.f, 192.f, 70.f,
		MakeButton(TEXT("HeroSelection.BottomRow.ChallengesButton"), FText::FromString(TEXT("CHALLENGES")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChallengesClicked), 0.f, 48.f, 16));
	HSAddCanvasSlot(RightClusterCanvas, 440.f, 82.f, 192.f, 48.f,
		MakeButton(TEXT("HeroSelection.BottomRow.TutorialButton"), FText::FromString(TEXT("TUTORIAL")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTutorialClicked), 0.f, 48.f, 16));
	HSAddCanvasSlot(RightClusterCanvas, 440.f, 141.f, 192.f, 48.f,
		MakeButton(TEXT("HeroSelection.BottomRow.ModsButton"), FText::FromString(TEXT("MODS")), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleModsClicked), 0.f, 48.f, 16));
	HSAddCanvasSlot(RightClusterCanvas, 440.f, 200.f, 192.f, 48.f,
		MakeButton(TEXT("HeroSelection.BottomRow.TestButton"), FText::FromString(TEXT("TEST")), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTestClicked), 0.f, 48.f, 16));

	const TSharedRef<SConstraintCanvas> BottomCanvas = HSMakeCanvas();
	HSAddCanvasSlot(BottomCanvas, 17.f, 0.f, 660.f, 216.f, HSMakePanel(TEXT("HeroSelection.BottomRow.SteamPartyPanel"), ET66FlatState::Default, PartySlots, FMargin(12.f)));
	HSAddCanvasSlot(BottomCanvas, 710.f, 0.f, 511.f, 216.f, HSMakePanel(TEXT("HeroSelection.BottomRow.CompanionPanel"), ET66FlatState::Default, CompanionPanel, FMargin(12.f)));
	HSAddCanvasSlot(BottomCanvas, 1249.f, -20.f, 655.f, 276.f, HSMakePanel(TEXT("HeroSelection.BottomRow.RightCluster"), ET66FlatState::Default, RightClusterCanvas, FMargin(0.f)));

	const TSharedRef<SConstraintCanvas> RootCanvas = HSMakeCanvas();
	const TSharedRef<SConstraintCanvas> BackdropCanvas = HSMakeCanvas();
	HSAddCanvasSlot(BackdropCanvas, 0.f, 0.f, HeroSelectionCanvasW, 102.f, HSMakeBackdropSlab(TEXT("HeroSelection.Backdrop.Top")));
	HSAddCanvasSlot(BackdropCanvas, 0.f, 102.f, 576.f, 680.f, HSMakeBackdropSlab(TEXT("HeroSelection.Backdrop.Left")));
	HSAddCanvasSlot(BackdropCanvas, 1288.f, 102.f, 632.f, 680.f, HSMakeBackdropSlab(TEXT("HeroSelection.Backdrop.Right")));
	HSAddCanvasSlot(BackdropCanvas, 0.f, 782.f, HeroSelectionCanvasW, 298.f, HSMakeBackdropSlab(TEXT("HeroSelection.Backdrop.Bottom")));
	HSAddCanvasSlot(RootCanvas, 0.f, 0.f, HeroSelectionCanvasW, HeroSelectionCanvasH, HSTaggedBox(TEXT("HeroSelection.Backdrop"), BackdropCanvas, TEXT("Backdrop")));
	HSAddCanvasSlot(RootCanvas, 19.f, 17.f, 1258.f, 76.f, HSTaggedBox(TEXT("HeroSelection.TopRow"), TopCanvas, TEXT("TopRow")));
	HSAddCanvasSlot(RootCanvas, 0.f, 0.f, HeroSelectionCanvasW, HeroSelectionCanvasH, HSTaggedBox(TEXT("HeroSelection.MainBody"), MainBodyCanvas, TEXT("MainBody")));
	HSAddCanvasSlot(RootCanvas, 0.f, 815.f, HeroSelectionCanvasW, 218.f, HSTaggedBox(TEXT("HeroSelection.BottomRow"), BottomCanvas, TEXT("BottomRow")));

	TSharedRef<SWidget> Root = FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(HeroSelectionCanvasW)
		.HeightOverride(HeroSelectionCanvasH)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootCanvas
			]
		],
		HSName(TEXT("HeroSelection.Root")),
		TEXT("ScreenRoot"),
		ET66FlatState::Default);

	UpdateHeroDisplay();

	return Root;
}
