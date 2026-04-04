// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PowerUpScreen.h"
#include "Core/T66AchievementsSubsystem.h"
#include "UI/T66UIManager.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Layout/Clipping.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D PowerUpStatueImageSize(318.f, 468.f);
	constexpr int32 PowerUpStatsPerRow = 3;
	const FLinearColor PowerUpLockedTint(0.03f, 0.03f, 0.04f, 0.96f);
	const FLinearColor PowerUpFillTint = FLinearColor::White;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GPowerUpFileTextureCache;

	FLinearColor T66PowerUpShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66PowerUpPanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66PowerUpInsetFill()
	{
		return FT66Style::PanelInner();
	}

	FLinearColor T66PowerUpButtonFill()
	{
		return FLinearColor(0.53f, 0.62f, 0.47f, 1.0f);
	}

	FLinearColor T66PowerUpButtonDisabledFill()
	{
		return FLinearColor(0.29f, 0.31f, 0.33f, 1.0f);
	}

	FLinearColor T66PowerUpNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	FT66ButtonParams FlattenPowerUpButton(FT66ButtonParams Params)
	{
		Params
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		return Params;
	}

	TSharedRef<SWidget> MakePowerUpButton(const FT66ButtonParams& Params)
	{
		return FT66Style::MakeButton(FlattenPowerUpButton(Params));
	}

	TSharedRef<SWidget> MakePowerUpPanel(const TSharedRef<SWidget>& Content, const FLinearColor& FillColor, const FMargin& Padding, ET66PanelType Type = ET66PanelType::Panel)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}

	UTexture2D* LoadPowerUpFileTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GPowerUpFileTextureCache.Find(FilePath))
		{
			return CachedTexture->Get();
		}

		if (!FPaths::FileExists(FilePath))
		{
			return nullptr;
		}

		UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
		if (!Texture)
		{
			return nullptr;
		}

		Texture->SRGB = true;
		Texture->Filter = TextureFilter::TF_Trilinear;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		Texture->CompressionSettings = TC_EditorIcon;
		Texture->NeverStream = true;
		Texture->UpdateResource();

		GPowerUpFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	bool HasBrushResource(const FSlateBrush* Brush)
	{
		return Brush
			&& Brush->DrawAs != ESlateBrushDrawType::NoDrawType
			&& (Brush->GetResourceObject() != nullptr || Brush->GetResourceName() != NAME_None);
	}

	class ST66PowerUpStatueFillWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66PowerUpStatueFillWidget) {}
			SLATE_ARGUMENT(const FSlateBrush*, StatueBrush)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
			SLATE_ARGUMENT(float, FillFraction)
			SLATE_ARGUMENT(FLinearColor, LockedTint)
			SLATE_ARGUMENT(FLinearColor, FillTint)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			StatueBrush = InArgs._StatueBrush;
			DesiredSize = InArgs._DesiredSize;
			FillFraction = FMath::Clamp(InArgs._FillFraction, 0.f, 1.f);
			LockedTint = InArgs._LockedTint;
			FillTint = InArgs._FillTint;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			if (DesiredSize.X > 0.f && DesiredSize.Y > 0.f)
			{
				return DesiredSize;
			}

			return StatueBrush ? StatueBrush->GetImageSize() : FVector2D(1.f, 1.f);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			if (!HasBrushResource(StatueBrush))
			{
				return LayerId;
			}

			const bool bEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
			const FLinearColor BrushTint = StatueBrush->GetTint(InWidgetStyle);
			const FLinearColor WidgetTint = InWidgetStyle.GetColorAndOpacityTint();
			int32 RetLayerId = LayerId;

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(),
				StatueBrush,
				DrawEffects,
				WidgetTint * LockedTint * BrushTint
			);

			if (FillFraction <= KINDA_SMALL_NUMBER)
			{
				return RetLayerId;
			}

			if (FillFraction >= 1.f - KINDA_SMALL_NUMBER)
			{
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					RetLayerId++,
					AllottedGeometry.ToPaintGeometry(),
					StatueBrush,
					DrawEffects,
					WidgetTint * FillTint * BrushTint
				);
				return RetLayerId;
			}

			const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
			const float FillHeight = FMath::Max(0.f, LocalSize.Y * FillFraction);
			if (FillHeight <= KINDA_SMALL_NUMBER)
			{
				return RetLayerId;
			}

			const FVector2D FillTopLeft(0.f, LocalSize.Y - FillHeight);
			const FVector2D FillTopRight(LocalSize.X, LocalSize.Y - FillHeight);
			const FVector2D FillBottomLeft(0.f, LocalSize.Y);
			const FVector2D FillBottomRight(LocalSize.X, LocalSize.Y);
			const FSlateRenderTransform& Transform = AllottedGeometry.GetAccumulatedRenderTransform();
			const FSlateClippingZone FillClip(
				Transform.TransformPoint(FillTopLeft),
				Transform.TransformPoint(FillTopRight),
				Transform.TransformPoint(FillBottomLeft),
				Transform.TransformPoint(FillBottomRight));

			if (FillClip.HasZeroArea())
			{
				return RetLayerId;
			}

			OutDrawElements.PushClip(FillClip);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				RetLayerId++,
				AllottedGeometry.ToPaintGeometry(),
				StatueBrush,
				DrawEffects,
				WidgetTint * FillTint * BrushTint
			);
			OutDrawElements.PopClip();

			return RetLayerId;
		}

	private:
		const FSlateBrush* StatueBrush = nullptr;
		FVector2D DesiredSize = FVector2D::ZeroVector;
		float FillFraction = 0.f;
		FLinearColor LockedTint = FLinearColor::Black;
		FLinearColor FillTint = FLinearColor::White;
	};

	FString GetPowerUpStatKey(ET66HeroStatType StatType)
	{
		switch (StatType)
		{
			case ET66HeroStatType::Damage:      return TEXT("damage");
			case ET66HeroStatType::AttackSpeed: return TEXT("attack_speed");
			case ET66HeroStatType::AttackScale: return TEXT("attack_scale");
			case ET66HeroStatType::Armor:       return TEXT("armor");
			case ET66HeroStatType::Evasion:     return TEXT("evasion");
			case ET66HeroStatType::Luck:        return TEXT("luck");
			default:                            return TEXT("unknown");
		}
	}

	void EnsurePowerUpRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	TSharedPtr<FSlateBrush> MakePowerUpImageBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		const FString& BrushKey,
		const FString& SourceRelativePath,
		const FString& PackagePath,
		const FString& ObjectPath,
		const FVector2D& ImageSize)
	{
		const FString SourcePath = FPaths::ProjectDir() / SourceRelativePath;
		const bool bHasSourceFile = FPaths::FileExists(SourcePath);
		const bool bHasImportedTexture = TexPool && FPackageName::DoesPackageExist(PackagePath);
		TSharedPtr<FSlateBrush> Brush;

		if (bHasSourceFile)
		{
			if (UTexture2D* Texture = LoadPowerUpFileTexture(SourcePath))
			{
				Brush = MakeShared<FSlateBrush>();
				EnsurePowerUpRuntimeImageBrush(Brush, ImageSize);
				Brush->SetResourceObject(Texture);
				Brush->TintColor = FSlateColor(FLinearColor::White);
			}
			else
			{
				return nullptr;
			}
		}
		else if (bHasImportedTexture)
		{
			Brush = MakeShared<FSlateBrush>();
			EnsurePowerUpRuntimeImageBrush(Brush, ImageSize);
			const TSoftObjectPtr<UTexture2D> Soft{ FSoftObjectPath(ObjectPath) };
			T66SlateTexture::BindSharedBrushAsync(TexPool, Soft, Requester, Brush, FName(*BrushKey), false);
		}
		else
		{
			return nullptr;
		}

		OwnedBrushes.Add(BrushKey, Brush);
		return Brush;
	}

	TSharedPtr<FSlateBrush> GetPowerUpStatueBaseBrush(
		TMap<FString, TSharedPtr<FSlateBrush>>& OwnedBrushes,
		UT66UITexturePoolSubsystem* TexPool,
		UObject* Requester,
		ET66HeroStatType StatType,
		const FVector2D& ImageSize)
	{
		const FString StatKey = GetPowerUpStatKey(StatType);
		const FString FileStem = FString::Printf(TEXT("%s_base"), *StatKey);
		const FString BrushKey = FString::Printf(TEXT("%s::base::%.0fx%.0f"), *StatKey, ImageSize.X, ImageSize.Y);
		const FString SourceRelativePath = FString::Printf(TEXT("SourceAssets/UI/PowerUp/Statues/Generated/%s/%s.png"), *StatKey, *FileStem);
		const FString PackagePath = FString::Printf(TEXT("/Game/UI/PowerUp/Statues/%s/%s"), *StatKey, *FileStem);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *FileStem);

		return MakePowerUpImageBrush(OwnedBrushes, TexPool, Requester, BrushKey, SourceRelativePath, PackagePath, ObjectPath, ImageSize);
	}

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

void UT66PowerUpScreen::SetShowingConverter(bool bInShowingConverter)
{
	bShowingConverter = bInShowingConverter;
	if (PageSwitcher.IsValid())
	{
		PageSwitcher->SetActiveWidgetIndex(bShowingConverter ? 1 : 0);
	}
}

FReply UT66PowerUpScreen::HandleBackClicked()
{
	if (bShowingConverter)
	{
		SetShowingConverter(false);
		return FReply::Handled();
	}

	NavigateBack();
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleOpenConverterClicked()
{
	SetShowingConverter(true);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleUnlockClicked(ET66HeroStatType StatType)
{
	UT66PowerUpSubsystem* PowerUp = GetPowerUpSubsystem();
	if (PowerUp && PowerUp->UnlockNextFillStep(StatType))
	{
		RefreshScreen();
	}
	return FReply::Handled();
}

bool UT66PowerUpScreen::TryConvertPowerCoupons(int32 RequestedCoupons)
{
	UT66PowerUpSubsystem* PowerUp = GetPowerUpSubsystem();
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!PowerUp || !Achievements)
	{
		return false;
	}

	const int32 AvailableCoupons = PowerUp->GetPowerCrystalBalance();
	const int32 CouponsToConvert = RequestedCoupons > 0
		? FMath::Min(RequestedCoupons, AvailableCoupons)
		: AvailableCoupons;

	if (CouponsToConvert <= 0 || !PowerUp->SpendPowerCrystals(CouponsToConvert))
	{
		return false;
	}

	Achievements->AddAchievementCoins(CouponsToConvert * AchievementCoinsPerPowerCoupon);
	RefreshScreen();
	return true;
}

FReply UT66PowerUpScreen::HandleConvertOneClicked()
{
	TryConvertPowerCoupons(1);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleConvertFiveClicked()
{
	TryConvertPowerCoupons(5);
	return FReply::Handled();
}

FReply UT66PowerUpScreen::HandleConvertAllClicked()
{
	TryConvertPowerCoupons(0);
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
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	StatueBrushes.Reset();

	const FText TitleText = NSLOCTEXT("T66.PowerUp", "Title", "POWER UP");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	const int32 Balance = PowerUp ? PowerUp->GetPowerCrystalBalance() : 0;
	const int32 AchievementCoinBalance = Achievements ? Achievements->GetAchievementCoinsBalance() : 0;

	auto GetStatLabel = [Loc](ET66HeroStatType Stat) -> FText
	{
		if (Loc)
		{
			switch (Stat)
			{
				case ET66HeroStatType::Damage:      return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
				case ET66HeroStatType::Armor:       return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion:     return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck:        return Loc->GetText_Stat_Luck();
				default: break;
			}
		}
		return NSLOCTEXT("T66.PowerUp", "StatUnknown", "?");
	};

	const FText ConverterLabelText = NSLOCTEXT("T66.PowerUp", "Converter", "CONVERTER");
	const FText ConverterHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "ConverterHint", "Trade Power Coupons for Achievement Coins at {0} AC per coupon."),
		FText::AsNumber(AchievementCoinsPerPowerCoupon));
	const FText ConverterRateText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "ConverterRate", "1 POWER COUPON = {0} ACHIEVEMENT COINS"),
		FText::AsNumber(AchievementCoinsPerPowerCoupon));
	const FText ConverterButtonText = NSLOCTEXT("T66.PowerUp", "ConverterButton", "PC -> AC CONVERTER");
	const TArray<ET66HeroStatType> StatTypes = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackScale,
		ET66HeroStatType::Armor,
		ET66HeroStatType::Evasion,
		ET66HeroStatType::Luck
	};

	auto MakeStatueWidget = [&](ET66HeroStatType StatType, const FVector2D& ImageSize) -> TSharedRef<SWidget>
	{
		const int32 UnlockedSteps = PowerUp ? PowerUp->GetUnlockedFillStepCount(StatType) : 0;
		const float FillFraction = static_cast<float>(UnlockedSteps) / static_cast<float>(UT66PowerUpSubsystem::MaxFillStepsPerStat);
		const TSharedPtr<FSlateBrush> BaseBrush = GetPowerUpStatueBaseBrush(StatueBrushes, TexPool, this, StatType, ImageSize);

		const TSharedRef<SWidget> StatueContent = BaseBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SNew(ST66PowerUpStatueFillWidget)
				.StatueBrush(BaseBrush.Get())
				.DesiredSize(ImageSize)
				.FillFraction(FillFraction)
				.LockedTint(PowerUpLockedTint)
				.FillTint(PowerUpFillTint))
			: StaticCastSharedRef<SWidget>(
				SNew(SSpacer)
				.Size(ImageSize));

		return SNew(SBox)
			.WidthOverride(ImageSize.X)
			.HeightOverride(ImageSize.Y)
			[
				StatueContent
			];
	};

	auto MakeStatPanel = [&](ET66HeroStatType StatType) -> TSharedRef<SWidget>
	{
		const int32 Cost = PowerUp ? PowerUp->GetCostForNextFillStepUnlock(StatType) : 0;
		const bool bMaxed = PowerUp && PowerUp->IsStatMaxed(StatType);
		const FText ButtonText = bMaxed
			? NSLOCTEXT("T66.PowerUp", "Max", "MAX")
			: FText::FromString(FString::Printf(TEXT("%d POWER COUPON%s"), Cost, Cost == 1 ? TEXT("") : TEXT("S")));

		return MakePowerUpPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(GetStatLabel(StatType))
				.Font(FT66Style::Tokens::FontBold(29))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SBox)
				.MinDesiredHeight(470.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(1.f).HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 4.f)
					[
						MakeStatueWidget(StatType, PowerUpStatueImageSize)
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 12.f, 0.f, 0.f)
			[
				MakePowerUpButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleUnlockClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetHeight(56.f)
					.SetFontSize(20)
					.SetColor(TAttribute<FSlateColor>::CreateLambda([bMaxed, Balance, Cost]() -> FSlateColor
					{
						if (bMaxed)
						{
							return FSlateColor(T66PowerUpButtonFill());
						}

						return FSlateColor(Balance >= Cost ? T66PowerUpButtonFill() : T66PowerUpButtonDisabledFill());
					}))
					.SetEnabled(TAttribute<bool>::CreateLambda([bMaxed, Balance, Cost]() { return !bMaxed && Balance >= Cost; }))
				)
			]
		,
		T66PowerUpPanelFill(),
		FMargin(16.f, 14.f, 16.f, 14.f));
	};

	auto MakeStatRow = [&](int32 StartIndex) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> StatRow = SNew(SHorizontalBox);
		for (int32 LocalIndex = 0; LocalIndex < PowerUpStatsPerRow; ++LocalIndex)
		{
			const int32 StatIndex = StartIndex + LocalIndex;
			StatRow->AddSlot()
				.FillWidth(1.f)
				.Padding(LocalIndex < PowerUpStatsPerRow - 1 ? FMargin(0.f, 0.f, 14.f, 0.f) : FMargin(0.f))
				[
					StatIndex < StatTypes.Num()
						? MakeStatPanel(StatTypes[StatIndex])
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				];
		}

		return StatRow;
	};

	TSharedRef<SWidget> StatsPage =
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
		.ScrollBarVisibility(EVisibility::Visible)
		+ SScrollBox::Slot()
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeStatRow(0)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				MakeStatRow(PowerUpStatsPerRow)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
			[
				SNew(SSpacer)
				.Size(FVector2D(1.f, 1.f))
			]
		];

	TSharedRef<SWidget> ConverterPage =
		MakePowerUpPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterLabelText)
				.Font(FT66Style::Tokens::FontBold(20))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterRateText)
				.Font(FT66Style::Tokens::FontBold(13))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
						MakePowerUpPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.PowerUp", "CouponsBalanceLabel", "POWER COUPONS"))
								.Font(FT66Style::Tokens::FontBold(15))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::AsNumber(Balance))
								.Font(FT66Style::Tokens::FontBold(28))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							],
							T66PowerUpInsetFill(),
							FMargin(16.f, 14.f),
							ET66PanelType::Panel)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 0.f, 0.f)
				[
						MakePowerUpPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.PowerUp", "AchievementCoinsBalanceLabel", "ACHIEVEMENT COINS"))
								.Font(FT66Style::Tokens::FontBold(15))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::AsNumber(AchievementCoinBalance))
								.Font(FT66Style::Tokens::FontBold(28))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							],
							T66PowerUpInsetFill(),
							FMargin(16.f, 14.f),
							ET66PanelType::Panel)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterHintText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)
				[
					MakePowerUpButton(
						FT66ButtonParams(
							FText::Format(NSLOCTEXT("T66.PowerUp", "ConvertOneButton", "CONVERT 1 PC (+{0} AC)"),
								FText::AsNumber(AchievementCoinsPerPowerCoupon)),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertOneClicked),
							ET66ButtonType::Primary)
						.SetHeight(52.f)
						.SetFontSize(16)
						.SetColor(T66PowerUpButtonFill())
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance >= 1; })))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 8.f, 0.f)
				[
					MakePowerUpButton(
						FT66ButtonParams(
							FText::Format(NSLOCTEXT("T66.PowerUp", "ConvertFiveButton", "CONVERT 5 PC (+{0} AC)"),
								FText::AsNumber(AchievementCoinsPerPowerCoupon * 5)),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertFiveClicked),
							ET66ButtonType::Neutral)
						.SetHeight(52.f)
						.SetFontSize(16)
						.SetColor(T66PowerUpNeutralButtonFill())
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance >= 5; })))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(8.f, 0.f, 0.f, 0.f)
				[
					MakePowerUpButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.PowerUp", "ConvertAllButton", "CONVERT ALL"),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertAllClicked),
							ET66ButtonType::Success)
						.SetHeight(52.f)
						.SetFontSize(16)
						.SetColor(T66PowerUpButtonFill())
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance > 0; })))
				]
			]
		,
		T66PowerUpPanelFill(),
		FMargin(18.f));

	const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f;
	const bool bShowBackButton = !(UIManager && UIManager->IsFrontendTopBarVisible());

	const TSharedRef<SWidget> Root =
		SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			MakePowerUpPanel(
		SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(40))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.Visibility_Lambda([this]() { return !bShowingConverter ? EVisibility::Visible : EVisibility::Collapsed; })
						[
							MakePowerUpButton(FT66ButtonParams(ConverterButtonText,
								FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleOpenConverterClicked),
								ET66ButtonType::Neutral)
								.SetMinWidth(210.f)
								.SetHeight(52.f)
								.SetFontSize(18)
								.SetColor(T66PowerUpNeutralButtonFill())
							)
						]
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					.WidgetIndex(bShowingConverter ? 1 : 0)
					+ SWidgetSwitcher::Slot()[ StatsPage ]
					+ SWidgetSwitcher::Slot()[ ConverterPage ]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.f, 0.f, 0.f, 20.f)
			[
				SNew(SBox)
				.Visibility(bShowBackButton ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakePowerUpButton(FT66ButtonParams(BackText,
						FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleBackClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(108.f)
						.SetFontSize(18)
						.SetColor(T66PowerUpNeutralButtonFill())
					)
				]
			]
		,
		T66PowerUpShellFill(),
		FMargin(18.f))
		];

	return Root;
}
