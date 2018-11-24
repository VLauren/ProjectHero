#include "MainCharMovement.h"
#include "MainChar.h"
#include "PHGame.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "DrawDebugHelpers.h"


void UMainCharMovement::BeginPlay()
{
	Super::BeginPlay();

	MainChar = (AMainChar*)GetOwner();

	Move = FVector::ZeroVector;
	ZVel = 0;
}

void UMainCharMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	isMoving = false;

	if (!PawnOwner || !UpdatedComponent || MainChar == nullptr || ShouldSkipUpdate(DeltaTime))
		return;

	// Movement vector calculation
	FVector movementVector;
	InputVector = ConsumeInputVector().GetClampedToMaxSize(1.0f);
	movementVector = InputVector * MainChar->MovementSpeed;
	if (MainChar->IsRunning())
		movementVector *= 2; // HACK TODO create RunningSpeed variable
	//// movementVector.Z = YVel;
	movementVector *= DeltaTime;

	// Move = movimientoDeseado;
	Move = FMath::Lerp(Move, movementVector, MainChar->StopLerpSpeed);

	// FVector movimientoEsteFrame = movimientoDeseado;

	// Movimiento
	if (!Move.IsNearlyZero() && (AMainChar::GetPlayerState() == EMainCharState::MOVING) || (AMainChar::GetPlayerState() == EMainCharState::ATTACK))
	{
		//// Move.Z = YVel;
		FHitResult Hit;

		// Movimiento
		if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
		{
			SafeMoveUpdatedComponent(Move, UpdatedComponent->GetComponentRotation(), true, Hit);
			if (!InputVector.IsNearlyZero())
				isMoving = true;
			UseGravity = true;
		}
		else
			UseGravity = false;

		// Si chocamos con algo, me deslizo sobre el
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(Move, 1.f - Hit.Time, Hit.Normal, Hit);
		
		movementVector.Z = 0;

		// HACKERINO
		if (AMainChar::GetPlayerState() == EMainCharState::ATTACK)
		{
			// Attack tracking

			if (MainChar->AutoTarget != nullptr)
			{
				FVector dir = MainChar->AutoTarget->GetActorLocation() - MainChar->GetActorLocation();

				dir.Z = 0;
				dir.Normalize();
				CurrentRotation = FMath::Lerp(CurrentRotation, dir.Rotation(), MainChar->RotationLerpSpeed);
				UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);

				// UE_LOG(LogTemp, Warning, TEXT("Target: %s"), *MainChar->AutoTarget->GetName());
			}
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("Target: NULL"));
			}
		}
		else

		if (!movementVector.IsNearlyZero())
		{
			// Movement rotation
			FRotator ctrlRot = movementVector.Rotation();

			if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot,  MainChar->RotationLerpSpeed);
			else if(PushActive && PushForward)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, MainChar->RotationLerpSpeed);
			UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
		}

	}
	else
	{
		if (AMainChar::GetPlayerState() == EMainCharState::DODGE)
			UseGravity = false;
	}

	// if (IsGrounded())
	// {
		// La velocidad Z es cero
		// YVel = 0.0f;
	// }
	// else
	// {
		// Aplico la gravedad
		// YVel -= MainChar->GravityStrength * DeltaTime;
	// }

	if (justJumped > 0)
		justJumped--;
}

bool UMainCharMovement::IsGrounded()
{
	if (justJumped > 0)
		return false;

	return Super::IsGrounded();
}

void UMainCharMovement::Jump()
{
	if (MainChar == nullptr)
		return;

	// if (IsGrounded())
	{
		UseGravity = true;
		ZVel = MainChar->JumpStrength;
		justJumped = 4;
	}
}

void UMainCharMovement::Dodge()
{
	FVector direction = InputVector;
	if (InputVector == FVector::ZeroVector)
		direction = -MainChar->GetActorForwardVector();
	
	MoveOverTime(2.5f * MainChar->MovementSpeed, MainChar->DodgeTime, false, direction);

	UpdatedComponent->GetOwner()->SetActorRotation(direction.Rotation());

	// if (InputVector != FVector::ZeroVector)
		// Push(2 * MainChar->MovementSpeed, MainChar->DodgeTime, false, InputVector);
	// else
		// Push(2 * MainChar->MovementSpeed, MainChar->DodgeTime, false, -MainChar->GetActorForwardVector());
}

void UMainCharMovement::ResetYVel()
{
	ZVel = 0;
}

bool UMainCharMovement::IsMoving()
{
	return isMoving;
}


void UMainCharMovement::Cancel()
{
	PushActive = false;
	ZVel = 0;
}
