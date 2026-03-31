// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBPlaybackComponent.h"

#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBVolume4.h"

UZibraVDBPlaybackComponent::UZibraVDBPlaybackComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	bTickInEditor = true;
	bAutoActivate = true;
}

void UZibraVDBPlaybackComponent::SetZibraVDBAssetComponent(TObjectPtr<UZibraVDBAssetComponent> InZibraVdbAssetComponent) noexcept
{
	ZibraVDBAssetComponent = InZibraVdbAssetComponent;
}

void UZibraVDBPlaybackComponent::OnRegister()
{
	Super::OnRegister();
	Init();
}

void UZibraVDBPlaybackComponent::Init() noexcept
{
	const UZibraVDBVolume4* ZibraVDBVolume = GetZibraVDBVolume();
	if (!ZibraVDBVolume)
	{
		return;
	}

	FrameCount = ZibraVDBVolume->GetZibraVDBFile().GetHeader().FrameCount;
}

void UZibraVDBPlaybackComponent::OnVolumeChanged() noexcept
{
	CurrentFrame = 0.f;
	StartFrame = 0.f;
	Init();
}

void UZibraVDBPlaybackComponent::TickComponent(
	float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UZibraVDBVolume4* ZibraVDBVolume = GetZibraVDBVolume();
	if (!ZibraVDBVolume)
	{
		return;
	}

	if (Animate)
	{
		CurrentFrame = FMath::Fmod(CurrentFrame + (Framerate * DeltaTime), FrameCount);
	}

	// We do not want to clamp CurrentFrame in UI
	ZibraVDBAssetComponent->BroadcastFrameChanged(FMath::Clamp(FMath::FloorToInt(CurrentFrame), 0, FrameCount - 1));
}

void UZibraVDBPlaybackComponent::PostLoad()
{
	Super::PostLoad();
	CurrentFrame = StartFrame;
}

const UZibraVDBVolume4* UZibraVDBPlaybackComponent::GetZibraVDBVolume() const noexcept
{
	return ZibraVDBAssetComponent ? ZibraVDBAssetComponent->GetZibraVDBVolume() : nullptr;
}

#if WITH_EDITOR
void UZibraVDBPlaybackComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Determine which property was changed
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBPlaybackComponent, StartFrame))
	{
		StartFrame = FMath::Clamp(StartFrame, 0, FrameCount - 1);
	}
}
#endif
