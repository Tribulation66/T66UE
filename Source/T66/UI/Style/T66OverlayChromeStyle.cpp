// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Style/T66OverlayChromeStyle.h"

#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66Style.h"

#include "Brushes/SlateColorBrush.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	using FOptionalBrush = T66RuntimeUIBrushAccess::FOptionalTextureBrush;

	const FString& GetChromeSliceDir()
	{
		static const FString Dir = FPaths::ProjectDir() / TEXT("SourceAssets/UI/MasterLibrary/Slices");
		return Dir;
	}

	FString GetChromeSlicePath(const TCHAR* FileName)
	{
		return GetChromeSliceDir() / FileName;
	}

	const TCHAR* GetChromeFileName(const ET66OverlayChromeBrush Brush)
	{
		switch (Brush)
		{
		case ET66OverlayChromeBrush::OverlayModalPanel:
			return TEXT("Panels/modal_frame_normal.png");
		case ET66OverlayChromeBrush::CasinoShellPanel:
			return TEXT("Panels/panel_large_normal.png");
		case ET66OverlayChromeBrush::ContentPanelTall:
			return TEXT("Panels/panel_large_normal.png");
		case ET66OverlayChromeBrush::InnerPanel:
			return TEXT("Panels/panel_square_normal.png");
		case ET66OverlayChromeBrush::HeaderSummaryBar:
			return TEXT("Panels/panel_square_normal.png");
		case ET66OverlayChromeBrush::CrateStripFrame:
			return TEXT("Panels/panel_large_normal.png");
		case ET66OverlayChromeBrush::SlotNormal:
			return TEXT("Slots/avatar_slot_normal.png");
		case ET66OverlayChromeBrush::SlotHover:
			return TEXT("Slots/avatar_slot_normal.png");
		case ET66OverlayChromeBrush::SlotSelected:
			return TEXT("Slots/party_slot_normal.png");
		case ET66OverlayChromeBrush::SlotDisabled:
			return TEXT("Slots/avatar_slot_normal.png");
		case ET66OverlayChromeBrush::OfferCardNormal:
			return TEXT("Panels/panel_square_normal.png");
		case ET66OverlayChromeBrush::OfferCardHover:
			return TEXT("Panels/panel_square_normal.png");
		case ET66OverlayChromeBrush::OfferCardSelected:
			return TEXT("Panels/modal_frame_normal.png");
		case ET66OverlayChromeBrush::OfferCardDisabled:
			return TEXT("Panels/panel_square_normal.png");
		case ET66OverlayChromeBrush::CrateWinnerMarker:
			return TEXT("Panels/modal_frame_normal.png");
		case ET66OverlayChromeBrush::ContentPanelWide:
		default:
			return TEXT("Panels/panel_large_normal.png");
		}
	}

	FMargin GetChromeMargin(const ET66OverlayChromeBrush Brush)
	{
		switch (Brush)
		{
		case ET66OverlayChromeBrush::HeaderSummaryBar:
			return FMargin(0.06f, 0.28f, 0.06f, 0.28f);
		case ET66OverlayChromeBrush::SlotNormal:
		case ET66OverlayChromeBrush::SlotHover:
		case ET66OverlayChromeBrush::SlotSelected:
		case ET66OverlayChromeBrush::SlotDisabled:
		case ET66OverlayChromeBrush::CrateWinnerMarker:
			return FMargin(0.18f);
		case ET66OverlayChromeBrush::OfferCardNormal:
		case ET66OverlayChromeBrush::OfferCardHover:
		case ET66OverlayChromeBrush::OfferCardSelected:
		case ET66OverlayChromeBrush::OfferCardDisabled:
			return FMargin(0.1f, 0.08f);
		case ET66OverlayChromeBrush::OverlayModalPanel:
		case ET66OverlayChromeBrush::CasinoShellPanel:
		case ET66OverlayChromeBrush::ContentPanelWide:
		case ET66OverlayChromeBrush::ContentPanelTall:
		case ET66OverlayChromeBrush::InnerPanel:
		case ET66OverlayChromeBrush::CrateStripFrame:
		default:
			return FMargin(0.055f, 0.07f, 0.055f, 0.07f);
		}
	}

	FOptionalBrush& GetChromeEntry(const ET66OverlayChromeBrush Brush)
	{
		static FOptionalBrush OverlayModalPanel;
		static FOptionalBrush CasinoShellPanel;
		static FOptionalBrush ContentPanelWide;
		static FOptionalBrush ContentPanelTall;
		static FOptionalBrush InnerPanel;
		static FOptionalBrush HeaderSummaryBar;
		static FOptionalBrush CrateStripFrame;
		static FOptionalBrush SlotNormal;
		static FOptionalBrush SlotHover;
		static FOptionalBrush SlotSelected;
		static FOptionalBrush SlotDisabled;
		static FOptionalBrush OfferCardNormal;
		static FOptionalBrush OfferCardHover;
		static FOptionalBrush OfferCardSelected;
		static FOptionalBrush OfferCardDisabled;
		static FOptionalBrush CrateWinnerMarker;

		switch (Brush)
		{
		case ET66OverlayChromeBrush::OverlayModalPanel:
			return OverlayModalPanel;
		case ET66OverlayChromeBrush::CasinoShellPanel:
			return CasinoShellPanel;
		case ET66OverlayChromeBrush::ContentPanelTall:
			return ContentPanelTall;
		case ET66OverlayChromeBrush::InnerPanel:
			return InnerPanel;
		case ET66OverlayChromeBrush::HeaderSummaryBar:
			return HeaderSummaryBar;
		case ET66OverlayChromeBrush::CrateStripFrame:
			return CrateStripFrame;
		case ET66OverlayChromeBrush::SlotNormal:
			return SlotNormal;
		case ET66OverlayChromeBrush::SlotHover:
			return SlotHover;
		case ET66OverlayChromeBrush::SlotSelected:
			return SlotSelected;
		case ET66OverlayChromeBrush::SlotDisabled:
			return SlotDisabled;
		case ET66OverlayChromeBrush::OfferCardNormal:
			return OfferCardNormal;
		case ET66OverlayChromeBrush::OfferCardHover:
			return OfferCardHover;
		case ET66OverlayChromeBrush::OfferCardSelected:
			return OfferCardSelected;
		case ET66OverlayChromeBrush::OfferCardDisabled:
			return OfferCardDisabled;
		case ET66OverlayChromeBrush::CrateWinnerMarker:
			return CrateWinnerMarker;
		case ET66OverlayChromeBrush::ContentPanelWide:
		default:
			return ContentPanelWide;
		}
	}

	const TCHAR* GetButtonFileName(const ET66OverlayChromeButtonFamily Family, const bool bHovered, const bool bPressed, const bool bDisabled, const bool bSelected)
	{
		if (Family == ET66OverlayChromeButtonFamily::Tab)
		{
			if (bDisabled)
			{
				return TEXT("Tabs/wide_tab_normal.png");
			}
			if (bPressed || bSelected)
			{
				return TEXT("Tabs/wide_tab_selected.png");
			}
			return bHovered ? TEXT("Tabs/wide_tab_selected.png") : TEXT("Tabs/wide_tab_normal.png");
		}

		if (bDisabled)
		{
			return TEXT("Buttons/basic_button_disabled.png");
		}
		if (bPressed || bSelected)
		{
			return TEXT("Buttons/basic_button_pressed.png");
		}
		if (bHovered)
		{
			return TEXT("Buttons/basic_button_hover.png");
		}
		return TEXT("Buttons/basic_button_normal.png");
	}

	FMargin GetButtonMargin(const ET66OverlayChromeButtonFamily Family)
	{
		return Family == ET66OverlayChromeButtonFamily::Tab
			? FMargin(0.16f, 0.35f, 0.16f, 0.35f)
			: FMargin(0.18f, 0.34f, 0.18f, 0.34f);
	}

	FOptionalBrush& GetButtonEntry(const ET66OverlayChromeButtonFamily Family, const bool bHovered, const bool bPressed, const bool bDisabled, const bool bSelected)
	{
		static FOptionalBrush NeutralNormal;
		static FOptionalBrush NeutralHover;
		static FOptionalBrush NeutralPressed;
		static FOptionalBrush NeutralDisabled;
		static FOptionalBrush PrimaryNormal;
		static FOptionalBrush PrimaryHover;
		static FOptionalBrush PrimaryPressed;
		static FOptionalBrush PrimaryDisabled;
		static FOptionalBrush DangerNormal;
		static FOptionalBrush DangerHover;
		static FOptionalBrush DangerPressed;
		static FOptionalBrush DangerDisabled;
		static FOptionalBrush TabNormal;
		static FOptionalBrush TabHover;
		static FOptionalBrush TabPressed;
		static FOptionalBrush TabSelected;
		static FOptionalBrush TabDisabled;

		if (Family == ET66OverlayChromeButtonFamily::Tab)
		{
			if (bDisabled)
			{
				return TabDisabled;
			}
			if (bPressed)
			{
				return TabPressed;
			}
			if (bSelected)
			{
				return TabSelected;
			}
			return bHovered ? TabHover : TabNormal;
		}

		if (Family == ET66OverlayChromeButtonFamily::Primary)
		{
			if (bDisabled)
			{
				return PrimaryDisabled;
			}
			if (bPressed || bSelected)
			{
				return PrimaryPressed;
			}
			return bHovered ? PrimaryHover : PrimaryNormal;
		}

		if (Family == ET66OverlayChromeButtonFamily::Danger)
		{
			if (bDisabled)
			{
				return DangerDisabled;
			}
			if (bPressed || bSelected)
			{
				return DangerPressed;
			}
			return bHovered ? DangerHover : DangerNormal;
		}

		if (bDisabled)
		{
			return NeutralDisabled;
		}
		if (bPressed || bSelected)
		{
			return NeutralPressed;
		}
		return bHovered ? NeutralHover : NeutralNormal;
	}

	const FSlateBrush* GetButtonBrush(const ET66OverlayChromeButtonFamily Family, const bool bHovered, const bool bPressed, const bool bDisabled, const bool bSelected)
	{
		const TCHAR* FileName = GetButtonFileName(Family, bHovered, bPressed, bDisabled, bSelected);
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			GetButtonEntry(Family, bHovered, bPressed, bDisabled, bSelected),
			nullptr,
			GetChromeSlicePath(FileName),
			GetButtonMargin(Family),
			FileName);
	}

	const FButtonStyle& GetTransparentButtonStyle()
	{
		static FButtonStyle Style;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			bInitialized = true;
			const FSlateBrush* NoBrush = FCoreStyle::Get().GetBrush("NoBrush");
			Style.SetNormal(*NoBrush);
			Style.SetHovered(*NoBrush);
			Style.SetPressed(*NoBrush);
			Style.SetDisabled(*NoBrush);
			Style.SetNormalPadding(FMargin(0.f));
			Style.SetPressedPadding(FMargin(0.f));
		}
		return Style;
	}

	class ST66OverlayChromeButton final : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66OverlayChromeButton)
			: _Family(ET66OverlayChromeButtonFamily::Neutral)
			, _MinWidth(120.f)
			, _MinHeight(44.f)
			, _Padding(FMargin(12.f, 5.f))
			, _IsEnabled(true)
			, _IsSelected(false)
		{
		}
			SLATE_DEFAULT_SLOT(FArguments, Content)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_ARGUMENT(ET66OverlayChromeButtonFamily, Family)
			SLATE_ARGUMENT(float, MinWidth)
			SLATE_ARGUMENT(float, MinHeight)
			SLATE_ARGUMENT(FMargin, Padding)
			SLATE_ATTRIBUTE(bool, IsEnabled)
			SLATE_ATTRIBUTE(bool, IsSelected)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Family = InArgs._Family;
			IsEnabledAttribute = InArgs._IsEnabled;
			IsSelectedAttribute = InArgs._IsSelected;

			ChildSlot
			[
				SNew(SBox)
				.MinDesiredWidth(InArgs._MinWidth)
				.MinDesiredHeight(InArgs._MinHeight)
				[
					SNew(SBorder)
					.BorderImage(this, &ST66OverlayChromeButton::GetCurrentBrush)
					.Padding(InArgs._Padding)
					[
						SAssignNew(Button, SButton)
						.Cursor(EMouseCursor::Hand)
						.ButtonStyle(&GetTransparentButtonStyle())
						.ContentPadding(FMargin(0.f))
						.IsEnabled(InArgs._IsEnabled)
						.OnClicked(FT66Style::DebounceClick(InArgs._OnClicked))
						[
							InArgs._Content.Widget
						]
					]
				]
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			const bool bDisabled = !IsEnabledAttribute.Get(true);
			const bool bPressed = Button.IsValid() && Button->IsPressed();
			const bool bHovered = Button.IsValid() && Button->IsHovered();
			const bool bSelected = IsSelectedAttribute.Get(false);

			if (const FSlateBrush* Brush = GetButtonBrush(Family, bHovered, bPressed, bDisabled, bSelected))
			{
				return Brush;
			}
			return FCoreStyle::Get().GetBrush("WhiteBrush");
		}

		ET66OverlayChromeButtonFamily Family = ET66OverlayChromeButtonFamily::Neutral;
		TAttribute<bool> IsEnabledAttribute;
		TAttribute<bool> IsSelectedAttribute;
		TSharedPtr<SButton> Button;
	};
}

namespace T66OverlayChromeStyle
{
	const FSlateBrush* GetBrush(const ET66OverlayChromeBrush Brush)
	{
		const TCHAR* FileName = GetChromeFileName(Brush);
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			GetChromeEntry(Brush),
			nullptr,
			GetChromeSlicePath(FileName),
			GetChromeMargin(Brush),
			FileName);
	}

	FT66OverlayChromeButtonParams MakeButtonParams(
		const FText& Label,
		FOnClicked OnClicked,
		const ET66OverlayChromeButtonFamily Family)
	{
		return FT66OverlayChromeButtonParams(Label, MoveTemp(OnClicked), Family);
	}

	TSharedRef<SWidget> MakePanel(
		const TSharedRef<SWidget>& Content,
		const ET66OverlayChromeBrush Brush,
		const FMargin& Padding,
		TSharedPtr<SBorder>* OutBorder)
	{
		if (const FSlateBrush* ChromeBrush = GetBrush(Brush))
		{
			TSharedPtr<SBorder> Panel;
			TSharedRef<SBorder> PanelRef =
				SAssignNew(Panel, SBorder)
				.BorderImage(ChromeBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];

			if (OutBorder)
			{
				*OutBorder = Panel;
			}
			return PanelRef;
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(Padding),
			OutBorder);
	}

	TSharedRef<SWidget> MakeButton(const FT66OverlayChromeButtonParams& Params)
	{
		TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(Params.Label)
				.Font(FT66Style::Tokens::FontBold(Params.FontSize))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.93f, 0.84f, 1.f)))
				.Justification(ETextJustify::Center));

		return SNew(ST66OverlayChromeButton)
			.Family(Params.Family)
			.MinWidth(Params.MinWidth)
			.MinHeight(Params.MinHeight)
			.Padding(Params.Padding)
			.IsEnabled(Params.IsEnabled)
			.IsSelected(Params.IsSelected)
			.OnClicked(Params.OnClicked)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeSlotPanel(
		const TSharedRef<SWidget>& Content,
		const TAttribute<bool>& IsSelected,
		const TAttribute<bool>& IsEnabled,
		const FMargin& Padding)
	{
		return SNew(SBorder)
			.BorderImage_Lambda([IsSelected, IsEnabled]()
			{
				if (!IsEnabled.Get(true))
				{
					return GetBrush(ET66OverlayChromeBrush::SlotDisabled);
				}
				if (IsSelected.Get(false))
				{
					return GetBrush(ET66OverlayChromeBrush::SlotSelected);
				}
				return GetBrush(ET66OverlayChromeBrush::SlotNormal);
			})
			.BorderBackgroundColor(FLinearColor::White)
			.Padding(Padding)
			[
				Content
			];
	}
}
