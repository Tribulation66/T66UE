// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66CompanionSelectionScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66CompanionPreviewStage.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	class ST66DragRotateCompanionPreview : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66DragRotateCompanionPreview) {}
			SLATE_ARGUMENT(TWeakObjectPtr<AT66CompanionPreviewStage>, Stage)
			SLATE_ARGUMENT(const FSlateBrush*, Brush)
			SLATE_ARGUMENT(float, DegreesPerPixel)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Stage = InArgs._Stage;
			DegreesPerPixel = InArgs._DegreesPerPixel;
			bDragging = false;

			ChildSlot
			[
				SNew(SImage).Image(InArgs._Brush)
			];
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bDragging = true;
				LastPos = MouseEvent.GetScreenSpacePosition();
				return FReply::Handled().CaptureMouse(SharedThis(this));
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				bDragging = false;
				return FReply::Handled().ReleaseMouseCapture();
			}
			return FReply::Unhandled();
		}

		virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (!bDragging)
			{
				return FReply::Unhandled();
			}

			const FVector2D Pos = MouseEvent.GetScreenSpacePosition();
			const FVector2D Delta = Pos - LastPos;
			LastPos = Pos;

			if (Stage.IsValid())
			{
				Stage->AddPreviewYaw(Delta.X * DegreesPerPixel);
			}
			return FReply::Handled();
		}

	private:
		TWeakObjectPtr<AT66CompanionPreviewStage> Stage;
		bool bDragging = false;
		FVector2D LastPos = FVector2D::ZeroVector;
		float DegreesPerPixel = 0.25f;
	};
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

void UT66CompanionSelectionScreen::GeneratePlaceholderSkins()
{
	PlaceholderSkins.Empty();

	FSkinData DefaultSkin;
	DefaultSkin.SkinID = FName(TEXT("Default"));
	DefaultSkin.DisplayName = FText::FromString(TEXT("Default"));
	DefaultSkin.bIsDefault = true;
	DefaultSkin.bIsOwned = true;
	DefaultSkin.bIsEquipped = true;
	DefaultSkin.CoinCost = 0;
	PlaceholderSkins.Add(DefaultSkin);

	FSkinData AngelicSkin;
	AngelicSkin.SkinID = FName(TEXT("Angelic"));
	AngelicSkin.DisplayName = FText::FromString(TEXT("Angelic Grace"));
	AngelicSkin.bIsDefault = false;
	AngelicSkin.bIsOwned = true;
	AngelicSkin.bIsEquipped = false;
	AngelicSkin.CoinCost = 3000;
	PlaceholderSkins.Add(AngelicSkin);

	FSkinData DemonicSkin;
	DemonicSkin.SkinID = FName(TEXT("Demonic"));
	DemonicSkin.DisplayName = FText::FromString(TEXT("True Form"));
	DemonicSkin.bIsDefault = false;
	DemonicSkin.bIsOwned = false;
	DemonicSkin.bIsEquipped = false;
	DemonicSkin.CoinCost = 6000;
	PlaceholderSkins.Add(DemonicSkin);
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	GeneratePlaceholderSkins();

	// Get localized text
	FText TitleText = Loc ? Loc->GetText_SelectCompanion() : FText::FromString(TEXT("SELECT COMPANION"));
	FText CompanionGridText = Loc ? Loc->GetText_CompanionGrid() : FText::FromString(TEXT("COMPANION GRID"));
	FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : FText::FromString(TEXT("NO COMPANION"));
	FText SkinsText = Loc ? Loc->GetText_Skins() : FText::FromString(TEXT("SKINS"));
	FText LoreText = Loc ? Loc->GetText_Lore() : FText::FromString(TEXT("LORE"));
	FText ConfirmText = Loc ? Loc->GetText_ConfirmCompanion() : FText::FromString(TEXT("CONFIRM COMPANION"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));
	FText BuyText = Loc ? Loc->GetText_Buy() : FText::FromString(TEXT("BUY"));
	FText EquipText = Loc ? Loc->GetText_Equip() : FText::FromString(TEXT("EQUIP"));

	// Build skins list
	TSharedRef<SVerticalBox> SkinsListBox = SNew(SVerticalBox);
	for (const FSkinData& Skin : PlaceholderSkins)
	{
		FLinearColor RowBg = Skin.bIsEquipped 
			? FLinearColor(0.2f, 0.4f, 0.3f, 1.0f) 
			: FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);

		SkinsListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 3.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowBg)
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Skin.DisplayName)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(78.0f).HeightOverride(28.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.ButtonColorAndOpacity(Skin.bIsOwned ? FLinearColor(0.2f, 0.5f, 0.3f, 1.0f) : FLinearColor(0.5f, 0.4f, 0.1f, 1.0f))
						[
							SNew(STextBlock)
							.Text(Skin.bIsOwned ? EquipText : BuyText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
			]
		];
	}

	// Carousel list: [NO COMPANION] + companions
	UT66GameInstance* GICheck = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	TArray<FName> CarouselIDs;
	CarouselIDs.Add(NAME_None);
	CarouselIDs.Append(AllCompanionIDs);
	const int32 NumCarousel = CarouselIDs.Num();
	const int32 CarouselIndex = FMath::Clamp(CurrentCompanionIndex + 1, 0, NumCarousel > 0 ? NumCarousel - 1 : 0);

	// Build carousel (5 colored tiles: prev2, prev1, current, next1, next2)
	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	for (int32 Offset = -2; Offset <= 2; Offset++)
	{
		int32 Idx = NumCarousel > 0 ? (CarouselIndex + Offset + NumCarousel * 2) % NumCarousel : 0;
		FName CompanionID = NumCarousel > 0 ? CarouselIDs[Idx] : NAME_None;
		FCompanionData Data;
		FLinearColor SpriteColor = FLinearColor(0.35f, 0.25f, 0.25f, 1.0f);
		if (!CompanionID.IsNone() && GICheck && GICheck->GetCompanionData(CompanionID, Data))
		{
			SpriteColor = Data.PlaceholderColor;
		}
		float BoxSize = (Offset == 0) ? 60.0f : 45.0f;
		float Opacity = (Offset == 0) ? 1.0f : 0.6f;
		CompanionCarousel->AddSlot()
			.AutoWidth()
			.Padding(4.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(BoxSize).HeightOverride(BoxSize)
				[
					SNew(SBorder)
					.BorderBackgroundColor(SpriteColor * Opacity)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				]
			];
	}

	// Current display name/lore for right panel
	FText CurrentCompanionName = NoCompanionText;
	FText CurrentCompanionLore = FText::FromString(TEXT("Selecting no companion means you face the tribulation alone."));
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
		{
			CurrentCompanionName = Loc ? Loc->GetCompanionDisplayName(Data) : Data.DisplayName;
			CurrentCompanionLore = Data.LoreText;
		}
	}

	// Preview color for center (fallback when no 3D stage)
	FLinearColor PreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
	if (!PreviewedCompanionID.IsNone())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
			PreviewColor = Data.PlaceholderColor;
	}

	const FButtonStyle& BtnNeutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	const FButtonStyle& BtnPrimary = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Primary");
	const FTextBlockStyle& TxtHeading = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const FTextBlockStyle& TxtChip = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

	return SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Bg"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// === TOP BAR: Companion Grid Button + Carousel (colored tiles) ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(20.0f, 15.0f, 20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 20.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(160.0f).HeightOverride(40.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleCompanionGridClicked))
							.ButtonStyle(&BtnNeutral)
							.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(12.f, 8.f))
							[
								SNew(STextBlock).Text(CompanionGridText)
								.TextStyle(&TxtChip)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(40.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandlePrevClicked))
								.ButtonStyle(&BtnNeutral)
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								[
									SNew(STextBlock).Text(FText::FromString(TEXT("<")))
									.Font(FT66Style::Tokens::FontBold(20))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							CompanionCarousel
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f)
						[
							SNew(SBox).WidthOverride(40.0f).HeightOverride(40.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center).VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNextClicked))
								.ButtonStyle(&BtnNeutral)
								.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
								[
									SNew(STextBlock).Text(FText::FromString(TEXT(">")))
									.Font(FT66Style::Tokens::FontBold(20))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(20.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBox).MinDesiredWidth(160.0f).HeightOverride(40.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center).VAlign(VAlign_Center)
							.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleNoCompanionClicked))
							.ButtonStyle(&BtnNeutral)
							.ButtonColorAndOpacity(PreviewedCompanionID.IsNone() ? FT66Style::Tokens::Accent : FT66Style::Tokens::Panel2)
							.ContentPadding(FMargin(12.f, 8.f))
							[
								SNew(STextBlock).Text(NoCompanionText)
								.TextStyle(&TxtChip)
							]
						]
					]
				]
				// === MAIN CONTENT: Left 0.28 | Center 0.44 | Right 0.28 (bigger panels, different colors) ===
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(20.0f, 10.0f)
				[
					SNew(SHorizontalBox)
					// LEFT PANEL: Skins (greenish)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space3))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(STextBlock).Text(SkinsText)
								.TextStyle(&TxtHeading)
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SkinsListBox
								]
							]
						]
					]
					// CENTER: Companion Preview (3D or colored box)
					+ SHorizontalBox::Slot()
					.FillWidth(0.44f)
					.Padding(10.0f, 0.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
						[
						SNew(SBox)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							CreateCompanionPreviewWidget(PreviewColor)
						]
						]
					]
					// RIGHT PANEL: Companion Info (blueish, name + LORE same row, bigger medals)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
						.Padding(FMargin(FT66Style::Tokens::Space4))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 8.0f)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("COMPANION INFO")))
								.TextStyle(&TxtHeading)
							]
							// Name + LORE button same row
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.VAlign(VAlign_Center)
								[
									SAssignNew(CompanionNameWidget, STextBlock)
									.Text(CurrentCompanionName)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
									.ColorAndOpacity(FLinearColor::White)
									.AutoWrapText(true)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(8.0f, 0.0f, 0.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(80.0f).HeightOverride(32.0f)
									[
										SNew(SButton)
										.HAlign(HAlign_Center).VAlign(VAlign_Center)
										.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleLoreClicked))
										.ButtonStyle(&BtnNeutral)
										.ButtonColorAndOpacity(FT66Style::Tokens::Panel2)
										.IsEnabled(!PreviewedCompanionID.IsNone())
										[
											SNew(STextBlock).Text(LoreText)
											.TextStyle(&TxtChip)
										]
									]
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SAssignNew(CompanionLoreWidget, STextBlock)
								.Text(CurrentCompanionLore)
								.Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
								.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(SBorder)
								.BorderBackgroundColor(FLinearColor(0.1f, 0.15f, 0.1f, 1.0f))
								.Padding(FMargin(10.0f))
								[
									SNew(STextBlock)
									.Text(FText::FromString(TEXT("Passive: Heals the player during combat")))
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
									.ColorAndOpacity(FLinearColor(0.6f, 0.9f, 0.6f, 1.0f))
									.AutoWrapText(true)
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.Padding(0.0f, 0.0f, 0.0f, 10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.2f, 0.5f, 0.2f, 1.0f))
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.4f, 0.1f, 1.0f))
									]
								]
								+ SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
								[
									SNew(SBox).WidthOverride(52.0f).HeightOverride(52.0f)
									[
										SNew(SBorder).BorderBackgroundColor(FLinearColor(0.5f, 0.15f, 0.15f, 1.0f))
									]
								]
							]
							+ SVerticalBox::Slot().FillHeight(1.0f)
						]
					]
				]
				// === BOTTOM BAR: Confirm (wider so text not cut off) ===
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(20.0f, 10.0f, 20.0f, 20.0f)
				[
					SNew(SBox).WidthOverride(320.0f).HeightOverride(55.0f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center).VAlign(VAlign_Center)
						.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleConfirmClicked))
						.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.2f, 1.0f))
						[
							SNew(STextBlock).Text(ConfirmText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
			]
			// Back button (bottom-left overlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(SBox).WidthOverride(100.0f).HeightOverride(40.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center).VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66CompanionSelectionScreen::HandleBackClicked))
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					[
						SNew(STextBlock).Text(BackText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

FReply UT66CompanionSelectionScreen::HandlePrevClicked() { PreviewPreviousCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNextClicked() { PreviewNextCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleNoCompanionClicked() { SelectNoCompanion(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleLoreClicked() { OnCompanionLoreClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleConfirmClicked() { OnConfirmCompanionClicked(); return FReply::Handled(); }
FReply UT66CompanionSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

AT66CompanionPreviewStage* UT66CompanionSelectionScreen::GetCompanionPreviewStage() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	for (TActorIterator<AT66CompanionPreviewStage> It(World); It; ++It)
		return *It;
	return nullptr;
}

TSharedRef<SWidget> UT66CompanionSelectionScreen::CreateCompanionPreviewWidget(const FLinearColor& FallbackColor)
{
	AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage();
	UTextureRenderTarget2D* RenderTarget = Stage ? Stage->GetRenderTarget() : nullptr;
	if (RenderTarget)
	{
		CompanionPreviewBrush = MakeShared<FSlateBrush>();
		CompanionPreviewBrush->SetResourceObject(RenderTarget);
		CompanionPreviewBrush->ImageSize = FVector2D(512.f, 720.f);
		CompanionPreviewBrush->DrawAs = ESlateBrushDrawType::Image;
		CompanionPreviewBrush->Tiling = ESlateBrushTileType::NoTile;
		return SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(ST66DragRotateCompanionPreview)
				.Stage(Stage)
				.Brush(CompanionPreviewBrush.Get())
				.DegreesPerPixel(0.28f)
			];
	}
	return SAssignNew(CompanionPreviewColorBox, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FallbackColor)
		[
			SNew(SBox)
		];
}

void UT66CompanionSelectionScreen::UpdateCompanionDisplay()
{
	if (AT66CompanionPreviewStage* Stage = GetCompanionPreviewStage())
	{
		Stage->SetPreviewCompanion(PreviewedCompanionID);
	}
	else if (CompanionPreviewColorBox.IsValid())
	{
		FCompanionData Data;
		if (GetPreviewedCompanionData(Data))
			CompanionPreviewColorBox->SetBorderBackgroundColor(Data.PlaceholderColor);
		else
			CompanionPreviewColorBox->SetBorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
	}

	if (CompanionNameWidget.IsValid())
	{
		if (PreviewedCompanionID.IsNone())
		{
			UT66LocalizationSubsystem* Loc = GetLocSubsystem();
			CompanionNameWidget->SetText(Loc ? Loc->GetText_NoCompanion() : FText::FromString(TEXT("No Companion")));
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
			CompanionLoreWidget->SetText(FText::FromString(TEXT("Selecting no companion means you face the tribulation alone.")));
		}
		else
		{
			FCompanionData Data;
			if (GetPreviewedCompanionData(Data))
				CompanionLoreWidget->SetText(Data.LoreText);
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
		if (!GI->SelectedCompanionID.IsNone())
			PreviewCompanion(GI->SelectedCompanionID);
		else
			SelectNoCompanion();
	}
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
	CurrentCompanionIndex = ID.IsNone() ? -1 : AllCompanionIDs.IndexOfByKey(ID);
	if (CurrentCompanionIndex == INDEX_NONE) CurrentCompanionIndex = -1;
	FCompanionData Data;
	bool bIsNoCompanion = ID.IsNone();
	if (!bIsNoCompanion) GetPreviewedCompanionData(Data);
	OnPreviewedCompanionChanged(Data, bIsNoCompanion);
	UpdateCompanionDisplay();
	// Rebuild UI to update button states
	TakeWidget();
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
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedCompanionID = PreviewedCompanionID;
	}
	NavigateBack();
}
void UT66CompanionSelectionScreen::OnBackClicked() { NavigateBack(); }

void UT66CompanionSelectionScreen::OnLanguageChanged(ET66Language NewLanguage)
{
	TakeWidget();
}
