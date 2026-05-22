// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66GameOverScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66GameOverScreen::UT66GameOverScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::GameOver;
	bIsModal = true;
}

void UT66GameOverScreen::GatherRunRewards(int32& OutCoupons, int32& OutAchievements, int32& OutSecretAchievements) const
{
	OutCoupons = 0;
	OutAchievements = 0;
	OutSecretAchievements = 0;

	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		OutCoupons = FMath::Max(0, RunState->GetPowerCrystalsEarnedThisRun());
	}

	const UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Achievements)
	{
		return;
	}

	const TArray<FName> CurrentRunAchievements = Achievements->GetCurrentRunUnlockedAchievementIDs();
	const TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
	for (const FName AchievementID : CurrentRunAchievements)
	{
		const FAchievementData* Definition = AllAchievements.FindByPredicate(
			[AchievementID](const FAchievementData& Achievement)
			{
				return Achievement.AchievementID == AchievementID;
			});

		if (Definition && Definition->Category == ET66AchievementCategory::Special)
		{
			++OutSecretAchievements;
		}
		else
		{
			++OutAchievements;
		}
	}
}

TSharedRef<SWidget> UT66GameOverScreen::BuildSlateUI()
{
	int32 Coupons = 0;
	int32 Achievements = 0;
	int32 SecretAchievements = 0;
	GatherRunRewards(Coupons, Achievements, SecretAchievements);

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FLinearColor Purple = FT66FlatStyle::PurpleAccent();
	const FLinearColor Red = FT66FlatStyle::SelectedText();
	const FLinearColor White = FT66FlatStyle::PrimaryText();
	const FLinearColor DimLine(Purple.R, Purple.G, Purple.B, 0.45f);

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};
	auto Label = [](const FName Tag, const FText& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = true, const ETextJustify::Type Justify = ETextJustify::Center) -> TSharedRef<SWidget>
	{
		TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justify)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(TextBlock, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	};
	auto Panel = [](const FName Tag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
	};
	auto ButtonShell = [](FOnClicked OnClicked, const FName Tag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
			SNullWidget::NullWidget,
			MoveTemp(OnClicked),
			FMargin(0.f),
			0.f,
			0.f,
			true,
			Tag);
	};
	auto StatPanel = [&](const float X, const FName Tag, const FText& LabelText, const int32 Value)
	{
		AddN(X, 0.395f, 0.205f, 0.130f, Panel(Tag));
		AddN(X + 0.018f, 0.420f, 0.169f, 0.028f, Label(FName(*(Tag.ToString() + TEXT(".Label"))), LabelText, 18, Purple, true));
		AddN(X + 0.065f, 0.454f, 0.075f, 0.044f, Label(FName(*(Tag.ToString() + TEXT(".Value"))), FText::AsNumber(FMath::Max(0, Value)), 34, White, true));
		AddN(X + 0.025f, 0.512f, 0.155f, 0.002f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
				.BorderBackgroundColor(DimLine)
				.Visibility(EVisibility::HitTestInvisible),
				FName(*(Tag.ToString() + TEXT(".Divider"))),
				TEXT("Divider"),
				ET66FlatState::Default));
	};

	AddN(0.f, 0.f, 1.f, 1.f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FT66FlatStyle::BackgroundColor()),
			FName(TEXT("GameOver.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.200f, 0.155f, 0.600f, 0.160f, Label(FName(TEXT("GameOver.Title")), NSLOCTEXT("T66.GameOver", "Title", "GAME OVER"), 86, Red, true));
	AddN(0.250f, 0.292f, 0.500f, 0.045f, Label(FName(TEXT("GameOver.Subtitle")), NSLOCTEXT("T66.GameOver", "Subtitle", "YOU DIDED LIKE A DOG"), 30, White, true));

	StatPanel(0.175f, FName(TEXT("GameOver.CouponsPanel")), NSLOCTEXT("T66.GameOver", "Coupons", "CHAD COUPONS"), Coupons);
	StatPanel(0.398f, FName(TEXT("GameOver.AchievementsPanel")), NSLOCTEXT("T66.GameOver", "Achievements", "ACHIEVEMENTS"), Achievements);
	StatPanel(0.621f, FName(TEXT("GameOver.SecretAchievementsPanel")), NSLOCTEXT("T66.GameOver", "SecretAchievements", "SECRET ACH."), SecretAchievements);

	AddN(0.370f, 0.660f, 0.260f, 0.092f, ButtonShell(FOnClicked::CreateUObject(this, &UT66GameOverScreen::HandleContinueClicked), FName(TEXT("GameOver.ContinueButton"))));
	AddN(0.420f, 0.682f, 0.160f, 0.044f, Label(FName(TEXT("GameOver.ContinueButton.Label")), NSLOCTEXT("T66.GameOver", "Continue", "CONTINUE"), 32, Red, true));

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SBox)
				.WidthOverride(CanvasW)
				.HeightOverride(CanvasH)
				[
					Canvas
				]
			]
		];

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

FReply UT66GameOverScreen::HandleContinueClicked()
{
	if (UIManager)
	{
		UIManager->CloseModal();
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	return FReply::Handled();
}
