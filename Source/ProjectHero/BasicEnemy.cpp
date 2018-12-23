// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void ABasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);

	HitRecooveryTime = 35;
	GroundRecoveryTime = 40;
	WakeUpTime = 40;
}

void ABasicEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EEnemyState::IDLE)
	{
		if (frameCount >= 120)
			State = EEnemyState::MOVING;
	}
	if (State == EEnemyState::MOVING)
	{
		Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerLocation());
	}

	if (State == EEnemyState::LAUNCHED)
	{
		if (Movement->IsGrounded())
		{
			State = EEnemyState::GROUND;
			frameCount = 0;
		}
	}
	if (State == EEnemyState::HIT)
	{
		// Wait for hit recovery time
		if (frameCount >= HitRecooveryTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}
	if (State == EEnemyState::GROUND)
	{
		if (frameCount >= GroundRecoveryTime)
		{
			State = EEnemyState::WAKE_UP;
			frameCount = 0;
		}
	}
	if (State == EEnemyState::WAKE_UP)
	{
		if (frameCount >= WakeUpTime)
		{
			State = EEnemyState::IDLE;
			frameCount = 0;
		}
	}

	frameCount += DeltaTime * 60;

	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyState"), true);
	if (GEngine) GEngine->AddOnScreenDebugMessage(4, 1.5, FColor::White, EnumPtr->GetNameByValue((int64)State).ToString());;
}

void ABasicEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch)
{
	if (launch)
	{
		State = EEnemyState::LAUNCHED;
		frameCount = 0;
	}
	else
	{
		State = EEnemyState::HIT;
		frameCount = 0;
	}

	Super::Damage(amount, sourcePoint, knockBack, launch);
}
