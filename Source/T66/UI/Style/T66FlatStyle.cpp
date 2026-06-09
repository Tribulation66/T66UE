// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66FlatStyle.h"

#include "Brushes/SlateColorBrush.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Styling/CoreStyle.h"
#include "UI/T66TooltipResolvers.h"
#include "UI/T66TooltipSlate.h"
#include "UI/Style/T66FlatWidgetMetadata.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FLinearColor HexColor(uint8 R, uint8 G, uint8 B, float Alpha = 1.0f)
	{
		FLinearColor Color = FLinearColor::FromSRGBColor(FColor(R, G, B, 255));
		Color.A = Alpha;
		return Color;
	}

	const FSlateBrush* FlatWhiteBrush()
	{
		return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	}

	const FButtonStyle& FlatNoBorderButtonStyle()
	{
		static FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder"));
		static bool bInitialized = false;
		if (!bInitialized)
		{
			Style.SetNormalPadding(FMargin(0.f));
			Style.SetPressedPadding(FMargin(0.f));
			bInitialized = true;
		}
		return Style;
	}

	TAutoConsoleVariable<int32> CVarT66FriendslopGlobalChrome(
		TEXT("T66.UI.FriendslopGlobal"),
		1,
		TEXT("When 1 (default), FlatStyle chrome entry points render FriendslopStyle plates; FlatStyle becomes a legacy-compat adapter. 0 restores flat rendering. Command line -T66FlatLegacy forces 0."),
		ECVF_Default);

	bool UseFriendslopGlobalChrome()
	{
		static const bool bForcedLegacy = FParse::Param(FCommandLine::Get(), TEXT("T66FlatLegacy"));
		return !bForcedLegacy && CVarT66FriendslopGlobalChrome.GetValueOnGameThread() > 0;
	}

	ET66FlatState FlatStateForOverlayBrush(const ET66FlatOverlayChromeBrush Brush)
	{
		switch (Brush)
		{
		case ET66FlatOverlayChromeBrush::SlotSelected:
		case ET66FlatOverlayChromeBrush::OfferCardSelected:
		case ET66FlatOverlayChromeBrush::CrateWinnerMarker:
			return ET66FlatState::Selected;
		case ET66FlatOverlayChromeBrush::SlotDisabled:
		case ET66FlatOverlayChromeBrush::OfferCardDisabled:
			return ET66FlatState::Disabled;
		case ET66FlatOverlayChromeBrush::SlotHover:
		case ET66FlatOverlayChromeBrush::OfferCardHover:
			return ET66FlatState::Ready;
		default:
			return ET66FlatState::Default;
		}
	}

	ET66FlatState FlatStateForOverlayButton(
		const ET66FlatOverlayChromeButtonFamily Family,
		const bool bSelected,
		const bool bEnabled)
	{
		if (!bEnabled)
		{
			return ET66FlatState::Disabled;
		}
		if (bSelected || Family == ET66FlatOverlayChromeButtonFamily::Danger)
		{
			return ET66FlatState::Selected;
		}
		if (Family == ET66FlatOverlayChromeButtonFamily::Primary
			|| Family == ET66FlatOverlayChromeButtonFamily::Central
			|| Family == ET66FlatOverlayChromeButtonFamily::Select)
		{
			return ET66FlatState::Ready;
		}
		return ET66FlatState::Default;
	}

	FComboButtonStyle& FlatComboButtonStyle()
	{
		static FComboButtonStyle Style = FT66Style::GetDropdownComboButtonStyle();
		static bool bInitialized = false;
		if (!bInitialized)
		{
			Style.ButtonStyle = FlatNoBorderButtonStyle();
			bInitialized = true;
		}
		return Style;
	}

	struct FFlatDropdownMenuContext
	{
		float Width = 0.f;
		float Height = 42.f;
		int32 FontSize = 20;
	};

	TArray<FFlatDropdownMenuContext> GFlatDropdownMenuContextStack;

	class FScopedFlatDropdownMenuContext
	{
	public:
		FScopedFlatDropdownMenuContext(const float InWidth, const float InHeight, const int32 InFontSize)
		{
			GFlatDropdownMenuContextStack.Add({
				InWidth,
				InHeight > 0.f ? InHeight : 42.f,
				InFontSize > 0 ? InFontSize : 20
			});
		}

		~FScopedFlatDropdownMenuContext()
		{
			if (GFlatDropdownMenuContextStack.Num() > 0)
			{
				GFlatDropdownMenuContextStack.Pop(EAllowShrinking::No);
			}
		}
	};

	const FFlatDropdownMenuContext* GetActiveFlatDropdownMenuContext()
	{
		return GFlatDropdownMenuContextStack.Num() > 0
			? &GFlatDropdownMenuContextStack.Last()
			: nullptr;
	}

	const FProgressBarStyle& FlatProgressBarStyle()
	{
		static FSlateColorBrush BackgroundBrush(FT66FlatStyle::DisabledBorder());
		static FSlateColorBrush FillBrush(FLinearColor::White);
		static FSlateColorBrush MarqueeBrush(FLinearColor::Transparent);
		static FProgressBarStyle Style;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			Style.SetBackgroundImage(BackgroundBrush)
				.SetFillImage(FillBrush)
				.SetMarqueeImage(MarqueeBrush);
			bInitialized = true;
		}
		return Style;
	}

	FSlateFontInfo FlatFont(const TCHAR* Weight, int32 Size)
	{
		const FString LockedFontPath = T66RuntimeUIFontAccess::ResolveLockedUIFontPath();
		if (!LockedFontPath.IsEmpty())
		{
			return T66RuntimeUIFontAccess::MakeFontFromAbsoluteFile(LockedFontPath, Size);
		}
		return T66RuntimeUIFontAccess::MakeLocalizedEngineFont(Size, T66RuntimeUIFontAccess::IsBoldWeight(Weight));
	}

	int32 FontSizeForRole(const ET66FlatLabelRole Role)
	{
		switch (Role)
		{
		case ET66FlatLabelRole::Title:
			return 44;
		case ET66FlatLabelRole::Header:
			return 28;
		case ET66FlatLabelRole::SubHeader:
		case ET66FlatLabelRole::PurpleAccent:
			return 22;
		case ET66FlatLabelRole::Caption:
			return 15;
		case ET66FlatLabelRole::StatValue:
			return 20;
		case ET66FlatLabelRole::Button:
			return 20;
		case ET66FlatLabelRole::StatLabel:
		case ET66FlatLabelRole::Body:
		default:
			return 18;
		}
	}

	FSlateColor TextColorForRole(const ET66FlatLabelRole Role)
	{
		switch (Role)
		{
		case ET66FlatLabelRole::SubHeader:
		case ET66FlatLabelRole::PurpleAccent:
		case ET66FlatLabelRole::StatLabel:
			return FSlateColor(FT66FlatStyle::PurpleAccent());
		case ET66FlatLabelRole::Caption:
			return FSlateColor(FT66FlatStyle::SecondaryText());
		case ET66FlatLabelRole::Body:
			return FSlateColor(FT66FlatStyle::PrimaryText());
		case ET66FlatLabelRole::Title:
		case ET66FlatLabelRole::Header:
		case ET66FlatLabelRole::StatValue:
		case ET66FlatLabelRole::Button:
		default:
			return FSlateColor(FT66FlatStyle::PrimaryText());
		}
	}

	TSharedRef<SWidget> WrapInOptionalBox(
		const TSharedRef<SWidget>& Content,
		const float MinWidth,
		const float Height)
	{
		TSharedRef<SBox> Box = SNew(SBox)
			.MinDesiredWidth(MinWidth > 0.f ? FOptionalSize(MinWidth) : FOptionalSize())
			.HeightOverride(Height > 0.f ? FOptionalSize(Height) : FOptionalSize())
			[
				Content
		];
		return Box;
	}

	TSharedPtr<TWeakPtr<SWidget>> MakeHoverProbe()
	{
		return MakeShared<TWeakPtr<SWidget>>();
	}

	void BindHoverProbe(const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe, const TSharedRef<SWidget>& Widget)
	{
		if (HoverProbe.IsValid())
		{
			*HoverProbe = Widget;
		}
	}

	bool IsHoverProbeHovered(const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		if (!HoverProbe.IsValid())
		{
			return false;
		}

		const TSharedPtr<SWidget> Widget = HoverProbe->Pin();
		return Widget.IsValid() && Widget->IsHovered();
	}

	bool IsHoverVisualActive(
		const ET66FlatState State,
		const TAttribute<bool>& IsEnabled,
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		return State != ET66FlatState::Disabled
			&& IsEnabled.Get(true)
			&& IsHoverProbeHovered(HoverProbe);
	}

	FSlateColor InteractiveBorderColorForState(
		const ET66FlatState State,
		const TAttribute<bool>& IsEnabled,
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		return FSlateColor(IsHoverVisualActive(State, IsEnabled, HoverProbe)
			? FT66FlatStyle::HoverBorder()
			: FT66FlatStyle::BorderForState(State));
	}

	FSlateColor InteractiveFillColorForState(
		const ET66FlatState State,
		const TAttribute<bool>& IsEnabled,
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		return FSlateColor(IsHoverVisualActive(State, IsEnabled, HoverProbe)
			? FT66FlatStyle::HoverFill()
			: FT66FlatStyle::FillForState(State));
	}

	FSlateColor InteractiveTextColorForState(
		const ET66FlatState State,
		const TAttribute<bool>& IsEnabled,
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		return IsHoverVisualActive(State, IsEnabled, HoverProbe)
			? FSlateColor(FT66FlatStyle::HoverText())
			: FT66FlatStyle::TextColorForState(State);
	}

	TAttribute<FSlateColor> MakeInteractiveTextColorAttribute(
		const ET66FlatState State,
		const TAttribute<bool>& IsEnabled,
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe)
	{
		return TAttribute<FSlateColor>::CreateLambda([State, IsEnabled, HoverProbe]() -> FSlateColor
		{
			return InteractiveTextColorForState(State, IsEnabled, HoverProbe);
		});
	}

	TSharedRef<SWidget> MakeFlatPanelSurface(
		const ET66FlatState State,
		const FMargin& Padding,
		const TSharedRef<SWidget>& Content,
		TSharedPtr<SBorder>* OutBorder = nullptr,
		TSharedPtr<SBorder>* OutFillBorder = nullptr,
		const TAttribute<bool>& IsEnabled = TAttribute<bool>(true),
		const TSharedPtr<TWeakPtr<SWidget>>& HoverProbe = nullptr)
	{
		if (UseFriendslopGlobalChrome())
		{
			// Friendslop global chrome: interactive surfaces (hover-probed) use the state-mapped
			// button plate, static surfaces use the large panel plate. Hover/disabled feedback is
			// a translucent film on the inner border so caller-driven dynamic recolors keep working.
			const ET66FriendslopChrome Chrome = HoverProbe.IsValid()
				? FT66FriendslopStyle::ButtonChromeForState(State)
				: ET66FriendslopChrome::PanelLargeDark;
			TSharedPtr<SBorder> FriendslopFillBorder;
			TSharedRef<SBorder> FriendslopBorder = SNew(SBorder)
				.BorderImage(FT66FriendslopStyle::GetChromeBrush(Chrome))
				.BorderBackgroundColor(FSlateColor(FLinearColor::White))
				.Padding(FMargin(FT66FlatStyle::FlatStroke))
				[
					SAssignNew(FriendslopFillBorder, SBorder)
					.BorderImage(FlatWhiteBrush())
					.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([State, IsEnabled, HoverProbe]() -> FSlateColor
					{
						if (State == ET66FlatState::Disabled || !IsEnabled.Get(true))
						{
							return FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.42f));
						}
						if (IsHoverVisualActive(State, IsEnabled, HoverProbe))
						{
							return FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.10f));
						}
						return FSlateColor(FLinearColor::Transparent);
					}))
					.Padding(Padding)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						Content
					]
				];

			if (OutBorder)
			{
				*OutBorder = FriendslopBorder;
			}
			if (OutFillBorder)
			{
				*OutFillBorder = FriendslopFillBorder;
			}

			return FT66FlatStyle::WrapWithoutRetainer(FriendslopBorder);
		}

		TSharedPtr<SBorder> FillBorder;
		TSharedRef<SBorder> Border = SNew(SBorder)
			.BorderImage(FlatWhiteBrush())
			.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([State, IsEnabled, HoverProbe]() -> FSlateColor
			{
				return InteractiveBorderColorForState(State, IsEnabled, HoverProbe);
			}))
			.Padding(FMargin(FT66FlatStyle::FlatStroke))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SAssignNew(FillBorder, SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([State, IsEnabled, HoverProbe]() -> FSlateColor
				{
					return InteractiveFillColorForState(State, IsEnabled, HoverProbe);
				}))
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				]
			];

		if (OutBorder)
		{
			*OutBorder = Border;
		}
		if (OutFillBorder)
		{
			*OutFillBorder = FillBorder;
		}

		return FT66FlatStyle::WrapWithoutRetainer(Border);
	}

	TSharedRef<SWidget> MakeFlatInteractiveBareButton(
		const ET66FlatState State,
		const TSharedRef<SWidget>& Content,
		FOnClicked OnClicked,
		const FMargin& Padding,
		const float MinWidth,
		const float Height,
		const TAttribute<bool>& IsEnabled,
		const FName Tag,
		const FName ToggleGroup,
		const FString& IntendedRole,
		const TSharedPtr<TWeakPtr<SWidget>>& ButtonHoverProbe)
	{
		const bool bHasClickHandler = OnClicked.IsBound();
		const bool bHoverCapable = bHasClickHandler && State != ET66FlatState::Disabled && IsEnabled.Get(true);
		const TSharedRef<SWidget> ButtonSurface = MakeFlatPanelSurface(State, Padding, Content, nullptr, nullptr, IsEnabled, ButtonHoverProbe);
		TSharedRef<SWidget> Button = FT66Style::MakeBareButton(
			FT66BareButtonParams(MoveTemp(OnClicked), WrapInOptionalBox(ButtonSurface, MinWidth, Height))
				.SetButtonStyle(&FlatNoBorderButtonStyle())
				.SetPadding(FMargin(0.f))
				.SetEnabled(IsEnabled)
				.SetMinWidth(MinWidth)
				.SetWidth(MinWidth)
				.SetHeight(Height));
		BindHoverProbe(ButtonHoverProbe, Button);
		return FT66FlatStyle::AttachMetadata(Button, Tag, IntendedRole, State, TOptional<FLinearColor>(), bHasClickHandler, ToggleGroup, false, bHoverCapable);
	}

	FString StateName(const ET66FlatState State)
	{
		switch (State)
		{
		case ET66FlatState::Disabled:
			return TEXT("Disabled");
		case ET66FlatState::Selected:
			return TEXT("Selected");
		case ET66FlatState::Ready:
			return TEXT("Ready");
		case ET66FlatState::Default:
		default:
			return TEXT("Default");
		}
	}

	FString LabelRoleName(const ET66FlatLabelRole Role)
	{
		switch (Role)
		{
		case ET66FlatLabelRole::Title:
			return TEXT("Title");
		case ET66FlatLabelRole::Header:
			return TEXT("Header");
		case ET66FlatLabelRole::SubHeader:
			return TEXT("SubHeader");
		case ET66FlatLabelRole::Body:
			return TEXT("Body");
		case ET66FlatLabelRole::Caption:
			return TEXT("Caption");
		case ET66FlatLabelRole::PurpleAccent:
			return TEXT("PurpleAccent");
		case ET66FlatLabelRole::StatLabel:
			return TEXT("StatLabel");
		case ET66FlatLabelRole::StatValue:
			return TEXT("StatValue");
		case ET66FlatLabelRole::Button:
			return TEXT("Button");
		default:
			return TEXT("Unknown");
		}
	}
}

FLinearColor FT66FlatStyle::Tokens::Bg(0.08f, 0.08f, 0.10f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Panel(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Panel2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Stroke(0.18f, 0.18f, 0.20f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Scrim(0.f, 0.f, 0.f, 0.70f);
FLinearColor FT66FlatStyle::Tokens::Text(244.f / 255.f, 236.f / 255.f, 216.f / 255.f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::TextMuted(0.75f, 0.75f, 0.75f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Accent(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Accent2(0.0f, 0.0f, 0.0f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Danger(0.95f, 0.15f, 0.15f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Success(0.20f, 0.80f, 0.35f, 1.0f);
FLinearColor FT66FlatStyle::Tokens::Border(1.0f, 1.0f, 1.0f, 1.0f);

const FMargin FT66FlatStyle::Tokens::ButtonPadding(12.f, 4.f);
const FMargin FT66FlatStyle::Tokens::ButtonPaddingPressed(12.f, 5.f, 12.f, 3.f);

FSlateFontInfo FT66FlatStyle::Tokens::FontRegular(const int32 Size)
{
	return FT66FlatStyle::MakeFont(Size);
}

FSlateFontInfo FT66FlatStyle::Tokens::FontBold(const int32 Size)
{
	return FT66FlatStyle::MakeBoldFont(Size);
}

FSlateFontInfo FT66FlatStyle::Tokens::FontTitle()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeBoldFont(56);
	Font.LetterSpacing = 180;
	return Font;
}

FSlateFontInfo FT66FlatStyle::Tokens::FontHeading()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeBoldFont(26);
	Font.LetterSpacing = 120;
	return Font;
}

FSlateFontInfo FT66FlatStyle::Tokens::FontBody()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeFont(15);
	Font.LetterSpacing = 20;
	return Font;
}

FSlateFontInfo FT66FlatStyle::Tokens::FontSmall()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeFont(12);
	Font.LetterSpacing = 18;
	return Font;
}

FSlateFontInfo FT66FlatStyle::Tokens::FontChip()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeBoldFont(12);
	Font.LetterSpacing = 90;
	return Font;
}

FSlateFontInfo FT66FlatStyle::Tokens::FontButton()
{
	FSlateFontInfo Font = FT66FlatStyle::MakeBoldFont(18);
	Font.LetterSpacing = 110;
	return Font;
}

FLinearColor FT66FlatStyle::BackgroundColor()
{
	return HexColor(8, 8, 12);
}

FLinearColor FT66FlatStyle::DisabledFill()
{
	return HexColor(20, 20, 28);
}

FLinearColor FT66FlatStyle::DisabledBorder()
{
	return HexColor(52, 52, 60);
}

FLinearColor FT66FlatStyle::DisabledText()
{
	return HexColor(90, 85, 105);
}

FLinearColor FT66FlatStyle::DefaultFill()
{
	return HexColor(23, 23, 30);
}

FLinearColor FT66FlatStyle::DefaultBorder()
{
	return HexColor(74, 74, 85);
}

FLinearColor FT66FlatStyle::DefaultText()
{
	return HexColor(220, 215, 235);
}

FLinearColor FT66FlatStyle::PurpleAccent()
{
	return HexColor(138, 138, 149);
}

FLinearColor FT66FlatStyle::SelectedFill()
{
	return HexColor(28, 14, 16);
}

FLinearColor FT66FlatStyle::SelectedBorder()
{
	return HexColor(225, 35, 45);
}

FLinearColor FT66FlatStyle::SelectedText()
{
	return HexColor(255, 80, 95);
}

FLinearColor FT66FlatStyle::ProgressFill()
{
	return SelectedBorder();
}

FLinearColor FT66FlatStyle::GoodStandingGreen()
{
	return HexColor(31, 179, 88);
}

FLinearColor FT66FlatStyle::ReadyBorder()
{
	return GoodStandingGreen();
}

FLinearColor FT66FlatStyle::HoverBorder()
{
	return HexColor(31, 179, 88);
}

FLinearColor FT66FlatStyle::HoverText()
{
	return HexColor(79, 208, 136);
}

FLinearColor FT66FlatStyle::HoverFill()
{
	return HexColor(14, 20, 14);
}

FLinearColor FT66FlatStyle::PrimaryText()
{
	return HexColor(240, 240, 245);
}

FLinearColor FT66FlatStyle::SecondaryText()
{
	return HexColor(167, 167, 176);
}

FLinearColor FT66FlatStyle::DataAccent()
{
	return HexColor(60, 220, 240);
}

FLinearColor FT66FlatStyle::Text()
{
	return FT66Style::Text();
}

FLinearColor FT66FlatStyle::TextMuted()
{
	return FT66Style::TextMuted();
}

FLinearColor FT66FlatStyle::Accent2()
{
	return FT66Style::Accent2();
}

FLinearColor FT66FlatStyle::Danger()
{
	return FT66Style::Danger();
}

FLinearColor FT66FlatStyle::Border()
{
	return FT66Style::Border();
}

FLinearColor FT66FlatStyle::Background()
{
	return FT66Style::Background();
}

FLinearColor FT66FlatStyle::PanelOuter()
{
	return FT66Style::PanelOuter();
}

FLinearColor FT66FlatStyle::PanelInner()
{
	return FT66Style::PanelInner();
}

FLinearColor FT66FlatStyle::BossBarBackground()
{
	return FT66Style::BossBarBackground();
}

FLinearColor FT66FlatStyle::BossBarFill()
{
	return FT66Style::BossBarFill();
}

FLinearColor FT66FlatStyle::MinimapBackground()
{
	return FT66Style::MinimapBackground();
}

FLinearColor FT66FlatStyle::MinimapTerrain()
{
	return FT66Style::MinimapTerrain();
}

FLinearColor FT66FlatStyle::MinimapGrid()
{
	return FT66Style::MinimapGrid();
}

FLinearColor FT66FlatStyle::MinimapFriendly()
{
	return FT66Style::MinimapFriendly();
}

FLinearColor FT66FlatStyle::MinimapEnemy()
{
	return FT66Style::MinimapEnemy();
}

FLinearColor FT66FlatStyle::MinimapNeutral()
{
	return FT66Style::MinimapNeutral();
}

float FT66FlatStyle::GetGlobalUIScale()
{
	return FT66Style::GetGlobalUIScale();
}

FVector2D FT66FlatStyle::GetViewportLogicalSize()
{
	return FT66Style::GetViewportLogicalSize();
}

FVector2D FT66FlatStyle::GetSafeFrameSize(const float AspectRatio)
{
	return FT66Style::GetSafeFrameSize(AspectRatio);
}

FMargin FT66FlatStyle::GetSafeFrameInsets(const float AspectRatio)
{
	return FT66Style::GetSafeFrameInsets(AspectRatio);
}

FMargin FT66FlatStyle::GetSafePadding(const FMargin& Padding, const float AspectRatio)
{
	return FT66Style::GetSafePadding(Padding, AspectRatio);
}

TSharedRef<SWidget> FT66FlatStyle::MakeResponsiveRoot(
	const TSharedRef<SWidget>& Content,
	const FVector2D& ReferenceResolution,
	const bool bAllowUpscale)
{
	return FT66Style::MakeResponsiveRoot(Content, ReferenceResolution, bAllowUpscale);
}

void FT66FlatStyle::DeferRebuild(UUserWidget* Widget, const int32 ZOrder)
{
	FT66Style::DeferRebuild(Widget, ZOrder);
}

TSharedRef<SWidget> FT66FlatStyle::MakeButton(const FT66ButtonParams& Params)
{
	if (UseFriendslopGlobalChrome())
	{
		// Legacy in-run button path: translate to a Friendslop plate button. Plates carry color,
		// so legacy color overrides are intentionally ignored.
		ET66FlatState State = ET66FlatState::Default;
		switch (Params.Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Danger:
		case ET66ButtonType::ToggleActive:
			State = ET66FlatState::Selected;
			break;
		case ET66ButtonType::Success:
			State = ET66FlatState::Ready;
			break;
		case ET66ButtonType::Neutral:
		case ET66ButtonType::Row:
		default:
			State = ET66FlatState::Default;
			break;
		}

		const TAttribute<FText> LabelAttribute = Params.DynamicLabel.IsBound()
			? Params.DynamicLabel
			: TAttribute<FText>(Params.Label);
		const FMargin ContentPadding = (Params.Padding.Left < 0.f)
			? FMargin(14.f, 8.f)
			: Params.Padding;

		const TSharedRef<SWidget> Button = MakeFlatButton(
			State,
			LabelAttribute,
			Params.OnClicked,
			nullptr,
			nullptr,
			ContentPadding,
			Params.MinWidth,
			Params.Height,
			Params.IsEnabled,
			Params.FontSize > 0 ? Params.FontSize : 16);

		return SNew(SBox)
			.Visibility(Params.Visibility)
			[
				Button
			];
	}
	return FT66Style::MakeButton(Params);
}

FT66ButtonParams FT66FlatStyle::MakeInRunButtonParams(
	const FText& Label,
	FOnClicked OnClicked,
	ET66ButtonType Type)
{
	return FT66Style::MakeInRunButtonParams(Label, MoveTemp(OnClicked), Type);
}

TSharedRef<SWidget> FT66FlatStyle::MakeBareButton(const FT66BareButtonParams& Params, TSharedPtr<SButton>* OutButton)
{
	return FT66Style::MakeBareButton(Params, OutButton);
}

TSharedRef<SWidget> FT66FlatStyle::MakePanel(
	const TSharedRef<SWidget>& Content,
	const FT66PanelParams& Params,
	TSharedPtr<SBorder>* OutBorder)
{
	return FT66Style::MakePanel(Content, Params, OutBorder);
}

TSharedRef<SWidget> FT66FlatStyle::MakePanel(
	const TSharedRef<SWidget>& Content,
	const ET66PanelType Type,
	const FMargin Padding)
{
	return FT66Style::MakePanel(Content, Type, Padding);
}

FSlateFontInfo FT66FlatStyle::MakeFont(const TCHAR* Weight, const int32 Size)
{
	return FT66Style::MakeFont(Weight, Size);
}

const FTextBlockStyle& FT66FlatStyle::GetTextBlockStyle(const FName StyleName)
{
	return FT66Style::Get().GetWidgetStyle<FTextBlockStyle>(StyleName);
}

const FSlateBrush* FT66FlatStyle::GetBrush(const FName BrushName)
{
	return FT66Style::Get().GetBrush(BrushName);
}

const FButtonStyle& FT66FlatStyle::GetButtonStyle(const FName StyleName)
{
	return FT66Style::Get().GetWidgetStyle<FButtonStyle>(StyleName);
}

const FComboButtonStyle& FT66FlatStyle::GetDropdownComboButtonStyle()
{
	return FT66Style::GetDropdownComboButtonStyle();
}

FLinearColor FT66FlatStyle::Scrim()
{
	return FT66Style::Scrim();
}

TSharedRef<SWidget> FT66FlatStyle::MakeHudPanel(
	const TSharedRef<SWidget>& Content,
	const FText& Title,
	const FMargin& Padding)
{
	if (UseFriendslopGlobalChrome())
	{
		const TSharedRef<SWidget> PanelContent = Title.IsEmpty()
			? Content
			: StaticCastSharedRef<SWidget>(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(MakeBoldFont(14))
					.ColorAndOpacity(FLinearColor(0.86f, 0.68f, 0.34f, 1.f))
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					Content
				]);
		return FT66FriendslopStyle::MakePanel(ET66FlatState::Default, Padding, PanelContent);
	}
	return FT66Style::MakeHudPanel(Content, Title, Padding);
}

TSharedRef<SWidget> FT66FlatStyle::MakeHudPanel(
	const TSharedRef<SWidget>& Content,
	const FMargin& Padding)
{
	if (UseFriendslopGlobalChrome())
	{
		return MakeHudPanel(Content, FText::GetEmpty(), Padding);
	}
	return FT66Style::MakeHudPanel(Content, Padding);
}

const FT66FlatStyle::FFrontendChromeMetrics& FT66FlatStyle::GetFrontendChromeMetrics()
{
	return T66ScreenSlateHelpers::GetFrontendChromeMetrics();
}

float FT66FlatStyle::GetFrontendChromeTopInset(const UT66UIManager* UIManager)
{
	return T66ScreenSlateHelpers::GetFrontendChromeTopInset(UIManager);
}

FT66FlatStyle::FTopBarScreenLayoutMetrics FT66FlatStyle::MakeTopBarScreenLayoutMetrics(
	const UT66UIManager* UIManager,
	const FMargin& ExtraPadding)
{
	return T66ScreenSlateHelpers::MakeTopBarScreenLayoutMetrics(UIManager, ExtraPadding);
}

TSharedRef<SWidget> FT66FlatStyle::MakeTopBarScreenRoot(
	const UT66UIManager* UIManager,
	const TSharedRef<SWidget>& Content,
	const TSharedRef<SWidget>& BackgroundContent,
	const FLinearColor& OverlayTint,
	const FMargin& ExtraPadding)
{
	return T66ScreenSlateHelpers::MakeTopBarScreenRoot(UIManager, Content, BackgroundContent, OverlayTint, ExtraPadding);
}

FSlateFontInfo FT66FlatStyle::MakeFrontendChromeTitleFont()
{
	return T66ScreenSlateHelpers::MakeFrontendChromeTitleFont();
}

FSlateFontInfo FT66FlatStyle::MakeFrontendChromeTabFont()
{
	return T66ScreenSlateHelpers::MakeFrontendChromeTabFont();
}

TSharedPtr<FSlateBrush> FT66FlatStyle::MakeSlateBrush(const FVector2D& ImageSize, const ESlateBrushDrawType::Type DrawAs)
{
	return T66ScreenSlateHelpers::MakeSlateBrush(ImageSize, DrawAs);
}

FT66FlatStyle::FResponsiveGridModalMetrics FT66FlatStyle::MakeResponsiveGridModalMetrics(
	const int32 ItemCount,
	const FVector2D& SafeFrameSize)
{
	return T66ScreenSlateHelpers::MakeResponsiveGridModalMetrics(ItemCount, SafeFrameSize);
}

void FT66FlatStyle::AddUniformGridPaddingSlots(
	SGridPanel& GridPanel,
	const int32 FilledSlotCount,
	const FResponsiveGridModalMetrics& Metrics)
{
	T66ScreenSlateHelpers::AddUniformGridPaddingSlots(GridPanel, FilledSlotCount, Metrics);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFilledButtonText(
	const FT66ButtonParams& Params,
	const float ButtonHeight,
	const TAttribute<FSlateColor>& DefaultTextColor,
	const TAttribute<FLinearColor>& DefaultShadowColor)
{
	return T66ScreenSlateHelpers::MakeFilledButtonText(Params, ButtonHeight, DefaultTextColor, DefaultShadowColor);
}

TSharedRef<SWidget> FT66FlatStyle::BuildFlatHorizontalSlicedImage(
	TAttribute<const FSlateBrush*> Brush,
	const FVector2D& DesiredSize,
	const float SourceCapFraction)
{
	return T66ScreenSlateHelpers::MakeReferenceHorizontalSlicedImage(Brush, DesiredSize, SourceCapFraction);
}

float FT66FlatStyle::NormalizeFlatSlicedButtonMinWidth(const float RequestedMinWidth, const float Height)
{
	return T66ScreenSlateHelpers::NormalizeReferenceSlicedButtonMinWidth(RequestedMinWidth, Height);
}

FT66FlatStyle::ET66ReferenceChromePreset FT66FlatStyle::GetFlatChromePreset()
{
	return T66ScreenSlateHelpers::GetReferenceChromePreset();
}

const TCHAR* FT66FlatStyle::GetFlatChromePresetName()
{
	return T66ScreenSlateHelpers::GetReferenceChromePresetName();
}

void FT66FlatStyle::SetFlatChromePresetForSession(const ET66ReferenceChromePreset Preset)
{
	T66ScreenSlateHelpers::SetReferenceChromePresetForSession(Preset);
}

FString FT66FlatStyle::GetFlatMainMenuElementAssetPath(const TCHAR* FileName)
{
	return T66ScreenSlateHelpers::MakeReferenceMainMenuElementAssetPath(FileName);
}

FString FT66FlatStyle::GetFlatChromeElementAssetPath(const TCHAR* FileName)
{
	return T66ScreenSlateHelpers::MakeReferenceChromeElementAssetPath(FileName);
}

FString FT66FlatStyle::GetFlatLongPanelAssetPath(const TCHAR* State)
{
	return T66ScreenSlateHelpers::MakeReferenceLongPanelAssetPath(State);
}

FString FT66FlatStyle::GetFlatRedSquareButtonAssetPath(const TCHAR* State)
{
	return T66ScreenSlateHelpers::MakeReferenceRedSquareButtonAssetPath(State);
}

FString FT66FlatStyle::GetFlatChromeButtonAssetPath(const TCHAR* Family, const TCHAR* State)
{
	return T66ScreenSlateHelpers::MakeReferenceChromeButtonAssetPath(Family, State);
}

FString FT66FlatStyle::GetFlatButtonAssetPath(const TCHAR* FamilyStem, const TCHAR* State)
{
	return T66ScreenSlateHelpers::MakeReferenceButtonAssetPath(FamilyStem, State);
}

FString FT66FlatStyle::GetFlatSharedAssetPath(const TCHAR* RelativeAssetPath)
{
	return T66ScreenSlateHelpers::MakeReferenceSharedAssetPath(RelativeAssetPath);
}

const FSlateBrush* FT66FlatStyle::GetFlatSharedBrush(
	const TCHAR* RelativeAssetPath,
	const FMargin& Margin,
	const TCHAR* DebugLabel)
{
	return T66ScreenSlateHelpers::GetReferenceSharedBrush(RelativeAssetPath, Margin, DebugLabel);
}

bool FT66FlatStyle::IsFlatChromeButtonAssetPath(const FString& SourceRelativePath)
{
	return T66ScreenSlateHelpers::IsReferenceChromeButtonAssetPath(SourceRelativePath);
}

bool FT66FlatStyle::IsFlatChromePillButtonAssetPath(const FString& SourceRelativePath)
{
	return T66ScreenSlateHelpers::IsReferenceChromePillButtonAssetPath(SourceRelativePath);
}

bool FT66FlatStyle::IsFlatChromeCTAButtonAssetPath(const FString& SourceRelativePath)
{
	return T66ScreenSlateHelpers::IsReferenceChromeCTAButtonAssetPath(SourceRelativePath);
}

TSharedRef<SWidget> FT66FlatStyle::BuildFlatSharedBorder(
	const TCHAR* RelativeAssetPath,
	const TSharedRef<SWidget>& Content,
	const FMargin& BrushMargin,
	const FMargin& Padding,
	const TCHAR* DebugLabel,
	const FLinearColor& FallbackColor)
{
	return T66ScreenSlateHelpers::MakeReferenceSharedBorder(
		RelativeAssetPath,
		Content,
		BrushMargin,
		Padding,
		DebugLabel,
		FallbackColor);
}

TSharedRef<SWidget> FT66FlatStyle::BuildFlatSlicedPlateButton(
	FOnClicked OnClicked,
	const TSharedRef<SWidget>& Content,
	const FSlateBrush* NormalBrush,
	const FSlateBrush* HoveredBrush,
	const FSlateBrush* PressedBrush,
	const FSlateBrush* DisabledBrush,
	const float MinWidth,
	const float Height,
	const FMargin& ContentPadding,
	const TAttribute<bool>& IsEnabled,
	const TAttribute<EVisibility>& Visibility,
	const float SourceCapFraction,
	const FSlateBrush* SelectedBrush,
	const TAttribute<bool>& IsSelected)
{
	return T66ScreenSlateHelpers::MakeReferenceSlicedPlateButton(
		MoveTemp(OnClicked),
		Content,
		NormalBrush,
		HoveredBrush,
		PressedBrush,
		DisabledBrush,
		MinWidth,
		Height,
		ContentPadding,
		IsEnabled,
		Visibility,
		SourceCapFraction,
		SelectedBrush,
		IsSelected);
}

TSharedRef<SWidget> FT66FlatStyle::BuildFlatProgressBar(
	TAttribute<TOptional<float>> Percent,
	const FVector2D& DesiredSize,
	const FLinearColor& FallbackFill,
	const FMargin& Padding)
{
	return T66ScreenSlateHelpers::MakeReferenceProgressBar(Percent, DesiredSize, FallbackFill, Padding);
}

TSharedRef<SWidget> FT66FlatStyle::BuildFlatProgressBar(
	const float Percent,
	const FVector2D& DesiredSize,
	const FLinearColor& FallbackFill,
	const FMargin& Padding)
{
	return T66ScreenSlateHelpers::MakeReferenceProgressBar(Percent, DesiredSize, FallbackFill, Padding);
}

TSharedRef<SWidget> FT66FlatStyle::MakeResponsiveGridTile(
	const FT66ButtonParams& ButtonParams,
	const FLinearColor& BackgroundColor,
	const TSharedRef<SWidget>& Content,
	const FResponsiveGridModalMetrics& Metrics)
{
	return T66ScreenSlateHelpers::MakeResponsiveGridTile(ButtonParams, BackgroundColor, Content, Metrics);
}

TSharedRef<SWidget> FT66FlatStyle::MakeResponsiveGridModal(
	const FText& TitleText,
	const TSharedRef<SWidget>& GridContent,
	const TSharedRef<SWidget>& FooterContent,
	const FResponsiveGridModalMetrics& Metrics)
{
	return T66ScreenSlateHelpers::MakeResponsiveGridModal(TitleText, GridContent, FooterContent, Metrics);
}

TSharedRef<SWidget> FT66FlatStyle::MakeCenteredScrimModal(
	const TSharedRef<SWidget>& Content,
	const FMargin& OuterPadding,
	const float WidthOverride,
	const float HeightOverride,
	const bool bUseWhiteBrush)
{
	return T66ScreenSlateHelpers::MakeCenteredScrimModal(Content, OuterPadding, WidthOverride, HeightOverride, bUseWhiteBrush);
}

TSharedRef<SWidget> FT66FlatStyle::MakeTwoButtonRow(
	const TSharedRef<SWidget>& LeftButton,
	const TSharedRef<SWidget>& RightButton,
	const FMargin& LeftPadding,
	const FMargin& RightPadding,
	const EVisibility Visibility)
{
	return T66ScreenSlateHelpers::MakeTwoButtonRow(LeftButton, RightButton, LeftPadding, RightPadding, Visibility);
}

FLinearColor FT66FlatStyle::FillForState(const ET66FlatState State)
{
	switch (State)
	{
	case ET66FlatState::Disabled:
		return DisabledFill();
	case ET66FlatState::Selected:
		return SelectedFill();
	case ET66FlatState::Ready:
		return DefaultFill();
	case ET66FlatState::Default:
	default:
		return DefaultFill();
	}
}

FLinearColor FT66FlatStyle::BorderForState(const ET66FlatState State)
{
	switch (State)
	{
	case ET66FlatState::Disabled:
		return DisabledBorder();
	case ET66FlatState::Selected:
		return SelectedBorder();
	case ET66FlatState::Ready:
		return ReadyBorder();
	case ET66FlatState::Default:
	default:
		return DefaultBorder();
	}
}

FSlateColor FT66FlatStyle::TextColorForState(const ET66FlatState State)
{
	switch (State)
	{
	case ET66FlatState::Disabled:
		return FSlateColor(DisabledText());
	case ET66FlatState::Selected:
		return FSlateColor(SelectedText());
	case ET66FlatState::Ready:
		return FSlateColor(GoodStandingGreen());
	case ET66FlatState::Default:
	default:
		return FSlateColor(DefaultText());
	}
}

FSlateFontInfo FT66FlatStyle::MakeFont(const int32 Size)
{
	return FlatFont(TEXT("Regular"), Size);
}

FSlateFontInfo FT66FlatStyle::MakeBoldFont(const int32 Size)
{
	return FlatFont(TEXT("Bold"), Size);
}

TSharedRef<SWidget> FT66FlatStyle::AttachMetadata(
	const TSharedRef<SWidget>& Widget,
	const FName Tag,
	const FString& IntendedRole,
	const ET66FlatState IntendedState,
	const TOptional<FLinearColor>& BorderColor,
	const bool bHasClickHandler,
	const FName ToggleGroup,
	const bool bIsLabel,
	const bool bHoverCapable)
{
	if (Tag.IsNone())
	{
		return Widget;
	}

	Widget->SetTag(Tag);
	Widget->AddMetadata(MakeShared<FT66FlatWidgetMetadata>(
		Tag,
		IntendedRole,
		IntendedState,
		BorderColor,
		bHasClickHandler,
		ToggleGroup,
		bIsLabel,
		bHoverCapable));
	return Widget;
}

TSharedRef<SWidget> FT66FlatStyle::WrapWithoutRetainer(const TSharedRef<SWidget>& Widget, const FName Tag)
{
	return AttachMetadata(Widget, Tag, TEXT("Wrapper"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatPanel(
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag)
{
	TSharedPtr<SBorder> PanelBorder;
	TSharedRef<SWidget> Surface = MakeFlatPanelSurface(State, Padding, Content, &PanelBorder);
	if (OutBorder)
	{
		*OutBorder = PanelBorder;
	}
	if (!Tag.IsNone() && PanelBorder.IsValid())
	{
		AttachMetadata(PanelBorder.ToSharedRef(), Tag, TEXT("Panel"), State);
	}
	return Surface;
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatInteractivePanel(
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TAttribute<bool> IsEnabled,
	const FName Tag,
	const FString& IntendedRole)
{
	const TSharedPtr<TWeakPtr<SWidget>> HoverProbe = MakeHoverProbe();
	TSharedRef<SWidget> Surface = MakeFlatPanelSurface(State, Padding, Content, nullptr, nullptr, IsEnabled, HoverProbe);
	TSharedRef<SWidget> Wrapper = SNew(SBox)
		[
			Surface
		];
	BindHoverProbe(HoverProbe, Wrapper);
	const bool bHoverCapable = State != ET66FlatState::Disabled && IsEnabled.Get(true);
	return AttachMetadata(Wrapper, Tag, IntendedRole, State, TOptional<FLinearColor>(), true, NAME_None, false, bHoverCapable);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatSubPanel(
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	TSharedPtr<SBorder>* OutBorder,
	const FName Tag)
{
	return MakeFlatPanel(State, Padding, Content, OutBorder, Tag);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTransparentRegion(
	const FT66FlatTransparentRegionParams& Params,
	const TSharedRef<SWidget>& Content)
{
	const FLinearColor StrokeColor = BorderForState(Params.State);
	TSharedRef<SOverlay> Region = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FlatWhiteBrush())
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(Params.Padding)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				Content
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			SNew(SBox)
			.HeightOverride(FlatStroke)
			[
				SNew(SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(StrokeColor)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		[
			SNew(SBox)
			.HeightOverride(FlatStroke)
			[
				SNew(SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(StrokeColor)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(FlatStroke)
			[
				SNew(SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(StrokeColor)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(FlatStroke)
			[
				SNew(SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(StrokeColor)
			]
		];

	return AttachMetadata(WrapWithoutRetainer(Region), Params.Tag, TEXT("TransparentRegion"), Params.State, StrokeColor);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTransparentRegion(
	const ET66FlatState State,
	const FMargin& Padding,
	const TSharedRef<SWidget>& Content,
	const FName Tag)
{
	return MakeFlatTransparentRegion(FT66FlatTransparentRegionParams(State, Padding, Tag), Content);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatOuterContainer(
	const ET66FlatState State,
	const TArray<TSharedRef<SWidget>>& Children,
	const float Gap,
	const EOrientation Orientation,
	const FMargin& Padding,
	const FName Tag)
{
	if (Orientation == Orient_Horizontal)
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		for (int32 Index = 0; Index < Children.Num(); ++Index)
		{
			Row->AddSlot()
				.AutoWidth()
				.Padding(Index > 0 ? FMargin(Gap, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					Children[Index]
				];
		}
		return AttachMetadata(MakeFlatPanel(State, Padding, Row), Tag, TEXT("OuterContainer"), State);
	}

	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);
	for (int32 Index = 0; Index < Children.Num(); ++Index)
	{
		Column->AddSlot()
			.AutoHeight()
			.Padding(Index > 0 ? FMargin(0.f, Gap, 0.f, 0.f) : FMargin(0.f))
			[
				Children[Index]
			];
	}
	return AttachMetadata(MakeFlatPanel(State, Padding, Column), Tag, TEXT("OuterContainer"), State);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatHeaderedPanel(
	const ET66FlatState State,
	const TAttribute<FText>& HeaderText,
	const TSharedRef<SWidget>& BodyContent,
	const TSharedPtr<SWidget>& OptionalIcon,
	const TOptional<FLinearColor>& OptionalHeaderAccent,
	const FName Tag)
{
	const FSlateColor HeaderColor(OptionalHeaderAccent.Get(PrimaryText()));
	TSharedRef<SHorizontalBox> HeaderRow = SNew(SHorizontalBox);
	if (OptionalIcon.IsValid())
	{
		HeaderRow->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				OptionalIcon.ToSharedRef()
			];
	}

	HeaderRow->AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.Font(MakeBoldFont(FontSizeForRole(ET66FlatLabelRole::Header)))
			.ColorAndOpacity(HeaderColor)
		];

	return AttachMetadata(MakeFlatPanel(
		State,
		FMargin(16.f, 14.f),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			HeaderRow
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 10.f)
		[
			MakeFlatDivider(Orient_Horizontal)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			BodyContent
		]), Tag, TEXT("HeaderedPanel"), State);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatHeaderedPanel(
	const ET66FlatState State,
	const FText& HeaderText,
	const TSharedRef<SWidget>& BodyContent,
	const TSharedPtr<SWidget>& OptionalIcon,
	const TOptional<FLinearColor>& OptionalHeaderAccent,
	const FName Tag)
{
	return MakeFlatHeaderedPanel(State, TAttribute<FText>(HeaderText), BodyContent, OptionalIcon, OptionalHeaderAccent, Tag);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatButton(const FT66FlatButtonParams& Params)
{
	return MakeFlatButton(
		Params.State,
		Params.Label,
		Params.OnClicked,
		Params.OptionalLeftIcon,
		Params.OptionalRightIcon,
		Params.Padding,
		Params.MinWidth,
		Params.Height,
		Params.IsEnabled,
		Params.FontSize,
		Params.Tag,
		Params.ToggleGroup);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatButton(
	const ET66FlatState State,
	const TAttribute<FText>& Label,
	FOnClicked OnClicked,
	const TSharedPtr<SWidget>& OptionalLeftIcon,
	const TSharedPtr<SWidget>& OptionalRightIcon,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const int32 FontSize,
	const FName Tag,
	const FName ToggleGroup)
{
	const bool bHasClickHandler = OnClicked.IsBound();
	const bool bHoverCapable = bHasClickHandler && State != ET66FlatState::Disabled && IsEnabled.Get(true);
	const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	if (OptionalLeftIcon.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				OptionalLeftIcon.ToSharedRef()
			];
	}

	Row->AddSlot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(MakeBoldFont(FontSize))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe))
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];

	if (OptionalRightIcon.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				OptionalRightIcon.ToSharedRef()
			];
	}

	const TSharedRef<SWidget> ButtonSurface = MakeFlatPanelSurface(State, Padding, Row, nullptr, nullptr, IsEnabled, ButtonHoverProbe);
	TSharedRef<SWidget> Button = FT66Style::MakeBareButton(
		FT66BareButtonParams(MoveTemp(OnClicked), WrapInOptionalBox(ButtonSurface, MinWidth, Height))
			.SetButtonStyle(&FlatNoBorderButtonStyle())
			.SetPadding(FMargin(0.f))
			.SetEnabled(IsEnabled)
			.SetMinWidth(MinWidth)
			.SetHeight(Height));
	BindHoverProbe(ButtonHoverProbe, Button);
	return AttachMetadata(Button, Tag, TEXT("Button"), State, TOptional<FLinearColor>(), bHasClickHandler, ToggleGroup, false, bHoverCapable);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatButton(
	const ET66FlatState State,
	const FText& Label,
	FOnClicked OnClicked,
	const TSharedPtr<SWidget>& OptionalLeftIcon,
	const TSharedPtr<SWidget>& OptionalRightIcon,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const int32 FontSize,
	const FName Tag,
	const FName ToggleGroup)
{
	return MakeFlatButton(State, TAttribute<FText>(Label), MoveTemp(OnClicked), OptionalLeftIcon, OptionalRightIcon, Padding, MinWidth, Height, IsEnabled, FontSize, Tag, ToggleGroup);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatUnavailablePanel(
	const FText& Text,
	const FName Tag,
	const float MaxWidth,
	const int32 FontSize)
{
	const TSharedRef<SWidget> TextContent =
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.MaxDesiredWidth(MaxWidth > 0.f ? MaxWidth : 220.f)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(MakeBoldFont(FontSize > 0 ? FontSize : 20))
					.ColorAndOpacity(FSlateColor(PrimaryText()))
					.Justification(ETextJustify::Center)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
			]
		];

	return AttachMetadata(
		MakeFlatPanel(ET66FlatState::Selected, FMargin(8.f, 3.f), TextContent),
		Tag,
		TEXT("UnavailablePanel"),
		ET66FlatState::Disabled);
}

TSharedRef<SWidget> FT66FlatStyle::WrapWithFlatUnavailableOverlay(
	const TSharedRef<SWidget>& Content,
	const bool bShowOverlay,
	const FText& OverlayText,
	const FName Tag,
	const bool bBlockInteraction)
{
	if (!bShowOverlay)
	{
		return Content;
	}

	const TSharedRef<SWidget> ProtectedContent = bBlockInteraction
		? StaticCastSharedRef<SWidget>(SNew(SBox).IsEnabled(false)[Content])
		: Content;

	return AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			ProtectedContent
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.Visibility(bBlockInteraction ? EVisibility::Visible : EVisibility::HitTestInvisible)
			[
				MakeFlatUnavailablePanel(
					OverlayText,
					Tag.IsNone() ? NAME_None : FName(*(Tag.ToString() + TEXT(".Panel"))))
			]
		],
		Tag,
		TEXT("UnavailableOverlay"),
		ET66FlatState::Disabled);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatToggleGroupButton(
	const ET66FlatState State,
	const TSharedRef<SWidget>& Content,
	FOnClicked OnClicked,
	const FMargin& Padding,
	const float MinWidth,
	const float Height,
	const TAttribute<bool> IsEnabled,
	const FName Tag,
	const FName ToggleGroup)
{
	const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
	return MakeFlatInteractiveBareButton(
		State,
		Content,
		MoveTemp(OnClicked),
		Padding,
		MinWidth,
		Height,
		IsEnabled,
		Tag,
		ToggleGroup,
		TEXT("ToggleButton"),
		ButtonHoverProbe);
}

const FSlateBrush* FT66FlatStyle::GetFlatOverlayBrush(const ET66FlatOverlayChromeBrush)
{
	return FlatWhiteBrush();
}

FT66FlatOverlayChromeButtonParams FT66FlatStyle::MakeFlatOverlayButtonParams(
	const FText& Label,
	FOnClicked OnClicked,
	const ET66FlatOverlayChromeButtonFamily Family)
{
	return FT66FlatOverlayChromeButtonParams(Label, MoveTemp(OnClicked), Family);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatOverlayPanel(
	const TSharedRef<SWidget>& Content,
	const ET66FlatOverlayChromeBrush Brush,
	const FMargin& Padding,
	TSharedPtr<SBorder>* OutBorder)
{
	return MakeFlatPanel(FlatStateForOverlayBrush(Brush), Padding, Content, OutBorder);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatOverlayButton(const FT66FlatOverlayChromeButtonParams& Params)
{
	const ET66FlatState State = FlatStateForOverlayButton(
		Params.Family,
		Params.IsSelected.Get(false),
		Params.IsEnabled.Get(true));

	if (Params.CustomContent.IsValid())
	{
		return MakeFlatToggleGroupButton(
			State,
			Params.CustomContent.ToSharedRef(),
			Params.OnClicked,
			Params.Padding,
			Params.MinWidth,
			Params.MinHeight,
			Params.IsEnabled);
	}

	return MakeFlatButton(
		State,
		Params.Label,
		Params.OnClicked,
		nullptr,
		nullptr,
		Params.Padding,
		Params.MinWidth,
		Params.MinHeight,
		Params.IsEnabled,
		Params.FontSize);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatOverlaySlotPanel(
	const TSharedRef<SWidget>& Content,
	const TAttribute<bool>& IsSelected,
	const TAttribute<bool>& IsEnabled,
	const FMargin& Padding)
{
	const ET66FlatState State = !IsEnabled.Get(true)
		? ET66FlatState::Disabled
		: (IsSelected.Get(false) ? ET66FlatState::Selected : ET66FlatState::Default);

	return MakeFlatPanel(State, Padding, Content);
}

TArray<TSharedRef<SWidget>> FT66FlatStyle::MakeFlatToggleGroup(const FT66FlatToggleGroupParams& Params)
{
	TArray<TSharedRef<SWidget>> Result;
	Result.Reserve(Params.Items.Num());

	int32 InitialSelectedIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Params.Items.Num(); ++Index)
	{
		if (Params.Items[Index].bIsSelected.Get(false))
		{
			InitialSelectedIndex = Index;
			break;
		}
	}
	const TSharedRef<int32> SelectedIndex = MakeShared<int32>(InitialSelectedIndex);

	for (int32 Index = 0; Index < Params.Items.Num(); ++Index)
	{
		const FT66FlatToggleGroupItem& Item = Params.Items[Index];
		const ET66FlatState RenderState = Item.bIsSelected.Get(false) ? ET66FlatState::Selected : Item.State;
		const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
		if (Item.OptionalLeftIcon.IsValid())
		{
			Row->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 8.f, 0.f)
				[
					Item.OptionalLeftIcon.ToSharedRef()
				];
		}

		Row->AddSlot()
			.FillWidth(1.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Item.Label)
				.Font(MakeBoldFont(Item.FontSize))
				.ColorAndOpacity(MakeInteractiveTextColorAttribute(RenderState, Item.IsEnabled, ButtonHoverProbe))
				.Justification(ETextJustify::Center)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			];

		if (Item.OptionalRightIcon.IsValid())
		{
			Row->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					Item.OptionalRightIcon.ToSharedRef()
				];
		}

		FOnClicked WrappedOnClicked = Item.OnClicked;
		if (Params.bMutuallyExclusive)
		{
			const FOnClicked UserOnClicked = Item.OnClicked;
			WrappedOnClicked = FOnClicked::CreateLambda([SelectedIndex, Index, UserOnClicked]()
			{
				*SelectedIndex = Index;
				return UserOnClicked.IsBound() ? UserOnClicked.Execute() : FReply::Handled();
			});
		}

		Result.Add(MakeFlatInteractiveBareButton(
			RenderState,
			Row,
			WrappedOnClicked,
			Item.Padding,
			Item.MinWidth,
			Item.Height,
			Item.IsEnabled,
			Item.Tag,
			Params.bMutuallyExclusive ? Params.GroupName : NAME_None,
			TEXT("ToggleButton"),
			ButtonHoverProbe));
	}

	return Result;
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatIconButton(
	const ET66FlatState State,
	const FSlateBrush* Icon,
	FOnClicked OnClicked,
	const FVector2D& SizeHint,
	const FName Tag)
{
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
	TSharedRef<SWidget> IconWidget = Icon
		? StaticCastSharedRef<SWidget>(
			SNew(SImage)
			.Image(Icon)
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe)))
		: StaticCastSharedRef<SWidget>(
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("?")))
			.Font(MakeBoldFont(18))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe)));

	return MakeFlatInteractiveBareButton(
		State,
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			IconWidget
		],
		MoveTemp(OnClicked),
		FMargin(8.f),
		SizeHint.X,
		SizeHint.Y,
		IsEnabled,
		Tag,
		NAME_None,
		TEXT("IconButton"),
		ButtonHoverProbe);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTooltipContent(
	const FText& Text,
	const float Width,
	const FName Tag)
{
	const TSharedRef<SWidget> Body = SNew(SBox)
		.WidthOverride(Width)
		[
			SNew(STextBlock)
			.Visibility(EVisibility::HitTestInvisible)
			.Text(Text)
			.Font(MakeFont(16))
			.ColorAndOpacity(PrimaryText())
			.AutoWrapText(true)
			.WrapTextAt(Width - 24.f)
			.Justification(ETextJustify::Left)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];

	return AttachMetadata(
		MakeFlatPanel(ET66FlatState::Default, FMargin(12.f, 9.f), Body, nullptr, NAME_None),
		Tag,
		TEXT("Tooltip"),
		ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTooltipIcon(
	const ET66FlatState State,
	const FSlateBrush* Icon,
	const FText& TooltipText,
	const FVector2D& SizeHint,
	const FName Tag,
	FOnClicked OnClicked)
{
	if (!OnClicked.IsBound())
	{
		OnClicked = FOnClicked::CreateLambda([]()
		{
			return FReply::Handled();
		});
	}

	TSharedRef<SWidget> IconButton = MakeFlatIconButton(
		State,
		Icon,
		MoveTemp(OnClicked),
		SizeHint,
		Tag);

	const FName TooltipId = Tag.IsNone() ? FName(TEXT("Flat.TooltipIcon.Tooltip")) : FName(*(Tag.ToString() + TEXT(".Tooltip")));
	FT66TooltipPayload TooltipPayload = T66TooltipResolvers::MakeRichTooltip(
		TooltipId,
		ET66TooltipKind::Action,
		FText::GetEmpty(),
		TooltipText,
		Tag);
	T66TooltipSlate::SetTooltip(IconButton, TooltipPayload, true);

	return IconButton;
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTabButton(
	const ET66FlatState State,
	const TAttribute<FText>& Label,
	FOnClicked OnClicked,
	const TSharedPtr<SWidget>& OptionalLeftIcon,
	const FText& TooltipText,
	const float MinWidth,
	const float Height,
	const FName Tag)
{
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	if (OptionalLeftIcon.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				OptionalLeftIcon.ToSharedRef()
			];
	}

	Row->AddSlot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(MakeBoldFont(20))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe))
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		];

	Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("i")))
			.Font(MakeBoldFont(18))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe))
			.ToolTipText(TooltipText)
		];

	return MakeFlatInteractiveBareButton(
		State,
		Row,
		MoveTemp(OnClicked),
		FMargin(14.f, 8.f),
		MinWidth,
		Height,
		IsEnabled,
		Tag,
		NAME_None,
		TEXT("TabButton"),
		ButtonHoverProbe);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatActionRow(
	const TArray<TSharedRef<SWidget>>& Buttons,
	const ET66FlatActionAlignment Alignment,
	const float Gap,
	const FName Tag)
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < Buttons.Num(); ++Index)
	{
		if (Alignment == ET66FlatActionAlignment::Fill)
		{
			Row->AddSlot()
				.FillWidth(1.f)
				.Padding(Index > 0 ? FMargin(Gap, 0.f, 0.f, 0.f) : FMargin(0.f))
				.VAlign(VAlign_Fill)
				[
					Buttons[Index]
				];
		}
		else
		{
			Row->AddSlot()
				.AutoWidth()
				.Padding(Index > 0 ? FMargin(Gap, 0.f, 0.f, 0.f) : FMargin(0.f))
				.VAlign(VAlign_Fill)
				[
					Buttons[Index]
				];
		}
	}
	return AttachMetadata(Row, Tag, TEXT("ActionRow"), ET66FlatState::Default);
}

float FT66FlatStyle::GetActiveDropdownMenuWidth(const float Fallback)
{
	if (const FFlatDropdownMenuContext* Context = GetActiveFlatDropdownMenuContext())
	{
		return Context->Width > 0.f ? Context->Width : Fallback;
	}
	return Fallback;
}

float FT66FlatStyle::GetActiveDropdownMenuHeight(const float Fallback)
{
	if (const FFlatDropdownMenuContext* Context = GetActiveFlatDropdownMenuContext())
	{
		return Context->Height > 0.f ? Context->Height : Fallback;
	}
	return Fallback;
}

int32 FT66FlatStyle::GetActiveDropdownMenuFontSize(const int32 Fallback)
{
	if (const FFlatDropdownMenuContext* Context = GetActiveFlatDropdownMenuContext())
	{
		return Context->FontSize > 0 ? Context->FontSize : Fallback;
	}
	return Fallback;
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatDropdownMenuPanel(
	const TSharedRef<SWidget>& Options,
	const float MinWidth)
{
	const float ResolvedWidth = MinWidth > 0.f
		? MinWidth
		: GetActiveDropdownMenuWidth(0.f);
	TSharedRef<SWidget> MenuContent = ResolvedWidth > 0.f
		? StaticCastSharedRef<SWidget>(
			SNew(SBox)
			.WidthOverride(ResolvedWidth)
			[
				Options
			])
		: Options;

	return MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), MenuContent);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatDropdownOptionButton(
	const ET66FlatState State,
	const FText& Label,
	FOnClicked OnClicked,
	const float MinWidth,
	const float Height,
	const int32 FontSize,
	const FName Tag,
	const FName ToggleGroup)
{
	const float ResolvedWidth = MinWidth > 0.f ? MinWidth : GetActiveDropdownMenuWidth(0.f);
	const float ResolvedHeight = Height > 0.f ? Height : GetActiveDropdownMenuHeight(42.f);
	const int32 ResolvedFontSize = FontSize > 0 ? FontSize : GetActiveDropdownMenuFontSize(20);
	const bool bHasClickHandler = OnClicked.IsBound();
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> ButtonHoverProbe = MakeHoverProbe();
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(MakeBoldFont(ResolvedFontSize))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, ButtonHoverProbe))
			.Justification(ETextJustify::Center)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
		];

	const TSharedRef<SWidget> ButtonSurface = MakeFlatPanelSurface(State, FMargin(10.f, 4.f), Row, nullptr, nullptr, IsEnabled, ButtonHoverProbe);
	FT66BareButtonParams ButtonParams(MoveTemp(OnClicked), ButtonSurface);
	ButtonParams
		.SetButtonStyle(&FlatNoBorderButtonStyle())
		.SetPadding(FMargin(0.f))
		.SetEnabled(IsEnabled)
		.SetClickMethod(EButtonClickMethod::MouseDown)
		.SetDebounceClick(false);
	if (ResolvedWidth > 0.f)
	{
		ButtonParams.SetWidth(ResolvedWidth);
	}
	if (ResolvedHeight > 0.f)
	{
		ButtonParams.SetHeight(ResolvedHeight);
	}

	TSharedRef<SWidget> Button = FT66Style::MakeBareButton(ButtonParams);
	BindHoverProbe(ButtonHoverProbe, Button);
	Button = AttachMetadata(
		Button,
		Tag,
		TEXT("DropdownOption"),
		State,
		TOptional<FLinearColor>(),
		bHasClickHandler,
		ToggleGroup,
		false,
		bHasClickHandler && State != ET66FlatState::Disabled);

	if (ResolvedWidth > 0.f)
	{
		return SNew(SBox)
			.WidthOverride(ResolvedWidth)
			[
				Button
			];
	}
	return Button;
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatDropdownOptionsMenu(
	const TArray<FT66FlatDropdownOptionData>& Options,
	const float MinWidth,
	const float Height,
	const int32 FontSize,
	const FName Tag)
{
	TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
	for (const FT66FlatDropdownOptionData& Option : Options)
	{
		const float OptionWidth = Option.MinWidth > 0.f ? Option.MinWidth : MinWidth;
		const float OptionHeight = Option.Height > 0.f ? Option.Height : Height;
		const int32 OptionFontSize = Option.FontSize > 0 ? Option.FontSize : FontSize;
		const bool bEnabled = Option.bEnabled && Option.State != ET66FlatState::Disabled;
		const ET66FlatState OptionState = bEnabled ? Option.State : ET66FlatState::Disabled;
		FOnClicked OptionClicked = bEnabled ? Option.OnClicked : FOnClicked();

		TSharedRef<SWidget> OptionWidget = MakeFlatDropdownOptionButton(
			OptionState,
			Option.Label,
			MoveTemp(OptionClicked),
			OptionWidth,
			OptionHeight,
			OptionFontSize,
			Option.Tag,
			Option.ToggleGroup);

		if (Option.bShowUnavailableOverlay)
		{
			OptionWidget = WrapWithFlatUnavailableOverlay(
				OptionWidget,
				true,
				Option.UnavailableText.IsEmpty()
					? NSLOCTEXT("T66.FlatStyle", "UnavailableFallback", "COMING SOON")
					: Option.UnavailableText,
				Option.OverlayTag,
				true);
		}

		Menu->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				OptionWidget
			];
	}

	return AttachMetadata(Menu, Tag, TEXT("DropdownOptionsMenu"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatDropdown(
	const ET66FlatState State,
	const TAttribute<FText>& CurrentValueText,
	TFunction<TSharedRef<SWidget>()> OptionsProvider,
	const bool bForceSelectedState,
	const float MinWidth,
	const float Height,
	const int32 FontSize,
	const FName Tag)
{
	const ET66FlatState RenderState = bForceSelectedState ? ET66FlatState::Selected : State;
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> DropdownHoverProbe = MakeHoverProbe();
	TSharedRef<SHorizontalBox> ButtonRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(CurrentValueText)
			.Font(MakeBoldFont(FontSize))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(RenderState, IsEnabled, DropdownHoverProbe))
			.Justification(ETextJustify::Left)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("v")))
			.Font(MakeBoldFont(FMath::Max(12, FontSize - 4)))
			.ColorAndOpacity(MakeInteractiveTextColorAttribute(RenderState, IsEnabled, DropdownHoverProbe))
		];

	TSharedRef<SWidget> ButtonSurface = MakeFlatPanelSurface(
		RenderState,
		FMargin(12.f, 7.f),
		ButtonRow,
		nullptr,
		nullptr,
		IsEnabled,
		DropdownHoverProbe);

	TSharedPtr<SComboButton> Combo;
	SAssignNew(Combo, SComboButton)
		.ComboButtonStyle(&FlatComboButtonStyle())
		.MenuPlacement(MenuPlacement_BelowAnchor)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f))
		.IsEnabled(IsEnabled)
		.OnGetMenuContent_Lambda([OptionsProvider = MoveTemp(OptionsProvider), MinWidth, Height, FontSize]()
		{
			FScopedFlatDropdownMenuContext Context(MinWidth, Height, FontSize);
			return FT66FlatStyle::MakeFlatDropdownMenuPanel(OptionsProvider(), MinWidth);
		})
		.ButtonContent()
		[
			ButtonSurface
		];
	BindHoverProbe(DropdownHoverProbe, Combo.ToSharedRef());

	return AttachMetadata(WrapInOptionalBox(
		Combo.ToSharedRef(),
		MinWidth,
		Height), Tag, TEXT("Dropdown"), RenderState, TOptional<FLinearColor>(), true, NAME_None, false, RenderState != ET66FlatState::Disabled);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatSlider(
	const ET66FlatState State,
	const float Min,
	const float Max,
	TAttribute<float> Current,
	FOnFloatValueChanged OnChange,
	const TSharedPtr<SWidget>& OptionalValueDisplay,
	const FName Tag)
{
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> SliderHoverProbe = MakeHoverProbe();
	TSharedPtr<SHorizontalBox> Row;
	SAssignNew(Row, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(SSlider)
			.MinValue(Min)
			.MaxValue(Max)
			.Value(Current)
			.IsEnabled(IsEnabled)
			.OnValueChanged(OnChange)
		];
	BindHoverProbe(SliderHoverProbe, Row.ToSharedRef());

	if (OptionalValueDisplay.IsValid())
	{
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(12.f, 0.f, 0.f, 0.f)
			[
				OptionalValueDisplay.ToSharedRef()
			];
	}

	return AttachMetadata(MakeFlatPanelSurface(State, FMargin(12.f, 8.f), Row.ToSharedRef(), nullptr, nullptr, IsEnabled, SliderHoverProbe), Tag, TEXT("Slider"), State, TOptional<FLinearColor>(), OnChange.IsBound(), NAME_None, false, OnChange.IsBound() && State != ET66FlatState::Disabled);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatCheckbox(
	const ET66FlatState State,
	TAttribute<ECheckBoxState> Checked,
	FOnCheckStateChanged OnToggle,
	const TAttribute<FText>& OptionalLabel,
	const FName Tag)
{
	const TAttribute<bool> IsEnabled(State != ET66FlatState::Disabled);
	const TSharedPtr<TWeakPtr<SWidget>> CheckboxHoverProbe = MakeHoverProbe();
	const FName SquareTag = Tag.IsNone() ? NAME_None : FName(*(Tag.ToString() + TEXT(".Square")));
	auto IsChecked = [Checked]() { return Checked.Get() == ECheckBoxState::Checked; };
	const TSharedRef<SWidget> Square = AttachMetadata(
		SNew(SBox)
		.WidthOverride(20.f)
		.HeightOverride(20.f)
		[
			SNew(SBorder)
			.BorderImage(FlatWhiteBrush())
			.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([IsChecked]() { return FSlateColor(IsChecked() ? SelectedBorder() : DefaultBorder()); }))
			.Padding(2.f)
			[
				SNew(SBorder)
				.BorderImage(FlatWhiteBrush())
				.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([IsChecked]() { return FSlateColor(IsChecked() ? SelectedBorder() : PanelInner()); }))
			]
		],
		SquareTag,
		TEXT("CheckboxSquare"),
		State,
		TOptional<FLinearColor>(),
		false,
		NAME_None,
		false,
		false);

	TSharedPtr<SHorizontalBox> Row;
	SAssignNew(Row, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			Square
		];

	if (OptionalLabel.IsSet())
	{
		Row->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(OptionalLabel)
				.Font(MakeFont(FontSizeForRole(ET66FlatLabelRole::Body)))
				.ColorAndOpacity(MakeInteractiveTextColorAttribute(State, IsEnabled, CheckboxHoverProbe))
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			];
	}

	TSharedRef<SWidget> Content = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush(TEXT("NoBrush")))
		.Padding(OptionalLabel.IsSet() ? FMargin(12.f, 8.f) : FMargin(0.f))
		[
			Row.ToSharedRef()
		];

	FOnClicked ToggleClicked = FOnClicked::CreateLambda([Checked, OnToggle]() mutable
	{
		if (OnToggle.IsBound())
		{
			OnToggle.Execute(Checked.Get() == ECheckBoxState::Checked ? ECheckBoxState::Unchecked : ECheckBoxState::Checked);
		}
		return FReply::Handled();
	});

	FT66BareButtonParams ButtonParams(ToggleClicked, Content);
	ButtonParams
		.SetButtonStyle(&FlatNoBorderButtonStyle())
		.SetPadding(FMargin(0.f))
		.SetEnabled(IsEnabled)
		.SetClickMethod(EButtonClickMethod::MouseDown)
		.SetDebounceClick(false);

	TSharedRef<SWidget> Button = FT66Style::MakeBareButton(ButtonParams);
	BindHoverProbe(CheckboxHoverProbe, Button);

	return AttachMetadata(Button, Tag, TEXT("CheckboxButton"), State, TOptional<FLinearColor>(), OnToggle.IsBound(), NAME_None, false, OnToggle.IsBound() && State != ET66FlatState::Disabled);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatToggleButton(
	const ET66FlatState State,
	const TAttribute<FText>& Label,
	TAttribute<bool> bIsActive,
	FOnClicked OnToggle,
	const float MinWidth,
	const float Height,
	const FName Tag)
{
	const ET66FlatState RenderState = bIsActive.Get(false) ? ET66FlatState::Selected : State;
	return MakeFlatButton(RenderState, Label, MoveTemp(OnToggle), nullptr, nullptr, FMargin(14.f, 8.f), MinWidth, Height, State != ET66FlatState::Disabled, 20, Tag);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatLabel(
	const TAttribute<FText>& Text,
	const ET66FlatLabelRole Role,
	const ETextJustify::Type Justification,
	const FName Tag)
{
	const bool bBold = Role == ET66FlatLabelRole::Title
		|| Role == ET66FlatLabelRole::Header
		|| Role == ET66FlatLabelRole::SubHeader
		|| Role == ET66FlatLabelRole::Button
		|| Role == ET66FlatLabelRole::StatValue;

	TSharedRef<STextBlock> Label = SNew(STextBlock)
		.Text(Text)
		.Font(bBold ? MakeBoldFont(FontSizeForRole(Role)) : MakeFont(FontSizeForRole(Role)))
		.ColorAndOpacity(TextColorForRole(Role))
		.Justification(Justification)
		.OverflowPolicy(ETextOverflowPolicy::Ellipsis);
	return AttachMetadata(Label, Tag, FString::Printf(TEXT("Label.%s"), *LabelRoleName(Role)), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatLabel(
	const FText& Text,
	const ET66FlatLabelRole Role,
	const ETextJustify::Type Justification,
	const FName Tag)
{
	return MakeFlatLabel(TAttribute<FText>(Text), Role, Justification, Tag);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatStatRow(
	const TAttribute<FText>& LabelText,
	const TAttribute<FText>& ValueText,
	const ET66FlatLabelRole LabelRole,
	const ET66FlatLabelRole ValueRole,
	const FName Tag)
{
	TSharedRef<SWidget> Row = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				MakeFlatLabel(LabelText, LabelRole)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				MakeFlatLabel(ValueText, ValueRole, ETextJustify::Right)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 6.f, 0.f, 0.f)
		[
			MakeFlatDivider(Orient_Horizontal)
		];
	return AttachMetadata(Row, Tag, TEXT("StatRow"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatStatsTable(
	const TArray<FT66FlatStatRowData>& Rows,
	const int32 Columns,
	const FName Tag)
{
	const int32 SafeColumns = FMath::Max(1, Columns);
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		const int32 Column = Index % SafeColumns;
		const int32 Row = Index / SafeColumns;
		Grid->AddSlot(Column, Row)
			.Padding(Column > 0 ? FMargin(18.f, 0.f, 0.f, 6.f) : FMargin(0.f, 0.f, 0.f, 6.f))
			[
				MakeFlatStatRow(Rows[Index].Label, Rows[Index].Value, Rows[Index].LabelRole, Rows[Index].ValueRole)
			];
	}
	return AttachMetadata(Grid, Tag, TEXT("StatsTable"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatProgressBar(
	TAttribute<float> Percent,
	const TOptional<FLinearColor>& OptionalColor,
	const FName Tag)
{
	TSharedRef<SWidget> Progress = SNew(SBox)
		.HeightOverride(12.f)
		[
			SNew(SProgressBar)
			.Style(&FlatProgressBarStyle())
			.Percent_Lambda([Percent]()
			{
				return TOptional<float>(FMath::Clamp(Percent.Get(0.f), 0.f, 1.f));
			})
			.FillColorAndOpacity(OptionalColor.Get(ProgressFill()))
		];
	return AttachMetadata(Progress, Tag, TEXT("ProgressBar"), ET66FlatState::Ready);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatDivider(
	const EOrientation Orientation,
	const float Length,
	const TOptional<FLinearColor>& OptionalColor,
	const FName Tag)
{
	const FLinearColor Color = OptionalColor.Get(DisabledBorder());
	TSharedRef<SWidget> Divider = SNew(SBox)
		.WidthOverride(Orientation == Orient_Vertical && Length > 0.f ? FOptionalSize(Length) : FOptionalSize())
		.HeightOverride(Orientation == Orient_Horizontal ? FOptionalSize(FlatStroke) : FOptionalSize())
		.MinDesiredWidth(Orientation == Orient_Vertical ? FlatStroke : 0.f)
		.MinDesiredHeight(Orientation == Orient_Vertical ? (Length > 0.f ? Length : 1.f) : FlatStroke)
		[
			SNew(SBorder)
			.BorderImage(FlatWhiteBrush())
			.BorderBackgroundColor(Color)
		];
	return AttachMetadata(Divider, Tag, TEXT("Divider"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatPaginationIndicator(
	const int32 Total,
	const int32 CurrentIndex,
	const TOptional<FLinearColor>& CurrentColor,
	const FName Tag)
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < FMath::Max(0, Total); ++Index)
	{
		Row->AddSlot()
			.AutoWidth()
			.Padding(Index > 0 ? FMargin(6.f, 0.f, 0.f, 0.f) : FMargin(0.f))
			[
				SNew(SBox)
				.WidthOverride(24.f)
				.HeightOverride(6.f)
				[
					SNew(SBorder)
					.BorderImage(FlatWhiteBrush())
					.BorderBackgroundColor(Index == CurrentIndex ? CurrentColor.Get(SelectedBorder()) : DisabledBorder())
				]
			];
	}
	return AttachMetadata(Row, Tag, TEXT("PaginationIndicator"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatPortraitSlot(
	const ET66FlatState State,
	const FSlateBrush* PortraitTexture,
	const FSlateBrush* OptionalRoleIcon,
	const FVector2D& OptionalSize,
	const FName Tag)
{
	TSharedRef<SOverlay> Overlay = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			PortraitTexture
				? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitTexture))
				: StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("?")))
					.Font(MakeBoldFont(22))
					.ColorAndOpacity(SecondaryText())
					.Justification(ETextJustify::Center))
		];

	if (OptionalRoleIcon)
	{
		Overlay->AddSlot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(4.f)
			[
				SNew(SBox)
				.WidthOverride(22.f)
				.HeightOverride(22.f)
				[
					SNew(SImage)
					.Image(OptionalRoleIcon)
					.ColorAndOpacity(PrimaryText())
				]
			];
	}

	TSharedRef<SWidget> Slot = SNew(SBox)
		.WidthOverride(OptionalSize.X)
		.HeightOverride(OptionalSize.Y)
		[
			MakeFlatPanel(State, FMargin(0.f), Overlay)
		];
	return AttachMetadata(Slot, Tag, TEXT("PortraitSlot"), State);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatRankingRow(
	const int32 Rank,
	const TAttribute<FText>& Name,
	const TAttribute<FText>& Score,
	const ET66FlatState OptionalState,
	const FName Tag)
{
	return AttachMetadata(MakeFlatSubPanel(
		OptionalState,
		FMargin(10.f, 6.f),
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			MakeFlatLabel(FText::Format(FText::FromString(TEXT("#{0}")), FText::AsNumber(Rank)), ET66FlatLabelRole::StatValue)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(12.f, 0.f)
		[
			MakeFlatLabel(Name, ET66FlatLabelRole::Body)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			MakeFlatLabel(Score, ET66FlatLabelRole::StatValue, ETextJustify::Right)
		]), Tag, TEXT("RankingRow"), OptionalState);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatTopBar(
	const TArray<TSharedRef<SWidget>>& Buttons,
	const bool bUseOuterContainer,
	const FName Tag)
{
	TSharedRef<SWidget> Row = MakeFlatActionRow(Buttons, ET66FlatActionAlignment::Fill, FlatGap);
	return AttachMetadata(bUseOuterContainer ? MakeFlatPanel(ET66FlatState::Default, FMargin(FlatGap), Row) : Row, Tag, TEXT("TopBar"), ET66FlatState::Default);
}

TSharedRef<SWidget> FT66FlatStyle::MakeFlatSlimTopBar(
	const TArray<TSharedRef<SWidget>>& LeftButtons,
	const TSharedRef<SWidget>& CenterButton,
	const TArray<TSharedRef<SWidget>>& RightButtons,
	const FName Tag)
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	for (const TSharedRef<SWidget>& Button : LeftButtons)
	{
		Row->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, FlatGap, 0.f)
			[
				Button
			];
	}
	Row->AddSlot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		[
			CenterButton
		];
	for (const TSharedRef<SWidget>& Button : RightButtons)
	{
		Row->AddSlot()
			.AutoWidth()
			.Padding(FlatGap, 0.f, 0.f, 0.f)
			[
				Button
			];
	}
	return AttachMetadata(MakeFlatPanel(ET66FlatState::Default, FMargin(FlatGap), Row), Tag, TEXT("SlimTopBar"), ET66FlatState::Default);
}
