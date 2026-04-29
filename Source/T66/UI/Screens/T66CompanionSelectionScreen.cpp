// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Screens/T66SelectionScreenUtils.h"
#include "UI/T66UIManager.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66SkinSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66FrontendGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SProgressBar.h"
// Render target removed — in-world preview uses main viewport camera with full Lumen GI.

namespace
{
	AT66PlayerController* T66GetLocalFrontendCompanionPlayerController(UObject* ContextObject)
	{
		return ContextObject ? Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(ContextObject, 0)) : nullptr;
	}

	void T66PositionCompanionPreviewCamera(UObject* ContextObject)
	{
		if (!ContextObject)
		{
			return;
		}

		if (UWorld* World = ContextObject->GetWorld())
		{
			if (AT66FrontendGameMode* GM = Cast<AT66FrontendGameMode>(World->GetAuthGameMode()))
			{
				GM->PositionCameraForCompanionPreview();
				return;
			}
		}

		if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(ContextObject))
		{
			PC->PositionLocalFrontendCameraForCompanionPreview();
		}
	}

	enum class ET66CompanionReferenceButtonFamily : uint8
	{
		CompactNeutral,
		ToggleOn,
		ToggleOff,
		CtaPrimary
	};

	enum class ET66CompanionReferenceButtonState : uint8
	{
		Normal,
		Hovered,
		Pressed,
		Disabled
	};

	struct FCompanionReferenceButtonBrushSet
	{
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Normal;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Hovered;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Pressed;
		T66RuntimeUIBrushAccess::FOptionalTextureBrush Disabled;
	};

	const FSlateBrush* ResolveCompanionReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	FCompanionReferenceButtonBrushSet& GetCompanionReferenceButtonBrushSet(const ET66CompanionReferenceButtonFamily Family)
	{
		static FCompanionReferenceButtonBrushSet CompactNeutral;
		static FCompanionReferenceButtonBrushSet ToggleOn;
		static FCompanionReferenceButtonBrushSet ToggleOff;
		static FCompanionReferenceButtonBrushSet CtaPrimary;

		switch (Family)
		{
		case ET66CompanionReferenceButtonFamily::ToggleOn:
			return ToggleOn;
		case ET66CompanionReferenceButtonFamily::ToggleOff:
			return ToggleOff;
		case ET66CompanionReferenceButtonFamily::CtaPrimary:
			return CtaPrimary;
		case ET66CompanionReferenceButtonFamily::CompactNeutral:
		default:
			return CompactNeutral;
		}
	}

	FString GetCompanionReferenceButtonPath(
		const ET66CompanionReferenceButtonFamily Family,
		const ET66CompanionReferenceButtonState State)
	{
		const TCHAR* StateSuffix = TEXT("normal");
		switch (State)
		{
		case ET66CompanionReferenceButtonState::Hovered:
			StateSuffix = TEXT("hover");
			break;
		case ET66CompanionReferenceButtonState::Pressed:
			StateSuffix = TEXT("pressed");
			break;
		case ET66CompanionReferenceButtonState::Disabled:
			StateSuffix = TEXT("disabled");
			break;
		case ET66CompanionReferenceButtonState::Normal:
		default:
			break;
		}

		if (Family == ET66CompanionReferenceButtonFamily::ToggleOn && State == ET66CompanionReferenceButtonState::Normal)
		{
			StateSuffix = TEXT("selected");
		}

		const TCHAR* Prefix = Family == ET66CompanionReferenceButtonFamily::ToggleOn || Family == ET66CompanionReferenceButtonFamily::ToggleOff
			? TEXT("select_button")
			: TEXT("basic_button");
		return FString::Printf(TEXT("SourceAssets/UI/MasterLibrary/Slices/Buttons/%s_%s.png"), Prefix, StateSuffix);
	}

	FMargin GetCompanionReferenceButtonMargin(const ET66CompanionReferenceButtonFamily /*Family*/)
	{
		return FMargin(0.093f, 0.213f, 0.093f, 0.213f);
	}

	const FSlateBrush* ResolveCompanionReferenceButtonBrush(
		const ET66CompanionReferenceButtonFamily Family,
		const ET66CompanionReferenceButtonState State)
	{
		FCompanionReferenceButtonBrushSet& Set = GetCompanionReferenceButtonBrushSet(Family);
		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Set.Normal;
		if (State == ET66CompanionReferenceButtonState::Hovered)
		{
			Entry = &Set.Hovered;
		}
		else if (State == ET66CompanionReferenceButtonState::Pressed)
		{
			Entry = &Set.Pressed;
		}
		else if (State == ET66CompanionReferenceButtonState::Disabled)
		{
			Entry = &Set.Disabled;
		}

		return ResolveCompanionReferenceBrush(
			*Entry,
			GetCompanionReferenceButtonPath(Family, State),
			GetCompanionReferenceButtonMargin(Family),
			TEXT("CompanionReferenceButton"));
	}

	const FSlateBrush* GetCompanionLeftPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("CompanionLeftShell"));
	}

	const FSlateBrush* GetCompanionRightPanelShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("CompanionRightShell"));
	}

	const FSlateBrush* GetCompanionRowShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Panels/basic_panel_normal.png"),
			FMargin(0.067f, 0.043f, 0.067f, 0.043f),
			TEXT("CompanionRowShell"));
	}

	const FSlateBrush* GetCompanionFieldShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Controls/dropdown_field_normal.png"),
			FMargin(0.06f, 0.34f, 0.06f, 0.34f),
			TEXT("CompanionFieldShell"));
	}

	const FSlateBrush* GetCompanionAvatarFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/Slices/Slots/basic_slot_normal.png"),
			FMargin(0.20f, 0.20f, 0.20f, 0.20f),
			TEXT("CompanionAvatarFrame"));
	}

	const FSlateBrush* GetCompanionSceneBackgroundBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveCompanionReferenceBrush(
			Entry,
			TEXT("SourceAssets/UI/MasterLibrary/ScreenArt/MainMenu/main_menu_scene_plate_imagegen_20260425_v1.png"),
			FMargin(0.f),
			TEXT("CompanionSceneBackground"));
	}

	TSharedRef<SWidget> MakeCompanionReferencePanel(
		const TSharedRef<SWidget>& Content,
		const FSlateBrush* Brush,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (Brush)
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionReferenceRow(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (const FSlateBrush* Brush = GetCompanionRowShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel2)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionReferenceField(
		const TSharedRef<SWidget>& Content,
		const FMargin& Padding,
		const FSlateColor& FallbackFill)
	{
		if (const FSlateBrush* Brush = GetCompanionFieldShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(Brush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(FallbackFill)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeCompanionAvatarSocket(
		const TSharedRef<SWidget>& Content,
		const FLinearColor& FallbackFill,
		const float Opacity,
		const bool bSelected)
	{
		if (const FSlateBrush* FrameBrush = GetCompanionAvatarFrameBrush())
		{
			const FMargin ContentInset = bSelected ? FMargin(5.f) : FMargin(4.f);
			return SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FallbackFill * FMath::Clamp(Opacity, 0.0f, 1.0f))
				]
				+ SOverlay::Slot()
				.Padding(ContentInset)
				[
					Content
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(FrameBrush)
					.ColorAndOpacity(bSelected
						? FLinearColor(1.12f, 1.04f, 0.82f, 1.0f)
						: FLinearColor(0.78f, 0.88f, 0.78f, 0.88f))
				];
		}

		return FT66Style::MakeSlotFrame(Content, FallbackFill * Opacity, FMargin(2.f));
	}

	class ST66CompanionReferenceButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66CompanionReferenceButton)
			: _ButtonFamily(ET66CompanionReferenceButtonFamily::CompactNeutral)
			, _MinWidth(0.f)
			, _Height(0.f)
			, _ContentPadding(FMargin(0.f))
			, _IsEnabled(true)
			, _Visibility(EVisibility::Visible)
		{
		}
			SLATE_ATTRIBUTE(ET66CompanionReferenceButtonFamily, ButtonFamily)
			SLATE_ARGUMENT(float, MinWidth)
			SLATE_ARGUMENT(float, Height)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(TAttribute<bool>, IsEnabled)
			SLATE_ARGUMENT(TAttribute<EVisibility>, Visibility)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			ButtonFamily = InArgs._ButtonFamily;
			ContentPadding = InArgs._ContentPadding;
			OwnedButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			OwnedButtonStyle.SetNormalPadding(FMargin(0.f));
			OwnedButtonStyle.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(this, &ST66CompanionReferenceButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(this, &ST66CompanionReferenceButton::GetContentPadding)
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&OwnedButtonStyle)
					.SetPadding(FMargin(0.f))
					.SetEnabled(InArgs._IsEnabled)
					.SetMinWidth(InArgs._MinWidth)
					.SetHeight(InArgs._Height)
					.SetVisibility(InArgs._Visibility),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			const ET66CompanionReferenceButtonFamily Family = ButtonFamily.Get(ET66CompanionReferenceButtonFamily::CompactNeutral);
			if (!Button.IsValid() || !Button->IsEnabled())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Disabled);
			}
			if (Button->IsPressed())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Pressed);
			}
			if (Button->IsHovered())
			{
				return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Hovered);
			}
			return ResolveCompanionReferenceButtonBrush(Family, ET66CompanionReferenceButtonState::Normal);
		}

		FMargin GetContentPadding() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FMargin(
					ContentPadding.Left,
					ContentPadding.Top + 1.f,
					ContentPadding.Right,
					FMath::Max(0.f, ContentPadding.Bottom - 1.f));
			}
			return ContentPadding;
		}

		TAttribute<ET66CompanionReferenceButtonFamily> ButtonFamily;
		FMargin ContentPadding = FMargin(0.f);
		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};

	TSharedRef<SWidget> MakeCompanionReferenceButton(
		const FT66ButtonParams& Params,
		TAttribute<ET66CompanionReferenceButtonFamily> ButtonFamily)
	{
		const ET66CompanionReferenceButtonFamily InitialFamily = ButtonFamily.Get(ET66CompanionReferenceButtonFamily::CompactNeutral);
		if (!ResolveCompanionReferenceButtonBrush(InitialFamily, ET66CompanionReferenceButtonState::Normal))
		{
			return FT66Style::MakeButton(Params);
		}

		const float ButtonHeight = Params.Height > 0.f ? Params.Height : 44.f;
		const FMargin ContentPadding = Params.Padding.Left >= 0.f ? Params.Padding : FMargin(6.f, 2.f);

		const TSharedRef<SWidget> Content = Params.CustomContent.IsValid()
			? Params.CustomContent.ToSharedRef()
			: T66ScreenSlateHelpers::MakeFilledButtonText(
				Params,
				ButtonHeight,
				TAttribute<FSlateColor>(FSlateColor(FT66Style::Tokens::Text)),
				TAttribute<FLinearColor>(FLinearColor(0.f, 0.f, 0.f, 0.68f)));

		return SNew(ST66CompanionReferenceButton)
			.ButtonFamily(ButtonFamily)
			.MinWidth(Params.MinWidth)
			.Height(ButtonHeight)
			.ContentPadding(ContentPadding)
			.IsEnabled(Params.IsEnabled)
			.Visibility(Params.Visibility)
			.OnClicked(Params.OnClicked)
			[
				Content
			];
	}

	TSharedRef<SWidget> MakeCompanionReferenceButton(
		const FT66ButtonParams& Params,
		const ET66CompanionReferenceButtonFamily ButtonFamily)
	{
		return MakeCompanionReferenceButton(Params, TAttribute<ET66CompanionReferenceButtonFamily>(ButtonFamily));
	}
}

UT66CompanionSelectionScreen::UT66CompanionSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::CompanionSelection;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66CompanionSelectionScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

bool UT66CompanionSelectionScreen::IsCompanionUnlocked(FName CompanionID) const
{
	if (CompanionID.IsNone())
	{
		return true;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
		{
			return Unlocks->IsCompanionUnlocked(CompanionID);
		}
	}
	return true; // fail-open so we don't hard-lock the UI if subsystem is missing
}

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SkinSubsystem* Skin = GI ? GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
	FName CompanionForSkins = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;
	if (Skin && !CompanionForSkins.IsNone())
	{
		PlaceholderSkins = Skin->GetSkinsForEntity(ET66SkinEntityType::Companion, CompanionForSkins);
	}
	else
	{
		T66SelectionScreenUtils::PopulateDefaultOwnedSkins(PlaceholderSkins);
	}
}

void UT66CompanionSelectionScreen::RefreshSkinsList()
{
	GeneratePlaceholderSkins();
	if (SkinsListBoxWidget.IsValid())
	{
		SkinsListBoxWidget->ClearChildren();
		AddSkinRowsToBox(SkinsListBoxWidget);
	}
	if (ACBalanceTextBlock.IsValid())
	{
		ACBalanceTextBlock->SetText(T66SelectionScreenUtils::FormatAchievementCoinBalance(
			GetLocSubsystem(),
			T66SelectionScreenUtils::GetAchievementCoinBalance(this)));
	}
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::AddSkinRowsToBox(const TSharedPtr<SVerticalBox>& Box)
{
	if (!Box.IsValid()) return;
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	const FText EquippedText = NSLOCTEXT("T66.HeroSelection", "Equipped", "EQUIPPED");
	const FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	static constexpr int32 BeachgoerPriceAC = UT66SkinSubsystem::DefaultSkinPriceAC;
	const float ActionMinHeight = 36.f;
	const float ActionMinWidth = 96.f;
	const float EquippedMinWidth = 104.f;
	const float BuyButtonMinWidth = 108.f;
	const float BuyButtonHeight = 44.f;
	const FText BeachgoerPriceText = T66SelectionScreenUtils::FormatAchievementCoinBalance(Loc, BeachgoerPriceAC);
	const FSlateColor SkinRowFill = FT66Style::IsDotaTheme()
		? FSlateColor(FLinearColor(0.028f, 0.028f, 0.031f, 1.0f))
		: FT66Style::Tokens::Panel2;
	const FSlateColor SkinFieldFill = FT66Style::IsDotaTheme()
		? FSlateColor(FLinearColor(0.075f, 0.075f, 0.08f, 1.0f))
		: FT66Style::Tokens::Accent2;

	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FName SkinIDCopy = Skin.SkinID;
		bool bIsDefault = Skin.bIsDefault;
		bool bIsOwned = Skin.bIsOwned;
		bool bIsEquipped = Skin.bIsEquipped;
		FName CID = PreviewedCompanionID.IsNone() && AllCompanionIDs.Num() > 0 ? AllCompanionIDs[0] : PreviewedCompanionID;

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SkinName(SkinIDCopy) : FText::FromName(SkinIDCopy))
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];

		if (bIsDefault)
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(ActionMinHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(bIsEquipped ? 1 : 0)
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, UT66SkinSubsystem::DefaultSkinID);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								MakeCompanionReferenceField(
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center),
									FMargin(10.0f, 4.0f),
									SkinFieldFill)
							]
						]
					]
				];
		}
		else
		{
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(5.0f, 0.0f)
				[
					MakeCompanionReferenceButton(FT66ButtonParams(PreviewText,
					FOnClicked::CreateLambda([this, SkinIDCopy]()
					{
						PreviewedCompanionSkinIDOverride = (PreviewedCompanionSkinIDOverride == SkinIDCopy) ? NAME_None : SkinIDCopy;
						UpdateCompanionDisplay();
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
				];
			Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(EquippedMinWidth).MinDesiredHeight(BuyButtonHeight)
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(!bIsOwned ? 0 : (bIsEquipped ? 2 : 1))
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(BuyText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>();
								if (!SkinSub || !SkinSub->PurchaseCompanionSkin(CID, SkinIDCopy, BeachgoerPriceAC)) return FReply::Handled();
								SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
								PreviewedCompanionSkinIDOverride = NAME_None;
								RefreshSkinsList();
								return FReply::Handled();
							}),
							ET66ButtonType::Primary)
							.SetMinWidth(BuyButtonMinWidth)
							.SetHeight(BuyButtonHeight)
							.SetColor(FT66Style::Tokens::Accent)
							.SetPadding(FMargin(6.f, 3.f))
							.SetContent(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BuyText).Font(FT66Style::Tokens::FontBold(15)).ColorAndOpacity(FT66Style::Tokens::Text) ]
								+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[ SNew(STextBlock).Text(BeachgoerPriceText).Font(FT66Style::Tokens::FontRegular(13)).ColorAndOpacity(FT66Style::Tokens::Text) ]
							),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
						MakeCompanionReferenceButton(FT66ButtonParams(EquipText,
							FOnClicked::CreateLambda([this, CID, SkinIDCopy]()
							{
								if (CID.IsNone()) return FReply::Handled();
								if (UT66SkinSubsystem* SkinSub = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UT66SkinSubsystem>())
								{
									SkinSub->SetEquippedCompanionSkinID(CID, SkinIDCopy);
									PreviewedCompanionSkinIDOverride = NAME_None;
									RefreshSkinsList();
								}
								return FReply::Handled();
							}),
							ET66ButtonType::Primary).SetMinWidth(ActionMinWidth).SetHeight(ActionMinHeight).SetFontSize(16),
							ET66CompanionReferenceButtonFamily::ToggleOn)
						]
						+ SWidgetSwitcher::Slot()
						[
							SNew(SBox).MinDesiredWidth(EquippedMinWidth).HeightOverride(ActionMinHeight)
							[
								MakeCompanionReferenceField(
									SNew(STextBlock)
									.Text(EquippedText)
									.Font(FT66Style::Tokens::FontBold(16))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center),
									FMargin(10.0f, 4.0f),
									SkinFieldFill)
							]
						]
					]
				];
		}
		Box->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				MakeCompanionReferenceRow(
					Row,
					FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3),
					SkinRowFill)
			];
	}
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	FText CompanionGridText = Loc ? Loc->GetText_CompanionGrid() : NSLOCTEXT("T66.CompanionSelection", "Grid", "COMPANION GRID");
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanion", "NO COMPANION");
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.CompanionSelection", "Skins", "SKINS");
	FText LoreText = Loc ? Loc->GetText_Lore() : NSLOCTEXT("T66.CompanionSelection", "Lore", "LORE");
	FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : NSLOCTEXT("T66.CompanionSelection", "ConfirmCompanion", "CONFIRM COMPANION");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	const FText ACBalanceText = T66SelectionScreenUtils::FormatAchievementCoinBalance(
		Loc,
		T66SelectionScreenUtils::GetAchievementCoinBalance(this));

	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FSlateColor SelectionPanelFill = FLinearColor(0.027f, 0.025f, 0.038f, 1.0f);
	const FSlateColor SelectionInsetFill = FLinearColor(0.046f, 0.042f, 0.058f, 1.0f);
	const FSlateColor SelectionToggleFill = FLinearColor(0.12f, 0.18f, 0.10f, 1.0f);

	UT66GameInstance* GICheck = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	TArray<FName> CarouselIDs;
	CarouselIDs.Add(NAME_None);
	CarouselIDs.Append(AllCompanionIDs);
	const int32 NumCarousel = CarouselIDs.Num();
	const int32 CarouselIndex = FMath::Clamp(CurrentCompanionIndex + 1, 0, NumCarousel > 0 ? NumCarousel - 1 : 0);

	CompanionCarouselPortraitBrushes.SetNum(5);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	for (int32 Offset = -2; Offset <= 2; Offset++)
	{
		const int32 SlotIdx = Offset + 2;
		const int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
		const FName CompanionID = NumCarousel > 0 ? CarouselIDs[Idx] : NAME_None;
		FCompanionData Data;
		FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f);
		if (!CompanionID.IsNone() && GICheck && GICheck->GetCompanionData(CompanionID, Data))
		{
			SpriteColor = Data.PlaceholderColor;
		}
		const bool bUnlocked = IsCompanionUnlocked(CompanionID);
		if (!CompanionID.IsNone() && !bUnlocked)
		{
			SpriteColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		}

		const float BoxSize = (Offset == 0) ? 60.0f : 45.0f;
		const float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		const TSharedRef<SWidget> CarouselSlotWidget = MakeCompanionAvatarSocket(
			SNew(SImage)
			.Image_Lambda([this, SlotIdx]() -> const FSlateBrush*
			{
				return CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? CompanionCarouselPortraitBrushes[SlotIdx].Get() : nullptr;
			})
			.Visibility_Lambda([this, SlotIdx]() -> EVisibility
			{
				if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
				{
					return EVisibility::Collapsed;
				}
				return CompanionCarouselPortraitBrushes[SlotIdx]->GetResourceObject() ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity)),
			SpriteColor,
			Opacity,
			Offset == 0);

		CompanionCarousel->AddSlot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BoxSize)
				.HeightOverride(BoxSize)
				[
					CarouselSlotWidget
				]
			];
	}

	FText CurrentCompanionName = NoCompanionText;
	FText CurrentCompanionLore = NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone.");
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = Data.LoreText;
		}
	}

	FLinearColor PreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			PreviewColor = Data.PlaceholderColor;
		}
	}
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		PreviewColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	}

	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const float SidePanelFill = 0.30f;
	const float CenterPanelFill = 0.40f;
	const float TopBarBottomGap = 0.f;
	const float ContentTopGap = 0.f;
	const float PanelBottomInset = 0.f;
	const float PanelGap = 8.f;
	const int32 ScreenHeaderFontSize = 21;
	const int32 CompactButtonFontSize = 18;
	const int32 PrimaryCtaFontSize = 28;
	const float CompanionGridButtonWidth = bDotaTheme ? 172.f : 150.f;
	const float NoCompanionButtonWidth = bDotaTheme ? 172.f : 150.f;
	const FMargin TopBarButtonPadding = FMargin(6.f, 2.f);
	const float ArrowButtonWidth = bDotaTheme ? 44.f : 40.f;
	const float ArrowButtonHeight = bDotaTheme ? 36.f : 34.f;
	const int32 ArrowFontSize = 20;
	const TAttribute<FMargin> ScreenSafePadding = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		const float TopInset = (UIManager && UIManager->IsFrontendTopBarVisible())
			? UIManager->GetFrontendTopBarContentHeight()
			: 0.f;
		return FMargin(0.f, TopInset, 0.f, 0.f);
	});
	auto MakePreviewFocusMask = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	auto MakeWorldScrim = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	const TSharedRef<SWidget> TopBarContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						CompanionGridText,
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleCompanionGridClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(CompanionGridButtonWidth)
					.SetHeight(34.f)
					.SetPadding(TopBarButtonPadding)
					.SetFontSize(CompactButtonFontSize),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Common", "Prev", "<"),
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(ArrowButtonWidth)
					.SetHeight(ArrowButtonHeight)
					.SetFontSize(ArrowFontSize),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				CompanionCarousel
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Common", "Next", ">"),
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(ArrowButtonWidth)
					.SetHeight(ArrowButtonHeight)
					.SetFontSize(ArrowFontSize),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(14.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						NoCompanionText,
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNoCompanionClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(NoCompanionButtonWidth)
					.SetHeight(34.f)
					.SetPadding(TopBarButtonPadding)
					.SetFontSize(CompactButtonFontSize)
					.SetColor(PreviewedCompanionID.IsNone() ? SelectionToggleFill : SelectionPanelFill),
					TAttribute<ET66CompanionReferenceButtonFamily>::CreateLambda([this]() -> ET66CompanionReferenceButtonFamily
					{
						return PreviewedCompanionID.IsNone()
							? ET66CompanionReferenceButtonFamily::ToggleOn
							: ET66CompanionReferenceButtonFamily::CompactNeutral;
					}))
			]
		];

	const TSharedRef<SWidget> TopBarWidget = MakeCompanionReferenceRow(
		TopBarContent,
		FMargin(8.f, 6.f),
		SelectionPanelFill);

	const TSharedRef<SWidget> LeftPanelWidget = MakeCompanionReferencePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(SkinsText)
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				MakeCompanionReferenceField(
					SAssignNew(ACBalanceTextBlock, STextBlock)
					.Text(ACBalanceText)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text),
					FMargin(10.0f, 5.0f),
					bDotaTheme ? SelectionInsetFill : FSlateColor(FT66Style::Tokens::Panel))
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SkinsListBoxWidget.ToSharedRef()
			]
		],
		GetCompanionLeftPanelShellBrush(),
		FMargin(22.f, 24.f, 22.f, 22.f),
		bDotaTheme ? SelectionPanelFill : FSlateColor(FT66Style::Tokens::Panel));

	TSharedRef<SWidget> PreviewWidget =
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			CreateCompanionPreviewWidget(PreviewColor)
		];

	const TSharedRef<SWidget> CompanionInfoView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionInfo", "COMPANION INFO"))
			.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				MakeCompanionReferenceField(
					SAssignNew(CompanionNameWidget, STextBlock)
					.Text(CurrentCompanionName)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true),
					FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2),
					bDotaTheme ? SelectionInsetFill : FSlateColor(FT66Style::Tokens::Panel))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						LoreText,
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(64.f)
					.SetHeight(32.f)
					.SetFontSize(CompactButtonFontSize),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(CompanionLoreWidget, STextBlock)
			.Text(CurrentCompanionLore)
			.Font(FT66Style::Tokens::FontRegular(15))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeCompanionReferenceRow(
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.CompanionSelection", "CompanionPassivePlaceholder", "Passive: Heals the player during combat"))
				.Font(FT66Style::Tokens::FontRegular(15))
				.ColorAndOpacity(bDotaTheme
					? FLinearColor(0.75f, 0.75f, 0.78f, 1.0f)
					: FLinearColor(0.6f, 0.9f, 0.6f, 1.0f))
				.AutoWrapText(true),
				FMargin(10.0f, 8.0f),
				bDotaTheme
					? FSlateColor(FLinearColor(0.032f, 0.032f, 0.036f, 1.0f))
					: FSlateColor(FLinearColor(0.1f, 0.15f, 0.1f, 1.0f)))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(CompanionUnionBox, SBox)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(CompanionUnionText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 6.f)
				[
					SNew(SBox)
					.WidthOverride(180.f)
					.HeightOverride(9.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SAssignNew(CompanionUnionProgressBar, SProgressBar)
							.Percent_Lambda([this]() -> TOptional<float>
							{
								return FMath::Clamp(CompanionUnionProgress01, 0.f, 1.f);
							})
							.FillColorAndOpacity(FLinearColor(0.20f, 0.65f, 0.35f, 1.0f))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.Padding(FMargin(180.f * 0.25f - 1.f, 0.f, 0.f, 0.f))
						[
							SNew(SBox)
							.WidthOverride(2.f)
							.HeightOverride(9.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Left)
						.Padding(FMargin(180.f * 0.50f - 1.f, 0.f, 0.f, 0.f))
						[
							SNew(SBox)
							.WidthOverride(2.f)
							.HeightOverride(9.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						[
							SNew(SBox)
							.WidthOverride(2.f)
							.HeightOverride(9.f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(CompanionUnionHealingText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(15))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBox)
		];

	const TSharedRef<SWidget> CompanionLoreView =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(LoreText)
			.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				MakeCompanionReferenceField(
					SNew(STextBlock)
					.Text_Lambda([this, Loc, NoCompanionText]() -> FText
					{
						if (PreviewedCompanionID.IsNone())
						{
							return NoCompanionText;
						}

						FCompanionData Data;
						if (GetPreviewedCompanionData(Data))
						{
							return Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
						}

						return NoCompanionText;
					})
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center),
					FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2),
					bDotaTheme ? SelectionInsetFill : FSlateColor(FT66Style::Tokens::Panel))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeCompanionReferenceButton(
					FT66ButtonParams(
						BackText,
						FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(64.f)
					.SetHeight(32.f)
					.SetFontSize(CompactButtonFontSize),
					ET66CompanionReferenceButtonFamily::CompactNeutral)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(CompanionLoreDetailWidget, STextBlock)
				.Text_Lambda([this, Loc]() -> FText
				{
					return Loc
						? Loc->GetText_CompanionLore(PreviewedCompanionID)
						: NSLOCTEXT("T66.CompanionLore", "FallbackLore", "No lore available.");
				})
				.Font(FT66Style::Tokens::FontRegular(15))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			]
		];

	const TSharedRef<SWidget> RightPanelWidget = MakeCompanionReferencePanel(
		SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() -> int32
		{
			return bShowingLore ? 1 : 0;
		})
		+ SWidgetSwitcher::Slot()
		[
			CompanionInfoView
		]
		+ SWidgetSwitcher::Slot()
		[
			CompanionLoreView
		],
		GetCompanionRightPanelShellBrush(),
		FMargin(24.f, 26.f, 24.f, 24.f),
		bDotaTheme ? SelectionPanelFill : FSlateColor(FT66Style::Tokens::Panel));

	const TSharedRef<SWidget> ConfirmButtonWidget = bDotaTheme
		? StaticCastSharedRef<SWidget>(
			MakeCompanionReferenceButton(
				FT66ButtonParams(
					ConfirmText,
					FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked),
					ET66ButtonType::Primary)
				.SetMinWidth(260.f)
				.SetHeight(58.f)
				.SetFontSize(PrimaryCtaFontSize)
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() -> bool
				{
					return PreviewedCompanionID.IsNone() || IsCompanionUnlocked(PreviewedCompanionID);
				})),
				ET66CompanionReferenceButtonFamily::CtaPrimary))
		: StaticCastSharedRef<SWidget>(
			MakeCompanionReferenceButton(
				FT66ButtonParams(
					ConfirmText,
					FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked),
					ET66ButtonType::Primary)
				.SetMinWidth(220.f)
				.SetHeight(50.f)
				.SetFontSize(PrimaryCtaFontSize)
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() -> bool
				{
					return PreviewedCompanionID.IsNone() || IsCompanionUnlocked(PreviewedCompanionID);
				})),
				ET66CompanionReferenceButtonFamily::CtaPrimary));

	PreviewWidget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				CreateCompanionPreviewWidget(PreviewColor)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0f, -4.0f, 0.0f, 0.0f)
			[
			MakeCompanionReferenceRow(
				SNew(SBox)
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(bDotaTheme ? 280.f : 240.f)
					[
						ConfirmButtonWidget
					]
				],
				FMargin(12.f, 10.f),
				bDotaTheme ? SelectionPanelFill : FSlateColor(FT66Style::Tokens::Panel))
		];

	const TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.BorderBackgroundColor(FLinearColor::Transparent)
		.Padding(0)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				MakePreviewFocusMask()
			]
			+ SOverlay::Slot()
			[
				MakeWorldScrim()
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(ScreenSafePadding)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, TopBarBottomGap)
					[
						SNew(SBox)
						[
							TopBarWidget
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, ContentTopGap, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(SidePanelFill)
						.VAlign(VAlign_Fill)
						.Padding(0.0f, 0.0f, PanelGap, PanelBottomInset)
						[
							LeftPanelWidget
						]
						+ SHorizontalBox::Slot()
						.FillWidth(CenterPanelFill)
						.Padding(PanelGap, 0.0f, PanelGap, PanelBottomInset)
						[
							PreviewWidget
						]
						+ SHorizontalBox::Slot()
						.FillWidth(SidePanelFill)
						.Padding(PanelGap, 0.0f, 0.0f, PanelBottomInset)
						[
							RightPanelWidget
						]
					]
				]
			]
		];
	if (const FSlateBrush* SceneBackgroundBrush = GetCompanionSceneBackgroundBrush())
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(SceneBackgroundBrush)
				.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.42f))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.32f))
			]
			+ SOverlay::Slot()
			[
				Root
			];
	}
	return Root;
}
FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { bShowingLore = !bShowingLore; UpdateCompanionDisplay(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

AT66CompanionPreviewStage* UT66CompanionSelectionScreen::GetCompanionPreviewStage() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	if (AT66PlayerController* PC = T66GetLocalFrontendCompanionPlayerController(const_cast<UT66CompanionSelectionScreen*>(this)))
	{
		PC->EnsureLocalFrontendPreviewScene();
	}
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
		return *It;
	return nullptr;
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::CreateCompanionPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage();

	if (Stage)
	{
		const TWeakObjectPtr<AT66CompanionPreviewStage> WeakStage(Stage);
		// In-world preview: transparent overlay for drag-rotate/zoom.
		// The main viewport renders the companion with full Lumen GI behind the UI.
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateStagePreview)
				.DegreesPerPixel(0.28f)
				.OnRotateYaw(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float DeltaYaw)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewYaw(DeltaYaw);
					}
				}))
				.OnZoom(FT66DragPreviewDeltaDelegate::CreateLambda([WeakStage](const float ZoomDelta)
				{
					if (AT66CompanionPreviewStage* PreviewStage = WeakStage.Get())
					{
						PreviewStage->AddPreviewZoom(ZoomDelta);
						T66PositionCompanionPreviewCamera(PreviewStage);
					}
				}))
			];
	}
	return SAssignNew(CompanionPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::IsDotaTheme() ? FLinearColor::Transparent : FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	FName EffectiveSkin = PreviewedCompanionSkinIDOverride;
	if (EffectiveSkin.IsNone() && !PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				EffectiveSkin = SkinSub->GetEquippedCompanionSkinID(PreviewedCompanionID);
			}
		}
	}
	if (EffectiveSkin.IsNone()) EffectiveSkin = FName(TEXT("Default"));

	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
		if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			Stage->SetPreviewDifficulty(GI->SelectedDifficulty);
		}
		Stage->SetPreviewCompanion(PreviewedCompanionID, EffectiveSkin);
		T66PositionCompanionPreviewCamera(this);
	}
	else if (CompanionPreviewColorBox.IsValid())
	{
		if (FT66Style::IsDotaTheme())
		{
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor::Transparent);
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(Data.PlaceholderColor);
			}
			else
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
			}

			if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
			{
				CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f));
			}
		}
	}

	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.CompanionSelection", "NoCompanionTitle", "No Companion"));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
			{
				UT66LocalizationSubsystem* Loc = GetLocSubsystem();
				CompanionNameWidget->SetText(Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName);
			}
		}
	}

	if (CompanionLoreWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			CompanionLoreWidget->SetText(NSLOCTEXT("T66.CompanionSelection", "NoCompanionLore", "Selecting no companion means you face the tribulation alone."));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
				CompanionLoreWidget->SetText(Data.LoreText);
		}
	}

	// Friendliness / Union UI (profile progression; only meaningful when a companion is selected)
	if (CompanionUnionBox.IsValid())
	{
		const bool bShowUnion = !PreviewedCompanionID.IsNone() && IsCompanionUnlocked(PreviewedCompanionID);
		CompanionUnionBox->SetVisibility(bShowUnion ? EVisibility::Visible : EVisibility::Collapsed);
	}

	CompanionUnionProgress01 = 0.f;
	CompanionUnionStagesCleared = 0;
	if (!PreviewedCompanionID.IsNone())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				CompanionUnionStagesCleared = FMath::Max(0, Ach->GetCompanionUnionStagesCleared(PreviewedCompanionID));
				CompanionUnionProgress01 = FMath::Clamp(Ach->GetCompanionUnionProgress01(PreviewedCompanionID), 0.f, 1.f);

				const int32 Needed = UT66AchievementsSubsystem::UnionTier_HyperStages;
				if (CompanionUnionText.IsValid())
				{
					CompanionUnionText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "FriendlinessStagesFormat", "Stages: {0}/{1}"),
						FText::AsNumber(CompanionUnionStagesCleared),
						FText::AsNumber(Needed)));
				}

				int32 HealingType = 1; // Basic
				if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_HyperStages) HealingType = 4;
				else if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_MediumStages) HealingType = 3;
				else if (CompanionUnionStagesCleared >= UT66AchievementsSubsystem::UnionTier_GoodStages) HealingType = 2;

				if (CompanionUnionHealingText.IsValid())
				{
					CompanionUnionHealingText->SetText(FText::Format(
						NSLOCTEXT("T66.CompanionSelection", "HealingTypeFormat", "Healing Type: {0}"),
						FText::AsNumber(HealingType)));
				}
			}
		}
	}

	RefreshCompanionCarouselPortraits();
}

void UT66CompanionSelectionScreen::RefreshCompanionCarouselPortraits()
{
	TArray<FName> CarouselIDs;
	CarouselIDs.Add(NAME_None);
	CarouselIDs.Append(AllCompanionIDs);
	const int32 NumCarousel = CarouselIDs.Num();
	const int32 CarouselIndex = FMath::Clamp(CurrentCompanionIndex + 1, 0, NumCarousel > 0 ? NumCarousel - 1 : 0);

	CompanionCarouselPortraitBrushes.SetNum(5);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
		}
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		UT66UITexturePoolSubsystem* TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		for (int32 Offset = -2; Offset <= 2; ++Offset)
		{
			const int32 SlotIdx = Offset + 2;
			if (!CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) || !CompanionCarouselPortraitBrushes[SlotIdx].IsValid())
			{
				continue;
			}
			const int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
			const FName CompanionID = NumCarousel > 0 ? CarouselIDs[Idx] : NAME_None;
			TSoftObjectPtr<UTexture2D> PortraitSoft;
			if (!CompanionID.IsNone())
			{
				FCompanionData D;
				if (GI->GetCompanionData(CompanionID, D))
				{
					PortraitSoft = !D.SelectionPortrait.IsNull() ? D.SelectionPortrait : D.Portrait;
				}
			}
			const float BoxSize = (Offset == 0) ? 60.f : 45.f;
			if (PortraitSoft.IsNull() || !TexPool)
			{
				CompanionCarouselPortraitBrushes[SlotIdx]->SetResourceObject(nullptr);
			}
			else
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PortraitSoft, this, CompanionCarouselPortraitBrushes[SlotIdx], FName(TEXT("CompanionCarousel"), SlotIdx + 1), /*bClearWhileLoading*/ false);
			}
			CompanionCarouselPortraitBrushes[SlotIdx]->ImageSize = FVector2D(BoxSize, BoxSize);
		}
	}
}

void UT66CompanionSelectionScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->OnLanguageChanged.AddUniqueDynamic(this, &UT66CompanionSelectionScreen::OnLanguageChanged);
	}
	RefreshCompanionList();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->PrimeHeroSelectionAssetsAsync();
		GI->PrimeHeroSelectionPreviewVisualsAsync();
		if (!GI->SelectedCompanionID.IsNone() && AllCompanionIDs.Contains(GI->SelectedCompanionID))
			PreviewCompanion(GI->SelectedCompanionID);
		else
			SelectNoCompanion();
	}
	T66PositionCompanionPreviewCamera(this);
}

void UT66CompanionSelectionScreen::RefreshScreen_Implementation()
{
	FCompanionData Data;
	bool bIsNoCompanion = PreviewedCompanionID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
}

void UT66CompanionSelectionScreen::RefreshCompanionList()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		AllCompanionIDs = GI->GetAllCompanionIDs();
}

TArray<FCompanionData> UT66CompanionSelectionScreen::GetAllCompanions()
{
	TArray<FCompanionData> Companions;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& ID : AllCompanionIDs)
		{
			FCompanionData Data;
			if (GI->GetCompanionData(ID, Data)) Companions.Add(Data);
		}
	}
	return Companions;
}

bool UT66CompanionSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutData)
{
	if (PreviewedCompanionID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		return GI->GetCompanionData(PreviewedCompanionID, OutData);
	return false;
}

void UT66CompanionSelectionScreen::PreviewCompanion(FName ID)
{
	PreviewedCompanionID = ID;
	PreviewedCompanionSkinIDOverride = NAME_None;
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	RefreshSkinsList();
}

void UT66CompanionSelectionScreen::SelectNoCompanion() { PreviewCompanion(NAME_None); }

void UT66CompanionSelectionScreen::PreviewNextCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex++;
	if (CurrentCompanionIndex >= AllCompanionIDs.Num())
		SelectNoCompanion();
	else
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::PreviewPreviousCompanion()
{
	if (AllCompanionIDs.Num() == 0) return;
	CurrentCompanionIndex--;
	if (CurrentCompanionIndex < -1)
	{
		CurrentCompanionIndex = AllCompanionIDs.Num() - 1;
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
	}
	else if (CurrentCompanionIndex == -1)
		SelectNoCompanion();
	else
		PreviewCompanion(AllCompanionIDs[CurrentCompanionIndex]);
}

void UT66CompanionSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }
void UT66CompanionSelectionScreen::OnCompanionLoreClicked() { if (!PreviewedCompanionID.IsNone()) ShowModal(ET66ScreenType::CompanionLore); }
void UT66CompanionSelectionScreen::OnConfirmCompanionClicked()
{
	// Locked companions cannot be confirmed/selected.
	if (!PreviewedCompanionID.IsNone() && !IsCompanionUnlocked(PreviewedCompanionID))
	{
		return;
	}
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
		GI->PersistRememberedSelectionDefaults();
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(false);
		}

		if (UWorld* World = GetWorld())
		{
			FName EffectiveHeroSkinID = GI->SelectedHeroSkinID;
			if (EffectiveHeroSkinID.IsNone())
			{
				EffectiveHeroSkinID = FName(TEXT("Default"));
			}

			for (TActorIterator<AT66HeroPreviewStage> It(World); It; ++It)
			{
				AT66HeroPreviewStage* HeroStage = *It;
				HeroStage->SetPreviewStageMode(ET66PreviewStageMode::Selection);
				HeroStage->SetPreviewDifficulty(GI->SelectedDifficulty);
				HeroStage->SetPreviewHero(GI->SelectedHeroID, GI->SelectedHeroBodyType, EffectiveHeroSkinID, GI->SelectedCompanionID);
				break;
			}
		}
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	FT66Style::DeferRebuild(this);
}

