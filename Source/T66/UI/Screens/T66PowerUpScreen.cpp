// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Math/UnrealMathUtility.h"

namespace
{
	/** Wheel with N slices; first UnlockedCount slices are filled with SliceColor. Border and radial lines always visible. */
	class ST66PowerUpWheelWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66PowerUpWheelWidget) {}
			SLATE_ATTRIBUTE(int32, NumSlices)
			SLATE_ATTRIBUTE(int32, UnlockedCount)
			SLATE_ATTRIBUTE(FLinearColor, SliceColor)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			NumSlices = InArgs._NumSlices.Get(10);
			UnlockedCount = FMath::Clamp(InArgs._UnlockedCount.Get(0), 0, NumSlices);
			SliceColor = InArgs._SliceColor.Get(FLinearColor::White);
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(120.f, 120.f);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const float MinDim = FMath::Max(2.f, FMath::Min(Size.X, Size.Y));
			const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
			const float Radius = (MinDim * 0.5f) - 2.f;
			const float StartAngle = -PI * 0.5f; // top

			auto AngleToPoint = [&](float Angle) -> FVector2D
			{
				return Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
			};

			const int32 Segs = 32;
			TArray<FVector2D> CirclePts;
			CirclePts.Reserve(Segs + 1);
			for (int32 i = 0; i <= Segs; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(Segs);
				CirclePts.Add(AngleToPoint(StartAngle + 2.f * PI * T));
			}
			const FLinearColor BorderColor(0.25f, 0.25f, 0.28f, 1.f);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), CirclePts, ESlateDrawEffect::None, BorderColor, true, 2.f);

			// Radial lines (slice borders)
			for (int32 i = 0; i <= NumSlices; ++i)
			{
				const float A = StartAngle + (2.f * PI * static_cast<float>(i) / static_cast<float>(NumSlices));
				TArray<FVector2D> Radial;
				Radial.Add(Center);
				Radial.Add(AngleToPoint(A));
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Radial, ESlateDrawEffect::None, BorderColor, false, 1.5f);
			}

			// Filled wedges for unlocked slices (draw many radii to simulate fill)
			for (int32 s = 0; s < UnlockedCount; ++s)
			{
				const float A0 = StartAngle + (2.f * PI * static_cast<float>(s) / static_cast<float>(NumSlices));
				const float A1 = StartAngle + (2.f * PI * static_cast<float>(s + 1) / static_cast<float>(NumSlices));
				const int32 Sub = 8;
				for (int32 k = 0; k < Sub; ++k)
				{
					const float T = static_cast<float>(k) / static_cast<float>(Sub);
					const float A = FMath::Lerp(A0, A1, T);
					TArray<FVector2D> Rad;
					Rad.Add(Center);
					Rad.Add(AngleToPoint(A));
					FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Rad, ESlateDrawEffect::None, SliceColor, false, 4.f);
				}
			}

			return LayerId + 3;
		}

	private:
		int32 NumSlices = 10;
		int32 UnlockedCount = 0;
		FLinearColor SliceColor = FLinearColor::White;
	};

	/** Simple circle outline only (for random panel). */
	class ST66PowerUpCircleOutlineWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66PowerUpCircleOutlineWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
		}

		virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(120.f, 120.f); }

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const float MinDim = FMath::Max(2.f, FMath::Min(Size.X, Size.Y));
			const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);
			const float Radius = (MinDim * 0.5f) - 2.f;
			const float StartAngle = -PI * 0.5f;
			const int32 Segs = 48;
			TArray<FVector2D> Pts;
			Pts.Reserve(Segs + 1);
			for (int32 i = 0; i <= Segs; ++i)
			{
				const float T = static_cast<float>(i) / static_cast<float>(Segs);
				const float A = StartAngle + 2.f * PI * T;
				Pts.Add(Center + FVector2D(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius));
			}
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Pts, ESlateDrawEffect::None, FLinearColor(0.4f, 0.4f, 0.45f, 1.f), true, 2.f);
			return LayerId + 1;
		}
	};
}

UT66PowerUpScreen::UT66PowerUpScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PowerUp;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66PowerUpScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66PowerUpSubsystem* UT66PowerUpScreen::GetPowerUpSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66PowerUpSubsystem>();
	}
	return nullptr;
}

FReply UT66PowerUpScreen::HandleBackClicked()
{
	if (CurrentPage > 0)
	{
		CurrentPage = 0;
		if (PageSwitcher.IsValid()) PageSwitcher->SetActiveWidgetIndex(0);
		return FReply::Handled();
	}
	NavigateBack();
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleNextClicked()
{
	CurrentPage = 1;
	if (PageSwitcher.IsValid()) PageSwitcher->SetActiveWidgetIndex(1);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleUnlockClicked(ET66HeroStatType StatType)
{
	UT66PowerUpSubsystem* PowerUp = GetPowerUpSubsystem();
	if (PowerUp && PowerUp->UnlockPowerupSlice(StatType))
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleUnlockRandomClicked()
{
	UT66PowerUpSubsystem* PowerUp = GetPowerUpSubsystem();
	if (PowerUp && PowerUp->UnlockRandomStat())
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

void UT66PowerUpScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66PowerUpScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PowerUpSubsystem* PowerUp = GetPowerUpSubsystem();

	const FText TitleText = NSLOCTEXT("T66.PowerUp", "Title", "POWER UP");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText BalanceLabelText = NSLOCTEXT("T66.PowerUp", "PowerCrystals", "Power Crystals");
	const int32 Cost = UT66PowerUpSubsystem::CrystalsPerSlice;
	const FText UnlockButtonFormat = FText::Format(
		NSLOCTEXT("T66.PowerUp", "UnlockPCFormat", "UNLOCK PC {0}"),
		FText::AsNumber(Cost)
	);

	const int32 Balance = PowerUp ? PowerUp->GetPowerCrystalBalance() : 0;
	const FText BalanceValueText = FText::AsNumber(Balance);

	auto GetStatColor = [](ET66HeroStatType Stat) -> FLinearColor
	{
		switch (Stat)
		{
			case ET66HeroStatType::Damage:      return FLinearColor(0.85f, 0.2f, 0.2f, 1.f);
			case ET66HeroStatType::AttackSpeed: return FLinearColor(0.95f, 0.5f, 0.1f, 1.f);
			case ET66HeroStatType::AttackScale:  return FLinearColor(0.9f, 0.85f, 0.2f, 1.f);
			case ET66HeroStatType::Armor:       return FLinearColor(0.2f, 0.5f, 0.9f, 1.f);
			case ET66HeroStatType::Evasion:     return FLinearColor(0.2f, 0.8f, 0.4f, 1.f);
			case ET66HeroStatType::Luck:       return FLinearColor(0.7f, 0.3f, 0.9f, 1.f);
			default: return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
		}
	};

	static const TArray<ET66HeroStatType> StatTypes = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackScale,
		ET66HeroStatType::Armor,
		ET66HeroStatType::Evasion,
		ET66HeroStatType::Luck
	};

	auto GetStatLabel = [Loc](ET66HeroStatType Stat) -> FText
	{
		if (Loc)
		{
			switch (Stat)
			{
				case ET66HeroStatType::Damage:      return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale:  return Loc->GetText_Stat_AttackScale();
				case ET66HeroStatType::Armor:       return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion:     return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck:        return Loc->GetText_Stat_Luck();
				default: break;
			}
		}
		return NSLOCTEXT("T66.PowerUp", "StatUnknown", "?");
	};

	const FText RandomLabelText = NSLOCTEXT("T66.PowerUp", "Random", "RANDOM");
	const FText NextText = NSLOCTEXT("T66.Common", "Next", "NEXT");
	const FText PCsFormat = FText::Format(NSLOCTEXT("T66.PowerUp", "PCsFormat", "{0} PCs"), FText::AsNumber(Balance));

	// Helper: build a stat panel widget for a given stat type
	auto MakeStatPanel = [&](ET66HeroStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Unlocked = PowerUp ? PowerUp->GetPowerupSlicesUnlocked(StatType) : 0;
		const FLinearColor StatColor = GetStatColor(StatType);

		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(GetStatLabel(StatType))
				.Font(FT66Style::Tokens::FontBold(14))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(120.f)
				.HeightOverride(120.f)
				[
					SNew(ST66PowerUpWheelWidget)
					.NumSlices(UT66PowerUpSubsystem::MaxSlicesPerStat)
					.UnlockedCount(Unlocked)
					.SliceColor(StatColor)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					UnlockButtonFormat,
					FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleUnlockClicked, StatType),
					ET66ButtonType::Primary,
					140.f
				)
			]
		,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4));
	};

	// ===== Page 0: 2x3 stat grid =====
	TSharedRef<SVerticalBox> StatsPage =
		SNew(SVerticalBox)
		// Row 1: Damage, Attack Speed, Attack Size
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 0.f, 0.f, FT66Style::Tokens::Space4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
			[ MakeStatPanel(ET66HeroStatType::Damage) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
			[ MakeStatPanel(ET66HeroStatType::AttackSpeed) ]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[ MakeStatPanel(ET66HeroStatType::AttackScale) ]
		]
		// Row 2: Armor, Evasion, Luck
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
			[ MakeStatPanel(ET66HeroStatType::Armor) ]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, FT66Style::Tokens::Space4, 0.f)
			[ MakeStatPanel(ET66HeroStatType::Evasion) ]
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[ MakeStatPanel(ET66HeroStatType::Luck) ]
		];

	// ===== Page 1: Random panel (full-width) =====
	TSharedRef<SWidget> RandomPage =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(RandomLabelText)
				.Font(FT66Style::Tokens::FontBold(24))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(200.f)
				.HeightOverride(200.f)
				[
					SNew(ST66PowerUpCircleOutlineWidget)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				FT66Style::MakeButton(
					UnlockButtonFormat,
					FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleUnlockRandomClicked),
					ET66ButtonType::Primary,
					180.f
				)
			]
		,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(24.f)));

	// Reset page to 0 on rebuild
	CurrentPage = 0;

	return FT66Style::MakePanel(
		SNew(SOverlay)
			// Main content
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// Header (title centered, PCs display right)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SOverlay)
					// Centered title
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(32))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// PCs display (right, styled box like ACs)
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						FT66Style::MakePanel(
							SNew(STextBlock)
							.Text(PCsFormat)
							.Font(FT66Style::Tokens::FontBold(22))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						,
						FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(15.0f, 8.0f)))
					]
				]
				// Page content (switcher)
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					.WidgetIndex(0)
					+ SWidgetSwitcher::Slot()[ StatsPage ]
					+ SWidgetSwitcher::Slot()[ RandomPage ]
				]
			]
			// Back button (bottom-left)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.f, 0.f, 0.f, 20.f)
			[
				FT66Style::MakeButton(FT66ButtonParams(BackText,
					FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleBackClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(120.f)
				)
			]
			// Next button (bottom-right, page 0 only)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 0.f, 20.f, 20.f)
			[
				SNew(SBox)
				.Visibility_Lambda([this]() { return CurrentPage == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					FT66Style::MakeButton(FT66ButtonParams(NextText,
						FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleNextClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(120.f)
					)
				]
			]
		,
		ET66PanelType::Panel,
		FMargin(24.f));
}
