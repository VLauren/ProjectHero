#include "EnemyMovement.h"
#include "Enemy.h"
#include "MainChar.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

void UEnemyMovement::BeginPlay()
{
	Super::BeginPlay();

	MoveVector = FVector::ZeroVector;

	StartGravity = Cast<AEnemy>(GetOwner())->GravityStrength;
}

void UEnemyMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SpLaunch && (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED || Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED_HIT) && ZVel < 1 && ZVel > -1)
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = 5;
	}
	else
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = StartGravity;
	}
}

void UEnemyMovement::Launch(float amount, bool spLaunch)
{
	UseGravity = true;
	ZVel = amount;
	SpLaunch = spLaunch;
}

void UEnemyMovement::AirHit()
{
	ZVel = 1;
}

void UEnemyMovement::Move(float DeltaTime, FVector Destination)
{
	// Get path
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), GetOwner()->GetActorLocation(), AMainChar::GetPlayerGroundLocation(), GetOwner());

	if (path != NULL && path->PathPoints.Num() > 1)
	{
		// for (int i = 0; i < path->PathPoints.Num(); i++)
			// DrawDebugSphere(GetWorld(), path->PathPoints[i], 10, 8, FColor::Green, true, 0.5f);

		// Get direction towards first route point
		FVector direction;
		
		int nextPoint = 0;
		while (path->PathPoints.Num() > nextPoint && FVector::Distance(GetOwner()->GetActorLocation(), path->PathPoints[nextPoint]) < 85)
			nextPoint++;

		if (nextPoint == 0)
			nextPoint++;

		direction = path->PathPoints[nextPoint] - GetOwner()->GetActorLocation();
		direction.Z = 0;
		direction.Normalize();

		// Move
		FVector move = direction * DeltaTime * Cast<AEnemy>(GetOwner())->MovementSpeed;
		move.Z = 0;
		FHitResult Hit;

		MoveVector = FMath::Lerp(MoveVector, move, 0.1f); // TODO move lerp speed variable

		SafeMoveUpdatedComponent(MoveVector, UpdatedComponent->GetComponentRotation(), true, Hit);
		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(move, 1.f - Hit.Time, Hit.Normal, Hit);

		if (!MoveVector.IsNearlyZero())
		{
			// Movement rotation
			FRotator ctrlRot = MoveVector.Rotation();

			if(Cast<AEnemy>(GetOwner())->State == EEnemyState::MOVING)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
			else if(PushActive && PushForward)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
			UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
		}
	}
}

void UEnemyMovement::RotateTowards(float DeltaTime, FVector Destination)
{
	// Movement rotation
	FRotator ctrlRot = (Destination - GetOwner()->GetActorLocation()).Rotation();

	// TODO WHAT THE FUDGE player state?
	if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
		CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
	else if (PushActive && PushForward)
		CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
	UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
}

bool UEnemyMovement::IsGrounded()
{
	// if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED)
	if(ZVel > 0)
		return false;

	return Super::IsGrounded();
}
