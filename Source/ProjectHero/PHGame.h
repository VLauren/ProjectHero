// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "GameFramework/GameModeBase.h"
#include "PHGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API APHGame : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSet<AEnemy*> Enemies;

	UFUNCTION(BlueprintPure)
		TArray<AEnemy*> GetEnemies();

	void AddEnemy(AEnemy* enemy);

	TSet<AEnemy*> GetEnemiesInFront(FVector Position, FVector Direction);
	AEnemy * GetClosestEnemy(TSet<AEnemy*> Enems, FVector Position);
};
