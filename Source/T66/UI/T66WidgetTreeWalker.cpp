// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WidgetTreeWalker.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66FlatWidgetMetadata.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float NormalizedReferenceWidth = 1920.f;
	constexpr float NormalizedReferenceHeight = 1080.f;

	FString VisibilityToString(const EVisibility Visibility)
	{
		if (Visibility == EVisibility::Visible)
		{
			return TEXT("Visible");
		}
		if (Visibility == EVisibility::Collapsed)
		{
			return TEXT("Collapsed");
		}
		if (Visibility == EVisibility::Hidden)
		{
			return TEXT("Hidden");
		}
		if (Visibility == EVisibility::HitTestInvisible)
		{
			return TEXT("HitTestInvisible");
		}
		if (Visibility == EVisibility::SelfHitTestInvisible)
		{
			return TEXT("SelfHitTestInvisible");
		}
		return TEXT("Unknown");
	}

	FString FlatStateToString(const ET66FlatState State)
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

	FString BrushDrawTypeToString(const ESlateBrushDrawType::Type DrawType)
	{
		switch (DrawType)
		{
		case ESlateBrushDrawType::NoDrawType:
			return TEXT("NoDraw");
		case ESlateBrushDrawType::Box:
			return TEXT("Box");
		case ESlateBrushDrawType::Border:
			return TEXT("Border");
		case ESlateBrushDrawType::Image:
			return TEXT("Image");
		case ESlateBrushDrawType::RoundedBox:
			return TEXT("RoundedBox");
		default:
			return TEXT("Unknown");
		}
	}

	FString BrushResourceName(const FSlateBrush* Brush)
	{
		if (!Brush)
		{
			return FString();
		}

		if (UObject* ResourceObject = Brush->GetResourceObject())
		{
			return ResourceObject->GetPathName();
		}

		const FName ResourceName = Brush->GetResourceName();
		return ResourceName.IsNone() ? FString() : ResourceName.ToString();
	}

	FString ColorToHex(const FLinearColor& Color)
	{
		const FColor SRGB = Color.ToFColorSRGB();
		return FString::Printf(TEXT("#%02X%02X%02X%02X"), SRGB.R, SRGB.G, SRGB.B, SRGB.A);
	}

	TSharedRef<FJsonObject> MakeColorObject(const FLinearColor& Color)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("r"), Color.R);
		Object->SetNumberField(TEXT("g"), Color.G);
		Object->SetNumberField(TEXT("b"), Color.B);
		Object->SetNumberField(TEXT("a"), Color.A);
		Object->SetStringField(TEXT("hex"), ColorToHex(Color));
		return Object;
	}

	TSharedRef<FJsonObject> MakeMarginObject(const FMargin& Margin)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("left"), Margin.Left);
		Object->SetNumberField(TEXT("top"), Margin.Top);
		Object->SetNumberField(TEXT("right"), Margin.Right);
		Object->SetNumberField(TEXT("bottom"), Margin.Bottom);
		return Object;
	}

	TSharedRef<FJsonObject> MakeVectorObject(const FVector2D& Vector)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("x"), Vector.X);
		Object->SetNumberField(TEXT("y"), Vector.Y);
		return Object;
	}

	TSharedRef<FJsonObject> MakeBrushObject(const FSlateBrush* Brush)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("resource"), BrushResourceName(Brush));
		Object->SetStringField(TEXT("draw_type"), Brush ? BrushDrawTypeToString(Brush->DrawAs) : FString());
		Object->SetObjectField(TEXT("tint"), Brush ? MakeColorObject(Brush->TintColor.GetSpecifiedColor()) : MakeColorObject(FLinearColor::Transparent));
		Object->SetObjectField(TEXT("margin"), Brush ? MakeMarginObject(Brush->Margin) : MakeMarginObject(FMargin()));
		return Object;
	}

	TSharedPtr<SBorder> FindFirstBorderWidget(const TSharedRef<SWidget>& Widget, const bool bIncludeSelf)
	{
		const FString WidgetType = Widget->GetTypeAsString();
		if (bIncludeSelf && WidgetType == TEXT("SBorder"))
		{
			return StaticCastSharedRef<SBorder>(Widget);
		}

		FChildren* Children = Widget->GetChildren();
		if (!Children)
		{
			return nullptr;
		}

		const int32 NumChildren = Children->Num();
		for (int32 Index = 0; Index < NumChildren; ++Index)
		{
			const TSharedRef<SWidget> Child = Children->GetChildAt(Index);
			if (TSharedPtr<SBorder> Result = FindFirstBorderWidget(Child, true))
			{
				return Result;
			}
		}

		return nullptr;
	}

	TSharedPtr<STextBlock> FindFirstTextBlockWidget(const TSharedRef<SWidget>& Widget, const bool bIncludeSelf)
	{
		if (bIncludeSelf && Widget->GetTypeAsString() == TEXT("STextBlock"))
		{
			return StaticCastSharedRef<STextBlock>(Widget);
		}

		FChildren* Children = Widget->GetChildren();
		if (!Children)
		{
			return nullptr;
		}

		const int32 NumChildren = Children->Num();
		for (int32 Index = 0; Index < NumChildren; ++Index)
		{
			const TSharedRef<SWidget> Child = Children->GetChildAt(Index);
			if (TSharedPtr<STextBlock> Result = FindFirstTextBlockWidget(Child, true))
			{
				return Result;
			}
		}

		return nullptr;
	}

	FVector2D SafeViewportSize(const FVector2D& Requested, const TSharedRef<SWidget>& RootWidget)
	{
		if (Requested.X > 1.f && Requested.Y > 1.f)
		{
			return Requested;
		}

		const FVector2D RootSize = RootWidget->GetTickSpaceGeometry().GetLocalSize();
		if (RootSize.X > 1.f && RootSize.Y > 1.f)
		{
			return RootSize;
		}

		return FVector2D(1920.f, 1080.f);
	}

	struct FWalkContext
	{
		TArray<TSharedPtr<FJsonValue>> Widgets;
		FVector2D ViewportSize = FVector2D(1920.f, 1080.f);
		FVector2D RootOrigin = FVector2D::ZeroVector;
	};

	int32 WalkWidget(
		const TSharedRef<SWidget>& Widget,
		FWalkContext& Context,
		const int32 ParentIndex,
		const FString& ParentTag,
		const int32 ChildIndex,
		const int32 ChildCount)
	{
		const int32 WidgetIndex = Context.Widgets.Num();
		const FName SlateTag = Widget->GetTag();
		const TSharedPtr<FT66FlatWidgetMetadata> T66Metadata = Widget->GetMetaData<FT66FlatWidgetMetadata>();
		const FName MetadataTag = T66Metadata.IsValid() ? T66Metadata->Tag : NAME_None;
		const FName EffectiveTag = !SlateTag.IsNone() ? SlateTag : MetadataTag;
		const FString TagString = EffectiveTag.IsNone() ? FString() : EffectiveTag.ToString();

		const FGeometry& Geometry = Widget->GetTickSpaceGeometry();
		const FVector2D AbsolutePosition = Geometry.GetAbsolutePosition() - Context.RootOrigin;
		const FVector2D AbsoluteSize = Geometry.GetAbsoluteSize();
		const FVector2D DesiredSize = Widget->GetDesiredSize();
		const float ViewW = FMath::Max(1.f, Context.ViewportSize.X);
		const float ViewH = FMath::Max(1.f, Context.ViewportSize.Y);
		const float ReferenceX = AbsolutePosition.X * (NormalizedReferenceWidth / ViewW);
		const float ReferenceY = AbsolutePosition.Y * (NormalizedReferenceHeight / ViewH);
		const float ReferenceW = AbsoluteSize.X * (NormalizedReferenceWidth / ViewW);
		const float ReferenceH = AbsoluteSize.Y * (NormalizedReferenceHeight / ViewH);

		TSharedRef<FJsonObject> Record = MakeShared<FJsonObject>();
		Record->SetNumberField(TEXT("index"), WidgetIndex);
		Record->SetStringField(TEXT("tag"), TagString);
		Record->SetBoolField(TEXT("untagged"), TagString.IsEmpty());
		Record->SetStringField(TEXT("type"), Widget->GetTypeAsString());
		Record->SetStringField(TEXT("source"), Widget->GetReadableLocation());
		Record->SetStringField(TEXT("parent"), ParentTag);
		Record->SetStringField(TEXT("parent_tag"), ParentTag);
		Record->SetNumberField(TEXT("parent_index"), ParentIndex);
		Record->SetNumberField(TEXT("child_index"), ChildIndex);
		Record->SetNumberField(TEXT("child_count"), ChildCount);
		Record->SetBoolField(TEXT("is_label"), T66Metadata.IsValid() ? T66Metadata->bIsLabel : false);

		TSharedRef<FJsonObject> GeometryObject = MakeShared<FJsonObject>();
		GeometryObject->SetNumberField(TEXT("absolute_x"), AbsolutePosition.X);
		GeometryObject->SetNumberField(TEXT("absolute_y"), AbsolutePosition.Y);
		GeometryObject->SetNumberField(TEXT("width"), AbsoluteSize.X);
		GeometryObject->SetNumberField(TEXT("height"), AbsoluteSize.Y);
		GeometryObject->SetObjectField(TEXT("desired_size"), MakeVectorObject(DesiredSize));
		GeometryObject->SetNumberField(TEXT("layer_id"), Widget->GetPersistentState().LayerId);
		GeometryObject->SetStringField(TEXT("visibility"), VisibilityToString(Widget->GetVisibility()));
		GeometryObject->SetBoolField(TEXT("enabled"), Widget->IsEnabled());
		GeometryObject->SetBoolField(TEXT("hovered"), Widget->IsHovered());
		GeometryObject->SetBoolField(TEXT("pressed"), false);

		TSharedRef<FJsonObject> NormalizedObject = MakeShared<FJsonObject>();
		NormalizedObject->SetNumberField(TEXT("x"), ReferenceX / NormalizedReferenceWidth);
		NormalizedObject->SetNumberField(TEXT("y"), ReferenceY / NormalizedReferenceHeight);
		NormalizedObject->SetNumberField(TEXT("w"), ReferenceW / NormalizedReferenceWidth);
		NormalizedObject->SetNumberField(TEXT("h"), ReferenceH / NormalizedReferenceHeight);
		GeometryObject->SetObjectField(TEXT("normalized"), NormalizedObject);

		TSharedRef<FJsonObject> ReferenceRectObject = MakeShared<FJsonObject>();
		ReferenceRectObject->SetNumberField(TEXT("x"), ReferenceX);
		ReferenceRectObject->SetNumberField(TEXT("y"), ReferenceY);
		ReferenceRectObject->SetNumberField(TEXT("width"), ReferenceW);
		ReferenceRectObject->SetNumberField(TEXT("height"), ReferenceH);
		GeometryObject->SetObjectField(TEXT("reference_1920x1080"), ReferenceRectObject);
		Record->SetObjectField(TEXT("geometry"), GeometryObject);

		TSharedRef<FJsonObject> BorderObject = MakeShared<FJsonObject>();
		BorderObject->SetStringField(TEXT("brush_resource"), FString());
		BorderObject->SetStringField(TEXT("draw_type"), FString());
		BorderObject->SetObjectField(TEXT("tint"), MakeColorObject(FLinearColor::Transparent));
		BorderObject->SetObjectField(TEXT("background_color"), MakeColorObject(FLinearColor::Transparent));
		BorderObject->SetObjectField(TEXT("margin"), MakeMarginObject(FMargin()));

		const FString WidgetType = Widget->GetTypeAsString();
		if (const TSharedPtr<SBorder> BorderWidget = FindFirstBorderWidget(Widget, WidgetType != TEXT("SButton")))
		{
			const FSlateBrush* BorderBrush = BorderWidget->GetBorderImage();
			const FLinearColor BackgroundColor = BorderWidget->GetBorderBackgroundColor().GetSpecifiedColor();
			BorderObject->SetStringField(TEXT("brush_resource"), BrushResourceName(BorderBrush));
			BorderObject->SetStringField(TEXT("draw_type"), BorderBrush ? BrushDrawTypeToString(BorderBrush->DrawAs) : FString());
			BorderObject->SetObjectField(TEXT("tint"), BorderBrush ? MakeColorObject(BorderBrush->TintColor.GetSpecifiedColor()) : MakeColorObject(FLinearColor::Transparent));
			BorderObject->SetObjectField(TEXT("background_color"), MakeColorObject(BackgroundColor));
			BorderObject->SetObjectField(TEXT("margin"), BorderBrush ? MakeMarginObject(BorderBrush->Margin) : MakeMarginObject(FMargin()));
		}
		if (T66Metadata.IsValid() && T66Metadata->BorderColor.IsSet())
		{
			const FLinearColor AuthoritativeBorderColor = T66Metadata->BorderColor.GetValue();
			BorderObject->SetStringField(TEXT("brush_resource"), TEXT("(flat)"));
			BorderObject->SetStringField(TEXT("draw_type"), TEXT("Box"));
			BorderObject->SetObjectField(TEXT("tint"), MakeColorObject(AuthoritativeBorderColor));
			BorderObject->SetObjectField(TEXT("background_color"), MakeColorObject(AuthoritativeBorderColor));
			BorderObject->SetObjectField(TEXT("margin"), MakeMarginObject(FMargin(2.f)));
		}
		Record->SetObjectField(TEXT("border"), BorderObject);

		TSharedRef<FJsonObject> TextObject = MakeShared<FJsonObject>();
		TextObject->SetStringField(TEXT("content"), FString());
		TextObject->SetStringField(TEXT("font_path"), FString());
		TextObject->SetNumberField(TEXT("size"), 0);
		TextObject->SetObjectField(TEXT("color"), MakeColorObject(FLinearColor::Transparent));
		TextObject->SetNumberField(TEXT("letter_spacing"), 0);
		TextObject->SetObjectField(TEXT("shadow"), MakeColorObject(FLinearColor::Transparent));
		if (const TSharedPtr<STextBlock> TextWidget = FindFirstTextBlockWidget(Widget, true))
		{
			const FSlateFontInfo Font = TextWidget->GetFont();
			FString FontPath;
			if (Font.FontObject)
			{
				FontPath = Font.FontObject->GetPathName();
			}
			else if (!Font.TypefaceFontName.IsNone())
			{
				FontPath = Font.TypefaceFontName.ToString();
			}
			else
			{
				FontPath = TEXT("CompositeFont");
			}

			TextObject->SetStringField(TEXT("content"), TextWidget->GetText().ToString());
			TextObject->SetStringField(TEXT("font_path"), FontPath);
			TextObject->SetNumberField(TEXT("size"), Font.Size);
			TextObject->SetObjectField(TEXT("color"), MakeColorObject(TextWidget->GetColorAndOpacity().GetSpecifiedColor()));
			TextObject->SetNumberField(TEXT("letter_spacing"), Font.LetterSpacing);
			TextObject->SetObjectField(TEXT("shadow"), MakeColorObject(TextWidget->GetShadowColorAndOpacity()));
			TextObject->SetObjectField(TEXT("shadow_offset"), MakeVectorObject(FVector2D(TextWidget->GetShadowOffset())));
		}
		Record->SetObjectField(TEXT("text"), TextObject);

		TSharedRef<FJsonObject> ButtonStateObject = MakeShared<FJsonObject>();
		ButtonStateObject->SetStringField(TEXT("state"), T66Metadata.IsValid() ? FlatStateToString(T66Metadata->IntendedState) : FString());
		if (WidgetType == TEXT("SButton"))
		{
			const TSharedRef<SButton> ButtonWidget = StaticCastSharedRef<SButton>(Widget);
			GeometryObject->SetBoolField(TEXT("pressed"), ButtonWidget->IsPressed());
			ButtonStateObject->SetBoolField(TEXT("pressed"), ButtonWidget->IsPressed());
		}
		else
		{
			ButtonStateObject->SetBoolField(TEXT("pressed"), false);
		}
		Record->SetObjectField(TEXT("button_state"), ButtonStateObject);

		const bool bReportedHoverCapable = T66Metadata.IsValid() && T66Metadata->bHoverCapable && Widget->IsEnabled();
		TSharedRef<FJsonObject> InteractivityObject = MakeShared<FJsonObject>();
		InteractivityObject->SetBoolField(TEXT("has_click_handler"), T66Metadata.IsValid() ? T66Metadata->bHasClickHandler : false);
		InteractivityObject->SetBoolField(TEXT("hover_capable"), bReportedHoverCapable);
		InteractivityObject->SetStringField(TEXT("toggle_group"), T66Metadata.IsValid() && !T66Metadata->ToggleGroup.IsNone() ? T66Metadata->ToggleGroup.ToString() : FString());
		Record->SetObjectField(TEXT("interactivity"), InteractivityObject);

		TSharedRef<FJsonObject> ImageObject = MakeShared<FJsonObject>();
		ImageObject->SetObjectField(TEXT("rect"), NormalizedObject);
		ImageObject->SetObjectField(TEXT("desired_size"), MakeVectorObject(DesiredSize));
		ImageObject->SetStringField(TEXT("brush_resource"), FString());
		ImageObject->SetObjectField(TEXT("tint"), MakeColorObject(FLinearColor::White));
		Record->SetObjectField(TEXT("image"), ImageObject);

		TSharedRef<FJsonObject> MetadataObject = MakeShared<FJsonObject>();
		MetadataObject->SetStringField(TEXT("tag"), TagString);
		MetadataObject->SetStringField(TEXT("intended_role"), T66Metadata.IsValid() ? T66Metadata->IntendedRole : FString());
		MetadataObject->SetStringField(TEXT("intended_state"), T66Metadata.IsValid() ? FlatStateToString(T66Metadata->IntendedState) : FString());
		MetadataObject->SetBoolField(TEXT("has_click_handler"), T66Metadata.IsValid() ? T66Metadata->bHasClickHandler : false);
		MetadataObject->SetBoolField(TEXT("hover_capable"), bReportedHoverCapable);
		MetadataObject->SetStringField(TEXT("toggle_group"), T66Metadata.IsValid() && !T66Metadata->ToggleGroup.IsNone() ? T66Metadata->ToggleGroup.ToString() : FString());
		MetadataObject->SetBoolField(TEXT("is_label"), T66Metadata.IsValid() ? T66Metadata->bIsLabel : false);
		if (T66Metadata.IsValid() && T66Metadata->BorderColor.IsSet())
		{
			MetadataObject->SetObjectField(TEXT("border_color"), MakeColorObject(T66Metadata->BorderColor.GetValue()));
		}
		Record->SetObjectField(TEXT("t66_metadata"), MetadataObject);

		Context.Widgets.Add(MakeShared<FJsonValueObject>(Record));

		FChildren* Children = Widget->GetAllChildren();
		const int32 NumChildren = Children ? Children->Num() : 0;
		for (int32 Index = 0; Index < NumChildren; ++Index)
		{
			WalkWidget(Children->GetChildAt(Index), Context, WidgetIndex, TagString, Index, NumChildren);
		}

		return WidgetIndex;
	}
}

bool FT66WidgetTreeWalker::DumpWidgetTreeToJson(
	const TSharedRef<SWidget>& RootWidget,
	const FString& ScreenName,
	const FVector2D& ViewportSize,
	const FString& OutputPath,
	FString& OutError)
{
	FWalkContext Context;
	Context.ViewportSize = SafeViewportSize(ViewportSize, RootWidget);
	Context.RootOrigin = RootWidget->GetTickSpaceGeometry().GetAbsolutePosition();
	WalkWidget(RootWidget, Context, INDEX_NONE, FString(), 0, 1);

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("screen"), ScreenName);
	Root->SetStringField(TEXT("capture_timestamp"), FDateTime::UtcNow().ToIso8601());

	TSharedRef<FJsonObject> ViewportObject = MakeShared<FJsonObject>();
	ViewportObject->SetNumberField(TEXT("width"), Context.ViewportSize.X);
	ViewportObject->SetNumberField(TEXT("height"), Context.ViewportSize.Y);
	ViewportObject->SetStringField(TEXT("normalized_basis"), TEXT("1920x1080"));
	ViewportObject->SetNumberField(TEXT("normalized_basis_width"), NormalizedReferenceWidth);
	ViewportObject->SetNumberField(TEXT("normalized_basis_height"), NormalizedReferenceHeight);
	Root->SetObjectField(TEXT("viewport"), ViewportObject);
	Root->SetArrayField(TEXT("widgets"), Context.Widgets);

	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		OutError = TEXT("Failed to serialize widget dump JSON.");
		return false;
	}

	const FString AbsoluteOutputPath = FPaths::ConvertRelativePathToFull(OutputPath);
	const FString Directory = FPaths::GetPath(AbsoluteOutputPath);
	if (!Directory.IsEmpty())
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	if (!FFileHelper::SaveStringToFile(JsonText, *AbsoluteOutputPath))
	{
		OutError = FString::Printf(TEXT("Failed to write widget dump: %s"), *AbsoluteOutputPath);
		return false;
	}

	return true;
}
