#include "MainCharMovement.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, text)

void UMainCharMovement::BeginPlay()
{
	Super::BeginPlay();

	MainChar = (AMainChar*)GetOwner();

	Move = FVector::ZeroVector;
	YVel = 0;
}

void UMainCharMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	// SafeMoveUpdatedComponent

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || MainChar == nullptr || ShouldSkipUpdate(DeltaTime))
		return;

	// Calculo el vector de movimiento
	FVector movementVector;
	InputVector = ConsumeInputVector().GetClampedToMaxSize(1.0f);
	movementVector = InputVector * MainChar->MovementSpeed;
	// TODO if running use run speed
	movementVector.Z = YVel;
	movementVector *= DeltaTime;

	// Move = movimientoDeseado;
	Move = FMath::Lerp(Move, movementVector, MainChar->StopLerpSpeed);

	// FVector movimientoEsteFrame = movimientoDeseado;

	// Movimiento
	if (!Move.IsNearlyZero() && (AMainChar::GetPlayerState() == EMainCharState::MOVING) || (AMainChar::GetPlayerState() == EMainCharState::ATTACK))
	{
		Move.Z = YVel;
		FHitResult Hit;

		// Movimiento
		if(AMainChar::GetPlayerState() == EMainCharState::MOVING)
			SafeMoveUpdatedComponent(Move, UpdatedComponent->GetComponentRotation(), true, Hit);

		// Si chocamos con algo, me deslizo sobre el
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(Move, 1.f - Hit.Time, Hit.Normal, Hit);
		
		movementVector.Z = 0;

		if (!movementVector.IsNearlyZero())
		{
			// Rotacion de la malla
			FRotator ctrlRot = movementVector.Rotation();

			if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot,  MainChar->RotationLerpSpeed);
			else if(PushActive && PushForward)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, MainChar->RotationLerpSpeed);
			UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
		}
	}

	if (IsGrounded())
	{
		// La velocidad Z es cero
		YVel = 0.0f;
	}
	else
	{
		// Aplico la gravedad
		YVel -= MainChar->GravityStrength * DeltaTime;
	}

	if (justJumped > 0)
		justJumped--;

	if (PushActive)
	{
		FHitResult Hit;
		FVector PushFrameMove;

		if (!PushForward)
			PushFrameMove = PushDir * DeltaTime * PushStrength;
		else
			PushFrameMove = GetOwner()->GetActorForwardVector() * DeltaTime * PushStrength;
		SafeMoveUpdatedComponent(PushFrameMove, UpdatedComponent->GetComponentRotation(), true, Hit);

		// Cuanto termine el tiempo, dejo de moverme por el knockback
		PushElapsedTime += DeltaTime;
		if (PushElapsedTime >= PushTime)
			PushActive = false;
	}

}

void UMainCharMovement::Jump()
{
	if (MainChar == nullptr)
		return;

	// if (IsGrounded())
	{
		YVel = MainChar->JumpStrength;
		justJumped = 4;
	}
}

void UMainCharMovement::Dodge()
{
	if (InputVector != FVector::ZeroVector)
		Push(2 * MainChar->MovementSpeed, MainChar->DodgeTime, false, InputVector);
	else
		Push(2 * MainChar->MovementSpeed, MainChar->DodgeTime, false, -MainChar->GetActorForwardVector());
}

void UMainCharMovement::ResetYVel()
{
	UE_LOG(LogTemp, Warning, TEXT("Reset Y Vel"));
	YVel = 0;
}

bool UMainCharMovement::IsGrounded()
{
	if (justJumped > 0)
		return false;

	FHitResult OutHit;
	
	FVector Start = UpdatedComponent->GetOwner()->GetActorLocation();

	float CapsuleHalfHeight = Cast<UCapsuleComponent>(UpdatedComponent)->GetUnscaledCapsuleHalfHeight();
	FVector End = Start + FVector(0, 0, -CapsuleHalfHeight - 12); // Capsule Half Height = 88

	FCollisionQueryParams ColParams;

	// DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1, 0, 1);

	// TODO change to SwipeTrace using a sphere

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, ColParams))
	{
		if (OutHit.bBlockingHit)
		{
			return true;

			print(FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			print(FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
			print(FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
		}
		return false;
	}

	return false;
}

void UMainCharMovement::Push(float strength, float time, bool forward, FVector direction)
{
	PushActive = true;
	PushStrength = strength;
	PushTime = time;
	PushElapsedTime = 0;
	PushDir = direction.GetSafeNormal();
	PushForward = forward;
}

void UMainCharMovement::Cancel()
{
	PushActive = false;
	YVel = 0;
}
