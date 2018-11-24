// Fill out your copyright notice in the Description page of Project Settings.

#include "PHGame.h"
#include "MainChar.h"


void APHGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	Enemies.Reset();

	UE_LOG(LogTemp, Warning, TEXT("PHGame InitGame !"));
}

TSet<AEnemy*> APHGame::GetEnemiesInFront(FVector Position, FVector Direction)
{
	TSet<AEnemy*> res;

	return res;
}

AEnemy* APHGame::GetClosestEnemy(TSet<AEnemy*> Enems, FVector Position)
{
	if (Enemies.Num() == 0)
		return nullptr;

	TArray<AEnemy*> arr = Enemies.Array();
	AEnemy* res = arr[0];

	for (int i = 0; i < Enemies.Num(); i++)
		if (FVector::Distance(arr[i]->GetActorLocation(), AMainChar::GetPlayerLocation()) < FVector::Distance(res->GetActorLocation(), AMainChar::GetPlayerLocation()))
			res = arr[i];

	return res;
}