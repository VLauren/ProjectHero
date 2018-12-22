// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicEnemy.h"
#include "EnemyMovement.h"
#include "MainChar.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"

void ABasicEnemy::BeginPlay()
{
	Super::BeginPlay();

	CapsuleComponent->SetVisibility(true);
	CapsuleComponent->SetHiddenInGame(false);
}

void ABasicEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Cast<UEnemyMovement>(GetMovement())->Move(DeltaTime, AMainChar::GetPlayerLocation());
}

void ABasicEnemy::Damage(int amount, FVector sourcePoint, float knockBack, bool launch)
{
	Super::Damage(amount, sourcePoint, knockBack, launch);
}
