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
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D PowerUpStatueImageSize(200.f, 280.f);
	const FLinearColor PowerUpLockedTint(0.03f, 0.03f, 0.04f, 0.96f);
	const FLinearColor PowerUpFillTint = FLinearColor::White;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GPowerUpFileTextureCache;

	FLinearColor T66FrontendShellFill()
	{
		return FLinearColor(0.004f, 0.005f, 0.010f, 0.985f);
	}

	FLinearColor T66PowerUpPanelFill()
	{
		return FLinearColor(0.024f, 0.025f, 0.030f, 1.0f);
	}

	FLinearColor T66PowerUpInsetFill()
	{
		return FLinearColor(0.018f, 0.019f, 0.024f, 1.0f);
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
		Texture->Filter = TextureFilter::TF_Bilinear;
		Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
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

	void EnsureRuntimeImageBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
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
				EnsureRuntimeImageBrush(Brush, ImageSize);
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
			EnsureRuntimeImageBrush(Brush, ImageSize);
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

FReply UT66PowerUpScreen::HandleOpenConverterClicked()
{
	CurrentPage = 1;
	if (PageSwitcher.IsValid()) PageSwitcher->SetActiveWidgetIndex(1);
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
	UT66AchievementsSubsystem* Achievements = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
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
	UT66AchievementsSubsystem* Achievements = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
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

	const FText FillPercentFormatText = NSLOCTEXT("T66.PowerUp", "FillPercentFormat", "{0}% FILLED");
	const FText BonusOverflowFormatText = NSLOCTEXT("T66.PowerUp", "OverflowBonusFormat", "BONUS +{0}");
	const FText ConverterLabelText = NSLOCTEXT("T66.PowerUp", "Converter", "CONVERTER");
	const FText ConverterHintText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "ConverterHint", "Trade Power Coupons for Achievement Coins at {0} AC per coupon."),
		FText::AsNumber(AchievementCoinsPerPowerCoupon));
	const FText ConverterRateText = FText::Format(
		NSLOCTEXT("T66.PowerUp", "ConverterRate", "1 POWER COUPON = {0} ACHIEVEMENT COINS"),
		FText::AsNumber(AchievementCoinsPerPowerCoupon));
	const FText ConverterButtonText = NSLOCTEXT("T66.PowerUp", "ConverterButton", "PC -> AC CONVERTER");

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
		const int32 UnlockedSteps = PowerUp ? PowerUp->GetUnlockedFillStepCount(StatType) : 0;
		const int32 FillPercent = FMath::Clamp((UnlockedSteps * 100) / UT66PowerUpSubsystem::MaxFillStepsPerStat, 0, 100);
		const int32 TotalBonus = PowerUp ? PowerUp->GetTotalStatBonus(StatType) : 0;
		const int32 ExtraBonus = FMath::Max(0, TotalBonus - UnlockedSteps);
		const int32 Cost = PowerUp ? PowerUp->GetCostForNextFillStepUnlock(StatType) : 0;
		const bool bMaxed = PowerUp && PowerUp->IsStatMaxed(StatType);
		const FText ButtonText = bMaxed
			? NSLOCTEXT("T66.PowerUp", "Max", "MAX")
			: FText::Format(NSLOCTEXT("T66.PowerUp", "UnlockPCFormat", "UNLOCK {0} PC"), FText::AsNumber(Cost));
		const FText FillText = FText::Format(FillPercentFormatText, FText::AsNumber(FillPercent));
		const FText BonusText = FText::Format(BonusOverflowFormatText, FText::AsNumber(ExtraBonus));

		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(STextBlock)
				.Text(GetStatLabel(StatType))
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					MakeStatueWidget(StatType, PowerUpStatueImageSize)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f).HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FillText)
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f).HAlign(HAlign_Center)
				[
					ExtraBonus > 0
						? StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.Text(BonusText)
							.Font(FT66Style::Tokens::FontBold(11))
							.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.65f, 1.f)))
						: StaticCastSharedRef<SWidget>(SNew(SSpacer).Size(FVector2D(1.f, 18.f)))
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SSpacer)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(ButtonText, FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleUnlockClicked, StatType), ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetEnabled(TAttribute<bool>::CreateLambda([bMaxed, Balance, Cost]() { return !bMaxed && Balance >= Cost; }))
				)
			]
		,
		FT66PanelParams(ET66PanelType::Panel)
			.SetColor(T66PowerUpPanelFill())
			.SetPadding(FMargin(18.f, 16.f, 18.f, 18.f)));
	};

	TSharedRef<SVerticalBox> StatsPage =
		SNew(SVerticalBox)
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

	TSharedRef<SWidget> ConverterPage =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterLabelText)
				.Font(FT66Style::Tokens::FontBold(24))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterRateText)
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
				[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.PowerUp", "CouponsBalanceLabel", "POWER COUPONS"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::AsNumber(Balance))
								.Font(FT66Style::Tokens::FontBold(34))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							],
							FT66PanelParams(ET66PanelType::Panel)
								.SetColor(T66PowerUpInsetFill())
								.SetPadding(FMargin(20.f, 18.f)))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f, 0.f, 0.f, 0.f)
				[
						FT66Style::MakePanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.PowerUp", "AchievementCoinsBalanceLabel", "ACHIEVEMENT COINS"))
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(FText::AsNumber(AchievementCoinBalance))
								.Font(FT66Style::Tokens::FontBold(34))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							],
							FT66PanelParams(ET66PanelType::Panel)
								.SetColor(T66PowerUpInsetFill())
								.SetPadding(FMargin(20.f, 18.f)))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f).HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(ConverterHintText)
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 22.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							FText::Format(NSLOCTEXT("T66.PowerUp", "ConvertOneButton", "CONVERT 1 PC (+{0} AC)"),
								FText::AsNumber(AchievementCoinsPerPowerCoupon)),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertOneClicked),
							ET66ButtonType::Primary)
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance >= 1; })))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 10.f, 0.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							FText::Format(NSLOCTEXT("T66.PowerUp", "ConvertFiveButton", "CONVERT 5 PC (+{0} AC)"),
								FText::AsNumber(AchievementCoinsPerPowerCoupon * 5)),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertFiveClicked),
							ET66ButtonType::Neutral)
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance >= 5; })))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 0.f, 0.f)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.PowerUp", "ConvertAllButton", "CONVERT ALL"),
							FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleConvertAllClicked),
							ET66ButtonType::Success)
						.SetEnabled(TAttribute<bool>::CreateLambda([Balance]() { return Balance > 0; })))
				]
			]
		,
		FT66PanelParams(ET66PanelType::Panel)
			.SetColor(T66PowerUpPanelFill())
			.SetPadding(FMargin(24.f)));

	CurrentPage = FMath::Clamp(CurrentPage, 0, 1);
	const float TopInset = UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f;
	const bool bShowBackButton = !(UIManager && UIManager->IsFrontendTopBarVisible());

	return SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			FT66Style::MakePanel(
		SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(32))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.Visibility_Lambda([this]() { return CurrentPage == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
						[
							FT66Style::MakeButton(FT66ButtonParams(ConverterButtonText,
								FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleOpenConverterClicked),
								ET66ButtonType::Neutral)
								.SetMinWidth(220.f)
							)
						]
					]
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SAssignNew(PageSwitcher, SWidgetSwitcher)
					.WidgetIndex(CurrentPage)
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
					FT66Style::MakeButton(FT66ButtonParams(BackText,
						FOnClicked::CreateUObject(this, &UT66PowerUpScreen::HandleBackClicked),
						ET66ButtonType::Neutral)
						.SetMinWidth(120.f)
					)
				]
			]
		,
		FT66PanelParams(ET66PanelType::Panel)
			.SetColor(T66FrontendShellFill())
			.SetPadding(FMargin(24.f)))
		];
}
