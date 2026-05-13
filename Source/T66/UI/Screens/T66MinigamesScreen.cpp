// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MinigamesScreen.h"

#include "Core/T66LocalizationSubsystem.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	TMap<FString, TStrongObjectPtr<UTexture2D>> GMinigamesFlatTextureCache;
	TMap<FString, TSharedPtr<FSlateBrush>> GMinigamesFlatBrushCache;

	bool T66IsMinigamesPausedGameplayWidget(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		return PC && PC->IsPaused();
	}

	UTexture2D* LoadMinigamesFlatTexture(const FString& SourceRelativePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GMinigamesFlatTextureCache.Find(SourceRelativePath))
		{
			return CachedTexture->Get();
		}

		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(SourceRelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
				CandidatePath,
				TextureFilter::TF_Trilinear,
				true,
				TEXT("MinigamesFlatUI"));
			if (!Texture)
			{
				Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					TEXT("MinigamesFlatUI"));
			}

			if (Texture)
			{
				GMinigamesFlatTextureCache.Add(SourceRelativePath, TStrongObjectPtr<UTexture2D>(Texture));
				return Texture;
			}
		}

		return nullptr;
	}

	const FSlateBrush* ResolveMinigamesFlatBrush(const FString& SourceRelativePath, const FVector2D& ImageSize = FVector2D::ZeroVector)
	{
		const FString BrushKey = FString::Printf(TEXT("%s::%.0fx%.0f"), *SourceRelativePath, ImageSize.X, ImageSize.Y);
		if (const TSharedPtr<FSlateBrush>* CachedBrush = GMinigamesFlatBrushCache.Find(BrushKey))
		{
			return CachedBrush->Get();
		}

		UTexture2D* Texture = LoadMinigamesFlatTexture(SourceRelativePath);
		if (!Texture)
		{
			return nullptr;
		}

		const FVector2D ResolvedSize = ImageSize.X > 0.f && ImageSize.Y > 0.f
			? ImageSize
			: FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()));

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ResolvedSize;
		Brush->Margin = FMargin(0.f);
		Brush->TintColor = FSlateColor(FLinearColor::White);
		Brush->SetResourceObject(Texture);

		GMinigamesFlatBrushCache.Add(BrushKey, Brush);
		return Brush.Get();
	}
}

UT66MinigamesScreen::UT66MinigamesScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Minigames;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66MinigamesScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66MinigamesScreen::BuildSlateUI()
{
	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	constexpr int32 CardsPerPage = 4;
	TSharedRef<SConstraintCanvas> MinigamesCanvas = SNew(SConstraintCanvas);
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder"));

	auto DTag = [](const TCHAR* Tag) -> FName
	{
		return FName(Tag);
	};

	auto AddCanvas = [&MinigamesCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		MinigamesCanvas->AddSlot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(X, Y, W, H))
		[
			Widget
		];
	};

	auto AddN = [&AddCanvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		AddCanvas(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH, Widget);
	};

	auto MakeMetadataRegion = [](const FName Tag, const FString& Role, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(SNew(SSpacer), Tag, Role, State);
	};

	auto TaggedText = [](
		const FName Tag,
		const FText& Text,
		const int32 FontSize,
		const FLinearColor& Color,
		const bool bBold = true,
		const ETextJustify::Type Justification = ETextJustify::Center,
		const bool bAutoWrap = false) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Visibility(EVisibility::HitTestInvisible)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.AutoWrapText(bAutoWrap)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	auto MakePanelSurface = [](const FName Tag, const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatPanel(
			State,
			FMargin(0.f),
			SNew(SSpacer),
			nullptr,
			Tag);
	};

	auto MakeArtwork = [&TaggedText](const FName Tag, const FString& RelativePath, const FVector2D& SizeHint) -> TSharedRef<SWidget>
	{
		const FSlateBrush* Brush = ResolveMinigamesFlatBrush(RelativePath, SizeHint);
		const TSharedRef<SWidget> Content = Brush
			? StaticCastSharedRef<SWidget>(
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFill)
				[
					SNew(SImage)
					.Visibility(EVisibility::HitTestInvisible)
					.Image(Brush)
				])
			: StaticCastSharedRef<SWidget>(
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					TaggedText(NAME_None, NSLOCTEXT("T66.MiniGames", "MissingFlatMinigameArt", "MINIGAME"), 18, FT66FlatStyle::SecondaryText(), true, ETextJustify::Center)
				]);

		return FT66FlatStyle::AttachMetadata(
			SNew(SBox)
			.Visibility(EVisibility::HitTestInvisible)
			.Clipping(EWidgetClipping::ClipToBounds)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				Content
			],
			Tag,
			TEXT("Artwork"),
			ET66FlatState::Default);
	};

	auto MakeBareInteractive = [&NoBorderButtonStyle](
		const FName Tag,
		const FString& Role,
		const TSharedRef<SWidget>& Content,
		FOnClicked OnClicked,
		const ET66FlatState State = ET66FlatState::Default) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			FT66Style::MakeBareButton(
				FT66BareButtonParams(MoveTemp(OnClicked), Content)
				.SetButtonStyle(&NoBorderButtonStyle)
				.SetPadding(FMargin(0.f))
				.SetDebounceClick(false)),
			Tag,
			Role,
			State,
			TOptional<FLinearColor>(),
			true);
	};

	auto MakeIcon = [&TaggedText](const FSlateBrush* Brush, const FText& FallbackText, const FLinearColor& Tint = FLinearColor::White) -> TSharedRef<SWidget>
	{
		return Brush
			? StaticCastSharedRef<SWidget>(
				SNew(SImage)
				.Visibility(EVisibility::HitTestInvisible)
				.Image(Brush)
				.ColorAndOpacity(Tint))
			: StaticCastSharedRef<SWidget>(TaggedText(NAME_None, FallbackText, 28, Tint, true, ETextJustify::Center));
	};

	auto MakePlayButton = [&TaggedText](const FName ButtonTag, const FName LabelTag, FOnClicked OnClicked, const float Width, const float Height) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				TaggedText(LabelTag, NSLOCTEXT("T66.MiniGames", "FlatPlay", "PLAY"), 27, FT66FlatStyle::SelectedText(), true, ETextJustify::Center)
			],
			MoveTemp(OnClicked),
			FMargin(0.f),
			Width,
			Height,
			true,
			ButtonTag);
	};

	auto MakePaginationDot = [&NoBorderButtonStyle](const FName Tag, const int32 PageIndex, const bool bSelected, TWeakObjectPtr<UT66MinigamesScreen> WeakThis) -> TSharedRef<SWidget>
	{
		const FLinearColor DotColor = bSelected ? FT66FlatStyle::SelectedBorder() : FT66FlatStyle::PurpleAccent();
		return FT66FlatStyle::AttachMetadata(
			FT66Style::MakeBareButton(
				FT66BareButtonParams(
					FOnClicked::CreateLambda([WeakThis, PageIndex]()
					{
						if (UT66MinigamesScreen* Screen = WeakThis.Get())
						{
							return Screen->HandleSelectMinigamePageClicked(PageIndex);
						}
						return FReply::Handled();
					}),
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
					.BorderBackgroundColor(DotColor))
				.SetButtonStyle(&NoBorderButtonStyle)
				.SetPadding(FMargin(0.f))
				.SetDebounceClick(false)),
			Tag,
			TEXT("Button"),
			bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
			DotColor,
			true);
	};

	struct FFlatMinigameCard
	{
		FText Title;
		FText Body;
		FString ArtPath;
		FOnClicked OnClicked;
	};

	TArray<FFlatMinigameCard> Cards;
	Cards.Reserve(5);
	Cards.Add({
		NSLOCTEXT("T66.MiniGames", "FlatMiniTitle", "CHADPOCALYPSE MINI"),
		NSLOCTEXT("T66.MiniGames", "FlatMiniBody", "A 2D survivor minigame with its own saves, heroes, idols, enemies, and progression."),
		TEXT("RuntimeDependencies/T66/UI/Minigames/FlatReference/minigames_card01_art.png"),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenMiniChadpocalypseClicked)
	});
	Cards.Add({
		NSLOCTEXT("T66.MiniGames", "FlatTDTitle", "CHADPOCALYPSE TOWER DEFENSE"),
		NSLOCTEXT("T66.MiniGames", "FlatTDBody", "A tower defense minigame with hero placement, enemy waves, upgrades, and rotating maps."),
		TEXT("RuntimeDependencies/T66/UI/Minigames/FlatReference/minigames_card02_art.png"),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenChadpocalypseTDClicked)
	});
	Cards.Add({
		NSLOCTEXT("T66.MiniGames", "FlatDeckTitle", "CHADPOCALYPSE DECKBUILDER"),
		NSLOCTEXT("T66.MiniGames", "FlatDeckBody", "A dungeon-descent deckbuilder with card combat, route choices, relics, and reward drafts."),
		TEXT("RuntimeDependencies/T66/UI/Minigames/FlatReference/minigames_card03_art.png"),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenChadpocalypseDeckbuilderClicked)
	});
	Cards.Add({
		NSLOCTEXT("T66.MiniGames", "FlatIdleTitle", "CHADPOCALYPSE IDLE"),
		NSLOCTEXT("T66.MiniGames", "FlatIdleBody", "An offline-progress idle minigame with heroes, upgrades, stage pushing, and comeback rewards."),
		TEXT("RuntimeDependencies/T66/UI/Minigames/FlatReference/minigames_card04_art.png"),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenIdleChadpocalypseClicked)
	});
	Cards.Add({
		NSLOCTEXT("T66.MiniGames", "FlatVersusTitle", "VERSUS"),
		NSLOCTEXT("T66.MiniGames", "FlatVersusBody", "A 1v1 arcade gauntlet where friends compete across cabinet games like Whack-a-Mole."),
		TEXT("RuntimeDependencies/T66/Arcade/Selector/arcade_selector_front_cabinet.png"),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleOpenVersusClicked)
	});

	const int32 TotalPages = FMath::Max(1, FMath::DivideAndRoundUp(Cards.Num(), CardsPerPage));
	MinigamesPageIndex = FMath::Clamp(MinigamesPageIndex, 0, TotalPages - 1);

	const FSlateBrush* LeftArrowBrush = ResolveMinigamesFlatBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/pagination_left.png"), FVector2D(30.f, 42.f));
	const FSlateBrush* RightArrowBrush = ResolveMinigamesFlatBrush(TEXT("RuntimeDependencies/T66/UI/Icons/Flat/pagination_right.png"), FVector2D(30.f, 42.f));

	AddN(0.031f, 0.141f, 0.936f, 0.790f, MakeMetadataRegion(DTag(TEXT("Minigames.Root")), TEXT("Root")));
	AddN(0.381f, 0.141f, 0.237f, 0.071f, TaggedText(DTag(TEXT("Minigames.Title")), NSLOCTEXT("T66.MiniGames", "Title", "MINIGAMES"), 72, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center));
	AddN(0.277f, 0.253f, 0.447f, 0.032f, TaggedText(
		DTag(TEXT("Minigames.DescriptionBand")),
		NSLOCTEXT("T66.MiniGames", "FlatDescription", "Earn Chad Coupons and compete with friends and the world in the minigames."),
		24,
		FT66FlatStyle::PrimaryText(),
		false,
		ETextJustify::Center));
	AddN(0.031f, 0.307f, 0.936f, 0.624f, MakeMetadataRegion(DTag(TEXT("Minigames.MainOuterContainer")), TEXT("OuterContainer")));
	AddN(0.033f, 0.307f, 0.934f, 0.624f, MakeMetadataRegion(DTag(TEXT("Minigames.CardsRow")), TEXT("CardRow")));

	AddN(0.010f, 0.570f, 0.018f, 0.045f, MakeBareInteractive(
		DTag(TEXT("Minigames.Carousel.LeftNavButton")),
		TEXT("Button"),
		MakeIcon(LeftArrowBrush, FText::FromString(TEXT("<")), FT66FlatStyle::PurpleAccent()),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandlePrevMinigamePageClicked)));
	AddN(0.972f, 0.570f, 0.018f, 0.045f, MakeBareInteractive(
		DTag(TEXT("Minigames.Carousel.RightNavButton")),
		TEXT("Button"),
		MakeIcon(RightArrowBrush, FText::FromString(TEXT(">")), FT66FlatStyle::PurpleAccent()),
		FOnClicked::CreateUObject(this, &UT66MinigamesScreen::HandleNextMinigamePageClicked)));

	constexpr float CardX[4] = { 0.033f, 0.271f, 0.506f, 0.742f };
	constexpr float CardW[4] = { 0.225f, 0.224f, 0.224f, 0.225f };
	constexpr float ArtX[4] = { 0.040f, 0.278f, 0.513f, 0.749f };
	constexpr float ArtW[4] = { 0.211f, 0.210f, 0.210f, 0.211f };
	constexpr float TitleX[4] = { 0.068f, 0.288f, 0.522f, 0.774f };
	constexpr float TitleW[4] = { 0.155f, 0.190f, 0.190f, 0.160f };
	constexpr float ButtonX[4] = { 0.043f, 0.281f, 0.516f, 0.752f };
	constexpr float ButtonW[4] = { 0.202f, 0.202f, 0.202f, 0.202f };

	for (int32 VisibleIndex = 0; VisibleIndex < CardsPerPage; ++VisibleIndex)
	{
		const int32 CardIndex = MinigamesPageIndex * CardsPerPage + VisibleIndex;
		if (!Cards.IsValidIndex(CardIndex))
		{
			continue;
		}

		const FFlatMinigameCard& Card = Cards[CardIndex];
		const FString Prefix = FString::Printf(TEXT("Minigames.Card%02d"), VisibleIndex + 1);

		AddN(CardX[VisibleIndex], 0.307f, CardW[VisibleIndex], 0.624f, MakePanelSurface(FName(*Prefix), ET66FlatState::Default));
		AddN(ArtX[VisibleIndex], 0.321f, ArtW[VisibleIndex], 0.304f, MakeArtwork(FName(*(Prefix + TEXT(".Artwork"))), Card.ArtPath, FVector2D(405.f, 328.f)));
		AddN(TitleX[VisibleIndex], 0.662f, TitleW[VisibleIndex], 0.043f, TaggedText(FName(*(Prefix + TEXT(".Title"))), Card.Title, 28, FT66FlatStyle::PrimaryText(), true, ETextJustify::Center));
		AddN(CardX[VisibleIndex] + 0.020f, 0.713f, CardW[VisibleIndex] - 0.040f, 0.094f, TaggedText(FName(*(Prefix + TEXT(".Description"))), Card.Body, 23, FT66FlatStyle::PrimaryText(), false, ETextJustify::Center, true));
		AddN(ButtonX[VisibleIndex], 0.839f, ButtonW[VisibleIndex], 0.069f, MakePlayButton(
			FName(*(Prefix + TEXT(".PlayButton"))),
			FName(*(Prefix + TEXT(".PlayButton.Label"))),
			Card.OnClicked,
			ButtonW[VisibleIndex] * CanvasW,
			0.069f * CanvasH));
	}

	AddN(0.471f, 0.948f, 0.020f, 0.018f, MakePaginationDot(DTag(TEXT("Minigames.Pagination.Dot01")), 0, MinigamesPageIndex == 0, this));
	AddN(0.509f, 0.948f, 0.020f, 0.018f, MakePaginationDot(DTag(TEXT("Minigames.Pagination.Dot02")), 1, MinigamesPageIndex == 1, this));
	AddN(0.471f, 0.948f, 0.058f, 0.018f, MakeMetadataRegion(DTag(TEXT("Minigames.Pagination")), TEXT("Pagination")));

	const TSharedRef<SWidget> Content = SNew(SBox)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.WidthOverride(CanvasW)
				.HeightOverride(CanvasH)
				[
					MinigamesCanvas
				]
			]
		];

	return T66ScreenSlateHelpers::MakeTopBarScreenRoot(
		UIManager,
		Content,
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black),
		FLinearColor::Transparent,
		FMargin(0.f, -96.f, 0.f, -20.f));
}

void UT66MinigamesScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66MinigamesScreen::HandleLanguageChanged);
	}
}

void UT66MinigamesScreen::OnScreenDeactivated_Implementation()
{
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.RemoveDynamic(this, &UT66MinigamesScreen::HandleLanguageChanged);
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66MinigamesScreen::OnBackClicked()
{
	if (T66IsMinigamesPausedGameplayWidget(this) && UIManager)
	{
		ShowModal(ET66ScreenType::PauseMenu);
		return;
	}

	NavigateBack();
}

FReply UT66MinigamesScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenMiniChadpocalypseClicked()
{
	NavigateTo(ET66ScreenType::MiniMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenChadpocalypseTDClicked()
{
	NavigateTo(ET66ScreenType::TDMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenIdleChadpocalypseClicked()
{
	NavigateTo(ET66ScreenType::IdleMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenChadpocalypseDeckbuilderClicked()
{
	NavigateTo(ET66ScreenType::DeckMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleOpenVersusClicked()
{
	NavigateTo(ET66ScreenType::VersusMainMenu);
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandlePrevMinigamePageClicked()
{
	constexpr int32 TotalPages = 2;
	MinigamesPageIndex = (MinigamesPageIndex + TotalPages - 1) % TotalPages;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleNextMinigamePageClicked()
{
	constexpr int32 TotalPages = 2;
	MinigamesPageIndex = (MinigamesPageIndex + 1) % TotalPages;
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66MinigamesScreen::HandleSelectMinigamePageClicked(const int32 PageIndex)
{
	constexpr int32 TotalPages = 2;
	MinigamesPageIndex = FMath::Clamp(PageIndex, 0, TotalPages - 1);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

void UT66MinigamesScreen::HandleLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}
