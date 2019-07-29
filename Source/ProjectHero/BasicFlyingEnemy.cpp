// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicFlyingEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ABasicFlyingEnemy::ABasicFlyingEnemy()
{
	HitRecooveryTime = 35;
	GroundRecoveryTime = 40;
	WakeUpTime = 40;
	MovementSpeed = 400;
}

void ABasicFlyingEnemy::BeginPlay()
{
	Super::BeginPlay();

	Movement->Flying = true;
}

void ABasicFlyingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EEnemyState::DEATH)
		return;

	if (State == EEnemyState::IDLE)
	{
		if (frameCount >= 5)
			State = EEnemyState::MOVING;
	}
	else if (State == EEnemyState::MOVING)
	{
		int FlyingHeight = 700;

		FVector v = (AMainChar::GetPlayerGroundLocation() - GetActorLocation()).GetSafeNormal();

		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(v, GetActorForwardVector())));
		bool orientationOk = angle < 30;
		bool distanceOk = FVector::Distance(GetActorLocation(), AMainChar::GetPlayerLocation()) < 500;

		FVector direction = (AMainChar::GetPlayerLocation() - GetActorLocation()).GetSafeNormal();
		if (GetActorLocation().Z - AMainChar::GetPlayerGroundLocation().Z < FlyingHeight)
			direction = (direction + FVector::UpVector).GetSafeNormal();
		else if(GetActorLocation().Z - AMainChar::GetPlayerGroundLocation().Z > FlyingHeight + 100)
			direction = (direction - FVector::UpVector).GetSafeNormal();

		if (!distanceOk)
			Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, GetActorLocation() + direction);
		else if (GetActorLocation().Z - AMainChar::GetPlayerGroundLocation().Z < FlyingHeight)
			Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, GetActorLocation() + FVector::UpVector);
		else if(GetActorLocation().Z - AMainChar::GetPlayerGroundLocation().Z > FlyingHeight + 100)
			Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, GetActorLocation() - FVector::UpVector);

		/*
		if (distanceOk && orientationOk)
		{
			currentAttackFrame = 0;
			AlreadyHit = false;
			State = EEnemyState::ATTACK_A;
		}
		*/
	}

	else if (State == EEnemyState::LAUNCHED)
	{
		if (Movement->IsGrounded())
		{
			State = EEnemyState::KNOCKED_DOWN;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::KD_HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::WAKE_UP;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::LAUNCHED_HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::LAUNCHED;
			frameCount = 0;
		}
		if (Movement->IsGrounded())
		{
			State = EEnemyState::KNOCKED_DOWN;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::KNOCKED_DOWN)
	{
		if (frameCount >= GroundRecoveryTime)
		{
			State = EEnemyState::WAKE_UP;
			frameCount = 0;
		}
	}
	else if (State == EEnemyState::WAKE_UP)
	{
		if (frameCount >= WakeUpTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
	frameCount += DeltaTime * 60;

	// Screen log
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	FString msg = FString::Printf(TEXT("FE State: %s, dtp:%f"), *EnumPtr->GetNameByValue((int64)State).ToString(), FVector::Dist(GetActorLocation(), AMainChar::GetPlayerGroundLocation()));
	if (GEngine) GEngine->AddOnScreenDebugMessage(4, 1.5, FColor::Blue, msg);
}

void ABasicFlyingEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
{
}

void ABasicFlyingEnemy::Death()
{
}
