// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WidgetDumpTargets.h"

#include "Blueprint/GameViewportSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "UI/T66WidgetTreeWalker.h"
#include "UI/Style/T66FlatWidgetMetadata.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/SWidget.h"

namespace
{
	struct FT66ViewportWidgetCandidate
	{
		UUserWidget* Widget = nullptr;
		int32 ZOrder = 0;
		FString Name;
	};

	struct FT66ResolvedDumpTarget
	{
		TSharedPtr<SWidget> RootWidget;
		FString DumpName;
	};

	FString StripOptionalTargetPrefix(FString TargetSpec)
	{
		TargetSpec = TargetSpec.TrimStartAndEnd().TrimQuotes();

		FString Key;
		FString Value;
		if (TargetSpec.Split(TEXT("="), &Key, &Value)
			&& Key.Equals(TEXT("Target"), ESearchCase::IgnoreCase))
		{
			return Value.TrimStartAndEnd().TrimQuotes();
		}

		return TargetSpec;
	}

	bool ClassOrSuperNameMatches(const UClass* Class, const FString& RequestedClassName)
	{
		const FString Requested = RequestedClassName.TrimStartAndEnd();
		if (Requested.IsEmpty())
		{
			return false;
		}
		const FString RequestedWithoutCppPrefix =
			(Requested.Len() > 1 && (Requested[0] == TCHAR('U') || Requested[0] == TCHAR('A')))
				? Requested.Mid(1)
				: Requested;

		for (const UClass* WalkClass = Class; WalkClass; WalkClass = WalkClass->GetSuperClass())
		{
			const FString ClassName = WalkClass->GetName();
			if (ClassName.Equals(Requested, ESearchCase::IgnoreCase)
				|| ClassName.Equals(RequestedWithoutCppPrefix, ESearchCase::IgnoreCase)
				|| ClassName.Equals(Requested + TEXT("_C"), ESearchCase::IgnoreCase)
				|| ClassName.Equals(RequestedWithoutCppPrefix + TEXT("_C"), ESearchCase::IgnoreCase)
				|| (ClassName.EndsWith(TEXT("_C")) && ClassName.LeftChop(2).Equals(Requested, ESearchCase::IgnoreCase))
				|| (ClassName.EndsWith(TEXT("_C")) && ClassName.LeftChop(2).Equals(RequestedWithoutCppPrefix, ESearchCase::IgnoreCase))
				|| WalkClass->GetPathName().Equals(Requested, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	FVector2D GetViewportSize(UWorld* World)
	{
		if (World)
		{
			if (UGameViewportClient* GameViewportClient = World->GetGameViewport())
			{
				if (GameViewportClient->Viewport)
				{
					const FIntPoint SizeXY = GameViewportClient->Viewport->GetSizeXY();
					if (SizeXY.X > 0 && SizeXY.Y > 0)
					{
						return FVector2D(static_cast<float>(SizeXY.X), static_cast<float>(SizeXY.Y));
					}
				}
			}
		}

		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			const FIntPoint SizeXY = GEngine->GameViewport->Viewport->GetSizeXY();
			if (SizeXY.X > 0 && SizeXY.Y > 0)
			{
				return FVector2D(static_cast<float>(SizeXY.X), static_cast<float>(SizeXY.Y));
			}
		}

		return FVector2D(1920.f, 1080.f);
	}

	TSharedPtr<SWidget> GetSlateRoot(UWidget* Widget)
	{
		if (!Widget || Widget->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed))
		{
			return nullptr;
		}

		TSharedPtr<SWidget> RootWidget = Widget->GetCachedWidget();
		if (!RootWidget.IsValid())
		{
			RootWidget = Widget->TakeWidget();
		}

		return RootWidget;
	}

	bool WidgetBelongsToWorld(const UUserWidget* Widget, const UWorld* World)
	{
		if (!Widget || !World)
		{
			return false;
		}

		if (Widget->GetWorld() == World)
		{
			return true;
		}

		if (const APlayerController* OwningPlayer = Widget->GetOwningPlayer())
		{
			return OwningPlayer->GetWorld() == World;
		}

		return false;
	}

	void CollectViewportUserWidgets(UWorld* World, TArray<FT66ViewportWidgetCandidate>& OutWidgets)
	{
		OutWidgets.Reset();
		if (!World)
		{
			return;
		}

		UGameViewportSubsystem* ViewportSubsystem = UGameViewportSubsystem::Get(World);
		for (TObjectIterator<UUserWidget> It; It; ++It)
		{
			UUserWidget* Widget = *It;
			if (!IsValid(Widget)
				|| Widget->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed)
				|| !WidgetBelongsToWorld(Widget, World))
			{
				continue;
			}

			const bool bInViewport = Widget->IsInViewport()
				|| (ViewportSubsystem && ViewportSubsystem->IsWidgetAdded(Widget))
				|| Widget->GetCachedWidget().IsValid();
			if (!bInViewport)
			{
				continue;
			}

			FT66ViewportWidgetCandidate Candidate;
			Candidate.Widget = Widget;
			Candidate.Name = FString::Printf(TEXT("%s.%s"), *Widget->GetClass()->GetName(), *Widget->GetName());
			if (ViewportSubsystem && ViewportSubsystem->IsWidgetAdded(Widget))
			{
				Candidate.ZOrder = ViewportSubsystem->GetWidgetSlot(Widget).ZOrder;
			}
			OutWidgets.Add(MoveTemp(Candidate));
		}

		OutWidgets.Sort([](const FT66ViewportWidgetCandidate& Left, const FT66ViewportWidgetCandidate& Right)
		{
			if (Left.ZOrder != Right.ZOrder)
			{
				return Left.ZOrder < Right.ZOrder;
			}
			return Left.Name < Right.Name;
		});
	}

	void CollectVisibleWidgetComponentUserWidgets(UWorld* World, TArray<UUserWidget*>& OutWidgets)
	{
		OutWidgets.Reset();
		if (!World)
		{
			return;
		}

		for (TObjectIterator<UWidgetComponent> It; It; ++It)
		{
			UWidgetComponent* WidgetComponent = *It;
			if (!IsValid(WidgetComponent)
				|| WidgetComponent->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed)
				|| WidgetComponent->GetWorld() != World
				|| !WidgetComponent->IsVisible())
			{
				continue;
			}

			WidgetComponent->InitWidget();
			UUserWidget* ComponentWidget = WidgetComponent->GetWidget();
			if (IsValid(ComponentWidget)
				&& WidgetBelongsToWorld(ComponentWidget, World))
			{
				OutWidgets.Add(ComponentWidget);
			}
		}
	}

	bool WidgetTagMatches(const TSharedRef<SWidget>& Widget, const FName RequestedTag)
	{
		if (RequestedTag.IsNone())
		{
			return false;
		}

		if (Widget->GetTag() == RequestedTag)
		{
			return true;
		}

		const TSharedPtr<FT66FlatWidgetMetadata> Metadata = Widget->GetMetaData<FT66FlatWidgetMetadata>();
		return Metadata.IsValid() && Metadata->Tag == RequestedTag;
	}

	TSharedPtr<SWidget> FindDescendantByTag(const TSharedRef<SWidget>& Widget, const FName RequestedTag)
	{
		if (WidgetTagMatches(Widget, RequestedTag))
		{
			return Widget;
		}

		FChildren* Children = Widget->GetAllChildren();
		const int32 NumChildren = Children ? Children->Num() : 0;
		for (int32 Index = 0; Index < NumChildren; ++Index)
		{
			if (TSharedPtr<SWidget> Result = FindDescendantByTag(Children->GetChildAt(Index), RequestedTag))
			{
				return Result;
			}
		}

		return nullptr;
	}

	TSharedPtr<SWidget> FindDescendantByType(const TSharedRef<SWidget>& Widget, const FString& RequestedType)
	{
		if (Widget->GetTypeAsString().Equals(RequestedType, ESearchCase::IgnoreCase))
		{
			return Widget;
		}

		FChildren* Children = Widget->GetAllChildren();
		const int32 NumChildren = Children ? Children->Num() : 0;
		for (int32 Index = 0; Index < NumChildren; ++Index)
		{
			if (TSharedPtr<SWidget> Result = FindDescendantByType(Children->GetChildAt(Index), RequestedType))
			{
				return Result;
			}
		}

		return nullptr;
	}

	bool ResolveByClass(UWorld* World, const FString& RequestedClassName, FT66ResolvedDumpTarget& OutTarget, FString& OutError)
	{
		TArray<FT66ViewportWidgetCandidate> ViewportWidgets;
		CollectViewportUserWidgets(World, ViewportWidgets);
		if (ViewportWidgets.IsEmpty())
		{
			OutError = TEXT("No active viewport UUserWidget roots were found.");
			return false;
		}

		for (const FT66ViewportWidgetCandidate& Candidate : ViewportWidgets)
		{
			UUserWidget* Widget = Candidate.Widget;
			if (Widget && ClassOrSuperNameMatches(Widget->GetClass(), RequestedClassName))
			{
				OutTarget.RootWidget = GetSlateRoot(Widget);
				OutTarget.DumpName = FString::Printf(TEXT("Widget.Class.%s"), *RequestedClassName);
				return OutTarget.RootWidget.IsValid();
			}
		}

		for (const FT66ViewportWidgetCandidate& Candidate : ViewportWidgets)
		{
			if (UUserWidget* Widget = Candidate.Widget)
			{
				if (TSharedPtr<SWidget> RootWidget = GetSlateRoot(Widget))
				{
					if (TSharedPtr<SWidget> Descendant = FindDescendantByType(RootWidget.ToSharedRef(), RequestedClassName))
					{
						OutTarget.RootWidget = Descendant;
						OutTarget.DumpName = FString::Printf(TEXT("Widget.SlateType.%s"), *RequestedClassName);
						return true;
					}
				}
			}
		}

		TArray<UUserWidget*> ComponentWidgets;
		CollectVisibleWidgetComponentUserWidgets(World, ComponentWidgets);
		for (UUserWidget* Widget : ComponentWidgets)
		{
			if (Widget && ClassOrSuperNameMatches(Widget->GetClass(), RequestedClassName))
			{
				OutTarget.RootWidget = GetSlateRoot(Widget);
				OutTarget.DumpName = FString::Printf(TEXT("Widget.ComponentClass.%s"), *RequestedClassName);
				return OutTarget.RootWidget.IsValid();
			}
		}

		for (TObjectIterator<UUserWidget> It; It; ++It)
		{
			UUserWidget* Widget = *It;
			if (!IsValid(Widget)
				|| Widget->HasAnyFlags(RF_ClassDefaultObject | RF_BeginDestroyed | RF_FinishDestroyed)
				|| !ClassOrSuperNameMatches(Widget->GetClass(), RequestedClassName)
				|| !Widget->GetCachedWidget().IsValid())
			{
				continue;
			}

			OutTarget.RootWidget = GetSlateRoot(Widget);
			OutTarget.DumpName = FString::Printf(TEXT("Widget.Class.%s"), *RequestedClassName);
			return OutTarget.RootWidget.IsValid();
		}

		OutError = FString::Printf(TEXT("No active viewport widget matched Class=%s."), *RequestedClassName);
		return false;
	}

	bool ResolveByTag(UWorld* World, const FString& RequestedTag, FT66ResolvedDumpTarget& OutTarget, FString& OutError)
	{
		TArray<FT66ViewportWidgetCandidate> ViewportWidgets;
		CollectViewportUserWidgets(World, ViewportWidgets);
		if (ViewportWidgets.IsEmpty())
		{
			OutError = TEXT("No active viewport UUserWidget roots were found.");
			return false;
		}

		const FName TagName(*RequestedTag.TrimStartAndEnd());
		for (const FT66ViewportWidgetCandidate& Candidate : ViewportWidgets)
		{
			if (UUserWidget* Widget = Candidate.Widget)
			{
				if (TSharedPtr<SWidget> RootWidget = GetSlateRoot(Widget))
				{
					if (TSharedPtr<SWidget> Descendant = FindDescendantByTag(RootWidget.ToSharedRef(), TagName))
					{
						OutTarget.RootWidget = Descendant;
						OutTarget.DumpName = FString::Printf(TEXT("Widget.Tag.%s"), *RequestedTag);
						return true;
					}
				}
			}
		}

		OutError = FString::Printf(TEXT("No active viewport widget matched Tag=%s."), *RequestedTag);
		return false;
	}

	bool ResolveByViewportIndex(UWorld* World, const FString& RequestedIndex, FT66ResolvedDumpTarget& OutTarget, FString& OutError)
	{
		TArray<FT66ViewportWidgetCandidate> ViewportWidgets;
		CollectViewportUserWidgets(World, ViewportWidgets);

		int32 Index = INDEX_NONE;
		LexTryParseString(Index, *RequestedIndex.TrimStartAndEnd());
		if (!ViewportWidgets.IsValidIndex(Index))
		{
			OutError = FString::Printf(TEXT("ViewportIndex=%s is invalid; found %d active viewport widget roots."), *RequestedIndex, ViewportWidgets.Num());
			return false;
		}

		UUserWidget* Widget = ViewportWidgets[Index].Widget;
		OutTarget.RootWidget = GetSlateRoot(Widget);
		OutTarget.DumpName = Widget
			? FString::Printf(TEXT("Widget.ViewportIndex.%d.%s"), Index, *Widget->GetClass()->GetName())
			: FString::Printf(TEXT("Widget.ViewportIndex.%d"), Index);
		return OutTarget.RootWidget.IsValid();
	}

	AActor* FindActorByName(UWorld* World, const FString& RequestedActorName)
	{
		if (!World)
		{
			return nullptr;
		}

		const FString Requested = RequestedActorName.TrimStartAndEnd();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (IsValid(Actor)
				&& (Actor->GetName().Equals(Requested, ESearchCase::IgnoreCase)
					|| Actor->GetFName().ToString().Equals(Requested, ESearchCase::IgnoreCase)))
			{
				return Actor;
			}
		}

		return nullptr;
	}

	bool ResolveByActor(UWorld* World, const FString& RequestedActorName, FT66ResolvedDumpTarget& OutTarget, FString& OutError)
	{
		AActor* Actor = FindActorByName(World, RequestedActorName);
		if (!Actor)
		{
			OutError = FString::Printf(TEXT("No actor matched Actor=%s."), *RequestedActorName);
			return false;
		}

		TInlineComponentArray<UWidgetComponent*> WidgetComponents(Actor);
		for (UWidgetComponent* WidgetComponent : WidgetComponents)
		{
			if (!WidgetComponent)
			{
				continue;
			}

			WidgetComponent->InitWidget();
			if (UUserWidget* ComponentWidget = WidgetComponent->GetWidget())
			{
				OutTarget.RootWidget = GetSlateRoot(ComponentWidget);
				if (OutTarget.RootWidget.IsValid())
				{
					OutTarget.DumpName = FString::Printf(TEXT("Widget.Actor.%s.%s"), *Actor->GetName(), *ComponentWidget->GetClass()->GetName());
					return true;
				}
			}
		}

		if (ClassOrSuperNameMatches(Actor->GetClass(), TEXT("AT66WorldInteractableBase"))
			&& ResolveByClass(World, TEXT("UT66GameplayHUDWidget"), OutTarget, OutError))
		{
			OutTarget.DumpName = FString::Printf(TEXT("Widget.ActorPrompt.%s.UT66GameplayHUDWidget"), *Actor->GetName());
			return true;
		}

		OutError = FString::Printf(TEXT("Actor=%s has no dumpable UWidgetComponent. T66 world interactables currently render prompts through UT66GameplayHUDWidget."), *RequestedActorName);
		return false;
	}

	bool ParseTargetSelector(const FString& RawTargetSpec, FString& OutSelector, FString& OutValue, FString& OutError)
	{
		const FString TargetSpec = StripOptionalTargetPrefix(RawTargetSpec);
		FString Key;
		FString Value;
		if (!TargetSpec.Split(TEXT("="), &Key, &Value))
		{
			OutError = FString::Printf(TEXT("Invalid target '%s'. Expected Class=, Tag=, ViewportIndex=, or Actor=."), *TargetSpec);
			return false;
		}

		OutSelector = Key.TrimStartAndEnd();
		OutValue = Value.TrimStartAndEnd().TrimQuotes();
		if (OutSelector.IsEmpty() || OutValue.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Invalid target '%s'. Selector and value are required."), *TargetSpec);
			return false;
		}

		return true;
	}

	void EnsureDumpRootHasTag(const TSharedRef<SWidget>& RootWidget, const FString& DumpName)
	{
		if (!RootWidget->GetTag().IsNone())
		{
			return;
		}

		const TSharedPtr<FT66FlatWidgetMetadata> Metadata = RootWidget->GetMetaData<FT66FlatWidgetMetadata>();
		if (Metadata.IsValid() && !Metadata->Tag.IsNone())
		{
			return;
		}

		RootWidget->SetTag(FName(*FString::Printf(TEXT("%s.Root"), *DumpName)));
	}
}

bool FT66WidgetDumpTargets::ParseAutomationSpec(
	const FString& AutomationSpec,
	FString& OutTargetSpec,
	FString& OutOutputPath,
	FString& OutError)
{
	const FString Spec = AutomationSpec.TrimStartAndEnd().TrimQuotes();
	const int32 SeparatorIndex = Spec.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart);
	if (SeparatorIndex == INDEX_NONE)
	{
		OutError = FString::Printf(TEXT("Invalid -T66AutoDumpWidget value '%s'. Expected <Target>:<Path>."), *Spec);
		return false;
	}

	OutTargetSpec = Spec.Left(SeparatorIndex).TrimStartAndEnd().TrimQuotes();
	OutOutputPath = Spec.Mid(SeparatorIndex + 1).TrimStartAndEnd().TrimQuotes();
	if (OutTargetSpec.IsEmpty() || OutOutputPath.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Invalid -T66AutoDumpWidget value '%s'. Target and path are required."), *Spec);
		return false;
	}

	return true;
}

bool FT66WidgetDumpTargets::DumpTargetToJson(
	UWorld* World,
	const FString& TargetSpec,
	const FString& OutputPath,
	FString& OutError)
{
	if (!World)
	{
		OutError = TEXT("World is null.");
		return false;
	}

	FString Selector;
	FString Value;
	if (!ParseTargetSelector(TargetSpec, Selector, Value, OutError))
	{
		return false;
	}

	FT66ResolvedDumpTarget ResolvedTarget;
	if (Selector.Equals(TEXT("Class"), ESearchCase::IgnoreCase))
	{
		if (!ResolveByClass(World, Value, ResolvedTarget, OutError))
		{
			return false;
		}
	}
	else if (Selector.Equals(TEXT("Tag"), ESearchCase::IgnoreCase))
	{
		if (!ResolveByTag(World, Value, ResolvedTarget, OutError))
		{
			return false;
		}
	}
	else if (Selector.Equals(TEXT("ViewportIndex"), ESearchCase::IgnoreCase))
	{
		if (!ResolveByViewportIndex(World, Value, ResolvedTarget, OutError))
		{
			return false;
		}
	}
	else if (Selector.Equals(TEXT("Actor"), ESearchCase::IgnoreCase))
	{
		if (!ResolveByActor(World, Value, ResolvedTarget, OutError))
		{
			return false;
		}
	}
	else
	{
		OutError = FString::Printf(TEXT("Unsupported widget dump selector '%s'. Expected Class, Tag, ViewportIndex, or Actor."), *Selector);
		return false;
	}

	if (!ResolvedTarget.RootWidget.IsValid())
	{
		OutError = FString::Printf(TEXT("Resolved target '%s' has no valid Slate root."), *TargetSpec);
		return false;
	}

	EnsureDumpRootHasTag(ResolvedTarget.RootWidget.ToSharedRef(), ResolvedTarget.DumpName);

	FString WalkerError;
	const FString ResolvedOutputPath = FPaths::ConvertRelativePathToFull(OutputPath);
	const bool bDumped = FT66WidgetTreeWalker::DumpWidgetTreeToJson(
		ResolvedTarget.RootWidget.ToSharedRef(),
		ResolvedTarget.DumpName,
		GetViewportSize(World),
		ResolvedOutputPath,
		WalkerError);
	if (!bDumped)
	{
		OutError = WalkerError;
		return false;
	}

	return true;
}
