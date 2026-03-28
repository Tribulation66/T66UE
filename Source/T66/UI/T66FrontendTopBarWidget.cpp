// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendTopBarWidget.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "UI/Dota/T66DotaTheme.h"
#include "UI/ST66PulsingIcon.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float GTopBarReservedHeight = 146.f;
	constexpr float GTopBarButtonWidth = 92.f;
	constexpr float GTopBarButtonHeight = 78.f;
	constexpr float GTopBarSurfaceHeight = 118.f;
	constexpr float GTopBarPlateVisibleHeightRatio = 98.f / 108.f;
	constexpr float GTopBarInactiveNavHeight = 72.f;
	constexpr float GTopBarActiveNavHeight = 80.f;
	constexpr float GTopBarInactiveNavMinWidth = 176.f;
	constexpr float GTopBarActiveNavMinWidth = 188.f;
	constexpr float GTopBarIconOnlyNavWidth = 80.f;
	constexpr float GTopBarNavSeparatorHorizontalPadding = 10.f;
	constexpr float GTopBarRightClusterGap = 12.f;
	constexpr int32 GTopBarViewportZOrder = 50;
	TMap<FString, TStrongObjectPtr<UTexture2D>> GTopBarFileTextureCache;

	FVector2D GetEffectiveFrontendViewportSize()
	{
		FVector2D EffectiveSize = FT66Style::GetViewportSize();

		if (GEngine)
		{
			if (UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings())
			{
				const FIntPoint ScreenResolution = GameUserSettings->GetScreenResolution();
				if (ScreenResolution.X > 0 && ScreenResolution.Y > 0
					&& GameUserSettings->GetFullscreenMode() != EWindowMode::Windowed)
				{
					EffectiveSize.X = FMath::Max(EffectiveSize.X, static_cast<float>(ScreenResolution.X));
					EffectiveSize.Y = FMath::Max(EffectiveSize.Y, static_cast<float>(ScreenResolution.Y));
				}
			}
		}

		return EffectiveSize;
	}

	float GetTopBarLayoutScale()
	{
		const FVector2D ReferenceViewportSize(1920.f, 1080.f);
		const FVector2D EffectiveViewportSize = GetEffectiveFrontendViewportSize();
		const float WidthScale = EffectiveViewportSize.X / FMath::Max(ReferenceViewportSize.X, 1.f);
		const float HeightScale = EffectiveViewportSize.Y / FMath::Max(ReferenceViewportSize.Y, 1.f);
		return FMath::Clamp(FMath::Sqrt(WidthScale * HeightScale), 0.70f, 3.0f);
	}

	UTexture2D* LoadTopBarFileTexture(const FString& FilePath)
	{
		if (const TStrongObjectPtr<UTexture2D>* CachedTexture = GTopBarFileTextureCache.Find(FilePath))
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

		GTopBarFileTextureCache.Add(FilePath, TStrongObjectPtr<UTexture2D>(Texture));
		return Texture;
	}

	void LoadBrushFromRelativePath(const FString& RelativePath, TSharedPtr<FSlateBrush>& Brush, const FVector2D& DesiredSize = FVector2D::ZeroVector)
	{
		if (Brush.IsValid())
		{
			return;
		}

		const FString FullPath = FPaths::ProjectDir() / RelativePath;
		if (UTexture2D* Texture = LoadTopBarFileTexture(FullPath))
		{
			Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->Tiling = ESlateBrushTileType::NoTile;
			Brush->ImageSize = DesiredSize.IsNearlyZero()
				? FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()))
				: DesiredSize;
			Brush->SetResourceObject(Texture);
			Brush->TintColor = FSlateColor(FLinearColor::White);
		}
	}
}

UT66FrontendTopBarWidget::UT66FrontendTopBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

float UT66FrontendTopBarWidget::GetReservedHeight()
{
	return GTopBarReservedHeight * GetTopBarLayoutScale();
}

float UT66FrontendTopBarWidget::GetVisibleContentHeight()
{
	return (GTopBarSurfaceHeight * GTopBarPlateVisibleHeightRatio) * GetTopBarLayoutScale();
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::RebuildWidget()
{
	// The top bar computes its own resolution-aware scale and reserved height.
	return BuildSlateUI();
}

UT66LocalizationSubsystem* UT66FrontendTopBarWidget::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

UT66FrontendTopBarWidget::ETopBarSection UT66FrontendTopBarWidget::GetActiveSection() const
{
	if (!UIManager)
	{
		return ETopBarSection::Home;
	}

	ET66ScreenType FocusedScreenType = UIManager->GetCurrentScreenType();
	if (UIManager->IsModalActive() && UIManager->GetCurrentModalType() == ET66ScreenType::AccountStatus)
	{
		FocusedScreenType = ET66ScreenType::AccountStatus;
	}

	switch (FocusedScreenType)
	{
	case ET66ScreenType::AccountStatus:
		return ETopBarSection::AccountStatus;
	case ET66ScreenType::PowerUp:
		return ETopBarSection::PowerUp;
	case ET66ScreenType::Achievements:
		return ETopBarSection::Achievements;
	case ET66ScreenType::MainMenu:
	default:
		return ETopBarSection::Home;
	}
}

FText UT66FrontendTopBarWidget::GetAchievementCoinsValueText() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return FText::AsNumber(Achievements->GetAchievementCoinsBalance());
		}
	}

	return FText::AsNumber(0);
}

FText UT66FrontendTopBarWidget::GetPowerCouponsValueText() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PowerUpSubsystem* PowerUp = GI->GetSubsystem<UT66PowerUpSubsystem>())
		{
			return FText::AsNumber(PowerUp->GetPowerCrystalBalance());
		}
	}

	return FText::AsNumber(0);
}

void UT66FrontendTopBarWidget::RequestTopBarAssets()
{
#if WITH_EDITOR
	TopBarPlateBrush.Reset();
	InactiveTabBrush.Reset();
	ActiveTabBrush.Reset();
	NavSeparatorBrush.Reset();
	SettingsSlotBrush.Reset();
	UtilitySlotBrush.Reset();
	QuitSlotBrush.Reset();
	SettingsIconBrush.Reset();
	LanguageIconBrush.Reset();
	AchievementCoinsIconBrush.Reset();
	PowerCouponsIconBrush.Reset();
	QuitIconBrush.Reset();
	QuitGlowBrush.Reset();
	GTopBarFileTextureCache.Empty();
#endif

	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_plate.png"), TopBarPlateBrush);
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_nav_tab.png"), InactiveTabBrush);
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_active_tab.png"), ActiveTabBrush);
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_separator.png"), NavSeparatorBrush);
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_slot.png"), SettingsSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_utility_slot.png"), UtilitySlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_slot.png"), QuitSlotBrush, FVector2D(78.f, 78.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_settings_icon.png"), SettingsIconBrush, FVector2D(44.f, 44.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_language_icon.png"), LanguageIconBrush, FVector2D(44.f, 44.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_achievement_coins_icon.png"), AchievementCoinsIconBrush, FVector2D(44.f, 44.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_power_coupons_icon.png"), PowerCouponsIconBrush, FVector2D(44.f, 44.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_icon.png"), QuitIconBrush, FVector2D(44.f, 44.f));
	LoadBrushFromRelativePath(TEXT("SourceAssets/UI/Dota/Generated/frontend_topbar_quit_glow.png"), QuitGlowBrush, FVector2D(62.f, 62.f));

	if (TopBarPlateBrush.IsValid())
	{
		TopBarPlateBrush->DrawAs = ESlateBrushDrawType::Box;
		TopBarPlateBrush->Margin = FMargin(0.10f, 0.18f, 0.10f, 0.22f);
	}

	if (InactiveTabBrush.IsValid())
	{
		InactiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		InactiveTabBrush->Margin = FMargin(0.16f, 0.18f, 0.16f, 0.28f);
	}

	if (ActiveTabBrush.IsValid())
	{
		ActiveTabBrush->DrawAs = ESlateBrushDrawType::Box;
		ActiveTabBrush->Margin = FMargin(0.16f, 0.16f, 0.16f, 0.28f);
	}
}

void UT66FrontendTopBarWidget::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	FT66Style::DeferRebuild(this, GTopBarViewportZOrder);
}

TSharedRef<SWidget> UT66FrontendTopBarWidget::BuildSlateUI()
{
	CachedViewportSize = GetEffectiveFrontendViewportSize();
	bViewportResponsiveRebuildQueued = false;
	RequestTopBarAssets();

	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const ETopBarSection ActiveSection = GetActiveSection();

	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.MainMenu", "Settings", "SETTINGS");
	const FText LanguageText = Loc ? Loc->GetText_LangButton() : NSLOCTEXT("T66.LanguageSelect", "LangButton", "LANG");
	const FText HomeText = NSLOCTEXT("T66.MainMenu", "Home", "HOME");
	const FText PowerUpText = NSLOCTEXT("T66.MainMenu", "PowerUp", "POWER UP");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.MainMenu", "Achievements", "ACHIEVEMENTS");
	const FText AccountStatusText = NSLOCTEXT("T66.AccountStatus", "TopBarTitle", "ACCOUNT STATUS");
	const FText QuitTooltipText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.MainMenu", "Quit", "QUIT");

	const FButtonStyle& FlatButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>(TEXT("T66.Button.FlatTransparent"));
	const float LayoutScale = GetTopBarLayoutScale();
	const float TopBarButtonWidth = GTopBarButtonWidth * LayoutScale;
	const float TopBarButtonHeight = GTopBarButtonHeight * LayoutScale;
	const float TopBarSurfaceHeight = GTopBarSurfaceHeight * LayoutScale;
	const float InactiveNavHeight = GTopBarInactiveNavHeight * LayoutScale;
	const float ActiveNavHeight = GTopBarActiveNavHeight * LayoutScale;
	const float InactiveNavMinWidth = GTopBarInactiveNavMinWidth * LayoutScale;
	const float ActiveNavMinWidth = GTopBarActiveNavMinWidth * LayoutScale;
	const float UtilityClusterGap = 12.f * LayoutScale;
	const float ClusterGap = 18.f * LayoutScale;
	const float NavButtonGap = 8.f * LayoutScale;
	const float ReadoutGap = 10.f * LayoutScale;
	const float QuitGap = 12.f * LayoutScale;
	const float RowHorizontalPadding = 18.f * LayoutScale;
	const float RowTopPadding = 8.f * LayoutScale;
	const FVector2D UtilitySlotSize = FVector2D(78.f, 78.f) * LayoutScale;
	const FVector2D SettingsIconSize = FVector2D(44.f, 44.f) * LayoutScale;
	const FVector2D LanguageIconSize = FVector2D(50.f, 50.f) * LayoutScale;
	const FVector2D BalanceIconSize = FVector2D(50.f, 50.f) * LayoutScale;
	const FVector2D QuitIconSize = FVector2D(44.f, 44.f) * LayoutScale;
	const FVector2D QuitGlowSize = FVector2D(70.f, 70.f) * LayoutScale;
	constexpr int32 TopBarTitleFontSize = 16;

	FSlateFontInfo NavFont = FT66DotaTheme::MakeFont(TEXT("Bold"), TopBarTitleFontSize);
	NavFont.LetterSpacing = 14;

	FSlateFontInfo ActiveNavFont = FT66DotaTheme::MakeFont(TEXT("Bold"), TopBarTitleFontSize + 1);
	ActiveNavFont.LetterSpacing = 16;

	FSlateFontInfo BalanceFont = FT66DotaTheme::MakeFont(TEXT("Bold"), TopBarTitleFontSize);
	BalanceFont.LetterSpacing = 8;

	auto MakeFallbackIconLabel = [](const FText& FallbackText) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(FallbackText)
			.Font(FT66Style::Tokens::FontBold(TopBarTitleFontSize))
			.ColorAndOpacity(FLinearColor(0.92f, 0.90f, 0.84f, 1.0f));
	};

	auto MakeBrushIconWidget = [&MakeFallbackIconLabel](const TSharedPtr<FSlateBrush>& IconBrush, const FVector2D& Size, const FText& FallbackText) -> TSharedRef<SWidget>
	{
		if (!IconBrush.IsValid())
		{
			return MakeFallbackIconLabel(FallbackText);
		}

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SImage)
				.Image(IconBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
			];
	};

	auto MakeOffsetWidget = [](const TSharedRef<SWidget>& Widget, const FVector2D& Offset) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.RenderTransform(FSlateRenderTransform(Offset))
			[
				Widget
			];
	};

	auto MakeNavLabel = [&NavFont, &ActiveNavFont](const FText& Text, bool bActive) -> TSharedRef<SWidget>
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(bActive ? ActiveNavFont : NavFont)
			.ColorAndOpacity(bActive ? FT66DotaTheme::Text() : FLinearColor(0.80f, 0.82f, 0.84f, 0.94f))
			.ShadowOffset(FVector2D(0.f, 1.f))
			.ShadowColorAndOpacity(bActive ? FLinearColor(0.f, 0.f, 0.f, 0.55f) : FLinearColor(0.f, 0.f, 0.f, 0.72f))
			.Justification(ETextJustify::Center);
	};

	auto MakeNavContent =
		[&MakeBrushIconWidget, &MakeNavLabel](
			const FText& Text,
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			bool bIconOnly) -> TSharedRef<SWidget>
	{
		if (bIconOnly)
		{
			return MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText);
		}

		if (!IconBrush.IsValid())
		{
			return MakeNavLabel(Text, bActive);
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				MakeBrushIconWidget(IconBrush, IconSize, IconFallbackText)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				MakeNavLabel(Text, bActive)
			];
	};

	auto MakeNavButton =
		[this, &FlatButtonStyle, &MakeNavContent, LayoutScale, TopBarButtonWidth = TopBarButtonWidth, LanguageIconSize = LanguageIconSize, ActiveNavMinWidth = ActiveNavMinWidth, ActiveNavHeight = ActiveNavHeight, InactiveNavMinWidth = InactiveNavMinWidth, InactiveNavHeight = InactiveNavHeight](
			const FText& Text,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			bool bActive,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& IconFallbackText,
			bool bIconOnly = false) -> TSharedRef<SWidget>
	{
		const float IconOnlyNavWidth = FMath::Max(TopBarButtonWidth, LanguageIconSize.X + (28.f * LayoutScale));
		const TSharedPtr<FSlateBrush>& BackgroundBrush = InactiveTabBrush.IsValid() ? InactiveTabBrush : ActiveTabBrush;
		const float ButtonMinWidth = bActive ? ActiveNavMinWidth : InactiveNavMinWidth;
		const float ButtonHeight = bActive ? ActiveNavHeight : InactiveNavHeight;
		return SNew(SBox)
			.MinDesiredWidth(bIconOnly ? IconOnlyNavWidth : ButtonMinWidth)
			.HeightOverride(ButtonHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(BackgroundBrush.Get())
					.ColorAndOpacity(FLinearColor::White)
					.Visibility(BackgroundBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(&FlatButtonStyle)
					.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(bIconOnly ? FMargin(18.f * LayoutScale) : FMargin(24.f * LayoutScale, 12.f * LayoutScale, 24.f * LayoutScale, 12.f * LayoutScale))
					[
						MakeNavContent(Text, bActive, IconBrush, IconSize, IconFallbackText, bIconOnly)
					]
				]
			];
	};

	auto MakeUtilityButton =
		[this, &FlatButtonStyle, &MakeOffsetWidget, TopBarButtonWidth, TopBarButtonHeight](
			const FText& TooltipText,
			const TSharedRef<SWidget>& ContentWidget,
			FReply (UT66FrontendTopBarWidget::*ClickFunc)(),
			const TSharedPtr<FSlateBrush>& SlotBrush = TSharedPtr<FSlateBrush>(),
			const FVector2D& SlotSize = FVector2D::ZeroVector,
			const FVector2D& ContentOffset = FVector2D::ZeroVector) -> TSharedRef<SWidget>
	{
		const TSharedRef<SWidget> DisplayContent =
			ContentOffset.IsNearlyZero()
			? ContentWidget
			: MakeOffsetWidget(ContentWidget, ContentOffset);

		return SNew(SBox)
			.WidthOverride(TopBarButtonWidth)
			.HeightOverride(TopBarButtonHeight)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(SlotSize.X)
					.HeightOverride(SlotSize.Y)
					.Visibility(SlotBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
					[
						SNew(SImage)
						.Image(SlotBrush.Get())
						.ColorAndOpacity(FLinearColor::White)
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(&FlatButtonStyle)
					.ToolTipText(TooltipText)
					.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
					.ContentPadding(FMargin(0.f))
					[
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							DisplayContent
						]
					]
				]
			];
	};

	auto MakeBalanceReadout =
		[this, &BalanceFont, &MakeBrushIconWidget, LayoutScale, InactiveNavHeight = InactiveNavHeight](
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FVector2D& IconSize,
			const FText& TooltipText,
			const FText& FallbackText,
			FText (UT66FrontendTopBarWidget::*ValueGetter)() const) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HeightOverride(InactiveNavHeight)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ToolTipText(TooltipText)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					MakeBrushIconWidget(IconBrush, IconSize, FallbackText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.f * LayoutScale, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text_Lambda([this, ValueGetter]() -> FText
					{
						return (this->*ValueGetter)();
					})
					.Font(BalanceFont)
					.ColorAndOpacity(FT66DotaTheme::Text())
					.ShadowOffset(FVector2D(0.f, 1.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.55f))
				]
			];
	};

	const TSharedRef<SWidget> SettingsIconWidget = MakeBrushIconWidget(SettingsIconBrush, SettingsIconSize, NSLOCTEXT("T66.MainMenu", "SettingsFallback", "S"));
	const TSharedRef<SWidget> LanguageIconWidget = MakeBrushIconWidget(LanguageIconBrush, LanguageIconSize, NSLOCTEXT("T66.LanguageSelect", "LanguageIconFallback", "L"));
	const TSharedRef<SWidget> SettingsButtonWidget =
		MakeUtilityButton(
			SettingsText,
			SettingsIconWidget,
			&UT66FrontendTopBarWidget::HandleSettingsClicked,
			SettingsSlotBrush,
			UtilitySlotSize);
	const TSharedRef<SWidget> LanguageButtonWidget =
		MakeUtilityButton(
			LanguageText,
			LanguageIconWidget,
			&UT66FrontendTopBarWidget::HandleLanguageClicked,
			UtilitySlotBrush,
			UtilitySlotSize);
	const TSharedRef<SWidget> AchievementCoinsWidget =
		MakeBalanceReadout(
			AchievementCoinsIconBrush,
			BalanceIconSize,
			NSLOCTEXT("T66.Achievements", "AchievementCoinsBalanceTooltip", "Achievement Coins"),
			NSLOCTEXT("T66.Achievements", "AchievementCoinsFallback", "A"),
			&UT66FrontendTopBarWidget::GetAchievementCoinsValueText);
	const TSharedRef<SWidget> PowerCouponsWidget =
		MakeBalanceReadout(
			PowerCouponsIconBrush,
			BalanceIconSize,
			NSLOCTEXT("T66.PowerUp", "PowerCouponsBalanceTooltip", "Power Coupons"),
			NSLOCTEXT("T66.PowerUp", "PowerCouponsFallback", "P"),
			&UT66FrontendTopBarWidget::GetPowerCouponsValueText);
	const TSharedRef<SWidget> QuitIconWidget =
		(QuitIconBrush.IsValid() && QuitGlowBrush.IsValid())
		? StaticCastSharedRef<SWidget>(
			SNew(ST66PulsingIcon)
			.IconBrush(QuitIconBrush.Get())
			.GlowBrush(QuitGlowBrush.Get())
			.IconSize(QuitIconSize)
			.GlowSize(QuitGlowSize)
			.GlowOpacityMin(0.34f)
			.GlowOpacityMax(0.62f)
			.GlowOpacityFrequency(0.48f)
			.GlowScaleAmplitude(0.028f)
			.GlowScaleFrequency(0.30f))
		: MakeBrushIconWidget(QuitIconBrush, QuitIconSize, NSLOCTEXT("T66.MainMenu", "QuitFallback", "Q"));

	const TSharedRef<SWidget> LeftUtilityCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SettingsButtonWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(UtilityClusterGap, 0.f, 0.f, 0.f)
		[
			LanguageButtonWidget
		];

	const TSharedRef<SWidget> CenterNavCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeNavButton(AccountStatusText, &UT66FrontendTopBarWidget::HandleAccountStatusClicked, ActiveSection == ETopBarSection::AccountStatus, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(HomeText, &UT66FrontendTopBarWidget::HandleHomeClicked, ActiveSection == ETopBarSection::Home, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(PowerUpText, &UT66FrontendTopBarWidget::HandlePowerUpClicked, ActiveSection == ETopBarSection::PowerUp, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(NavButtonGap, 0.f, 0.f, 0.f)
		[
			MakeNavButton(AchievementsText, &UT66FrontendTopBarWidget::HandleAchievementsClicked, ActiveSection == ETopBarSection::Achievements, TSharedPtr<FSlateBrush>(), FVector2D::ZeroVector, FText::GetEmpty(), false)
		];

	const TSharedRef<SWidget> RightUtilityCluster =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			PowerCouponsWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(ReadoutGap, 0.f, 0.f, 0.f)
		[
			AchievementCoinsWidget
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(QuitGap, 0.f, 0.f, 0.f)
		[
			MakeUtilityButton(QuitTooltipText, QuitIconWidget, &UT66FrontendTopBarWidget::HandleQuitClicked, QuitSlotBrush, UtilitySlotSize)
		];

	const TSharedRef<SWidget> TopBarRow =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			LeftUtilityCluster
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(ClusterGap, 0.f)
		[
			CenterNavCluster
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			RightUtilityCluster
		];

	const TSharedRef<SWidget> Surface =
		SNew(SBox)
		.HeightOverride(TopBarSurfaceHeight)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			[
				SNew(SImage)
				.Image(TopBarPlateBrush.Get())
				.ColorAndOpacity(FLinearColor::White)
				.Visibility(TopBarPlateBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(RowHorizontalPadding, RowTopPadding, RowHorizontalPadding, 0.f)
			[
				TopBarRow
			]
		];

	return SNew(SVerticalBox)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(GetReservedHeight())
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Top)
				[
					Surface
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNullWidget::NullWidget
		];
}

void UT66FrontendTopBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bViewportResponsiveRebuildQueued)
	{
		return;
	}

	const FVector2D CurrentViewportSize = GetEffectiveFrontendViewportSize();
	if (!CurrentViewportSize.Equals(CachedViewportSize, 1.0f))
	{
		CachedViewportSize = CurrentViewportSize;
		bViewportResponsiveRebuildQueued = true;
		ForceRebuildSlate();
	}
}

FReply UT66FrontendTopBarWidget::HandleSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleLanguageClicked()
{
	ShowModal(ET66ScreenType::LanguageSelect);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleHomeClicked()
{
	NavigateTo(ET66ScreenType::MainMenu);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandlePowerUpClicked()
{
	NavigateTo(ET66ScreenType::PowerUp);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleUnlocksClicked()
{
	NavigateTo(ET66ScreenType::Unlocks);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAchievementsClicked()
{
	NavigateTo(ET66ScreenType::Achievements);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleAccountStatusClicked()
{
	ShowModal(ET66ScreenType::AccountStatus);
	return FReply::Handled();
}

FReply UT66FrontendTopBarWidget::HandleQuitClicked()
{
	ShowModal(ET66ScreenType::QuitConfirmation);
	return FReply::Handled();
}
