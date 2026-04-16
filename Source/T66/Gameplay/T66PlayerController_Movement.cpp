// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"

#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/Movement/T66HeroMovementComponent.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "NiagaraSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PlayerMovement, Log, All);

namespace
{
	static UT66HeroMovementComponent* T66ResolveHeroMovementComponent(const APawn* Pawn)
	{
		const AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
		return Hero ? Hero->GetHeroMovementComponent() : nullptr;
	}
}

void AT66PlayerController::UpdateHeroMovementIntent()
{
	UT66HeroMovementComponent* HeroMovement = T66ResolveHeroMovementComponent(GetPawn());
	if (!HeroMovement)
	{
		return;
	}

	float AppliedForward = RawMoveForwardValue;
	float AppliedRight = RawMoveRightValue;
	if (bWorldDialogueOpen)
	{
		AppliedForward = 0.f;
		AppliedRight = 0.f;
	}

	if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted())
		{
			AppliedForward = 0.f;
			AppliedRight = 0.f;
		}
	}

	HeroMovement->SetMoveInputAxes(AppliedForward, AppliedRight);
}

void AT66PlayerController::HandleDashPressed()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	if (UT66HeroMovementComponent* HeroMovement = T66ResolveHeroMovementComponent(GetPawn()))
	{
		HeroMovement->SetDashModifierHeld(true);
	}
}

void AT66PlayerController::HandleDashReleased()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	if (UT66HeroMovementComponent* HeroMovement = T66ResolveHeroMovementComponent(GetPawn()))
	{
		HeroMovement->SetDashModifierHeld(false);
	}
}

void AT66PlayerController::HandleMoveForward(const float Value)
{
	if (!IsGameplayLevel())
	{
		return;
	}

	RawMoveForwardValue = Value;
	UpdateHeroMovementIntent();

	if (bWorldDialogueOpen)
	{
		if (UWorld* World = GetWorld())
		{
			const float Now = World->GetTimeSeconds();
			if (Now - LastWorldDialogueNavTimeSeconds >= WorldDialogueNavDebounceSeconds)
			{
				if (Value <= -0.6f)
				{
					NavigateWorldDialogue(+1);
					LastWorldDialogueNavTimeSeconds = Now;
				}
				else if (Value >= 0.6f)
				{
					NavigateWorldDialogue(-1);
					LastWorldDialogueNavTimeSeconds = Now;
				}
			}
		}
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted())
		{
			if (AT66PilotableTractor* Tractor = Hero->GetMountedTractor())
			{
				Tractor->SetDriveForwardInput(Value);
			}
			return;
		}
	}

	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialMoveInput();
		}
	}

	if (APawn* MyPawn = GetPawn())
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;

		const FVector ForwardDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
		MyPawn->AddMovementInput(ForwardDir, Value);
	}
}

void AT66PlayerController::HandleMoveRight(const float Value)
{
	if (!IsGameplayLevel())
	{
		return;
	}

	RawMoveRightValue = Value;
	UpdateHeroMovementIntent();

	if (bWorldDialogueOpen)
	{
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
	{
		if (Hero->IsVehicleMounted())
		{
			if (AT66PilotableTractor* Tractor = Hero->GetMountedTractor())
			{
				Tractor->SetDriveRightInput(Value);
			}
			return;
		}
	}

	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialMoveInput();
		}
	}

	if (APawn* MyPawn = GetPawn())
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;

		const FVector RightDir = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
		MyPawn->AddMovementInput(RightDir, Value);
	}
}

void AT66PlayerController::HandleJumpPressed()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	UT66HeroMovementComponent* HeroMovement = T66ResolveHeroMovementComponent(GetPawn());
	if (!HeroMovement || !HeroMovement->TryJump())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->NotifyTutorialJumpInput();
		}
	}

	if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* CMC = MyCharacter->GetCharacterMovement())
		{
			const FString MoveMode = StaticEnum<EMovementMode>()->GetNameStringByValue(static_cast<int64>(CMC->MovementMode));
			UE_LOG(
				LogT66PlayerMovement,
				Verbose,
				TEXT("[JUMP] Space pressed: JumpCount=%d/%d OnGround=%d Falling=%d MoveMode=%s Z=%.1f"),
				MyCharacter->JumpCurrentCount,
				MyCharacter->JumpMaxCount,
				CMC->IsMovingOnGround() ? 1 : 0,
				CMC->IsFalling() ? 1 : 0,
				*MoveMode,
				MyCharacter->GetActorLocation().Z);
		}

		if (UWorld* World = GetWorld())
		{
			if (UNiagaraSystem* JumpVFX = GetActiveJumpVFXSystem())
			{
				const FVector FeetLoc = MyCharacter->GetActorLocation() - FVector(0.f, 0.f, 50.f);
				UT66PixelVFXSubsystem* PixelVFX = World->GetSubsystem<UT66PixelVFXSubsystem>();
				for (int32 Index = 0; Index < 6; ++Index)
				{
					const FVector Offset(FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(-30.f, 30.f), 0.f);
					if (PixelVFX)
					{
						PixelVFX->SpawnPixelAtLocation(
							FeetLoc + Offset,
							FLinearColor(1.f, 1.f, 1.f, 0.8f),
							FVector2D(2.f, 2.f),
							ET66PixelVFXPriority::Low,
							FRotator::ZeroRotator,
							FVector(1.f),
							JumpVFX);
					}
				}
			}
		}
	}
}

void AT66PlayerController::HandleJumpReleased()
{
	if (!IsGameplayLevel())
	{
		return;
	}

	if (UT66HeroMovementComponent* HeroMovement = T66ResolveHeroMovementComponent(GetPawn()))
	{
		HeroMovement->StopJumping();
	}
}
