// Fill out your copyright notice in the Description page of Project Settings.

#include "PHGame.h"
#include "MainChar.h"


void APHGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	Enemies.Reset();

	// UE_LOG(LogTemp, Warning, TEXT("PHGame InitGame !"));
}

TArray<AEnemy*> APHGame::GetEnemies()
{
	return Enemies.Array();
}

void APHGame::AddEnemy(AEnemy* enemy)
{
	Enemies.Add(enemy);

	// UE_LOG(LogTemp, Warning, TEXT("AddEnemy - %d"), Enemies.Num());
}

TSet<AEnemy*> APHGame::GetEnemiesInFront(FVector Position, FVector Direction)
{
	TSet<AEnemy*> res;

	for (int i = 0; i < GetEnemies().Num(); i++)
	{
		float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct((GetEnemies()[i]->GetActorLocation() - Position).GetSafeNormal(), Direction)));
		if (angle < 45)
			res.Add(GetEnemies()[i]);
	}

	return res;
}

AEnemy* APHGame::GetClosestEnemy(TSet<AEnemy*> Enems, FVector Position)
{
	if (Enems.Num() == 0)
		return nullptr;

	TArray<AEnemy*> arr = Enems.Array();
	AEnemy* res = arr[0];

	for (int i = 0; i < Enems.Num(); i++)
		if (FVector::Distance(arr[i]->GetActorLocation(), AMainChar::GetPlayerLocation()) < FVector::Distance(res->GetActorLocation(), AMainChar::GetPlayerLocation()))
			res = arr[i];

	return res;
}

AEnemy* APHGame::GetEnemyToTheRight(AEnemy* Current)
{
	if (Current == nullptr)
		return nullptr;

	FVector Origin = Current->GetActorLocation() - AMainChar::GetPlayerLocation();

	float MinAngle = 360;
	AEnemy* Res = nullptr;

	for (int i = 0; i < GetEnemies().Num(); i++)
	{
		FVector Dest = GetEnemies()[i]->GetActorLocation() - AMainChar::GetPlayerLocation();

		// Right angle check
		if (FVector::CrossProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal()).Z > 0)
		{
			// Get smallest angle
			float Angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct(Origin.GetSafeNormal(), Dest.GetSafeNormal())));
			if (Angle < MinAngle && GetEnemies()[i] != Current)
				Res = GetEnemies()[i];
		}
	}

	return Res;
}