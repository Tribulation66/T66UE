// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66FrontendBackButtonWidget.h"

#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 GFrontendBackButtonViewportZOrder = 40;

	struct FBackButtonBrushEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	const FSlateBrush* ResolveBackButtonBrush(FBackButtonBrushEntry& Entry, const TCHAR* RelativePath, const FVector2D& ImageSize)
	{
		if (!Entry.Brush.IsValid())
		{
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->DrawAs = ESlateBrushDrawType::Box;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
			Entry.Brush->ImageSize = ImageSize;
			Entry.Brush->Margin = FMargin(0.16f, 0.28f, 0.16f, 0.28f);
		}

		if (!Entry.Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					TEXT("FrontendBackButtonSprite")))
				{
					Entry.Texture.Reset(Texture);
					break;
				}
			}
		}

		Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
		return Entry.Texture.IsValid() ? Entry.Brush.Get() : FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	const FSlateBrush* GetBackButtonBrush(bool bHovered, bool bPressed)
	{
		static FBackButtonBrushEntry Normal;
		static FBackButtonBrushEntry Hover;
		static FBackButtonBrushEntry Pressed;

		if (bPressed)
		{
			return ResolveBackButtonBrush(
				Pressed,
				TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_toggle_on_pressed.png"),
				FVector2D(187.f, 67.f));
		}
		if (bHovered)
		{
			return ResolveBackButtonBrush(
				Hover,
				TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_toggle_on_hover.png"),
				FVector2D(180.f, 67.f));
		}
		return ResolveBackButtonBrush(
			Normal,
			TEXT("SourceAssets/UI/SettingsReference/SheetSlices/Center/settings_toggle_on_normal.png"),
			FVector2D(180.f, 68.f));
	}

	class ST66FrontendBackPlateButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66FrontendBackPlateButton)
			: _Width(205.f)
			, _Height(62.f)
		{}
			SLATE_ARGUMENT(FText, Text)
			SLATE_ARGUMENT(float, Width)
			SLATE_ARGUMENT(float, Height)
			SLATE_EVENT(FOnClicked, OnClicked)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle.SetNormalPadding(FMargin(0.f));
			ButtonStyle.SetPressedPadding(FMargin(0.f));
			OwnedButtonStyle = ButtonStyle;

			ChildSlot
			[
				SNew(SBox)
				.WidthOverride(InArgs._Width)
				.HeightOverride(InArgs._Height)
				[
					FT66Style::MakeBareButton(
						FT66BareButtonParams(
							InArgs._OnClicked,
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
							.Image(this, &ST66FrontendBackPlateButton::GetCurrentBrush)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(FMargin(16.f, 7.f, 16.f, 6.f))
							[
								SNew(STextBlock)
								.Text(InArgs._Text)
								.Font(FT66Style::Tokens::FontBold(22))
								.ColorAndOpacity(FT66Style::Text())
								.ShadowOffset(FVector2D(1.f, 1.f))
								.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.65f))
								.Justification(ETextJustify::Center)
									]
								]
						)
						.SetButtonStyle(&OwnedButtonStyle)
						.SetPadding(FMargin(0.f)),
						&Button)
				]
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			const bool bPressed = Button.IsValid() && Button->IsPressed();
			const bool bHovered = Button.IsValid() && Button->IsHovered();
			return GetBackButtonBrush(bHovered, bPressed);
		}

		FButtonStyle OwnedButtonStyle;
		TSharedPtr<SButton> Button;
	};
}

UT66FrontendBackButtonWidget::UT66FrontendBackButtonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::None;
	bIsModal = false;
}

TSharedRef<SWidget> UT66FrontendBackButtonWidget::RebuildWidget()
{
	// This overlay positions itself directly against the live viewport and safe frame.
	return BuildSlateUI();
}

UT66LocalizationSubsystem* UT66FrontendBackButtonWidget::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

TSharedRef<SWidget> UT66FrontendBackButtonWidget::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const bool bSettingsScreen = UIManager && UIManager->GetCurrentScreenType() == ET66ScreenType::Settings;
	const float TopPadding = (UIManager ? UIManager->GetFrontendTopBarReservedHeight() : 0.f) + (bSettingsScreen ? 80.f : 12.f);
	const FMargin BackButtonPadding = FT66Style::GetSafePadding(FMargin(bSettingsScreen ? 38.f : 24.f, TopPadding, 0.f, 0.f));

	return SNew(SOverlay)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(BackButtonPadding)
		[
			SNew(ST66FrontendBackPlateButton)
			.Text(BackText)
			.Width(bSettingsScreen ? 205.f : 180.f)
			.Height(bSettingsScreen ? 62.f : 56.f)
			.OnClicked(FOnClicked::CreateUObject(this, &UT66FrontendBackButtonWidget::HandleBackClicked))
		];
}

void UT66FrontendBackButtonWidget::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	FT66Style::DeferRebuild(this, GFrontendBackButtonViewportZOrder);
}

FReply UT66FrontendBackButtonWidget::HandleBackClicked()
{
	if (!UIManager)
	{
		return FReply::Handled();
	}

	if (UIManager->CanGoBack())
	{
		UIManager->GoBack();
	}
	else if (UIManager->GetCurrentScreenType() != ET66ScreenType::MainMenu)
	{
		UIManager->ShowScreenWithoutHistory(ET66ScreenType::MainMenu);
	}

	return FReply::Handled();
}
