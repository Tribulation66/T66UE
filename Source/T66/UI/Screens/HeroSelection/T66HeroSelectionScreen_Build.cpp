// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "TimerManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"

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
		// UI Reimagine transplant: per-tag hellfire plates extracted 1:1 from the
		// approved reference (transplant/HeroSelection). Unmapped tags fall back.
		static const TMap<FString, FString> HellfirePlates = {
			{ TEXT("HeroSelection.LeftColumn.SkinsPanel"),              TEXT("panel_skins.png") },
			{ TEXT("HeroSelection.LeftColumn.DrugsPanel"),              TEXT("panel_ster.png") },
			{ TEXT("HeroSelection.RightColumn.OuterPanel"),             TEXT("panel_right.png") },
			{ TEXT("HeroSelection.RightColumn.KitPreviewPanel"),        TEXT("card_ability.png") },
			{ TEXT("HeroSelection.RightColumn.RankPanel"),              TEXT("row_rank.png") },
			{ TEXT("HeroSelection.RightColumn.MasteryPanel"),           TEXT("row_mastery.png") },
			{ TEXT("HeroSelection.RightColumn.StatsPanel"),             TEXT("panel_stats.png") },
			{ TEXT("HeroSelection.MiddleColumn.CharacterPreviewPanel"), TEXT("panel_center.png") },
			{ TEXT("HeroSelection.BottomRow.SteamPartyPanel"),          TEXT("panel_ready.png") },
			{ TEXT("HeroSelection.BottomRow.CompanionPanel"),           TEXT("panel_comp.png") },
			{ TEXT("HeroSelection.BottomRow.RightCluster"),             TEXT("panel_br.png") },
		};
		if (const FString* Plate = HellfirePlates.Find(FString(Tag)))
		{
			return FT66FriendslopStyle::MakeCustomSurface(
				FString(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/")) + *Plate,
				FMargin(0.f),
				ESlateBrushDrawType::Image,
				FVector2D(500.f, 500.f),
				State,
				Padding,
				Content,
				nullptr,
				HSName(Tag),
				TEXT("Panel"));
		}
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
			GI->SelectedCompanionID = GI->ResolvePlayableCompanionID(GI->SelectedCompanionID);
			PreviewedCompanionID = GI->SelectedCompanionID;
		}
	}
	GeneratePlaceholderSkins();

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (T66GI)
	{
		AllHeroIDs = T66GI->GetPlayableHeroIDs();
		if (!PreviewedHeroID.IsNone() && !AllHeroIDs.Contains(PreviewedHeroID))
		{
			PreviewedHeroID = T66GI->ResolvePlayableHeroID(PreviewedHeroID);
		}
	}
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (SessionSubsystem)
	{
		SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	}
	if (T66GI)
	{
		SelectedDifficulty = T66GI->ResolvePlayableDifficulty(SelectedDifficulty);
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

	auto MakeButton = [](const TCHAR* Tag, const FText& Label, const ET66FlatState State, FOnClicked OnClicked, const float MinWidth, const float Height, const int32 FontSize = 20, const TSharedPtr<SWidget>& Icon = nullptr, const FName ToggleGroup = NAME_None, const bool bEnabled = true)
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
			bEnabled,
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
		const TCHAR* NameTag,
		const FName SkinID,
		const FText& Name,
		const float RowHeight,
		TFunction<void(const TSharedRef<SConstraintCanvas>&)> AddActions)
	{
		const ET66FlatState RowState = CurrentFlatSkinID == SkinID ? ET66FlatState::Selected : ET66FlatState::Default;
		const TSharedRef<SConstraintCanvas> RowCanvas = HSMakeCanvas();
		HSAddCanvasSlot(RowCanvas, 20.f, 20.f, 300.f, 34.f,
			HSTaggedText(NameTag, Name, 18, FT66FlatStyle::TextColorForState(RowState)));
		AddActions(RowCanvas);

		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/row_skin.png"),
			FMargin(0.f),
			FVector2D(512.f, 64.f),
			RowState,
			RowCanvas,
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleFlatSkinRowClicked, SkinID),
			FMargin(0.f),
			0.f,
			RowHeight,
			true,
			HSName(RowTag),
			SkinSelectionGroup,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image);
	};

	auto MakeEmptyDrugSlot = [](const TCHAR* Tag)
	{
		// Transplant: baked + well extract (completeness fix).
		return FT66FriendslopStyle::MakeCustomFixedImage(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/ster_well.png"),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			FVector2D(54.f, 49.f),
			HSName(Tag),
			TEXT("Panel"));
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

		// Transplant: ready/idle slot plates extracted from the reference.
		return FT66FriendslopStyle::MakeCustomSurface(
			State == ET66FlatState::Ready
				? TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/slot_ready_on.png")
				: TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/slot_ready_off.png"),
			FMargin(0.f),
			ESlateBrushDrawType::Image,
			FVector2D(124.f, 110.f),
			State,
			FMargin(0.f),
			SlotCanvas,
			nullptr,
			HSName(Tag),
			TEXT("Panel"));
	};

	const TSharedRef<SConstraintCanvas> TopCanvas = HSMakeCanvas();
	// BACK: baked extract (label baked, per-language punch list).
	HSAddCanvasSlot(TopCanvas, 0.f, 2.f, 150.f, 62.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/btn_back.png"),
			FMargin(0.f),
			FVector2D(156.f, 72.f),
			ET66FlatState::Selected,
			SNew(SBox).WidthOverride(150.f).HeightOverride(62.f),
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked),
			FMargin(0.f),
			150.f,
			62.f,
			true,
			HSName(TEXT("HeroSelection.TopRow.BackButton")),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image));

	const TSharedRef<SConstraintCanvas> CarouselCanvas = HSMakeCanvas();
	HSAddCanvasSlot(CarouselCanvas, 0.f, 0.f, 52.f, 70.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/tile_idle.png"),
			FMargin(0.f),
			FVector2D(56.f, 60.f),
			ET66FlatState::Default,
			SNew(SBox)
			.WidthOverride(52.f)
			.HeightOverride(70.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("?")))
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(24, true))
				.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.76f, 1.f))
				.Justification(ETextJustify::Center)
			],
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleRandomHeroClicked),
			FMargin(0.f),
			52.f,
			70.f,
			AllHeroIDs.Num() > 0,
			HSName(TEXT("HeroSelection.TopRow.HeroCarousel.RandomButton")),
			NAME_None,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image));
	HSAddCanvasSlot(CarouselCanvas, 58.f, 0.f, 52.f, 70.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/carousel_arrow_l.png"),
			FMargin(0.f),
			FVector2D(56.f, 60.f),
			ET66FlatState::Default,
			SNew(SBox).WidthOverride(52.f).HeightOverride(70.f),
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePrevClicked),
			FMargin(0.f),
			52.f,
			70.f,
			true,
			HSName(TEXT("HeroSelection.TopRow.HeroCarousel.LeftArrow")),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image));
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
		const int32 OffsetFromCenter = Index - HeroSelectionHeroCarouselCenterIndex;
		const int32 SlotHeroIndex = AllHeroIDs.Num() > 0 ? (CurrentHeroIndex + OffsetFromCenter + AllHeroIDs.Num()) % AllHeroIDs.Num() : INDEX_NONE;
		const FName SlotHeroID = AllHeroIDs.IsValidIndex(SlotHeroIndex) ? AllHeroIDs[SlotHeroIndex] : NAME_None;
		const bool bHeroPlayable = !SlotHeroID.IsNone() && (!T66GI || T66GI->IsHeroPlayable(SlotHeroID));
		const ET66FlatState SlotState = !bHeroPlayable
			? ET66FlatState::Disabled
			: (bSelected ? ET66FlatState::Selected : ET66FlatState::Default);
		const float X = 123.f + Index * 82.f;
		const FString Tag = FString::Printf(TEXT("HeroSelection.TopRow.HeroCarousel.Portrait%02d"), Index + 1);
		const TSharedPtr<FSlateBrush> PortraitBrush = HeroCarouselPortraitBrushes.IsValidIndex(Index)
			? HeroCarouselPortraitBrushes[Index]
			: nullptr;
		// Transplant: hellfire tile plates + the live hero portrait inset.
		TSharedPtr<SImage> PortraitImage;
		const bool bCustomHeroSlot = UT66GameInstance::IsCustomHeroID(SlotHeroID);
		const TSharedRef<SWidget> TileContent = SNew(SBox)
			.WidthOverride(54.f)
			.HeightOverride(58.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(PortraitImage, SImage)
					.Image(PortraitBrush.IsValid() ? PortraitBrush.Get() : nullptr)
					.ColorAndOpacity(bHeroPlayable && !bCustomHeroSlot ? FLinearColor::White : FLinearColor(0.35f, 0.33f, 0.33f, 1.f))
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility(bCustomHeroSlot ? EVisibility::Visible : EVisibility::Collapsed)
					.Text(FText::FromString(TEXT("?")))
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(30, true))
					.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.76f, 1.f))
					.Justification(ETextJustify::Center)
				]
			];
		if (HeroCarouselImageWidgets.IsValidIndex(Index))
		{
			HeroCarouselImageWidgets[Index] = PortraitImage;
		}
		const TSharedRef<SWidget> HeroCarouselButton = FT66FriendslopStyle::MakeCustomToggleGroupButton(
			bSelected
				? TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/tile_selected.png")
				: TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/tile_idle.png"),
			FMargin(0.f),
			FVector2D(86.f, 82.f),
			SlotState,
			TileContent,
			bHeroPlayable ? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroCarouselPortraitClicked, Index) : FOnClicked(),
			FMargin(4.f),
			67.f,
			Index < 5 ? 72.f : 71.f,
			bHeroPlayable,
			FName(*Tag),
			HeroCarouselGroup,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image);
		HSAddCanvasSlot(CarouselCanvas, X, Index < 5 ? 1.f : 2.f, 67.f, Index < 5 ? 72.f : 71.f,
			HeroCarouselButton);
	}
	HSAddCanvasSlot(CarouselCanvas, 698.f, 0.f, 52.f, 70.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/carousel_arrow_r.png"),
			FMargin(0.f),
			FVector2D(56.f, 60.f),
			ET66FlatState::Default,
			SNew(SBox).WidthOverride(52.f).HeightOverride(70.f),
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked),
			FMargin(0.f),
			52.f,
			70.f,
			true,
			HSName(TEXT("HeroSelection.TopRow.HeroCarousel.RightArrow")),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image));
	HSAddCanvasSlot(TopCanvas, 512.f, 0.f, 746.f, 76.f,
		HSTaggedBox(TEXT("HeroSelection.TopRow.HeroCarousel"), CarouselCanvas, TEXT("HeroCarousel")));
	HSAddCanvasSlot(TopCanvas, 1274.f, 12.f, 250.f, 52.f,
		MakeButton(
			TEXT("HeroSelection.TopRow.BuildCustomHeroButton"),
			NSLOCTEXT("T66.HeroSelection", "BuildCustomHero", "BUILD YOUR OWN HERO"),
			ET66FlatState::Selected,
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBuildCustomHeroClicked),
			250.f,
			52.f,
			17));

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
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.Name"),
			FName(TEXT("Default")),
			NSLOCTEXT("T66.HeroSelection", "FlatSkinDefault", "DEFAULT"),
			112.f,
			[](const TSharedRef<SConstraintCanvas>& RowCanvas)
			{
				HSAddCanvasSlot(RowCanvas, 350.f, 50.f, 132.f, 44.f,
					FT66FriendslopStyle::MakeCustomFixedImage(
						TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/pill_equipped.png"),
						FMargin(0.f),
						ESlateBrushDrawType::Image,
						FVector2D(132.f, 44.f),
						FName(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.Default.EquippedBadge")),
						TEXT("Badge")));
			}));
	HSAddCanvasSlot(SkinsCanvas, 19.f, 184.f, 495.f, 111.f,
		MakeSkinRow(
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.DemoSkin"),
			TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.DemoSkin.Name"),
			UT66SkinSubsystem::DemoSkinID,
			NSLOCTEXT("T66.HeroSelection", "FlatSkinHeroDemo", "DEMO"),
			111.f,
			[&](const TSharedRef<SConstraintCanvas>& RowCanvas)
			{
				HSAddCanvasSlot(RowCanvas, 147.f, 54.f, 136.f, 44.f,
					FT66FriendslopStyle::MakeCustomToggleGroupButton(
						TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/pill_preview.png"),
						FMargin(0.f),
						FVector2D(136.f, 44.f),
						ET66FlatState::Default,
						SNew(SBox).WidthOverride(136.f).HeightOverride(44.f),
						FOnClicked::CreateLambda([]()
						{
							UE_LOG(LogT66HeroSelection, Warning, TEXT("Action SkinPreview clicked - backend not yet implemented"));
							return FReply::Handled();
						}),
						FMargin(0.f),
						136.f,
						44.f,
						true,
						FName(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.DemoSkin.PreviewButton")),
						NAME_None,
						FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
						ESlateBrushDrawType::Image));
				HSAddCanvasSlot(RowCanvas, 357.f, 54.f, 121.f, 44.f,
					FT66FriendslopStyle::MakeCustomSurface(
						TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/price_pill.png"),
						FMargin(0.f),
						ESlateBrushDrawType::Image,
						FVector2D(121.f, 44.f),
						ET66FlatState::Default,
						FMargin(8.f, 4.f),
						SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("50")))
							.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(16, true))
							.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.76f, 1.f))
						],
						nullptr,
						FName(TEXT("HeroSelection.LeftColumn.SkinsPanel.SkinRow.DemoSkin.Cost")),
						TEXT("Badge")));
			}));

	const TSharedRef<SConstraintCanvas> DrugsCanvas = HSMakeCanvas();
	HSAddCanvasSlot(DrugsCanvas, 23.f, 11.f, 121.f, 32.f,
		HSTaggedText(TEXT("HeroSelection.LeftColumn.DrugsPanel.Header"), NSLOCTEXT("T66.HeroSelection", "FlatDrugs", "DRUGS"), 25, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(DrugsCanvas, 19.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot01")));
	HSAddCanvasSlot(DrugsCanvas, 94.f, 45.f, 56.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot02")));
	HSAddCanvasSlot(DrugsCanvas, 169.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot03")));
	HSAddCanvasSlot(DrugsCanvas, 238.f, 45.f, 54.f, 49.f, MakeEmptyDrugSlot(TEXT("HeroSelection.LeftColumn.DrugsPanel.EquipSlot04")));
	const bool bDemoDrugPurchasesBlocked = T66GI
		&& T66GI->GetSubsystem<UT66BuffSubsystem>()
		&& !T66GI->GetSubsystem<UT66BuffSubsystem>()->AreSingleUseBuffPurchasesAllowed();
	const TSharedRef<SWidget> DrugsBuyButton = FT66FriendslopStyle::MakeCustomToggleGroupButton(
		TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/buy_pill.png"),
		FMargin(0.f),
		FVector2D(104.f, 52.f),
		bDemoDrugPurchasesBlocked ? ET66FlatState::Disabled : ET66FlatState::Selected,
		SNew(SBox).WidthOverride(77.f).HeightOverride(50.f),
		bDemoDrugPurchasesBlocked ? FOnClicked() : FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, 0),
		FMargin(0.f),
		77.f,
		50.f,
		!bDemoDrugPurchasesBlocked,
		HSName(TEXT("HeroSelection.LeftColumn.DrugsPanel.BuyButton")),
		NAME_None,
		FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
		ESlateBrushDrawType::Image);
	HSAddCanvasSlot(DrugsCanvas, 326.f, 44.f, 77.f, 50.f,
		T66DemoModeUI::WrapWithComingSoonOverlay(
			DrugsBuyButton,
			bDemoDrugPurchasesBlocked,
			this,
			HSName(TEXT("HeroSelection.LeftColumn.DrugsPanel.BuyButton.DemoOverlay"))));
	HSAddCanvasSlot(DrugsCanvas, 421.f, 44.f, 100.f, 50.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/clear_pill.png"),
			FMargin(0.f),
			FVector2D(108.f, 52.f),
			ET66FlatState::Default,
			SNew(SBox).WidthOverride(100.f).HeightOverride(50.f),
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleClearTemporaryBuffsClicked),
			FMargin(0.f),
			100.f,
			50.f,
			true,
			HSName(TEXT("HeroSelection.LeftColumn.DrugsPanel.ClearButton")),
			NAME_None,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image));

	const TSharedRef<SConstraintCanvas> LeftCanvas = HSMakeCanvas();
	HSAddCanvasSlot(LeftCanvas, 0.f, 0.f, 532.f, 528.f, HSMakePanel(TEXT("HeroSelection.LeftColumn.SkinsPanel"), ET66FlatState::Default, SkinsCanvas, FMargin(0.f)));
	HSAddCanvasSlot(LeftCanvas, 0.f, 556.f, 532.f, 116.f, HSMakePanel(TEXT("HeroSelection.LeftColumn.DrugsPanel"), ET66FlatState::Default, DrugsCanvas, FMargin(0.f)));

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
	HSAddCanvasSlot(RankCanvas, 51.f, 16.f, 150.f, 30.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.RankPanel.Label"), NSLOCTEXT("T66.HeroSelection", "FlatGlobalRank", "GLOBAL RANK"), 19, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(RankCanvas, 205.f, 15.f, 23.f, 25.f,
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
	HSAddCanvasSlot(MasteryCanvas, 51.f, 16.f, 160.f, 30.f,
		HSTaggedText(TEXT("HeroSelection.RightColumn.MasteryPanel.Label"), NSLOCTEXT("T66.HeroSelection", "FlatHeroMastery", "HERO MASTERY"), 19, FSlateColor(FT66FlatStyle::PrimaryText())));
	HSAddCanvasSlot(MasteryCanvas, 218.f, 22.f, 158.f, 19.f,
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

	const TSharedRef<SConstraintCanvas> KitPreviewCanvas = HSMakeCanvas();
	HSAddCanvasSlot(KitPreviewCanvas, 8.f, 8.f, 258.f, 110.f,
		FT66FlatStyle::AttachMetadata(
			SAssignNew(KitPreviewColorBox, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.025f, 0.027f, 0.035f, 1.f))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SImage)
				.Image_UObject(this, &UT66HeroSelectionScreen::GetKitPreviewVideoBrush)
			],
			HSName(TEXT("HeroSelection.RightColumn.KitPreviewPanel.Video")),
			TEXT("KitVideo"),
			ET66FlatState::Default));
	HSAddCanvasSlot(KitPreviewCanvas, 286.f, 9.f, 244.f, 26.f,
		FT66FlatStyle::AttachMetadata(
			SAssignNew(KitPreviewTitleWidget, STextBlock)
			.Text(NSLOCTEXT("T66.HeroSelection", "KitPreviewInitialTitle", "WEAPON"))
			.Font(FT66FlatStyle::MakeBoldFont(19))
			.ColorAndOpacity(FSlateColor(FT66FlatStyle::PrimaryText()))
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			HSName(TEXT("HeroSelection.RightColumn.KitPreviewPanel.Title")),
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true));
	HSAddCanvasSlot(KitPreviewCanvas, 286.f, 39.f, 244.f, 56.f,
		FT66FlatStyle::AttachMetadata(
			SAssignNew(KitPreviewDescriptionWidget, STextBlock)
			.Text(NSLOCTEXT("T66.HeroSelection", "KitPreviewInitialDescription", "Primary kit preview."))
			.Font(FT66FlatStyle::MakeFont(12))
			.ColorAndOpacity(FSlateColor(FT66FlatStyle::SecondaryText()))
			.AutoWrapText(true)
			.WrapTextAt(238.f)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			HSName(TEXT("HeroSelection.RightColumn.KitPreviewPanel.Description")),
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true));
	// Transplant: baked kit pills (labels baked; selected-state visual static — PARTIAL).
	auto MakeKitPill = [&](const TCHAR* Plate, const TCHAR* Tag, const ET66FlatState St, FOnClicked InClicked, const float W, const float H) -> TSharedRef<SWidget>
	{
		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			FString(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/")) + Plate,
			FMargin(0.f),
			FVector2D(W, H),
			St,
			SNew(SBox).WidthOverride(W).HeightOverride(H),
			MoveTemp(InClicked),
			FMargin(0.f),
			W,
			H,
			true,
			HSName(Tag),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image);
	};
	HSAddCanvasSlot(KitPreviewCanvas, 286.f, 106.f, 124.f, 50.f,
		MakeKitPill(TEXT("pill_weapon.png"), TEXT("HeroSelection.RightColumn.KitPreviewPanel.WeaponButton"),
			SelectedKitPreviewSlot == ET66HeroKitPreviewSlot::Weapon ? ET66FlatState::Selected : ET66FlatState::Default,
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleWeaponPreviewClicked), 124.f, 50.f));

	const TSharedRef<SConstraintCanvas> RightContent = HSMakeCanvas();
	const bool bLabPlayable = !T66GI || T66GI->IsRunCategoryPlayable(ET66RunCategory::Lab);
	HSAddCanvasSlot(RightContent, 31.f, 32.f, 186.f, 50.f,
		FT66FlatStyle::MakeFlatLabel(
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66HeroSelectionScreen::GetPreviewedHeroTitleText)),
			ET66FlatLabelRole::Title,
			ETextJustify::Left,
			HSName(TEXT("HeroSelection.RightColumn.HeaderRow.HeroName"))));
	if (bLabPlayable)
	{
		HSAddCanvasSlot(RightContent, 455.f, 24.f, 119.f, 50.f,
			MakeButton(
				TEXT("HeroSelection.RightColumn.HeaderRow.LabButton"),
				NSLOCTEXT("T66.HeroSelection", "FlatLab", "LAB"),
				ET66FlatState::Selected,
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleLabClicked),
				119.f,
				50.f,
				22,
				MakeIconWidget(LabBrush, FVector2D(22.f, 22.f), FT66FlatStyle::PrimaryText()),
				NAME_None,
				true));
	}
	HSAddCanvasSlot(RightContent, 31.f, 97.f, 363.f, 24.f,
		FT66FlatStyle::MakeFlatLabel(
			TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(this, &UT66HeroSelectionScreen::GetPreviewedHeroSubtitleText)),
			ET66FlatLabelRole::PurpleAccent,
			ETextJustify::Left,
			HSName(TEXT("HeroSelection.RightColumn.Subtitle"))));
	HSAddCanvasSlot(RightContent, 16.f, 137.f, 559.f, 180.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.KitPreviewPanel"), ET66FlatState::Default, HSTaggedBox(TEXT("HeroSelection.RightColumn.KitPreviewPanel.Content"), KitPreviewCanvas, TEXT("KitPreview")), FMargin(8.f)));
	HSAddCanvasSlot(RightContent, 18.f, 333.f, 557.f, 56.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.RankPanel"), ET66FlatState::Default, RankCanvas, FMargin(0.f)));
	HSAddCanvasSlot(RightContent, 18.f, 404.f, 557.f, 60.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.MasteryPanel"), ET66FlatState::Default, MasteryCanvas, FMargin(0.f)));
	HSAddCanvasSlot(RightContent, 16.f, 497.f, 559.f, 221.f,
		HSMakePanel(TEXT("HeroSelection.RightColumn.StatsPanel"), ET66FlatState::Default, HSTaggedBox(TEXT("HeroSelection.RightColumn.StatsPanel.Grid"), StatsCanvas, TEXT("StatsGrid")), FMargin(0.f)));

	const TSharedRef<SConstraintCanvas> MainBodyCanvas = HSMakeCanvas();
	HSAddCanvasSlot(MainBodyCanvas, 36.f, 112.f, 532.f, 700.f, HSTaggedBox(TEXT("HeroSelection.LeftColumn"), LeftCanvas, TEXT("Column")));
	HSAddCanvasSlot(MainBodyCanvas, 606.f, 112.f, 702.f, 718.f,
		HSTaggedBox(
			TEXT("HeroSelection.MiddleColumn"),
			FT66FlatStyle::MakeFlatTransparentRegion(
				ET66FlatState::Default,
				FMargin(10.f),
				PreviewOverlay,
				HSName(TEXT("HeroSelection.MiddleColumn.CharacterPreviewPanel"))),
			TEXT("Column")));
	HSAddCanvasSlot(MainBodyCanvas, 1352.f, 100.f, 528.f, 692.f,
		HSTaggedBox(
			TEXT("HeroSelection.RightColumn"),
			HSMakePanel(TEXT("HeroSelection.RightColumn.OuterPanel"), ET66FlatState::Default,
			// Rule 5: borders are sacred — content downscales uniformly to fit inside.
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.WidthOverride(591.f)
				.HeightOverride(772.f)
				[
					RightContent
				]
			],
			FMargin(16.f, 14.f)),
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
	HSAddCanvasSlot(PartySlots, 16.f, 48.f, 118.f, 112.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot01"), ET66FlatState::Ready, SkinDefaultBrush));
	HSAddCanvasSlot(PartySlots, 146.f, 48.f, 118.f, 112.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot02"), ET66FlatState::Default, SkinDefaultBrush));
	HSAddCanvasSlot(PartySlots, 276.f, 48.f, 118.f, 112.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot03"), ET66FlatState::Default, SkinBeachBrush));
	HSAddCanvasSlot(PartySlots, 406.f, 48.f, 118.f, 112.f,
		MakePartySlot(TEXT("HeroSelection.BottomRow.SteamPartyPanel.Slot04"), ET66FlatState::Default, SkinDefaultBrush));

	const TSharedRef<SConstraintCanvas> CompanionPanel = HSMakeCanvas();
	// Transplant: hellfire companion pills with live names + colored icons.
	auto MakeCompanionPill = [&](const FText& Label, const bool bSelected, FOnClicked InClicked, const TSharedPtr<SWidget>& Icon, const TCHAR* Tag) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		if (Icon.IsValid())
		{
			Row->AddSlot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f).VAlign(VAlign_Center)[ Icon.ToSharedRef() ];
		}
		Row->AddSlot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(18, true))
			.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.76f, 1.f))
		];
		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			bSelected
				? TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/pill_comp_sel.png")
				: TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/pill_comp_idle.png"),
			FMargin(0.f),
			FVector2D(242.f, 68.f),
			bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)[ Row ],
			MoveTemp(InClicked),
			FMargin(8.f, 4.f),
			0.f,
			58.f,
			true,
			HSName(Tag),
			GenderToggleGroup,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image);
	};
	HSAddCanvasSlot(CompanionPanel, 19.f, 11.f, 209.f, 58.f,
		MakeCompanionPill(FText::FromString(TEXT("CHAD")), T66BodyTypeAliases::IsChad(SelectedBodyType), FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChadBodyClicked), MakeTaggedIconWidget(TEXT("HeroSelection.BottomRow.CompanionPanel.ChadButton.Icon"), ChadIconBrush, FVector2D(22.f, 22.f), FLinearColor::FromSRGBColor(FColor(60, 200, 240))), TEXT("HeroSelection.BottomRow.CompanionPanel.ChadButton")));
	HSAddCanvasSlot(CompanionPanel, 257.f, 13.f, 202.f, 58.f,
		MakeCompanionPill(FText::FromString(TEXT("STACY")), T66BodyTypeAliases::IsStacy(SelectedBodyType), FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStacyBodyClicked), MakeTaggedIconWidget(TEXT("HeroSelection.BottomRow.CompanionPanel.StacyButton.Icon"), StacyIconBrush, FVector2D(22.f, 22.f), FLinearColor::FromSRGBColor(FColor(240, 100, 180))), TEXT("HeroSelection.BottomRow.CompanionPanel.StacyButton")));
	HSAddCanvasSlot(CompanionPanel, 19.f, 103.f, 440.f, 66.f,
		FT66FriendslopStyle::MakeCustomToggleGroupButton(
			TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/bar_comp.png"),
			FMargin(0.f),
			FVector2D(546.f, 66.f),
			ET66FlatState::Default,
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("GIRLFRIEND")))
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(17, true))
				.ColorAndOpacity(FLinearColor(0.96f, 0.89f, 0.76f, 1.f))
			],
			FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked),
			FMargin(8.f, 4.f),
			0.f,
			66.f,
			true,
			HSName(TEXT("HeroSelection.BottomRow.CompanionPanel.ChooseCompanionButton")),
			NAME_None,
			FLinearColor(0.08f, 0.085f, 0.11f, 1.f),
			ESlateBrushDrawType::Image));

	auto MakeDifficultyMenu = [this, Loc, T66GI, Difficulties]() -> TSharedRef<SWidget>
	{
		TArray<FT66FlatDropdownOptionData> Options;
		for (ET66Difficulty Difficulty : Difficulties)
		{
			const FText Label = Loc ? Loc->GetText_Difficulty(Difficulty) : FText::FromString(TEXT("Easy"));
			const bool bDifficultyPlayable = !T66GI || T66GI->IsDifficultyPlayable(Difficulty);
			FT66FlatDropdownOptionData Option;
			Option.Label = Label;
			Option.State = !bDifficultyPlayable
				? ET66FlatState::Disabled
				: (Difficulty == SelectedDifficulty ? ET66FlatState::Selected : ET66FlatState::Default);
			Option.bEnabled = bDifficultyPlayable;
			Option.bShowUnavailableOverlay = !bDifficultyPlayable;
			Option.UnavailableText = T66DemoModeUI::GetUnavailableContentText(this);
			Option.MinWidth = 160.f;
			Option.Height = 56.f;
			Option.FontSize = 20;
			Option.Tag = FName(*FString::Printf(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown.Option.%d"), static_cast<int32>(Difficulty)));
			Option.OverlayTag = FName(*FString::Printf(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown.Option.%d.DemoOverlay"), static_cast<int32>(Difficulty)));
			Option.OnClicked = FOnClicked::CreateLambda([this, Difficulty]()
			{
				SelectDifficulty(Difficulty);
				FSlateApplication::Get().DismissAllMenus();
				return FReply::Handled();
			});
			Options.Add(MoveTemp(Option));
		}
		return HSMakePanel(
			TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown.Menu"),
			ET66FlatState::Default,
			FT66FlatStyle::MakeFlatDropdownOptionsMenu(Options, 160.f, 56.f, 20, HSName(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown.Options"))),
			FMargin(6.f));
	};

	const TSharedRef<SVerticalBox> DifficultyPanel = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			HSTaggedText(TEXT("HeroSelection.BottomRow.DifficultyPanel.Label"), FText::FromString(TEXT("DIFFICULTY")), 16, FSlateColor(FT66FlatStyle::PrimaryText()))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			FT66FlatStyle::MakeFlatDropdown(
				ET66FlatState::Default,
				TAttribute<FText>::CreateLambda([this, Loc]()
				{
					return CurrentDifficultyOption.IsValid()
						? FText::FromString(*CurrentDifficultyOption)
						: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
				}),
				MakeDifficultyMenu,
				false,
				0.f,
				56.f,
				20,
				HSName(TEXT("HeroSelection.BottomRow.DifficultyPanel.Dropdown")))
		];

	const TSharedRef<SConstraintCanvas> RightClusterCanvas = HSMakeCanvas();
	HSAddCanvasSlot(RightClusterCanvas, 21.f, 35.f, 156.f, 109.f,
		HSMakePanel(TEXT("HeroSelection.BottomRow.DifficultyPanel"), ET66FlatState::Default, DifficultyPanel, FMargin(10.f)));
	// Transplant: baked extracts (labels baked — per-language punch list).
	auto MakeBakedButton = [&](const TCHAR* PlateFile, const TCHAR* Tag, const ET66FlatState State, FOnClicked InClicked, const float W, const float H) -> TSharedRef<SWidget>
	{
		return FT66FriendslopStyle::MakeCustomToggleGroupButton(
			FString(TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/HeroSelection/")) + PlateFile,
			FMargin(0.f),
			FVector2D(W, H),
			State,
			SNew(SBox).WidthOverride(W).HeightOverride(H),
			MoveTemp(InClicked),
			FMargin(0.f),
			W,
			H,
			true,
			HSName(Tag),
			NAME_None,
			FLinearColor(0.62f, 0.04f, 0.075f, 1.f),
			ESlateBrushDrawType::Image);
	};
	HSAddCanvasSlot(RightClusterCanvas, 186.f, 22.f, 178.f, 150.f,
		MakeBakedButton(TEXT("btn_enter.png"), TEXT("HeroSelection.BottomRow.DifficultyPanel.EnterButton"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked), 178.f, 150.f));
	HSAddCanvasSlot(RightClusterCanvas, 376.f, 20.f, 184.f, 48.f,
		MakeBakedButton(TEXT("btn_challenges.png"), TEXT("HeroSelection.BottomRow.ChallengesButton"), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChallengesClicked), 184.f, 48.f));
	HSAddCanvasSlot(RightClusterCanvas, 376.f, 76.f, 184.f, 48.f,
		MakeBakedButton(TEXT("btn_tutorial.png"), TEXT("HeroSelection.BottomRow.TutorialButton"), ET66FlatState::Default, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTutorialClicked), 184.f, 48.f));
	HSAddCanvasSlot(RightClusterCanvas, 390.f, 130.f, 146.f, 56.f,
		MakeBakedButton(TEXT("btn_test.png"), TEXT("HeroSelection.BottomRow.TestButton"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTestClicked), 146.f, 56.f));

	const TSharedRef<SConstraintCanvas> BottomCanvas = HSMakeCanvas();
	HSAddCanvasSlot(BottomCanvas, 30.f, -11.f, 542.f, 242.f, HSMakePanel(TEXT("HeroSelection.BottomRow.SteamPartyPanel"), ET66FlatState::Default, PartySlots, FMargin(12.f)));
	HSAddCanvasSlot(BottomCanvas, 650.f, 31.f, 594.f, 192.f, HSMakePanel(TEXT("HeroSelection.BottomRow.CompanionPanel"), ET66FlatState::Default, CompanionPanel, FMargin(12.f)));
	HSAddCanvasSlot(BottomCanvas, 1324.f, 3.f, 566.f, 222.f, HSMakePanel(TEXT("HeroSelection.BottomRow.RightCluster"), ET66FlatState::Default, RightClusterCanvas, FMargin(0.f)));

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

	if (bShowRunWillNotCountWarning)
	{
		const FText WarningBody = FText::FromString(RunWillNotCountReasonText.IsEmpty()
			? FString(TEXT("This run will not count for the leaderboard."))
			: RunWillNotCountReasonText);

		Root = SNew(SOverlay)
			+ SOverlay::Slot()
			[
				Root
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.72f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						FT66FlatStyle::MakeFlatPanel(
							ET66FlatState::Selected,
							FMargin(32.f, 28.f),
							SNew(SBox)
							.WidthOverride(620.f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									HSTaggedText(TEXT("HeroSelection.RunWillNotCountWarning.Title"), NSLOCTEXT("T66.HeroSelection", "RunWillNotCountWarningTitle", "RUN WILL NOT COUNT"), 30, FSlateColor(FT66FlatStyle::PrimaryText()), ETextJustify::Center)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
								[
									FT66FlatStyle::AttachMetadata(
										SNew(STextBlock)
										.Text(WarningBody)
										.Font(FT66FlatStyle::MakeBoldFont(20))
										.ColorAndOpacity(FT66FlatStyle::PrimaryText())
										.AutoWrapText(true)
										.WrapTextAt(560.f)
										.Justification(ETextJustify::Center),
										HSName(TEXT("HeroSelection.RunWillNotCountWarning.Body")),
										TEXT("Label"),
										ET66FlatState::Default)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f)
								[
									FT66FlatStyle::AttachMetadata(
										SNew(SCheckBox)
										.IsChecked_Lambda([this]()
										{
											return bRunWillNotCountDontShowAgainChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
										})
										.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
										{
											bRunWillNotCountDontShowAgainChecked = NewState == ECheckBoxState::Checked;
										})
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.HeroSelection", "RunWillNotCountDoNotShowAgain", "DO NOT SHOW AGAIN"))
											.Font(FT66FlatStyle::MakeBoldFont(16))
											.ColorAndOpacity(FT66FlatStyle::PrimaryText())
										],
										HSName(TEXT("HeroSelection.RunWillNotCountWarning.DoNotShowAgain")),
										TEXT("Checkbox"),
										ET66FlatState::Default)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 24.f, 0.f, 0.f)
								.HAlign(HAlign_Center)
								[
									MakeButton(TEXT("HeroSelection.RunWillNotCountWarning.OkayButton"), NSLOCTEXT("T66.Common", "OkayAllCaps", "OKAY"), ET66FlatState::Selected, FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleRunWillNotCountOkayClicked), 180.f, 54.f, 20)
								]
							],
							nullptr,
							HSName(TEXT("HeroSelection.RunWillNotCountWarning.Panel")))
					],
					HSName(TEXT("HeroSelection.RunWillNotCountWarning.Overlay")),
					TEXT("ModalOverlay"),
					ET66FlatState::Default)
			];
	}

	return Root;
}
