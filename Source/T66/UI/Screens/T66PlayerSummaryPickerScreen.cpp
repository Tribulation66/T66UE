// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66GameInstance.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

namespace
{
	enum class ET66PickerButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn
	};

	enum class ET66PickerButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed
	};

	struct FT66PickerSpriteBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	struct FT66PickerButtonBrushSet
	{
		FT66PickerSpriteBrushEntry Normal;
		FT66PickerSpriteBrushEntry Hover;
		FT66PickerSpriteBrushEntry Pressed;
		FT66PickerSpriteBrushEntry Disabled;
	};

	const FLinearColor T66PickerFantasyText(0.953f, 0.925f, 0.835f, 1.0f);
	const FLinearColor T66PickerFallbackPanel(0.025f, 0.023f, 0.034f, 0.97f);

	const FSlateBrush* ResolvePickerSpriteBrush(
		FT66PickerSpriteBrushEntry& Entry,
		const FString& RelativePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		const ESlateBrushDrawType::Type DrawAs)
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
					TextureFilter::TF_Trilinear,
					true,
					TEXT("PlayerPickerReferenceSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
	}

	const FSlateBrush* GetPickerContentShellBrush()
	{
		static FT66PickerSpriteBrushEntry Entry;
		return ResolvePickerSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/panel_modal.png"),
			FVector2D(1521.f, 463.f),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			ESlateBrushDrawType::Box);
	}

	const FSlateBrush* GetPickerRowShellBrush()
	{
		static FT66PickerSpriteBrushEntry Entry;
		return ResolvePickerSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/row_shell.png"),
			FVector2D(861.f, 74.f),
			FMargin(0.055f, 0.32f, 0.055f, 0.32f),
			ESlateBrushDrawType::Box);
	}

	const FSlateBrush* GetPickerAvatarFrameBrush()
	{
		static FT66PickerSpriteBrushEntry Entry;
		return ResolvePickerSpriteBrush(
			Entry,
			TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/portrait_socket.png"),
			FVector2D(56.f, 56.f),
			FMargin(0.f),
			ESlateBrushDrawType::Image);
	}

	FString GetPickerButtonPath(const ET66PickerButtonFamily Family, const ET66PickerButtonState State)
	{
		const TCHAR* Prefix = Family == ET66PickerButtonFamily::ToggleOn ? TEXT("button_success") : TEXT("button_neutral");
		const TCHAR* Suffix = TEXT("normal");
		if (State == ET66PickerButtonState::Hovered)
		{
			Suffix = TEXT("hover");
		}
		else if (State == ET66PickerButtonState::Pressed)
		{
			Suffix = TEXT("pressed");
		}
		return FString::Printf(TEXT("SourceAssets/UI/Worker2Reference/SheetSlices/Common/%s_%s.png"), Prefix, Suffix);
	}

	FVector2D GetPickerButtonSize(const ET66PickerButtonFamily Family, const ET66PickerButtonState State)
	{
		if (Family == ET66PickerButtonFamily::ToggleOn)
		{
			return State == ET66PickerButtonState::Pressed ? FVector2D(187.f, 67.f) : FVector2D(180.f, 68.f);
		}
		return State == ET66PickerButtonState::Pressed ? FVector2D(186.f, 68.f) : FVector2D(180.f, 68.f);
	}

	FT66PickerButtonBrushSet& GetPickerButtonBrushSet(const ET66PickerButtonFamily Family)
	{
		static FT66PickerButtonBrushSet CompactNeutral;
		static FT66PickerButtonBrushSet ToggleOn;
		return Family == ET66PickerButtonFamily::ToggleOn ? ToggleOn : CompactNeutral;
	}

	const FSlateBrush* GetPickerButtonBrush(const ET66PickerButtonFamily Family, const ET66PickerButtonState State)
	{
		FT66PickerButtonBrushSet& Set = GetPickerButtonBrushSet(Family);
		FT66PickerSpriteBrushEntry* Entry = &Set.Normal;
		if (State == ET66PickerButtonState::Hovered)
		{
			Entry = &Set.Hover;
		}
		else if (State == ET66PickerButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}

		return ResolvePickerSpriteBrush(
			*Entry,
			GetPickerButtonPath(Family, State),
			GetPickerButtonSize(Family, State),
			FMargin(0.14f, 0.30f, 0.14f, 0.30f),
			ESlateBrushDrawType::Box);
	}

	TSharedRef<SWidget> MakePickerSpritePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FLinearColor& FallbackColor = T66PickerFallbackPanel)
	{
		return SNew(SBorder)
			.BorderImage(Brush ? Brush : FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Brush ? FLinearColor::White : FallbackColor)
			.Padding(Padding)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakePickerSpriteButton(
		const FText& Label,
		const FOnClicked& OnClicked,
		const ET66PickerButtonFamily Family,
		const float MinWidth,
		const float Height,
		const int32 FontSize)
	{
		const FSlateBrush* NormalBrush = GetPickerButtonBrush(Family, ET66PickerButtonState::Normal);
		const FSlateBrush* HoverBrush = GetPickerButtonBrush(Family, ET66PickerButtonState::Hovered);
		const FSlateBrush* PressedBrush = GetPickerButtonBrush(Family, ET66PickerButtonState::Pressed);
		if (!NormalBrush)
		{
			return FT66Style::MakeButton(
				FT66ButtonParams(Label, OnClicked, Family == ET66PickerButtonFamily::ToggleOn ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(MinWidth)
				.SetHeight(Height)
				.SetFontSize(FontSize));
		}

		const TSharedPtr<ET66PickerButtonState> ButtonState = MakeShared<ET66PickerButtonState>(ET66PickerButtonState::Normal);
		const TAttribute<const FSlateBrush*> BrushAttr = TAttribute<const FSlateBrush*>::CreateLambda(
			[ButtonState, NormalBrush, HoverBrush, PressedBrush]() -> const FSlateBrush*
			{
				if (ButtonState.IsValid() && *ButtonState == ET66PickerButtonState::Pressed)
				{
					return PressedBrush ? PressedBrush : NormalBrush;
				}
				if (ButtonState.IsValid() && *ButtonState == ET66PickerButtonState::Hovered)
				{
					return HoverBrush ? HoverBrush : NormalBrush;
				}
				return NormalBrush;
			});

		return SNew(SBox)
			.MinDesiredWidth(MinWidth > 0.f ? MinWidth : FOptionalSize())
			.HeightOverride(Height > 0.f ? Height : FOptionalSize())
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Visibility(EVisibility::HitTestInvisible)
							.Image(BrushAttr)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.Padding(FMargin(12.f, 7.f, 12.f, 6.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(Label)
								.Font(FT66Style::Tokens::FontBold(FontSize))
								.ColorAndOpacity(T66PickerFantasyText)
								.Justification(ETextJustify::Center)
							]
						])
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetColor(FLinearColor::Transparent)
					.SetPadding(FMargin(0.f))
					.SetHAlign(HAlign_Fill)
					.SetVAlign(VAlign_Fill)
					.SetOnHovered(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66PickerButtonState::Hovered; }))
					.SetOnUnhovered(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66PickerButtonState::Normal; }))
					.SetOnPressed(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66PickerButtonState::Pressed; }))
					.SetOnReleased(FSimpleDelegate::CreateLambda([ButtonState]() { *ButtonState = ET66PickerButtonState::Hovered; })))
			];
	}
}

UT66PlayerSummaryPickerScreen::UT66PlayerSummaryPickerScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PlayerSummaryPicker;
	bIsModal = true;
}

TSharedRef<SWidget> UT66PlayerSummaryPickerScreen::BuildSlateUI()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66UIManager* Manager = UIManager.Get();

	if (!LB || !Manager)
	{
		return SNew(SBorder)
			.BorderBackgroundColor(FT66Style::Background())
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.Picker", "Error", "Error loading picker."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	}

	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& Snapshots = LB->GetPendingPickerSnapshots();
	if (Snapshots.Num() == 0)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Scrim())
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakePickerSpritePanel(
					SNew(SBox)
					.WidthOverride(520.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 0.f, 0.f, 14.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Picker", "Title", "Pick the Player"))
							.Font(FT66Style::Tokens::FontBold(28))
							.ColorAndOpacity(T66PickerFantasyText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Picker", "NoPlayers", "No players."))
							.Font(FT66Style::Tokens::FontRegular(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					],
					GetPickerContentShellBrush(),
					FMargin(30.f, 24.f))
			];
	}

	HeroBrushes.Reset();
	HeroBrushes.SetNum(Snapshots.Num());
	UT66UITexturePoolSubsystem* TexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;

	for (int32 i = 0; i < Snapshots.Num(); ++i)
	{
		UT66LeaderboardRunSummarySaveGame* Snap = Snapshots[i];
		if (!Snap) continue;

		TSharedPtr<FSlateBrush>& Brush = HeroBrushes[i];
		if (!Brush.IsValid())
		{
			Brush = T66ScreenSlateHelpers::MakeSlateBrush(FVector2D(96.f, 96.f));
		}
		if (T66GI && TexPool && !Snap->HeroID.IsNone())
		{
			FHeroData HeroData;
			if (T66GI->GetHeroData(Snap->HeroID, HeroData))
			{
				TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroData, Snap->HeroBodyType, ET66HeroPortraitVariant::Half);
				if (!PortraitSoft.IsNull())
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, Brush,
						FName(*FString::Printf(TEXT("PickerHero_%d"), i)), /*bClearWhileLoading*/ true);
				}
			}
		}
	}

	TSharedRef<SHorizontalBox> OptionsBox = SNew(SHorizontalBox);
	for (int32 i = 0; i < Snapshots.Num(); ++i)
	{
		UT66LeaderboardRunSummarySaveGame* Snap = Snapshots[i];
		if (!Snap) continue;

		const FString DisplayName = Snap->DisplayName.IsEmpty() ? TEXT("Player") : Snap->DisplayName;
		const int32 CapturedIndex = i;

		OptionsBox->AddSlot()
			.FillWidth(1.0f)
			.Padding(16.0f, 0.0f)
			[
				MakePickerSpritePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(DisplayName))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SNew(SBox)
						.WidthOverride(96.0f)
						.HeightOverride(96.0f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.025f, 0.023f, 0.034f, 0.95f))
								.Padding(12.f)
								[
									HeroBrushes.IsValidIndex(CapturedIndex) && HeroBrushes[CapturedIndex].IsValid()
										? StaticCastSharedRef<SWidget>(SNew(SImage).Image(HeroBrushes[CapturedIndex].Get()))
										: StaticCastSharedRef<SWidget>(SNew(SSpacer))
								]
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Visibility(EVisibility::HitTestInvisible)
								.Image(GetPickerAvatarFrameBrush())
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						MakePickerSpriteButton(
							NSLOCTEXT("T66.Picker", "Select", "SELECT"),
							FOnClicked::CreateUObject(this, &UT66PlayerSummaryPickerScreen::HandleSelectClicked, CapturedIndex),
							ET66PickerButtonFamily::ToggleOn,
							140.f,
							38.f,
							12)
					]
				,
				GetPickerRowShellBrush(),
				FMargin(18.0f, 14.0f))
			];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::Scrim())
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakePickerSpritePanel(
				SNew(SBox)
				.WidthOverride(900.f)
				.Padding(FMargin(10.f, 4.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 24.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Picker", "Title", "Pick the Player"))
						.Font(FT66Style::Tokens::FontBold(28))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						OptionsBox
					]
				],
				GetPickerContentShellBrush(),
				FMargin(30.0f, 24.0f))
		];
}

FReply UT66PlayerSummaryPickerScreen::HandleSelectClicked(int32 Index)
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB || !UIManager) return FReply::Handled();

	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& Snapshots = LB->GetPendingPickerSnapshots();
	if (!Snapshots.IsValidIndex(Index) || !Snapshots[Index])
	{
		CloseModal();
		return FReply::Handled();
	}

	UT66LeaderboardRunSummarySaveGame* Chosen = Snapshots[Index];
	LB->SetPendingFakeRunSummarySnapshot(Chosen);
	LB->ClearPendingPickerSnapshots();
	CloseModal();
	UIManager->ShowModal(ET66ScreenType::RunSummary);
	return FReply::Handled();
}

