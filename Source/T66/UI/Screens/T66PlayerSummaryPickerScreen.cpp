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
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Data/T66DataTypes.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

namespace
{
	FName PickerTag(const TCHAR* Tag)
	{
		return FName(Tag);
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
	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;

	auto MakeRect = [](const FLinearColor& Color, const FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(Color)
			.Visibility(EVisibility::HitTestInvisible),
			Tag,
			Role,
			ET66FlatState::Default);
	};

	auto MakeLabel = [](
		const FName Tag,
		const FText& Text,
		const int32 FontSize,
		const FLinearColor& Color,
		const bool bBold,
		const ETextJustify::Type Justification = ETextJustify::Center) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justification)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	auto BuildCanvasRoot = [CanvasW, CanvasH](const TSharedRef<SConstraintCanvas>& Canvas) -> TSharedRef<SWidget>
	{
		const TSharedRef<SWidget> RootContent = SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			];

		return FT66FlatStyle::AttachMetadata(
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
			PickerTag(TEXT("PlayerSummaryPicker.Root")),
			TEXT("Root"),
			ET66FlatState::Default);
	};

	auto AddSlot = [](const TSharedRef<SConstraintCanvas>& Canvas, const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	auto BuildEmptyPicker = [&MakeRect, &MakeLabel, &BuildCanvasRoot, &AddSlot, CanvasW, CanvasH](const FText& BodyText, const FName BodyTag) -> TSharedRef<SWidget>
	{
		const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
		AddSlot(Canvas, 0.f, 0.f, CanvasW, CanvasH,
			MakeRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.93f), PickerTag(TEXT("PlayerSummaryPicker.Scrim")), TEXT("Scrim")));
		AddSlot(Canvas, 542.4f, 449.86f, 835.2f, 180.28f,
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(0.f),
				SNullWidget::NullWidget,
				nullptr,
				PickerTag(TEXT("PlayerSummaryPicker.ModalPanel"))));
		AddSlot(Canvas, 585.6f, 484.42f, 748.8f, 111.16f,
			FT66FlatStyle::AttachMetadata(SNew(SBox), PickerTag(TEXT("PlayerSummaryPicker.Content")), TEXT("Content"), ET66FlatState::Default));
		AddSlot(Canvas, 820.f, 484.42f, 280.f, 58.f,
			MakeLabel(
				PickerTag(TEXT("PlayerSummaryPicker.Title")),
				NSLOCTEXT("T66.Picker", "Title", "Pick the Player"),
				28,
				FT66FlatStyle::PrimaryText(),
				true));
		AddSlot(Canvas, 900.f, 562.58f, 120.f, 33.f,
			MakeLabel(
				BodyTag,
				BodyText,
				16,
				FT66FlatStyle::SecondaryText(),
				false));

		return BuildCanvasRoot(Canvas);
	};

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66UIManager* Manager = UIManager.Get();

	if (!LB || !Manager)
	{
		return BuildEmptyPicker(
			NSLOCTEXT("T66.Picker", "Error", "Error loading picker."),
			PickerTag(TEXT("PlayerSummaryPicker.ErrorLabel")));
	}

	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& Snapshots = LB->GetPendingPickerSnapshots();
	if (Snapshots.Num() == 0)
	{
		return BuildEmptyPicker(
			NSLOCTEXT("T66.Picker", "NoPlayers", "No players."),
			PickerTag(TEXT("PlayerSummaryPicker.EmptyLabel")));
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
	int32 ValidOptionCount = 0;
	for (int32 i = 0; i < Snapshots.Num(); ++i)
	{
		UT66LeaderboardRunSummarySaveGame* Snap = Snapshots[i];
		if (!Snap) continue;
		++ValidOptionCount;

		const FString DisplayName = Snap->DisplayName.IsEmpty() ? TEXT("Player") : Snap->DisplayName;
		const int32 CapturedIndex = i;

		OptionsBox->AddSlot()
			.AutoWidth()
			.Padding(16.0f, 0.0f)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Default,
					FMargin(18.f, 14.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(SBox)
						.WidthOverride(218.f)
						[
							MakeLabel(
								PickerTag(*FString::Printf(TEXT("PlayerSummaryPicker.Option.%02d.Name"), CapturedIndex + 1)),
								FText::FromString(DisplayName),
								16,
								FT66FlatStyle::PrimaryText(),
								true)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 12.f)
					[
						FT66FlatStyle::MakeFlatPortraitSlot(
							ET66FlatState::Default,
							HeroBrushes.IsValidIndex(CapturedIndex) && HeroBrushes[CapturedIndex].IsValid() ? HeroBrushes[CapturedIndex].Get() : nullptr,
							nullptr,
							FVector2D(96.f, 96.f),
							PickerTag(*FString::Printf(TEXT("PlayerSummaryPicker.Option.%02d.Avatar"), CapturedIndex + 1)))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						FT66FlatStyle::MakeFlatButton(
							ET66FlatState::Selected,
							NSLOCTEXT("T66.Picker", "Select", "SELECT"),
							FOnClicked::CreateUObject(this, &UT66PlayerSummaryPickerScreen::HandleSelectClicked, CapturedIndex),
							nullptr,
							nullptr,
							FMargin(14.f, 8.f),
							140.f,
							38.f,
							true,
							14,
							PickerTag(*FString::Printf(TEXT("PlayerSummaryPicker.Option.%02d.SelectButton"), CapturedIndex + 1)))
					],
					nullptr,
					PickerTag(*FString::Printf(TEXT("PlayerSummaryPicker.Option.%02d"), CapturedIndex + 1)))
			];
	}

	const float PickerModalWidthMax = 1240.f;
	const float PickerModalWidth = FMath::Clamp(
		(static_cast<float>(ValidOptionCount) * 286.0f) + 96.0f,
		620.0f,
		PickerModalWidthMax);
	const float PickerModalHeight = 330.f;
	const float PickerModalX = (CanvasW - PickerModalWidth) * 0.5f;
	const float PickerModalY = (CanvasH - PickerModalHeight) * 0.5f;

	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	AddSlot(Canvas, 0.f, 0.f, CanvasW, CanvasH,
		MakeRect(FLinearColor(0.008f, 0.012f, 0.020f, 0.93f), PickerTag(TEXT("PlayerSummaryPicker.Scrim")), TEXT("Scrim")));
	AddSlot(Canvas, PickerModalX, PickerModalY, PickerModalWidth, PickerModalHeight,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(30.f, 24.f),
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 24.f)
			[
				MakeLabel(
					PickerTag(TEXT("PlayerSummaryPicker.Title")),
					NSLOCTEXT("T66.Picker", "Title", "Pick the Player"),
					28,
					FT66FlatStyle::PrimaryText(),
					true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				OptionsBox
			],
			nullptr,
			PickerTag(TEXT("PlayerSummaryPicker.ModalPanel"))));

	return BuildCanvasRoot(Canvas);
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

