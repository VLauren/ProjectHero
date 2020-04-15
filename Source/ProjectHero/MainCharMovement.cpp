#include "MainCharMovement.h"
#include "MainChar.h"
#include "PHGame.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "DrawDebugHelpers.h"


void UMainCharMovement::BeginPlay()
{
	Super::BeginPlay();

	MainChar = (AMainChar*)GetOwner();

	startGravityStrength = MainChar->GravityStrength;

	TickMove = FVector::ZeroVector;
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
		movementVector *= MainChar->RunSpeedMultiplier;
	movementVector *= DeltaTime;

	TickMove = FMath::Lerp(TickMove, movementVector, MainChar->StopLerpSpeed);

	// Control movement is only applied in neutral or attack state
	if (!TickMove.IsNearlyZero() && (AMainChar::GetPlayerState() == EMainCharState::MOVING) || (AMainChar::GetPlayerState() == EMainCharState::ATTACK))
	{
		FHitResult Hit;

		// Standard control movement
		if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
		{
			SafeMoveUpdatedComponent(TickMove, UpdatedComponent->GetComponentRotation(), true, Hit);
			if (!InputVector.IsNearlyZero())
				isMoving = true;
			else
				MainChar->Running = false;
			UseGravity = true;
		}
		else if(!MainChar->RisingAttack())
			UseGravity = false;

		// If it hits something, it slides along its surface
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(TickMove, 1.f - Hit.Time, Hit.Normal, Hit);
		
		movementVector.Z = 0;

		// Attack tracking
		if (AMainChar::GetPlayerState() == EMainCharState::ATTACK)
		{
			if (MainChar->AutoTarget != nullptr && MainChar->CanTrack())
			{
				// Target direction
				FVector dir = MainChar->AutoTarget->GetActorLocation() - MainChar->GetActorLocation();

				// Rotate character towards target
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

				// Auto target camera follow
				if(MainChar->LockTarget == nullptr && FMath::Abs(DeltaYaw) > 60)
					MainChar->AddControllerYawInput(DeltaYaw * DeltaTime * 0.5f / InputScale);

				// Lock target camera follow
				if (MainChar->LockTarget != nullptr && FMath::Abs(DeltaYaw) > 5)
					MainChar->AddControllerYawInput(DeltaYaw * DeltaTime / InputScale);
			}
		}
		else if (!movementVector.IsNearlyZero())
		{
			// Target rotation
			FRotator ctrlRot = movementVector.Rotation();

			// Rotate character towards target rotation
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

	// Ignore gravity frames
	if (justJumped > 0)
		justJumped--;

	// Launch attack float
	if (rising && ZVel <= 0.1f && ZVel > -2)
		MainChar->GravityStrength = startGravityStrength / 4;
	else
		MainChar->GravityStrength = startGravityStrength;
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

	UseGravity = true;
	ZVel = MainChar->JumpStrength;
	justJumped = 4;
	rising = false;
}

void UMainCharMovement::Rise()
{
	if (MainChar == nullptr)
		return;

	UseGravity = true;
	ZVel = MainChar->JumpStrength;
	justJumped = 4;
	rising = true;
}

void UMainCharMovement::Launch(float amount, bool spLaunch)
{
	UseGravity = true;
	ZVel = amount;
	SpLaunch = spLaunch;
	justJumped = 4;
	rising = false;
}

void UMainCharMovement::Dodge()
{
	FVector direction = InputVector;
	rising = false;

	// Dodge following input vector
	if (InputVector != FVector::ZeroVector)
	{
		MoveOverTime(2.5f * MainChar->MovementSpeed, MainChar->DodgeTime, false, direction);
		UpdatedComponent->GetOwner()->SetActorRotation(direction.Rotation());

		MainChar->BackDodge = false;

		CurrentRotation = InputVector.Rotation();
	}
	// If the is no direction to dodge, perform a back dodge
	else
	{
		direction = -MainChar->GetActorForwardVector();
		MoveOverTime(2.5f * MainChar->MovementSpeed, MainChar->DodgeTime, false, direction);

		MainChar->BackDodge = true;
	}
}

void UMainCharMovement::ResetZVel()
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

void UMainCharMovement::ForceSetRotation(FRotator NewRotation)
{
	// Rotate character towards target rotation
	CurrentRotation = NewRotation;
	UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
}
