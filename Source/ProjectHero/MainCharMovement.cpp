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
			else
				MainChar->Running = false;
			UseGravity = true;
		}
		else if(!MainChar->RisingAttack())
			UseGravity = false;

		// Si chocamos con algo, me deslizo sobre el
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(Move, 1.f - Hit.Time, Hit.Normal, Hit);
		
		movementVector.Z = 0;

		// Attack tracking
		if (AMainChar::GetPlayerState() == EMainCharState::ATTACK)
		{
			if (MainChar->AutoTarget != nullptr && MainChar->CanTrack())
			{
				FVector dir = MainChar->AutoTarget->GetActorLocation() - MainChar->GetActorLocation();

				dir.Z = 0;
				dir.Normalize();
				CurrentRotation = FMath::Lerp(CurrentRotation, dir.Rotation(), MainChar->RotationLerpSpeed);
				UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);

				// Camera auto reorientation
				FRotator Current = MainChar->Controller->GetControlRotation();
				FRotator Target = MainChar->Mesh->GetComponentRotation() + FRotator(0, 90, 0);
				float InputScale = Cast<APlayerController>(MainChar->Controller)->InputYawScale;
				float DeltaYaw = (Target - Current).Yaw;

				// WHY UNREAL
				if (DeltaYaw > 180) DeltaYaw -= 360;
				if (DeltaYaw < -180) DeltaYaw += 360;
				// TODO ComposeRotator

				// Auto target adjustment
				if(MainChar->LockTarget == nullptr && FMath::Abs(DeltaYaw) > 60)
					MainChar->AddControllerYawInput(DeltaYaw * DeltaTime * 0.5f / InputScale);

				// Lock target adjustment
				if (MainChar->LockTarget != nullptr && FMath::Abs(DeltaYaw) > 5)
					MainChar->AddControllerYawInput(DeltaYaw * DeltaTime / InputScale);
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
		if (AMainChar::GetPlayerState() == EMainCharState::MOVING) // Moving sate but without innput (eg: after air attack)
			UseGravity = true;
	}

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
	if (InputVector != FVector::ZeroVector)
	{
		MoveOverTime(2.5f * MainChar->MovementSpeed, MainChar->DodgeTime, false, direction);
		UpdatedComponent->GetOwner()->SetActorRotation(direction.Rotation());

		MainChar->BackDodge = false;

		CurrentRotation = InputVector.Rotation();
	}
	else
	{
		direction = -MainChar->GetActorForwardVector();
		MoveOverTime(2.5f * MainChar->MovementSpeed, MainChar->DodgeTime, false, direction);

		MainChar->BackDodge = true;
	}
	

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
	QuickFall = false;
	ZVel = 0;
}

FVector UMainCharMovement::GetCurrentInputVector()
{
	return InputVector;
}

