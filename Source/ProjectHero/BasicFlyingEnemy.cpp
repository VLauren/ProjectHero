// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicFlyingEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "DrawDebugHelpers.h"
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
		FVector HeightMovement = FVector::ZeroVector;

		if (GetActorLocation().Z < FlyingHeight - 100)
		{
			// Cast<UEnemyMovement>(GetMovement())->SimpleMove(DeltaTime, FVector::UpVector, 400);
			HeightMovement = FVector::UpVector;
		}
		if (GetActorLocation().Z > FlyingHeight + 100)
		{
			// Cast<UEnemyMovement>(GetMovement())->SimpleMove(DeltaTime, FVector::DownVector, 400);
			HeightMovement = -FVector::UpVector;
		}

		// ==========================

		FVector GroundLoc = GetActorLocation();
		GroundLoc.Z = AMainChar::GetPlayerGroundLocation().Z;

		FVector dirTowardsPlayer = (AMainChar::GetPlayerGroundLocation() - GroundLoc).GetSafeNormal();
		dirTowardsPlayer.Z = 0;
		dirTowardsPlayer.GetSafeNormal();

		// DrawDebugSphere(GetWorld(), GroundLoc, 50, 8, FColor::Blue);
		// DrawDebugSphere(GetWorld(), AMainChar::GetPlayerGroundLocation(), 50, 8, FColor::Yellow);
		// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (dirTowardsPlayer * 200), FColor::Magenta);
		// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (FVector::UpVector * 200), FColor::Red);
		// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (FVector::DownVector * 200), FColor::White);

		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(dirTowardsPlayer, GetActorForwardVector())));
		bool orientationOk = angle < 30;
		bool farDistanceOk = FVector::Distance(GroundLoc, AMainChar::GetPlayerGroundLocation()) < DistanceToSeek + 200;
		bool closeDistanceOk = FVector::Distance(GroundLoc, AMainChar::GetPlayerGroundLocation()) > 150;

		FRotator targetRot = dirTowardsPlayer.Rotation();
		if (!farDistanceOk)
		{
			Cast<UEnemyMovement>(GetMovement())->SimpleMove(DeltaTime, dirTowardsPlayer + HeightMovement, 400);
			RotateTowards(DeltaTime, targetRot.Add(-FlyingInclination, 0, 0));
		}
		else if (!closeDistanceOk)
		{
			Cast<UEnemyMovement>(GetMovement())->SimpleMove(DeltaTime, -dirTowardsPlayer + HeightMovement, 400);
			RotateTowards(DeltaTime, targetRot.Add(FlyingInclination, 0, 0));
		}
		else
		{
			Cast<UEnemyMovement>(GetMovement())->SimpleMove(DeltaTime, HeightMovement, 400);
			RotateTowards(DeltaTime, targetRot);
		}

		if (farDistanceOk && !orientationOk)
		{
			RotateTowards(DeltaTime, targetRot);
		}

		AddActorWorldOffset(HeightMovement * 400 * DeltaTime);

		if (frameCount > AttackTime * 60)
		{
			OnAttack();
			frameCount = 0;
		}

		// ===========================================

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
		FVector GroundLoc = GetActorLocation();
		GroundLoc.Z = AMainChar::GetPlayerGroundLocation().Z;

		FVector dirTowardsPlayer = (AMainChar::GetPlayerGroundLocation() - GroundLoc).GetSafeNormal();
		dirTowardsPlayer.Z = 0;
		dirTowardsPlayer.GetSafeNormal();

		FRotator targetRot = dirTowardsPlayer.Rotation();
		targetRot.Yaw = 0;
		RotateTowards(DeltaTime, targetRot);

		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::MOVING;
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

void ABasicFlyingEnemy::RotateTowards(float DeltaTime, FRotator TargetRot)
{
	if (AMainChar::GetPlayerState() == EMainCharState::MOVING)
		CurrentRotation = FMath::Lerp(CurrentRotation, TargetRot, AngleLerpStrength);

	SetActorRotation(CurrentRotation);
}

void ABasicFlyingEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch, float riseAmount, bool spLaunch)
{
	if (launch)
	{
		State = EEnemyState::LAUNCHED;
	}
	else
	{
		if(State == EEnemyState::KNOCKED_DOWN || State == EEnemyState::KD_HIT)
			State = EEnemyState::KD_HIT;
		else if (Movement->IsGrounded())
			State = EEnemyState::HIT;
		else
			State = EEnemyState::LAUNCHED_HIT;
	}

	frameCount = 0;

	Super::Damage(amount, sourcePoint, knockBack, launch, riseAmount, spLaunch);

	SetActorRotation(GetActorRotation().Add(60, 0, 0));
	CurrentRotation = GetActorRotation();
}

void ABasicFlyingEnemy::Death()
{
}
