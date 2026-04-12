// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldInteractableBase.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static UMaterialInterface* T66_GetPromptTextMaterial()
	{
		static TObjectPtr<UMaterialInterface> CachedMaterial = nullptr;
		if (!CachedMaterial)
		{
			CachedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/UnlitText.UnlitText"));
			if (!CachedMaterial)
			{
				CachedMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque.DefaultTextMaterialOpaque"));
			}
		}

		return CachedMaterial;
	}

	static void T66_ConfigurePromptTextComponent(UTextRenderComponent* TextComponent, float WorldSize)
	{
		if (!TextComponent)
		{
			return;
		}

		TextComponent->SetHorizontalAlignment(EHTA_Center);
		TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
		TextComponent->SetWorldSize(WorldSize);
		TextComponent->SetTextRenderColor(FColor::White);
		TextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TextComponent->SetCastShadow(false);
		TextComponent->SetReceivesDecals(false);
		TextComponent->bAlwaysRenderAsText = true;
		if (UMaterialInterface* PromptMaterial = T66_GetPromptTextMaterial())
		{
			TextComponent->SetTextMaterial(PromptMaterial);
		}
		TextComponent->SetHiddenInGame(true, true);
		TextComponent->SetVisibility(false, true);
	}
}

AT66WorldInteractableBase::AT66WorldInteractableBase()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(GetMinimumInteractionExtent());
	RootComponent = TriggerBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(RootComponent);
	T66_ConfigurePromptTextComponent(PromptText, GetInteractionPromptWorldSize());

	PromptTextBack = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptTextBack"));
	PromptTextBack->SetupAttachment(RootComponent);
	T66_ConfigurePromptTextComponent(PromptTextBack, GetInteractionPromptWorldSize());

	ApplyRarityVisuals();
}

void AT66WorldInteractableBase::BeginPlay()
{
	Super::BeginPlay();

	if (PromptText && GetRootComponent() && PromptText->GetAttachParent() != GetRootComponent())
	{
		PromptText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (PromptTextBack && GetRootComponent() && PromptTextBack->GetAttachParent() != GetRootComponent())
	{
		PromptTextBack->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66WorldInteractableBase::HandleTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AT66WorldInteractableBase::HandleTriggerEndOverlap);
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());
	if (const AT66HeroBase* LocalHero = GetLocalHero())
	{
		LocalHeroOverlapCount = (TriggerBox && TriggerBox->IsOverlappingActor(LocalHero)) ? 1 : 0;
	}
	UpdateInteractionBounds();
	RefreshInteractionPrompt();
}

void AT66WorldInteractableBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &AT66WorldInteractableBase::HandleTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.RemoveDynamic(this, &AT66WorldInteractableBase::HandleTriggerEndOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66WorldInteractableBase::SetRarity(ET66Rarity InRarity)
{
	Rarity = InRarity;
	ApplyRarityVisuals();
	UpdateInteractionBounds();
	RefreshInteractionPrompt();
}

void AT66WorldInteractableBase::ApplyRarityVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FT66RarityUtil::GetRarityColor(Rarity));
}

bool AT66WorldInteractableBase::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	return LocalHero && !bConsumed && LocalHeroOverlapCount > 0;
}

FText AT66WorldInteractableBase::BuildInteractionPromptText() const
{
	UGameInstance* GI = GetGameInstance();
	UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	if (!PromptSubsystem)
	{
		return FText::GetEmpty();
	}

	return PromptSubsystem->BuildPromptText(PromptSubsystem->GetPromptActionForActor(this));
}

void AT66WorldInteractableBase::RefreshInteractionPrompt()
{
	if (!PromptText)
	{
		return;
	}

	if (HasAnyFlags(RF_ClassDefaultObject) || !GetWorld())
	{
		HideInteractionPrompt();
		return;
	}

	const AT66HeroBase* LocalHero = GetLocalHero();
	const bool bShowPrompt = ShouldShowInteractionPrompt(LocalHero);
	const FText Prompt = bShowPrompt ? BuildInteractionPromptText() : FText::GetEmpty();

	if (!bShowPrompt || Prompt.IsEmpty())
	{
		HideInteractionPrompt();
		return;
	}

	PromptText->SetWorldSize(GetInteractionPromptWorldSize());
	PromptText->SetTextRenderColor(FColor::White);
	UpdateInteractionPromptPlacement();
	PromptText->SetText(Prompt);
	PromptText->SetHiddenInGame(false, true);
	PromptText->SetVisibility(true, true);
	if (PromptTextBack)
	{
		// UTextRender already stays readable from both sides with the engine text material.
		// Keeping a mirrored duplicate here causes doubled glyphs on imported interactables.
		PromptTextBack->SetText(Prompt);
		PromptTextBack->SetHiddenInGame(true, true);
		PromptTextBack->SetVisibility(false, true);
	}
	UpdateInteractionPromptFacing();
}

const AT66HeroBase* AT66WorldInteractableBase::GetLocalHero() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	return Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
}

bool AT66WorldInteractableBase::TryApplyImportedMesh()
{
	if (!VisualMesh) return false;

	UStaticMesh* M = nullptr;

	if (RarityMeshes.Num() > 0)
	{
		const TSoftObjectPtr<UStaticMesh>* Ptr = RarityMeshes.Find(Rarity);
		if (Ptr && !Ptr->IsNull())
		{
			M = Ptr->Get();
			if (!M) M = Ptr->LoadSynchronous();
		}
	}

	if (!M && !SingleMesh.IsNull())
	{
		M = SingleMesh.Get();
		if (!M) M = SingleMesh.LoadSynchronous();
	}

	if (!M) return false;

	VisualMesh->EmptyOverrideMaterials();
	VisualMesh->SetStaticMesh(M);
	VisualMesh->SetRelativeScale3D(GetImportedVisualScale());
	VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, M);
	if (HasActorBegunPlay())
	{
		FT66VisualUtil::SnapToGround(this, GetWorld());
	}
	UpdateInteractionBounds();
	UpdateInteractionPromptPlacement();
	return true;
}

void AT66WorldInteractableBase::UpdateInteractionBounds()
{
	if (!TriggerBox)
	{
		return;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	const FTransform ActorTransform = GetActorTransform();
	const FVector MinimumExtent = GetMinimumInteractionExtent();
	const FVector Padding = GetInteractionBoundsPadding();
	FVector RequiredExtent = MinimumExtent;

	for (UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (!Primitive
			|| Primitive == TriggerBox
			|| Primitive == PromptText
			|| Primitive == PromptTextBack
			|| !Primitive->IsRegistered())
		{
			continue;
		}

		const bool bVisible = Primitive->IsVisible() && !Primitive->bHiddenInGame;
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Primitive))
		{
			if (bVisible)
			{
				StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				StaticMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
				StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
				StaticMeshComponent->SetCanEverAffectNavigation(false);
			}
			else
			{
				StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}

		if (!bVisible)
		{
			continue;
		}

		const FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
		const FVector LocalCenter = ActorTransform.InverseTransformPosition(Bounds.Origin);
		const FVector PrimitiveExtent(
			FMath::Abs(LocalCenter.X) + Bounds.SphereRadius,
			FMath::Abs(LocalCenter.Y) + Bounds.SphereRadius,
			FMath::Abs(LocalCenter.Z) + Bounds.SphereRadius);
		RequiredExtent.X = FMath::Max(RequiredExtent.X, PrimitiveExtent.X + Padding.X);
		RequiredExtent.Y = FMath::Max(RequiredExtent.Y, PrimitiveExtent.Y + Padding.Y);
		RequiredExtent.Z = FMath::Max(RequiredExtent.Z, PrimitiveExtent.Z + Padding.Z);
	}

	TriggerBox->SetBoxExtent(RequiredExtent);
}

void AT66WorldInteractableBase::UpdateInteractionPromptFacing() const
{
	if (!PromptText || !PromptText->IsVisible())
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector ToCamera = CameraLocation - PromptText->GetComponentLocation();
	if (ToCamera.IsNearlyZero())
	{
		return;
	}

	const FRotator Facing = ToCamera.Rotation();
	PromptText->SetWorldRotation(FRotator(0.f, Facing.Yaw, 0.f));
	if (PromptTextBack)
	{
		PromptTextBack->SetWorldRotation(FRotator(0.f, Facing.Yaw + 180.f, 0.f));
	}
}

void AT66WorldInteractableBase::UpdateInteractionPromptPlacement()
{
	if (!PromptText || !GetRootComponent())
	{
		return;
	}

	float HighestWorldZ = GetActorLocation().Z;
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (const UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (!Primitive || Primitive == TriggerBox || Primitive == PromptText || Primitive == PromptTextBack || !Primitive->IsRegistered())
		{
			continue;
		}

		const FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
		HighestWorldZ = FMath::Max(HighestWorldZ, Bounds.Origin.Z + Bounds.BoxExtent.Z);
	}

	const float RelativeZ = (HighestWorldZ - GetRootComponent()->GetComponentLocation().Z) + GetInteractionPromptVerticalPadding();
	PromptText->SetRelativeLocation(FVector(0.f, 0.f, RelativeZ));
	if (PromptTextBack)
	{
		PromptTextBack->SetRelativeLocation(FVector(0.f, 0.f, RelativeZ));
	}
}

void AT66WorldInteractableBase::HideInteractionPrompt()
{
	if (!PromptText)
	{
		return;
	}

	PromptText->SetHiddenInGame(true, true);
	PromptText->SetVisibility(false, true);
	if (PromptTextBack)
	{
		PromptTextBack->SetHiddenInGame(true, true);
		PromptTextBack->SetVisibility(false, true);
	}
}

bool AT66WorldInteractableBase::IsLocalHeroActor(const AActor* OtherActor) const
{
	if (!GetWorld())
	{
		return false;
	}

	return OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0);
}

void AT66WorldInteractableBase::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	LocalHeroOverlapCount++;
	RefreshInteractionPrompt();
}

void AT66WorldInteractableBase::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	if (!IsLocalHeroActor(OtherActor))
	{
		return;
	}

	LocalHeroOverlapCount = FMath::Max(0, LocalHeroOverlapCount - 1);
	RefreshInteractionPrompt();
}
