#include "EnemyMovement.h"
#include "Enemy.h"
#include "MainChar.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

void UEnemyMovement::BeginPlay()
{
	Super::BeginPlay();

	StartGravity = Cast<AEnemy>(GetOwner())->GravityStrength;
}

void UEnemyMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED && ZVel < 1 && ZVel > -1)
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = 5;
	}
	else
	{
		Cast<AEnemy>(GetOwner())->GravityStrength = StartGravity;
	}
}

void UEnemyMovement::Launch()
{
	UseGravity = true;
	ZVel = 24; // TODO variable or constant
}

void UEnemyMovement::AirHit()
{
	ZVel = 1;
}

void UEnemyMovement::Move(float DeltaTime, FVector Destination)
{
	// Get path
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), GetOwner()->GetActorLocation(), AMainChar::GetPlayerLocation(), GetOwner());

	if (path != NULL && path->PathPoints.Num() > 1)
	{
		// for (int i = 0; i < path->PathPoints.Num(); i++)
			// DrawDebugSphere(GetWorld(), path->PathPoints[i], 10, 8, FColor::Green, true, 0.5f);
		// DrawDebugSphere(GetWorld(), path->PathPoints[0], 10, 8, FColor::Green, true, 0.5f);
		// DrawDebugSphere(GetWorld(), path->PathPoints[1], 10, 8, FColor::Green, true, 0.5f);

		// Get direction towards first route point
		FVector direction;
		
		int nextPoint = 0;
		while (path->PathPoints.Num() > nextPoint + 1 && FVector::Distance(GetOwner()->GetActorLocation(), path->PathPoints[nextPoint]) < 85)
			nextPoint++;

		direction = path->PathPoints[nextPoint] - GetOwner()->GetActorLocation();
		direction.Z = 0;
		direction.Normalize();

		// Move
		FVector move = direction * DeltaTime * 200;
		move.Z = 0;
		FHitResult Hit;
		SafeMoveUpdatedComponent(move, UpdatedComponent->GetComponentRotation(), true, Hit);

		if (Hit.IsValidBlockingHit())
			SlideAlongSurface(move, 1.f - Hit.Time, Hit.Normal, Hit);

		if (!move.IsNearlyZero())
		{
			// Movement rotation
			FRotator ctrlRot = move.Rotation();

			if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
			else if(PushActive && PushForward)
				CurrentRotation = FMath::Lerp(CurrentRotation, ctrlRot, 0.1f);
			UpdatedComponent->GetOwner()->SetActorRotation(CurrentRotation);
		}
	}
}

bool UEnemyMovement::IsGrounded()
{
	// if (Cast<AEnemy>(GetOwner())->State == EEnemyState::LAUNCHED)
	if(ZVel > 0)
		return false;

	return Super::IsGrounded();
}
