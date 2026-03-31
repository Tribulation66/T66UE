// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAssetComponent.h"

#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBVolume4.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

UZibraVDBAssetComponent::UZibraVDBAssetComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UZibraVDBAssetComponent::SetZibraVDBMaterialComponent(UZibraVDBMaterialComponent* InZibraVDBMaterialComponent)
{
	ZibraVDBMaterialComponent = InZibraVDBMaterialComponent;
}

void UZibraVDBAssetComponent::SetZibraVDBPlaybackComponent(UZibraVDBPlaybackComponent* InZibraVDBPlaybackComponent)
{
	ZibraVDBPlaybackComponent = InZibraVDBPlaybackComponent;
}

FString UZibraVDBAssetComponent::GetAssetName() const noexcept
{
	return ZibraVDBVolume->GetName();
}

void UZibraVDBAssetComponent::BroadcastFrameChanged(uint32 Frame) noexcept
{
	if (CurrentFrame != Frame)
	{
		CurrentFrame = Frame;
		OnFrameChanged.Broadcast(Frame);
	}
}

uint32 UZibraVDBAssetComponent::GetCurrentFrame() const noexcept
{
	return CurrentFrame;
}

void UZibraVDBAssetComponent::GetReferencedContentObjects(TArray<UObject*>& Objects) const noexcept
{
	if (ZibraVDBVolume)
	{
		Objects.Add(ZibraVDBVolume.Get());
	}
}

void UZibraVDBAssetComponent::SetZibraVDBVolume(UZibraVDBVolume4* InZibraVDBVolume) noexcept
{
	if (ZibraVDBVolume != InZibraVDBVolume)
	{
		ZibraVDBVolume = InZibraVDBVolume;
		OnVolumeChanged();
	}
}

bool UZibraVDBAssetComponent::HasZibraVDBVolume() const noexcept
{
	return ZibraVDBVolume != nullptr;
}

const UZibraVDBVolume4* UZibraVDBAssetComponent::GetZibraVDBVolume() const noexcept
{
	return ZibraVDBVolume.Get();
}

void UZibraVDBAssetComponent::OnZibraVDBVolumeReimport(TObjectPtr<UZibraVDBVolume4> InZibraVDBVolume)
{
	SetZibraVDBVolume(InZibraVDBVolume);
	OnVolumeChanged();
}

void UZibraVDBAssetComponent::OnVolumeChanged() noexcept
{
	CurrentFrame = 0;
	bIsAssetValid = false;

	if (ZibraVDBMaterialComponent != nullptr)
	{
		ZibraVDBMaterialComponent->OnVolumeChanged();
	}

	if (ZibraVDBPlaybackComponent != nullptr)
	{
		ZibraVDBPlaybackComponent->OnVolumeChanged();
	}

	bIsAssetValid = true;
}

bool UZibraVDBAssetComponent::IsAssetValid() const noexcept
{
	return bIsAssetValid;
}

#if WITH_EDITOR
void UZibraVDBAssetComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Determine which property was changed
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// React to the specific property change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBAssetComponent, ZibraVDBVolume))
	{
		OnVolumeChanged();
	}
}

#endif

#undef LOCTEXT_NAMESPACE
